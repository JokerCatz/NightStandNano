# 移植到其他板子與擴充點

NightStand Nano 的韌體預設跑在 ESP32-S31 開發板上（TF 卡座接 SDMMC 槽 0、GPIO60 一顆 WS2812、USB-C 接 DAC 小尾巴）。這份文件說明：不改程式碼就能改的硬體設定、哪些腳位不能用（和依據）、TF 卡在這顆晶片上的實際限制，以及三個「只留插入點、沒有實作」的擴充點。

> **先講清楚**：顯示器、GPIO 按鍵、I2S 輸出**都沒有實作，也沒有在任何硬體上測過**。`board/` 底下留的是空的函式和一份編得過的範本，要用的人自己改韌體。自訂 TF 卡腳位（SPI 模式）與外接 LED 在這個專案的測試板上也**沒有電氣上驗證過**（測試板的接線固定，只驗了設定、拒絕規則、存檔、重開機後的行為和「接不到卡」時盒子仍可用）。

## 1. 不改程式碼：`hw` 設定

TF 卡腳位與 LED 腳位在初始化時就要用，所以存在板上的 NVS（不在卡上）。**設定入口是燒錄工具（UART 序列埠），BLE 控制頁只能讀**，以免誤操作：BLE 上 `hw` 可以讀，任何帶參數的 `hw`（和 `sdpins` 的設定）都回 `?`。設定在開機時讀一次，改完要重開機才生效。

| 指令 | 誰能用 | 回覆 |
|---|---|---|
| `hw` | 所有人（含 BLE） | 見下 |
| `hw sd <clk> <cmd> <d0> <d1> <d2> <d3> <width>` | 只有 UART | `hw sd: ok, reboot to apply` 或 `hw sd: refused <原因>` |
| `hw sd spi <clk> <mosi> <miso> <cs>` | 只有 UART | 同上 |
| `hw led <gpio> [rgb\|grb\|brg\|rbg\|gbr\|bgr]` | 只有 UART | `hw led: ok, reboot to apply` 或 `hw led: refused <原因>` |
| `hw led off` | 只有 UART | 同上（不啟動 LED 工作，腳位不動） |
| `hw reset` | 只有 UART | `hw reset: ok, reboot to apply`（全部回預設） |
| `sdpins`、`sdpins a b c d e f w`、`sdpins reset` | 讀：所有人；設定：UART | 舊指令，仍可用，規則同 `hw sd` |
| `ver` | 所有人 | `ver <版本> idf=<IDF 版本> chip=esp32s31 built=<ELF SHA 前 9 碼>` |

`width` 為 1 時，`d1`..`d3` 要填 `-1`。

### `hw` 的回覆（每行都以 `hw ` 開頭，方便從雜訊日誌中挑出）

```
hw sd clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 width=4 src=default state=mounted mode=sdmmc
hw sdmmc routing=iomux clk=24 cmd=25 d0=20 d1=21 d2=22 d3=23 spi=any
hw led gpio=60 order=grb on=1 src=default
hw reserved 26,27,28,29,30,31,32,33,34,36,37,38,39,40,41,58,59,60,61
hw gpios 0-61
hw pending 0
```

- `hw sd` 與 `hw led` 顯示的是**已儲存**的設定（下次開機會用的）；`state` 是**這次開機**的實際結果：`mounted`、`unreadable`（卡有回應但檔案系統掛不上）、`none`（沒卡或沒回應）。`src=custom` 表示與板子預設不同。
- SPI 模式的 `hw sd` 行是 `hw sd clk=.. mosi=.. miso=.. cs=.. src=.. state=.. mode=spi`。
- LED 關閉時：`hw led gpio=-1 order=grb on=0 src=custom`。
- `hw sdmmc` 這行說明 SDMMC 主機只能用固定腳位（見 3.），`spi=any` 表示 SPI 模式可用任意腳位。
- `hw reserved` 是不能指派的 GPIO（含晶片上不存在的 29、41）。**預設值永遠被接受**：板子預設的 LED 腳位 60 在保留表裡（它是開機模式腳），但 `hw led 60` 與 `hw reset` 仍然成立。
- `hw pending 1`：儲存的設定與這次開機實際使用的不同，需要重開機。

### 拒絕原因（`refused …`）

