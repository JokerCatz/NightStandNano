# 研究筆記：Waveshare ESP32-S31-Zero

狀態：**暫緩**（主人 2026-11-25）。先記錄，之後可能還有其他硬體可選，到時候再一起評估。這裡的內容都來自官方資料與推論，**沒有實物驗證**。

資料來源：
- https://docs.waveshare.com/ESP32-S31-Zero （腳位圖只有圖片）
- https://github.com/waveshareteam/ESP32-S31-Zero （範例 `01_gpio_output`、`05_uart_loopback`、`06_i2c_scan`、`11_board_test`）

## 板子上有什麼
- 晶片 ESP32-S31；有 16／32 MB Flash 與 16／32 MB PSRAM 的組合（N16R16、N32R16、N16R32、N32R32）。
- WS2812B-1010 RGB LED 在 **GPIO60**（與 Nano 的預設相同）。
- BOOT 鍵在 GPIO61（低電位有效）；另有 RESET 鍵（接晶片的 EN）。
- **只有一個 USB-C**，直接接到晶片的 USB（GPIO33／34，USB-Serial-JTAG）。**沒有 USB-UART 轉接晶片**。
- 板子背面有「USB Host 焊點」（D+、D−），與 USB-C 是同一組訊號。
- **沒有 TF 卡座**。
- 一個 40 針板對板連接器（細間距），上面有 RGB LCD、觸控、SD 的訊號。

## 兩側焊盤（USB-C 朝上，由上到下）
| 左側 | 右側 |
|---|---|
| 5V | GPIO58（UART0 TX） |
| GND | GPIO59（UART0 RX） |
| 3V3 | GPIO51 |
| GPIO0 | GPIO50 |
| GPIO1 | GPIO49 |
| GPIO2 | GPIO48 |
| GPIO3 | GPIO47 |
| GPIO4 | GPIO46 |
| GPIO5 | GPIO6 |

共 15 支 GPIO：0–6、46–51、58、59。GPIO0–6 屬低功耗域（LP_GPIO），46–51 有 ADC 功能。

## 40 針連接器上與 Nano 有關的訊號
- SD（SDMMC 固定腳位）：D0=GPIO20、D1=21、D2=22、D3=23、CLK=24、CMD=25（第 29–39 針的奇數針）。
- USB：GPIO33（D−）、GPIO34（D+）也出現在這裡（與 LCD 資料線共用）。
- 其餘是 LCD（GPIO8–19、35–37、39、40、42–45）與觸控（GPIO52、53、7、38）。

## 對 Nano 的影響
1. **SD 卡**：這顆晶片的 SDMMC 只能用固定腳位（GPIO20–25，見 `docs/porting.md`），而 Zero 上這六支腳只在 40 針細間距連接器上，不適合手焊。接 SD 卡套只能走 **SPI 模式**（`hw sd spi …`）。
2. **小尾巴（USB DAC）**：USB-C 是裝置端設計，推測不會對外供 5 V；小尾巴需要主機供電。可行的做法推測是：在背面的 USB Host 焊點接一個 USB 母座，加上 5V 與 GND。USB-C 與 Host 焊點是同一組訊號，同一時間只能用一邊。**這是目前最大的疑問，主人也認為只有一個 Type-C 埠有問題。**
3. **燒錄與主控台**：沒有 UART 轉接晶片。韌體啟動 USB 主機功能後，電腦就無法經 USB-C 連線；重新燒錄要按住 BOOT 再開機。硬體設定（`hw`）需要主控台，選項有：外接 USB-UART 到 GPIO58／59，或在韌體加一個「設定模式」（開機後按 BOOT，不啟動 USB 主機、主控台改開在 USB-C 上）。後者未實作。
4. **預設腳位**：Nano 的預設是內建卡座（SDMMC）。Zero 需要另一組板級預設（SPI），可以用板級設定架構出一個專用的韌體版本。

## 曾經提出的接線草案（未驗證）
SD 卡套全部接在左側，SPI 模式：

| SD 卡套接點 | SPI 功能 | Zero |
|---|---|---|
| 1 | CS | GPIO0 |
| 2 | MOSI | GPIO1 |
| 3 | GND | GND |
| 4 | 3.3 V | 3V3 |
| 5 | CLK | GPIO2 |
| 6 | GND | GND |
| 7 | MISO | GPIO3 |
| 8、9 | 不接 | — |

設定指令：`hw sd spi 2 1 3 0`（clk mosi miso cs）。GPIO4、5 留給日後的按鍵或 I2S。備案是右側的 GPIO46–51（電源線要橫跨板子）。

## 動手前要先確認的事
- SPI 模式在真卡上能不能用、速度多少（Nano 目前沒有在真卡上跑過 SPI 模式）。
- 低功耗域的 GPIO0–5 能不能跑 20 MHz 的 SPI（預覽晶片，未驗證）。
- USB-C 直接插小尾巴會不會動；Host 焊點加母座的供電方式。
- USB-C 在 BOOT 模式下燒錄、以及「設定模式」主控台的可行性。
- 16 MB 以外的 Flash／PSRAM 組合是否需要不同的建置設定。
