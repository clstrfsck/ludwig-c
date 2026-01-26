/**
 * @file test_helpfile.cpp
 * Unit tests for helpfile.(cpp|h)
 */

#include <catch2/catch_test_macros.hpp>
#include "fyle.h"

#include <memory>
#include <string>

constexpr const std::string_view LONG_TEST_FILENAME  = "/this/is/a/long/path/to/a/test/file/thats/too/long.txt";
constexpr const std::string_view EQUAL_TEST_FILENAME = "/not/an/extra/long/but/not/short/also/filename.txt";
constexpr const std::string_view SHORT_TEST_FILENAME = "/not/a/super/long/filename.txt";
constexpr size_t MAX_TEST_FILENAME_LEN = EQUAL_TEST_FILENAME.size();

TEST_CASE("file_name limits length", "[fyle]") {
    SECTION("filename longer than max length is truncated") {
        auto fp = std::make_unique<file_object>();
        fp->filename = LONG_TEST_FILENAME;
        REQUIRE(fp->filename.size() > MAX_TEST_FILENAME_LEN);

        std::string result;
        file_name(fp.get(), MAX_TEST_FILENAME_LEN, result);

        REQUIRE(result == "/this/is/a/long/path/to/---file/thats/too/long.txt");
    }

    SECTION("filename shorter than max length is unchanged") {
        auto fp = std::make_unique<file_object>();
        fp->filename = SHORT_TEST_FILENAME;
        REQUIRE(fp->filename.size() < MAX_TEST_FILENAME_LEN);

        std::string result;
        file_name(fp.get(), MAX_TEST_FILENAME_LEN, result);

        REQUIRE(result == SHORT_TEST_FILENAME);
    }

    SECTION("filename exactly max length is unchanged") {
        auto fp = std::make_unique<file_object>();
        fp->filename = EQUAL_TEST_FILENAME;
        REQUIRE(fp->filename.size() == MAX_TEST_FILENAME_LEN);

        std::string result;
        file_name(fp.get(), MAX_TEST_FILENAME_LEN, result);

        REQUIRE(result == EQUAL_TEST_FILENAME);
    }

    SECTION("filename one greater than max length is truncated") {
        auto fp = std::make_unique<file_object>();
        fp->filename = EQUAL_TEST_FILENAME;
        REQUIRE(fp->filename.size() == MAX_TEST_FILENAME_LEN);

        std::string result;
        file_name(fp.get(), MAX_TEST_FILENAME_LEN - 1, result);

        REQUIRE(result == "/not/an/extra/long/but/---short/also/filename.txt");
    }

    SECTION("max_len very short is truncated to minimum length") {
        auto fp = std::make_unique<file_object>();
        fp->filename = SHORT_TEST_FILENAME;

        std::string result;
        file_name(fp.get(), 3, result);

        // Minimum length is 5, so should be truncated to 5 chars
        REQUIRE(result == "/---t");
    }
}
