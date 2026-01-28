############################################################################## 
# blosc2-openzl: OpenZL plugin for Blosc2
#
# Copyright (c) 2026  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################


# ============================================================
# Step 2: Fetch OpenZL (C-only)
# ============================================================
# The following options would work if we were able to compile just the C-code from source
# However, due to dependencies, one is forced to compile everything if using OpenZL's CMakelists.txt
# workflow. This causes problems on Windows for xgboost.

# Disable everything non-C
# set(OPENZL_BUILD_ALL OFF CACHE BOOL "" FORCE)
# set(OPENZL_INSTALL OFF CACHE BOOL "" FORCE)
# set(OPENZL_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # build statically
# set(OPENZL_ALLOW_INTROSPECTION OFF CACHE BOOL "" FORCE)

# # NOTE: Disabling some but not all of these may break the build, use with caution.
# set(OPENZL_BUILD_CPP ON CACHE BOOL "" FORCE)
# set(OPENZL_BUILD_CUSTOM_PARSERS OFF CACHE BOOL "" FORCE)
# set(OPENZL_BUILD_TOOLS OFF CACHE BOOL "" FORCE) # <- even when set to OFF, still builds tools dir, which requires cpp
# and this builds XGBOOST, which is problematic on windows
# set(OPENZL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# set(OPENZL_BUILD_CLI OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    openzl
    GIT_REPOSITORY https://github.com/facebook/openzl.git
    GIT_TAG 4010c5f047441756092b039dcc8cfe7565a91d85 # pin exact version
)


FetchContent_GetProperties(openzl)
if(NOT openzl_POPULATED)
    FetchContent_Populate(openzl)
endif()

# Hard-code just compiling get C sources (basically copy-paste from OpenZL CMakelists.txt)
# ========================================================================================
set(CMAKE_MODULE_PATH
    "${openzl_SOURCE_DIR}/build-scripts/cmake"
    # For in-fbsource release builds
    "${openzl_SOURCE_DIR}/../../opensource/fbcode_builder/CMake"
    # For in-fbsource dev builds
    "${openzl_SOURCE_DIR}/../../../opensource/fbcode_builder/CMake"
    # For shipit-transformed builds
    "${openzl_SOURCE_DIR}/build-scripts/fbcode_builder/CMake"
    ${CMAKE_MODULE_PATH})

