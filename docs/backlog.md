# NightStand Nano — 還能做什麼（候選功能總表）

整理日期：2026-11-25。依主人的原則：「Nano 只是沒有螢幕；參考專案能做、又不需要螢幕的，都該支援，便宜的優先。」另外主人問：參考專案裝置上的「小程式」能不能變成控制網頁裡的網頁版（即使跟盒子無關）。

**怎麼讀**：每列一個功能。A 是只動韌體；B 是韌體加網頁（原本靠螢幕，改由網頁顯示或操作）；C 是純網頁小程式；D 是板子沒有的硬體；E 是主人已否決或不適用（不要再提）；F 是已排入、不必再選。最後有我的建議。
- 成本：**S** 半天內、幾十行；**M** 一到三天；**L** 一週以上，或要新元件／新的量測。
- 「Nano 現況」：我先對照過 `README.md`、`docs/requirements.md`、`docs/design.md`、`docs/ideas.md`、`web/` 與 `app/main/` 才寫「沒有」；沒把握的寫「未確認」。
- 「備註」：主要風險是 **內部 RAM**（約剩 100 KB，是瓶頸）、**核心 1**（解碼＋藍牙編碼器約吃一半）、或需要新的資料。
- 來源縮寫：**R** = `ESP32S31_round`（JukeDial）；**NS** = `nightstand`；**CC** = `chipchill`；**TT** = `tunetap`；**SM** = `sleepmixer`；**JC** = `~/self/RP2350_player`（JukeChip）。`pc/xxx.h` 指 `R/components/player_core/include/pc/xxx.h`（純 C、不依賴 ESP-IDF，header 開頭註解說明用途）；`ui/xxx` 指 `R/app/main/ui/xxx`；`guide/NN` 指 `R/docs/guide/zh-TW/NN-*.md`；`features/xxx` 指 `R/docs/features/xxx.md`。
- C 節的欄位不同：功能｜來源｜盒子參與｜移植方式｜成本｜聲音與備註（理由見該節開頭）。

---

## A. 韌體：不需要螢幕就能用的

### A1 播放
| 功能 | 來源 | Nano 現況 | 做法 | 成本 | 備註 |
|---|---|---|---|---|---|
| 音量上限（任何輸出都不超過） | R `guide/11`（`volume_max`）、`guide/12` | 沒有（已排的「換輸出音量保護」是另一件） | 韌體；網頁加滑桿 | S | 夜間防突然大聲；可與已排項目一起做 |
| 每個輸出各自記音量（有線一個、每支耳機依位址各一個、喇叭模式一個） | R `pc/pc_volsel.h`、`features/bluetooth.md`「每個輸出各自的音量」 | 未確認（已排的音量保護可能只涵蓋一部分） | 韌體 | S | NVS 以藍牙位址為鍵 |
| 加權隨機（新檔與收藏權重高、最近 20 首權重 0） | R `pc/pc_shuffle.h`、`features/base.md` LIBTREE | 部分：隨機只是「另一首隨機」，上一首走歷史（`player.c` `s_hist`） | 韌體 | S | 權重要用到收藏（已排）與檔案日期 |
| 歌間淡入淡出（等功率，2／4／6 秒；取樣率相同才作用） | R `guide/03`「倍速與淡入淡出」、`pc/pc_gapless.h` | 沒有；已排「無縫換曲」，可能併入 | 韌體 | S–M | 核心 1 要預開下一首；LDAC／Auracast 時 round 自動關閉 |
| 略過「上次當機時正在播的那首」 | R `pc/pc_crashnet.h`、`app/main/crashnet.c`、`guide/03` | 部分：有 `crash.txt` 與開機迴圈保護；曲目黑名單未確認 | 韌體 | S | 記在 RTC 記憶體；沒螢幕時這是避免開機一直播壞檔的唯一辦法 |
| 手機串流進來時取得歌名／演出者／專輯（AVRCP 1.6 CT） | R `app/main/bt_meta*.c`、`pc/pc_btmeta.h`、`guide/05` | 沒有：`bt_audio.c` 的 `avrc_ct_cb` 只記連線 | 韌體（`st` 加欄位，網頁已會顯示標籤） | M | 封面不做；只在喇叭模式有用 |
| AVRCP 絕對音量（耳機音量鍵直接調盒子的耳機音量） | R `features/bluetooth.md`「藍牙耳機按鍵與音量」；requirements B9 | 沒有（B9 註明未做） | 韌體 | S–M | 與「每個輸出各自記音量」一起設計 |

