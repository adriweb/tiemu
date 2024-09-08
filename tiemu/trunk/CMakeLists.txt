cmake_minimum_required(VERSION 3.12)

option(DEPS_RELEASE_ONLY "Build only release versions of vcpkg dependencies" OFF)
if(1)
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        message(FATAL_ERROR "Must provide a VCPKG_TARGET_TRIPLET to set as release only")
    endif()
    if(NOT VCPKG_TARGET_TRIPLET MATCHES "-release$")
        set(VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_TRIPLET}-release")
        message("Updated VCPKG_TARGET_TRIPLET to ${VCPKG_TARGET_TRIPLET}")
    endif()
endif()

if(WIN32 AND VCPKG_TARGET_TRIPLET MATCHES "-static(-|$)" AND NOT VCPKG_TARGET_TRIPLET MATCHES "-md(-|$)")
    message("Using static MSVC runtime...")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(tiemu C)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# In case there was an env-var override...
if(DEFINED ENV{CMAKE_INSTALL_PREFIX_OVERRIDE})
    set(CMAKE_INSTALL_PREFIX "$ENV{CMAKE_INSTALL_PREFIX_OVERRIDE}")
endif()
# Be sure to have the expected install prefix format
if(CMAKE_INSTALL_PREFIX)
    file(TO_CMAKE_PATH "${CMAKE_INSTALL_PREFIX}" CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "CMAKE_INSTALL_PREFIX" FORCE)
    message("CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
endif()

set(USED_CMAKE_GENERATOR "${CMAKE_GENERATOR}" CACHE STRING "Expose CMAKE_GENERATOR" FORCE)
message(STATUS "Detected system: ${CMAKE_SYSTEM_NAME} - host processor: ${CMAKE_SYSTEM_PROCESSOR} - C_COMPILER: ${CMAKE_C_COMPILER_ID}")

# Our modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/.cmake)
file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/.cmake/*.cmake")
foreach(file ${files})
    include(${file})
endforeach()

include(GNUInstallDirs)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8 /D_CRT_SECURE_NO_WARNINGS")
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
    # sane defaults + hardening
    set(GLOBAL_COMPILE_FLAGS "-W -Wall -Wextra -Wno-unused-parameter -Werror=write-strings -Wredundant-decls -Werror=date-time -Werror=return-type -Werror=pointer-arith")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GLOBAL_COMPILE_FLAGS} -Werror=implicit-function-declaration")
    # useful flags for debugging
    set(GLOBAL_DEBUG_FLAGS "-fno-omit-frame-pointer -fsanitize=address,bounds -fsanitize-undefined-trap-on-error")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${GLOBAL_DEBUG_FLAGS} ")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${GLOBAL_DEBUG_FLAGS} ")
    # only for clang/gcc
    add_compile_definitions(HAVE_FVISIBILITY=1)
endif()

if(APPLE)
    add_compile_definitions(__MACOSX__)
elseif(UNIX)
    add_compile_definitions(__LINUX__)
    set(LINUX TRUE)
elseif(MINGW)
    add_compile_definitions(__MINGW32__)
elseif(WIN32)
    add_compile_definitions(__WIN32__)
    add_compile_definitions(HAVE_CONFIG_H)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    include_directories(${CMAKE_SOURCE_DIR}/win32_config)
endif()

# i18n support checking
include(FindGettext)
include(FindIntl)
if(Intl_FOUND AND GETTEXT_FOUND)
    set(ENABLE_NLS 1)
    include_directories(${Intl_INCLUDE_DIR})
    add_compile_definitions(ENABLE_NLS=1)
    link_directories(${Intl_LIBRARY_DIRS})
    link_libraries(${Intl_LIBRARIES})
else()
    message(WARNING "The Intl and GetText libs are needed for translations - Only English will be available")
endif()
set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LOCALEDIR}")

# Global defines
add_compile_definitions(ENABLE_LOGGING=1 LOCALEDIR="${LOCALEDIR}")
if (NOT BUILD_SHARED_LIBS OR VCPKG_TARGET_TRIPLET MATCHES "-static" OR NOT VCPKG_TARGET_TRIPLET MATCHES "-dynamic")
    # When we link statically, we don't want the dllimport stuff, so let's workaround that here
    add_compile_definitions(TICALCS_EXPORTS TIFILES_EXPORTS TICONV_EXPORTS TICABLES_EXPORTS)
endif()

# Don't mess up vcpkg libs finding
if(CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg" AND DEFINED VCPKG_TARGET_TRIPLET)
    set(ENV{PKG_CONFIG_PATH} "")
else()
    # Set manually when installed with Homebrew, see https://github.com/Homebrew/legacy-homebrew/issues/45891
    # Note: we can't do the same for libiconv because it exports libiconv_* symbols and not iconv_*
    if(APPLE)
        set(BREW_LIB_PATHS "/opt/homebrew/lib" "/opt/homebrew/opt" "/usr/local/opt" "/usr/local/lib")
        foreach(LIB_DIR ${BREW_LIB_PATHS})
            if(EXISTS "${LIB_DIR}/libarchive/include/archive.h")
                set(LibArchive_INCLUDE_DIR "${LIB_DIR}/libarchive/include" CACHE INTERNAL "LibArchive_INCLUDE_DIR")
                break()
            endif()
        endforeach()
    endif()
endif()
find_package(PkgConfig)

find_library(LIBTICONV    NAMES ticonv REQUIRED)
find_library(LIBTIFILES2  NAMES tifiles2 REQUIRED)
find_library(LIBTICABLES2 NAMES ticables2 REQUIRED)
find_library(LIBTICALCS2  NAMES ticalcs2 REQUIRED)
find_path(TILIBS_INCLUDES_DIR NAMES ticalcs.h PATH_SUFFIXES tilp2 REQUIRED)

file(GLOB HEADER_FILES src/**/*.h)
set(SRC_FILES
        ${HEADER_FILES}
        src/core/dbg/bkpts.c
        src/core/dbg/debug.c
        src/core/dbg/disasm.c
        src/core/dbg/fpudasm.c
        src/core/dbg/gdbcall.c
        src/core/engine.c
        src/core/error.c
        src/core/hwpm.c
        src/core/images.c
        src/core/interface.c
        src/core/state.c
        src/core/ti_hw/dbus.c
        src/core/ti_hw/flash.c
        src/core/ti_hw/gscales.c
        src/core/ti_hw/hw.c
        src/core/ti_hw/hwprot.c
        src/core/ti_hw/kbd.c
        src/core/ti_hw/m68k.c
        src/core/ti_hw/mem.c
        src/core/ti_hw/mem89.c
        src/core/ti_hw/mem89tm.c
        src/core/ti_hw/mem92.c
        src/core/ti_hw/mem92p.c
        src/core/ti_hw/memv2.c
        src/core/ti_hw/ports.c
        src/core/ti_hw/rtc_hw3.c
        src/core/ti_hw/tichars.c
        src/core/ti_sw/er_codes.c
        src/core/ti_sw/handles.c
        src/core/ti_sw/iodefs.c
        src/core/ti_sw/mem_map.c
        src/core/ti_sw/registers.c
        src/core/ti_sw/romcalls.c
        src/core/ti_sw/timem.c
        src/core/ti_sw/vat.c
        src/core/type2str.c
#        src/core/uae/build68k.c
        src/core/uae/cpudefs.c
        src/core/uae/cpudefs.c
        src/core/uae/cpuemu.c
        src/core/uae/cpustbl.c
        src/core/uae/fpp.c
#        src/core/uae/gencpu.c
        src/core/uae/missing.c
        src/core/uae/missing.c
        src/core/uae/newcpu.c
        src/core/uae/readcpu.c
        src/core/uae/readcpu.c
        src/core/uae/xmalloc.c
        src/core/uae/xmalloc.c
        src/gui/about.c
        src/gui/calc/calc.c
        src/gui/calc/dnd.c
        src/gui/calc/keyboard.c
        src/gui/calc/keymap.c
        src/gui/calc/keynames.c
        src/gui/calc/keypress.c
        src/gui/calc/popup.c
        src/gui/calc/screen.c
        src/gui/calc/skinkeys.c
        src/gui/calc/skinops.c
        src/gui/dboxes.c
        src/gui/debugger/dbg_all.c
        src/gui/debugger/dbg_bits.c
        src/gui/debugger/dbg_bkpts.c
        src/gui/debugger/dbg_cause.c
        src/gui/debugger/dbg_code.c
        src/gui/debugger/dbg_data.c
        src/gui/debugger/dbg_dock.c
        src/gui/debugger/dbg_entry.c
        src/gui/debugger/dbg_heap.c
        src/gui/debugger/dbg_iop.c
        src/gui/debugger/dbg_mem.c
        src/gui/debugger/dbg_pclog.c
        src/gui/debugger/dbg_regs.c
        src/gui/debugger/dbg_romcall.c
        src/gui/debugger/dbg_stack.c
        src/gui/debugger/dbg_vectors.c
        src/gui/debugger/dbg_wnds.c
        src/gui/device.c
        src/gui/filesel.c
        src/gui/fs_misc.c
        src/gui/infos.c
        src/gui/logger/log_link.c
        src/gui/main.c
        src/gui/manpage.c
        src/gui/pbars.c
        src/gui/quicksend.c
        src/gui/refresh.c
        src/gui/release.c
        src/gui/romversion.c
        src/gui/scroptions.c
        src/gui/splash.c
        src/gui/support.c
        src/gui/tsource.c
        src/gui/wizard.c
        src/misc/cmdline.c
        src/misc/files.c
        src/misc/logging.c
        src/misc/paths.c
        src/misc/rcfile.c
        src/misc/screenshot.c
        src/misc/tie_error.c
        src/sound/audio.c
        src/sound/stream.c
)

