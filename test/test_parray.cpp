/**
 * @file test_parray.cpp
 * Unit tests for parray template class
 */

#include "parray.h"
#include "prange.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("parray construction and initialization", "[parray]") {
    SECTION("default construction initializes with default value") {
        using range_t = prange<0, 10>;
        parray<int, range_t> arr;
        REQUIRE(range_t::size() == 11);
        REQUIRE(arr[0] == 0); // default int is 0
    }

    SECTION("construction with custom initial value") {
        using array_t = parray<int, prange<0, 5>>;
        array_t arr(42);
        REQUIRE(array_t::index_type::size() == 6);
        REQUIRE(arr[1] == 42);
        REQUIRE(arr[3] == 42);
        REQUIRE(arr[5] == 42);
    }

    SECTION("copy construction") {
        parray<int, prange<0, 5>> arr1;
        arr1[2] = 99;
        parray<int, prange<0, 5>> arr2(arr1);
        REQUIRE(arr2[2] == 99);
    }
}

TEST_CASE("parray element access", "[parray]") {
    parray<int, prange<1, 10>> arr;

    SECTION("can write and read elements") {
        arr[1] = 100;
        arr[5] = 200;
        arr[10] = 300;

        REQUIRE(arr[1] == 100);
        REQUIRE(arr[5] == 200);
        REQUIRE(arr[10] == 300);
    }

    SECTION("const access") {
        arr[3] = 42;
        const auto &carr = arr;
        REQUIRE(carr[3] == 42);
    }
}

TEST_CASE("parray with different types", "[parray]") {
    SECTION("array of doubles") {
        parray<double, prange<0, 5>> arr(3.14);
        REQUIRE(arr[0] == 3.14);
        REQUIRE(arr[5] == 3.14);
    }

    SECTION("array of chars") {
        parray<char, prange<1, 10>> arr('X');
        REQUIRE(arr[1] == 'X');
        REQUIRE(arr[10] == 'X');
    }

    SECTION("array of booleans") {
        parray<bool, prange<0, 3>> arr(true);
        REQUIRE(arr[0] == true);
        REQUIRE(arr[3] == true);
        arr[1] = false;
        REQUIRE(arr[1] == false);
    }
}

TEST_CASE("parray size and capacity", "[parray]") {
    SECTION("size returns correct value") {
        using range_t = prange<0, 10>;
        parray<int, range_t> arr;
        REQUIRE(range_t::size() == 11);
    }

    SECTION("size for 1-based range") {
        using range_t = prange<1, 100>;
        parray<int, range_t> arr;
        REQUIRE(range_t::size() == 100);
    }

    SECTION("size for negative range") {
        using range_t = prange<-5, 5>;
        parray<int, range_t> arr;
        REQUIRE(range_t::size() == 11);
    }
}

TEST_CASE("parray fill operation", "[parray]") {
    parray<int, prange<0, 5>> arr(0);

    SECTION("fill with new value") {
        arr.fill(99);
        REQUIRE(arr[0] == 99);
        REQUIRE(arr[2] == 99);
        REQUIRE(arr[5] == 99);
    }
}

TEST_CASE("parray with custom ranges", "[parray]") {
    SECTION("array with range starting at 10") {
        parray<int, prange<10, 20>> arr;
        arr[10] = 1;
        arr[15] = 2;
        arr[20] = 3;

        REQUIRE(arr[10] == 1);
        REQUIRE(arr[15] == 2);
        REQUIRE(arr[20] == 3);
    }

    SECTION("array with large range") {
        using range_t = prange<100, 200>;
        parray<int, range_t> arr(42);
        REQUIRE(arr[100] == 42);
        REQUIRE(arr[150] == 42);
        REQUIRE(arr[200] == 42);
        REQUIRE(range_t::size() == 101);
    }
}

TEST_CASE("parray iterators", "[parray]") {
    parray<int, prange<0, 5>> arr;
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    arr[3] = 40;
    arr[4] = 50;
    arr[5] = 60;

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

TEST_CASE("parray assignment operator", "[parray]") {
    parray<int, prange<0, 5>> arr1;
    arr1[2] = 42;
    arr1[4] = 99;

    parray<int, prange<0, 5>> arr2;
    arr2 = arr1;

    REQUIRE(arr2[2] == 42);
    REQUIRE(arr2[4] == 99);
}

TEST_CASE("parray with structs", "[parray]") {
    struct Point {
        int x = 0;
        int y = 0;
        Point() = default;
        Point(int x_, int y_) : x(x_), y(y_) {
        }
    };

    SECTION("array of structs") {
        parray<Point, prange<1, 3>> points;
        points[1] = Point(10, 20);
        points[2] = Point(30, 40);

        REQUIRE(points[1].x == 10);
        REQUIRE(points[1].y == 20);
        REQUIRE(points[2].x == 30);
        REQUIRE(points[2].y == 40);
    }
}