### A2 來源與格式
| 功能 | 來源 | Nano 現況 | 做法 | 成本 | 備註 |
|---|---|---|---|---|---|
| 晶片音樂 `.m3u`：用曲名與長度當播放清單，其餘子曲放「其他子曲」 | NS `TODO.md`「M3U playlists (chiplib)」；GME `M3u_Playlist` | 沒有（只有 `radio.c:87` 讀電台的 .pls／.m3u） | 韌體 | S–M | 注意 KSS 從 0 算起、其他從 1 算起 |
| 一般播放清單檔 `.m3u`（本機曲目順序） | CC `crates/chipchill-library/src/playlist_files.rs` | 沒有 | 韌體 | M | 路徑解析與編碼；可與「曲庫索引」一起做 |
| 曲庫索引：掃 8 層資料夾、快取在卡上、全部隨機、掃描不中斷播放 | R `app/main/library_svc.c`、`pc/pc_library.h`、`guide/03` | 沒有（只有 `ls`，一層一層、只回檔名） | 韌體（B 節的網頁瀏覽要靠它） | M | 掃描吃核心 0 與 PSRAM；數萬首要分頁；內部 RAM 小 |
| 晶片電台：依機種洗牌成一袋、略過太短的曲、每首上限 2:30 淡出 | NS `TODO.md`「Chip radio」 | 沒有（多曲目檔案會逐首接續，但沒有跨檔電台） | 韌體 | M | 需要曲庫索引與曲長資料 |
| 晶片音樂 GME 原生的音色、速度、立體寬度控制 | JC `src/app/screen_sound.c`；NS `TODO.md`「Not ported」 | 部分：音效鏈有音色與立體寬度；GME 速度沒有（與已排的「播放速度」同源） | 韌體 | S–M | 速度請與已排項目合併 |
| 實驗性格式分級（OPL／部分 tracker 預設關，壞檔可能重開機） | R `guide/03`「格式分級」、`pc_chipseq.c` | 沒有分級；OPL／tracker 已接線但未測 | 韌體＋網頁開關 | S | 沒螢幕時，壞檔重開機更難察覺，建議先做分級 |
| Amiga 音色（Paula 模擬：A500／A500+LED／A1200） | R `guide/03`、`features/base.md` AMIGA | 未確認 | 韌體＋網頁 | S | 4 聲道 `.mod` 約吃核心 55% |
| 更多晶片格式：SNDH（Atari ST）、S98／VGM 多晶片（ymfm 38 種）、PSF／NCSF、libopenmpt 的 MO3／Future Composer | CC `crates/libatariaudio-sys`、`libvgmchips-sys`、`chipchill-core/src/psf.rs`、`ncsf`、`libopenmpt-sys` | 沒有 | 韌體（新元件） | M–L 每項 | 要先量 CPU 與內部 RAM；CC 自己標示多數「還沒有人實際聽過」 |
| ADPCM 串流：DSP／BCSTM／BFSTM | CC `README`／`requirements.md` §2.1 | 沒有（ADX／VAG／BRSTM 已有） | 韌體 | M | 沒有真實遊戲檔測過 |
| 多組電台清單 `radio_<名稱>.txt` | R `guide/04` | 部分：只有一份 `radio.txt` | 韌體＋網頁 | S | BLE 只能寫 `radio.txt` 的規則要放寬，涉及已定的安全邊界 |
| 電台連線診斷（為什麼連不上：DNS、TLS、格式） | R `features/bluetooth.md`「電台：…連線診斷」 | 部分：紅燈閃 3 次＋`rstate` | 韌體（`st` 加原因欄位） | S | 網頁顯示見 B 節 |
| https 內部記憶體不足時拒絕並說明（每條 TLS 約 31 KB PSRAM） | R `guide/04` | 未確認 | 韌體 | S | 避免電台把內部 RAM 吃光 |
| ALAC（m4a 內） | `docs/ideas.md`「未驗證的假設」 | 未確認（m4a 路徑是否涵蓋） | 韌體（先量測） | S | 只要用一個 ALAC 檔試一次 |

### A3 藍牙
| 功能 | 來源 | Nano 現況 | 做法 | 成本 | 備註 |
|---|---|---|---|---|---|
| aptX／aptX HD 編碼 | R `pc/pc_btcodec.h`、`components/libfreeaptx`；requirements B5 | 沒有（B5 P2） | 韌體 | M–L | 核心 1 負載；LGPL；需要真 aptX 耳機驗證 |
| 編碼自動降級與升回（aptX HD→aptX→SBC，安靜 2 分鐘後試升） | R `guide/05`、`pc/pc_btcodec.h` | 部分：LDAC 有 ABR；其他無 | 韌體 | S–M | 依賴 aptX |
| 藍牙音訊上限設定（編碼、取樣率、位元深度、LDAC 最高位元率） | R `ui/scr_btaudio.c`、`guide/17` | 沒有（`bt` 指令沒有上限設定） | 韌體＋網頁 | S | 顯示見 B 節 |
| Auracast 品質檔位與低品質來源拒絕 | R `pc/pc_castq.h` | 部分：固定 LC3 48 kHz | 韌體 | S | 價值待主人判斷 |

### A4 USB
| 功能 | 來源 | Nano 現況 | 做法 | 成本 | 備註 |
|---|---|---|---|---|---|
| DAC 取樣率規則：高於上限時減半／四分之一、低於下限拒絕 | R `pc/pc_halve.h`、`pc_dacmax.h`、`pc_lowrate.h`、`guide/17` | 部分：Nano 一律 polyphase 重取樣，不減半、不拒絕 | 韌體 | S | 與已排的「取樣率測試＋DAC 上限偵測」緊密相關，等它做完再決定 |
| （USB 隨身碟、USB 音效卡、USB 鍵盤） | R `guide/14` | 見 D 節 | — | — | 只有一個 USB 主機埠，被 DAC 佔用 |

### A5 卡片與檔案
| 功能 | 來源 | Nano 現況 | 做法 | 成本 | 備註 |
|---|---|---|---|---|---|
| 掛載失敗原因白話化（GPT、無檔案系統、無回應）與「該怎麼辦」 | R `pc/pc_sdguide.h`、`guide/12` | 部分：紅燈 2 次＝無卡；原因未確認 | 韌體（`st` 加 `sdreason`）；顯示見 B | S | 與已排的卡片格式化流程相鄰 |
| 卡片保護：不正常拔線後卡片無回應的處理 | R `features/base.md` CARDSAFE | 未確認 | 韌體 | S–M | 沒有隨身碟模式時風險較低 |
| 2 GiB 以上檔案略過 | R `guide/12` | 未確認 | 韌體 | S | 避免 FatFs 邊界問題 |
| 設定備份到卡片（保留 30 份、不含 Wi-Fi 密碼與藍牙金鑰） | NS `TODO.md`「Backup」 | 沒有 | 韌體（UART）；或 B 節的網頁匯出 | S | BLE 不能寫卡的規則下，網頁匯出較合適 |
| 卡片內容包版本檢查（`pack.txt`） | R `app/main/pack_stamp.c`、`pc/pc_pack.h` | 沒有 | 韌體 | S | Nano 的場景檔由 `make_scenes.py` 產生；價值低 |