| 原因 | 意思 |
|---|---|
| `syntax` | 參數不是數字、個數不對 |
| `range` | 晶片上沒有這個 GPIO（0..61，29 與 41 沒接出），或數字太大 |
| `reserved <n>` | GPIO n 在保留表內 |
| `duplicate <n>` | SD 腳位組內 GPIO n 重複 |
| `width` | 寬度不是 1 或 4；或寬度 1 時 d1..d3 不是 -1 |
| `order` | 顏色順序不是 rgb grb brg rbg gbr bgr |
| `sd/led conflict <n>` | LED 腳位與已儲存的 SD 腳位相同（GPIO n），反之亦然 |
| `not supported <原因>` | 晶片做不到。目前只有 `sdmmc-pins-fixed`：SDMMC 模式的腳位不是槽 0 的固定腳位，請改用 `hw sd spi …` |
| `storage` | NVS 寫入失敗 |

**寫錯腳位不會讓盒子不能用**：卡掛不上時（`state=none`），BLE、UART、`hw` 指令都不依賴卡，盒子照常運作，`st` 的 `card` 是 `none`，用 UART 的 `hw reset` 再重開機即可復原。真正無法用 `hw` 救回的只有「讓晶片開不了機」的接線（例如把外部電路接在開機模式腳上並拉到錯誤準位），這也是下面保留表偏保守的原因；那種情況只能用 esptool 清除 NVS（`esptool erase-region`）。

### 保留腳位表（`app/main/board/board_default.c`，依據 IDF v6.1）

| GPIO | 原因 | 依據（`~/.espressif/v6.1/esp-idf/components/` 下） |
|---|---|---|
| 29、41 | 晶片上沒有接出 | `soc/esp32s31/include/soc/soc_caps.h`：`SOC_GPIO_VALID_GPIO_MASK` 去掉 BIT29、BIT41 |
| 26 27 28 30 31 32 | SPI flash（CS0、Q、WP、HOLD、CLK、D） | `esp_hal_gpspi/esp32s31/include/soc/spi_pins.h`：`MSPI_IOMUX_PIN_NUM_*`；`soc/esp32s31/register/soc/io_mux_reg.h`：`FUNC_SPI*_FLASH_*_PAD`（功能 0） |
| 33、34 | USB 內建 PHY0 的 D-、D+，就是接 DAC 的 USB 主機埠 | `io_mux_reg.h`：`USB_INT_PHY0_DM_GPIO_NUM`、`USB_INT_PHY0_DP_GPIO_NUM`。USB-Serial-JTAG 在標頭檔裡沒有另外的腳位（`CONFIG_SOC_USB_FSLS_PHY_NUM=0`） |
| 36 | 開機腳：LDO 選擇 | `io_mux_reg.h` GPIO36 上方註解 `Strapping: LDO sel` |
| 37 | 開機腳：USB-JTAG / pad-JTAG 選擇 | 同上，GPIO37 |
| 38 39 40 | 開機腳：boot mode 0、1、2 | 同上；`soc/esp32s31/include/soc/boot_mode.h` 說明各組合（含下載模式） |
| 58、59 | 序列主控台 UART0 TX / RX，也是燒錄工具用的口 | `soc/esp32s31/include/soc/uart_pins.h`：`U0TXD_GPIO_NUM 58`、`U0RXD_GPIO_NUM 59`；`sdkconfig`：`CONFIG_ESP_CONSOLE_UART_NUM=0`；開機日誌「GPIO 59 and 58 are used as console UART I/O pins」 |
| 60 | 開機腳：boot mode 3。**板子預設 LED 在這裡，實測能開機**，所以只有 LED 預設值可用；其他用途保守保留 | `io_mux_reg.h`，GPIO60 |
| 61 | 開機腳：boot mode 4，保守保留 | `io_mux_reg.h`，GPIO61 |

不保留、但要知道的：

- **八線 PSRAM**：用的是專屬接腳，沒有 GPIO 編號（`soc/esp32s31/register/soc/iomux_mspi_pin_reg.h` 裡的 `IOMUX_MSPI_PIN_PSRAM_*`，不在 `io_mux_reg.h` 的 GPIO 列表）。GPIO9–19 只有一個選用的除錯功能 `DBG_PSRAM_*`（功能 4，預設是 GPIO），所以**不保留**。
- **沒有「只能輸入」的腳**：`SOC_GPIO_VALID_OUTPUT_GPIO_MASK` 等於 `SOC_GPIO_VALID_GPIO_MASK`，`SOC_GPIO_OUT_RANGE_MAX` 也是 61，不需要另外排除。
- **GPIO0、1**（32 kHz 晶振腳）：本韌體 `CONFIG_RTC_CLK_SRC_INT_RC=y`，沒有用外接晶振，所以不保留。板子若焊了 32 kHz 晶振，請把它們加進表。
- **GPIO54–57**（pad JTAG：MTDO、MTCK、MTDI、MTMS）：不保留；拿來當一般腳只會失去 pad-JTAG 除錯。
- 預覽晶片的資料不完整之處一律偏保守（表中標「保守保留」），要放寬請先在實體板上驗證，再改 `board_default.c`。

