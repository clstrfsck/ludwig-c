/**
 * @file test_code.cpp
 * Unit and integration tests for code module (compiler and interpreter)
 *
 * LIMITATIONS:
 * ============
 * code.cpp is not really well-designed for unit testing.  It is more-or-less
 * a transliteration of the original Pascal code, and relies heavily on global
 * state and free functions, which makes it a bit awkward to test in isolation.
 *
 * The integration tests initialize the required global state to allow
 * testing of compilation and interpretation.
 *
 * I've tried to focus on testing the core logic of compilation and
 * interpreting.  There is still quite a bit of untested code, especially
 * around vdu and screen interactions.
 */

#include "code.h"
#include "const.h"
#include "line.h"
#include "mark.h"
#include "type.h"
#include "value.h"
#include "var.h"

#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <vector>

namespace {

/**
 * Initialize the minimal global state required for code compilation and interpretation.
 * This sets up:
 * - code_list: The linked list header for compiled code blocks
 * - code_top: The index tracking compiled code storage
 * - cmd_attrib: Command attributes (via value_initializations)
 * - lookup: Command lookup table (character to command mapping)
 * - Various global flags
 */
void init_code_globals() {
    // Initialize command table and other values
    value_initializations();

    // Initialize the lookup table
    load_command_table(true);

    // Initialize code compiler state
    code_top = 0;

    // Create the code list header (circular doubly-linked list)
    code_list = new code_header;
    code_list->flink = code_list;
    code_list->blink = code_list;
    code_list->ref = 1;
    code_list->code = 0;
    code_list->len = 0;

    // Clear flags that might affect interpretation
    exit_abort = false;
    tt_controlc = false;
}

/**
 * Clean up global state after tests.
 */
void cleanup_code_globals() {
    // Delete code_list if allocated
    if (code_list != nullptr) {
        delete code_list;
        code_list = nullptr;
    }
    code_top = 0;
}

/**
 * Create a line with specified text content.
 * Returns nullptr on failure.
 */
line_ptr create_line_with_text(const char *text) {
    line_ptr line = nullptr;
    line_ptr dummy = nullptr;

    if (!lines_create(1, line, dummy)) {
        return nullptr;
    }

    int len = std::strlen(text);
    if (len > 0) {
        if (!line_change_length(line, len)) {
            lines_destroy(line, line);
            return nullptr;
        }
        // Copy text into line
        for (int i = 0; i < len; ++i) {
            (*line->str)[i + 1] = text[i];
        }
        line->used = len;
    } else {
        line->used = 0;
    }

    return line;
}

/**
 * Create a possibly multi-line span containing the given command text lines.
 * Lines are linked together and marks are placed at start and end.
 */
bool create_test_span(
    span_object &span,
    std::initializer_list<const char *> lines_text
) {
    if (lines_text.size() == 0) {
        return false;
    }

    line_ptr first_line = nullptr;
    line_ptr last_line = nullptr;
    int last_line_len = 0;

    for (const char *text : lines_text) {
        line_ptr line = create_line_with_text(text);
        if (line == nullptr) {
            // Clean up any lines already created
            if (first_line != nullptr) {
                lines_destroy(first_line, last_line);
            }
            return false;
        }

        if (first_line == nullptr) {
            first_line = line;
            last_line = line;
        } else {
            // Link lines together
            last_line->flink = line;
            line->blink = last_line;
            last_line = line;
        }
        last_line_len = std::strlen(text);
    }

    // Create marks at start and end
    if (!mark_create(first_line, 1, span.mark_one)) {
        lines_destroy(first_line, last_line);
        return false;
    }

    if (!mark_create(last_line, last_line_len + 1, span.mark_two)) {
        mark_destroy(span.mark_one);
        lines_destroy(first_line, last_line);
        return false;
    }

    // Initialize span fields
    span.flink = nullptr;
    span.blink = nullptr;
    span.frame = nullptr;
    span.code = nullptr;
    span.name = "test";

    return true;
}

/**
 * Clean up a multi-line test span and its associated resources.
 */
void destroy_test_span(span_object &span) {
    // Get the first and last lines from the marks
    line_ptr first_line = nullptr;
    line_ptr last_line = nullptr;

    if (span.mark_one != nullptr) {
        first_line = span.mark_one->line;
    }
    if (span.mark_two != nullptr) {
        last_line = span.mark_two->line;
    }

    // Destroy marks
    if (span.mark_one != nullptr) {
        mark_destroy(span.mark_one);
    }
    if (span.mark_two != nullptr) {
        mark_destroy(span.mark_two);
    }

    // Destroy all lines from first to last
    if (first_line != nullptr && last_line != nullptr) {
        lines_destroy(first_line, last_line);
    }

    // Destroy compiled code if present
    if (span.code != nullptr) {
        code_discard(span.code);
    }
}

/**
 * Mock execute function that records commands for verification.
 */
struct ExecutionRecord {
    commands cmd;
    leadparam lp;
    int cnt;

