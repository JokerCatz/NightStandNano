# exFAT

ESP-IDF v6.1 hard-codes `FF_FS_EXFAT 0`. `tools/fatfs_exfat/make_fatfs.sh` (copied from JukeDial, which documents it in
`ESP32S31_round/tools/fatfs_exfat/README.md`) makes a patched copy of `components/fatfs` in `.local/fatfs_exfat/` with exFAT on,
the volume label API on, and `st_size` saturated at 0xFFFFFFFF (32-bit `off_t`: files of 2 GiB or more read as "too big").
`app/CMakeLists.txt` runs it at configure and adds the copy as a project component. An IDF upgrade that moves those lines fails
the configure loudly. FatFs stays at `FF_LBA64 0`: cards up to 2 TB with an MBR table; GPT is not read. The mount never formats.