### A6 聲音處理
| 功能 | 來源 | Nano 現況 | 做法 | 成本 | 備註 |
|---|---|---|---|---|---|
| velvet room 殘響（round 的第 14 個效果） | R `features/fx.md` T17、`pc/pc_fxapp.h` | 沒有（Nano 有 13 個） | 韌體 | S | 重型；round 只在有線播放時開 |
| 提示音：晶片音色合成的短音，在音量之前混入輸出 | R `app/main/sfx.c`、`sfx_bank.def`、`pc/pc_sfx.h` | 沒有（`ideas.md` #3 提過，未裁定） | 韌體 | S–M | 要在解碼任務加混音點；是 C 節遊戲／計時器出聲的前提 |
| 測試音產生器（正弦、方波、三角、鋸齒，C2–C6） | R `app/main/test_tone.c`、`pc/pc_tuner.h` | 部分：`bt tone`（只到耳機）、`mkwav`（UART） | 韌體 | S | 試耳機、試 DAC；網頁加按鈕 |
| 人聲消除（左右相同的部分相減，每取樣幾次加減） | CC `requirements.md` §2.7；`chipchill-dsp` | 沒有 | 韌體（加進效果鏈） | S | 效果便宜；CC 說明 800 Hz 約去 30 dB |
| 升降 Key ±12 半音（速度不變） | CC `crates/libstretch-sys`；`requirements.md` §2.8 | 沒有（已排「播放速度」只管速度） | 韌體 | M | Signalsmith Stretch 吃核心 1；一般音檔才要 |
| 更多效果節點：Lo-fi、齒音抑制、瞬態、廳堂混響（FDN）、卷積（脈衝響應）、線性相位 EQ、耳機環繞 | CC `crates/chipchill-dsp/src/nodes`；`requirements.md` §2.5 | 沒有 | 韌體 | M–L 每項 | 全是重型；round 的效果鏈已佔核心 1 的 60% |
| 響度正規化（EBU R128／BS.1770） | CC `crates/chipchill-loudness`；requirements D7 | 沒有（ReplayGain 只讀標籤） | 板上 L；或電腦端工具寫標籤（S） | S–L | 板上掃描太重；建議電腦端 |
| 晶片音樂每首音量校正（取整首中位數，落在 −22…−16 dBFS 走廊外才調） | NS `TODO.md`「Level correction (chiplib)」 | 沒有（晶片檔沒有 ReplayGain 標籤） | 韌體或離線 | M | NS 全庫第一次索引 21 分鐘；需離線算好存卡 |
| 拍點偵測（BPM、拍相位） | R `pc/pc_beat.h`；CC `chipchill-dsp/src/beat.rs` | 沒有 | 韌體；或網頁自己用 32 頻段算 | S–M | 只有 LED 隨拍、節拍遊戲才需要 |
| 依輸出裝置記憶音效設定（DAC、每支耳機各一組） | CC `requirements.md` §2.5 | 沒有 | 韌體 | S–M | 每組 fx.txt 約數百位元組 |
| 雙耳節拍（腦波音）：兩個正弦波即時合成 | SM `data_pipeline/render_brainwave_loops.py`；`docs/ideas.md` #20 | 未確認（場景檔由 SM 資料庫產生，是否含腦波迴圈未查） | 韌體 | S | 有損編碼可能削弱效果 |
| 自製噪音場景：選層、各層音量、存成自訂場景 | R `features/bluetooth.md`「自製噪音情境包」、`pc/pc_layerset.h`；SM App Scenes | 部分：場景由電腦腳本產生；裝置上不能編 | 韌體＋網頁（見 B） | M | 場景比其他來源多用約 26 KB 內部 RAM；層數上限 3 |

### A7 時間與排程
| 功能 | 來源 | Nano 現況 | 做法 | 成本 | 備註 |
|---|---|---|---|---|---|
| 時區（POSIX 字串如 `CST-8`） | R `guide/12`（`tz`）、`app/main/clock_svc.c` | 未確認（夜間排程要 NTP；時區處理未查） | 韌體 | S | 與已排「網頁對時」一起做 |
| 呼吸引導：LED 呼吸＋噪音跟隨（吸、停、吐、停） | R `pc/pc_breath.h`、`ui/scr_breath.c`；requirements F10（P3） | 沒有 | 韌體（LED、噪音）＋網頁（節奏設定） | S | 單顆 LED；夜間上限要處理；與被否決的「LED 時間燈」不同 |

### A8 診斷與維護
| 功能 | 來源 | Nano 現況 | 做法 | 成本 | 備註 |
|---|---|---|---|---|---|
| 當機記錄補上 PC 位址（panic handler） | R `pc/pc_crash.h`；requirements F1「尚無 PC 位址」 | 部分 | 韌體 | S–M | 沒螢幕時是唯一線索 |
| 播放卡住自動重啟（位置 60 秒沒動就重開來源，每小時最多 3 次） | NS `TODO.md`「Watchdog」 | 沒有（只有 IDF 任務看門狗） | 韌體 | S | 已有 underrun 計數可當訊號 |
| 晶片音樂解碼 CPU 基準（逐區塊計時，報分佈） | R `app/main/chipbench_ctl.c`、`pc/pc_chipbench.h` | 部分：`tools/stress.py` 從外面量 | 韌體 | S | 各格式的 CPU 表還沒有 |
| LAB：執行期旋鈕（原本編譯期常數，用指令改） | R `pc/pc_lab.h`、`features/base.md` LAB | 沒有 | 韌體 | S | 需要的時候再做即可 |
| 低記憶體保護（內部 RAM 太低時拒絕開 https、降級） | R `app/main/lowmem.c`、`pc/pc_lowmem.h` | 未確認 | 韌體 | S | 內部 RAM 是瓶頸，值得查 |
| 版本號與構建資訊顯示 | R `guide`「現況」頁 | 未確認 | 韌體（`st`／`ver`） | S | 排查時用 |

### A9 設定
| 功能 | 來源 | Nano 現況 | 做法 | 成本 | 備註 |
|---|---|---|---|---|---|
| LED 模式：隨音樂（頻譜色、拍點閃光）、呼吸、脈衝、純色 | JC `src/led/led_effects.c`（11 種，16 顆燈帶） | 沒有（LED 只顯示狀態） | 韌體＋網頁 | S–M | 只有一顆 RGB；夜間上限仍要生效；建議預設關 |
| 實驗性 CPU 時脈（320 MHz，當機後自動回復） | R `app/main/cpu_clock.c`、`guide/11` | 沒有 | 韌體 | S | round 自己標示實驗性；風險高，非必要 |

---

## B. 韌體＋網頁：原本靠螢幕，改由網頁顯示或操作就能用的

