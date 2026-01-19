cmake_minimum_required(VERSION 3.20)
project(blosc2_openzl LANGUAGES C)

include(GNUInstallDirs)
include(FetchContent)

# ============================================================
# Step 1: Find C-Blosc2
# ============================================================
# Find blosc2.h
find_package(Python COMPONENTS Interpreter NumPy Development.Module REQUIRED)
if (UNIX)
    cmake_path(SET Python_ROOT NORMALIZE "${Python_NumPy_INCLUDE_DIRS}/../../..")
else()
    cmake_path(SET Python_ROOT NORMALIZE "${Python_NumPy_INCLUDE_DIRS}/../../..")
endif()
cmake_path(SET Python_INCLUDE NORMALIZE "${Python_ROOT}/include")
message(STATUS "Found Python include: ${Python_ROOT}")
cmake_path(SET Python_LIB NORMALIZE "${Python_ROOT}/lib")
cmake_path(SET Python_LIB64 NORMALIZE "${Python_ROOT}/lib64")
cmake_path(SET Python_Blosc2_INCLUDE NORMALIZE "${Python_ROOT}/include/blosc2.h")
cmake_path(HAS_FILENAME Python_Blosc2_INCLUDE HAS_BLOSC2)
if(HAS_BLOSC2)
    set(BLOSC2_INCLUDE_DIR ${Python_INCLUDE})
    message(STATUS "Found Blosc2 include: ${Python_Blosc2_INCLUDE}")
else()
    message(FATAL_ERROR "No Blosc2 includes found.  Aborting.")
endif()

include_directories("${BLOSC2_INCLUDE_DIR}")
# Link actual blosc2 binaries
link_directories(${Python_LIB})
if (UNIX AND NOT APPLE)
    link_directories(${Python_LIB64})
endif (UNIX AND NOT APPLE)

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

add_subdirectory(src)