add_executable(tiemu ${SRC_FILES})

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
    set_target_properties(tiemu PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
            INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
            INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
    )
else()
    message(STATUS "IPO/LTO not supported: <${error}>")
endif()

if(WIN32)
    target_include_directories(tiemu PRIVATE "build/msvc")
endif()

# External deps
find_package(GTK2 2.10 REQUIRED gtk glade)
find_package(TIFF REQUIRED) # from gtk deps
find_package(PNG REQUIRED) # from gtk deps
find_package(ZLIB REQUIRED) # for libtifiles
find_package(LibArchive REQUIRED)
pkg_check_modules(DEPS REQUIRED IMPORTED_TARGET glib-2.0 libffi)

if(WIN32)
    find_library(LIBUSB0 NAMES libusb0.lib usb0 REQUIRED)
    target_link_libraries(tiemu PRIVATE ${LIBUSB0})
else()
    pkg_check_modules(LibUSB REQUIRED IMPORTED_TARGET libusb-1.0>=1.0.16)
    target_link_libraries(tiemu PRIVATE PkgConfig::LibUSB)
endif()

target_include_directories(tiemu PRIVATE ${GTK2_INCLUDE_DIRS} ${TILIBS_INCLUDES_DIR})
target_link_libraries(tiemu PRIVATE ${GTK2_LIBRARIES} PNG::PNG TIFF::TIFF ZLIB::ZLIB LibArchive::LibArchive PkgConfig::DEPS)
target_link_libraries(tiemu PRIVATE ${LIBTICONV} ${LIBTIFILES2} ${LIBTICABLES2} ${LIBTICALCS2})

