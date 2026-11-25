# 更多 FM 晶片的 VGM／S98 播放：ymfm 可行性量測與設計（步驟 1）

日期：2026-11-25。範圍：只在 PC 上量測並寫設計；沒有動韌體建置目錄、板子、序列埠，也沒有改 `app/ components/ web/ flash/ tools/`，沒有 commit。
所有暫存檔、複製的原始碼、基準程式都在 scratchpad（路徑見最後一節），沒有加進 repo。

每個數字都標來源：**[PC]** = 在這台 PC 上量到的；**[RV]** = 用 RISC-V 編譯器（韌體旗標）編譯／連結／反組譯得到，但沒有跑；**[估]** = 換算到板子的估計（附方法與誤差）；**[引]** = 引用 JukeDial 或本 repo 文件裡的板上實測。

## 0. 結論先行

1. **ymfm 技術上可用，程式碼很小，記憶體很小，但 CPU 是真正的限制。** FM 部分四個晶片家族（YM2151、YM2203、YM2608、YM2610／B）合併連結只有約 **15 KB 程式碼 + 4.3 KB 唯讀表 [RV, -O2]**，每個晶片物件 3–5 KB（YM2151 約 7 KB 含動態配置）。分析文件裡的「+100–200 KB」高估了一個數量級；加上 8 個取樣晶片核心也只多 15–30 KB。
2. **ymfm 比現在的 GME YM2612 核心貴 3–5 倍**（同一個檔案、同一台 PC：YM2612 ymfm / GME 整體 = 3.0–5.3 倍 [PC]）。換成板上數字：一個「稠密」的 8 聲部 YM2151 約佔核心 1 的 **90–98 %**（只算晶片，不含重取樣與管線）[估，誤差約 ±35 %]；實際曲子比稠密合成流便宜 0.4–0.7 倍（用 Mega Drive 真檔校準 [PC]），所以多半在 35–70 %。**YM2203（3 聲部 FM + SSG）是唯一明顯寬裕的：約 17–49 %。**
3. **GME 並不「拒絕」這些檔案，而是靜音播放**（YM2151、YM2203、YM2608、YM2610 的 VGM 都能被 `gme_open_data` 打開，輸出 rms = 0、沒有警告 [PC]）。因為 GME 的 VGM 標頭結構只有 0x40 位元組，只認 SN76489／YM2413／YM2612。所以**路由必須在打開檔案前看標頭**，不能靠「打開失敗」再換後端。
4. **取樣晶片有可用的寬鬆授權實作**：libvgm 的 OKI6295、Sega PCM、K053260、K054539、C140、C352、QSound、RF5C68、YMZ280B、MultiPCM、K007232、X1-010、uPD7759、GA20、MSM6258、MSM5205 等檔案頭都標 `BSD-3-Clause`（我另外對 MAME 主線同名檔案核對過，標籤一致）。「MAME 的核心都是 GPL」已經過時：MAME 現在是逐檔授權，這些裝置檔是 BSD-3。**真正不能用的是**：libvgm 裡的 fmopn.c、fmopl.c、ym2151.c、ymf278b.c、msm5232.c、vsu.c（GPL-2.0+）和 Gens 系的 scd_pcm.c／ym2612.c。
5. **建議：Go，但分階段，先 YM2203 → YM2151（含 OKI6295）→ YM2610，板上量到數字再決定 YM2608／YM2610B 與重型輸出。** 只有 USB DAC 輸出看起來可行；藍牙 AAC 只剩輕的 YM2203 曲子勉強；LDAC 與 Auracast 不可行（見 §8.8）。

## 1. 量測環境與方法

| 項目 | 內容 |
|---|---|
| 機器 | Intel Core i5-12500（Alder Lake），Linux 6.17，gcc／g++ 15.2.0，cpufreq 800 MHz–4.6 GHz（時脈未鎖定） |
| 時間 | `clock_gettime(CLOCK_PROCESS_CPUTIME_ID)`，`taskset` 綁核，單執行緒，取 5–7 次最小值，音訊 30–40 秒；**時脈會浮動，所以時間只當參考** |
| 指令數 | `valgrind --tool=callgrind`，取 2 秒與 4 秒兩次的差除以 2（扣掉建表等一次性成本）。**`perf stat` 不能用**（`perf_event_paranoid = 4`，沒有 sudo），所以指令數全部是 callgrind 的 |
| 一個怪點 | 指令數除以時間約 15–27 G 指令／秒（IPC 約 4.5–6），所有基準一致，PC 這顆核心在這種小型整數迴圈上就是這麼快。因此「時間比」與「指令比」互差 ≤ 20 %，下面兩者都列 |
| 編譯 | `g++ -std=c++14 -O2 -DNDEBUG`（另有 `-Os`）；`-fno-jump-tables -fno-tree-switch-conversion`（韌體旗標）對 x86 的指令數沒有影響 |
| ymfm | https://github.com/aaronsgiles/ymfm commit `81aec25ccbb98f4873a255f7551ac4dadac59b4a`（2026-07-27），BSD-3-Clause |
| 對照 | 韌體的 GME（`components/gme`，與韌體同一組原始碼與 `VGM_YM2612_GENS`）在 PC 上編譯，用 `gme_play` 以 44.1 kHz 取樣率跑 |

### 測試資料（說了算）

- **合成 VGM**（我自己用 `gen_vgm.py` 產生，沒有任何第三方內容）：`ym2151 / ym2203 / ym2608 / ym2610 / ym2610b / ym2612`，各兩種密度，60 秒，VGM 1.71 格式，每 1/60 秒一個 tick。
  - 「dense」＝所有聲部首尾相接連續發音（閘門時間 100 %，釋放尾巴重疊）；「typical」＝閘門 60 %。每個聲部有各自的音色（演算法、回授、TL、AR／D1R／D2R／RR、DT／MUL）、OPM 開 LFO（PM／AM）、OPN 有 SSG 三音，YM2610／YM2608 另有 ADPCM 觸發（見下）。
  - YM2610：2 MB 的 ADPCM-A ROM 資料區塊（0x82）和 512 KB 的 Delta-T（0x83）。**取樣資料是隨機位元組，不是音樂**（成本與資料內容幾乎無關，聲音是雜訊）。YM2608：256 KB Delta-T（0x81）加 8 KB 假的節奏 ROM（真正的節奏 ROM 不在 VGM 裡，見 §3.3）。
  - **聲音正確性沒有人耳驗過**：只確認輸出非靜音（rms 見各檔輸出），且加入 ADPCM 後 rms 從 9192 升到 21326（ADPCM 確實在跑）。
- **真實 YM2612 VGM**（校準合成流用）：主人本機快取 `~/offsync/rgxx/cache/chiptune/genesis/sonic-knuckles/` 的 3 首（03、14、17 號）與 `ellsworth-all-by-myself.vgm`。只在本機讀取量測，沒有複製進 repo。
- **沒有真的 YM2151／OPN／OPNB 檔案可量**。vgmrips.net 在這次嘗試時回 HTTP 410（網站已下架該路徑），所以沒辦法確認「下載幾包做私人量測」的條款，也就沒下載。**請主人提供每個家族 3–5 個真檔做板上與 PC 驗證**（見 §10）。
- 基準程式內的 VGM 剖析器：標頭（各晶片時脈欄位、雙晶片位元、資料偏移、迴圈偏移）、指令 0x52–0x59（YM2612／2151／2203／2608／2610 的埠 0／1）、0x61／62／63／7n 等待、0x67 資料區塊（0x00 DAC、0x81／82／83 ADPCM）、0x8n DAC＋等待、0xE0 搜尋、迴圈、`.vgz` 用 zlib。寫入的時間點用 Q32 定點對齊到晶片原生取樣。

## 2. ymfm：取得、授權、需要的檔案、對主機的要求

### 2.1 授權

`LICENSE`：BSD 3-Clause，Copyright (c) 2021, Aaron Giles。三條：保留版權與條件（原始碼）、二進位發行時在文件中重製版權與條件、不得用作者名字背書。與 LGPL-2.1+ 相容；做法與其他第三方元件相同：把 LICENSE 放進 `docs/licenses/`，在 `THIRD_PARTY_NOTICES.md` 列入。

### 2.2 各晶片需要的來源檔（`src/`）

