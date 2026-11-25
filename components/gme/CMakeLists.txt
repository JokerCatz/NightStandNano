# Game_Music_Emu 0.6.5 (LGPL-2.1+), vendored with JukeChip's patches 0001-0005 already applied
# (patches/ keeps them for reference). emu2413 (YM2413 / VRC7) is the separate component components/emu2413.
set(G ${CMAKE_CURRENT_LIST_DIR}/gme)
set(GME_SRCS
    Blip_Buffer.cpp Classic_Emu.cpp Data_Reader.cpp Dual_Resampler.cpp Effects_Buffer.cpp Fir_Resampler.cpp
    gme.cpp Gme_File.cpp Multi_Buffer.cpp Music_Emu.cpp
    Ay_Apu.cpp Ym2612_GENS.cpp Sms_Apu.cpp Ay_Cpu.cpp Ay_Emu.cpp
    Gb_Apu.cpp Gb_Cpu.cpp Gb_Oscs.cpp Gbs_Emu.cpp Gym_Emu.cpp
    Hes_Apu.cpp Hes_Apu_Adpcm.cpp Hes_Cpu.cpp Hes_Emu.cpp Kss_Cpu.cpp Kss_Emu.cpp Kss_Scc_Apu.cpp
    Nes_Apu.cpp Nes_Cpu.cpp Nes_Fme7_Apu.cpp Nes_Namco_Apu.cpp Nes_Oscs.cpp Nes_Vrc6_Apu.cpp Nes_Fds_Apu.cpp Nes_Vrc7_Apu.cpp
    Nsf_Emu.cpp Nsfe_Emu.cpp Sap_Apu.cpp Sap_Cpu.cpp Sap_Emu.cpp
    Snes_Spc.cpp Spc_Cpu.cpp Spc_Dsp.cpp Spc_Emu.cpp Spc_Filter.cpp
    Vgm_Emu.cpp Vgm_Emu_Impl.cpp Ym2413_Emu.cpp)
list(TRANSFORM GME_SRCS PREPEND ${G}/)
idf_component_register(SRCS ${GME_SRCS} INCLUDE_DIRS "gme" PRIV_INCLUDE_DIRS "." REQUIRES emu2413)
# PUBLIC: the emulator headers decide the class layout (Ym2612_Emu.h picks the YM2612 core from this define), so every
# user of them (app/main/backend_gme.cpp) must see the same one as the library.
target_compile_definitions(${COMPONENT_LIB} PUBLIC VGM_YM2612_GENS BLARGG_LITTLE_ENDIAN=1)
target_compile_options(${COMPONENT_LIB} PRIVATE -fwrapv -w)
