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
# Disable everything non-C
set(OPENZL_BUILD_PYTHON_EXT OFF CACHE BOOL "" FORCE)
set(OPENZL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OPENZL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(OPENZL_INSTALL OFF CACHE BOOL "" FORCE)
set(OPENZL_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE) # build statically

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
# Step 3: Build C extension for blosc2_openzl
# ============================================================
add_library(blosc2_openzl SHARED blosc2_openzl.c)
target_include_directories(blosc2_openzl PUBLIC ${BLOSC2_INCLUDE_DIR})
# link to openzl (not to blosc2, since blosc2_openzl always called via blosc2 as plugin)
target_link_libraries(blosc2_openzl ${OPENZL_TARGET}) 

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
