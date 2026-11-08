# v32tools -- meta CMake project
#
# Drives the Vircon32 tools under tools/ (each with its own build system: a CMake
# project where it has one, otherwise its Makefile) from a single build directory.
# Nothing here compiles anything itself, so no compiler is needed to configure it.
# The tool list and build order come from tools.list, shared with the Makefile.
#
#   cmake -S . -B build [-DV32T_TOOLS="v32as;v32opt"] [-DV32T_BACKEND=auto|make|cmake]
#   cmake --build build                          build every selected tool
#   cmake --build build --target v32opt          build one tool
#   cmake --build build --target v32t-clean      (also -distclean -test -install -uninstall
#                                                 -sysinstall -sysuninstall -debug)
#   cmake --build build --target v32opt-clean    one step of one tool
#   cmake --install build [--prefix DIR]         system-wide install (= v32t-sysinstall)
#
# See docs/USAGE.md for the whole list of options.
cmake_minimum_required(VERSION 3.16)

project(v32tools
        VERSION 1.0.0
        DESCRIPTION "Meta project: build and install the Vircon32 community tools"
        LANGUAGES NONE)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/V32ToolsManifest.cmake")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tools.list")
v32t_read_manifest("${CMAKE_CURRENT_SOURCE_DIR}/tools.list")

# ---- options ---------------------------------------------------------------------------
set(V32T_TOOLS "all" CACHE STRING "Tools to operate on: 'all' or a ;-separated list (see tools.list)")
set(V32T_SKIP "" CACHE STRING "Tools to leave out")
set(V32T_BACKEND "auto" CACHE STRING "Tool build system: auto (CMake where a tool has CMakeLists.txt), make, cmake")
set_property(CACHE V32T_BACKEND PROPERTY STRINGS auto make cmake)
set(V32T_BUILD_TYPE "Release" CACHE STRING "Build type for tools built through CMake")
if(DEFINED ENV{HOME})
    set(_v32t_home "$ENV{HOME}")
else()
    set(_v32t_home "$ENV{USERPROFILE}")
endif()
set(V32T_USER_PREFIX "${_v32t_home}" CACHE PATH "Prefix of the user-level 'install' for CMake-built tools")
set(V32T_JOBS "" CACHE STRING "Parallel jobs inside each tool (empty = the tool's default; see docs about v32as and -j)")
set(V32T_FLEX "AUTO" CACHE STRING "AUTO: build the vendored flex when the host has none; OFF: never")
set_property(CACHE V32T_FLEX PROPERTY STRINGS AUTO OFF)
set(V32T_MAKE_ARGS "" CACHE STRING "Extra arguments for the make of Makefile-built tools")
set(V32T_CMAKE_ARGS "" CACHE STRING "Extra arguments for the configure step of CMake-built tools")
set(V32T_CMAKE_GENERATOR "" CACHE STRING "Generator for CMake-built tools (empty = CMake's default; on Windows use \"MinGW Makefiles\" or \"MSYS Makefiles\")")
find_program(V32T_MAKE_PROGRAM NAMES gmake make mingw32-make)
find_program(V32T_SH NAMES sh bash)

# ---- selection -------------------------------------------------------------------------
set(_want "${V32T_TOOLS}")
string(REPLACE "," ";" _want "${_want}")
if("all" IN_LIST _want)
    set(_want ${V32T_ALL_TOOLS})
endif()
foreach(_t IN LISTS _want)
    if(NOT _t IN_LIST V32T_ALL_TOOLS)
        message(FATAL_ERROR "V32T_TOOLS: unknown tool '${_t}'. Known: ${V32T_ALL_TOOLS}")
    endif()
endforeach()
set(V32T_SELECTED "")
set(V32T_ABSENT "")
foreach(_t IN LISTS V32T_ALL_TOOLS)           # keep manifest (build) order
    if(_t IN_LIST _want AND NOT _t IN_LIST V32T_SKIP)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/${_t}/Makefile"
           OR EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/${_t}/CMakeLists.txt")
            list(APPEND V32T_SELECTED ${_t})
        else()
            list(APPEND V32T_ABSENT ${_t})
        endif()
    endif()
endforeach()
if(V32T_ABSENT)
    message(WARNING "skipping ${V32T_ABSENT}: tools/<name> is empty (git submodule update --init, or scripts/v32t-bootstrap.sh)")
endif()

# An install prefix the user did not choose is passed on as "nothing", so each tool
# keeps its own platform default (they differ on Windows). An explicit
# -DCMAKE_INSTALL_PREFIX= / --prefix always wins.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(V32T_PREFIX_DEFAULTED TRUE)
else()
    set(V32T_PREFIX_DEFAULTED FALSE)
endif()

# ---- command lines for the driver script -------------------------------------------------
function(v32t_driver_command out action tools prefix)
    string(REPLACE ";" "," _tools "${tools}")
    set(${out}
        "${CMAKE_COMMAND}"
        "-DV32T_ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
        "-DV32T_ACTION=${action}"
        "-DV32T_TOOLS=${_tools}"
        "-DV32T_BACKEND=${V32T_BACKEND}"
        "-DV32T_BUILD_TYPE=${V32T_BUILD_TYPE}"
        "-DV32T_BUILD_DIR=${CMAKE_BINARY_DIR}"
        "-DV32T_USER_PREFIX=${V32T_USER_PREFIX}"
        "-DV32T_PREFIX=${prefix}"
        "-DV32T_JOBS=${V32T_JOBS}"
        "-DV32T_FLEX=${V32T_FLEX}"
        "-DV32T_MAKE_PROGRAM=${V32T_MAKE_PROGRAM}"
        "-DV32T_MAKE_ARGS=${V32T_MAKE_ARGS}"
        "-DV32T_CMAKE_ARGS=${V32T_CMAKE_ARGS}"
        "-DV32T_CMAKE_GENERATOR=${V32T_CMAKE_GENERATOR}"
        "-DV32T_SH=${V32T_SH}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/v32t-run.cmake"
        PARENT_SCOPE)
