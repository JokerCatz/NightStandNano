# NightStand Nano — 需求表

草稿：2026-11-24，由 Claude 依 round / nightstand / chipchill / JukeChip 的功能與目前實測進度整理，**優先順序待主人裁定**。
欄位：`P1` 第一版必做、`P2` 第一版之後、`P3` 想要但可放很後面。
狀態：`完成`（板上實測過）、`部分`（做了一半或只測了一半）、`未做`、`待決定`。
驗證：不靠主人也能做的寫「自動」，需要耳機／喇叭／手機／眼睛的寫「人」。

## A. 輸入來源（音樂從哪來）
| ID | 需求 | 來源 | 優先 | 狀態 | 驗證 |
|---|---|---|---|---|---|
| A1 | TF 卡檔案播放，FAT32 與 exFAT，資料夾連續播放、上一首／下一首 | round, nightstand | P1 | 部分：FAT32 完成；exFAT 只有 RAM disk 實測 | 自動；真 exFAT 卡要人插卡 |
| A2 | 格式：mp3 flac wav aiff mpc tta | round, chipchill | P1 | 完成（wav mp3 flac aiff 用 ffmpeg 產生的檔案實測，0 underrun；mpc tta 未測） | 自動（需要測試檔） |
| A3 | 格式：aac / m4a / ogg / opus | round (`backend_sdec`), chipchill | P1 | 完成（.aac .ogg .opus .m4a 用 ffmpeg 產生的檔案實測，0 underrun） | 自動 |
| A4 | 晶片音樂：gme（NES/GB/SNES…）、xmp tracker、sid、opl、hvl、ym、vgm | round, chipchill, JukeChip | **P1（主人 2026-11-24）** | **完成（2026-11-25 逐格式實測，表見文末「晶片／tracker 格式實測」）：nsf（含 VRC6 / VRC7 / N163 / MMC5 / FDS / 5B）gbs hes kss sap sid ay ym ahx hvl vgz mod xm s3m it rad hsc d00 a2m amd sa2 都用真檔播過；nsfe 只有合成檔；沒有測試檔所以沒測：spc gym p8 與大部分冷門 tracker／AdLib 副檔名。已知不能播：SAP TYPE D、A2M v9–11、RSID 中靠真 C64 時序的曲子、含 GME 不支援晶片的 VGM（現在明確拒絕並說出晶片）** | 自動（已做）；聽感要人 |
| A5 | 網路電台 MP3（http/https、ICY 歌名、.pls .m3u） | round, nightstand | P1 | 完成 | 自動 |
| A6 | 網路電台 AAC / Ogg / HLS、電台清單檔 `radio.txt`、備援網址 | round | P1 | **部分：AAC 實測（SomaFM，0 underrun）、`radio.txt` 與備援網址實測（主網址失效自動換備援）、依串流內容偵測格式；Ogg 已接線未測；HLS 不做** | 自動 |
| A7 | **手機藍牙串流進來，輸出到小尾巴**（A2DP sink → USB DAC），SBC / AAC | round（喇叭模式） | P1 | **完成（以 round 當手機，AAC，串流 92 秒 0 underrun；真手機未測）** | 自動（對 round）；真手機要人 |
| A8 | 手機串流中繼到藍牙耳機（sink + source 同時，不經小尾巴） | 主人新提出 | 不做 | **做不到**：板上實驗 `esp_a2d_sink_init()` 在 source 已初始化時回 `ESP_ERR_INVALID_STATE`（2026-11-24），Bluedroid 一次只能一種 A2DP 角色 | — |
| A9 | 白噪音：8 種顏色噪音 | nightstand, JukeChip, round | P1 | 完成 | 自動 |
| A10 | 白噪音場景：多層環境音混音（雨、海浪…，`library.muk`） | nightstand (sleepmix), round | P2 | 部分：`scene` / `scene N` 已寫（最多 3 層循環混音，PSRAM），`tools/make_scenes.py` 由 library.muk + default_scenes.json 產生卡上檔案；兩個場景上板實測（2 層混音、44.1／48 kHz、循環）；完整 695 MB 資料庫要把卡取出複製 | 自動 |
| A11 | 電腦用 USB 線把 Nano 當音效卡 | round（外接模式） | P3 | 未做 | 人 |
| A12 | Auracast 收聽（LE Audio 廣播接收） | round | 不做 | 主人裁定只要廣播出去（2026-11-25） | — |

## B. 輸出（音樂到哪去）
| ID | 需求 | 來源 | 優先 | 狀態 | 驗證 |
|---|---|---|---|---|---|
| B1 | USB-C DAC（小尾巴），8–384 kHz | 主人 | P1 | 完成 | 自動；音質要人 |
| B2 | 藍牙耳機／喇叭 A2DP SBC（外部編碼器） | round | P1 | 部分：SBC 編碼接好但只驗證了 AAC（round 協商到 AAC） | 自動（對 round）；真耳機要人 |
| B3 | A2DP AAC | round | P1 | **完成（對 round，開機自動重連 + 整個資料夾播放）** | 自動（對 round）；真耳機要人 |
| B4 | **A2DP LDAC**（含 ABR 降階） | round（`tools/bt_ldac`、libldac） | P1 | **部分：編碼與協商都接好（修補過的 Bluedroid、LDAC 端點、ABR、Wi-Fi 同時降階）；離線編碼檢查通過（四個品質 908/601/451/300 kbps，封包同步位元組全對）；LDAC 本身沿用 round（round 已用真耳機驗證），本專案不重測** | 沿用 round |
| B5 | A2DP aptX / aptX HD | round | P2 | 未做 | 同上 |
| B6 | 取樣率轉換（polyphase） | round | P1 | 完成 | 自動 |
| B7 | 輸出自動選擇／手動切換（DAC、藍牙），裝置消失時安全停止 | round, nightstand | P1 | 完成 | 自動 |
| B8 | 記住上次藍牙裝置並自動重連；斷線後重連 | round | P1 | 部分：開機重連完成；斷線後 10 分鐘內重連成功會自動續播同一首（已接線，編譯通過，未在板上測） | 自動 |
| B9 | AVRCP：耳機按鍵（播放／暫停／上下首）與絕對音量 | round | P2 | 部分：按鍵（播放 暫停 停止 上下首 音量加減）已接線，耳機模式連線回歸通過（AAC + avrcp connected）；真按鍵未測；絕對音量未做 | 人 |
| B10 | Auracast 廣播（一對多） | round | **P1（主人 2026-11-25：只要廣播出去）** | 完成：`out cast`（沿用 round 的廣播來源程式，無 UI 版 `cast.c`），LC3 48 kHz 立體聲；與藍牙耳機輸出互斥；Wi-Fi、BLE 控制可同時開；JukeDial 收聽實測約 1.1% 補償；數據見 `docs/measurements.md`。真 Auracast 耳機未測 | 自動（對 round）；真耳機要人 |

