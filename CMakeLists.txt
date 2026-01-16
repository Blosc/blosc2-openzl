cmake_minimum_required(VERSION 3.20)
project(blosc2_openzl LANGUAGES C)

include(GNUInstallDirs)
include(FetchContent)

# ============================================================
# Step 1: Find C-Blosc2
# ============================================================
# Assume C-Blosc2 is installed in $HOME/.local
list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/.local")
find_package(Blosc2 REQUIRED)

message(STATUS "Found Blosc2 include dirs: ${Blosc2_INCLUDE_DIRS}")
message(STATUS "Found Blosc2 libraries: ${Blosc2_LIBRARIES}")

# ============================================================
# Step 2: Fetch OpenZL (C-only)
# ============================================================
# Disable everything non-C
set(OPENZL_BUILD_PYTHON_EXT OFF CACHE BOOL "" FORCE)
set(OPENZL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OPENZL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(OPENZL_INSTALL OFF CACHE BOOL "" FORCE)
set(OPENZL_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    openzl
    GIT_REPOSITORY https://github.com/facebook/openzl.git
    GIT_TAG 4010c5f047441756092b039dcc8cfe7565a91d85 # pin exact version
)
FetchContent_MakeAvailable(openzl)

# OpenZL target resolution
if(TARGET OpenZL::openzl)
    set(OPENZL_TARGET OpenZL::openzl)
elseif(TARGET openzl)
    set(OPENZL_TARGET openzl)
else()
    message(FATAL_ERROR "OpenZL target not found")
endif()

# ============================================================
# Step 3: blosc2_openzl plugin
# ============================================================

add_library(blosc2_openzl SHARED
    src/blosc2_openzl.c
)

target_include_directories(blosc2_openzl
    PRIVATE
        ${Blosc2_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(blosc2_openzl
    PRIVATE
        Blosc2::blosc2
        ${OPENZL_TARGET}
)

# Ensure symbols from blosc2_openzl_public.h are exported
if(UNIX AND NOT APPLE)
    target_compile_options(blosc2_openzl PRIVATE -fvisibility=hidden)
endif()

add_subdirectory(src)
