# SPDX-License-Identifier: LGPL-2.1-or-later
# Portable player core (copied from JukeDial and pruned): no ESP-IDF or LVGL dependencies.
# The sources that put bytes into NightStandNano's image (tools/prune_report.py); the rest of JukeDial's player_core was removed (components/ORIGIN.md).
# A new source file has to be added here (the list used to be a glob).
set(PC_SRCS
    "src/pc_backend_aiff.c" "src/pc_backend_chain.c" "src/pc_backend_flac.c" "src/pc_backend_hvl.c"
    "src/pc_backend_mp3.c" "src/pc_backend_mpc.c" "src/pc_backend_opl.c" "src/pc_backend_p8.c"
    "src/pc_backend_sid.c" "src/pc_backend_stream.c" "src/pc_backend_tta.c" "src/pc_backend_wav.c"
    "src/pc_backend_xmp.c" "src/pc_backend_ym.c" "src/pc_btcodec.c" "src/pc_cardtest.c"
    "src/pc_chipseq.c" "src/pc_crash.c" "src/pc_crc32.c" "src/pc_dacmax.c"
    "src/pc_dsp_biquad.c" "src/pc_extvoice.c" "src/pc_fmtguard.c" "src/pc_fmtreason.c"
    "src/pc_fxapp.c" "src/pc_fxapp_dyn.c" "src/pc_fxapp_gain.c" "src/pc_fxapp_limit.c"
    "src/pc_fxapp_parts.c" "src/pc_fxapp_reverb.c" "src/pc_fxapp_stereo.c" "src/pc_fxapp_tone.c"
    "src/pc_fxapp_tube.c" "src/pc_fxchain.c" "src/pc_fxstate.c" "src/pc_i18n.c"
    "src/pc_hls.c" "src/pc_i18n_data.c" "src/pc_icy.c" "src/pc_id3.c" "src/pc_inflate.c"
    "src/pc_io.c" "src/pc_kv.c" "src/pc_lh5.c" "src/pc_loop.c"
    "src/pc_lowrate.c" "src/pc_mp4.c" "src/pc_noisegen.c" "src/pc_preset.c"
    "src/pc_radio.c" "src/pc_rate.c" "src/pc_rg.c" "src/pc_ring.c"
    "src/pc_rsq.c" "src/pc_stream.c" "src/pc_toml.c" "src/pc_ts.c" "src/pc_utf.c"
    "src/pc_vmute.c" "src/pc_voice.c" "src/pc_volsel.c" "src/pc_zip.c")
idf_component_register(SRCS ${PC_SRCS} INCLUDE_DIRS "include" "third_party/dr_flac" PRIV_INCLUDE_DIRS "third_party/minimp3" "third_party/miniz" "third_party/tomlc99" "src" PRIV_REQUIRES libxmp libmpcdec libtta libhvl libcrsid libadplug)
# player_core does not include sdkconfig.h: the switch reaches its sources as a plain define
if(CONFIG_FX_VELVET_ROOM)
  target_compile_definitions(${COMPONENT_LIB} PRIVATE CONFIG_FX_VELVET_ROOM=1)
endif()
