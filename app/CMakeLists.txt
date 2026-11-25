cmake_minimum_required(VERSION 3.16)
# exFAT: a generated, patched copy of IDF's fatfs (tools/fatfs_exfat, docs/exfat.md); a project component named fatfs replaces IDF's.
set(NSN_FATFS_DIR ${CMAKE_CURRENT_LIST_DIR}/../.local/fatfs_exfat)
execute_process(COMMAND env IDF_PATH=$ENV{IDF_PATH} bash ${CMAKE_CURRENT_LIST_DIR}/../tools/fatfs_exfat/make_fatfs.sh
                RESULT_VARIABLE NSN_FATFS_RC OUTPUT_VARIABLE NSN_FATFS_OUT ERROR_VARIABLE NSN_FATFS_ERR)
if(NOT NSN_FATFS_RC EQUAL 0 OR NOT EXISTS ${NSN_FATFS_DIR}/fatfs/CMakeLists.txt)
    message(FATAL_ERROR "tools/fatfs_exfat/make_fatfs.sh failed (${NSN_FATFS_RC}):\n${NSN_FATFS_OUT}${NSN_FATFS_ERR}")
endif()
list(APPEND EXTRA_COMPONENT_DIRS ${NSN_FATFS_DIR})
# Everything this project builds is inside the repo: decoders and player_core in ../components (copied from JukeDial, see components/ORIGIN.md),
# the patched Bluedroid (LDAC / aptX endpoints) generated into ../.local/bt_ldac by tools/bt_ldac/make_bt.sh (git-ignored, ~100 MB, a no-op while current).
set(NSN_BT_LDAC ${CMAKE_CURRENT_LIST_DIR}/../.local/bt_ldac)
execute_process(COMMAND env IDF_PATH=$ENV{IDF_PATH} bash ${CMAKE_CURRENT_LIST_DIR}/../tools/bt_ldac/make_bt.sh
                RESULT_VARIABLE NSN_BT_RC OUTPUT_VARIABLE NSN_BT_OUT ERROR_VARIABLE NSN_BT_ERR)
if(NOT NSN_BT_RC EQUAL 0 OR NOT EXISTS ${NSN_BT_LDAC}/bt/CMakeLists.txt)
    message(FATAL_ERROR "tools/bt_ldac/make_bt.sh failed (${NSN_BT_RC}):\n${NSN_BT_OUT}${NSN_BT_ERR}")
endif()
list(APPEND EXTRA_COMPONENT_DIRS ${NSN_BT_LDAC})        # a project component named bt replaces IDF's
foreach(c player_core libldac libxmp libmpcdec libtta libhvl libcrsid libadplug emu2413 gme fmvgm)
    list(APPEND EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/../components/${c})
endforeach()
get_filename_component(NSN_ROOT ${CMAKE_CURRENT_LIST_DIR}/.. ABSOLUTE)

# Version (`ver`, the boot log, esp_app_desc): -DPROJECT_VER=x.y.z for a release; otherwise `git describe --tags --dirty` when the checkout has a tag,
# otherwise version.txt at the repo root plus the commit ("0.1.0-19c6046[-dirty]"), or version.txt alone without git. IDF's own default (version.txt first)
# would hide the commit, so it is decided here. Re-run `idf.py reconfigure` after a commit or a new tag: it is read at configure time.
if(NOT PROJECT_VER)
    execute_process(COMMAND git -C ${NSN_ROOT} describe --tags --dirty OUTPUT_VARIABLE NSN_GIT_TAG RESULT_VARIABLE NSN_GIT_TAG_RC ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NSN_GIT_TAG_RC EQUAL 0 AND NSN_GIT_TAG)
        set(PROJECT_VER ${NSN_GIT_TAG})
    else()
        set(NSN_VER_FILE ${NSN_ROOT}/version.txt)
        set(PROJECT_VER "0.0.0")
        if(EXISTS ${NSN_VER_FILE})
            file(STRINGS ${NSN_VER_FILE} NSN_VER_LINES LIMIT_COUNT 1)
            list(GET NSN_VER_LINES 0 PROJECT_VER)
            string(STRIP "${PROJECT_VER}" PROJECT_VER)
            set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${NSN_VER_FILE})
        endif()
        execute_process(COMMAND git -C ${NSN_ROOT} describe --always --dirty OUTPUT_VARIABLE NSN_GIT_REV RESULT_VARIABLE NSN_GIT_REV_RC ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(NSN_GIT_REV_RC EQUAL 0 AND NSN_GIT_REV)
            set(PROJECT_VER "${PROJECT_VER}-${NSN_GIT_REV}")
        endif()
    endif()
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(COMPILE_DEFINITIONS "NSN_LDAC=1" APPEND)
# __FILE__ (asserts, logs) must not depend on where the repo is checked out: map the repo root to /NSN (IDF already maps this app directory to ".").
idf_build_set_property(COMPILE_OPTIONS "-fmacro-prefix-map=${NSN_ROOT}=/NSN" APPEND)
project(nightstand_nano)

# libadplug (copied from JukeDial) includes esp_timer.h without REQUIRES (JukeDial's build gets it another way); give it the dependency here.
idf_component_get_property(nsn_adplug libadplug COMPONENT_LIB)
target_link_libraries(${nsn_adplug} PRIVATE idf::esp_timer)

# Bluedroid trips -Wstringop-truncation at -O2 on this toolchain: relax it for the bt component only (as in JukeDial).
idf_component_get_property(nsn_bt_lib bt COMPONENT_LIB)
target_compile_options(${nsn_bt_lib} PRIVATE -Wno-error=stringop-truncation -Wno-stringop-truncation)