| 晶片 | 類別 | 必要檔案 | 連結後 RV32 大小（-O2，text／rodata；gc-sections，只含該晶片） [RV] |
|---|---|---|---|
| YM2151（OPM） | `ymfm::ym2151` | `ymfm.h ymfm_fm.h ymfm_fm.ipp ymfm_opm.h ymfm_opm.cpp` | 3,336 / 4,000 B |
| YM2203（OPN） | `ym2203` | `ymfm_opn.h ymfm_opn.cpp`（+ `ymfm_fm.*`）+ SSG：`ymfm_ssg.h ymfm_ssg.cpp` | 4,938 / 984 B |
| YM2608（OPNA） | `ym2608` | 同上 + ADPCM-A／B：`ymfm_adpcm.h ymfm_adpcm.cpp` | 7,930 / 1,172 B |
| YM2610／YM2610B（OPNB） | `ym2610`、`ym2610b` | 同上（兩者共用同一份程式，B 只是 6 聲道遮罩） | 5,604 / 1,106 B |
| YM2612／YM3438（對照用） | `ym2612` | `ymfm_opn.*` | 3,472 / 992 B |
| YM3812（OPL2）、YMF262（OPL3） | `ym3812`、`ymf262` | `ymfm_opl.h ymfm_opl.cpp`；**注意** `ymfm_opl.cpp` 也引用 `ymfm_pcm.cpp`（OPL4 的 PCM），連結時要一併給或用 `-ffunction-sections` 讓 gc 丟掉 | 3,198 / 792 B；4,638 / 816 B |
| YM2413（OPLL）、YM2149 | `ym2413`、`ym2149` | `ymfm_opl.*`；`ymfm_misc.*`（YM2149 單獨 686 / 64 B） | — |
| 不需要 | `ymfm_opq.* ymfm_opz.* ymfm_opx.h ymfm_pcm.*`（OPQ／OPZ／OPL4 PCM） | | |

「OPL 只在不費力時順便含入」：OPL2 約 +3.2 KB、OPL3 +4.6 KB 程式碼，但 OPL3 的物件 17 KB，且韌體已有 AdPlug 的 fmopl（DOS 音樂路線），所以 **VGM 的 OPL 先不做**（需要時再開）。

兩個合併的連結結果（RV32、韌體旗標、`--gc-sections`，只含這些晶片類別）[RV]：

| 組合 | .text | .rodata（含 .srodata） | .bss（晶片物件，未含動態配置） |
|---|---|---|---|
| OPM + OPN + OPNA + OPNB（-O2） | 15,254 B | 4,300 B | 12,936 B |
| 同上（-Os） | 13,374 B | 4,300 B | 12,936 B |
| 再加 YM2612 | 15,914 B（-O2） | 4,300 B | 15,664 B |
| 再加 YM2612 + OPL2 + OPLL | 22,328 B（-O2） | 4,324 B | 28,836 B |

### 2.3 主機要提供什麼（`ymfm_interface` 回呼，全部有預設實作）

只有兩個是播放 VGM 真正要寫的：

| 回呼 | 需要嗎 | 說明 |
|---|---|---|
| `ymfm_external_read(access_class, addr)` | **要**（ADPCM-A／B 資料） | `ACCESS_ADPCM_A`（YM2610 的 ADPCM-A ROM、YM2608 的節奏 ROM）、`ACCESS_ADPCM_B`（Delta-T ROM 或 RAM）。逐位元組呼叫，是虛擬函式；回傳 `data[addr]`，超出範圍回 0 |
| `ymfm_external_write(access_class, addr, data)` | YM2608 要（Delta-T RAM 256 KB） | 暫存器寫入 Delta-T 記憶體時呼叫（`ymfm_adpcm.cpp:674`） |
| `ymfm_set_timer`、`ymfm_sync_mode_write`、`ymfm_sync_check_interrupts`、`ymfm_update_irq` | 不需要 | 播檔案不需要計時器與 IRQ；預設實作可用（我的基準全部用預設，輸出正確非靜音）。只有「模擬 CPU 讀狀態暫存器」才需要 |
| `ymfm_set_busy_end`、`ymfm_is_busy` | 不需要 | 忙碌旗標只影響讀狀態暫存器 |

用法：`chip.write(offset, data)`（0／1 = 位址／資料，2／3 = 高埠位址／資料）、`chip.generate(out*, n)`、`chip.sample_rate(clock)`。

### 2.4 C++ 要求（對 `-fno-exceptions -fno-rtti` 的 ESP-IDF 建置）

- 標準：README 寫 C++14；我用韌體的旗標組（`riscv32-esp-elf-g++ 15.2 -std=gnu++26 -fno-exceptions -fno-rtti -fno-jump-tables -march=rv32imafc_zicsr_zifencei_zaamo_zalrsc_xesploop_xespv -mtune=esp-base`）編 opm／opn／adpcm／ssg／opl／misc 全部通過 [RV]。GCC 15 在 `ymfm_fm.ipp:1525`（OPL 路徑的計時器陣列）會報一個陣列邊界警告；韌體是 `-Wall -Werror`，元件要像 gme 一樣加 `-w`。
- 例外、RTTI：不需要（只有虛擬函式；沒有 `dynamic_cast`／`typeid`／`throw`）。
- STL：`<algorithm> <array> <memory> <string> <vector>` 被 `ymfm.h` 一律 include，但：`std::vector` 只用在存檔／還原與 wav 除錯類別；`std::string` 只在 `log_keyon`（`LOG_KEYON_EVENTS = false` 的 `constexpr`，死碼被丟掉）；`std::unique_ptr`／`make_unique` 用在聲道與運算元物件。連結 map 顯示沒有拉進 `std::string` 或 `std::vector` 的程式碼。
- 堆積：**只在建構時**配置（`make_unique`，每個運算元 68 B、每個聲道 36 B、ADPCM 聲道 40／48 B，見 §6），`generate()` 不配置 [PC：對全域 `operator new` 計數：建構後 0 次]。所以渲染路徑沒有配置。
- 浮點：**ymfm 是純整數**。確認方法：(1) `grep float|double` 只出現在註解與 wav 除錯類別；(2) 用 `-march=rv32imac -mabi=ilp32`（沒有 F 擴充）編 opm／opn／adpcm／ssg，`nm -u` 沒有任何 `__*sf3`／`__*df3` 浮點函式庫呼叫 [RV]。
- `assert`：`ymfm_opm.h`／`ymfm_opn.h` 等有一些索引斷言，OPL 路徑另有 `rhythm_enable` 斷言。韌體開著斷言（docs 說 GME 的 FIX6／FIX7 就是為了這個），所以：元件用 `-DNDEBUG` 編，且**一律在剖析層驗證輸入**（時脈範圍、區塊大小、位址），跟 GME 的 patch 0012／0013 同一套做法。時脈欄位尤其要驗：`0x3FFFFFFF` 會讓晶片每秒產生上千萬個取樣。

## 3. 取樣晶片（ymfm 沒有的）

### 3.1 哪個目標要哪些晶片

| 目標硬體 | FM／內建 | 還需要的取樣／其他晶片 |
|---|---|---|
| Sega System 16／18／OutRun／Space Harrier／Hang-On | YM2151 | Sega PCM（315-5218）；部分 System 16 板子另有 uPD7759 |
| Sega System 32 | YM3438（＝YM2612 相容） | RF5C68；Model 1／2 用 MultiPCM |
| CPS-1 | YM2151 | OKI MSM6295（早期板子有 YM2203 + MSM5205） |
| CPS-2 | — | QSound（取樣為主，沒有 FM） |
| Konami 街機 | YM2151（多數） | K007232、K053260、K054539、K051649（SCC）、其中 K054539／K053260 是取樣主體 |
| Namco 街機 | YM2151（System 1／2） | C140（System 2）、C352（System 21／22）、C219（NA-1/2）；System 1 的自家音源不在 VGM 內 |
| Irem M72／M92 | YM2151 | GA20（M92）；M72 用 DAC 取樣 |
| Taito F2／Z／B | YM2610 | 無（ADPCM 在晶片內，資料在檔案裡）；Taito F3 用 ES5505／6（沒有寬鬆實作） |
| Neo Geo | YM2610 | 無 |
| Capcom 早期（1942、Commando、Ghosts'n Goblins） | YM2203 × 2（雙晶片）+ 有的有 AY8910 | 無（AY 部分用 ymfm 的 SSG） |
| Toaplan／Seibu／Data East | YM2151 或 YM3812 或 YM2203 | OKI MSM6295（多數）、HuC6280 |
| X68000 | YM2151 | OKI MSM6258（ADPCM；多數曲子以 FM 為主） |
| PC-88／PC-98／FM Towns | YM2203、YM2608（含節奏與 ADPCM-B）、YMF288、FM Towns 的 YM2612 | 無額外晶片（節奏 ROM 見 §3.3） |

**單靠 ymfm 就能「聽起來對」的（純 FM 配樂）**：PC-88／PC-98、Neo Geo、Taito F2／Z／B、Capcom 的 YM2203 世代、X68000 的多數 FM 曲、CPS-1 的多數旋律（打擊樂與語音靠 OKI，少了會缺鼓與效果音）。
**需要取樣晶片才對**：CPS-2（QSound 是主音源）、Konami 的 K054539／K053260 世代、Namco System 2／21／22、Sega System 32 與 Model 1／2；Sega System 16／18 的鼓與人聲有 Sega PCM（FM 旋律可聽，少了鼓）。

