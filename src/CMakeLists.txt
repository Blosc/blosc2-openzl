# ============================================================
# Step 4: Executables
# ============================================================

add_executable(test_openzl test_openzl.c)
target_link_libraries(test_openzl PRIVATE blosc2_openzl ${OPENZL_TARGET} Blosc2::blosc2)
target_include_directories(test_openzl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(roundtrip roundtrip.c)
target_link_libraries(roundtrip PRIVATE blosc2_openzl ${OPENZL_TARGET} Blosc2::blosc2)
target_include_directories(roundtrip PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# RPATH so executables find blosc2_openzl locally
set_target_properties(test_openzl roundtrip PROPERTIES BUILD_RPATH "$ORIGIN")
set_target_properties(blosc2_openzl PROPERTIES BUILD_RPATH "$ORIGIN")