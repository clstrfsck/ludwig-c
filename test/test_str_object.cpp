/**
 * @file test_str_object.cpp
 * Unit tests for str_object template class
 */

#include "str_object.h"
#include "prange.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("str_object construction and initialization", "[str_object]") {
    SECTION("default construction initializes with default value") {
        str_object arr;
        REQUIRE(arr[1] == ' ');
    }

    SECTION("construction with custom initial value") {
        str_object arr('.');
        REQUIRE(arr[str_object::MIN_INDEX] == '.');
        REQUIRE(arr[str_object::MAX_INDEX] == '.');
    }

    SECTION("copy construction") {
        str_object arr1;
        arr1[2] = 99;
        str_object arr2(arr1);
        REQUIRE(arr2[2] == 99);
    }
}

TEST_CASE("str_object element access", "[str_object]") {
    str_object arr;

    SECTION("can write and read elements") {
        arr[1] = 10;
        arr[5] = 20;
        arr[10] = 30;

        REQUIRE(arr[1] == 10);
        REQUIRE(arr[5] == 20);
        REQUIRE(arr[10] == 30);
    }

    SECTION("const access") {
        arr[3] = 42;
        const auto &carr = arr;
        REQUIRE(carr[3] == 42);
    }
}

TEST_CASE("str_object size and capacity", "[str_object]") {
    SECTION("MAX_STRLEN has correct value") {
        REQUIRE(str_object::MAX_STRLEN == 400);
    }
}

TEST_CASE("str_object fill operation", "[str_object]") {
    str_object arr(0);

    SECTION("fill with new value") {
        arr.fill(99);
        REQUIRE(arr[1] == 99);
        REQUIRE(arr[10] == 99);
        REQUIRE(arr[20] == 99);
    }
}

TEST_CASE("str_object iterators", "[str_object]") {
    str_object arr(0);
    arr[1] = 10;
    arr[2] = 20;
    arr[3] = 30;
    arr[4] = 40;
    arr[5] = 50;
    arr[6] = 60;

    SECTION("can iterate with range-based for") {
        int sum = 0;
        for (int val : arr) {
            sum += val;
        }
        REQUIRE(sum == 210); // 10+20+30+40+50+60
    }

    SECTION("begin and end iterators") {
        auto it = arr.begin();
        REQUIRE(*it == 10);
        ++it;
        REQUIRE(*it == 20);

        auto end = arr.end();
        REQUIRE(end != it);
    }
}

TEST_CASE("str_object assignment operator", "[str_object]") {
    str_object arr1;
    arr1[2] = 42;
    arr1[4] = 99;

    str_object arr2;
    arr2 = arr1;

    REQUIRE(arr2[2] == 42);
    REQUIRE(arr2[4] == 99);
}