### 3.2 寬鬆授權的實作（逐檔核對）

方法：clone `ValleyBell/libvgm`（commit `c5c03a2c2514d38d2ab7b7e8dbd021af0b8292cb`，2026-11-04），逐檔讀檔頭；**該 repo 沒有頂層 LICENSE 檔，只有逐檔標示**。對標 `BSD-3-Clause` 的檔案，再到 `mamedev/mame` 主線讀對應 `src/devices/sound/*.cpp/.h` 的 `license:` 行（okim6295、segapcm、k054539、c140、rf5c68、ymz280b、qsound、multipcm、k053260、okim6258、k007232、c352、ymopm、ymopn 都是 `BSD-3-Clause`）。GitHub API 對整個 MAME repo 回「NOASSERTION」（混合授權）。

大小是用韌體旗標編成 RV32 的單一 `.o`（`-O2`／`-Os`），**含 libvgm 的裝置描述表，不含 libvgm 的播放器與重取樣**，.text 已含 .rodata [RV]；CPU 沒量（見註）。

| 晶片 | 用在 | 實作（libvgm `emu/cores/`） | 檔頭授權 | 原始碼大小 | RV32 -O2／-Os（text；bss） |
|---|---|---|---|---|---|
| OKI MSM6295 | CPS-1、Toaplan、Seibu、Data East、許多街機 | `okim6295.c` + `okiadpcm.c` | BSD-3-Clause | 18 KB + 3.5 KB | 1.7 + 0.6 KB；bss 1.6 KB |
| Sega PCM（315-5218） | Sega 16／18／OutRun | `segapcm.c` | BSD-3-Clause | 8 KB | 0.9 KB |
| K053260 | Konami | `k053260.c` | BSD-3-Clause | 18 KB | 2.1 KB |
| K054539 | Konami | `k054539.c` | BSD-3-Clause | 18 KB | 3.4 KB |
| K007232 | Konami | `k007232.c` | BSD-3-Clause（SPDX） | 10 KB | 1.1 KB |
| K051649（SCC） | Konami 舊街機 | `k051649.c` | BSD-3-Clause | — | 1.1 KB |
| C140 | Namco System 2 | `c140.c` | BSD-3-Clause | 11 KB | 1.3 KB |
| C219 | Namco NA-1／2 | `c219.c` | BSD-3-Clause | 12 KB | 1.3 KB |
| C352 | Namco System 21／22 | `c352.c` | BSD-3-Clause | 11 KB | 1.8 KB |
| QSound | CPS-2 | `qsound_mame.c`（HLE）；`qsound_ctr.c`（超高精度 DSP，檔頭無授權行） | `qsound_mame`：BSD-3-Clause；`qsound_ctr`：**未標示** | 10 KB；25 KB | 1.3 KB；6.1 KB |
| RF5C68／RF5C164 | Sega System 32、Mega-CD | `rf5c68.c`（BSD-3）；**`scd_pcm.c` 是 Gens 的 RF5C164（GPL）不要用** | BSD-3-Clause | 9 KB | 1.0 KB |
| YMZ280B | Capcom、Konami 等 | `ymz280b.c` | BSD-3-Clause | 29 KB | 4.6 KB |
| MultiPCM | Sega Model 1／2、System 32 | `multipcm.c` | BSD-3-Clause | 26 KB | 5.2 KB；**bss 38.9 KB（靜態可變表）** |
| MSM6258 | X68000 | `okim6258.c` | BSD-3-Clause | 17 KB | 2.0 KB；bss 3.1 KB |
| MSM5205 | Taito、Irem、Capcom 舊板 | `msm5205.c` | BSD-3-Clause | 13 KB | 2.2 KB；bss 3.1 KB |
| uPD7759 | Sega System 16 部分板子 | `upd7759.c` | BSD-3-Clause | 31 KB | 3.3 KB |
| IremGA20 | Irem M92 | `iremga20.c` | BSD-3-Clause | 10 KB | 1.1 KB |
| X1-010 | Seta | `x1_010.c` | BSD-3-Clause | 13 KB | 1.5 KB |
| ES5503 | Ensoniq | `es5503.c` | BSD-3-Clause | 17 KB | 2.2 KB |
| SCSP | Sega Saturn／Model 2 | `scsp.c`（BSD-3） | BSD-3-Clause | 35 KB | 7.8 KB；bss 24.6 KB |
| ES5505／6（Taito F3） | — | `es5506.c` 只有 38 行空殼 | — | **沒有可用實作** | — |
| AY8910／YM2149 | 街機 AY、PSG 系統 | ymfm 的 `ymfm_ssg`（YM2149，隨 OPN 附帶，已含）；另有 `ay8910.c`（BSD-3）、`emu2149.c`（Okazaki，檔頭無授權行，上游為 MIT） | BSD-3 | — | 3.3 KB（ay8910） |
| SN76489 | Master System／Genesis 與部分街機 | `sn76496.c`（MAME，BSD-3-Clause）；`sn76489.c`（Maxim，**檔頭無授權行**） | BSD-3-Clause | 26 KB | — |

**明確不能用的**（libvgm 檔頭是 GPL，或來源是 GPL 專案）：`fmopn.c`、`fmopl.c`、`ym2151.c`、`ymf262.c`（GPL-2.0+）、`ymf278b.c`（GPL-2.0）、`msm5232.c`（GPL-2.0+）、`vsu.c`（GPL）、`ymdeltat.c`（同 MAME fm 譜系，檔頭無授權行，視同 GPL）、`ym2612.c` 與 `scd_pcm.c`（Gens，GPL）。另外，**libvgm 的播放器程式（`vgmplayer.cpp`、`s98player.cpp`）與 vgmplay 本身我只當格式參考，不複製**（授權未逐檔核對，且我們要自己的簡單播放器）。LGPL 的 Nuked-OPM／OPN2／OPL3 與本專案相容，但比 ymfm 重得多，不採用。

- 註（CPU）：取樣晶片的 CPU **沒有量**（它們的暫存器流要真實 ROM 資料才有意義，我沒有）。從原始碼看每秒運算量：OKI 4 聲部（約 7.5–32 kHz）、Sega PCM 16 聲部（約 31 kHz）、MultiPCM／K054539／C352 24–32 聲部是整數迴圈，估計每個都在 YM2151 的 5 % 以下，QSound 的 DSP（`qsound_ctr`）與 SCSP 是例外（較重）。**標為猜測，板上或 PC 上要實測。**
- 把 libvgm 的核心搬出來要寫一層轉接：這些檔案是 C，依賴 libvgm 的 `stdtype.h`、`EmuStructs.h`、`snddef.h`、`EmuHelper.h`、`logging.h`，介面是 `DEV_DEF` 函式表（`Start`／`Update`／`Write` 位址 8 位元資料 8 位元／`WriteBlock` ROM）。每顆約 30–60 行轉接＋把那幾個 header 縮成約 100 行的相容層。或者直接取 MAME 現行的 `.cpp`，但那是 C++ 且綁 MAME 的 `device_t`，更費工。
- **未確定的授權風險**：QSound 與 YM2608 節奏 ROM 等「由晶片內部 ROM 推導的表格」（libvgm `fmopn_2608rom.h` 檔頭寫「這份資料是由晶片輸出推導的，內部 ROM 無法讀取」）；表格本身的著作權狀態不明（見 §3.3 與 §10）。

### 3.3 YM2608 節奏 ROM

YM2608 的 6 個節奏聲部（ADPCM-A）用晶片內建 8 KB ROM，**不在 VGM／S98 檔案裡**。ymfm 的範例播放器是從外部檔 `ym2608_adpcm_rom.bin` 讀入。libvgm 內含一份 8 KB 的重建資料（在 GPL 的 `fmopn_2608rom.h` 裡，來源是錄製晶片輸出）。我不能判斷這份資料的著作權；**建議作法**：不內嵌，改成「卡上放一個選配檔 `ym2608_adpcm_rom.bin`，有就用、沒有就節奏聲部靜音並在狀態顯示」。PC-98 的曲子多數大量用節奏，所以缺少時聽起來會缺鼓。

## 4. PC 量測：每秒音訊的成本（單執行緒，-O2）

表格說明：時間是 30 秒音訊的平均，取 5 次最小值，綁核；指令數是 callgrind 差值法。「原生取樣率」是 ymfm 依時脈算出的輸出率（`sample_rate(clock)`）。**OPN 家族的 ymfm 預設輸出率（MAX 擬真度）高得離譜（YM2203 約 1 MHz），務必設成 MIN 或 MED**，見下一張表。

### 4.1 每晶片（`fidelity = MIN`；YM2151 與 YM2612 沒有這個設定）

