/**
 * @file test_helpfile.cpp
 * Unit tests for helpfile.(cpp|h)
 */

#include "helpfile.h"

#include <catch2/catch_test_macros.hpp>
#include <string>

#ifndef TEST_DATA_PATH
#define TEST_DATA_PATH "./"
#endif

namespace {
    // RAII resource locator and cleanup.
    struct HelpfileFixture {
        HelpfileFixture() {
            auto helpfile = std::string(TEST_DATA_PATH) + "ludwighlp.idx";
            (void)helpfile_open(helpfile);
        }
        ~HelpfileFixture() {
            helpfile_close();
        }
    };
} // namespace

SCENARIO("help file can be opened and index read") {
    GIVEN("An open help file and an index entry read") {
        HelpfileFixture fixture;
        help_record record;
        bool found = helpfile_read("0", record);
        REQUIRE(found == true);

        WHEN("reading the remaining lines in the entry") {
            size_t line_count = 1;
            size_t char_count = record.txt.size() + 1; // +1 for newline
            while (found) {
                if ((found = helpfile_next(record))) {
                    line_count += 1;
                    char_count += record.txt.size() + 1;
                }
            }

            THEN("Correct number of lines and characters are read") {
                REQUIRE(line_count == 21);
                REQUIRE(char_count == 720);
            }
        }
    }
}

SCENARIO("help file can be opened and last entry read") {
    GIVEN("An open help file and the last index entry read") {
        HelpfileFixture fixture;
        help_record record;
        bool found = helpfile_read("?", record);
        REQUIRE(found == true);

        WHEN("reading the remaining lines in the entry") {
            size_t line_count = 1;
            size_t char_count = record.txt.size() + 1; // +1 for newline
            while (found) {
                if ((found = helpfile_next(record))) {
                    line_count += 1;
                    char_count += record.txt.size() + 1;
                }
            }

            THEN("Correct number of lines and characters are read") {
                REQUIRE(line_count == 22);
                REQUIRE(char_count == 538);
            }
        }
    }
}