| 功能 | 來源 | Nano 現況 | 做法 | 成本 | 備註（盒子還要送什麼） |
|---|---|---|---|---|---|
| 曲庫瀏覽：依歌手／專輯／年份／資料夾、全部隨機、最新、搜尋；資料存 IndexedDB | NS `panel/panel.py`（MUSIC）、`musiclib/app.py`（`/api/library`）；R `ui/scr_tracks.c`、`ui/scr_mp3.c`；`docs/ideas.md` #11 | 部分：頁面能逐層看檔案；沒有標籤、沒有搜尋 | 兩者 | M | **新**：批次標籤指令（現在 `ls` 只回檔名，`st` 只有目前這首）：標題、演出者、專輯、年、曲號、長度；一檔約 100 位元組，數千首要分批 |
| 佇列：下一首插隊、清單、清除 | NS `musiclib/app.py`（`/api/queue`）；CC `chipchill-core/src/queue.rs` | 沒有 | 兩者 | S–M | **新**：`queue add／clear／list` 指令；清單放 PSRAM |
| 封面顯示（內嵌或同資料夾 `cover.jpg`） | R `app/main/thumb_svc.c`、`ui/scr_covers.c`；NS `/api/cover` | 沒有；`ideas.md` 提案者不建議（BLE 慢），**未見主人裁定** | 兩者 | M–L | 若做：只傳 ≤30 KB 的內嵌封面，沿用 `lrc` 的 base64 分行；大圖要縮圖，round 在 FakePod 上每張 3.4 秒、吃 PSRAM。另一條路：網頁依標籤向網路查封面，盒子不用改 |
| 曲目詳細資訊頁：標題、演出者、專輯、日期、曲號、版權、說明；技術資訊（位元率、取樣率、聲道、檔案大小） | R `ui/scr_meta.c`；CC `requirements.md` §2.3 | 部分：只有標題／演出者／專輯 | 兩者 | S | **新**：`st` 或 `info` 多送幾個欄位（`pc_id3` 已解析更多） |
| 晶片音樂資訊：主機、晶片、遊戲、作曲者、曲目數、各聲道名稱 | CC `chipchill-core/src/details`；`requirements.md` §2.3 | 部分 | 兩者 | S–M | **新**：GME 的 info 欄位 |
| 聲部列／鋼琴捲簾；聲部靜音與獨奏（晶片、tracker） | R `ui/scr_viz.c`、`pc/pc_voice.h`、`pc_vmute.h`；JC；CC `chipchill-viz/src/pages/voices.rs`、`roll.rs` | 沒有（只送 32 頻段） | 兩者 | M | **新**：另一個 BLE 通知，每聲部音量＋音名約 16 位元組 × 30 次／秒；寫入靜音遮罩。後端的聲部快照放 PSRAM，核心 1 負擔小 |
| 晶片音樂導覽：機種→遊戲→曲目（含曲名） | R `ui/scr_chip.c`；NS `chiplib` | 部分：只能走資料夾 | 網頁（依標籤分組） | S–M | 依賴上面的批次標籤 |
| 輸出「誠實說明」頁：輸出、來源、處理、位元、槽寬、音量，每項附原因（設定／耳機／DAC／忙碌） | R `pc/pc_outstat.h`、`guide/17` | 部分：`st` 有格式、取樣率、輸出 | 兩者 | S | **新**：`st` 加原因欄位與實際處理（重取樣、直通、效果強制處理） |
| 藍牙音訊頁：耳機最高能力 vs 實際協商（編碼、取樣率、位元深度、位元率） | R `ui/scr_btaudio.c`、`guide/17` | 部分：`st` 有 `bt` 與編碼名 | 兩者 | S | **新**：協商結果、LDAC 目前位元率 |
| 診斷頁：重開機原因、當機記錄、underrun 歷史、運轉時間、Wi-Fi 訊號、晶片溫度 | NS `TODO.md`「hostctl status」；R `guide`「現況」頁 | 部分：頁面有 CPU 與記憶體曲線 | 兩者 | S–M | **新**：BLE 讀 `crash.txt`（目前是 UART 專用？未確認）；`st` 加 uptime、RSSI；S31 溫度感測器未確認 |
| 卡片頁：卡片狀態白話、「這張卡跟得上哪些格式」的天梯 | R `ui/scr_cardtest.c`、`ui/scr_card.c`、`guide/12` | （卡片測速與格式化已排，見 F） | 網頁 | — | 只提醒：天梯判定（需求速度的 1.5 倍／75%）是 round 已定的規則，可直接沿用 |
| 場景編輯器：選層、各層音量、存成「配方」（存網頁，必要時送盒子） | SM App `ScenesScreen`／`NowPlayingScreen`；R `ui/scr_noise.c`；`docs/ideas.md` §五 | 部分：`scene N` 播卡上固定場景 | 兩者 | M | **新**：`scene` 接受層清單與音量、即時調單層；層數上限 3、場景多吃約 26 KB 內部 RAM |
| 場景／配方網址（NFC 貼紙一碰就開網頁並套用「今晚配方」） | TT `js/nfc.js`、`ui-share.js`；`docs/ideas.md` #15 | 沒有 | 網頁 | S | Web Bluetooth 需要手勢，重新連線仍要點一下 |
| 歌詞：逐字高亮、SRT／WebVTT、解說字幕包（以檔案雜湊配對） | CC `crates/chipchill-lyrics`；`requirements.md` §2.7 | 部分：LRC 對時；逐字時間被丟掉（`lrc.js`）；SRT／VTT 沒有 | 網頁；盒子 `lrc` 要能送 `.srt` | S | `lrc` 目前只讀同名 `.lrc` |
| 標籤編碼修復（Big5／GBK／Shift-JIS 被當 Latin-1） | CC `chipchill-core/src/text_repair.rs`；`requirements.md` §2.3 | 未確認（歌詞已處理；標籤未查） | 網頁；盒子送原始位元組 | S–M | 預設不修、發生時讓使用者選 |
| 標籤不全時從檔名猜歌手與歌名 | CC `chipchill-library/src/guess.rs` | 沒有 | 網頁 | S | 只在沒標題時用 |
| 統計頁：聆聽時間、排行、十二週格子、徽章 | CC `requirements.md` §2.10；JC `src/app/play_stats.c` | （統計資料已排，見 F）；排行與徽章沒有 | 網頁 | M | 資料由已排的統計提供；徽章是額外內容，可不做 |
| 耳聽調音：25 個以聽感命名的詞（溫暖、明亮…）→ EQ 段、曲線分層 | CC `requirements.md` §2.5 | 部分：頁面已有 EQ 編輯器、AutoEq／APO 匯入（`web/eq-nano.js`）；REW 格式與詞彙式調音沒有 | 網頁（轉成 `fx set`） | M | 盒子不用改 |
| 設定備份／還原：把全部設定與效果匯出成檔、匯入 | NS `TODO.md`「Backup」；CC TOML preset | 沒有 | 網頁＋韌體讀全部設定 | S–M | **新**：一個傾印指令 |
| 網頁刷機（Web Serial＋esptool-js，含備份、雜湊檢查） | R `tools/web/flasher.js`、`guide/13`；requirements C8（P3） | 沒有 | 網頁（韌體不用改） | M | 不是 OTA（OTA 已否決）；開發板的序列口與 DAC 口是否分開未確認 |
| 電台頁補強：最近播放、收藏匯出入、電台 logo | TT `js/fav.js`、`ui-fav.js`、`README` | 部分：搜尋與卡上收藏已有 | 網頁 | S | 存 localStorage；試聽不做（網頁不出聲） |
| 頁面配色主題（十個） | R `guide/18`、`features/themes-icons.md` | 部分：跟隨系統亮暗；視覺化有調色盤 | 網頁 | S | 價值低 |
| 原生 App（iOS 沒有 Web Bluetooth） | requirements C3（P2） | 沒有 | — | L | 只列出；CC 有 Android／Kotlin，但那是另一個產品 |

