# Test CMakeLists.txt for Ludwig

# Download and configure Catch2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.12.0 # Use a stable version
)
FetchContent_MakeAvailable(Catch2)

# Add Catch2 CMake modules to the module path
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

# Include CTest for testing support
include(CTest)
include(Catch)

# Create a library from Ludwig sources (excluding main) for testing
file(GLOB_RECURSE LUDWIG_LIB_SOURCES "${SOURCE_DIR}/*.cpp")
list(REMOVE_ITEM LUDWIG_LIB_SOURCES
    ${SOURCE_DIR}/ludwighlpbld.cpp
    ${SOURCE_DIR}/ludwig.cpp  # Exclude main entry point
)

add_library(ludwig_lib STATIC ${LUDWIG_LIB_SOURCES})
target_include_directories(ludwig_lib PUBLIC ${SOURCE_DIR})
target_link_libraries(ludwig_lib PUBLIC ${CURSES_LIBRARIES})
target_include_directories(ludwig_lib PUBLIC ${CURSES_INCLUDE_DIRS})

# Test executables
set(TEST_SOURCES
    test_main.cpp
    test_prange.cpp
    test_parray.cpp
    test_const.cpp
)

add_executable(ludwig_tests ${TEST_SOURCES})
target_link_libraries(ludwig_tests PRIVATE
    ludwig_lib
    Catch2::Catch2WithMain
)

# Enable testing
enable_testing()

# Discover tests
catch_discover_tests(ludwig_tests)

# Optional: Add a custom target to run tests with verbose output
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS ludwig_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running Ludwig tests with verbose output"
)