## C. 控制
| ID | 需求 | 來源 | 優先 | 狀態 | 驗證 |
|---|---|---|---|---|---|
| C1 | BLE GATT 控制服務 + 頻譜通知 | JukeChip 0019, 主人 | P1 | 完成 | 自動（bleak） |
| C2 | Web Bluetooth 網頁（GitHub Pages）：播放、檔案瀏覽、頻譜、音量、計時、電台、噪音、鬧鐘 | 主人, tunetap | P1 | 部分：寫好，未在真瀏覽器測 | 人（Chrome） |
| C3 | 手機／原生 App 控制（iOS 沒有 Web Bluetooth） | 主人 | P2 | 未做 | 人 |
| C4 | UART 控制台（開發與救援） | round | P1 | 完成 | 自動 |
| C5 | BLE 安全：控制層開放（JukeChip 0019 立場）；檔案與設定指令僅序列埠 | JukeChip | P1 | 完成（採用 0019，待主人確認） | 自動 |
| C6 | Wi-Fi 帳密：卡上 `wifi.txt` | round, JukeChip | P1 | 完成 | 自動 |
| C7 | Wi-Fi 帳密：BLE 一次性密碼配置窗（沒螢幕，用 LED 閃碼顯示） | JukeChip 0019 | P2 | 未做 | 人 |
| C8 | 網頁刷機／韌體更新（Web Serial 或 OTA） | round | P3 | 未做 | 人 |
| C9 | 頻譜／視覺化網頁風格與 tunetap 外掛共用 | tunetap, chipchill | P1（主人 2026-11-24） | 完成：tunetap `js/viz` 原樣複製到 `web/viz/`（宿主、主題、預設、樣式），模式：頻譜、瀑布、夜行、錄音帶；波形與相位不支援（主人裁定，裝置不送波形）；資料為 32 段 BLE 頻譜經 `viz-nano-tap.js` 轉接；示範模式截圖看過；網頁變多檔，需經 http 開啟 | 人 |

## D. 床頭功能
| ID | 需求 | 來源 | 優先 | 狀態 | 驗證 |
|---|---|---|---|---|---|
| D1 | 睡眠計時（最後 30 秒淡出） | nightstand, round | P1 | 完成 | 自動 |
| D2 | 每日鬧鐘 | nightstand, JukeChip | 不做 | 主人裁定移除（2026-11-24） | — |
| D3 | 設定記憶（音量、輸出、LED、鬧鐘、最後來源） | nightstand | P1 | 完成 | 自動 |
| D4 | RGB LED 一直顯示狀態：顏色＝在做什麼（綠 小尾巴播放／青 藍牙耳機／紫 手機串流／琥珀 暫停／藍 待機），動態＝怎麼做（快閃 電台連線中、慢呼吸 睡眠計時），紅燈閃幾下＝缺什麼（1 無輸出、2 無卡、3 電台連不上、4 安全模式） | round, JukeChip, 主人 | P1 | 顏色順序主人實看確認（紅綠藍白正確，紅略偏橘）；新燈號已寫入，待主人實看 | 人（看） |
| D5 | 收藏與播放紀錄（存在卡上） | nightstand | P2 | 未做 | 自動 |
| D6 | EQ 與音效鏈（EQ、tone、bass、compressor、freeverb、tube… 共 13 個，匯入 chipchill preset） | nightstand, chipchill, round | P2 | 部分：直接移植 round 的 pc_fxchain + fx_state（去掉 LVGL），`fx on/set/state/import/export`、存卡上 fx.txt；7 個重型效果同時開 + 場景播放 25 秒 0 underrun；聽感未驗；APO/REW/AutoEq 匯入未做；BLE 網頁沒有 fx 介面 | 自動（頻響）＋人 |
| D7 | 響度正規化（EBU R128）與 ReplayGain | chipchill | P2 | 未做 | 自動 |
| D8 | 歌詞 LRC / SRT 隨 BLE 推到網頁 | chipchill | P3 | 未做 | 自動 |
| D9 | 場景預設（一鍵：睡眠 = 噪音 + 計時 + 暗 LED） | nightstand | P2 | 未做 | 自動 |
| D10 | 專注／番茄鐘、呼吸教練（噪音跟隨） | round | P3 | 未做 | 人 |
| D11 | 角色／萌寵、遊戲 | chipchill, round | 不做 | — | — |

## E. 平台與品質
| ID | 需求 | 來源 | 優先 | 狀態 | 驗證 |
|---|---|---|---|---|---|
| E1 | TF 腳位使用者可調（NVS，不用重編） | 主人 | P1 | 完成。已驗證：SDMMC 在 S31 上走 IOMUX、只能用槽 0 固定腳位，任意腳位改用 SPI 模式（`hw sd spi`，見 `docs/porting.md`）；SPI 模式沒有在自焊卡座上驗證 | 自動；換腳位要人接線 |
| E2 | 記憶體預算：內部 RAM 可用 ≥ 60 KB（Wi-Fi + 藍牙 + BLE + 播放中） | round | P1 | 完成：約 100 KB（不寫 flash 的任務堆疊放 PSRAM；解碼任務常駐） | 自動 |
| E3 | 長時間 soak（24 h 播放，Wi-Fi + 藍牙，無洩漏、無當機） | round | 不做 | 主人裁定：不做長時間測試 | — |
| E4 | 當機記錄與看門狗，開機連續當機保護 | round | P2 | 部分：當機記錄與開機迴圈保護完成（見 F1）；任務看門狗用 IDF 預設 | 自動 |
| E5 | 雙核配置：核心 0 協定與網路，核心 1 解碼與編碼（沒有 UI，兩核全給音訊） | 主人 | P1 | 部分 | 自動（CPU 量測） |
| E6 | 開發者文件、需求追蹤、測試腳本 | 主人 | P1 | 進行中 | — |
| E7 | 公開發布整理（授權清單、白名單匯出）如 JukeDial | round | P3 | 未做 | — |

