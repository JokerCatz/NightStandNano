# 指令參考

UART 序列埠（115200 baud，一行一個指令）與 BLE 的 CMD 特徵使用同一張指令表（`app/main/cmd.c` 的 `k_cmds`）。
網頁與 App 用的就是這些指令；回覆是文字行。標示「**只有 UART**」的指令經 BLE 一律回 `?`（由指令分派器統一擋下，見 `cmd.c`）：
寫檔、刪檔、改腳位、重開機、會讓裝置當機的測試，都只能接線操作。

## BLE GATT

服務 `7f2a0001-a1c4-4e53-9a31-6e73616e6f00`（廣告名稱 `NightStand Nano`）

| 特徵 | UUID | 方向 | 內容 |
|---|---|---|---|
| CMD | `7f2a0002-…` | 寫入 | 一行文字指令 |
| EVT | `7f2a0003-…` | 通知 | 回覆文字，每行以 `\n` 結尾，依 MTU 切段 |
| SPEC | `7f2a0004-…` | 通知／讀 | `[序號, 32, 32 個頻段 0..255]`（40 Hz–16 kHz，約每 33 ms 一筆） |

BLE 沒有加密，範圍內的任何人都能連線與控制播放，這是設計取捨（見 README「安全」）。

## 狀態

| 指令 | 說明 |
|---|---|
| `st` | 一行 JSON：狀態（`st`: idle／playing／paused）、卡片（`card`: ok／bad／none）、格式、取樣率、位置、長度、音量、曲目、睡眠剩餘、兩核心 CPU%（`cpu`）、播放方式、斷音次數（`und`）、輸出、DAC／藍牙／Wi-Fi 是否連線、可用記憶體、歌名、電台狀態、檔名、是否有書籤（`bm`）、標籤（`artist`／`album`）、ReplayGain（`rg`）、重型效果是否暫停（`fxshed`）、無縫播放（`gapless`）、音量上限（`vmax`）、DAC 上限（`dacmax`） |
| `status` | 播放狀態的一行文字 |
| `spec` | 目前的 32 頻段（十六進位） |
| `ver` | `ver <版本> idf=<IDF 版本> chip=esp32s31 built=<ELF SHA-256 前 9 碼>`；建置可重現，所以沒有日期 |
| `card` | 一行：`card state=mounted\|unreadable\|none size= free= fs= name= serial= fmt=` |
| `time` | 時鐘狀態；`time set <秒> [時區]`（網頁用來校時，NTP 已同步時只套用時區）、`time tz <時區>`（預設 CST-8） |

## 播放

