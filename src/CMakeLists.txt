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
        blosc2
        ${OPENZL_TARGET}
)

# Install
install(TARGETS blosc2_openzl LIBRARY DESTINATION blosc2_openzl)
install(FILES blosc2_openzl.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT DEV)

# ============================================================
# Step 4: Executables
# ============================================================

add_executable(test_openzl test_openzl.c)
target_link_libraries(test_openzl PRIVATE blosc2_openzl ${OPENZL_TARGET} blosc2)
target_include_directories(test_openzl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(roundtrip roundtrip.c)
target_link_libraries(roundtrip PRIVATE blosc2_openzl ${OPENZL_TARGET} blosc2)
target_include_directories(roundtrip PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# RPATH so executables find blosc2_openzl locally
set_target_properties(test_openzl roundtrip PROPERTIES BUILD_RPATH "$ORIGIN")
set_target_properties(blosc2_openzl PROPERTIES BUILD_RPATH "$ORIGIN")