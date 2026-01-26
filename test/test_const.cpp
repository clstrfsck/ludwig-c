/**
 * @file test_const.cpp
 * Unit tests for constants and type definitions
 */

#include "const.h"
#include "type.h"

#include <catch2/catch_test_macros.hpp>
#include <cstring>

TEST_CASE("Basic constant values", "[const]") {
    SECTION("ORD_MAXCHAR is 255") {
        REQUIRE(ORD_MAXCHAR == 255);
    }

    SECTION("MAX_FILES is positive") {
        REQUIRE(MAX_FILES > 0);
        REQUIRE(MAX_FILES == 100);
    }

    SECTION("MAX_STRLEN is reasonable") {
        REQUIRE(MAX_STRLEN == 400);
        REQUIRE(MAX_STRLENP == MAX_STRLEN + 1);
    }

    SECTION("Mark number ranges") {
        REQUIRE(MIN_MARK_NUMBER == -1);
        REQUIRE(MAX_MARK_NUMBER == 9);
        REQUIRE(MARK_EQUALS == 0);
        REQUIRE(MARK_MODIFIED == -1);
    }
}

TEST_CASE("Screen dimension constants", "[const]") {
    SECTION("screen rows") {
        REQUIRE(MAX_SCR_ROWS == 100);
        REQUIRE(MAX_SCR_ROWS > 0);
    }

    SECTION("screen columns") {
        REQUIRE(MAX_SCR_COLS == 255);
        REQUIRE(MAX_SCR_COLS > 0);
    }
}

TEST_CASE("Code and execution constants", "[const]") {
    SECTION("code array size") {
        REQUIRE(MAX_CODE == 4000);
    }

    SECTION("verify commands") {
        REQUIRE(MAX_VERIFY == 256);
    }

    SECTION("trailing parameter limits") {
        REQUIRE(MAX_TPCOUNT == 2);
        REQUIRE(MAX_TPAR_RECURSION == 100);
        REQUIRE(MAX_EXEC_RECURSION == 100);
    }
}

TEST_CASE("String length constants", "[const]") {
    SECTION("various string lengths") {
        REQUIRE(NAME_LEN == 31);
        REQUIRE(FILE_NAME_LEN > 0);
        REQUIRE(KEY_LEN == 4);
    }
}

TEST_CASE("Pattern matcher constants", "[const]") {
    SECTION("NFA and DFA state ranges") {
        REQUIRE(MAX_NFA_STATE_RANGE == 200);
        REQUIRE(MAX_DFA_STATE_RANGE == 255);
        REQUIRE(MAX_SET_RANGE == ORD_MAXCHAR);
    }

    SECTION("pattern state values") {
        REQUIRE(PATTERN_NULL == 0);
        REQUIRE(PATTERN_NFA_START == 1);
        REQUIRE(PATTERN_DFA_KILL == 0);
        REQUIRE(PATTERN_DFA_START == 2);
        REQUIRE(PATTERN_MAX_DEPTH == 20);
    }

    SECTION("pattern character constants") {
        REQUIRE(PATTERN_KSTAR == '*');
        REQUIRE(PATTERN_COMMA == ',');
        REQUIRE(PATTERN_RPAREN == ')');
        REQUIRE(PATTERN_LPAREN == '(');
        REQUIRE(PATTERN_PLUS == '+');
        REQUIRE(PATTERN_BAR == '|');
        REQUIRE(PATTERN_SPACE == ' ');
    }

    SECTION("pattern position constants") {
        REQUIRE(PATTERN_BEG_LINE == 0);
        REQUIRE(PATTERN_END_LINE == 1);
        REQUIRE(PATTERN_LEFT_MARGIN == 3);
        REQUIRE(PATTERN_RIGHT_MARGIN == 4);
        REQUIRE(PATTERN_DOT_COLUMN == 5);
    }
}

TEST_CASE("Message strings exist", "[const]") {
    SECTION("blank message") {
        REQUIRE(MSG_BLANK.empty());
    }
}

TEST_CASE("Frame name constants", "[const]") {
    SECTION("blank frame name is empty") {
        REQUIRE(BLANK_FRAME_NAME.empty());
    }

    SECTION("default frame name is LUDWIG") {
        REQUIRE(DEFAULT_FRAME_NAME == "LUDWIG");
    }
}

TEST_CASE("TPar delimiter characters", "[const]") {
    SECTION("trailing parameter delimiters") {
        REQUIRE(TPD_LIT == '\'');
        REQUIRE(TPD_SMART == '`');
        REQUIRE(TPD_EXACT == '"');
        REQUIRE(TPD_SPAN == '$');
        REQUIRE(TPD_PROMPT == '&');
        REQUIRE(TPD_ENVIRONMENT == '?');
        REQUIRE(EXPAND_LIM == 130);
    }
}

TEST_CASE("Range types", "[const]") {
    SECTION("col_range has correct bounds") {
        using col_t = col_range;
        REQUIRE(col_t::min() == 1);
        REQUIRE(col_t::max() == MAX_STRLENP);
    }

    SECTION("line_range has correct bounds") {
        using line_t = line_range;
        REQUIRE(line_t::min() == 0);
        REQUIRE(line_t::max() == MAX_LINES);
    }

    SECTION("mark_range has correct bounds") {
        using mark_t = mark_range;
        REQUIRE(mark_t::min() == MIN_MARK_NUMBER);
        REQUIRE(mark_t::max() == MAX_MARK_NUMBER);
    }
}

TEST_CASE("Keyboard constants", "[const]") {
    SECTION("keyboard interface limits") {
        REQUIRE(MAX_SPECIAL_KEYS == 1000);
        REQUIRE(MAX_NR_KEY_NAMES == 1000);
        REQUIRE(MAX_PARSE_TABLE == 300);
    }
}

TEST_CASE("Exit codes", "[const]") {
    SECTION("exit code values") {
        REQUIRE(NORMAL_EXIT == 0);
        REQUIRE(ABNORMAL_EXIT == 1);
    }
}
