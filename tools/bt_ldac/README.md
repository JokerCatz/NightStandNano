# Patched Bluedroid: vendor codecs (LDAC, aptX HD, aptX) for A2DP

ESP-IDF v6.1's Bluedroid "external codec" A2DP (`CONFIG_BT_A2DP_USE_EXTERNAL_CODEC`) negotiates only SBC and
AAC. `ldac.patch` adds vendor codecs (codec type 0xFF), driven by a table: **LDAC**, **aptX HD** and **aptX**.
Both roles are covered: source (the player's headphones mode) and sink (the test peer). The patch does not include
an encoder or decoder (the player encodes with `components/libldac` and `components/libfreeaptx`). It negotiates the
codec and moves the frames.

- `make_bt.sh`: copies `$IDF_PATH/components/bt` to `<repo>/.local/bt_ldac/bt` (git-ignored, never commit it), makes it
  a git repo (pristine commit of `host/bluedroid`), and applies `ldac.patch`. If the patch does not apply, it
  stops with an error. It also points `bt/Kconfig` at the patched `host/bluedroid/Kconfig.in` (the stock file
  sources Bluedroid's options from `$IDF_PATH`). `app/CMakeLists.txt` runs it at every configure; it does nothing while
  the copy is current (stamp of patch + script + IDF commit). `--force` regenerates, `BT_LDAC_DEST=/some/dir` does a trial run somewhere else.
- The project component named `bt` in `.local/bt_ldac` wins over IDF's.
- This directory was copied from JukeDial (`tools/bt_ldac`, same patch, LGPL-2.1) on 2026-11-25; NightStandNano maintains it
  independently from then on. References below to `make build-ldac` and `tools/bt_peer` are JukeDial's and do not exist here.

## What the patch changes (about 400 added lines in 17 files)

| File | Change |
|---|---|
| `common/include/common/bt_target.h` | `AVDT_CODEC_SIZE` goes from 10 to 20. The old size cut LDAC's 11-octet codec info element. |
| `btc/profile/std/include/btc_av_api.h` | `BTC_AV_CODEC_NONE` goes from 0xFF to 0xFE, because 0xFF is the on-air type of every vendor codec. Adds `BTC_AV_CODEC_VENDOR` (0xFF). |
| `stack/include/stack/a2d_api.h`, `bta/include/bta/bta_av_api.h` | `A2D_MEDIA_CT_NON_A2DP`, `BTA_AV_CODEC_VENDOR` |
| `api/include/api/esp_a2dp_api.h` | **API**: `esp_a2d_cie_vendor_t vendor_info` in the `esp_a2d_mcc_t.cie` union (vendor id, codec id, `value[5]`), `ESP_A2D_LDAC_*` constants. The union grows from 7 to 11 octets. |
| `btc/profile/std/a2dp/include/btc_av_co.h`, `bta_av_co.c` | **The vendor table** `bta_av_co_vendors[]`. Each row has the vendor id, codec id, CIE length, media header octets, a `media_ok` flag, and capability fields (octet offset, mask, bits in order of preference). Generic helpers: find, match, pick-one-config, find-local-endpoint. They are wired into every SBC/AAC dispatch site: getconfig (both roles), setconfig, build config, cfg-matches-caps, peer-supports-codec (both roles), sink/media supports config, build src cfg, source data path (media header), and the caps report. |
| `btc/profile/std/a2dp/btc_av.c` | Registering a vendor endpoint (`esp_a2d_*_register_stream_endpoint` with `ESP_A2D_MCT_NON_A2DP`; only rows with `media_ok`). `ESP_A2D_AUDIO_CFG_EVT` / `ESP_A2D_REPORT_SNK_CODEC_CAPS_EVT` with the vendor CIE. Room for the media header in `esp_a2d_audio_buff_alloc` / `esp_a2d_source_audio_data_send`. |
| `btc/profile/std/a2dp/btc_a2dp_sink_ext_coedc.c` | Sink data path: strips the media header and passes the frame count and frames to the app's data callback. |
| `bta/av/bta_av_aact.c` | An acceptor that got a vendor configuration no longer starts a discovery of its own (the path that SBC and AAC already skip). |
| `btc_av_co.c` (start), `stack/avdt/avdt_scb_act.c`, `avdt_int.h` | **No RTP header for classic aptX**: the source sets `no_rtp_hdr` when the stream starts (AVDT then leaves the header out, an option the stack already had); the sink's AVDT passes the whole packet up when the configuration is classic aptX (`bta_av_co_vendor_no_rtp`, a weak default for builds without the table). |
| `btc_av_co.h` | The peer's endpoint arrays (`snks`/`srcs`) hold 8 instead of the SBC/AAC count, so a peer that offers 5 codecs is not cut off ("no more room for SRC info"). |
| `Kconfig.in` | `BT_A2DP_SEP_NUM_MAX` up to 5 (was 3): LDAC, aptX HD, aptX, AAC, SBC. |
| `esp_a2dp_api.h/.c`, `btc_a2dp_source_ext_codec.c` | **API** `esp_a2d_source_get_tx_stats(&queued, &dropped)`: packets waiting in the source queue and packets dropped on a congested link (counted in `bta_av_co_audio_drop`), for the player's LDAC adaptive bit rate. aptX constants `ESP_A2D_APTX_*`. |

Table rows (checked against AOSP `a2dp_vendor_ldac*.cc`, `a2dp_vendor_aptx*.cc`): **LDAC** (vendor 0x0000012D,
codec 0x00AA, 8-octet CIE; sampling frequency 48 > 44.1 > 96 > 88.2 kHz, channel mode stereo > dual > mono; RTP
header, then a 1-octet media header holding the frame count, the same layout as SBC's). **aptX HD**
(0x000000D7/0x0024, 11-octet CIE) and **aptX** (0x0000004F/0x0001, 7-octet CIE): frequency in the high nibble
(48 > 44.1), channel mode in the low nibble (stereo); no media header; aptX HD has the RTP header, classic aptX does
not (`rtp = FALSE`). Adding a codec means adding a row. The dispatch code stays the same.

How to use it (app side): register an endpoint with `.type = ESP_A2D_MCT_NON_A2DP`, `cie.vendor_info.vendor_id`
/ `codec_id` / `value[]` (LDAC: `value[0]` frequency bits, `value[1]` channel-mode bits; aptX / aptX HD:
`value[0]` = frequency | channel bits). The stack offers the endpoints in seid order, so register the preferred one
first (the player: LDAC, aptX HD, aptX, AAC, SBC). The negotiated configuration arrives in `ESP_A2D_AUDIO_CFG_EVT`
(type `ESP_A2D_MCT_NON_A2DP`, one bit per field). Send packets as for SBC: `number_frame` = frames in the packet
(LDAC; 1 for aptX), `timestamp` in samples (ignored for classic aptX). The sink gets the frames without the media
header (LDAC: `number_frame` from it), and for classic aptX the whole packet with timestamp 0.

Not done: `esp_a2d_source_set_pref_mcc` with a vendor codec returns unsupported, and content protection
(SCMS-T) is not handled for vendor codecs.

## After an IDF update

1. `tools/bt_ldac/make_bt.sh --force`. If it applies, you are done (rebuild).
2. If it does not: apply what you can (`cd .local/bt_ldac/bt && git apply --reject ../../../tools/bt_ldac/ldac.patch`),
   fix the `*.rej` hunks by hand (each one is a `case BTC_AV_CODEC_VENDOR` / `BTA_AV_CODEC_VENDOR` next to the
   AAC case, so follow the `M24` code), then `git diff > ../../../tools/bt_ldac/ldac.patch` and run step 1 again.
3. Check that `AVDT_CODEC_SIZE` and `BTC_AV_CODEC_NONE` are still the values the patch expects, that
   `bt/Kconfig` still sources `host/bluedroid/Kconfig.in` the way `make_bt.sh` rewrites, and
   check whether IDF has added new places that switch on the codec (`grep -rn BTC_AV_CODEC_M24 host/bluedroid`).
