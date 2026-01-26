/**
 * @file test_prange.cpp
 * Unit tests for prange template class
 */

#include "prange.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("prange basic construction", "[prange]") {
    SECTION("default construction initializes to min") {
        prange<0, 10> r;
        REQUIRE(r.value() == 0);
    }

    SECTION("construction with value") {
        prange<1, 100> r(50);
        REQUIRE(r.value() == 50);
    }

    SECTION("construction with min value") {
        prange<5, 15> r(5);
        REQUIRE(r.value() == 5);
    }

    SECTION("construction with max value") {
        prange<5, 15> r(15);
        REQUIRE(r.value() == 15);
    }

    SECTION("copy construction") {
        prange<0, 10> r1(7);
        prange<0, 10> r2(r1);
        REQUIRE(r2.value() == 7);
    }
}

TEST_CASE("prange static methods", "[prange]") {
    using test_range = prange<10, 20>;

    SECTION("min() returns minimum value") {
        REQUIRE(test_range::min() == 10);
    }

    SECTION("max() returns maximum value") {
        REQUIRE(test_range::max() == 20);
    }

    SECTION("size() returns range size") {
        REQUIRE(test_range::size() == 11); // 10..20 inclusive = 11 values
    }

    SECTION("zero_based() converts to zero-based index") {
        REQUIRE(test_range::zero_based(10) == 0);
        REQUIRE(test_range::zero_based(15) == 5);
        REQUIRE(test_range::zero_based(20) == 10);
    }
}

TEST_CASE("prange assignment operators", "[prange]") {
    SECTION("assignment from another prange") {
        prange<0, 100> r1(50);
        prange<0, 100> r2(10);
        r2 = r1;
        REQUIRE(r2.value() == 50);
    }

    SECTION("assignment from int") {
        prange<0, 100> r(10);
        r = 75;
        REQUIRE(r.value() == 75);
    }
}

TEST_CASE("prange increment operators", "[prange]") {
    SECTION("pre-increment") {
        prange<0, 10> r(5);
        ++r;
        REQUIRE(r.value() == 6);
    }

    SECTION("post-increment") {
        prange<0, 10> r(5);
        prange<0, 10> old = r++;
        REQUIRE(old.value() == 5);
        REQUIRE(r.value() == 6);
    }

    SECTION("compound addition") {
        prange<0, 100> r(10);
        r += 25;
        REQUIRE(r.value() == 35);
    }
}

TEST_CASE("prange decrement operators", "[prange]") {
    SECTION("pre-decrement") {
        prange<0, 10> r(5);
        --r;
        REQUIRE(r.value() == 4);
    }

    SECTION("post-decrement") {
        prange<0, 10> r(5);
        prange<0, 10> old = r--;
        REQUIRE(old.value() == 5);
        REQUIRE(r.value() == 4);
    }

    SECTION("compound subtraction") {
        prange<0, 100> r(50);
        r -= 25;
        REQUIRE(r.value() == 25);
    }
}

TEST_CASE("prange conversion to int", "[prange]") {
    prange<0, 100> r(42);
    int value = r;
    REQUIRE(value == 42);
}

TEST_CASE("prange with negative ranges", "[prange]") {
    using neg_range = prange<-10, 10>;

    SECTION("construction with negative value") {
        neg_range r(-5);
        REQUIRE(r.value() == -5);
    }

    SECTION("range spans negative and positive") {
        REQUIRE(neg_range::min() == -10);
        REQUIRE(neg_range::max() == 10);
        REQUIRE(neg_range::size() == 21);
    }

    SECTION("zero_based with negative range") {
        REQUIRE(neg_range::zero_based(-10) == 0);
        REQUIRE(neg_range::zero_based(0) == 10);
        REQUIRE(neg_range::zero_based(10) == 20);
    }
}

TEST_CASE("prange with large ranges", "[prange]") {
    using large_range = prange<0, 1000000>;

    SECTION("can handle large values") {
        large_range r(999999);
        REQUIRE(r.value() == 999999);
    }

    SECTION("size calculation for large range") {
        REQUIRE(large_range::size() == 1000001);
    }
}