| 指令 | 說明 |
|---|---|
| `play <路徑或資料夾>` | 檔案、資料夾（連續播放）、`.zip`（當資料夾走進去，`<zip>/<項目>` 也可）。回覆 `play: ESP_OK`，或 `play: <esp 錯誤名>`；`.vgm` / `.vgz` 由 `vgmgate.c` 看標頭分流：GME 能播的晶片（SN76489、YM2612、YM2413）給 GME，YM2151／YM2203（單或雙晶片）給 FM 後端（`components/fmvgm`），`.s98` 一律給 FM 後端；單一檔案被拒絕時是 `play: ESP_ERR_NOT_SUPPORTED unsupported chip YM2608, OKIM6295`（寫了沒有人能播的晶片）、`… unsupported chip SN76489 + YM2151 (mixed)`（GME 的晶片與 FM 晶片混在同一檔，一個檔只能由一個後端播）或 `… too heavy for Bluetooth AAC`（這個晶片組在目前輸出與核心配置下已知太重，換輕的輸出就能播）。資料夾、播放清單、zip 內打不開或被拒絕的檔會被跳過，`events` 多一行 `skip <檔名>: chip YM2151`、`skip <檔名>: too heavy for Bluetooth AAC`（或 GME 的理由）；播到一半 FM 晶片的產生跟不上（見 `dbg fm`）也是同樣的一行，然後換下一首 |
| `stop`、`pause`、`resume` | 待機時 `resume` 從書籤（上次停止的位置）接著播 |
| `next`、`prev` | 多曲檔（晶片音樂）先在檔內換曲 |
| `vol [0-100]` | 目前輸出的音量（每種輸出各自記住），不會超過 `vollimit` |
| `vollimit [0-100]` | 音量上限，預設 100，任何路徑都不會超過 |
| `mode folder\|shuffle\|one\|once` | 資料夾循環、隨機、單曲重複、播完停止 |
| `gapless [on\|off]` | 無縫播放（同取樣率、同輸出時），預設開 |
| `expfmt [on\|off]` | 「實驗性」tracker／AdLib 格式（a2m a2t d00 sa2 …，見 `pc_chipseq.c`）：預設開，下一個檔起生效 |
| `rg [off\|track\|album]` | 依標籤套用 ReplayGain，下一首起生效 |
| `ls [路徑]` | 列出資料夾（預設 `/sdcard`）；資料夾與 `.zip` 結尾加 `/` |
| `lrc` | 把播放中曲目的同名 `.lrc` 以 base64 分行送出（`lrc begin <bytes>`、`lrc+ <base64>`…、`lrc end`；沒有則 `lrc none`），由網頁解析與對時 |
| `autoplay 0\|1` | 開機後是否接續上次的來源 |
| `timer <分鐘>` | 睡眠計時（0 取消），最後 30 秒淡出 |

## 來源

| 指令 | 說明 |
|---|---|
| `radio <網址>` | 播放網路電台（http／https；MP3／AAC／Ogg；HLS `.m3u8` 的 TS／AAC、packed AAC／MP3 分段；fMP4、AC-3、加密會被拒絕並在 `rstate` 顯示原因） |
| `radio` 或 `radio list` | 列出 `/sdcard/_player/config/radio.txt` 的收藏 |
| `radio <N>` | 播放第 N 個收藏 |
| `radio add <名稱>\|<網址>`、`radio del <N>` | 改收藏。**這是 BLE 唯一能改卡上檔案的路徑**：固定檔案 `radio.txt`、只收 http(s) 網址、長度與數量有限 |
| `noise [顏色]` | 白噪音：white pink brown blue violet grey green velvet |
| `scene`、`scene <N>` | 列出／播放場景（最多 3 層環境音循環混音，由 `tools/make_scenes.py` 產生在 `/sdcard/_player/scenes/`） |

## 輸出與藍牙

| 指令 | 說明 |
|---|---|
| `out auto\|dac\|bt\|cast` | 選輸出：自動、USB DAC、藍牙耳機、Auracast 廣播。`out bt` 會切到耳機模式；`out cast` 與藍牙耳機互斥，第一次啟動 LE Audio 時 BLE 控制會斷線約 0.3 秒（網頁自動重連），要重開機才會釋放 |
| `cast` | Auracast 廣播狀態 |
| `bt` | 藍牙狀態 |
| `bt mode speaker\|headphones` | A2DP 角色：接收（手機→盒子）或發送（盒子→耳機）；一次只能一種 |
| `bt pair [秒]` | 開放配對視窗（預設 120 秒） |
| `bt scan`、`bt list` | 搜尋與列出附近裝置 |
| `bt conn <MAC>`、`bt disc`、`bt pause`、`bt unpair` | 連線、斷線、暫停串流、忘記全部配對 |
| `bt tone`、`bt ldactest` | 測試：1 kHz 音調送到耳機；離線 LDAC 編碼測試 |
| `dacmax [auto\|<Hz>]` | USB DAC 的取樣率上限（48000／96000／192000／384000，`96k` 也可），超過的曲目降取樣 |
| `ratetest [status\|cancel]` | 逐一實測 DAC 宣告的取樣率，結果依 USB 識別碼存 NVS |
| `keys` | 顯示線控按鍵設定與最後偵測到的手勢；`keys preset 1\|2\|3`（單鍵、雙鍵、三鍵耳機的預設組）；`keys set <c\|p\|m> <1\|2\|3\|h> <none\|play\|next\|prev\|vol+\|vol-\|stop>`（鍵 × 手勢 → 動作） |