## 2. TF 卡

### 這顆晶片的 SDMMC 能不能用任意腳位？**不能。**

證據（`~/.espressif/v6.1/esp-idf/components/`）：

- `soc/esp32s31/include/soc/soc_caps.h`：`SOC_SDMMC_USE_IOMUX 1`，**沒有** `SOC_SDMMC_USE_GPIO_MATRIX`。
- `esp_hal_sd/esp32s31/include/hal/sdmmc_ll.h`：`SDMMC_LL_SLOT_SUPPORT_GPIO_MATRIX(SLOT_ID) 0`（兩個槽都是）。
- `esp_hal_sd/esp32s31/sdmmc_periph.c`：`sdmmc_slot_gpio_sig[]` 全是 `-1`（沒有給 GPIO 矩陣用的訊號編號）。
- `esp_driver_sdmmc/src/sd_host_sdmmc.c`：槽 1 要走矩陣會直接回 `ESP_ERR_INVALID_ARG`（"doesn't support routing from GPIO matrix, driver uses dedicated IOs"）；槽 0 的檢查只是比對腳位是否等於 IOMUX 腳位。
- 固定腳位（`esp_hal_sd/esp32s31/include/soc/sdmmc_pins.h`）：**槽 0**：CLK 24、CMD 25、D0 20、D1 21、D2 22、D3 23；槽 1：CLK 39、CMD 40、D0 35、D1 36、D2 37、D3 38。槽 1 的腳位含 36–40（開機腳），在保留表內，`hw sd` 會以 `reserved` 拒絕。

所以：

- `hw sd <…>`（SDMMC 模式）只接受槽 0 的腳位（寬度 4 或 1）；其他組合一律 `refused not supported sdmmc-pins-fixed`，`hw` 的 `hw sdmmc routing=iomux …` 那行就是在說這件事。
- **想把卡接在別的腳位（焊在 S31 mini 上的 SD 卡座）請用 SPI 模式：`hw sd spi <clk> <mosi> <miso> <cs>`**。SPI 走 GPIO 矩陣，腳位任意（要通過上面的保留表檢查），CMD→MOSI、D0→MISO、D3→CS。實作在 `app/main/storage.c`（`esp_driver_sdspi`，SPI2 主機，20 MHz）。
- SPI 模式的速度：1 位元匯流排，最高約 20 MHz，實際讀取大約每秒 1–2 MB（同一張卡 SDMMC 4 位元實測 7.95 MB/s）。一般音檔足夠；高取樣率、高位元深度的未壓縮檔（例如 384 kHz/24 位元立體聲約 2.3 MB/s）可能跟不上。接線很長、很亂時可以在 `storage.c` 把 `SPI_FREQ_KHZ` 調低。
- 一張卡一旦被切到 SPI 模式，要**斷電重上電**才會回到 SDMMC 模式；改回 SDMMC 設定後若卡沒反應，請拔掉再插卡（或整機斷電）。

### 接線建議（自己焊卡座時）

- **只能 3.3 V**。卡座的電源要穩，卡寫入時瞬間電流可到 100 mA 以上，旁邊放 10 µF + 0.1 µF 電容。
- CMD、D0–D3（SPI 模式：MOSI、MISO、CS）各加一顆**外部 10 kΩ 上拉電阻到 3.3 V**。韌體開了晶片內建上拉，但那顆太弱（約 45 kΩ），手焊的長線很容易讀不到卡。
- CLK 線盡量短；SPI 模式 CS 要上拉，避免開機時卡被誤選。
- 卡座如果有偵測腳（CD），本韌體不使用它；沒有卡時 `state=none`，插卡後 5 秒內會自動掛上。

## 3. WS2812 LED

- `hw led <gpio> [順序]`：預設 GPIO60、順序 `grb`。順序是 LED 實際收到的顏色位元組順序，常見 WS2812 是 `grb`，有些 LED（如部分 WS2811 模組）是 `rgb`。顏色不對就換順序。
- 5 V 供電的 WS2812 接 3.3 V 資料線：規格上高準位要 0.7 × VDD = 3.5 V，3.3 V 常常勉強能用、也常常不行。穩當的作法是用 3.3 V 供電的型號（SK6812 等），或在資料線上加電位轉換（74HCT125 之類）。資料線串 220–470 Ω 電阻。
- 只驅動第一顆（`max_leds = 1`）：韌體假設只接一顆，後面的燈不會被熄滅。接燈條時要自己處理。
- 電流：一顆 WS2812 全白約 60 mA；韌體預設亮度很低（12/255），床頭用不會超過幾 mA。
- LED 型號（時序）在 `board_led_model()`，預設 `LED_MODEL_WS2812`；SK6812 / WS2811 在那裡改。
- `hw led off`：開機時完全不建立 LED 工作，腳位不被動到（接了別的東西也安全）。紅/白交替的格式化警示與卡片測速的脈動只有 LED 開著時才看得到。

