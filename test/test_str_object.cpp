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

    SECTION("initializer_list construction with values") {
        str_object arr({'A', 'B', 'C'});
        REQUIRE(arr[1] == 'A');
        REQUIRE(arr[2] == 'B');
        REQUIRE(arr[3] == 'C');
        REQUIRE(arr[4] == 'A'); // Repeating pattern
        REQUIRE(arr[5] == 'B');
        REQUIRE(arr[6] == 'C');
    }

    SECTION("initializer_list construction with empty list") {
        str_object arr({});
        REQUIRE(arr[1] == ' '); // Falls back to space
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
TEST_CASE("str_object comparison operators", "[str_object]") {
    SECTION("equality operator") {
        str_object arr1('A');
        str_object arr2('A');
        str_object arr3('B');

        REQUIRE(arr1 == arr2);
        REQUIRE_FALSE(arr1 == arr3);
    }

    SECTION("inequality operator") {
        str_object arr1('A');
        str_object arr2('B');

        REQUIRE(arr1 != arr2);
        REQUIRE_FALSE(arr1 != arr1);
    }

    SECTION("spaceship operator - less than") {
        str_object arr1('A');
        str_object arr2('B');

        REQUIRE(arr1 < arr2);
        REQUIRE_FALSE(arr2 < arr1);
        REQUIRE_FALSE(arr1 < arr1);
    }

    SECTION("spaceship operator - greater than") {
        str_object arr1('B');
        str_object arr2('A');

        REQUIRE(arr1 > arr2);
        REQUIRE_FALSE(arr2 > arr1);
    }

    SECTION("spaceship operator - less than or equal") {
        str_object arr1('A');
        str_object arr2('B');
        str_object arr3('A');

        REQUIRE(arr1 <= arr2);
        REQUIRE(arr1 <= arr3);
        REQUIRE_FALSE(arr2 <= arr1);
    }

    SECTION("spaceship operator - greater than or equal") {
        str_object arr1('B');
        str_object arr2('A');
        str_object arr3('B');

        REQUIRE(arr1 >= arr2);
        REQUIRE(arr1 >= arr3);
        REQUIRE_FALSE(arr2 >= arr1);
    }
}

TEST_CASE("str_object const iterators", "[str_object]") {
    str_object arr('X');
    arr[1] = 'A';
    arr[2] = 'B';
    arr[3] = 'C';

    SECTION("cbegin and cend") {
        auto it = arr.cbegin();
        REQUIRE(*it == 'A');
        ++it;
        REQUIRE(*it == 'B');
        ++it;
        REQUIRE(*it == 'C');
    }

    SECTION("const iteration") {
        const auto &carr = arr;
        int count = 0;
        for (auto it = carr.cbegin(); it != carr.cend() && count < 3; ++it, ++count) {
            REQUIRE(*it != 0);
        }
        REQUIRE(count == 3);
    }
}

TEST_CASE("str_object apply_n", "[str_object]") {
    str_object arr(' ');
    arr.fillcopy("hello", 1, 5, ' ');

    SECTION("apply function to n elements") {
        arr.apply_n([](char c) { return std::toupper(c); }, 5, 1);
        REQUIRE(arr[1] == 'H');
        REQUIRE(arr[2] == 'E');
        REQUIRE(arr[3] == 'L');
        REQUIRE(arr[4] == 'L');
        REQUIRE(arr[5] == 'O');
    }

    SECTION("apply with offset") {
        str_object arr2(' ');
        arr2.fillcopy("world", 5, 5, ' ');
        arr2.apply_n([](char c) { return std::toupper(c); }, 3, 5);
        REQUIRE(arr2[5] == 'W');
        REQUIRE(arr2[6] == 'O');
        REQUIRE(arr2[7] == 'R');
        REQUIRE(arr2[8] == 'l'); // Not transformed
    }

    SECTION("apply zero elements does nothing") {
        auto orig = arr[1];
        arr.apply_n([](char /*c*/) { return 'X'; }, 0, 1);
        REQUIRE(arr[1] == orig);
    }
}

TEST_CASE("str_object copy", "[str_object]") {
    str_object src(' ');
    src.fillcopy("ABCDEF", 1, 6, ' ');

    SECTION("copy from another str_object") {
        str_object dst(' ');
        dst.copy(src, 1, 3, 1);
        REQUIRE(dst[1] == 'A');
        REQUIRE(dst[2] == 'B');
        REQUIRE(dst[3] == 'C');
        REQUIRE(dst[4] == ' ');
    }

    SECTION("copy with source offset") {
        str_object dst(' ');
        dst.copy(src, 4, 3, 1);
        REQUIRE(dst[1] == 'D');
        REQUIRE(dst[2] == 'E');
        REQUIRE(dst[3] == 'F');
    }

    SECTION("copy with destination offset") {
        str_object dst(' ');
        dst.copy(src, 1, 3, 5);
        REQUIRE(dst[1] == ' ');
        REQUIRE(dst[5] == 'A');
        REQUIRE(dst[6] == 'B');
        REQUIRE(dst[7] == 'C');
    }

    SECTION("copy zero count does nothing") {
        str_object dst('X');
        dst.copy(src, 1, 0, 1);
        REQUIRE(dst[1] == 'X');
    }
}

TEST_CASE("str_object copy_n", "[str_object]") {
    const char *src = "Hello";

    SECTION("copy from C string") {
        str_object dst(' ');
        dst.copy_n(src, 5, 1);
        REQUIRE(dst[1] == 'H');
        REQUIRE(dst[2] == 'e');
        REQUIRE(dst[3] == 'l');
        REQUIRE(dst[4] == 'l');
        REQUIRE(dst[5] == 'o');
    }

    SECTION("copy with destination offset") {
        str_object dst(' ');
        dst.copy_n(src, 5, 10);
        REQUIRE(dst[10] == 'H');
        REQUIRE(dst[11] == 'e');
        REQUIRE(dst[14] == 'o');
    }

    SECTION("copy zero count does nothing") {
        str_object dst('X');
        dst.copy_n(src, 0, 1);
        REQUIRE(dst[1] == 'X');
    }
}

TEST_CASE("str_object equals", "[str_object]") {
    str_object arr1(' ');
    str_object arr2(' ');
    arr1.fillcopy("ABCDEF", 1, 6, ' ');
    arr2.fillcopy("XYZABC", 1, 6, ' ');

    SECTION("equals with matching content") {
        REQUIRE(arr1.equals(arr2, 3, 1, 4));
    }

    SECTION("equals with non-matching content") {
        REQUIRE_FALSE(arr1.equals(arr2, 3, 1, 1));
    }

    SECTION("equals with zero length") {
        REQUIRE(arr1.equals(arr2, 0, 1, 1));
    }

    SECTION("equals full match") {
        str_object arr3(' ');
        arr3.fillcopy("ABCDEF", 1, 6, ' ');
        REQUIRE(arr1.equals(arr3, 6, 1, 1));
    }
}

TEST_CASE("str_object erase", "[str_object]") {
    SECTION("erase elements from middle") {
        str_object arr(' ');
        arr.fillcopy("ABCDEFGH", 1, 8, ' ');
        arr.erase(3, 3); // Remove DEF
        REQUIRE(arr[1] == 'A');
        REQUIRE(arr[2] == 'B');
        REQUIRE(arr[3] == 'F');
        REQUIRE(arr[4] == 'G');
        REQUIRE(arr[5] == 'H');
        REQUIRE(arr[6] == ' ');
    }

    SECTION("erase from beginning") {
        str_object arr(' ');
        arr.fillcopy("ABCD", 1, 4, ' ');
        arr.erase(2, 1);
        REQUIRE(arr[1] == 'C');
        REQUIRE(arr[2] == 'D');
        REQUIRE(arr[3] == ' ');
    }

    SECTION("erase zero elements does nothing") {
        str_object arr(' ');
        arr.fillcopy("ABCD", 1, 4, ' ');
        arr.erase(0, 1);
        REQUIRE(arr[1] == 'A');
        REQUIRE(arr[2] == 'B');
    }
}

TEST_CASE("str_object fill_n", "[str_object]") {
    SECTION("fill n elements") {
        str_object arr(' ');
        arr.fill_n('X', 5, 1);
        REQUIRE(arr[1] == 'X');
        REQUIRE(arr[2] == 'X');
        REQUIRE(arr[5] == 'X');
        REQUIRE(arr[6] == ' ');
    }

    SECTION("fill with offset") {
        str_object arr(' ');
        arr.fill_n('Y', 3, 10);
        REQUIRE(arr[9] == ' ');
        REQUIRE(arr[10] == 'Y');
        REQUIRE(arr[11] == 'Y');
        REQUIRE(arr[12] == 'Y');
        REQUIRE(arr[13] == ' ');
    }

    SECTION("fill zero elements does nothing") {
        str_object arr(' ');
        arr.fill_n('X', 0, 1);
        REQUIRE(arr[1] == ' ');
    }
}

TEST_CASE("str_object fillcopy str_object overload", "[str_object]") {
    str_object src(' ');
    src.fillcopy("Hello", 1, 5, ' ');

    SECTION("fillcopy with exact length") {
        str_object dst(' ');
        dst.fillcopy(src, 1, 5, 1, 5, '.');
        REQUIRE(dst[1] == 'H');
        REQUIRE(dst[2] == 'e');
        REQUIRE(dst[5] == 'o');
        REQUIRE(dst[6] == ' ');
    }

    SECTION("fillcopy with padding") {
        str_object dst(' ');
        dst.fillcopy(src, 1, 5, 1, 10, '.');
        REQUIRE(dst[1] == 'H');
        REQUIRE(dst[5] == 'o');
        REQUIRE(dst[6] == '.');
        REQUIRE(dst[10] == '.');
        REQUIRE(dst[11] == ' ');
    }

    SECTION("fillcopy with truncation") {
        str_object dst(' ');
        dst.fillcopy(src, 1, 5, 1, 3, '.');
        REQUIRE(dst[1] == 'H');
        REQUIRE(dst[2] == 'e');
        REQUIRE(dst[3] == 'l');
        REQUIRE(dst[4] == ' ');
    }

    SECTION("fillcopy with zero dst_len") {
        str_object dst(' ');
        dst.fillcopy(src, 1, 5, 1, 0, '.');
        REQUIRE(dst[1] == ' ');
    }
}

TEST_CASE("str_object fillcopy string_view overload", "[str_object]") {
    SECTION("fillcopy with exact length") {
        str_object dst(' ');
        dst.fillcopy("Hello", 1, 5, '.');
        REQUIRE(dst[1] == 'H');
        REQUIRE(dst[2] == 'e');
        REQUIRE(dst[5] == 'o');
        REQUIRE(dst[6] == ' ');
    }

    SECTION("fillcopy with padding") {
        str_object dst(' ');
        dst.fillcopy("Hi", 1, 10, '.');
        REQUIRE(dst[1] == 'H');
        REQUIRE(dst[2] == 'i');
        REQUIRE(dst[3] == '.');
        REQUIRE(dst[10] == '.');
        REQUIRE(dst[11] == ' ');
    }

    SECTION("fillcopy with truncation") {
        str_object dst(' ');
        dst.fillcopy("Hello", 1, 3, '.');
        REQUIRE(dst[1] == 'H');
        REQUIRE(dst[2] == 'e');
        REQUIRE(dst[3] == 'l');
        REQUIRE(dst[4] == ' ');
    }
}

TEST_CASE("str_object insert", "[str_object]") {
    SECTION("insert space in middle") {
        str_object arr(' ');
        arr.fillcopy("ABCDEF", 1, 6, ' ');
        arr.insert(3, 3);
        REQUIRE(arr[1] == 'A');
        REQUIRE(arr[2] == 'B');
        REQUIRE(arr[3] == 'C');
        REQUIRE(arr[4] == 'D');
        REQUIRE(arr[5] == 'E');
        REQUIRE(arr[6] == 'C');
        REQUIRE(arr[7] == 'D');
        REQUIRE(arr[8] == 'E');
    }

    SECTION("insert at beginning") {
        str_object arr(' ');
        arr.fillcopy("ABC", 1, 3, ' ');
        arr.insert(2, 1);
        REQUIRE(arr[1] == 'A');
        REQUIRE(arr[2] == 'B');
        REQUIRE(arr[3] == 'A');
        REQUIRE(arr[4] == 'B');
        REQUIRE(arr[5] == 'C');
    }

    SECTION("insert zero elements does nothing") {
        str_object arr(' ');
        arr.fillcopy("ABC", 1, 3, ' ');
        arr.insert(0, 1);
        REQUIRE(arr[1] == 'A');
        REQUIRE(arr[2] == 'B');
        REQUIRE(arr[3] == 'C');
    }
}

TEST_CASE("str_object length", "[str_object]") {
    SECTION("length with trailing spaces") {
        str_object arr(' ');
        arr.fillcopy("Hello", 1, 5, ' ');
        REQUIRE(arr.length(' ') == 5);
    }

    SECTION("length with no trailing value") {
        str_object arr('X');
        REQUIRE(arr.length(' ') == str_object::MAX_STRLEN);
    }

    SECTION("length with all matching value") {
        str_object arr(' ');
        REQUIRE(arr.length(' ') == 0);
    }

    SECTION("length with from parameter") {
        str_object arr(' ');
        arr.fillcopy("Hello", 1, 5, ' ');
        REQUIRE(arr.length(' ', 10) == 5);
        REQUIRE(arr.length(' ', 3) == 3);
    }

    SECTION("length with custom value") {
        str_object arr('.');
        arr.fillcopy("Hello", 1, 5, '.');
        REQUIRE(arr.length('.') == 5);
    }
}

TEST_CASE("str_object slice", "[str_object]") {
    str_object arr(' ');
    arr.fillcopy("Hello World", 1, 11, ' ');

    SECTION("slice extracts substring") {
        auto view = arr.slice(1, 5);
        REQUIRE(view == "Hello");
    }

    SECTION("slice with offset") {
        auto view = arr.slice(7, 5);
        REQUIRE(view == "World");
    }

    SECTION("slice single character") {
        auto view = arr.slice(1, 1);
        REQUIRE(view == "H");
    }
}

TEST_CASE("str_object bounds checking", "[str_object]") {
    str_object arr;

    SECTION("access below MIN_INDEX throws") {
        REQUIRE_THROWS_AS(arr[0], std::out_of_range);
    }

    SECTION("access above MAX_INDEX throws") {
        REQUIRE_THROWS_AS(arr[str_object::MAX_INDEX + 1], std::out_of_range);
    }

    SECTION("fill_n out of range throws") {
        REQUIRE_THROWS_AS(arr.fill_n('X', 10, str_object::MAX_INDEX - 5), std::out_of_range);
    }

    SECTION("copy_n out of range throws") {
        const char *src = "test";
        REQUIRE_THROWS_AS(arr.copy_n(src, 10, str_object::MAX_INDEX - 5), std::out_of_range);
    }

    SECTION("slice out of range throws") {
        REQUIRE_THROWS_AS(arr.slice(str_object::MAX_INDEX - 5, 10), std::out_of_range);
    }
}

TEST_CASE("str_object overflow protection", "[str_object]") {
    str_object arr;

    SECTION("large offset causes overflow check") {
        // Attempting operations that would overflow should throw
        size_t large_index = std::numeric_limits<size_t>::max() - 10;
        REQUIRE_THROWS_AS(arr[large_index], std::out_of_range);
    }

    SECTION("fill_n with values causing overflow") {
        size_t large_n = std::numeric_limits<size_t>::max();
        REQUIRE_THROWS_AS(arr.fill_n('X', large_n, 1), std::out_of_range);
    }
}

TEST_CASE("str_object edge cases", "[str_object]") {
    SECTION("operations at MIN_INDEX") {
        str_object arr(' ');
        arr[str_object::MIN_INDEX] = 'X';
        REQUIRE(arr[str_object::MIN_INDEX] == 'X');

        arr.fill_n('Y', 1, str_object::MIN_INDEX);
        REQUIRE(arr[str_object::MIN_INDEX] == 'Y');
    }

    SECTION("operations at MAX_INDEX") {
        str_object arr(' ');
        arr[str_object::MAX_INDEX] = 'Z';
        REQUIRE(arr[str_object::MAX_INDEX] == 'Z');

        arr.fill_n('W', 1, str_object::MAX_INDEX);
        REQUIRE(arr[str_object::MAX_INDEX] == 'W');
    }

    SECTION("fill entire range") {
        str_object arr(' ');
        arr.fill('X', str_object::MIN_INDEX, str_object::MAX_INDEX);
        REQUIRE(arr[str_object::MIN_INDEX] == 'X');
        REQUIRE(arr[str_object::MAX_INDEX] == 'X');
    }
}

TEST_CASE("str_object method chaining", "[str_object]") {
    SECTION("chain fill and copy operations") {
        str_object arr(' ');
        arr.fill('X').fill_n('Y', 5, 1).fill_n('Z', 3, 10);

        REQUIRE(arr[1] == 'Y');
        REQUIRE(arr[6] == 'X');
        REQUIRE(arr[10] == 'Z');
    }

    SECTION("chain insert and erase") {
        str_object arr(' ');
        arr.fillcopy("ABCD", 1, 4, ' ')
           .insert(2, 3)
           .erase(1, 1);

        REQUIRE(arr[1] == 'B');
        REQUIRE(arr[2] == 'C');
        REQUIRE(arr[3] == 'D');
        REQUIRE(arr[4] == 'C');
    }
}