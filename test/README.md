# Ludwig Test Suite

This directory contains unit tests for the Ludwig text editor.

## Testing Framework

We use [Catch2](https://github.com/catchorg/Catch2) v3 as our testing framework. It is automatically downloaded via CMake's FetchContent during the build process.

## Building and Running Tests

### Build Tests

```bash
# From the project root
mkdir -p build && cd build
cmake ..
make
```

By default, tests are built automatically. To disable test building:

```bash
cmake -DBUILD_TESTING=OFF ..
```

### Run All Tests

```bash
# From the build directory
ctest

# Or with verbose output
ctest --output-on-failure --verbose

# Or using the custom target
make run_tests
```

### Run Tests Directly

```bash
# From the build directory
./test/ludwig_tests

# Run specific tests by tag
./test/ludwig_tests "[prange]"
./test/ludwig_tests "[str_object]"
./test/ludwig_tests "[const]"

# List all tests
./test/ludwig_tests --list-tests

# List all tags
./test/ludwig_tests --list-tags
```

## Adding New Tests

To add a new test file:

1. Create a new `.cpp` file in the `test/` directory
2. Include the Catch2 header: `#include <catch2/catch_test_macros.hpp>`
3. Add your test cases using the `TEST_CASE` macro
4. Add the filename to `TEST_SOURCES` in `test/CMakeLists.txt`
5. Rebuild

Example:

```cpp
#include <catch2/catch_test_macros.hpp>
#include "your_module.h"

TEST_CASE("Description of what you're testing", "[tag]") {
    SECTION("specific aspect") {
        REQUIRE(some_function() == expected_value);
    }
}
```

## Test Structure

Tests are organized using Catch2's `TEST_CASE` and `SECTION` macros:

- **TEST_CASE**: Top-level test with a description and tags
- **SECTION**: Sub-tests within a test case (each section runs independently)
- **REQUIRE**: Assertion that must pass (test stops on failure)
- **CHECK**: Assertion that is checked but doesn't stop the test

## Available Assertions

- `REQUIRE(expr)` - Must be true, stops test on failure
- `REQUIRE_FALSE(expr)` - Must be false
- `REQUIRE_THROWS(expr)` - Must throw an exception
- `REQUIRE_NOTHROW(expr)` - Must not throw
- `CHECK(expr)` - Should be true, continues on failure
- Plus many more - see [Catch2 documentation](https://github.com/catchorg/Catch2/blob/devel/docs/assertions.md)

## Continuous Integration

Tests should be run on every commit. Consider setting up GitHub Actions or similar CI to automatically run tests on push/PR.

## Debugging Tests

This is probably easiest in vscode.  You can create a `tasks.json` entry similar to the following:

```json
{
    "label": "build-debug",
    "type": "shell",
    "command": "cd Debug && make",
    "group": "build",
    "problemMatcher": []
}
```

This assumes that your CMake build directory is `Debug`.  Flavour to suit.

Then you will need a configuration in `launch.json`:

```json
{
    "name": "Launch Tests",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceFolder}/build/test/ludwig_tests",
    "args": [],
    "stopAtEntry": false,
    "cwd": "${workspaceFolder}/build",
    "environment": [
        {
            "name": "TERM",
            "value": "vt100"
        }
    ],
    "externalConsole": false,
    "MIMode": "lldb",
    "preLaunchTask": "build-debug"
}
```

Then use the debug window (Cmd-Shift-D) to run the launch configuration.

### From the command line

To debug a failing test:

```bash
# Run with verbose output
./test/ludwig_tests -v

# Run a specific test case
./test/ludwig_tests "test case name"

# Break on first failure
./test/ludwig_tests --abort

# Use a debugger
gdb ./test/ludwig_tests
lldb ./test/ludwig_tests
```

## Resources

- [Catch2 Documentation](https://github.com/catchorg/Catch2/tree/devel/docs)
- [Catch2 Tutorial](https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md)
- [CMake Testing](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