## 4. 擴充點（沒有實作）

所有檔案都在 `app/main/board/`，介面是 `board.h`。這裡放在 `main` 元件裡、不另做元件：這些 hook 要呼叫播放器、按鍵、頻譜等 `main` 內部模組，獨立元件會造成循環相依。

通則（`board.h` 開頭也有）：

- 內部 RAM 很緊（藍牙、Wi-Fi 住在裡面）：緩衝區、畫面緩衝、環形緩衝、工作堆疊一律放 PSRAM（`heap_caps_malloc(…, MALLOC_CAP_SPIRAM)`、`xTaskCreate…WithCaps(…, MALLOC_CAP_SPIRAM)`）。用 `WithCaps` 建立的工作**不可自己刪除自己**（堆疊在 PSRAM，要做成常駐、用通知喚醒）。
- **會寫 NVS（任何會寫 flash）的工作要用內部堆疊**；PSRAM 堆疊的工作不可寫 NVS（flash 寫入時 PSRAM 不可存取）。
- hook 只在開機時呼叫一次，跑在 `app_main` 的工作（8 KB 堆疊）裡：啟動自己的工作、快速返回，不要在 hook 裡無窮迴圈。
- 失敗要回錯誤碼，不可 abort；盒子必須在 hook 失敗時仍可用。

### 4.1 顯示器：`board_display_init()`

- 檔案與函式：`app/main/board/board_default.c` 的 `board_display_init()`（預設空）。
- 呼叫時機：`app_main` 裡，儲存裝置、播放器、BLE 控制服務都起來之後，主迴圈之前（日誌：`board display: ESP_OK`）。
- 可以做：初始化匯流排與面板，建立自己的顯示工作（PSRAM 堆疊、**畫面緩衝放 PSRAM**）。**LVGL 刻意沒有放進來**（它的工作記憶體與計時工作都不便宜）。
- 唯讀資料來源：
  - `nsn_player_info()`（`player.h`）：播放狀態、標題、演出者、位置、音量、取樣率、睡眠計時。
  - 頻譜（`spectrum.h`）：`nsn_spec_last()` 取最新 32 個頻帶；原本只有 BLE 控制那邊用 `nsn_spec_start()` 註冊一個回呼，**新增了 `nsn_spec_observe()`** 讓第二個消費者（顯示器）也能註冊（最多 2 個，在頻譜工作裡每秒約 30 次呼叫，要短、不可阻塞）。
- 不可做：直接改播放器狀態。需要下指令就走 `nsn_cmd_exec()`（`cmd.h`，BLE 用的同一個入口）。

### 4.2 GPIO 按鍵：`board_buttons_init()`

- 檔案與函式：`board_default.c` 的 `board_buttons_init()`（預設空），註解裡有可抄的範例。
- 呼叫時機：和顯示器同一處，緊接在後（日誌：`board buttons: ESP_OK`）。
- 做法：設定腳位（輸入、上拉；避開保留表與 TF 卡/LED 腳位），起一個輪詢工作（10 ms 就夠去彈跳），按下與放開時呼叫 `nsn_keys_event(NSN_KEY_CENTER / NSN_KEY_PLUS / NSN_KEY_MINUS, down)`（`keys.h`）。之後一切都由 `keys.c` 處理：點一下／兩下／三下／長按 → 動作，用 `keys` 指令設定，存在 NVS。
- 這個工作可以用內部堆疊（範例如此）；動作與 NVS 寫入由 `keys.c` 自己的內部堆疊工作做，所以這個工作本身不寫 NVS。
- 鍵的數量：`NSN_KEY_N` 是 3（中、加、減），沒有為此擴充，以免動到已存的對應表。要更多鍵時把 `keys.h` 的列舉與 `keys.c` 的 `BTN`、`NVKEY` 表往後加，前三個的儲存格式不變。

### 4.3 I2S 輸出（與 I2C 編解碼器初始化）

