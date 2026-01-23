############################################################################## 
# blosc2-openzl: OpenZL plugin for Blosc2
#
# Copyright (c) 2026  The Blosc Development Team <blosc@blosc.org>
# https://blosc.org
# License: GNU Affero General Public License v3.0 (see LICENSE.txt)
##############################################################################

# Build statically
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build C extension for blosc2_openzl
add_library(blosc2_openzl SHARED
    blosc2_openzl.c
)

target_include_directories(blosc2_openzl
    PRIVATE
        ${BLOSC2_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(blosc2_openzl
    PRIVATE
        ${BLOSC2_LIBRARIES}
        ${OPENZL_TARGET}
)

# Set RPATH for locating libblosc2 at runtime
if(DEFINED BLOSC2_LIBRARY_DIR_RESOLVED)
    set_target_properties(blosc2_openzl PROPERTIES
        BUILD_RPATH "${BLOSC2_LIBRARY_DIR_RESOLVED}"
    )
endif()

set(_blosc2_runtime_rpaths "")
if(APPLE)
    list(APPEND _blosc2_runtime_rpaths
        "@loader_path/../blosc2/lib"
    )
elseif(UNIX)
    list(APPEND _blosc2_runtime_rpaths
        "$ORIGIN/../blosc2/lib"
    )
endif()

if(_blosc2_runtime_rpaths)
    set_target_properties(blosc2_openzl PROPERTIES
        INSTALL_RPATH "${_blosc2_runtime_rpaths}"
    )
endif()
unset(_blosc2_runtime_rpaths)

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