endfunction()

set(_explicit_prefix "")
if(NOT V32T_PREFIX_DEFAULTED)
    set(_explicit_prefix "${CMAKE_INSTALL_PREFIX}")
endif()

set(V32T_ACTIONS build debug clean distclean test install uninstall sysinstall sysuninstall)

# aggregates: one script run per action, tools handled one after the other in build order
foreach(_a IN LISTS V32T_ACTIONS)
    v32t_driver_command(_cmd ${_a} "${V32T_SELECTED}" "${_explicit_prefix}")
    if(_a STREQUAL "build")
        add_custom_target(v32t-build ALL COMMAND ${_cmd} USES_TERMINAL VERBATIM
                          COMMENT "v32tools: build ${V32T_SELECTED}")
    else()
        add_custom_target(v32t-${_a} COMMAND ${_cmd} USES_TERMINAL VERBATIM
                          COMMENT "v32tools: ${_a} ${V32T_SELECTED}")
    endif()
endforeach()

# per tool: <tool> (= build) and <tool>-<action>
foreach(_t IN LISTS V32T_SELECTED)
    foreach(_a IN LISTS V32T_ACTIONS)
        v32t_driver_command(_cmd ${_a} "${_t}" "${_explicit_prefix}")
        add_custom_target(${_t}-${_a} COMMAND ${_cmd} USES_TERMINAL VERBATIM)
    endforeach()
    v32t_driver_command(_cmd build "${_t}" "${_explicit_prefix}")
    add_custom_target(${_t} COMMAND ${_cmd} USES_TERMINAL VERBATIM)
endforeach()

# helpers
v32t_driver_command(_cmd flex "" "")
add_custom_target(v32t-flex COMMAND ${_cmd} USES_TERMINAL VERBATIM COMMENT "v32tools: build the vendored flex")
v32t_driver_command(_cmd doctor "" "")
add_custom_target(v32t-doctor COMMAND ${_cmd} USES_TERMINAL VERBATIM)
v32t_driver_command(_cmd list "" "")
add_custom_target(v32t-list COMMAND ${_cmd} USES_TERMINAL VERBATIM)

# `cmake --install build [--prefix DIR]` (and the generator's `install` target) = system install of the
# selected tools. The prefix is evaluated at install time so --prefix works; if it is still the
# platform default that configure picked, the tools' own defaults apply.
string(REPLACE ";" "," _sel_csv "${V32T_SELECTED}")
install(CODE "
    set(_prefix \"\${CMAKE_INSTALL_PREFIX}\")
    if(${V32T_PREFIX_DEFAULTED} AND \"\${_prefix}\" STREQUAL \"${CMAKE_INSTALL_PREFIX}\")
        set(_prefix \"\")
    endif()
    execute_process(COMMAND \"${CMAKE_COMMAND}\"
        \"-DV32T_ROOT=${CMAKE_CURRENT_SOURCE_DIR}\" -DV32T_ACTION=sysinstall
        \"-DV32T_TOOLS=${_sel_csv}\" \"-DV32T_BACKEND=${V32T_BACKEND}\"
        \"-DV32T_BUILD_TYPE=${V32T_BUILD_TYPE}\" \"-DV32T_BUILD_DIR=${CMAKE_BINARY_DIR}\"
        \"-DV32T_PREFIX=\${_prefix}\" \"-DV32T_JOBS=${V32T_JOBS}\" \"-DV32T_FLEX=${V32T_FLEX}\"
        \"-DV32T_MAKE_PROGRAM=${V32T_MAKE_PROGRAM}\" \"-DV32T_MAKE_ARGS=${V32T_MAKE_ARGS}\"
        \"-DV32T_CMAKE_ARGS=${V32T_CMAKE_ARGS}\" \"-DV32T_CMAKE_GENERATOR=${V32T_CMAKE_GENERATOR}\"
        \"-DV32T_SH=${V32T_SH}\"
        -P \"${CMAKE_CURRENT_SOURCE_DIR}/cmake/v32t-run.cmake\"
        RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR \"v32tools: system install failed\")
    endif()
" COMPONENT v32tools)

# ---- summary -------------------------------------------------------------------------------
message(STATUS "v32tools ${PROJECT_VERSION}")
message(STATUS "  tools:    ${V32T_SELECTED}")
message(STATUS "  backend:  ${V32T_BACKEND}   build type: ${V32T_BUILD_TYPE}")
if(V32T_PREFIX_DEFAULTED)
    message(STATUS "  prefix:   each tool's default (${CMAKE_INSTALL_PREFIX} unless it says otherwise)")
else()
    message(STATUS "  prefix:   ${CMAKE_INSTALL_PREFIX}")
endif()
message(STATUS "  make:     ${V32T_MAKE_PROGRAM}")