## F. 研究後新增的候選功能（2026-11-24，來源：round `docs/dev/features.md`、nightstand TODO、chipchill）
判斷標準：沒有螢幕、只有藍牙控制、記憶體小。「沿用」指 round `player_core` 已有純 C 實作，直接連結。
| ID | 功能 | 沿用 | 適合度 | 優先 | 狀態 |
|---|---|---|---|---|---|
| F1 | 當機記錄（重開機原因、PC、時間存卡）與連續開機失敗保護 | `pc_crash`（行格式） | 高：沒螢幕時唯一的除錯線索 | P1 | 完成：RTC 麻包屑 + 開機後寫 `/sdcard/_player/logs/crash.txt`；連續 3 次 60 秒內當機進安全模式（不自動播放、不開音效）；UART `crash` 測試指令，實測通過。尚無 PC 位址（沒包 panic handler） |
| F2 | 隨機播放（shuffle）、重複模式（單曲／資料夾／關） | 自寫（小歷史） | 高 | P1 | 完成：`mode folder|shuffle|one|once`，存設定，板上實測（隨機、上一首走歷史、單曲重複、播一次） |
| F3 | 斷電續播：記住曲目與位置 | 自寫（settings 已有 last） | 更正（2026-11-24）：後端有 `byte_pos` 續播權杖與 `open(start_off)`，可以續播，Nano 只是沒接線（先前寫「解碼器不能跳轉」是錯的） | 待定 | 部分：只記來源（autoplay），不記位置；見 `docs/ideas.md` #4 |
| F4 | ReplayGain / 響度正規化 | `pc_rg`（標籤）、`loudness` 效果 | 中：睡前不要忽大忽小 | P2 | 未做 |
| F5 | 場景預設：一鍵「睡眠」＝噪音／場景＋計時＋LED 暗 | 自寫 | 高 | P2 | 完成：`bedtime [分]`（夜間音效 + 上次的噪音／場景或棕噪音 + 計時 + LED 上限 2）、`wake`、`night on/off`；網頁兩顆按鈕；板上實測（含 fx 開關與還原） |
| F6 | 夜間 LED 排程（自動變暗／全關） | 自寫 | 高 | P2 | 完成：`night auto HH:MM HH:MM`（需 NTP，邊緣觸發，手動切換持續到下個邊緣），存 NVS 獨立鍵；排程切換的實際時間邊緣未等到，只測了指令與狀態 |
| F7 | 多組鬧鐘／星期選擇 | `alarm.c` 擴充 | 中 | P2 | 未做 |
| F8 | 無縫循環（場景接縫交叉淡化）、無縫換曲 | `pc_loop`, `pc_gapless` | 中：場景現在是簡單重播，接縫可能有聲音 | P2 | 未做 |
| F9 | 收藏與播放紀錄 | `pc_hist`、stats | 中 | P2 | 未做 |
| F10 | 呼吸引導（LED 呼吸 + 噪音跟隨） | `pc_breath` | 中：LED 本來就在，很貼合 | P3 | 未做 |
| F11 | 網頁介面：分組分頁、亮暗主題、手機與 PC（最寬 1024） | web/index.html | 高 | P1 | 完成重做（2026-11-24）：上方「正在播放」常駐（頻譜、上下首、音量、輸出），六個分頁：檔案／電台／噪音／床頭／音效／系統；跟隨系統亮暗；寬度 ≥ 860 px 變兩欄；狀態回填；`?demo` 示範模式（無裝置可預覽）。用無頭 Chrome 截圖看過三種版面；**真裝置上的 Web Bluetooth 連線仍未測** |
| F12 | 歌詞（LRC）：整包送到網頁，網頁依播放位置對時 | 裝置只讀檔傳送 | 主人同意此做法 | P2 | 完成：`lrc` 指令板上實測（base64 分行）；網頁解析（UTF-8／Big5／GBK）、卡片三行＋「歌詞」分頁，示範模式看過，真連線未測 |
| F13 | 韌體更新（OTA） | — | 主人裁定不做，故障重刷 | 不做 | — |
| F14 | 電腦當 USB 音效卡、Auracast、遊戲、儀器模式、相簿 | round | 低或不適用（沒螢幕／沒硬體） | 不做 | — |

### F11a 音效／濾波評估（裝置端處理；網頁與 App 只是遙控，本身不出聲）
| 效果 | 床頭用途 | 成本（round 標示） | 建議 |
|---|---|---|---|
| equalizer（參數式，最多 10 段） | 補耳機／小尾巴頻響、夜間削高頻 | 重 | 保留；段數調整頁待做（目前只能調 preamp，段參數是 BAND 類） |
| loudness | 小音量時補低高頻（睡前最常用） | 輕 | **預設推薦** |
| bass / tone | 低音增強、音色微調 | 重 | 保留 |
| compressor + limiter | 夜間模式：壓縮動態，防突然大聲（鬧鐘、爆音） | 重／輕 | **預設推薦**，可做「夜間模式」一鍵 |
| crossfeed | 耳機長時間聆聽減少疲勞 | 輕 | 耳機模式建議 |
| gain / balance | 左右聲道不平衡、整體增益 | 輕 | 保留 |
| widener / velvet_widener | 立體聲加寬 | 輕／重 | 低優先，已有不拆 |
| tube / freeverb | 暖化、殘響 | 重 | 娛樂性，保留但不推薦；LDAC 與重型效果同時開要量 CPU |
行動：已做 `fx params` 並讓 `fx …` 輸出回到 BLE，網頁新增「音效細部調整」面板（依 `fx params` 自動產生開關與滑桿）。待做：一鍵「夜間模式」（loudness + compressor + limiter）。