輸出現在是一張「sink 表」（`app/main/sink.h`）：播放器只認 `nsn_sink_t`，USB DAC（`dac`）、藍牙耳機（`bt`）、Auracast（`cast`）都是這張表裡的項目（定義在 `player.c`，id 固定 1、2、3）。

```c
typedef struct nsn_sink {
    const char *name;        /* "dac" "bt" "cast" "i2s"：`st` 的 "out"、`out <name>` */
    const char *label;       /* 日誌用短名 */
    const char *desc;        /* "first use of <desc>" 日誌用 */
    const char *vol_key;     /* 此輸出自己的音量 NVS 鍵；NULL = 主音量（只有 dac） */
    uint8_t auto_rank;       /* `out auto` 選「在場且 rank 最小」的；0 = auto 不會選（cast） */
    bool resume_after_drop;  /* 播放中消失時記住曲目、回來後續播（藍牙） */
    const char *gone_msg;    /* 播放中 present() 變 false 時的警告日誌 */
    bool (*present)(void);
    uint32_t (*link_rate)(uint32_t src_rate);
    void (*describe)(char *buf, size_t n);
    esp_err_t (*start)(uint32_t rate, nsn_sink_pull_t pull, void *ctx);
    void (*stop)(void);
} nsn_sink_t;
```

`auto` 的順序由 `auto_rank` 決定：dac 10 < i2s 20 < bt 30；cast 為 0（不會被 auto 選）。單選用 `out i2s`（只在 `present()` 回 true 時才有作用）。

範本 `app/main/board/sink_i2s.c` 只有在 Kconfig `NSN_SINK_I2S=y`（預設 n）時才編譯，關閉時沒有任何程式碼進映像。**範本沒有實作**：所有腳位是 `GPIO_NUM_NC`，`present()` 因此永遠回 false，`out i2s` 不會有任何效果；標了 `TODO(board)` 的地方是你要決定的。

**新增 I2S DAC 的檢查清單**

1. `idf.py menuconfig` → NightStand Nano → 打開 `NSN_SINK_I2S`（或在 `sdkconfig.defaults` 加 `CONFIG_NSN_SINK_I2S=y`）。
2. 在 `sink_i2s.c` 填 `I2S_PIN_MCLK/BCLK/WS/DOUT`。腳位要通過 `hw reserved`（不在保留表）、不與 TF 卡、LED 腳位重複；I2S 走 GPIO 矩陣，任何 GPIO 都可以。
3. `i2s_link_rate()`：這顆 DAC 吃哪些取樣率。回傳的就是播放器要重取樣成的速率（預設全部轉 48 kHz）。
4. 編解碼器要靠 I2C 設定的（例如 ES8311、PCM5xxx 的控制介面）：在 `board_default.c` 的 `board_codec_init(rate)` 建立 I2C 匯流排（`esp_driver_i2c`）、寫暫存器；純 I2S DAC（PCM5102 之類）就保持回 `ESP_ERR_NOT_SUPPORTED`。靜音/取消靜音寫在 `board_codec_mute()`。I2C 腳位同樣要避開保留表。
5. `i2s_present()`：填入「DAC 在不在」的判斷（焊死的就回 true；可拆的做探測並快取，不要在這裡阻塞）。
6. `slot_cfg`：非 Philips 格式的 DAC 改 MSB / PCM。要 MCLK 的改 `mclk_multiple`。
7. 輸出的音量是播放器的軟體音量，每個輸出自己一個，存在 NVS 鍵 `vol_i2s`；第一次使用的安全起始音量規則和藍牙、Auracast 相同。
8. 測：`out i2s` → `play /sdcard/…` → `st` 的 `"out":"i2s"`；`out auto` 時 USB DAC 優先、其次 I2S、再其次藍牙。

拉取回呼（`nsn_sink_pull_t`）跑在 sink 自己的環境（這個範本是 `i2s` 工作）：要快、不阻塞、不寫 NVS、不寫日誌。範本裡的 `i2s` 工作堆疊在 PSRAM，常駐、用通知喚醒，不會自己刪除。

## 5. 版本

- `ver` 回報專案版本、IDF 版本與 ELF SHA 前 9 碼（建置可重現，所以沒有建置日期，ELF SHA 用來區分同一版本的兩次建置）。開機日誌第一行也有版本。
- 版本來源（`app/CMakeLists.txt`）：`idf.py -DPROJECT_VER=1.2.3 build`（發行用）> `git describe --tags --dirty`（有 tag 時）> repo 根目錄 `version.txt` 加上 commit（`0.1.0-19c6046`，沒有 git 時只有 `0.1.0`）。版本在設定階段讀取，commit 或打 tag 後請執行 `idf.py reconfigure`。