| 晶片（測試時脈） | 原生輸出率 | dense：ms／秒音訊 | dense：M 指令／秒 | typical：ms／秒 | typical：M 指令／秒 |
|---|---|---|---|---|---|
| YM2151（3.579545 MHz，8 聲部，LFO 開） | 55,930 Hz（時脈／64） | 10.68 | 258.4 | 10.27 | 247.5 |
| YM2612（7.670454 MHz，6 聲部；ymfm） | 53,267 Hz | 9.43 | 215.1 | 8.89 | 206.8 |
| YM2203（3.9936 MHz，3 FM + 3 SSG） | **166,400 Hz**（FM 55,467 Hz 重複 3 次） | 5.36 | 119.2 | 5.11 | 114.7 |
| YM2608（7.9872 MHz，6 FM + 3 SSG + 節奏 + ADPCM-B） | **166,400 Hz**（FM 55,467 Hz） | 11.33 | 266.3 | 11.05 | 259.0 |
| YM2610（8 MHz，4 FM + 3 SSG + ADPCM-A 6 路 + ADPCM-B） | 55,555 Hz | 9.89 | 220.9 | 9.51 | 215.2 |
| YM2610B（6 FM） | 55,555 Hz | 13.07 | 296.1 | 12.87 | 287.0 |

### 4.2 OPN 家族的擬真度設定（唯一的「降低擬真度」旋鈕；OPM／OPN2 沒有）

`opn_fidelity`：MAX＝時脈／4（YM2203、YM2608）或時脈／16（YM2610）；MIN＝FM 最快速率（時脈／24 或 /144）；MED＝介於中間。差別在輸出率與 SSG 取樣：

| 晶片 | MIN 率 → ms／M 指令 | MED 率 → ms／M 指令 | MAX 率 → ms／M 指令 | MAX 相對 MIN |
|---|---|---|---|---|
| YM2203（dense） | 166.4k → 5.36 ／ 119.2 | 332.8k → 5.43 ／ 131.4 | **998.4k** → 6.95 ／ 145.0 | +30 % 時間、+22 % 指令 |
| YM2608（dense） | 166.4k → 11.33 ／ 266.3 | 332.8k → 11.44 ／ 277.2 | **998.4k** → 12.93 ／ 301.9 | +14 %、+13 % |
| YM2610（dense） | 55.6k → 9.89 ／ 220.9 | 55.6k → 9.89 ／ 220.9 | 500k → 10.14 ／ 239.4 | +3 %、+8 % |

結論：**MIN 就夠**（FM 部分在三種設定下完全相同，只有 SSG 的取樣方式不同）。MAX 在 YM2203 會產生 1 MHz 的輸出，後面不可能重取樣，絕對不要用。

### 4.3 成本拆解（dense，MIN）

| 變化 | ms／秒 | M 指令／秒 | 說明 |
|---|---|---|---|
| YM2610 完整 | 9.89 | 220.9 | 4 FM + SSG + ADPCM-A 6 路 + ADPCM-B |
| YM2610 沒有 ADPCM 觸發 | 9.36 | 213.2 | ADPCM 兩個引擎合計只多約 **3.6 %**（指令）、5.6 %（時間） |
| YM2608 完整 | 11.33 | 266.3 | |
| YM2608 沒有節奏／ADPCM-B 觸發 | 11.24 | 264.4 | 多 0.7 % |
| YM2203 完整 | 5.36 | 119.2 | |
| YM2203 沒有 SSG 寫入 | 5.52 | 118.1 | **SSG 幾乎不花成本**（在 MIN 擬真度下） |
| YM2151 完整（LFO 開） | 10.68 | 258.4 | |
| YM2151 LFO 關（PMS／AMS = 0） | 8.22 | 181 | LFO 佔 **約 30 %**（`compute_phase_step` 每取樣每運算元重算） |
| YM2151 全部靜音（只剩設定寫入） | 4.43 | 122 | **安靜時仍有滿載的 47 %**：ymfm 不會跳過靜音聲道。需要像 GME patch 0011（emu2413 的 idle-skip）那樣自己在外層做 |
| YM2151 熱點（callgrind） | — | — | `fm_channel::output_4op` 28 %、`opm_registers::compute_phase_step` 28 %、`fm_operator::compute_volume` 21 %、`ym2151::generate` 21 % |

### 4.4 `-O2` 與 `-Os`

| | -O2 | -Os |
|---|---|---|
| YM2151 dense | 10.7 ms、258 M 指令 | 18.3 ms（**+71 %**）、272 M 指令（+5 %） |
| YM2610 dense | 9.6 ms | 12.9 ms（+34 %） |
| RV32 程式碼（OPM + OPN + OPNA + OPNB） | 15,254 B | 13,374 B（−12 %） |

x86 上 `-Os` 時間大幅變慢但指令數幾乎不變（IPC 下降）；RISC-V 上不知道是否同樣，**但程式碼只省 2 KB，建議這幾個檔案一律用 `-O2`**（`set_source_files_properties`）。

### 4.5 真實 Mega Drive 檔案校準（YM2612，ymfm 對 GME）

| 檔案 | ymfm YM2612：ms／M 指令 | GME 整體（含 SN76489 與重取樣）：ms／M 指令 | 時間比 | 指令比 |
|---|---|---|---|---|
| ellsworth-all-by-myself.vgm | 6.29 ／ 141 | 1.48 ／ 39.2 | 4.25 | 3.6 |
| Sonic&Knuckles 03 Mushroom Hill 1 | 4.59 ／ 89.1 | 0.87 ／ — | 5.3 | — |
| 14 Doomsday | 5.66 ／ 131.7 | 1.28 ／ — | 4.4 | — |
| 17 Special Stage | 6.31 ／ 141.9 | 2.08 ／ — | 3.0 | — |
| 合成 ym2612_dense（同一個流） | 9.43 ／ 215.1 | 1.92 ／ 46 | 4.9 | 4.7 |
| 合成 ym2612_typical | 8.89 ／ 206.8 | 1.93 ／ — | 4.6 | — |

重點：(1) **ymfm 約為 GME 的 3–5 倍**；而 GME 的 GENS 核心是以 44.1 kHz（`oversample_factor = 1.0`）跑的，ymfm 在原生 53.3 kHz，所以 ymfm 每個取樣的成本差距更大。(2) **真實曲子的 ymfm 成本只有合成 dense 的 0.41–0.66 倍**（指令：89／215、132／215、142／215、141／215），因為真曲有休止、聲部不是每刻都在、第 6 聲部常拿來做 DAC 鼓。合成流因此是上界，不是典型值。(3) 暫存器流量（ellsworth 約每秒 35 次寫入，Sonic 每秒約 7,000 次含 DAC）對 ymfm 成本影響很小，成本主要隨「作用中的聲部數」走。

## 5. 重取樣：需要什麼比例

韌體的 `pc_rsq`（68 階、256 相位多相窗化 sinc，比例限 1/4…4，**成本隨輸出取樣數，不隨輸入取樣數**）可以直接用。比例：

| 晶片 | 原生輸出率 | → 48 kHz | → 44.1 kHz | → 24 kHz（LC3 24k 模式） |
|---|---|---|---|---|
| YM2151 | 55,930 | 1.165 | 1.268 | 2.33 |
| YM2612（ymfm） | 53,267 | 1.110 | 1.208 | 2.22 |
| YM2203／YM2608（MIN） | 166,400 | 3.467 | 3.773 | **6.93（超過 4，pc_rsq 會拒絕）** |
| YM2610 | 55,555 | 1.157 | 1.260 | 2.31 |

建議：OPN 家族的 MIN 輸出由後端自己先 3:1 降到 FM 原生率（55,467 Hz，FM 部分本來就是重複 3 次，SSG 取 3 取樣平均），`rate()` 回報 55,467，這樣所有比例都在 pc_rsq 的範圍內。

**pc_rsq 在 PC 上的成本** [PC]：55,930→48,000 約 3.5 ms／秒、52 M 指令／秒（166,400→48,000：3.5 ms、52 M；48,000→44,100：3.2 ms、48 M），相當於 **YM2203 的 44 %**，是 YM2612 GME 整體的 2.4 倍。換算到板上約 18 %（指令法）到 32 %（時間法）——**這是個大項**，板上沒有它的獨立量測（FLAC→藍牙 AAC 的 45–49 % 裡混著 AAC 編碼）。如果實測偏高，後端可以自帶 16–24 階的便宜多相濾波器（約為 pc_rsq 的 1/3–1/4 成本；FM 晶片原本的輸出就有 DAC 鏡像，不需要 pc_rsq 的 75 dB 抑制）。

## 6. 記憶體（PSRAM 問題）

### 6.1 晶片物件大小

`sizeof`：RV32 的數值是用韌體編譯器（`-march=rv32imafc`，指標 4 B）編譯時的 `sizeof` 得到；x86-64 的數值是 PC 實測（指標 8 B）。動態配置：x86 用全域 `operator new` 計數得到次數與位元組數；RV32 的位元組數＝次數×型別大小（沒算 malloc 的每塊約 8 B 管理成本）。