## 主人裁定（2026-11-24）
- 參考 repo：round、nightstand、chipchill、tunetap、JukeChip(RP2350_player) 都可以參考。
- 晶片音樂系列晶片支援為 P1。Wi-Fi 與網路廣播都要支援。
- 藍牙堆疊與 BLE **常開**；A2DP 角色（接收／發送）互斥、可切換；沒有 LVGL，互斥的範圍可重新評估。
- **測試範圍（主人 2026-11-24）**：從 round 拿來的程式（解碼器、音效鏈、LDAC、電台工具等）已在 round 充分驗證（每項重複 10 次以上、LDAC 用真耳機測過），本專案不重測；也不做長時間測試。只對 Nano 自己寫的膠合程式做短的板上檢查。
- **方向（主人 2026-11-24 晚）**：
  - 藍牙輸出到耳機是必要功能。
  - 音效／濾波不是必要項目：希望支援，但忙碌時可以自動停用（例如 LDAC 編碼時停掉重型效果）。
  - 希望支援更多音訊格式；不做影片類。
  - 歌詞可以做，但負擔不要在裝置：整包 LRC 一次送到網頁，由網頁依播放位置自己對時。
  - OTA 不做，故障就重刷。
- **裁定（主人 2026-11-24 夜）**：
  - 不做：電台時光緩衝、耳機當按鍵、夜晚節目表、LED 時間燈、鬧鐘（盒子沒喇叭，鬧鐘可能沒人聽到；已從韌體與網頁移除）。保留定時淡出（睡眠計時）。
  - 要做：網頁搬進 tunetap 的電台搜尋；LED 要一直顯示狀態（多色、閃爍次數）。
  - 電台收藏存在 TF 卡（`radio.txt`），不存瀏覽器。
- **內部 RAM 非常小，能放 PSRAM 的一律放 PSRAM；雙核善用**（核心 0：藍牙／Wi-Fi／USB／網路；核心 1：解碼、編碼、頻譜）。

## 已知取捨與待決定
1. **A7 / B2–B5 共用藍牙堆疊**：A2DP 一次只能一種角色（實驗證實）。Bluedroid 與 BLE 常開，只切 A2DP 角色：接收（手機→Nano→小尾巴）或發送（Nano→耳機）。
2. **LDAC 的成本**：round 用修補過的 Bluedroid（`tools/bt_ldac/make_bt.sh`）與 libldac，編碼吃核心 1 約 45–60%。Nano 沒有畫面，預算較寬鬆，但要和解碼、頻譜、Wi-Fi 一起量。
3. **round 元件的共用方式**：目前以路徑引用 `../ESP32S31_round/components`。要吃進 `bt_svc*` 這類和 UI 纏在一起的程式，需要抽出無 UI 的版本，或複製後精簡。