---

## C. 純網頁版「小程式」：跟盒子無關或幾乎無關，放在控制網頁裡就能用

**先回答三個問題。**

1. **round 有沒有瀏覽器／WASM 版本？沒有。** 全 repo 的 WASM 只有網頁工具的 `tools/web/vendor/harfbuzzjs/.../harfbuzz-subset.wasm`（切字型子集用）。模擬器 `R/sim/` 是原生 Linux（SDL2＋LVGL＋`sim/shim`、`sim/fake_*.c`，要先 `make build` 產生 `app/sdkconfig`），沒有 Emscripten 設定；找不到任何 `emcc`／`wasm` 的建置腳本（`grep` 全 repo）。本機也沒裝 `emcc`／`clang`，只有 `node` v20，所以我**無法在這裡驗證**編譯成 WASM 能不能一次成功。
2. **邏輯能不能原樣用？大致可以，畫面不行。** 下列 `pc/*.h` 全是純 C、只用 libc（`string.h`、`math.h`、`stdlib.h`；沒有 ESP-IDF、沒有 LVGL）：`pc_game_*`（15 個遊戲的規則）、`pc_clockapp`、`pc_timers`、`pc_fortune`、`pc_habit`、`pc_breath`、`pc_vocab`、`pc_reel`、`pc_listpick`、`pc_ambient`、`pc_beat`、`pc_tuner`、`pc_dial`、`pc_clockface`，合計約 5,200 行，另有 182 個主機端測試（`components/player_core/host_test/test_*.c`）。**畫面在 `ui/scr_*.c` 與 `ui_arena`、`ui_sectors`、`ui_board` 等 LVGL 共用元件裡，無法重用**，每個程式都要用 canvas／SVG 重畫。
3. **所以成本主要在畫面，不在規則。** 做法有兩條：(a) 用 Emscripten 把 `pc_*.c` 編成約幾十 KB 的 `.wasm`，加薄包裝（結構體要手寫 JS 對照，每個遊戲約半天）；(b) 直接用 JS 照 C 重寫規則，再用主機測試產生的黃金向量比對。TT 已有這個先例（TT 的 `js/viz` 是 CC 視覺化的 JS 移植，`tests/fixtures/viz-*.json` 由 `tools/chipchill-fixtures` 產生）。我偏向 (b)：每個程式只有 60–500 行，又沒有 WASM 工具鏈；只有 AI 搜尋較重的遊戲（四子棋、骨牌，`pc_game_ai.h`）才值得考慮 WASM。若要「整台模擬器搬進瀏覽器」（LVGL＋SDL2 走 Emscripten），那是 L 且要帶 CJK 字型，不建議。授權：R 檔頭是 LGPL-2.1-or-later，移植成 JS 仍沿用；TT 也是同授權。

**聲音規則**：網頁不出聲。所以每個程式的音效只有三種辦法：(1) 靜音，改用震動（`navigator.vibrate`，需要手機）、通知、畫面閃爍；(2) 請盒子播——**現有指令只能 `play` 卡上的檔案，會打斷正在播的音樂**；(3) 做完 A6 的「提示音」之後，由盒子在音量之前混入短音（R 的做法：`sfx.c`，遊戲與計時器都走它）。下表「聲音」欄寫的是哪一種可行。**Nano 的網頁目前沒用到螢幕保持喚醒、通知、震動**（`index.html` 內查無），第一個 C 程式要順便加。