| 類別 | sizeof RV32 [RV] | sizeof x86-64 [PC] | 建構時動態配置（次數；RV32 位元組） | 合計 RV32（不含 malloc 管理） |
|---|---|---|---|---|
| `ym2151` | 4,564 | 4,736 | 40 次（32 運算元×68 + 8 聲道×36）＝2,464 B | **≈ 7.0 KB** |
| `ym2203` | 2,504 | 2,600 | 15 次（12×68 + 3×36）＝924 B | ≈ 3.4 KB |
| `ym2608` | 2,932 | 3,128 | 37 次（24×68 + 6×36 + ADPCM 聲道 6×40 + 48）＝2,136 B | ≈ 5.1 KB |
| `ym2610`／`ym2610b` | 2,932 | 3,128 | 同 ym2608 | ≈ 5.1 KB |
| `ym2612` | 2,728 | 2,856 | 30 次（24×68 + 6×36）＝1,848 B | ≈ 4.6 KB |
| `ym3812`／`ymf262` | 8,604／17,160 | 8,728／17,392 | 27／54 次（x86：2,016／4,032 B） | ≈ 10／21 KB |
| `ym2149`（單獨 SSG） | 68 | 80 | 0 | 68 B |

雙晶片（時脈欄位的第 30 位元，例如 2×YM2203、2×YM2151）物件數加倍。

### 6.2 什麼放哪裡

- **靜態常數表放 flash**：ymfm 沒有可變靜態表（`.data = 0`、`.bss` 只有晶片物件本身；我的連結結果 [RV]）。唯讀表：YM2151 約 4.0 KB（相位步進表 3 KB + 指數表 0.5 KB + 增量表 0.25 KB…）、OPN 家族約 1.0–1.2 KB。**函式內的 `static` 表都是 `const`（弱符號 `V`，在 .rodata），沒有保護變數**。
- **晶片物件 + 動態配置（OPM 約 7 KB、OPNA／OPNB 約 5 KB）建議放內部 RAM**：每取樣都要碰全部運算元狀態，放 PSRAM 會因為快取未命中變慢（板上沒量；沒有可用的 PSRAM 延遲數字，這是 §9 要量的項目之一）。內部 RAM 可用約 99 KB（最大連續區塊 63 KB，docs/measurements.md），7 KB 可接受；雙 YM2151 約 14 KB，建議上限。場景（26 KB）是目前最重的單項。做法：用 placement new 把物件放進 `heap_caps_malloc(…, MALLOC_CAP_INTERNAL)` 的緩衝，ymfm 內部的 `make_unique` 走全域 `new`（IDF 預設：小塊落在內部 RAM），總量小所以不需要改全域 `new`。
- **取樣資料放 PSRAM，且「原地參照」**：VGM 檔本身已被整個讀進 PSRAM（`player.c` 上限 8 MB，`.vgz` 用 `pc_gunzip` 展開）；資料區塊（0x82／0x83／0x81 等）只記錄 `(起點, 長度, 目標偏移)` 的片段表，`ymfm_external_read` 去片段表找指標；一般只有 1–2 個連續區塊，查表可以用「目前片段」快取，ADPCM 本來就是循序讀取。**不複製**。例外：YM2608 的 Delta-T 是可寫 RAM（256 KB，PSRAM 另配）；S98 檔沒有資料區塊，ADPCM-B 資料是透過暫存器寫進晶片 RAM，所以 S98 也需要這 256 KB。
- **Neo Geo 的大小**：典型 YM2610 VGM 的 ADPCM-A ROM 資料區塊數 MB（我的合成檔 2 MB + 0.5 MB）；真實 Neo Geo 檔可到 8–16 MB。`player.c` 的 8 MB 上限是檢查**壓縮後**檔案大小（`sz > 8*1024*1024`），`.vgz` 展開後的大小沒有檢查，而且展開時**壓縮檔與展開檔同時在 PSRAM**（約 15 MB 可用）：一個 5 MB 的 .vgz 展開成 14 MB 時會超過。需要串流解壓或限制。

## 7. 程式碼大小（彙整）

| | .text | .rodata | .bss／晶片物件 | 來源 |
|---|---|---|---|---|
| ymfm FM 四家族（OPM／OPN／OPNA／OPNB），RV32，-O2 | 15.3 KB | 4.3 KB | 12.9 KB | [RV] |
| 同上 -Os | 13.4 KB | 4.3 KB | 12.9 KB | [RV] |
| 個別：YM2151／2203／2608／2610 | 3.3／4.9／7.9／5.6 KB | 4.0／1.0／1.2／1.1 KB | 4.6／2.5／2.9／2.9 KB | [RV] |
| x86-64 -O2 物件（連結前，含未用函式）：opm／opn／adpcm／ssg | 19.1／41.5／7.3／4.1 KB | — | — | [PC] |
| 取樣晶片核心（每顆） | 0.9–6 KB（QSound 精準版 6.1 KB、SCSP 7.8 KB） | 含在 text | MultiPCM 38.9 KB、SCSP 24.6 KB、其餘 0–3 KB | [RV] |
| 估算整體（FM 四家族 + 8 顆常用取樣核心 + VGM／S98 剖析 + 轉接層） | **約 35–50 KB** | | | [估] |

與現有的相比：GME 整包（`components/gme`）約為這個的數倍；`ORIGIN.md` 的 AdPlug 約 290 KB。所以「+100–200 KB」不成立。

## 8. 設計

### 8.1 元件

- 名稱建議：`components/libfmvgm`（沿用 `libadplug`／`libhvl`／`libcrsid` 的命名；若主人偏好 `fmplay` 等再改）。
- 內容：
  - `ymfm/`（原樣複製：`LICENSE`＋上述 `.cpp/.h/.ipp`，記錄 commit 到 `SOURCE.md`；若需要補丁，放 `patches/`，沿用 `components/*/SOURCE.md` + `patches/` 的慣例）
  - `pcm/`（挑出的取樣核心，附轉接層，各自保留檔頭授權）
  - `src/fmvgm_vgm.cpp`（VGM 剖析與排程）、`src/fmvgm_s98.cpp`、`src/fmvgm_mix.cpp`（混音、降取樣）、`src/fmvgm_backend.cpp`（`pc_backend_t`）
  - `include/fmvgm.h`
  - `CMakeLists.txt`（`idf_component_register`；`-DNDEBUG`、`-w`、對 ymfm 檔案 `-O2`）、`SOURCE.md`、`Kconfig` 可選（晶片開關，節省程式碼）
- **只依賴**：`pc/pc_backend.h`（`pc_backend_t`）、`pc/pc_io.h`（`pc_reader_t`），以及可選的 `pc/pc_voice.h`。**不依賴**：Nano 的 `player.c`、`settings`、`cmd`、FreeRTOS、`esp_log`、`heap_caps`（配置透過一個 `fmvgm_alloc_hooks`，預設 `malloc`，Nano／JukeDial 各自傳 `heap_caps_malloc` 包裝）。這樣 JukeDial 只需要複製一個目錄（見 §8.9）。

### 8.2 公開介面

```c
extern const pc_backend_t pc_backend_fmvgm;     /* .vgm .vgz（已解壓的記憶體）.s98；name = "FM" */
typedef struct { const char *title, *game, *system, *author, *date, *notes; /* UTF-8，指向 info 內的緩衝 */ } fmvgm_tags_t;
/* 打開前判斷：0 本後端完整支援；1 只有 GME 支援的晶片（交給 GME）；2 本後端部分支援（缺晶片，見 *missing 位元遮罩）；-1 都不能播／壞檔 */
int  fmvgm_route(const void *mem, uint32_t size, uint32_t *chips_used, uint32_t *chips_missing);
int  fmvgm_probe(const void *mem, uint32_t size, fmvgm_tags_t *tags, char *buf, size_t cap, int32_t *length_ms);
void fmvgm_set_alloc(void *(*alloc)(size_t, int internal), void (*free_)(void *));   /* 預設 malloc／free */
```

`pc_backend_t` 欄位：`open`（要求 `r->mem.p`：整個檔案在記憶體；剖析標頭、建晶片、建片段表）；`render`（見 8.5）；`rate`（主晶片原生率，8.6）；`track_count`／`start_track`（VGM 一首；S98 一首；先不分曲）；`length_ms`（見 8.4）；`position_ms`；`close`；`byte_pos = NULL`（不能續播）；`name = "FM"`；`bits = NULL`；`burst = NULL`。

### 8.3 與 GME 共存：誰播哪個檔

**規則：GME 拿它能播的，新後端拿標頭裡有 GME 沒有的晶片的。** 判斷看標頭的時脈欄位（VGM 1.71 規格的偏移；有效位數取決於 `data_offset`，v1.50 以前標頭只有 0x40 位元組），遮掉第 30 位元（雙晶片）與第 31 位元（YM2610B／部分晶片的變體旗標）後非零就算有：