## 2026-11-24 夜新增
- `st` 多了 `cpu:[核心0,核心1]`（兩次 `st` 之間的負載百分比）。實測：MP3 → 小尾巴約 8% / 14%。
- 格式：接上 ADX、VAG、BRSTM（沿用 round 後端，不重測）。
- 電台：網頁用 tunetap 的 radio-browser 搜尋（`web/radio/` 原樣複製 rb.js 等），HLS 標為不支援；收藏＝卡上 `radio.txt`，指令 `radio add 名稱|網址`、`radio del N`（BLE 唯一能改卡上檔案的路徑：固定檔案、只收 http(s)、長度與數量受限），板上實測通過。
- 續播（F3）：停止／睡眠計時結束／輸出消失時記下卡上檔案的位置（書籤存 NVS，由主任務寫入）；待機時 `resume` 從該處接著播，並以所在資料夾為播放清單；`st` 有 `bm`。板上實測：MP3 播到 4.8 秒停止、重開機後 `resume` 從原位置接續、播完後書籤清除。支援的格式是後端有續播權杖者（mp3 flac wav aiff mpc tta 等）；m4a／ogg／aac 沒有，會從頭播。續播後該曲的總長度顯示為未知。
- 斷音計數修正：換曲與跳曲的空檔不再算成斷音（先前測到的 467 次是這個原因，已確認：同樣操作現在為 0）。
- 標籤與 ReplayGain（F4）：播放卡上檔案時用 round 的 `pc_id3` 讀標題／演出者／專輯（ID3、FLAC、Ogg、MP4），`st` 回報 `title`、`artist`、`album`；`rg off|track|album` 依標籤套用 ReplayGain（設定存 NVS）。板上實測：FLAC（中文標籤、-6 dB）與 MP3 的標籤都讀到，ReplayGain 生效的日誌出現；音量差異未實聽。
- EQ 頻段：`fx params` 回報等化器 10 段（`APP fxb …`），用 `fx set equalizer b<N>_fc` 等設定，板上驗過。
- 小尾巴線控按鍵（主人 2026-11-25）：經小尾巴的 HID 介面讀取有線耳機按鍵。這顆 CX31993 回報音量加、音量減、播放／暫停、語音助理四種（報告 ID 2 的位元 0–3，已讀描述元確認）；支援到三顆鍵（中、加、減）。每顆鍵四種手勢（按一下、兩下、三下、長按）各可指定一個動作（無、播放暫停、下一首、上一首、音量加、音量減、停止），設定存 NVS；預設組 `keys preset 1|2|3` 對應單鍵、雙鍵、三鍵耳機；`keys set <c|p|m> <1|2|3|h> <動作>` 自訂；`keys` 會顯示最後偵測到的手勢，方便確認耳機送了什麼。沒有設定連按的鍵會在按下瞬間動作，有設定的要等 0.4 秒。手勢邏輯用 UART 的 `keys sim` 模擬驗過（單擊、雙擊、三擊、長按連續調音量）；HID 端點已在讀取、不影響播放；**真的按鍵沒按過**。
- **TF 卡格式化（主人 2026-11-25）**：只有在「有卡、但檔案系統真的掛載失敗」時才提供，而且要再三確認（三步、兩組不同的確認碼）後才能在 Web BLE 進行。能正常讀取的卡在任何介面都不能格式化。TF 卡測速照常提供。
- **原則（主人 2026-11-25）**：Nano 只是沒有螢幕；round 能支援、不靠螢幕的項目都該支援，便宜的優先。
- TF 卡測速（沿用 round 的 `card_test.c`）：`cardtest raw|file`、`cardtest`（結果與可支援格式的判定）、`card`（大小、剩餘、檔案系統、序號、狀態）；`st` 有 `card: ok|bad|none`。板上實測這張 16 GB 卡：原始讀取平均 7.95 MB/s、檔案寫入 6.08、讀回 7.76、驗證通過，七種格式全部足夠。
- TF 卡格式化：只在「有卡但掛載失敗」時提供，三步兩組確認碼；能讀取的卡一律拒絕（板上驗過）。確認流程用只有序列埠能開、且綁在記憶體磁碟上的模擬驗過；實際的 f_mkfs＋重新掛載＋重建目錄在記憶體磁碟上驗過（exFAT、FAT16）。**真的「讀不到的卡→格式化→可用」沒在硬體上跑過**，需要一張分割表被清掉的備用卡來驗。
- 經 BLE 設定 Wi-Fi（主人 2026-11-25）：`wifi set 名稱|密碼`（存 NVS、立刻重連、密碼不可讀回也不進日誌）、`wifi forget`、`wifi scan`（最多 20 個、強到弱）、`wifi` 回報狀態（connected／connecting／wrong password／not found／off／no credentials）與來源（nvs／card／none）。經網頁設定的優先於卡上的 `wifi.txt`。板上實測：密碼錯、找不到網路、忘記後回到卡上設定並重新對時、重開機後仍保留，都通過；「用正確的新密碼設定成功」這一條沒有第二個已知網路可測。BLE 沒有加密，密碼以明文經 BLE 傳送，這是開放控制的既定取捨。
- 網頁校時與時區：`time`、`time set <秒> [時區]`、`time tz <時區>`；NTP 已同步時只套用時區；時區存 NVS（預設 CST-8）。板上實測：沒 Wi-Fi 時由用戶端設定時間後夜間排程會動作；NTP 回來後以 NTP 為準。
- **控制介面的方向（主人 2026-11-25）**：
  - 音量是最常用、最緊急的操作，要始終看得到。
  - 網頁之後會有一個雙胞胎原生 App，所以以 App 的方式構思：不用手勢（網頁手勢不好做，PC 瀏覽器上也怪），可以不靠收折而改用圖示導覽。
  - 不要被「床頭」這個主題綁住：它也可能放在客廳或餐廳，介面與命名走一般性用途，不限制它的潛力。