## 睡眠與夜間

| 指令 | 說明 |
|---|---|
| `bedtime [分鐘]` | 夜間模式＋睡眠計時（預設 30 分鐘） |
| `wake` | 結束夜間模式 |
| `night on\|off\|auto HH:MM HH:MM\|auto off` | 夜間模式與每日時段 |
| `led <亮度 0-255>` | LED 亮度（0 關閉；夜間模式會壓低上限） |
| `led <R> <G> <B>` | 測試顏色 3 秒 |

## 音效

| 指令 | 說明 |
|---|---|
| `fx params` | 列出效果與參數（網頁依此自動產生面板） |
| `fx state`、`fx on <名稱> 0\|1`、`fx set <名稱> <參數> <值>`、`fx off` | 讀狀態、開關、設參數、全部關閉 |
| `fx import <檔名>`、`fx export` | 匯入／匯出效果設定 |
| `fxs` | 列出卡上的效果設定檔 |

LDAC 或 Auracast 時重型效果自動暫停；核心 1 連續過載 2 秒也會暫停（`st` 的 `fxshed`）。

## Wi-Fi

| 指令 | 說明 |
|---|---|
| `wifi` | 狀態與來源：`wifi <state> ip= rssi= time= src=nvs\|card\|none ssid="…"`；state 為 connected／connecting／wrong password／not found／off／no credentials |
| `wifi on`、`wifi off` | 開關 |
| `wifi set <名稱>\|<密碼>` | 存入 NVS 並立刻重連（以第一個 `\|` 分隔，密碼可含 `\|`）。密碼不可讀回、不進日誌；經 BLE 設定時**密碼以明文傳送** |
| `wifi forget` | 忘記 NVS 的設定，回到卡上的 `wifi.txt`（若有） |
| `wifi scan` | 掃描，最多 20 個，強到弱：`wifi ap <rssi> open\|secured "<名稱>"` |

經網頁設定的優先於卡上的 `/sdcard/_player/config/wifi.txt`（格式 `ssid=` ／ `pass=` 各一行）。

## 記憶卡

| 指令 | 說明 |
|---|---|
| `cardtest`、`cardtest raw\|file`、`cardtest cancel` | 測速（原始讀取／檔案寫入讀回驗證），結果與各音訊格式的夠不夠快判定；除了一個暫存檔外不改卡上資料 |
| `cardfmt` | **只有**在卡片有回應但檔案系統掛載失敗時才提供格式化：三步、兩組不同的確認碼（各 30 秒內）。能讀取的卡一律拒絕 |
| `cardfmt <代碼>`、`cardfmt status`、`cardfmt cancel` | 確認流程 |
| `cardfmt selftest`、`cardfmt sim on\|off` | **只有 UART**：記憶體磁碟上的格式化測試／假裝卡片壞掉 |

## 硬體設定（腳位）

`hw` 與 `sdpins`：讀取所有人可用；**所有設定只有 UART**（BLE 上帶參數一律回 `?`），重開機才生效。細節與回覆格式見 `docs/porting.md`。

| 指令 | 說明 |
|---|---|
| `hw` | 讀目前設定與待生效的變更 |
| `hw sd <clk> <cmd> <d0> <d1> <d2> <d3> <width>` | TF 卡 SDMMC 腳位（`width` 為 1 時 `d1`–`d3` 填 `-1`） |
| `hw sd spi <clk> <mosi> <miso> <cs>` | TF 卡 SPI 模式（自己焊的卡座） |
| `hw led <gpio> [rgb\|grb\|brg\|rbg\|gbr\|bgr]`、`hw led off` | 狀態 LED 的 GPIO 與顏色順序 |
| `hw reset` | 全部回預設 |
| `sdpins [...]` | 舊指令，規則同 `hw sd` |