| 標頭偏移 | 晶片 | GME | fmvgm |
|---|---|---|---|
| 0x0C／0x10／0x2C | SN76489／YM2413／YM2612 | **能播** | 能播（SN76496 核心、ymfm YM2413／YM2612） |
| 0x30 | YM2151 | 無聲 | ymfm |
| 0x44／0x48／0x4C | YM2203／YM2608／YM2610(B) | 無聲 | ymfm |
| 0x74 | AY8910 | 無聲 | ymfm SSG |
| 0x38／0x98／0xA0／0xAC／0xA8／0xB4 等 | Sega PCM／OKI6295／K054539／K053260／C140／QSound | 無聲 | 依實作的取樣核心 |
| 其他（GB、NES、HuC6280、WonderSwan…） | | 無聲 | 先不支援 |

路由：
1. 遮罩 ⊆ {SN76489, YM2413, YM2612}（含「完全沒有晶片」）→ **GME**（便宜 3–5 倍，已接上聲部指示與靜音）。
2. 遮罩含有 fmvgm 支援的晶片且沒有不支援的 → **fmvgm**。
3. 兩個後端都缺某晶片（例如 Mega-CD 的 RF5C164、32X 的 PWM、Game Boy）→ 回傳 2（部分支援）：由「能播較多晶片的」後端播，狀態顯示 `partial`，**這是主人要決定的策略**（§10）。GME 目前對這類檔案是靜默地丟掉不認得的晶片，聲音缺一塊但不會報錯。
4. **同時有 GME 晶片與新晶片（YM2151 + SN76489；YM2612 + YM2151）**：整個檔案給 fmvgm，由它自己的 SN76496／ymfm YM2612 負責；**一個檔案只能由一個後端播**（兩個後端不能各播一半，因為時間軸與混音是一體的）。代價：這類檔案的 YM2612 部分比 GME 貴 3–5 倍，無法避免。
5. **YM2612 的 DAC 串流命令（0x90–0x95）**：GME 沒處理（`Vgm_Emu_Impl` 對 0x9x 走預設分支，長度當 1，可能脫序）。主人說這是 GME 的領域，所以本步驟不實作；但**路由時可選擇掃描命令流，遇到 0x90–0x95 就改給 fmvgm（日後實作串流）**，單純 YM2612 的 v1.50+ 檔案才不會壞。列為待決定。

接線：`player.c` 的 `backend_for()` 現在只看副檔名，把 `.vgm/.vgz` 直接給 `backend_gme`。最小改動是新增一個「VGM 路由後端」`pc_backend_vgm`（仿 `pc_backend_chain`：`open()` 看標頭呼叫 `fmvgm_route()`，再委派給 `backend_gme` 或 `pc_backend_fmvgm`，`pc_backend_resolve()` 回報真正的後端）。路由器放在 app 層（因為 `backend_gme` 在 app 裡），元件本身只提供 `fmvgm_route()`；JukeDial 複製元件後寫同樣的 30 行路由器。`.s98` 直接對應 fmvgm。`player.c` 的 `be == &backend_gme || …` 載入分支（整個檔案讀進 PSRAM）要把新後端加進去。

### 8.4 VGM 功能範圍

- **迴圈與淡出**：與 GME 一致，讓使用者換後端時行為不變：總長 = 前奏 + 2 × 迴圈（GME：`play_length = intro + 2 * loop`，沒有迴圈就是整首）、沒有長度資訊時 180 秒（`pc_chip_length_ms(-1)`）、最後 8 秒線性淡出（GME 的預設，`set_fade`）。迴圈資訊來自標頭 0x1C（迴圈偏移）與 0x20（迴圈取樣數）、v1.60 的迴圈次數修飾（0x7F）可忽略或當作加分項。淡出在 float 輸出上做增益斜坡。
- **GD3 標籤**：標頭 0x14（相對偏移）→ `"Gd3 "` 區塊，版本、長度，其後 11 個 UTF-16LE 字串（曲名英／日、遊戲英／日、系統英／日、作者英／日、日期、轉檔者、備註）。優先英文欄位，空的用日文欄位；UTF-16 轉 UTF-8（含代理對）。GME 的 `parse_gd3` 只取英文欄位。檢查長度（GME 的 `check_gd3_header` 作法：區塊長度 < 剩餘位元組、< 0x200 的版本檢查）。
- **雙晶片位元**：時脈欄位的 0x40000000 → 建第二個實例；`0xA1…0xAF` 命令是第二顆（命令減 0x50 加 0xA0 的規則；0x52→0xA2 等）。ymfm 側與命令表都要處理。
- **資料區塊**：0x67 `0x66 tt ss ss ss ss`；0x00–0x3F 是未壓縮資料串流（YM2612 DAC 等，現階段忽略）、0x80–0xBF 是 ROM 影像（`4 B 總大小 + 4 B 起始 + 資料`）、0xC0+ 是 RAM 寫入。**類型 0x40–0x7E 是壓縮串流（bit packing／DPCM），不支援，遇到則記為缺功能**。
- **等待、跳過**：0x61／62／63／7n／0x8n（YM2612 DAC）；未知命令要用規格的長度表跳過（0x30–0x3F：1 位元組、0x40–0x4E：2、0x4F：1、0x50–0x5F：2、0xA0–0xBF：2、0xC0–0xDF：3、0xE1–0xFF：4、0x90–0x95：4／4／5／10／1／4）。每個命令先檢查剩餘位元組（GME 的 FIX7）。
- **時脈驗證**：每種晶片接受的時脈範圍（例如 YM2151 與 OPN 大致 1–8 MHz；超出者拒絕並說原因），因為 ymfm 的取樣數與時脈成正比。

### 8.5 S98 剖析範圍

格式事實（對照 libvgm `s98player.cpp` 的讀法確認，只當格式參考，程式自己寫）：

- 標頭：`"S98"` + 版本字元 `'0'`–`'3'`；0x04 計時分子、0x08 計時分母（為 0 時用 10／1000，即 10 ms 一個 tick；V0 的分子視為 0）、0x0C 壓縮（保留，必為 0）、0x10 標籤偏移、0x14 資料偏移、0x18 迴圈偏移（絕對位置，0＝無迴圈）。
- 裝置表：V0／V1 沒有表，預設一顆 **YM2608，7,987,200 Hz**；V2 從 0x20 起每項 16 位元組（類型 4 B、時脈 4 B、…），以類型 0 結尾；V3 在 0x1C 有裝置數，0x20 起每項 16 位元組（類型、時脈、**聲像**、保留）。類型：1 YM2149、2 YM2203、3 YM2612、4 YM2608、5 YM2151、6 YM2413、7 YM3526、8 YM3812、9 YMF262、15 AY8910、16 SN76489。
- 命令：`0xFF` 前進 1 tick、`0xFE` + 變長整數 = 前進 2 + n 個 tick、`0xFD` 結束（有迴圈則跳回迴圈偏移）；其餘位元組 `b` → 裝置 = `b >> 1`、埠 = `b & 1`，後面接 `reg`、`data` 兩位元組。
- 標籤：V1／V2 的標籤偏移指向一個以 NUL 結尾的曲名字串；`"[S98]"` 開頭則是 `key=value` 逐行（TITLE、ARTIST、GAME、YEAR、GENRE、COMMENT、COPYRIGHT、S98BY、SYSTEM，鍵不分大小寫）；UTF-8 BOM（`EF BB BF`）開頭是 UTF-8，否則是 Shift-JIS。**Shift-JIS 轉 UTF-8 需要約 20 KB 以上的 JIS X 0208 表**；先做「只顯示 ASCII／已是 UTF-8 的標籤」，日文標籤是否值得 20 KB 由主人決定。
- 裝置映射：1／15 → ymfm SSG、2 → ym2203、3 → ym2612、4 → ym2608、5 → ym2151、6 → ym2413、7／8／9 → OPL 系列（先不做）、16 → SN76496。ADPCM-B 的資料靠暫存器寫入灌進晶片 RAM（所以 YM2608 要 256 KB RAM）。
- 計時：tick 長度 = 分子／分母秒，轉成晶片取樣數用同一個 Q32 累加器（與 VGM 同一套引擎，剖析層只是命令來源不同）。

### 8.6 排程、渲染、取樣率、執行緒

