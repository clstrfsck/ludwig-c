/**
 * @file test_prange.cpp
 * Unit tests for prange template class
 */

#include "prange.h"

#include <catch2/catch_test_macros.hpp>
#include <ranges>
#include <vector>
#include <algorithm>

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

TEST_CASE("prange boundary operations", "[prange]") {
    using test_range = prange<5, 15>;

    SECTION("increment at boundaries") {
        test_range r(14);
        ++r;
        REQUIRE(r.value() == 15);
    }

    SECTION("decrement at boundaries") {
        test_range r(6);
        --r;
        REQUIRE(r.value() == 5);
    }

    SECTION("compound operations at boundaries") {
        test_range r1(5);
        r1 += 10;
        REQUIRE(r1.value() == 15);

        test_range r2(15);
        r2 -= 10;
        REQUIRE(r2.value() == 5);
    }
}

TEST_CASE("prange chained operations", "[prange]") {
    SECTION("multiple increments") {
        prange<0, 100> r(10);
        ++r;
        ++r;
        ++r;
        REQUIRE(r.value() == 13);
    }

    SECTION("mixed increment and decrement") {
        prange<0, 100> r(50);
        r += 10;
        r -= 5;
        ++r;
        --r;
        REQUIRE(r.value() == 55);
    }

    SECTION("chained assignments") {
        prange<0, 100> r1, r2, r3;
        r1 = r2 = r3 = 42;
        REQUIRE(r1.value() == 42);
        REQUIRE(r2.value() == 42);
        REQUIRE(r3.value() == 42);
    }
}

TEST_CASE("prange implicit conversion in expressions", "[prange]") {
    prange<0, 100> r(25);

    SECTION("can be used in arithmetic expressions") {
        int result = r + 10;
        REQUIRE(result == 35);
    }

    SECTION("can be compared with integers") {
        REQUIRE(r == 25);
        REQUIRE(r != 30);
        REQUIRE(r < 50);
        REQUIRE(r > 10);
        REQUIRE(r <= 25);
        REQUIRE(r >= 25);
    }

    SECTION("can be used in conditionals") {
        if (r) {
            REQUIRE(true);
        } else {
            REQUIRE(false);
        }
    }
}

TEST_CASE("prange with single element range", "[prange]") {
    using single_range = prange<42, 43>;

    SECTION("has size of 2") {
        REQUIRE(single_range::size() == 2);
    }

    SECTION("min and max differ by 1") {
        REQUIRE(single_range::max() - single_range::min() == 1);
    }

    SECTION("can construct with both values") {
        single_range r1(42);
        single_range r2(43);
        REQUIRE(r1.value() == 42);
        REQUIRE(r2.value() == 43);
    }
}

TEST_CASE("prange constexpr functionality", "[prange]") {
    using test_range = prange<10, 20>;

    SECTION("static methods are constexpr") {
        constexpr int min_val = test_range::min();
        constexpr int max_val = test_range::max();
        constexpr size_t range_size = test_range::size();
        constexpr size_t zero_idx = test_range::zero_based(15);

        REQUIRE(min_val == 10);
        REQUIRE(max_val == 20);
        REQUIRE(range_size == 11);
        REQUIRE(zero_idx == 5);
    }
}

TEST_CASE("prange different template specializations", "[prange]") {
    SECTION("ranges with different bounds don't interfere") {
        prange<0, 10> r1(5);
        prange<100, 200> r2(150);
        prange<-50, 50> r3(0);

        REQUIRE(r1.value() == 5);
        REQUIRE(r2.value() == 150);
        REQUIRE(r3.value() == 0);
    }

    SECTION("each specialization has correct static values") {
        REQUIRE(prange<0, 10>::min() == 0);
        REQUIRE(prange<100, 200>::min() == 100);
        REQUIRE(prange<-50, 50>::min() == -50);

        REQUIRE(prange<0, 10>::size() == 11);
        REQUIRE(prange<100, 200>::size() == 101);
        REQUIRE(prange<-50, 50>::size() == 101);
    }
}

