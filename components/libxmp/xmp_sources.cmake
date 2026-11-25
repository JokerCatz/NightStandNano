# The libxmp sources we build and the compile definitions. Included by CMakeLists.txt.
set(_XMP ${CMAKE_CURRENT_LIST_DIR}/libxmp/src)
set(XMP_SRCS
    virtual.c mix_paula.c format.c period.c player.c read_event.c dataio.c lfo.c scan.c control.c far_extras.c flt_extras.c
    med_extras.c filter.c effects.c flow.c mixer.c mix_all.c rng.c load_helpers.c load.c hio.c hmn_extras.c extras.c smix.c
    path.c filetype.c memio.c miniz_tinfl.c
    loaders/common.c loaders/iff.c loaders/itsex.c loaders/lzw.c loaders/voltable.c loaders/sample.c
    loaders/xm_load.c loaders/mod_load.c loaders/s3m_load.c loaders/stm_load.c loaders/669_load.c loaders/far_load.c
    loaders/mtm_load.c loaders/ptm_load.c loaders/okt_load.c loaders/ult_load.c loaders/mdl_load.c loaders/it_load.c
    loaders/stx_load.c loaders/pt3_load.c loaders/sfx_load.c loaders/flt_load.c loaders/st_load.c loaders/emod_load.c
    loaders/imf_load.c loaders/digi_load.c loaders/fnk_load.c loaders/ice_load.c loaders/liq_load.c loaders/ims_load.c
    loaders/masi_load.c loaders/masi16_load.c loaders/amf_load.c loaders/stim_load.c loaders/mmd_common.c loaders/mmd1_load.c
    loaders/mmd3_load.c loaders/rtm_load.c loaders/dt_load.c loaders/no_load.c loaders/arch_load.c loaders/sym_load.c
    loaders/med2_load.c loaders/med3_load.c loaders/med4_load.c loaders/dbm_load.c loaders/umx_load.c loaders/gdm_load.c
    loaders/pw_load.c loaders/gal5_load.c loaders/gal4_load.c loaders/mfp_load.c loaders/asylum_load.c loaders/muse_load.c
    loaders/hmn_load.c loaders/mgt_load.c loaders/chip_load.c loaders/abk_load.c loaders/coco_load.c loaders/xmf_load.c
    loaders/prowizard/prowiz.c loaders/prowizard/ptktable.c loaders/prowizard/tuning.c loaders/prowizard/ac1d.c
    loaders/prowizard/di.c loaders/prowizard/eureka.c loaders/prowizard/fc-m.c loaders/prowizard/fuchs.c
    loaders/prowizard/fuzzac.c loaders/prowizard/gmc.c loaders/prowizard/heatseek.c loaders/prowizard/ksm.c
    loaders/prowizard/mp.c loaders/prowizard/np1.c loaders/prowizard/np2.c loaders/prowizard/np3.c loaders/prowizard/p61a.c
    loaders/prowizard/pm10c.c loaders/prowizard/pm18a.c loaders/prowizard/pha.c loaders/prowizard/prun1.c
    loaders/prowizard/prun2.c loaders/prowizard/tdd.c loaders/prowizard/unic.c loaders/prowizard/unic2.c loaders/prowizard/wn.c
    loaders/prowizard/zen.c loaders/prowizard/tp1.c loaders/prowizard/tp3.c loaders/prowizard/p40.c loaders/prowizard/xann.c
    loaders/prowizard/theplayer.c loaders/prowizard/pp10.c loaders/prowizard/pp21.c loaders/prowizard/starpack.c
    loaders/prowizard/titanics.c loaders/prowizard/skyt.c loaders/prowizard/novotrade.c loaders/prowizard/hrt.c
    loaders/prowizard/noiserun.c
    depackers/depacker.c depackers/ppdepack.c depackers/unsqsh.c depackers/mmcmp.c depackers/s404_dec.c depackers/arc.c
    depackers/arcfs.c depackers/arc_unpack.c depackers/lzx.c depackers/lzx_unpack.c depackers/ice.c depackers/ice_unpack.c
    depackers/crc32.c depackers/xfnmatch.c)
list(TRANSFORM XMP_SRCS PREPEND ${_XMP}/)
set(XMP_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/libxmp/include)
# The Paula (BLEP) mixer is built (AMIGA task, patch 0008): off until a cflag asks for it (pc_backend_xmp: setting amiga_mix). HAVE_* are left unset: libxmp then uses its own fallbacks (no umask, no fork/popen).
set(XMP_DEFS )   # LIBXMP_LITE_COMPAT (patch 0005) is OFF since 2026-11-18: libxmp's own accurate MOD / IT playback; define it here to get libxmp-lite's output back