- **時間基準**：以「主晶片」的原生取樣率為輸出率（YM2151 55,930／YM2612 53,267／OPN 的 FM 原生 55,467／YM2610 55,555），`rate()` 回報它。所有時間（VGM 的 44.1 kHz 等待、S98 的 tick）都換成這個率的 Q32 累加。其他晶片各用自己的原生率產生，混進主率：取樣率更低的（OKI 約 7.5–32 kHz、SN76489 的 3.58 MHz／16 約 224 kHz 是更高，用平均）用線性內插或平均到主率。
- **render(h, float *stereo, frames)**：frames 是主率的取樣數（呼叫端的區塊上限，GME 包裝是 480）。迴圈：若目前等待還沒用完，讓所有晶片產生 `min(剩餘, frames − 已產生)` 個取樣（ymfm 的 `generate(out, n)` 一次最多 512 個，棧上的小緩衝）→ 轉 float 混入輸出；否則剖析下一個命令（寫暫存器／資料區塊／設等待）。寫入時間與取樣邊界對齊到最近的晶片取樣（誤差 ≤ 0.5 取樣，與 vgmrender 一致）。結束（淡出完）回 0。
- **渲染路徑不配置**：所有晶片與緩衝在 `open()` 配好；`generate()` 本身不配置（§2.4 已量）；`render` 只用棧與 handle 內固定陣列。
- **執行緒**：與 GME 相同，由解碼任務（核心 1）呼叫，`open`／`render`／`close` 同一個執行緒。`fmvgm_probe`／`fmvgm_route` 是純函式，可以在任何任務呼叫（核心 0 的清單掃描用）。如果之後決定把渲染搬去核心 0（見 §8.8），那時再加「生產者任務 + 環」的選配層，不改這個介面。
- **增益**：每種晶片一個常數，讓不同晶片的整體響度與 GME 的同類曲子接近（用 RMS 對照調）；ymfm 輸出是 int32（YM2151／YM2612 約 ±8 k，OPN 的 SSG 另計）。

### 8.7 負載與降級

目前韌體的卸載機制只暫停「重型音效」。晶片播放沒有音效可卸，所以後端自己需要的降級手段（由便宜到貴）：(1) 靜音時跳過晶片（idle-skip，§4.3：全靜音時省約 47 %）；(2) OPM 的 LFO 快取（需要補丁，最多省約 30 %）；(3) 超載時暫停該曲而不是斷斷續續（`und`）；(4) OPN 家族的 FM 改用 GME 的 GENS 核心（見下）。

**Plan B（如果 §9 的板上量測證實太重）**：OPN 家族（2203／2608／2610）的 FM 部分是同一個運算元結構，YM2612 的 GENS 核心（已在 GME 裡，LGPL，約為 ymfm 的 1/4–1/5）可以當成 FM 引擎，再接 ymfm 的 SSG 與 ADPCM 引擎。品質不如 ymfm（GENS 不是週期精確），但 PC-88／98 與 Neo Geo 的曲子成本降到約 25 %。YM2151 沒有現成的便宜核心。這個選項要寫轉接層，列為備案，不是第一步。

### 8.8 哪些「晶片 + 輸出」組合看起來可行

總負載 ≈ 管線底（Nano 的 FLAC 48 kHz 是 12 %、待機 1 %，故約 11 點）+ 晶片 + 重取樣（pc_rsq 約 18–32 %，便宜版約 6–10 %）+ 輸出編碼。輸出編碼的點數用 docs/measurements.md：藍牙 AAC 約 +33–37 點（FLAC→AAC 45–49 % 對 FLAC→DAC 12 %）；LDAC 約半個核心；Auracast 廣播＋資料夾播放 79–90 %（比 FLAC 本身多出 65–78 點）。晶片部分用 §5/§8 的估計區間（「稠密上界～真曲下界」）。規則沿用 JukeDial 的「解碼＋管線 ≤ 60 %」。

| 晶片（核心 1，只算晶片） | USB DAC | 藍牙 AAC／SBC | LDAC | Auracast |
|---|---|---|---|---|
| YM2203：17–49 % | 可（總 35–90 %，輕曲 OK） | 邊緣（總 70–120 %；只有最輕的曲子） | 不可 | 不可 |
| YM2610：31–91 % | 邊緣（總 50–125 %） | 不可 | 不可 | 不可 |
| YM2151：37–98 % | 邊緣（總 55–135 %） | 不可 | 不可 | 不可 |
| YM2608：38–104 % | 邊緣偏重 | 不可 | 不可 | 不可 |
| YM2610B／雙晶片：42–120 % 以上 | 多半不可 | 不可 | 不可 | 不可 |

**結論：只有 USB DAC 輸出實際，而且 YM2151／YM2610／YM2608 的稠密曲子單核心裝不下。** 核心 0 平常只用 2–14 %（含 BLE），可以當作「渲染任務」的位置（環形緩衝 200 ms），把晶片整個移出核心 1；藍牙 AAC 時核心 0 另有 11–12 % 編碼負擔，仍有空間。這需要板上實測 Wi-Fi／BLE 突波對 10 ms 級排程的影響，列為 §9 第二步。

### 8.9 JukeDial 複製回去要什麼

- 複製整個 `components/libfmvgm/`（含 `SOURCE.md`、`patches/`、`LICENSE`／取樣核心的授權文字、`docs/licenses/` 需補的授權檔）；介面只用 `pc_backend.h`、`pc_io.h`（兩邊相同）。
- **要避免的依賴**：Nano 的 `player.c`／`settings`／`cmd`／`nsn_*`、FreeRTOS 任務、`esp_log`、`heap_caps_*`（改走 `fmvgm_set_alloc`）。
- JukeDial 另外需要的：路由器（約 30 行，因為它的 `backend_gme` 在 app 層）、聲部指示器（可選，用 ymfm 的 `debug_channel()`／`debug_operator()` 唯讀存取器讀包絡，另寫；Nano 目前沒有使用聲部快照，GME 包裝的 `backend_gme_voices` 在 `app/main` 內沒有被引用）、螢幕底線（螢幕開時核心 1 多 13–20 點，JukeDial 的預算比 Nano 緊，同一個晶片在 JukeDial 上更難過 60 % 線）。
- 授權：LICENSE（BSD-3 ymfm 與各取樣核心）、`THIRD_PARTY_NOTICES.md` 條目；兩邊同步方式沿用 `components/ORIGIN.md` 的「各自維護」。

## 9. GME 的其他核心要不要接到 VGM（驗證）

分析文件的說法**大致合理**：GME 的 AY、SCC、NES、Game Boy、PC Engine 核心各自綁在自己的檔案類型（`Ay_Emu`、`Kss_Emu`、`Nsf_Emu`、`Gbs_Emu`、`Hes_Emu`）裡，接到 `Vgm_Emu` 需要修改被我們 vendor 的 GME（`Vgm_Emu::header_t` 只有 0x40 位元組、沒有對應的標頭欄位與命令），而且這些機器都有原生格式（NSF、GBS、HES、KSS、AY），VGM 版本沒有增加內容。**兩個例外值得注意**：(1) 街機的 AY8910（Capcom／Konami 的許多板）與 K051649（SCC）、Pokey 沒有原生格式，只存在於 VGM；這些在 fmvgm 裡幾乎免費——AY 就是 ymfm 隨 OPN 附帶的 `ymfm_ssg`（YM2149，已連結），SCC 是 libvgm 的 `k051649.c`（BSD-3，約 1.1 KB）；(2) 因此「不接 GME 的核心」是對的，但「這些晶片的 VGM 不能播」不是結論，它們進新後端。

## 10. 建議、順序、板上先量什麼、開放問題

### 10.1 Go／No-go 與順序

| 順序 | 項目 | 決定 | 理由 |
|---|---|---|---|
| 0 | 路由器（`fmvgm_route`、`pc_backend_vgm`） | **Go（立刻）** | 現在這些檔案「打得開卻無聲」，這是已存在的缺陷；至少要顯示「不支援 YM2151／…」 |
| 1 | YM2203 + SSG（含 VGM／S98 剖析、GD3、迴圈、淡出、重取樣） | **Go** | 最便宜（約 17–49 %）、代碼最小，驗證整個骨架；覆蓋 PC-88／98 的 OPN 檔、Capcom 世代 |
| 2 | YM2151 + OKI6295 | **Go（有條件：先板上量）** | 街機價值最高（Sega／CPS-1／Konami／Namco／X68000）；但稠密曲子約 90 % 核心 1。先做 idle-skip 與 LFO 快取，目標 ≤ 60 % |
| 3 | YM2610（ADPCM-A／B） | **Go（板上量過再定）** | Neo Geo／Taito；ADPCM 只多 3.6 %，成本主要是 4 聲部 FM；要處理多 MB 資料與 .vgz 展開 |
| 4 | YM2608（含節奏／ADPCM-B） | **有條件** | 6 聲部 FM 稠密約 92 %；節奏 ROM 授權問題；PC-98 世代檔案也能由 S98 取得 |
| 5 | YM2610B、雙晶片、多晶片混合 | **暫不** | 成本超過單核心預算；除非 Plan B（GENS）或核心 0 渲染驗證可行 |
| 6 | S98（V0–V3） | **Go（接在 1 之後）** | 剖析器小（約 250 行），一旦有 OPN 就免費增加 PC-88／98／X1／MSX 的大量檔案 |
| 7 | 其餘取樣核心 | **依主人選的晶片再做** | 轉接層各約 30–60 行；授權乾淨（BSD-3）；QSound／YMZ280B／K054539 較重且 ROM 資料量大，放最後 |
| — | OPL（YM3812／YMF262 的 VGM）、OPL4 | **不做** | 韌體已有 AdPlug 的 OPL 路線，且 YMF262 物件 17 KB |
| — | 把 GME 其他核心接到 VGM | **不做** | 見 §9 |