| 功能 | 來源 | 盒子參與 | 移植方式 | 成本 | 聲音與備註 |
|---|---|---|---|---|---|
| **時鐘與時間** | | | | | |
| 床頭鐘面：暗紅大時鐘＋睡眠剩餘＋頻譜，螢幕保持喚醒 | `docs/ideas.md` #16；R `ui/clock_face.c`、`pc/pc_clockface.h`；TT `js/wakelock.js` | 用盒子的 `st`（睡眠剩餘）與頻譜；時鐘本身不用 | JS（指針角度公式在 `pc_clockface.h`，很短） | S | 靜音。夜間要很暗；螢幕常亮會耗手機電 |
| 多種時鐘錶面（類比、數位、資訊、HUD） | R `ui/clock_face.c`、`ui/scr_clock.c`、`pc/pc_clockface.h`；`_player/faces.zip`（卡片內容包的圖片） | 不用 | canvas／SVG 重畫；HUD 錶面是圖片包，要自己美術 | S–M 每種 | 靜音 |
| 世界時鐘／時區 | R `app/main/clock_svc.c` | 不用 | JS `Intl` | S | — |
| 日出日落、月相、24 節氣、城市表 | R `pc/pc_clockface.h`（日晷）、主控台 moon；JC `docs/decisions/0018`、`tools/gen_city_table.py`；JC `src/time/moon.c`、`city.c` | 不用（位置由使用者選，不要求定位） | JS 公式（只靠日期時間可算）；農曆 JC 已明確放棄 | S–M | 靜音 |
| 倒數計時、碼表、圈速 | R `pc/pc_timers.h`（96 行）、`app/main/timers_svc.c` | 不用；可選：到時讓盒子燈閃或停播 | JS | S | 到時：震動＋通知＋閃屏；盒子出聲要 A6 提示音 |
| 番茄鐘 | R `pc/pc_clockapp.h`（75 行）；requirements D10（P3） | 不用；可選：盒子播噪音 | JS | S | 同上；R 在放音樂時只震動不打斷 |
| 喝水提醒 | R `app/main/drink_svc.c`、`pc/pc_habit.h` | 不用 | JS＋localStorage | S | 需要頁面開著；通知權限 |
| 習慣打卡（每日勾選清單） | R `pc/pc_habit.h`（100 行）、`ui/scr_habits.c` | 不用 | JS＋IndexedDB | S | 靜音 |
| 呼吸引導（圓環，可選呼吸音） | R `pc/pc_breath.h`（69 行）、`ui/scr_breath.c` | 可選：盒子 LED 與噪音同步（A7） | JS | S | 呼吸音走盒子的噪音；網頁靜音 |
| 計數器（2／4／8 個） | R `ui/scr_counter.c`、`pc/pc_gamesave.h` | 不用 | JS＋localStorage | S | 震動 |
| 手電筒（全白、最亮） | R `ui/ui_torch.c`、`guide/07` | 不用 | 純白畫面＋螢幕保持喚醒 | S | 手機本來就有；價值低 |
| **小工具** | | | | | |
| 籤詩（御神籤）與轉盤；文字可自訂 | R `pc/pc_fortune.h`（252 行）、`app/main/fortune_text.c`、`ui/scr_fortune.c` | 不用 | JS；資料是 C 字串，抄出來即可 | S | 靜音；日種子讓一天內結果相同 |
| 午餐／抽人（輪流不重複）／點子產生器 | R `pc/pc_listpick.h`、`ui/scr_lists.c`；資料 `R/tools/web/lists/*.json`（純 JSON） | 不用 | JS；清單 JSON 現成 | S | 靜音。注意 `assets/lists/*.zip` 是字形包，不要拿它當資料 |
| 骰子、拉霸 | R `pc/pc_reel.h`（176 行）、`assets/slot_syms.bin`、`ui/scr_games.c` | 不用 | JS＋canvas | S | 震動；拉霸賠率規則在 `pc_reel.h` |
| 單字測驗（字卡、答錯重現） | R `pc/pc_vocab.h`（370 行）、`ui/scr_vocab.c`、`tools/gen/make_vocab.py`；資料 ECDICT（MIT） | 不用 | JS；字庫要自備（`assets/vocab/sample.zip` 是字形包格式，不是純文字） | M | 靜音 |
| 捏泡泡紙 | R `ui/ui_bubbles.c`、`ui/scr_bubbles.c` | 不用 | canvas | S | 震動 |
| 音效庫試聽（晶片音色） | R `ui/scr_sfxbank.c`、`sfx_bank.def` | 要盒子出聲 | 網頁只送「播第 N 號」 | M | 需 A6 提示音；網頁自己不播 |
| 氛圍畫面：熔岩燈、魚缸、篝火、夜燈（五段亮度、三種色調） | R `pc/pc_ambient.h`（495 行）、`ui/ambient_tone.c`、`ui/scr_ambient.c` | 可選：隨頻譜或睡眠剩餘變化 | JS／canvas；運動邏輯全在純 C | M | 靜音。亮度階梯 20–80%，夜間友善 |
| **遊戲**（15 個；R 都只用點擊，不需拖曳，網頁版直接可用觸控） | `app/main/ui/games.def`；`pc/pc_game_*.h`；`guide/07` | 見各列 | 規則 JS／WASM，畫面重畫 | 見各列 | 全部：靜音＋震動；若要聲音，走 A6 提示音。D11／F14 當初寫「遊戲不做」是指盒子沒螢幕，**網頁版是新問題，請主人重新判斷** |
| 井字棋（有電腦對手） | `pc/pc_game_ttt.h`（204 行）、`pc_game_ai.h` | 不用 | JS | S | — |
| 四子棋 | `pc/pc_game_c4.h`（317 行） | 不用 | JS（搜尋要時間預算，`pc_game_ai.h`） | S–M | — |
| 骨牌 | `pc/pc_game_dominoes.h`（163 行） | 不用 | JS | S–M | — |
| 15 拼圖 | `pc/pc_game_puzzle15.h`（105 行） | 不用 | JS | S | — |
| 生命遊戲（細格版） | `pc/pc_game_life.h`（276 行） | 不用 | JS／canvas | S | — |
| 疊塔 | `pc/pc_game_tower.h`（159 行） | 不用 | JS／canvas | S–M | — |
| 四色記憶（Simon） | `pc/pc_game_simon.h`（87 行） | 不用 | JS | S | 原本靠四個音；靜音版用閃光＋震動 |
| 保險箱開鎖 | `pc/pc_game_safe.h`（83 行）、`pc_dial.h` | 不用 | JS：用拖曳或兩個按鈕取代旋鈕 | S | 近距提示改震動 |
| 旋轉擋板 | `pc/pc_game_deflect.h`（212 行） | 不用 | JS／canvas | M | 旋鈕改拖曳 |
| 環形打磚塊 | `pc/pc_game_breakout.h`（396 行） | 不用 | JS／canvas | M | 旋鈕改拖曳 |
| 萬花尺 | `pc/pc_game_spiro.h`（117 行） | 不用 | JS／canvas | S | 純繪圖，最適合網頁 |
| 旋鈕小玩意（上發條、釣魚、迷宮） | `pc/pc_game_knobtoy.h`（110 行）、`pc_game_maze.h`（264 行）、`ui/knobtoy_maze.h` | 不用 | JS；迷宮重力可用手機動作感測（見 D） | M | — |
| 節拍點擊 | `pc/pc_game_beat.h`、`pc_beat.h`、`ui/ui_beat.c` | **用頻譜**：盒子約每 33 ms 送一次 32 頻段 | JS：節拍偵測用頻譜算；判定視窗要放寬（BLE 抖動，估計 30–100 ms，未實測） | M | 音樂由盒子播，網頁只判定；與耳機延遲不同步 |
| 猜歌 | `pc/pc_game_guess.h`、`ui/scr_game_guess.c` | **用音訊**：盒子播 2 或 5 秒片段、選項來自曲庫標籤 | JS；需要 B 節的批次標籤 | M | 沒有「從檔案中段開始播」指令（只有續播書籤）；從頭播的版本可行 |
| 電台調頻尋寶 | `pc/pc_game_radio.h`（131 行）、`pc_dial.h` | 不用（可選：真的切電台） | JS | S | 刻度盤近距提示改震動 |

---

## D. 需要板子沒有的硬體