    bool operator==(const ExecutionRecord &other) const {
        return (cmd == other.cmd) &&
               (lp == other.lp) &&
               (cnt == other.cnt);
    }
};

std::vector<ExecutionRecord> g_execution_log;

// Macro to check execution log against expected records.  A macro rather than
// a function to allow caller's line numbers to be output on failure.
#define CHECK_EXECUTION_LOG(...)                                         \
    do {                                                                 \
        std::initializer_list<ExecutionRecord> expected = {__VA_ARGS__}; \
        REQUIRE(g_execution_log.size() == expected.size());              \
        size_t i = 0;                                                    \
        for (const auto &e : expected) {                                 \
            INFO("Mismatch at index " << i);                             \
            REQUIRE(g_execution_log[i] == e);                            \
            ++i;                                                         \
        }                                                                \
    } while (0)

bool mock_execute(commands cmd, leadparam lp, int cnt, tpar_ptr /*tpar*/, bool /*from_span*/) {
    g_execution_log.push_back({cmd, lp, cnt});
    return true; // Always succeed
}

/**
 * Mock execute function that fails on specific commands.
 */
commands g_fail_on_command = commands::cmd_noop;

bool mock_execute_with_failure(
    commands cmd,
    leadparam lp,
    int cnt,
    tpar_ptr /*tpar*/,
    bool /*from_span*/
) {
    g_execution_log.push_back({cmd, lp, cnt});
    return (cmd != g_fail_on_command);
}

} // anonymous namespace

// Catch2 StringMaker specialization for better error messages
template <>
struct Catch::StringMaker<ExecutionRecord> {
    static std::string convert(const ExecutionRecord &rec) {
        return "{cmd=" + std::to_string(static_cast<int>(rec.cmd)) +
               ", lp=" + std::to_string(static_cast<int>(rec.lp)) +
               ", cnt=" + std::to_string(rec.cnt) + "}";
    }
};

// ============================================================================

TEST_CASE("code module function signatures exist", "[code][api]") {
    SECTION("code_compile function pointer can be taken") {
        // Verifies function exists and signature is correct
        bool (*func_ptr)(span_object &, bool) = &code_compile;
        REQUIRE(func_ptr != nullptr);
    }

    SECTION("code_interpret function pointer can be taken") {
        // Verifies function exists and signature is correct
        bool (*func_ptr)(leadparam, int, code_ptr, bool) = &code_interpret;
        REQUIRE(func_ptr != nullptr);
    }

    SECTION("code_discard function pointer can be taken") {
        // Verifies function exists and signature is correct
        void (*func_ptr)(code_ptr &) = &code_discard;
        REQUIRE(func_ptr != nullptr);
    }

    SECTION("code_interpret_execute function pointer can be taken") {
        // Verifies testable function exists and signature is correct
        bool (*func_ptr)(execute_fn, leadparam, int, code_ptr, bool) = &code_interpret_execute;
        REQUIRE(func_ptr != nullptr);
    }
}

TEST_CASE("code_header structure is accessible", "[code][types]") {
    SECTION("can create pointer to code_header") {
        code_ptr ptr = nullptr;

        // Verifies code_header type is defined
        // (code_ptr is typedef for code_header*)
        REQUIRE(sizeof(ptr) == sizeof(void *));
    }
}