### 10.2 板上先量什麼（精確步驟）

**A. 現在就能量、不需要寫程式（重取樣的真實成本）。**
1. 用 ffmpeg 產生兩個 WAV：`ffmpeg -f lavfi -i "anoisesrc=d=30:c=pink:r=55930" -ac 2 -c:a pcm_s16le r55930.wav` 與 `... r=48000 ...`，放在卡上 `/sdcard/nsn_fmt/`。
2. 在一次序列連線中（開埠會重置板子，所以整個流程要在同一個連線內）：`out dac`、`play /sdcard/nsn_fmt/r48000.wav`，隔 2 秒讀三次 `st`，取 `cpu` 的第二個數（核心 1）與 `und`；再 `play /sdcard/nsn_fmt/r55930.wav` 一樣量。**兩者的差 = pc_rsq（55.9k→DAC 的轉換）的實際成本。** 預估 18–32 點，若小於 10 點則不需要便宜版重取樣器。
3. 同樣做 `r166400.wav`（比例 3.47）。

**B. 原型後端完成後（步驟 2）。**
1. 把 `bench/syn/` 的合成檔放到 `/sdcard/nsn_fmt/`：`ym2203_typical.vgm`、`ym2203_dense.vgm`、`ym2151_typical.vgm`、`ym2151_dense.vgm`、`ym2610_typical.vgm`。
2. `play <檔>`，音量不重要（可 `vol 0`），10 秒窗讀 `st`：`cpu[1]`（核心 1 %）、`und`（斷音）；與同一次開機播 `ellsworth-all-by-myself.vgm`（GME，JukeDial 板上 13.6 %）當錨點。**用「合成 dense／typical 相對 GME」的比例驗證我的換算係數（0.347 %／M 指令）。**
3. 加一個 `fmbench`（像 JukeDial 的 `chipbench`）：直接呼叫 `render` 10 秒、印 µs／10 ms 與 `sum=`（左右聲道 int 累加）。PC 端 `bench` 也印 `sumL／sumR`；ymfm 是純整數，所以 **-O2 與 -Os、PC 與板子的 `sumL／sumR` 必須完全相同**（已驗：x86 的 -O2 與 -Os 相同），不同就是編譯器或平台問題。
4. 量「晶片物件放內部 RAM vs PSRAM」的差：同一個檔案，對照 `heap_caps_malloc(MALLOC_CAP_INTERNAL)` 與 `MALLOC_CAP_SPIRAM`。
5. 核心 0 渲染的可行性：生產者任務固定在核心 0，環 200 ms，量 Wi-Fi 連線中的 `blk_max` 與 `und`。

### 10.3 開放問題給主人

1. **取樣晶片的優先順序**：先做 OKI6295（CPS-1／Toaplan／Seibu）和 Sega PCM，還是要 Konami／Namco／QSound？每多一顆約 1–6 KB、30–60 行轉接層，但需要真實檔案驗證。
2. **測試檔**：我沒有真實的 YM2151／OPN／OPNB／取樣晶片 VGM（vgmrips 網站回 410、沒確認它的條款）。能否由主人提供每個家族 3–5 個檔案做私人測試？檔案能不能放進 repo 的測試資料？我的合成產生器（`gen_vgm.py`，無第三方內容）本身可以放進 repo。
3. **命名**：`libfmvgm` 可以嗎？路由後端叫 `pc_backend_vgm`？
4. **部分支援策略**：檔案含有兩個後端都沒有的晶片（Mega-CD 的 RF5C164、32X 的 PWM）時，「播能播的部分並標記 partial」還是「拒絕並說明原因」？目前 GME 是無聲丟掉那一塊。
5. **YM2608 節奏 ROM**：接受「卡上選配檔 `ym2608_adpcm_rom.bin`」嗎？還是願意內嵌一份重建資料（授權不明）？QSound 的 DSP 表同樣有來源問題。
6. **ymfm 要不要打補丁**（LFO 相位步進快取約省 20–30 %、靜音聲道跳過）？打補丁的代價是和上游分離（像 GME 的 patch 系列）；不打則成本留在 §8.8 表的上緣。
7. **預算規則**：沿用「解碼＋管線 ≤ 60 %」嗎？若是，YM2151／YM2610 的稠密曲子要靠核心 0 渲染或 Plan B。
8. **S98 的日文標籤**：要不要為 Shift-JIS 付 20 KB 以上的轉換表？
9. **YM2612 DAC 串流命令**（0x90–0x95，GME 會脫序）：要不要讓 fmvgm 接手這類檔案？
10. **`.vgz` 與大檔**：Neo Geo 的檔案展開後可超過 PSRAM 餘量；要不要做串流解壓或明確的上限與錯誤訊息？

## 11. 這次沒能確定的事

- 板上實際成本：全是估計（從 JukeDial 的 GME YM2612 板上量測 13.6 % 校準，誤差約 ±35 %；兩種換算法在稠密 YM2151 給出 90 % 與 98 %）。沒有板上的 PSRAM 延遲、快取大小、RISC-V 核心的 IPC 資料。
- 真實 YM2151／OPN／OPNB 曲子的成本（用 Mega Drive 真檔的 0.41–0.66 倍外推，未驗）。
- 取樣晶片的 CPU。
- 合成檔的聽感正確性；ymfm 輸出與其他參考核心的對照沒做。
- libvgm／MAME 授權：我讀了檔頭與 MAME 主線的 `license:` 行，沒有法務等級的逐版追溯；`qsound_ctr.c`、`sn76489.c`、`emu2149.c`、`ymdeltat.c` 的檔頭沒有授權行，不要使用或另外確認。
- vgmrips 的使用條款（網站回 410）。

## 附：檔案位置與重現

scratchpad 根目錄：`/tmp/claude-1000/-home-jc-self-remote/4efacb23-5e7e-46f9-960d-229f48452003/scratchpad/`（以下相對於它；工作階段結束或重開機後可能清掉，需要保留請複製出來）

| 路徑 | 內容 |
|---|---|
| `ymfm_src/ymfm/` | ymfm 原始碼（commit `81aec25…`） |
| `libvgm_src/libvgm/` | libvgm（commit `c5c03a2…`，只讀來核對授權與格式） |
| `bench/bench.cpp`、`bench/build.sh` | ymfm 基準（VGM 剖析器 + 晶片驅動）；`./build.sh -O2 bench_O2` |
| `bench/gen_vgm.py` | 合成 VGM 產生器：`python3 -I gen_vgm.py syn` |
| `bench/measure.sh`、`bench/results.tsv` | 全部量測流程與結果（欄：標籤、建置、擬真度、原生率、ms／秒、M 指令／秒） |
| `bench/syn/*.vgm` | 合成測試檔（含 `_noadpcm`、`_nolfo`、`_idle`、`_nossg` 變體） |
| `gmebench/gmebench.cpp`、`build.sh` | 韌體 GME 在 PC 上的基準（`components/gme` 原始碼，只讀） |
| `anchor/rsq.c` | `pc_rsq` 基準；`anchor/anchor.c` FLAC／MP3 解碼錨點（結論：解碼本身很小，不能當錨點，故未採用） |
| `rv/rvbuild.sh`、`rv/allsize.sh`、`rv/sizes.cpp`、`rv/icount.sh`、`rv/pcmsize.sh`、`rv/min.ld` | RISC-V 編譯、單晶片 gc 連結大小、`sizeof` 與靜態指令數比較（RV32／x86 ≈ 1.0）、取樣核心大小 |
| `rv/sizes_rv.txt`、`rv/sizes_all.txt`、`rv/sizes_host.txt`、`rv/pcm/sizes.txt` | 上述結果 |

換算係數（§8.8 與 §0 用的）：`板上% = M 指令／秒 × 0.347`（來源：JukeDial `docs/tradeoffs.md`「VGM ellsworth 14 µs ↔ 板上 1364 µs」與「Mega Drive 13.5 %」，加上我在 PC 上量同一檔案 GME 為 1.48 ms／39.2 M 指令每秒，13.6 % ÷ 39.2 = 0.347）；時間法：`板上% = PC ms／秒 × 9.19`（13.6 % ÷ 1.48 ms）。RV32 與 x86 的靜態指令數比（opm 1.04、opn 1.03、adpcm 0.96、ssg 0.95）支持「指令數可以直接搬」。