- **網頁的建置與發布（主人 2026-11-25）**：控制網頁改用建置流程（原始碼分模組，建置輸出提交進 git），用 GitHub Pages 發布；這一批完成後開 GitHub repo 與 Pages，之後主人只用 GitHub Pages 驗證。
- **發布（主人 2026-11-25）**：repo 公開；授權在 MIT 與慣用的 LGPL 之間選；GitHub Pages 網址盡量短。round 的公開版是經匯出流程產生的 `JokerCatz/JukeDial`。
- **repo（主人 2026-11-25）**：名稱 `NightStandNano`（Pages：`https://jokercatz.github.io/NightStandNano/`）。這是一個獨立的 repo：把用到的 round 程式複製進來，不再以相對路徑引用同層的 round。
- **發布與網頁（主人 2026-11-25）**：燒機頁沿用 round 的工具，放同一個 repo 的另一個路徑（`…/NightStandNano/flash/`）；README、控制頁、燒機頁互相連結。Nano 要有自己的 release。用不到的 round 程式要精簡，兩邊各自維護、可互相參考。
- **硬體適應（主人 2026-11-25）**：TF 卡腳位與 RGB LED 的 GPIO 要能設定（有預設值、存在板上、要有安全提示）；**設定入口放在燒錄工具（經 UART），不放在 BLE 的控制頁**，以免誤操作，讓沒有內建 TF 卡座的板子（例如 S31 mini，自己焊 SD 卡套、外加 WS2812）也能用。另外留下擴充的插入點但不實作：螢幕初始化、GPIO 按鍵、I2S 輸出（角色同小尾巴，可能還需要 I2C 初始化）；需要的人自己改韌體。
- zip 內的音樂：`ls` 把 `.zip` 當資料夾走進去，`play <zip>` 或 `<zip>/<項目>` 播放；項目整個解到 PSRAM（上限 8 MB）；標籤、歌詞、續播在壓縮檔內都可用。板上實測 mp3 flac ogg opus m4a vgm。舊式編碼的檔名可播但顯示亂碼。
- 取樣率實測 `ratetest`：逐一實測小尾巴宣告的取樣率並記錄（依 USB 識別碼存 NVS），`dacmax auto|<Hz>` 設上限。這顆 CX31993 的 8 種取樣率（8–384 kHz）主機端全部通過；384 kHz 原生播放 0 underrun。只證明主機端資料送得出去，聲音正不正確沒有聽過。384→192 kHz 用現有重取樣器會大量斷音（待改用整數減半）。
- 曲尾修整：Ogg、Opus、M4A 依容器資訊修掉編碼器填充，三段接合後總長一格不差、沒有靜音段；原始 AAC（ADTS）沒有資訊可用，維持原樣。
- `backend_sdec.c` 修正：檔案結尾時解碼器裡還有未取出的封包會被丟掉（6 秒的 Ogg 只播 4.8 秒），已補上；此檔與 round 的版本不再相同。
- **主人裁定（2026-11-25）**：樂器模式不做；遊戲不做；燒機頁連線時機器會重新啟動，可以接受。HLS 電台待主人了解後決定。
- **授權與發布（主人 2026-11-25）**：授權 LGPL-2.1-or-later（與 JukeDial、tunetap 相同；`LICENSE`、`THIRD_PARTY_NOTICES.md`）；第一版 `v0.1.0`；repo 公開於 `github.com/JokerCatz/NightStandNano`；Pages：控制頁在根路徑、燒機頁在 `flash/`；repo 有自己的 release（`tools/release.py`、`tools/publish_pages.sh`、`tools/publish_release.sh`）。裝置可以放在任何地方，不限床頭，「NightStand Nano」只是名字。
- **Waveshare ESP32-S31-Zero（主人 2026-11-25）**：先當研究素材記錄在 `docs/research/waveshare-esp32-s31-zero.md`，暫緩；之後可能有更多硬體可選，到時再評估。它只有一個 Type-C 埠，主人認為有問題。
- **主人裁定（2026-11-25）**：公開提交的作者信箱用主人的 Gmail。HLS 電台：能支援就支援。repo 已公開（`JokerCatz/NightStandNano`），歷史從一筆初始提交開始。
- **HLS 電台（2026-11-25，主人「能支援就支援」）**：已做並在板子上驗證。`radio <網址>`（`.m3u8`、播放清單 content type、或內容以 `#EXTM3U` 開頭）播 HLS：直播（滑動視窗）與已結束的清單（從第一個分段開始，超過 16 個分段的清單分窗播放）；母清單選音訊版本（純音訊優先，其次 `#EXT-X-MEDIA` 音訊，最後才選含影像的；AC-3／E-AC-3 等不選；在 200 kbit/s 以下取最高，否則最低）；分段為 TS（AAC 或 MP3）或 packed AAC／MP3（可帶 ID3 標籤）。fMP4（`#EXT-X-MAP`／ftyp）→ `HLS: fMP4 not supported`；`#EXT-X-KEY` 非 NONE → `HLS: encrypted`；AC-3 或 TS 內沒有 AAC／MP3 → `HLS: unsupported audio`；拒絕的原因留在 `st` 的 `rstate` 直到下一次播放。實作與 JukeDial 的差異見 `docs/design.md`「HLS radio」。
  - 驗證（真實公開串流）：https TS／AAC 直播 FIP（母清單三個版本，選 185 kbit/s）、France Info、Radio Italia、BBC Radio One（母清單，HE-AAC v1）、RTL（https 母清單，經 302 轉址，800 字元的簽章網址，HE-AAC v2，48 kHz 輸出）；http TS／AAC：BBC 6 Music、AIR（24 秒分段）；packed AAC（ID3 開頭）：NHK World、TRT、Rádio Comercial（單版本母清單）、RTL 102.5（母清單）、Apple bipbop 純音訊 VOD（180 段，播過第 16 段的視窗邊界）；拒絕：Apple fMP4 母清單（走音訊群組 → `HLS: fMP4 not supported`）、longtail AES-128 → `HLS: encrypted`；區網伺服器上的假 SAMPLE-AES 與 E-AC-3 母清單、帶 `#EXTINF` 歌名的 VOD（歌名依分段進入環形緩衝的時間更新，約比聲音早一個緩衝長度）、沒有副檔名也沒有 m3u8 content type 的清單（用內容判斷）。
  - 3 分鐘以上、斷音 0：FIP（20 分鐘）、Radio Italia、BBC 6 Music、Apple VOD。從指令到出聲：ICY 電台 0.7–1.9 秒；HLS 約 1–3 秒（RTL 的來源伺服器慢，約 15 秒），偶爾 6–9 秒（DNS／TLS 較慢）。`stop`：播放中 0.1–0.4 秒、連到不回應的位址 ≤3 秒（連線以 3 秒為單位重試，其間檢查取消；ICY 路徑的首次連線也一併改成這樣，原本最長 8 秒）。HLS／ICY 電台／卡上檔案來回切換多次正常。`wifi off` → 約 16 秒後 DNS 失敗，狀態 `reconnecting`／`no connection`，`wifi on` 後約 4 秒恢復播放；整個區域網路 DNS 斷線約 70 秒也自動恢復。
  - 連線重用：同一個 `esp_http_client` handle 的 `set_url` 在同主機時保留連線，https 串流 20 分鐘只做 1 次 TLS 握手（關掉重用：約每分鐘 20 次）。
  - Auracast＋https HLS＋BLE 頻譜可以同時用（內部 RAM 59.6 KB、最大區塊 31 KB、斷音 0）；數字在 `docs/measurements.md`。
  - 未驗證：`https: low memory` 的拒絕路徑（找不到安全造出該記憶體狀況的方法）；AC-3 的 TS 分段（只用假清單驗證到 `HLS: unsupported audio` 的清單層級，沒有真的 AC-3 串流）；聲音聽感（沒有麥克風，只看速率、`pos` 前進、斷音計數）；SAMPLE-AES 真串流；時間化 ID3（TS 裡的歌名）沒有擷取（JukeDial 也沒有），只用 `#EXTINF` 歌名；裸 `.ts` 串流（無清單）會回 `HLS: bare TS stream not supported`。