## 只有 UART

| 指令 | 說明 |
|---|---|
| `reboot` | 重新啟動 |
| `putx! <路徑> <十六進位>`、`putx <路徑> <十六進位>` | 寫檔（`!` 從頭寫，否則附加）；內容不回顯、不進日誌。`tools/put_file.py` 用它推檔案到卡上 |
| `mkdir <路徑>`、`rm <路徑>` | 建資料夾、刪檔 |
| `mkwav <路徑> [取樣率]` | 產生 5 秒測試 WAV |
| `crash` | 故意當機（測試當機記錄與安全模式） |
| `memwatch [秒]` | 記錄這段時間內可用內部記憶體的最低點（1–60 秒） |
| `keys sim <c\|p\|m> <1\|2\|3\|h>` | 模擬線控按鍵的手勢 |
| `dbg …` | 測試用：`dbg oc arm\|off\|show`（輸出檢查）、`dbg vol [forget bt\|cast]`、`dbg ramp`、`dbg looplen <ms>`、`dbg scene …`、`dbg trim on\|off`、`dbg rt <rate> ok\|bad\|untested`、`dbg opendelay <ms>`、`dbg xfade <ms>`、`dbg spare 0\|1`、`dbg hlsreuse 0\|1`（HLS 是否重用連線）、`dbg conns`（開過幾條連線）、`dbg fm …`（FM 後端，見下）|
| `hw sd\|led\|reset`、`sdpins` 的設定 | 見上 |
| `cardfmt selftest`、`cardfmt sim` | 見上 |

不認得的指令回 `?`；需要參數卻沒給、或不該有參數卻給了，也回 `?`。

## `dbg fm`（只有 UART；FM 後端 `components/fmvgm`，YM2151 / YM2203 的 VGM 與 S98）
| 指令 | 說明 |
|---|---|
| `dbg fm core 0\|1\|inline` | 晶片合成在哪裡跑：核心 0 的常駐工作者、核心 1 的工作者、或直接在解碼任務的 `render()` 裡（預設 `inline`，量測後依 `fmglue.c` 的註解決定）。播放中也能換，下一個區塊生效 |
| `dbg fm prio N` | 工作者優先權（預設 1：比所有有工作要做的任務低，只用沒人要的 CPU） |
| `dbg fm taps N`、`dbg fm ring MS`、`dbg fm loops N` | 重取樣器長度（預設 24）、生產者環形緩衝深度（預設 340 ms）、迴圈次數（預設 2，之後淡出 8 秒）；下一個檔起生效 |
| `dbg fm rate auto\|native\|<Hz>` | 後端交給播放器的取樣率：`auto` = 輸出端的取樣率（播放器不再重取樣）；`native` = 晶片原生率（播放器的 pc_rsq 轉換，量測舊作法用）；下一個檔起生效 |
| `dbg fm thr <視窗ms> <門檻ms>` | 過載規則：視窗內插入的靜音超過門檻就停掉這首並換下一首（預設 6000 / 600；門檻 0 = 不檢查） |
| `dbg fm check 0\|1` | 靜態預檢（哪些晶片組太重）開關，預設 1 |
| `dbg fm sum on\|off`、`dbg fm stat` | 整數校驗和（晶片原始輸出總和、輸出雜湊）；目前或最近一首的統計（生產者環形緩衝最低水位、插入靜音、每秒音訊的生產成本、跳過次數） |
| `dbg fm bench <路徑> [秒] [核心] [優先權]` | 只跑生產者（晶片 + 重取樣）、不出聲，印每秒音訊的成本與校驗和（與 PC 的 `test_fmvgm bench` 逐位元相同） |
