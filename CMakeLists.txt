cmake_minimum_required(VERSION 3.31) # Use a modern CMake version
project(Ludwig VERSION 1.0 LANGUAGES CXX)

# --- General Configuration ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Prefer standard-compliant C++

# Define source and include directories for clarity
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add global compile options.
add_compile_options(-Wall -Wextra -Wno-tautological-pointer-compare)

# --- Debug/Release Specific Flags ---
# CMake automatically sets -g for Debug and -O3 for Release by default.
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -DDEBUG -D_GLIBCXX_DEBUG -fsanitize=address,undefined -O1 -g -fno-omit-frame-pointer")

# --- Find External Dependencies ---
find_package(Curses REQUIRED)

# --- Build the Help File Indexer: ludwighlpbld ---
set(HLPBLD_SOURCE_FILE ${SOURCE_DIR}/ludwighlpbld.cpp)
add_executable(ludwighlpbld ${HLPBLD_SOURCE_FILE})

# --- Generate Index Files ---
set(HELP_T_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/ludwighlp.t
    ${CMAKE_CURRENT_SOURCE_DIR}/ludwignewhlp.t
)

set(GENERATED_IDX_FILES "")
foreach(T_FILE IN LISTS HELP_T_FILES)
    get_filename_component(T_BASENAME ${T_FILE} NAME_WE)
    set(IDX_FILE ${CMAKE_CURRENT_BINARY_DIR}/${T_BASENAME}.idx)
    list(APPEND GENERATED_IDX_FILES ${IDX_FILE})

    add_custom_command(
        OUTPUT ${IDX_FILE}
        COMMAND $<TARGET_FILE:ludwighlpbld> ${T_FILE} ${IDX_FILE}
        DEPENDS ${T_FILE} ludwighlpbld
        VERBATIM
    )
endforeach()

add_custom_target(generate_help_indices ALL DEPENDS ${GENERATED_IDX_FILES})


# --- Build the Main Program: ludwig ---

# Find all .cpp files in the src directory, excluding ludwighlpbld.cpp
file(GLOB_RECURSE LUDWIG_SOURCES "${SOURCE_DIR}/*.cpp")
list(REMOVE_ITEM LUDWIG_SOURCES ${HLPBLD_SOURCE_FILE})

add_executable(ludwig ${LUDWIG_SOURCES})

# Ensure ludwig executable depends on the generated files.
add_dependencies(ludwig generate_help_indices)

# Link ncurses library to the main program.
target_include_directories(ludwig PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(ludwig PRIVATE ${CURSES_LIBRARIES})

# --- Testing ---
option(BUILD_TESTING "Build the testing tree" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