| 功能 | 來源 | 缺什麼 | 網頁替代 | 成本 | 備註 |
|---|---|---|---|---|---|
| 調音器（聽音名、偏幾音分、A4 基準 430–450） | R `app/main/tuner_svc.c`、`pc/pc_tuner.h`（138 行 YIN 偵測）、`ui/scr_tuner.c` | 麥克風 | **有，移到 C**：手機麥克風 `getUserMedia`＋JS 移植 YIN | S–M | 「發音」那一半不能由網頁做（不出聲），可用 A6 測試音交給盒子 |
| 小尾巴上的麥克風（量環境噪音、讓遮蔽噪音跟著變） | `docs/ideas.md` #18 | 麥克風；小尾巴是否有輸入未驗證 | 手機麥克風也可量，但要頁面開著 | M–L | 原始提案者建議放後面 |
| 傾斜迷宮、體感遊戲 | R `app/main/imu_svc.c`、`pc/pc_imu.h`、`pc_imucal.h`、`pc/pc_game_maze.h` | IMU（QMI8658） | **有，移到 C**：手機 `DeviceMotionEvent`（iOS 要授權） | M | 校正與歸零邏輯在 `pc_imucal.h` |
| 震動回饋（觸覺馬達 DRV2605L） | R `guide/06`、`guide/11`；bsp `haptics` | 觸覺馬達 | **有，移到 C**：`navigator.vibrate`（iOS Safari 不支援） | S | 已併入 C 節各程式 |
| 電池與電源管理（電量計、充電畫面、自動關機、低電量拒開機） | R `pc/pc_axp2101.h`、`pc/pc_pwrpol.h`、`app/main/power_svc.c`、`charge_mode.c`、`ui/scr_charge.c` | 電池與 PMU | 無 | — | Nano 用 USB 供電，不適用 |
| 即時時鐘（斷電後時間仍在） | R `pc/pc_rtc85063.h`；JC `docs/decisions/0018`（DS3231） | RTC 晶片 | 部分：已排的「網頁對時」每次連線補時間 | M（硬體） | 斷電後夜間排程要等對時；JC 的結論是 RTC 與 NTP 互補 |
| USB 隨身碟（卡當電腦的碟）、USB 音效卡、USB／BLE 鍵盤 | R `app/main/ext_mode.c`、`ext_uac.c`、`ext_hid_send.c`、`guide/14`；requirements A11（P3） | 第二個 USB 裝置埠（Nano 唯一的 USB-C 是主機埠，被 DAC 佔用；開發板另一個埠能否當裝置未確認） | 部分：BLE 鍵盤與 BLE MIDI 不需要 USB | L | round 這個模式是重開機換模式，Nano 沒有螢幕可提示 |
| 螢幕邊緣光環（音量弧線、LED 環特效、樂器 LED 環） | R `ui/ledrim.c`、`pc/pc_ledring.h`、`pc_ledfx.h`、`pc/pc_viz.h` | 螢幕或 LED 燈環 | 網頁視覺化已有頻譜等 | — | 單顆 LED 的對應見 A9 |
| 觸控／旋鈕專用操作（九宮格、軸按鈕、旋鈕借用、重新排版） | R `ui/ui_ninegrid.c`、`ui_axis.c`、`ui_ring.c`；`features/ui.md` | 螢幕、旋鈕 | 網頁本身就是觸控 UI | — | 不必移植 |
| 讓 Nano 發聲的喇叭（鬧鐘、遊戲聲、提示音） | 全盤 | 喇叭；只能經 DAC 或藍牙 | 無（網頁不出聲） | — | 提示音只在有輸出時才聽得到；這是鬧鐘被否決的原因 |

---

## E. 不適用或已被主人否決（不要再提）

**E1 主人裁定（出自 `docs/requirements.md`）**
| 項目 | 來源 | 裁定 |
|---|---|---|
| 鬧鐘（含多組、星期、鬧鐘保險、貪睡） | NS `hostctl/cues.py`；CC；JC `wake_alarm.c`；R `pc/pc_clockapp.h` | 2026-11-24 夜：盒子沒喇叭，可能沒人聽到；已從韌體與網頁移除（D2、F7） |
| 電台時光緩衝（暫停、倒退） | `ideas.md` #9 | 2026-11-24 夜 不做 |
| 耳機當按鍵（耳機開關觸發 bedtime、貪睡） | `ideas.md` #2 | 2026-11-24 夜 不做（耳機按鍵與小尾巴線控仍保留） |
| 夜晚節目表（cues：來源＋時間窗＋淡入淡出） | NS `hostctl/cues.py`、`panel`「排程」 | 2026-11-24 夜 不做 |
| LED 時間燈／日出漸亮 | `ideas.md` #6 | 2026-11-24 夜 不做 |
| OTA 更新 | R `docs/web-hosting.md`；requirements F13 | 不做，故障重刷（網頁 Web Serial 刷機是另一件，見 B） |
| 長時間 soak 測試 | R `app/main/soak_log.c` | 2026-11-24 不做 |
| 波形（示波器）與相位（goniometer）視覺化 | TT `js/viz/plugins`；CC `chipchill-viz`（scope、xy） | 裝置不送波形；已裁定不支援（C9） |
| 手機→盒子→耳機中繼 | requirements A8 | 做不到（Bluedroid 一次只能一種 A2DP 角色，板上實驗過） |
| Auracast 收聽 | R `guide/05`；requirements A12 | 2026-11-25 只要廣播 |
| 影片 | `README.md` | 不做 |
| 經 BLE 寫檔、刪檔、改腳位（含網頁上傳音樂、卡片檔案管理） | NS `/api/upload`；CC `chipchill-transfer` | C5：控制層開放，寫檔只有序列埠；唯一例外 `radio.txt` |
| 把 Wi-Fi 當控制通道（AirPlay、DLNA、Spotify Connect、本機網頁） | NS `dashboard`、shairport-sync | `README.md`：Wi-Fi 只用於電台與 NTP |
| 網頁出聲（網頁試聽電台、鎖屏媒體鍵用的無聲音訊） | TT `README`（preview-listen、media keys）；`ideas.md` | 主人規則：網頁不得出聲 |
| TF 卡格式化（能正常讀取的卡） | R `app/main/sd_format.c` | 2026-11-25 只允許掛載失敗時、三步確認（已排，見 F） |