if (APPLE)
    target_link_libraries(tiemu PRIVATE "resolv") # for gio for gtk...
endif()

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(tiemu
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

# Defines
set(SHAREDIR  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}")
target_compile_definitions(tiemu PRIVATE PACKAGE="${PROJECT_NAME}" VERSION="${PROJECT_VERSION}" SHARE_DIR="${SHAREDIR}")

# No GDB support
target_compile_definitions(tiemu PRIVATE NO_GDB)

target_include_directories(tiemu PRIVATE
        src/core
        src/core/uae
        src/core/dbg
        src/core/ti_hw
        src/core/ti_sw
        src/gui
        src/gui/calc
        src/gui/debugger
        src/gui/logger
        src/misc
        src/sound
        src/ipc/dbus
)

if(APPLE)
    target_link_libraries(tiemu PRIVATE "-Wl,-framework,CoreVideo -Wl,-framework,IOSurface -Wl,-framework,QuartzCore")
endif()

if(APPLE OR UNIX)
    set_target_properties(tiemu PROPERTIES LINK_FLAGS "-rdynamic")  # rdynamic is needed for the GUI callback functions
endif()

# Takes care of the i18n po/pot/gmo/mo files
if(ENABLE_NLS)
    i18n_mo_from_po_pot()
    if(TARGET potfiles_1)
        #add_dependencies(tiemu potfiles_1)
    else()
        message(WARNING "tiemu target potfiles_1 (i18n) is not available - did you run intltool-update?")
        remove_definitions(-DENABLE_NLS)
    endif()
endif()

# Install rules
install(TARGETS tiemu
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE  DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY   pixmaps help glade desktop skins misc
        DESTINATION "${SHAREDIR}")
install(FILES       man/tiemu.1
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")
