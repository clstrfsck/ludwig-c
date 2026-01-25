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

# Code coverage support
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)

if(ENABLE_COVERAGE)
    # Add coverage flags to the library and tests
    target_compile_options(ludwig_lib PUBLIC --coverage -O0 -g)
    target_link_options(ludwig_lib PUBLIC --coverage)

    message(STATUS "Code coverage enabled")
endif()

# Test executables
set(TEST_SOURCES
    test_const.cpp
    test_helpfile.cpp
    test_main.cpp
    test_mark.cpp
    test_parray.cpp
    test_prange.cpp
)

add_executable(ludwig_tests ${TEST_SOURCES})
target_compile_definitions(ludwig_tests PRIVATE
    TEST_DATA_PATH="${CMAKE_CURRENT_BINARY_DIR}/../"
)
target_link_libraries(ludwig_tests PRIVATE
    ludwig_lib
    Catch2::Catch2WithMain
)

# Enable testing
enable_testing()

# Code coverage target
if(ENABLE_COVERAGE)
    # Find coverage tools
    find_program(GCOVR_PATH gcovr)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(GCOVR_PATH)
        # Add coverage target using gcovr (simpler, works with both GCC and Clang)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_COMMAND} -E echo "Running tests to generate coverage data..."
            COMMAND $<TARGET_FILE:ludwig_tests>
            COMMAND ${CMAKE_COMMAND} -E echo "Generating coverage report..."
            COMMAND ${GCOVR_PATH}
                --root ${CMAKE_SOURCE_DIR}
                --filter '${CMAKE_SOURCE_DIR}/src/.*'
                --exclude '${CMAKE_SOURCE_DIR}/src/ludwighlpbld.cpp'
                --exclude '${CMAKE_SOURCE_DIR}/src/ludwig.cpp'
                --html --html-details
                --output ${CMAKE_BINARY_DIR}/coverage/index.html
                --print-summary
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated: ${CMAKE_BINARY_DIR}/coverage/index.html"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ludwig_tests
            COMMENT "Generating code coverage report with gcovr"
        )

        # Also add a text-only coverage summary target
        add_custom_target(coverage-summary
            COMMAND ${CMAKE_COMMAND} -E echo "Running tests to generate coverage data..."
            COMMAND $<TARGET_FILE:ludwig_tests>
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage Summary:"
            COMMAND ${GCOVR_PATH}
                --root ${CMAKE_SOURCE_DIR}
                --filter '${CMAKE_SOURCE_DIR}/src/.*'
                --exclude '${CMAKE_SOURCE_DIR}/src/ludwighlpbld.cpp'
                --exclude '${CMAKE_SOURCE_DIR}/src/ludwig.cpp'
                --print-summary
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ludwig_tests
            COMMENT "Displaying code coverage summary"
        )
    elseif(LCOV_PATH AND GENHTML_PATH)
        # Fall back to lcov/genhtml
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_COMMAND} -E echo "Clearing previous coverage data..."
            COMMAND ${LCOV_PATH} --directory . --zerocounters
            COMMAND ${CMAKE_COMMAND} -E echo "Running tests to generate coverage data..."
            COMMAND $<TARGET_FILE:ludwig_tests>
            COMMAND ${CMAKE_COMMAND} -E echo "Capturing coverage data..."
            COMMAND ${LCOV_PATH} --directory . --capture --output-file ${CMAKE_BINARY_DIR}/coverage.info
            COMMAND ${LCOV_PATH} --remove ${CMAKE_BINARY_DIR}/coverage.info
                '/usr/*' '*/test/*' '*/_deps/*'
                '${CMAKE_SOURCE_DIR}/src/ludwighlpbld.cpp'
                '${CMAKE_SOURCE_DIR}/src/ludwig.cpp'
                --output-file ${CMAKE_BINARY_DIR}/coverage_filtered.info
            COMMAND ${CMAKE_COMMAND} -E echo "Generating HTML report..."
            COMMAND ${GENHTML_PATH} ${CMAKE_BINARY_DIR}/coverage_filtered.info
                --output-directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated: ${CMAKE_BINARY_DIR}/coverage/index.html"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ludwig_tests
            COMMENT "Generating code coverage report with lcov"
        )
    else()
        message(WARNING "Neither gcovr nor lcov/genhtml found. Coverage target will not be available. Install with: brew install gcovr")
    endif()
endif()

# Discover tests
catch_discover_tests(ludwig_tests)

# Optional: Add a custom target to run tests with verbose output
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS ludwig_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running Ludwig tests with verbose output"
)