## 2026-11-25 晶片／tracker 格式實測（`nsn_chip/<格式>/`，板上，輸出端檢查 `dbg oc`）
方法：每個檔播約 6 秒，記下解碼器名稱、速率、長度、曲目數、`title`/`artist`、`pos` 是否前進、`und`、輸出檢查的 `maxpeak`（0 = 數位靜音）與 `starved`；多曲檔再 `next` 兩次、`prev` 一次。測試檔是私人收藏（`~/self/ESP32S31_round/.local/chiplib/free/`），不進 git，測完已從卡上清掉，只各留一個小檔給主人試聽。解碼器本身不重測（主人裁定），只看 Nano 的整合：副檔名對應、載入、曲目、標籤、zip、狀態。
| 格式 | 試的檔案 | 結果 |
|---|---|---|
| nsf | japan（VRC6）、BoBombBattlefield（FDS）、radiozonde celestial elegy（MMC5）、energy breaker（N163）、bass samples king（VRC7）、t to s（VRC6+VRC7+MMC5+N163+5B）、3-n163_vrc6（N163+VRC6）、Danish Mega Pony（12 曲）、lan master（4 曲） | 全部出聲（maxpeak 1.5 k–13.8 k）、斷音 0、`pos` 前進；沒有長度資料 → 150 s；`next`／`prev` 在檔內換曲（2/12、3/12…）；title／artist 取自檔頭（曲名或遊戲名、作者） |
| nsfe | 只有合成檔：`tools/hosttest/mk_nsfe.py` 由 NSF 轉成（含 time／auth／tlbl 區塊），lan_short（4 曲）、pony_short（12 曲） | 出聲；長度取自 time 區塊（3 s + GME 的 8 s 淡出）；曲名來自 tlbl（`Track 2`…），artist 來自 auth。**沒有真的 NSFE 檔** |
| gbs | cpl gbc intro 6、knark、mental respirator（3 曲） | 出聲、斷音 0；3 曲換曲正確；title／artist 有（`Mental Respirator`／`Goto80, Mr. Death`） |
| hes | 1941 counter attack (sgx)、ropm 02 | 出聲、斷音 0。**曲目數一律 256**（HES 檔頭沒有曲數，GME 取 256），`next` 會走到空的曲號；沒有標籤。1941 的前兩個曲號是空的，約 3 秒內自動跳過 |
| kss | 2001 a space odyssey、ind revolution、nemesis gametune | 出聲（MSX-MUSIC 的 YM2413 預設開，見 CPU 表）、斷音 0；曲目數 256（同 HES）；沒有標籤。`next` 造成的斷音計數已修（見下） |
| sap | ABBUC JHV 2002 2（3 曲）、Electra（4 曲）、Pixel | 前兩個出聲，每曲長度取自 TIME 行（53.9 / 61.6 / 2.4 s）；title／artist 有。**Pixel 是 TYPE D（GME 的「Digimusic」），GME 不支援**：不播，日誌與 `events` 寫 `Digimusic not supported`（資料庫 77 個 SAP 裡 10 個是 TYPE D） |
| sid | 15 Years Oxyron（2 曲）、Santas Little Helper（3 曲）、Crank Powered Async Loader（2SID）、Light Years（3SID）、Stereophonik（RSID，2SID） | 前四個出聲；固定 180 s（沒有 Songlengths）；title／artist／album（= released）取自檔頭。**Stereophonik 是靜音**（maxpeak 0）：PC 上用同一份 cRSID 原始碼（`components/libcrsid`）渲染 30 秒峰值也只有 2，是解碼器不模擬那首 RSID 靠的真 C64 行為，不是 Nano 的問題 |
| ay | interchrome（2 曲）、acoustic dreams（10 曲） | 出聲；每曲各有曲名（`Agent-X`、`H-Core`…）、作者；`next`／`prev` 正確 |
| ym | lap2、ooh yeah、decade demo - boot（都是 LHA 壓縮的 YM） | 出聲，長度已知（46 / 100 / 261 s）；沒有標籤（檔頭被壓縮，Nano 沒讀） |
| ahx / hvl | vhs-binge、virgin-style / postmortem1、waiting for a message | 出聲，長度 23–77 s，title 有 |
| vgz（SN76489） | 12 bar blues、hitting enter、stormy changes、short motif、lizards forest、boss ix（資料庫 20 個 vgz 都過了主機端標頭檢查，見下） | 都出聲、斷音 0；GD3 標籤（曲名、作者）有；與 zip 內的 vgz 一樣 |
| mod / xm / s3m / it | random voice monday、giel / jt strng、radix unreal superhero / overdriv、ZAK / 1 channel moog、beek substitutionology | 都出聲（`MOD` 解碼器 = libxmp）、斷音 0；title 取自檔頭（Nano 自己讀，見下） |
| rad / hsc / amd | backlash、christmas song / hardc1 / cosmos-theme | 出聲（`OPL` = AdPlug），長度已知；沒有標籤 |
| d00 / sa2 | ergo、jungle / fairy tale | **原本打不開**：這些是 pc_chipseq「實驗性」層，Nano 從沒註冊開關所以永遠被拒絕。現在預設開（`expfmt on|off`），三個檔都出聲 |
| a2m | adventure（v5）、drgwrrtt（v13）、step1856（v12）；15 minute compo trial（v11）、ballad（v9） | v5、v12、v13 出聲；**v9–v11 拒絕**（AdPlug 2.4 不支援 aPlib 壓縮；資料庫 15 個中 8 個是 v9–11） |
| zip | `pack.zip` 內 nsf、gbs（子資料夾）、sid、vgz、mod；整包當播放清單 | 單檔、整包都正常：標籤、曲目、順播（`gapless:` 行）、`ls pack.zip`、`ls pack.zip/sub` |
| 沒有測試檔 | spc、gym、p8（含 p8.png）、真的 nsfe；tracker 副檔名 stm okt med ptm far 669 mtm ult amf dbm gdm … 與 AdLib 副檔名 imf wlf a2t cmf adl ksm mdi sci dro lds rol … | 只有接線（`backend_for` 與 pc_chipseq 表）；沒有檔案，沒測 |
| 其它 | resume 書籤、shuffle、順播 | 見下 |