TEST_CASE("prange operations return correct types", "[prange]") {
    prange<0, 100> r(50);

    SECTION("pre-increment returns reference") {
        auto& result = ++r;
        result = 60;
        REQUIRE(r.value() == 60);
    }

    SECTION("post-increment returns value") {
        auto old = r++;
        old = 99; // This shouldn't affect r
        REQUIRE(r.value() == 51);
    }

    SECTION("compound assignment returns reference") {
        auto& result = (r += 10);
        result -= 5;
        REQUIRE(r.value() == 55);
    }
}

TEST_CASE("prange edge case values", "[prange]") {
    SECTION("range starting at 0") {
        prange<0, 255> r;
        REQUIRE(r.min() == 0);
        REQUIRE(r.value() == 0);
    }

    SECTION("range starting at 1") {
        prange<1, 100> r;
        REQUIRE(r.min() == 1);
        REQUIRE(r.value() == 1);
    }

    SECTION("range with max at boundary") {
        prange<0, 255> r(255);
        REQUIRE(r.value() == 255);
    }
}

TEST_CASE("prange zero_based indexing", "[prange]") {
    SECTION("works correctly for various ranges") {
        using r1 = prange<0, 10>;
        REQUIRE(r1::zero_based(0) == 0);
        REQUIRE(r1::zero_based(5) == 5);
        REQUIRE(r1::zero_based(10) == 10);

        using r2 = prange<1, 10>;
        REQUIRE(r2::zero_based(1) == 0);
        REQUIRE(r2::zero_based(5) == 4);
        REQUIRE(r2::zero_based(10) == 9);

        using r3 = prange<100, 200>;
        REQUIRE(r3::zero_based(100) == 0);
        REQUIRE(r3::zero_based(150) == 50);
        REQUIRE(r3::zero_based(200) == 100);
    }
}

TEST_CASE("prange iota view", "[prange]") {
    SECTION("generates correct range of values") {
        using r = prange<0, 5>;
        auto view = r::iota();
        std::vector<int> values(view.begin(), view.end());

        REQUIRE(values.size() == 6);
        REQUIRE(values[0] == 0);
        REQUIRE(values[1] == 1);
        REQUIRE(values[2] == 2);
        REQUIRE(values[3] == 3);
        REQUIRE(values[4] == 4);
        REQUIRE(values[5] == 5);
    }

    SECTION("works with non-zero starting range") {
        using r = prange<10, 15>;
        auto view = r::iota();
        std::vector<int> values(view.begin(), view.end());

        REQUIRE(values.size() == 6);
        REQUIRE(values[0] == 10);
        REQUIRE(values[5] == 15);
    }

    SECTION("works with negative ranges") {
        using r = prange<-5, 5>;
        auto view = r::iota();
        std::vector<int> values(view.begin(), view.end());

        REQUIRE(values.size() == 11);
        REQUIRE(values[0] == -5);
        REQUIRE(values[5] == 0);
        REQUIRE(values[10] == 5);
    }

    SECTION("can be used with range algorithms") {
        using r = prange<1, 10>;
        auto view = r::iota();

        // Count elements
        auto count = std::ranges::distance(view);
        REQUIRE(count == 10);

        // Check all values are in range
        bool all_in_range = std::ranges::all_of(view, [](int n) {
            return n >= 1 && n <= 10;
        });
        REQUIRE(all_in_range);
    }

    SECTION("can be piped with other range operations") {
        using r = prange<0, 10>;
        auto view = r::iota() | std::views::filter([](int n) { return n % 2 == 0; });
        std::vector<int> evens(view.begin(), view.end());

        REQUIRE(evens.size() == 6);
        REQUIRE(evens[0] == 0);
        REQUIRE(evens[1] == 2);
        REQUIRE(evens[2] == 4);
        REQUIRE(evens[3] == 6);
        REQUIRE(evens[4] == 8);
        REQUIRE(evens[5] == 10);
    }

    SECTION("works with minimal range") {
        using r = prange<42, 43>;
        auto view = r::iota();
        std::vector<int> values(view.begin(), view.end());

        REQUIRE(values.size() == 2);
        REQUIRE(values[0] == 42);
        REQUIRE(values[1] == 43);
    }

    SECTION("iota matches prange size") {
        using test_range = prange<5, 25>;
        auto view = test_range::iota();

        REQUIRE(std::ranges::distance(view) == test_range::size());
    }
}
