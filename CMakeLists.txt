############################################################################## 
# blosc2-openzl: OpenZL plugin for Blosc2
#
# Copyright (c) 2026  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

cmake_minimum_required(VERSION 3.20)
project(blosc2_openzl LANGUAGES C)

include(GNUInstallDirs)
include(FetchContent)

message("Building Blosc2 plugin...")

# The following two steps are necessary when calling via cmake
find_package(Python REQUIRED COMPONENTS Interpreter) 

# Propagate CMAKE_OSX_ARCHITECTURES env variable into CMAKE_SYSTEM_PROCESSOR
if(DEFINED ENV{CMAKE_OSX_ARCHITECTURES})
    if("$ENV{CMAKE_OSX_ARCHITECTURES}" STREQUAL "arm64")
        set(CMAKE_SYSTEM_PROCESSOR arm64)
    endif()
endif()
# ============================================================
# Step 1: Find C-Blosc2
# ============================================================

# Find blosc2 package location using Python
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c "import blosc2, pathlib; print(pathlib.Path(blosc2.__file__).parent)"
    OUTPUT_VARIABLE BLOSC2_PACKAGE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE BLOSC2_IMPORT_RESULT
)

if(BLOSC2_IMPORT_RESULT EQUAL 0 AND EXISTS "${BLOSC2_PACKAGE_DIR}")
    message(STATUS "Found blosc2 package at: ${BLOSC2_PACKAGE_DIR}")

    # Try only PEP 427 compliant structure (blosc2/include/)
    set(BLOSC2_INCLUDE_DIR "${BLOSC2_PACKAGE_DIR}/include")
    if(NOT EXISTS "${BLOSC2_INCLUDE_DIR}/blosc2.h")
        message(FATAL_ERROR "blosc2 package found but blosc2.h not found. Tried:\n  ${BLOSC2_PACKAGE_DIR}/include")
    else()
        message(STATUS "Found Blosc2 include: ${BLOSC2_INCLUDE_DIR}/blosc2.h")
    endif()

    # Try only PEP 427 compliant structure (blosc2/lib/)
    set(BLOSC2_LIB_DIR "${BLOSC2_PACKAGE_DIR}/lib")
    if(NOT EXISTS "${BLOSC2_LIB_DIR}")
        message(FATAL_ERROR "blosc2 lib dir not found. Tried:\n  ${BLOSC2_PACKAGE_DIR}/lib")
    endif()

    find_library(BLOSC2_LIBRARY
        NAMES blosc2 libblosc2
        PATHS "${BLOSC2_LIB_DIR}"
        NO_DEFAULT_PATH
    )

    if(BLOSC2_LIBRARY)
        message(STATUS "Found Blosc2 library: ${BLOSC2_LIBRARY}")
        set(BLOSC2_LIBRARIES "${BLOSC2_LIBRARY}")
        get_filename_component(BLOSC2_LIBRARY_DIR_RESOLVED "${BLOSC2_LIBRARY}" DIRECTORY)
    else()
        message(FATAL_ERROR "Blosc2 library not found in ${BLOSC2_LIB_DIR}. Please install the blosc2 Python package with bundled libraries.")
    endif()
else()
    message(FATAL_ERROR "Could not import blosc2 package. Please install it first: pip install blosc2")
endif()

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
include_directories("${BLOSC2_INCLUDE_DIR}")
add_subdirectory(src)