**整合行為（板上）**
- 續播：以前停在晶片檔上不會記書籤（沒有位元組位置），`resume` 會去播很久以前的舊書籤（實際發生：播到無關的 mp3）。現在晶片檔、SID、tracker 都會記書籤：GME 與 SID 記**曲號**，tracker／AdLib／YM／P8 記檔案（從頭播）；NSF 第 3 曲停止 → `resume` 回到第 3 曲，SID 第 2 曲、zip 內的 SID、mod 都驗過。
- 順播（gapless）：同速率的晶片檔之間接續正常（`gapless:` 行，打開約 170 ms、環形緩衝還剩 380 ms），`dbg oc` 接點列出、接點處沒有額外斷音。
- shuffle：混合資料夾（15 個檔、10 種格式）`next` 14 次 + 自然結束，沒有當機、斷音 0；曲目多的檔先走完檔內曲目再換檔。
- 修掉的整合錯誤：(1) 實驗性層永遠被拒（d00 a2m sa2 …）；(2) 晶片檔沒有 title／artist；(3) `resume` 對晶片檔沒用；(4) 檔內 `next` 的重新填充被算成斷音（`und` 暴增）；(5) 順播接到打不開的檔時，同一個檔被打開兩次；(6) `.vgm` 其實是 gzip、`.vgz` 其實沒壓縮的檔打不開；(7) 打不開的原因只寫 `decoder open failed`，現在 GME 的理由也寫進日誌與 `events`。

**VGM：GME 靜音播放不支援的晶片（2026-11-25 修）**
- 驗證（舊韌體）：YM2151 的 VGM → `play: ESP_OK`、`Chip 44100 Hz`、`maxpeak=0`（數位靜音）；SN76489 + YM2151 → 只播 PSG 那一半，沒有任何警告。
- 現在播檔前先看 VGM 標頭（`components/player_core/src/pc_vgmhdr.c`）：GME 會播的是 SN76489（含雙晶片、T6W28）、YM2612、YM2413（YM2612 與 YM2413 同時存在時 GME 只播 YM2612，所以這對組合算 YM2413 不支援）。含其他晶片、且指令串流真的寫了它（宣告了時脈但從沒寫過的不算）→ 拒絕。
- 單檔 `play`：`play: ESP_ERR_NOT_SUPPORTED unsupported chip YM2151, OKIM6295`（第一個字仍是 esp 錯誤名，舊版網頁顯示「不支援這種格式」）；原本在播的東西不受影響。資料夾、播放清單、zip 內：該檔被跳過，日誌 `skip <路徑>: chip YM2151`，`events` 多一行 `skip <檔名>: chip YM2151`；全部都被拒絕時照舊 `nothing playable`。

| 測試檔（`tools/hosttest/gen_vgm_cases.py` 產生，27 個、共約 6 KB） | 結果 |
|---|---|
| ok_psg_v171 / v150_garbage（v1.50，0x40 之後是亂碼）/ v150_hdr40 / v101 / dual_t6w28 / plain_named.vgz（沒壓縮）/ gz_named.vgm（有壓縮）/ v171.vgz | 全部出聲（maxpeak 2415、T6W28 雙晶片 1482） |
| ok_declared_unused（宣告 YM2151、OKIM6295，只寫 PSG）、ok_ym2413、ok_ym2612 | 出聲（431 / 6928） |
| refuse_YM2151、YM2203、SN76489_YM2151、YM2151_OKIM6295、v100_shared_clock_YM2151（v1.00 共用時脈，由指令判斷）、YM2612_YM2413、YM2608_YM2610B、YM2151.vgz | 都拒絕，晶片名如檔名 |
| bad_header_only / 10_bytes / empty / not_vgm / cut_in_command / not_gzip.vgz / gzip_cut.vgz / header_0x100_only | 不當機；`decoder open failed`（附 GME 的理由）或播空曲 |
- **FM 晶片後端（2026-11-25，階段 1；主人指示：先在 Nano 做、JukeDial 之後抄回，所以是自成一體的元件；「支援一切，兩顆核心都用，只排除真的播不了的，或在播不了的情況下跳過」）**：`components/fmvgm`（ymfm，BSD-3，三個本地補丁），設計與數字在 `docs/design.md`「FM-chip backend」與 `docs/measurements.md`。做了：YM2151（OPM）與 YM2203（OPN + SSG）的 VGM／VGZ（單或雙晶片、迴圈次數＋8 秒淡出、GD3 標籤、資料區塊安全略過）與 S98 v0–v3（裝置表、同步／等待／迴圈、標籤；寫到其他裝置時以名稱拒絕）；`.vgm` 由標頭分流（GME 的晶片給 GME、FM 晶片給新後端、混合或沒人會播的晶片拒絕並說出晶片名）；晶片合成可放在核心 0／核心 1 的常駐工作者或解碼任務裡（`dbg fm core auto|0|1|inline`，執行中可換）；晶片原生率（55.9 / 55.5 kHz）在生產者內以整數多相濾波器轉成輸出端的取樣率，播放器的 pc_rsq 不再參與；靜態預檢（`fmglue.c` 的表）與動態規則（過載就停這首、換下一首、`skip <檔名>: too heavy for <輸出>`）；ymfm 的三個補丁（釋放後的運算元略過、LFO 相位步進快取、配置掛鉤）逐位元不變，YM2151 實測省 24–26 %（無聲晶片 −64 %）。已驗證：PC 上輸出與研究的參考基準逐位元相同、板子上同一組校驗和（五個檔、核心 0／核心 1）、壞檔與截斷檔（ASan／UBSan／TSan）、GME 檔與拒絕行為不變、標籤、zip、順播、`next`／`prev`、續播（定義為從頭播）、停止延遲約 50 ms、`stress.py leak 3`、日誌沒有 Guru／assert／abort。**沒有驗證：沒有任何真的 YM2151／YM2203 曲子，所以音色與音高正不正確沒驗過，只驗了與 ymfm 在 PC 上逐位元相同與時間（CPU）；聽感（迴圈接點、淡出、濾波器的 −1 dB／20 kHz 衰減）沒人耳驗過；LDAC 輸出的成本是估計（沒有 LDAC 接收端）；雙晶片檔在 DAC 上也超過一顆核心，目前被靜態預檢拒絕（下一階段：兩顆晶片分到兩顆核心）；實際曲子比合成流輕 0.4–0.7 倍是用 Mega Drive 真檔外推的，沒有驗過。** 不做（下一階段）：YM2608／YM2610（ADPCM）、取樣晶片（OKI6295 …）、部分播放策略、Shift-JIS 的 S98 標籤（顯示為 `?`）。
