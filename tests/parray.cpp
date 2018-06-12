#include "parray.h"
#include "gtest/gtest.h"

#include <iostream>

namespace {

    // We'll use these everywhere
    typedef prange<1, 10>                one_to_ten;
    typedef parray<char, one_to_ten>     ten_chars;
    typedef prange<1, 100>               one_to_hundred;
    typedef parray<char, one_to_hundred> hundred_chars;

    // Constructor with single initialisation value.
    TEST(parray_test, construct_single_value) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ(  1, hundred_chars::index_type::min());
        EXPECT_EQ(100, hundred_chars::index_type::max());

        hundred_chars test0;
        for (int i = 1; i <= 100; ++i) {
            EXPECT_EQ(0, test0[i]);
        }
        hundred_chars test1(' ');
        for (int i = 1; i <= 100; ++i) {
            EXPECT_EQ(' ', test1[i]);
        }
    }

    // Constructor with pointer to array of values.
    TEST(parray_test, construct_pointer) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        ten_chars test1("0123456789ABCD");
        for (int i = 1; i <= 10; ++i) {
            EXPECT_EQ('0' + i - 1, test1[i]);
        }
        ten_chars test2("ABCDEFGHI");
        for (int i = 1; i <= 9; ++i) {
            EXPECT_EQ('A' + i - 1, test2[i]);
        }
        EXPECT_EQ(0, test2[10]);
    }

    // Test construction with a repeating init list
    TEST(parray_test, construct_init_list) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        ten_chars test1({'A', 'B'});
        for (int i = 1; i <= 10; i += 2) {
            EXPECT_EQ('A', test1[i]);
            EXPECT_EQ('B', test1[i + 1]);
        }
    }

    // Test copy construction.
    TEST(parray_test, construct_copy) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        ten_chars test1({'A', 'B'});
        ten_chars test2(test1);
        for (int i = 1; i <= 10; i += 2) {
            EXPECT_EQ('A', test1[i]);
            EXPECT_EQ('B', test1[i + 1]);
            EXPECT_EQ('A', test2[i]);
            EXPECT_EQ('B', test2[i + 1]);
        }
    }

    // I think we can assume operator[] is working based on other tests

    // Test the copy method.
    TEST(parray_test, method_copy) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        ten_chars test1("ABCDEXXXXX");
        ten_chars test2("FGHIJYYYYY");
        // Copy test2 from 1 through 5
        test1.copy(test2, 1, 5, 6);
        for (int i = 1; i <= 10; ++i) {
            EXPECT_EQ('A' + i - 1, test1[i]);
        }
    }

    // Test the copy_n method.
    TEST(parray_test, method_copy_n) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        ten_chars test1("ABCDEXXXXX");
        test1.copy_n("FGHIJ", 5, 6);
        for (int i = 1; i <= 10; ++i) {
            EXPECT_EQ('A' + i - 1, test1[i]);
        }
    }

} // namespace
