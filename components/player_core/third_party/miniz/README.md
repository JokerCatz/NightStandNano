miniz 3.0.2 (MIT), https://github.com/richgel999/miniz tag 3.0.2, unmodified.
Only miniz_tinfl.c is compiled, through src/pc_inflate.c, with every tinfl_* symbol renamed to pc_tinfl_*
so it cannot clash with the copy in the ESP32-S31 ROM.
miniz_export.h is a one-line stand-in for the file miniz's CMake build generates.
