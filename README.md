# NightStand Nano

**[控制頁 Control page](https://jokercatz.github.io/NightStandNano/)** · **[燒錄頁 Flasher](https://jokercatz.github.io/NightStandNano/flash/)** · **[最新版 Latest release](https://github.com/JokerCatz/NightStandNano/releases/latest)**

無螢幕的 ESP32-S31 音樂盒：TF 卡、網路電台或手機串流進來；USB-C DAC、藍牙耳機（含 LDAC）或 Auracast 廣播出去；用藍牙從網頁遙控；只有一顆狀態 LED。
可以放在床頭，也可以放在客廳或書桌；「NightStand」只是名字。

## English summary

A headless audio box on an ESP32-S31 board. **In:** TF card (FAT32 / exFAT, many formats including chip music), internet radio, a phone streaming over Bluetooth (A2DP).
**Out:** a USB-C DAC dongle, Bluetooth headphones (LDAC / AAC / SBC) or an Auracast broadcast. **Control:** a web page over Bluetooth LE (Web Bluetooth), no app, no screen;
one RGB status LED. Firmware (ESP-IDF v6.1 preview), the control page (`web/`) and a browser flasher with hardware setup (`flash/`) live in this repository;
licence LGPL-2.1-or-later. To try it: download a [release](https://github.com/JokerCatz/NightStandNano/releases/latest), open the
[flasher](https://jokercatz.github.io/NightStandNano/flash/) in desktop Chrome or Edge, flash, then open the [control page](https://jokercatz.github.io/NightStandNano/).
Everything below is in Traditional Chinese (the project owner's language); the command reference (`docs/commands.md`) and the code comments are mostly bilingual or English.
Honest status: the pieces that came from the author's other device (JukeDial) are well tested there; Nano's own glue was checked on one board only, and several things have
never run on real hardware (listed under "還沒實測過" below).

## 硬體

不需要照片或特別的外殼，這些就夠了：

- 一塊 **ESP32-S31 開發板**，要有 **USB 主機埠**（接 DAC）和 **UART 橋接埠**（燒錄與設定用）。16 MB flash、八線 PSRAM（`app/sdkconfig.defaults` 的設定）。
- 一個 **TF 卡座**：板上內建的，或自己焊的（自己焊的要用 SPI 模式，見下）。卡用 FAT32 或 exFAT。
- 一個 **USB-C DAC 小尾巴**（UAC2）。測過的是 Synaptics CX31993；其他 UAC2 裝置理論上可用，沒有逐一測過。
- 選用：一顆 **WS2812** 單顆 RGB LED（預設 GPIO60，腳位與顏色順序可在燒錄頁改）。沒有 LED 也能用。

## 第一次使用

1. 到 [Releases](https://github.com/JokerCatz/NightStandNano/releases/latest) 下載 `NightStandNano-<版本>-full.bin`（一個檔案就夠；也可以下載另外三個檔，見燒錄頁說明）。
2. 用**桌面版 Chrome 或 Edge** 開[燒錄頁](https://jokercatz.github.io/NightStandNano/flash/)，板子的 UART 橋接埠接電腦，選檔案、燒錄。燒錄頁預設保留板上設定（Wi-Fi、配對）。
   燒錄頁也能設定 TF 卡腳位與 LED 腳位（只能經 UART，不能經藍牙）。
3. 用 Chrome 或 Edge（桌面或 Android）開[控制頁](https://jokercatz.github.io/NightStandNano/)，按連線，選 `NightStand Nano`。
4. 設定 Wi-Fi（電台與校時用）：在控制頁的「設定 › Wi-Fi」輸入，或在卡上放 `/sdcard/_player/config/wifi.txt`（兩行：`ssid=名稱`、`pass=密碼`）。
5. 把音樂放進 TF 卡（任何資料夾），插進盒子，在控制頁的「音樂」頁選資料夾播放。

也可以不用燒錄頁：`python -m esptool --chip esp32s31 -b 460800 --before default-reset --after hard-reset --no-stub write-flash --flash-mode dio --flash-size 16MB --flash-freq 80m 0x0 NightStandNano-<版本>-full.bin`
（會連設定區一起清掉）。每個 release 的說明檔有完整指令與 SHA-256。

## 能做的

**來源**
- TF 卡（FAT32、exFAT）：mp3 flac wav aiff m4a aac ogg opus mpc tta adx vag brstm，整個資料夾連續播放，`.zip` 當資料夾。
- 晶片音樂：GME（nsf gbs spc vgm vgz …）、tracker、sid、opl、ym、ahx、PICO-8。
- 網路電台：http／https，MP3／AAC／Ogg，ICY 歌名，`.pls`／`.m3u`，HLS（`.m3u8`：TS／AAC 與 packed AAC／MP3 分段，直播與已結束的清單，母清單自動選音訊版本；fMP4（CMAF）、AC-3／E-AC-3、加密（AES-128／SAMPLE-AES）的串流會被拒絕，狀態欄寫明原因）。控制頁可以搜尋電台（radio-browser），收藏存在卡上的 `radio.txt`。
- 手機藍牙串流進來，播到小尾巴（喇叭模式，SBC／AAC）。
- 白噪音 8 種；場景（最多 3 層環境音循環混音；場景檔要自己用 `tools/make_scenes.py` 從自己的素材庫產生，repo 不附）。

**輸出**
- USB-C DAC「小尾巴」（自寫 UAC2 主機，8–384 kHz）。
- 藍牙耳機（耳機模式，LDAC／AAC／SBC），記住上次的裝置並自動重連。
- A2DP 一次只能一種角色：接收（手機→盒子）或發送（盒子→耳機），用 `bt mode` 切換；手機→盒子→耳機的中繼做不到。
- Auracast 廣播（`out cast`）：LE Audio 廣播，名稱「NightStand Nano」，LC3 48 kHz 立體聲，任意數量的 Auracast 耳機可以收聽。與藍牙耳機輸出互斥；Wi-Fi 與 BLE 控制可同時開。
  第一次啟動時 BLE 控制會斷線約 0.3 秒（網頁自動重連）；LE Audio 啟動後要重開機才會釋放記憶體。

**播放**
- 播放方式：資料夾循環、隨機、單曲重複、播完停止；無縫播放。
- 續播：停止時記下位置（存 NVS，重開機仍在），待機時 `resume` 接著播。
- 睡眠計時（最後 30 秒淡出）；`bedtime`：噪音或場景＋計時＋夜間模式；夜間模式可設每日時段。
- 音效鏈（十多個：等化器、響度、低音、壓縮、限幅、crossfeed …）。屬於可選功能：LDAC 或 Auracast 時重型效果自動暫停；其他情況下核心 1 連續過載 2 秒也會自動暫停。
- 標籤（標題／演出者／專輯）與 ReplayGain；歌詞（同名 `.lrc`，由網頁對時）。
- 每種輸出各自記音量，音量上限，輕柔起音。

**控制**
- 只經藍牙 LE（GATT）：Web Bluetooth 控制頁（`web/`），以及 UART 序列埠（開發與設定用）。另外兩種實體操作：藍牙耳機的按鍵（AVRCP），以及插在小尾巴上的有線耳機線控按鍵（最多三顆鍵、每顆四種手勢、動作可自訂）。Wi-Fi 只用於電台與校時，不做控制。
- 控制頁：英文／繁中／簡中、跟隨系統亮暗、手機與桌面、斷線自動重連；視覺化（頻譜、瀑布、夜行、錄音帶，來自 [tunetap](https://github.com/JokerCatz/tunetap)）。
  網址加 `?demo` 可以在沒有裝置時預覽。
- 指令清單：[`docs/commands.md`](docs/commands.md)。

**其他**
- 當機記錄寫到卡上 `_player/logs/crash.txt`；連續 3 次開機後 60 秒內當機會進安全模式。
- TF 卡腳位與 LED 腳位可設定（存板上 NVS，不用重編）。ESP32-S31 的 SDMMC 只能用固定腳位；自己焊的卡座請用 SPI 模式（`hw sd spi …`）。見 [`docs/porting.md`](docs/porting.md)。
- TF 卡測速；卡片有回應但檔案系統讀不到時，可以格式化（三步、兩組確認碼；讀得到的卡任何介面都不能格式化）。
- 電台不支援：HLS 的 fMP4／CMAF 分段、AC-3／E-AC-3、加密串流、裸 TS 串流（狀態文字 `HLS: fMP4 not supported`、`HLS: encrypted`、`HLS: unsupported audio`…）。
- 不做的（已裁定）：鬧鐘、OTA（故障就重刷）、波形與相位視覺化、手機→盒子→耳機中繼、Auracast 收聽（只做廣播出去）、影片、樂器模式、遊戲。

## 還沒實測過（請當成未驗證）

先講清楚，下面這些都沒有在真實條件下驗證過：

- 解碼器、音效鏈、LDAC 編碼、Auracast 廣播來源等「從 JukeDial 複製來的程式」，在 JukeDial 上充分驗證過；Nano 只對自己寫的膠合程式做了短的板上檢查，**不做長時間測試**。
- **控制頁**：功能對著假裝置（`?demo`）和電腦上的 BLE 測試程式驗過；**真手機、真瀏覽器的 Web Bluetooth 連線尚未實測**。
- **燒錄頁**：頁面邏輯與單元測試通過，但**真的把韌體燒進板子的流程（Web Serial）尚未實測**。
- **藍牙耳機輸出**：A2DP（AAC）只對另一塊 JukeDial 當對端驗證過；SBC 與 **LDAC 沒有對真耳機測過**（LDAC 只有離線編碼檢查，JukeDial 上的 LDAC 用真耳機測過）。耳機按鍵（AVRCP）、線控按鍵的**真實按鍵沒按過**。
- **Auracast**：廣播由 JukeDial 當接收端收過；**沒有對真正的 Auracast 耳機或接收器測過**。
- **exFAT**：只在記憶體磁碟上驗過，**沒有用真的 exFAT 卡測過**。**SPI 模式的 TF 卡沒有實體測過**。**格式化「讀不到的卡」**的確認流程與格式化在模擬環境驗過，真的「讀不到→格式化→可用」沒在硬體上跑過。
- **USB DAC**：只有一種 DAC（CX31993）；聲音正不正確沒有用耳朵聽過（只確認資料送得出去、沒有斷音）。
- 手機串流進來（喇叭模式）：以 JukeDial 當手機驗證，**真手機沒測過**。
- 用正確的新密碼經控制頁設定 Wi-Fi 成功的路徑沒有第二個已知網路可測（密碼錯、找不到網路、忘記等路徑通過）。
- 只有 ESP-IDF v6.1 preview、一塊 ESP32-S31 開發板、一張 16 GB FAT32 卡實測過。

逐項狀態見 [`docs/requirements.md`](docs/requirements.md)。

## LED 燈號（預設 GPIO60，單顆 RGB）

- **顏色＝在做什麼**：綠 播放到小尾巴／青 播放到藍牙耳機／紫 手機串流進來／琥珀 暫停／藍 待機。
- **動態＝怎麼做**：恆亮 播放中／快速脈動 電台連線或緩衝／慢呼吸 睡眠計時進行中（最後 30 秒隨聲音淡出）／藍色快閃 搜尋藍牙裝置。
- **紅燈每 4 秒閃幾下＝缺什麼**：1 沒有輸出／2 沒有 TF 卡／3 電台連不上／4 安全模式。播放中有故障時只插一下短紅光。
- 亮度 `led 0-255`（0 關閉）；夜間模式會把亮度上限壓低。

## 瀏覽器支援

| 功能 | 需要 | 可用的瀏覽器 |
|---|---|---|
| 控制頁（Web Bluetooth） | `https://` 或 `localhost`，藍牙 | 桌面與 Android 的 Chrome、Edge。**不支援 Safari、iOS 上的任何瀏覽器**（Firefox 也沒有 Web Bluetooth） |
| 燒錄頁（Web Serial） | USB 序列埠 | 只有桌面版 Chromium 系（Chrome、Edge）。手機不行 |

## 安全（白話版）

- **藍牙控制是開放的，這是設計**：在藍牙範圍內的任何人都可以連上盒子、換歌、調音量、開關 Wi-Fi 或 Auracast。沒有配對，也沒有加密。
- **從控制頁設定 Wi-Fi 時，密碼是以明文經藍牙傳送**（BLE 沒有加密）。在不信任的場合請改用卡上的 `wifi.txt`。密碼存進板上後不能讀回，也不會寫進日誌。
- **UART 專用規則**：寫檔、刪檔、建資料夾、重新啟動、改腳位、當機測試，只有接著序列線才做得到，藍牙上一律拒絕。唯一的例外是電台收藏：`radio add`／`radio del` 只能改固定的 `radio.txt`，只收 http(s) 網址，長度與數量有限。
  這條規則的目的：藍牙範圍內的人無法刪你的檔案或把板子的腳位改壞。
- 卡片格式化只在「卡片有回應但檔案系統讀不到」時才提供，要三步與兩組確認碼；讀得到的卡永遠不會被格式化。
- 控制頁與燒錄頁是純靜態頁面，沒有帳號、沒有伺服器；燒錄頁選的檔案不會離開瀏覽器。控制頁的電台搜尋會向 radio-browser 的公開伺服器查詢。

## 從原始碼建置

需要 ESP-IDF **v6.1（preview）**，目標 `esp32s31`，以及 git、Python 3、bash。完全在本 repo 內建置，沒有同層的相依 checkout。

```
source ~/.espressif/tools/activate_idf_v6.1.sh      # 或你安裝 IDF v6.1 的 export 腳本
cd app
idf.py --preview set-target esp32s31
idf.py --preview build
idf.py --preview -p <序列埠> flash
```

- 建置時會自動執行兩個產生器：`tools/fatfs_exfat/make_fatfs.sh`（打開 exFAT 的 FatFs 副本，`.local/fatfs_exfat`）與 `tools/bt_ldac/make_bt.sh`（打過補丁的 Bluedroid，LDAC 端點，`.local/bt_ldac`，約 100 MB，不進版控）。IDF 升級後補丁對不上會直接讓設定階段失敗。
- 元件管理員會下載 `espressif/usb`、`espressif/led_strip`、`espressif/esp_audio_codec`（版本鎖在 `app/dependencies.lock`）。
- 建置可重現（`CONFIG_APP_REPRODUCIBLE_BUILD`，原始碼路徑以 `/NSN` 取代）：相同原始碼、相同 IDF、相同 `-DPROJECT_VER`，得到相同的 `.bin`。版本字串：`-DPROJECT_VER`，否則 `git describe --tags --dirty`，否則 `version.txt`＋commit。
- 網頁：`cd web && npm ci && npm run build && npm test && npm run check`，燒錄頁同理（`cd flash`）。`dist/` 是提交進 git 的建置結果，GitHub Pages 直接發布它。細節見 `web/README.md`、`flash/README.md`。
- 測試板輔助腳本：`NSN_PORT=/dev/ttyUSB0 tools/board.sh flash`、`tools/board.sh log 8 "st"`（重開機、送指令、讀 8 秒輸出）。沒有設 `NSN_PORT` 時，只有一個序列裝置才會自動使用，否則停下來要你指定。
- 發布：`tools/release.py <版本>`（要乾淨的工作目錄與指在 HEAD 的 tag `v<版本>`；`--dry-run` 不需要）會做乾淨建置、合併映像、SHA256SUMS、發布說明、網頁檢查、組出 Pages 網站並做隱私掃描，輸出在 `release/`。
  `tools/publish_pages.sh` 與 `tools/publish_release.sh <版本>` 負責推送（都有 `--dry-run`）。

## 檔案地圖

| 路徑 | 內容 |
|---|---|
| `app/` | 韌體（ESP-IDF 專案；`main/` 是 Nano 自己的程式，`partitions.csv`、`sdkconfig.defaults`） |
| `components/` | 解碼器與 `player_core`（從 JukeDial 複製並精簡，來源與授權見 `components/ORIGIN.md`） |
| `web/` | 控制頁（Web Bluetooth），建置到 `web/dist/` |
| `flash/` | 燒錄頁與硬體設定頁（Web Serial），建置到 `flash/dist/` |
| `tools/` | 測試板腳本、BLE 測試程式、場景產生器、`release.py`、`publish_*.sh`、修補 IDF 的產生器（`bt_ldac`、`fatfs_exfat`） |
| `docs/` | 設計與開發紀錄 |
| `LICENSE`、`THIRD_PARTY_NOTICES.md`、`docs/licenses/` | 授權 |

## 文件

- [`docs/commands.md`](docs/commands.md)：UART／BLE 指令參考（哪些只有 UART）。
- [`docs/design.md`](docs/design.md)：設計與架構（前半是開發日誌，最後幾節是現況）。
- [`docs/porting.md`](docs/porting.md)：換板子：TF 卡腳位、LED、擴充點。
- [`docs/measurements.md`](docs/measurements.md)：CPU、記憶體、洩漏的量測。
- [`docs/requirements.md`](docs/requirements.md)：需求表與主人的裁定，是開發日誌；逐項狀態以它為準。
- [`docs/exfat.md`](docs/exfat.md)、[`docs/ideas.md`](docs/ideas.md)、[`docs/backlog.md`](docs/backlog.md)：exFAT 做法、提案、候選功能總表。

## 來源與授權

- 授權：**LGPL-2.1-or-later**（見 `LICENSE`）。每個自己寫的原始檔開頭有 `SPDX-License-Identifier`。
- `components/` 的解碼器與 `player_core` 是 2026-11-25 從作者的另一個專案 [JukeDial](https://github.com/JokerCatz/JukeDial)（commit `6fdce40`）複製來並精簡的，之後各自維護。控制頁的視覺化與電台搜尋來自 [tunetap](https://github.com/JokerCatz/tunetap)。
- 內含與下載的第三方元件及其授權（包含 LDAC 的認證提示、`esp_audio_codec` 的授權條款）：[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)。**場景素材庫不在 repo 裡。**

## 狀態與路線

目前是第一版 `v0.1.0`（`version.txt`）。功能大致到位；上面「還沒實測過」是接下來最需要的驗證。候選功能與評估在 [`docs/backlog.md`](docs/backlog.md)；已裁定不做的在上面與 `docs/requirements.md`。