**E2 不適用（缺硬體或沒螢幕；非主人裁定）**
| 項目 | 來源 | 理由 |
|---|---|---|
| 相簿、封面圓盤、開機畫面、主題與字型包、九宮格 UI、首次開機語言選單 | R `ui/scr_album.c`、`guide/12`、`guide/18`、`features/ui.md` | 沒有螢幕（網頁已有三語與亮暗） |
| 萌寵（角色跟著音樂跳舞） | CC `crates/chipchill-pet`；requirements D11 | requirements 標「不做」，且要螢幕；網頁版若想要請主人另外提 |
| 樂器 app（鼓墊、節拍器、looper、取樣套組、晶片／FM 音色、BLE MIDI） | R `app/main/instrument.c`、`inst_*.c`、`guide/08`；JC `src/looper`、`src/midi`；CC `chipchill-midi` | requirements F14 判為不適用；若要重評：網頁 Web MIDI→BLE→盒子合成，成本 L、吃內部 RAM 與核心 1 |
| HLS 電台（TS／AAC 分段） | R `pc/pc_hls.h`、`pc_ts.h`、`guide/04` | requirements A6 與 README 寫「不做／不支援」，沒記主人理由；若要重評 R 已有純 C 實作，成本 M |
| GSF（mGBA）、DSD、APE | CC `libmgba-sys`；`ideas.md`「不建議」 | 提案者判斷 CPU 太重、與 LDAC 搶核心；非主人裁定 |
| 語音助理、TTS | `ideas.md` | 沒麥克風與喇叭，吃內部 RAM |
| 睡眠分期偵測 | `ideas.md` | 沒有感測器，會是假數據 |
| 服務管理、系統級 EQ、PipeWire 音量模型、儀表板磚、Syncthing、面板（按鍵＋OLED） | NS `hostctl`、`dashboard`、`panel` | 屬於 CM4／Linux；Nano 是單一韌體 |
| 授權碼與試用期、成就徽章解鎖外觀 | CC `chipchill-license` | 商業與 App 功能，不適用 |
| 外接式模式切換（重開機換模式）、安全模式畫面、品牌更名說明 | R `guide/14`、`guide/15` | Nano 常駐、不用換模式；安全模式已有（LED 紅 4 次） |
| 電子寵物、Wi-Fi 分析儀、紅外線、NFC 讀寫、Sub-GHz、TOTP 等 Flipper 風格項目 | R `docs/ideas/flipper-pick-list.md` | round 並未實作，不屬於「參考專案能做」 |

---

## F. 已排入、不必再選

| 項目 | 備註 |
|---|---|
| TF 卡測速與「只在掛載失敗時」的受保護格式化 | 進行中（requirements 2026-11-25） |
| 無縫換曲（gapless） | `pc/pc_gapless.h`；requirements F8 |
| 場景無縫循環（接縫交叉淡化） | `pc/pc_loop.h` |
| zip 封存檔內的晶片音樂包 | `pc/pc_zip.h` |
| 播放速度（變速不變調） | `pc/pc_speed.h` |
| 播放統計 | `pc/pc_stats.h` |
| 收藏與播放紀錄 | `pc/pc_hist.h`、NS `playlog.py`（20 秒才算聽過） |
| 取樣率測試與 DAC 上限偵測 | R `app/main/rate_test.c`、`pc/pc_ratetest.h` |
| 換輸出時的音量保護 | 見 A1：音量上限與每輸出音量是相鄰但不同的項目 |
| Wi-Fi 設定經 BLE | JC `docs/decisions/0019`、`web/provision.js`；requirements C7 |
| 從網頁對時 | 見 A7：時區要一起處理 |

---

## 建議

**我會先做的 10 項（價值／成本，床頭盒子優先）**
1. **音量上限＋每個輸出各自記音量（A1，S）**：沒螢幕、半夜常用；與已排的音量保護一起做最省。
2. **曲庫瀏覽、搜尋、佇列（B，M＋S）**：沒螢幕時「找一首歌」只能靠網頁逐層 `ls`；批次標籤指令一做，後面好幾項（晶片導覽、猜歌、統計頁）都能用。
3. **掛載失敗原因與診斷頁（A5＋B，S）**：沒螢幕時唯一的線索；`st` 加幾個欄位就好，核心與內部 RAM 都不吃。
4. **輸出「誠實說明」頁＋藍牙音訊頁（B，S）**：耳機音質問題最常卡在「到底協商到什麼」，現在只能看 UART。
5. **略過上次當機的那首（A1，S）**：沒螢幕又常放晶片音樂／tracker 壞檔時，避免開機迴圈。
6. **手機串流顯示歌名（A1，M）**：喇叭模式是常用功能，網頁已有標籤欄位只缺資料。
7. **床頭鐘面（C，S）＋螢幕保持喚醒**：純網頁、不動韌體，也是 C 節其他程式需要的第一塊（喚醒鎖、通知、震動）。
8. **呼吸引導（A7＋C，S）**：LED 本來就在、噪音本來就有，requirements F10 早列為候選。
9. **提示音（A6，S–M）**：解鎖 C 節裡所有需要「聲音」的程式（計時器、猜歌、音效庫），而且是唯一不違反「網頁不出聲」的做法。
10. **聲部列與聲部靜音／獨奏（B，M）**：晶片音樂是 P1，這是 round／JC／CC 三個專案都有的招牌功能，成本在一個新通知特徵。

備選：加權隨機（A1，S）、設定備份（B，S–M）、場景編輯器（B，M，注意 26 KB 內部 RAM）、歌間淡入淡出（併入已排的無縫換曲）。

**好玩但可選的 5 個（C，全部不動韌體）**
1. **氛圍畫面**（熔岩燈、魚缸、篝火、夜燈；`pc_ambient.h`）：夜間低亮度、可隨頻譜變化，最適合床頭。
2. **籤詩與轉盤**（`pc_fortune.h`）：資料現成、邏輯 250 行，睡前小儀式。
3. **萬花尺**（`pc_game_spiro.h`）：純繪圖、沒有輸贏，網頁上比在 360 圓屏上更漂亮。
4. **節拍點擊**（`pc_game_beat.h`）：唯一會「聽」盒子頻譜的遊戲，順便驗證 BLE 頻譜的時間精度。
5. **月相／日出／節氣**（`pc_clockface.h`、JC 0018）：只靠日期可算，與床頭鐘面搭配。

---

## 附：我沒能確定的事
- 本機沒有 `emcc`／`clang`，無法驗證 WASM 編譯；`pc_*` 檔案只靠讀 `#include` 判斷「只用 libc」。
- `ls` 只回檔名是看原始碼得到的；BLE 能否讀 `crash.txt`、S31 有無溫度感測器、開發板第二個 USB 埠能否當裝置，都未確認。
- A1 的「每個輸出各自記音量」、A5 的卡片保護、A8 的低記憶體保護與版本顯示，Nano 現況標「未確認」，需要讀 `player.c`／`settings.c` 逐項查。
- C 節「BLE 頻譜判定視窗 30–100 ms」是估計，沒有量測。
- 成本 S／M／L 是我的估計，沒有實作過；R 的標籤與設定細節以 `docs/guide/zh-TW` 為準，未逐一在 `app/main` 內核對。