if (WIN32 AND (MSVC OR
              (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC") OR
              CMAKE_VS_PLATFORM_TOOLSET STREQUAL "ClangCL" OR
              CMAKE_GENERATOR_TOOLSET STREQUAL "ClangCL"))
    include(OpenZLCompilerMSVC)
else()
    include(OpenZLCompilerUnix)
endif()
include(OpenZLFunctions)

include(openzl-deps)
include(OpenZLConfigChecks)

configure_file(
    ${openzl_SOURCE_DIR}/build-scripts/cmake/zl_config.h.cmake
    ${openzl_BINARY_DIR}/include/openzl/zl_config.h
)
set(CONFIG "${openzl_BINARY_DIR}/include/openzl/zl_config.h")
set(OPENZL_SRC_DIR  ${openzl_SOURCE_DIR}/src)
set(OPENZL_INCLUDE_DIR  ${openzl_SOURCE_DIR}/include)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${OPENZL_SRC_DIR}/*.c")
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${OPENZL_SRC_DIR}/*.h")
file(GLOB_RECURSE public_headers CONFIGURE_DEPENDS "${OPENZL_INCLUDE_DIR}/*.h")

if (COMPILER_SUPPORT_AVX2 AND NOT MSVC)
    set_property(
        SOURCE
        ${OPENZL_SRC_DIR}/openzl/fse/decompress/huf_decompress_amd64.S
        APPEND PROPERTY COMPILE_OPTIONS "-x" "assembler-with-cpp"
    )
    list(APPEND sources ${OPENZL_SRC_DIR}/openzl/fse/decompress/huf_decompress_amd64.S)
endif()

list(APPEND openzl_files
    ${sources}
    ${headers}
)

add_library(openzl
    ${openzl_files}
    "${CONFIG}")

    # We need PIC to build shared libraries. For now just enable it for all builds.
# Later we could build both PIC & non-PIC versions of the library.
set_property(TARGET openzl PROPERTY POSITION_INDEPENDENT_CODE ON)

source_group(
    TREE ${openzl_SOURCE_DIR}
    PREFIX "openzl"
    FILES ${sources} ${headers} ${public_headers})
source_group(
    TREE ${openzl_BINARY_DIR}
    PREFIX "openzl"
    FILES "${CONFIG}")

target_include_directories(openzl_deps
    BEFORE
    INTERFACE
        $<BUILD_INTERFACE:${openzl_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${openzl_SOURCE_DIR}>
        $<BUILD_INTERFACE:${openzl_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${openzl_SOURCE_DIR}/src>
)

target_include_directories(openzl_deps
    INTERFACE
        $<INSTALL_INTERFACE:include>
)

target_include_directories(openzl
    PUBLIC
        $<TARGET_PROPERTY:openzl_deps,INTERFACE_INCLUDE_DIRECTORIES>
)

target_include_directories(openzl
    PUBLIC
        $<TARGET_PROPERTY:openzl_deps,INTERFACE_COMPILE_DEFINITIONS>
)

set_property(TARGET openzl PROPERTY VERSION ${PACKAGE_VERSION})
apply_openzl_compile_options_to_target(openzl)

target_link_libraries(openzl PUBLIC openzl_deps)
# ============================================================ end copy-paste

set(OPENZL_TARGET openzl)
# ============================================================
# Step 3: Build C extension for blosc2_openzl
# ============================================================
add_library(blosc2_openzl SHARED blosc2_openzl.c)
target_include_directories(blosc2_openzl PUBLIC ${BLOSC2_INCLUDE_DIR})
# link to openzl (not to blosc2, since blosc2_openzl always called via blosc2 as plugin)
# target_link_libraries(blosc2_openzl ${OPENZL_TARGET}) 

# if ever add blosc2 functions to blosc2_openzl, will need to link to blosc2 as well
# ----- and so will need to uncomment these lines -------
target_link_options(blosc2_openzl PRIVATE "-Wl,--enable-new-dtags") # need to force RUNPATH (not RPATH) for auditwheel repair by enabling new dtags
target_link_libraries(blosc2_openzl ${OPENZL_TARGET} ${BLOSC2_LIBRARIES}) 


# Add runtime path just in case (e.g. load plugin before blosc2 for example)
# will not always be used, but if can't find blosc2 gives another thing to check
# before failing
if(UNIX AND NOT APPLE)
    set_target_properties(blosc2_openzl PROPERTIES
        INSTALL_RPATH "$ORIGIN/../blosc2/lib"
    )
elseif(APPLE)
    set_target_properties(blosc2_openzl PROPERTIES
        INSTALL_RPATH "@loader_path/../blosc2/lib"
    )
endif()


# Install
install(TARGETS blosc2_openzl
    RUNTIME DESTINATION blosc2_openzl
    LIBRARY DESTINATION blosc2_openzl
)

# ============================================================
# Step 4: Executables
# ============================================================
if(NOT DEFINED ENV{DONT_BUILD_EXAMPLES})
    message(STATUS "DONT_BUILD_EXAMPLES not set-> Building examples")
    add_executable(test_openzl test_openzl.c)
    target_include_directories(test_openzl PRIVATE ${BLOSC2_INCLUDE_DIR})
    if(MSVC OR MINGW)
        target_link_libraries(test_openzl ${BLOSC2_LIBRARIES} ${OPENZL_TARGET})
    else()
        target_link_libraries(test_openzl ${BLOSC2_LIBRARIES} ${OPENZL_TARGET} m)
    endif()
    if(DEFINED BLOSC2_LIBRARY_DIR_RESOLVED)
        set_target_properties(test_openzl PROPERTIES
            BUILD_RPATH "${BLOSC2_LIBRARY_DIR_RESOLVED}"
            INSTALL_RPATH "${BLOSC2_LIBRARY_DIR_RESOLVED}"
        )
    endif()
else()
    message(STATUS "DONT_BUILD_EXAMPLES set")
endif()