TEST_CASE("code_compile basic commands", "[code][compile][integration]") {
    init_code_globals();

    SECTION("compiles single advance command") {
        span_object span;
        REQUIRE(create_test_span(span, {"A"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);
        REQUIRE(span.code->len > 0);

        destroy_test_span(span);
    }

    SECTION("compiles advance with count") {
        span_object span;
        REQUIRE(create_test_span(span, {"5A"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);
        REQUIRE(span.code->len > 0);

        destroy_test_span(span);
    }

    SECTION("compiles delete_char command") {
        span_object span;
        REQUIRE(create_test_span(span, {"D"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles multiple commands") {
        span_object span;
        REQUIRE(create_test_span(span, {"AD"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles command with leading plus") {
        span_object span;
        REQUIRE(create_test_span(span, {"+A"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles command with leading plus and number") {
        span_object span;
        REQUIRE(create_test_span(span, {"+5A"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles command with leading minus") {
        span_object span;
        REQUIRE(create_test_span(span, {"-A"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles command with leading minus and number") {
        span_object span;
        REQUIRE(create_test_span(span, {"-5A"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles command with indefinite forward") {
        span_object span;
        REQUIRE(create_test_span(span, {">A"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles command with indefinite backward") {
        span_object span;
        REQUIRE(create_test_span(span, {"<A"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_compile with trailing parameters", "[code][compile][integration]") {
    init_code_globals();

    SECTION("compiles insert command with text") {
        span_object span;
        REQUIRE(create_test_span(span, {"I/hello/"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles get command with pattern") {
        span_object span;
        REQUIRE(create_test_span(span, {"G/test/"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles replace command with two parameters") {
        span_object span;
        REQUIRE(create_test_span(span, {"R/old/new/"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_compile error handling", "[code][compile][integration]") {
    init_code_globals();

    SECTION("rejects empty span") {
        span_object span;
        REQUIRE(create_test_span(span, {""}));

        bool result = code_compile(span, true);
        REQUIRE(result == false);

        destroy_test_span(span);
    }

    SECTION("rejects invalid command character") {
        span_object span;
        REQUIRE(create_test_span(span, {"ZQ"}));

        REQUIRE(code_compile(span, true) == false);

        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_compile compound commands", "[code][compile][integration]") {
    init_code_globals();

    SECTION("compiles parenthesized command") {
        span_object span;
        REQUIRE(create_test_span(span, {"(A)"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles repeated parenthesized command") {
        span_object span;
        REQUIRE(create_test_span(span, {"5(a)"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("fails with negative repeat count") {
        span_object span;
        REQUIRE(create_test_span(span, {"-5(a)"}));

        bool result = code_compile(span, true);
        REQUIRE(result == false);
        REQUIRE(span.code == nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles nested parentheses") {
        span_object span;
        REQUIRE(create_test_span(span, {"(3(a))"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles commands with success and failure branches") {
        span_object span;
        REQUIRE(create_test_span(span, {"A[=J:I/didn't work/]"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles commands with nested branches") {
        span_object span;
        REQUIRE(create_test_span(span, {"A[=J:D[=J:I/inner/]I/outer/]"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("compiles commands with comments") {
        span_object span;
        REQUIRE(create_test_span(span, {"A! Advance one line ZZQ <- Not a valid command"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_compile multi-line spans", "[code][compile][multiline][integration]") {
    init_code_globals();

    SECTION("compiles commands spanning two lines") {
        span_object span;
        REQUIRE(create_test_span(span, {"A", "D"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        // Verify both commands execute
        g_execution_log.clear();
        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true));
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_delete_char, leadparam::none, 1}
        );

        destroy_test_span(span);
    }

    SECTION("compiles commands spanning three lines") {
        span_object span;
        REQUIRE(create_test_span(span, {"A", "D", "K"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        g_execution_log.clear();
        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true));
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_delete_char, leadparam::none, 1},
            {commands::cmd_delete_line, leadparam::none, 1}
        );

        destroy_test_span(span);
    }

    SECTION("compiles multi-line with multiple commands per line") {
        span_object span;
        REQUIRE(create_test_span(span, {"AA", "DD"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        g_execution_log.clear();
        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true));
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_delete_char, leadparam::none, 1},
            {commands::cmd_delete_char, leadparam::none, 1}
        );

        destroy_test_span(span);
    }

    SECTION("compiles parenthesized command spanning lines") {
        span_object span;
        // Opening paren on first line, content on second, closing on third
        REQUIRE(create_test_span(span, {"3(", "A", ")"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        g_execution_log.clear();
        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true));
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1}
        );

        destroy_test_span(span);
    }

    SECTION("compiles trailing parameter spanning lines") {
        span_object span;
        // Insert command with text on next line
        REQUIRE(create_test_span(span, {"I/hello", "/"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);
        REQUIRE(span.code != nullptr);

        destroy_test_span(span);
    }

    SECTION("handles whitespace at line boundaries") {
        span_object span;
        // Line boundary is treated as space
        REQUIRE(create_test_span(span, {"A", "D"}));

        bool result = code_compile(span, true);
        REQUIRE(result == true);

        // Commands should work - line boundary acts as separator
        g_execution_log.clear();
        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true));
        REQUIRE(g_execution_log.size() == 2);

        destroy_test_span(span);
    }

    SECTION("handles errors on subsequent lines") {
        span_object span;
        // Line boundary is treated as space
        REQUIRE(create_test_span(span, {"I/foo", ""})); // Missing closing '/'

        bool result = code_compile(span, true);
        REQUIRE(result == false);

        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_interpret_execute basic commands", "[code][interpret][integration]") {
    init_code_globals();
    g_execution_log.clear();

    SECTION("interprets single advance command") {
        span_object span;
        REQUIRE(create_test_span(span, {"A"}));
        REQUIRE(code_compile(span, true));

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG({commands::cmd_advance, leadparam::none, 1});

        destroy_test_span(span);
    }

    SECTION("interprets advance with count") {
        span_object span;
        REQUIRE(create_test_span(span, {"5A"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG({commands::cmd_advance, leadparam::pint, 5});

        destroy_test_span(span);
    }

    SECTION("interprets delete_char command") {
        span_object span;
        REQUIRE(create_test_span(span, {"D"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG({commands::cmd_delete_char, leadparam::none, 1});

        destroy_test_span(span);
    }

    SECTION("interprets multiple commands in sequence") {
        span_object span;
        REQUIRE(create_test_span(span, {"AAD"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_delete_char, leadparam::none, 1}
        );
        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_interpret_execute with repeat count", "[code][interpret][integration]") {
    init_code_globals();
    g_execution_log.clear();

    SECTION("executes command multiple times with outer count") {
        span_object span;
        REQUIRE(create_test_span(span, {"A"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        // Execute the command 3 times
        bool result = code_interpret_execute(mock_execute, leadparam::pint, 3, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1}
        );
        destroy_test_span(span);
    }

    SECTION("zero count executes zero times") {
        span_object span;
        REQUIRE(create_test_span(span, {"A"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::pint, 0, span.code, true);
        REQUIRE(result == true);
        REQUIRE(g_execution_log.empty());

        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_interpret_execute handles null code_head", "[code][interpret][integration]") {
    init_code_globals();
    g_execution_log.clear();

    SECTION("returns false for null code pointer") {
        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, nullptr, true);
        REQUIRE(result == false);
    }

    cleanup_code_globals();
}

TEST_CASE("code_interpret_execute with failing commands", "[code][interpret][integration]") {
    init_code_globals();
    g_execution_log.clear();

    SECTION("stops on command failure") {
        span_object span;
        REQUIRE(create_test_span(span, {"AAD"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        // Set up to fail on left command
        g_fail_on_command = commands::cmd_delete_char;

        bool result = code_interpret_execute(
            mock_execute_with_failure,
            leadparam::none,
            1,
            span.code,
            true
        );
        // The interpretation should fail because delete failed
        REQUIRE(result == false);
        // But advance commands should have executed
        REQUIRE(g_execution_log.size() >= 2);

        g_fail_on_command = commands::cmd_noop;
        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_interpret_execute parenthesized commands", "[code][interpret][integration]") {
    init_code_globals();
    g_execution_log.clear();

    SECTION("executes parenthesized command with count") {
        span_object span;
        REQUIRE(create_test_span(span, {"3(A)"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1}
        );
        destroy_test_span(span);
    }

    SECTION("executes nested parenthesized commands") {
        span_object span;
        REQUIRE(create_test_span(span, {"2(2(a))"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG(
            // 2 x 2 executions of 'a'
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_advance, leadparam::none, 1}
        );

        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_interpret_execute control commands", "[code][interpret][integration]") {
    init_code_globals();
    g_execution_log.clear();

    SECTION("executes failure command") {
        span_object span;
        REQUIRE(create_test_span(span, {"(xf)[1a:2a]"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::pint, 2}
        );

        destroy_test_span(span);
    }

    SECTION("executes success command") {
        span_object span;
        REQUIRE(create_test_span(span, {"(xs)[1a:2a]"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::pint, 1}
        );

        destroy_test_span(span);
    }

    SECTION("executes abort command") {
        span_object span;
        REQUIRE(create_test_span(span, {"(xa)[a:a]"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == false);
        REQUIRE(g_execution_log.empty());

        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_discard reference counting", "[code][discard][integration]") {
    init_code_globals();

    SECTION("null pointer does nothing") {
        code_ptr code = nullptr;
        code_discard(code);
        REQUIRE(code == nullptr);
    }

    SECTION("decrements reference count") {
        span_object span;
        REQUIRE(create_test_span(span, {"A"}));
        REQUIRE(code_compile(span, true));

        // Increment ref count manually to simulate multiple users
        span.code->ref += 1;
        REQUIRE(span.code->ref == 2);

        // First discard should decrement but not delete
        code_discard(span.code);
        REQUIRE(span.code != nullptr);
        REQUIRE(span.code->ref == 1);

        // Second discard should delete
        code_discard(span.code);
        REQUIRE(span.code == nullptr);

        // Clean up span without code (already discarded)
        span.code = nullptr;
        destroy_test_span(span);
    }

    SECTION("single reference causes deletion") {
        span_object span;
        REQUIRE(create_test_span(span, {"A"}));
        REQUIRE(code_compile(span, true));
        REQUIRE(span.code->ref == 1);

        code_discard(span.code);
        REQUIRE(span.code == nullptr);

        // Clean up span without code
        span.code = nullptr;
        destroy_test_span(span);
    }

    SECTION("compacts code indices when earlier code is deleted") {
        // Create three spans with compiled code
        span_object span1, span2, span3;
        REQUIRE(create_test_span(span1, {"A"}));
        REQUIRE(create_test_span(span2, {"AD"}));
        REQUIRE(create_test_span(span3, {"ADK"}));

        REQUIRE(code_compile(span1, true));
        REQUIRE(code_compile(span2, true));
        REQUIRE(code_compile(span3, true));

        // All three should have valid code
        REQUIRE(span1.code != nullptr);
        REQUIRE(span2.code != nullptr);
        REQUIRE(span3.code != nullptr);

        // Record the code indices before deletion
        // span2 and span3 should have higher indices than span1
        code_idx span1_idx = span1.code->code.value();
        code_idx span2_idx = span2.code->code.value();
        code_idx span3_idx = span3.code->code.value();
        code_idx span1_len = span1.code->len;

        REQUIRE(span2_idx > span1_idx);
        REQUIRE(span3_idx > span2_idx);

        // Delete span1's code - this should compact the array
        // and adjust span2 and span3's code indices
        code_discard(span1.code);
        REQUIRE(span1.code == nullptr);

        // span2 and span3's code indices should be reduced by span1's length
        REQUIRE(span2.code->code.value() == span2_idx - span1_len);
        REQUIRE(span3.code->code.value() == span3_idx - span1_len);

        // The code should still work correctly after compaction
        g_execution_log.clear();
        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span2.code, true));
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_delete_char, leadparam::none, 1}
        );

        g_execution_log.clear();
        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span3.code, true));
        CHECK_EXECUTION_LOG(
            {commands::cmd_advance, leadparam::none, 1},
            {commands::cmd_delete_char, leadparam::none, 1},
            {commands::cmd_delete_line, leadparam::none, 1}
        );

        // Clean up
        span1.code = nullptr;
        destroy_test_span(span1);
        destroy_test_span(span2);
        destroy_test_span(span3);
    }

    cleanup_code_globals();
}

TEST_CASE("code_compile leading parameters", "[code][compile][integration]") {
    init_code_globals();

    SECTION("compiles with plus prefix") {
        span_object span;
        REQUIRE(create_test_span(span, {"+A"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG({commands::cmd_advance, leadparam::plus, 1});

        destroy_test_span(span);
    }

    SECTION("compiles with minus prefix") {
        span_object span;
        REQUIRE(create_test_span(span, {"-A"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG({commands::cmd_advance, leadparam::minus, -1});

        destroy_test_span(span);
    }

    SECTION("compiles with dot (pindef) prefix") {
        span_object span;
        REQUIRE(create_test_span(span, {".A"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG({commands::cmd_advance, leadparam::pindef, 0});

        destroy_test_span(span);
    }

    SECTION("compiles with comma (nindef) prefix") {
        span_object span;
        REQUIRE(create_test_span(span, {",A"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        bool result = code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true);
        REQUIRE(result == true);
        CHECK_EXECUTION_LOG({commands::cmd_advance, leadparam::nindef, 0});

        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_compile various commands", "[code][compile][integration]") {
    init_code_globals();

    SECTION("compiles jump command") {
        span_object span;
        REQUIRE(create_test_span(span, {"J"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true));
        CHECK_EXECUTION_LOG({commands::cmd_jump, leadparam::none, 1});

        destroy_test_span(span);
    }

    SECTION("compiles delete_char command") {
        span_object span;
        REQUIRE(create_test_span(span, {"D"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true));
        CHECK_EXECUTION_LOG({commands::cmd_delete_char, leadparam::none, 1});

        destroy_test_span(span);
    }

    SECTION("compiles delete_line command") {
        span_object span;
        REQUIRE(create_test_span(span, {"K"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true));
        CHECK_EXECUTION_LOG({commands::cmd_delete_line, leadparam::none, 1});

        destroy_test_span(span);
    }

    SECTION("compiles insert_char command") {
        span_object span;
        REQUIRE(create_test_span(span, {"C"}));
        REQUIRE(code_compile(span, true));
        g_execution_log.clear();

        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true));
        CHECK_EXECUTION_LOG({commands::cmd_insert_char, leadparam::none, 1});

        destroy_test_span(span);
    }

    cleanup_code_globals();
}

TEST_CASE("code_compile recompilation", "[code][compile][integration]") {
    init_code_globals();

    SECTION("recompiling span discards old code") {
        span_object span;
        REQUIRE(create_test_span(span, {"A"}));
        REQUIRE(code_compile(span, true));

        code_ptr old_code = span.code;
        REQUIRE(old_code != nullptr);

        // Get line from existing marks to create new span content
        line_ptr line = span.mark_one->line;

        // Update line content for recompilation
        const char *new_text = "D";
        int new_len = std::strlen(new_text);
        REQUIRE(line_change_length(line, new_len));
        for (int i = 0; i < new_len; ++i) {
            (*line->str)[i + 1] = new_text[i];
        }
        line->used = new_len;

        // Update mark_two position
        span.mark_two->col = new_len + 1;

        // Recompile
        REQUIRE(code_compile(span, true));

        // Should have new code (old was discarded)
        REQUIRE(span.code != nullptr);

        g_execution_log.clear();
        REQUIRE(code_interpret_execute(mock_execute, leadparam::none, 1, span.code, true));
        CHECK_EXECUTION_LOG({commands::cmd_delete_char, leadparam::none, 1});

        destroy_test_span(span);
    }

    cleanup_code_globals();
}
