#include "parray.h"
#include "gtest/gtest.h"

#include <iostream>

namespace {

    // We'll use these everywhere
    typedef prange<1, 10>                one_to_ten;
    typedef parray<char, one_to_ten>     ten_chars;

    // Constructor with single initialisation value.
    TEST(parray_test, construct_single_value) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1,  ten_chars::index_type::min());
        EXPECT_EQ(10,  ten_chars::index_type::max());
        EXPECT_EQ(10u, ten_chars::index_type::size());

        ten_chars test0;
        size_t count = 0;
        // Make sure that begin() and end() are working ok too.
        // Default here should be char() which is 0.
        for (auto ch : test0) {
            EXPECT_EQ(0, ch);
            count += 1;
        }
        // Same thing, only with spaces and an index.
        // This should test operator[] const.
        EXPECT_EQ(ten_chars::index_type::size(), count);
        const ten_chars test1(' ');
        for (int i = 1; i <= 10; ++i) {
            EXPECT_EQ(' ', test1[i]);
        }
    }

    // Constructor with pointer to array of values.
    TEST(parray_test, construct_pointer) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        // Excess characters ignored
        ten_chars test1("0123456789ABCD");
        for (int i = 1; i <= 10; ++i) {
            EXPECT_EQ('0' + i - 1, test1[i]);
        }
        // This one includes the trailing '\0'.
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

        // Result string should be ABABABABAB
        ten_chars test1({'A', 'B'});
        for (int i = 1; i <= 10; i += 2) {
            EXPECT_EQ('A', test1[i]);
            EXPECT_EQ('B', test1[i + 1]);
        }

        EXPECT_THROW({
                // Empty init list
                ten_chars test2({});
                EXPECT_EQ(999, test2[1]);
            }, std::out_of_range);
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

        EXPECT_THROW({
                // Src index underflow
                test1.copy(test2, 0, 1, 5);
            }, std::out_of_range);
        EXPECT_THROW({
                // Src index overflow
                test1.copy(test2, 7, 5);
            }, std::out_of_range);
        EXPECT_THROW({
                // Dst index underflow
                test1.copy(test2, 1, 1, 0);
            }, std::out_of_range);
        EXPECT_THROW({
                // Dst index overflow
                test1.copy(test2, 1, 5, 7);
            }, std::out_of_range);
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
        EXPECT_THROW({
                // Dst index underflow
                test1.copy_n("ABCDE", 5, 0);
            }, std::out_of_range);
        EXPECT_THROW({
                // Dst index overflow
                test1.copy_n("ABCDE", 5, 7);
            }, std::out_of_range);
    }

    // Test the fillcopy method.
    TEST(parray_test, method_fillcopy) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        ten_chars test1('X');
        // Src shorter than target -> fill
        test1.fillcopy("Y", 1, 1, ten_chars::index_type::size(), ' ');
        EXPECT_EQ('Y', test1[1]);
        for (int i = 2; i <= 10; ++i) {
            EXPECT_EQ(' ', test1[i]);
        }
        test1.fill('X');
        // Src longer than target -> no fill
        test1.fillcopy("ABCDEFGHIJKLMNOPQRST", 20, 1, ten_chars::index_type::size(), ' ');
        for (int i = 1; i <= 10; ++i) {
            EXPECT_EQ('A' + i - 1, test1[i]);
        }
        EXPECT_THROW({
                // Dst index underflow
                test1.fillcopy("ABCDE", 5, 0, 5, ' ');
            }, std::out_of_range);
        EXPECT_THROW({
                // Dst index overflow
                test1.fillcopy("ABCDE", 5, 7, 5, ' ');
            }, std::out_of_range);
    }

    // Test the fill method.
    TEST(parray_test, method_fill) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        // Create full of 'X' and then replace with space.
        ten_chars test1('X');
        test1.fill(' ');
        for (int i = 1; i <= 10; ++i) {
            EXPECT_EQ(' ', test1[i]);
        }
        // Fill right half with 'X' again.
        test1.fill('X', 6, 10);
        for (int i = 1; i <= 5; ++i) {
            EXPECT_EQ(' ', test1[i]);
        }
        for (int i = 6; i <= 10; ++i) {
            EXPECT_EQ('X', test1[i]);
        }

        EXPECT_THROW({
                // Dst index underflow
                test1.fill('.', 0);
            }, std::out_of_range);
        EXPECT_THROW({
                // Dst index overflow
                test1.fill('.', 1, 11);
            }, std::out_of_range);
    }

    // Test the fill_n method.
    TEST(parray_test, method_fill_n) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        // This is the same test as fill(...), but
        // modified for fill_n(...).
        ten_chars test1('X');
        test1.fill_n(' ', ten_chars::index_type::size());
        for (int i = 1; i <= 10; ++i) {
            EXPECT_EQ(' ', test1[i]);
        }
        test1.fill_n('X', 5, 6);
        for (int i = 1; i <= 5; ++i) {
            EXPECT_EQ(' ', test1[i]);
        }
        for (int i = 6; i <= 10; ++i) {
            EXPECT_EQ('X', test1[i]);
        }

        EXPECT_THROW({
                // Dst index underflow
                test1.fill_n('.', 1, 0);
            }, std::out_of_range);
        EXPECT_THROW({
                // Dst index overflow
                test1.fill_n('.', 10, 2);
            }, std::out_of_range);
    }

    // Test the apply_n method.
    TEST(parray_test, method_apply_n) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        ten_chars test1('X');
        test1.apply_n([](char) -> char { return ' '; }, 10);
        for (int i = 1; i <= 10; ++i) {
            EXPECT_EQ(' ', test1[i]);
        }
        test1.apply_n([](char ch) -> char { return ch + 1; }, 5, 6);
        for (int i = 1; i <= 5; ++i) {
            EXPECT_EQ(' ', test1[i]);
        }
        for (int i = 6; i <= 10; ++i) {
            EXPECT_EQ(' ' + 1, test1[i]);
        }

        EXPECT_THROW({
                // Dst index underflow
                test1.apply_n([](char) -> char { return ' '; }, 1, 0);
            }, std::out_of_range);
        EXPECT_THROW({
                // Dst index overflow
                test1.apply_n([](char) -> char { return ' '; }, 6, 6);
            }, std::out_of_range);
    }

    // Test the insert method.
    TEST(parray_test, method_insert) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        ten_chars test1("1234567890");
        test1.insert(1, 1);
        EXPECT_EQ('1', test1[1]);               // Old value remains
        for (int i = 2; i <= 10; ++i) {         // Other values moved right
            EXPECT_EQ('0' + i - 1, test1[i]);
        }
        test1.copy_n("0123456789", 10);
        test1.insert(2, 6);
        for (int i = 1; i <= 7; ++i) {
            EXPECT_EQ('0' + i - 1, test1[i]);
        }
        EXPECT_EQ('5', test1[8]);
        EXPECT_EQ('6', test1[9]);
        EXPECT_EQ('7', test1[10]);

        EXPECT_THROW({
                // Dst index underflow
                test1.insert(1, 0);
            }, std::out_of_range);
        EXPECT_THROW({
                // Dst index overflow
                test1.insert(6, 6);
            }, std::out_of_range);
    }

    // Test the erase method.
    TEST(parray_test, method_erase) {
        // We test these to avoid unexpected failures elsewhere.
        EXPECT_EQ( 1, ten_chars::index_type::min());
        EXPECT_EQ(10, ten_chars::index_type::max());

        ten_chars test1("0123456789");
        test1.erase(1, 1);
        EXPECT_EQ('1', test1[1]);
        for (int i = 1; i <= 9; ++i) {
            EXPECT_EQ('0' + i, test1[i]);
        }
        // Previous value remains in place at end.
        EXPECT_EQ('9', test1[10]);


        test1.copy_n("0123456789", 10);
        test1.erase(2, 6);
        // 01234
        for (int i = 1; i <= 5; ++i) {
            EXPECT_EQ('0' + i - 1, test1[i]);
        }
        // 789
        for (int i = 6; i <= 8; ++i) {
            EXPECT_EQ('1' + i, test1[i]);
        }
        // Previous values remain in place at end.
        EXPECT_EQ('8', test1[9]);
        EXPECT_EQ('9', test1[10]);

        EXPECT_THROW({
                // Dst index underflow
                test1.erase(1, 0);
            }, std::out_of_range);
        EXPECT_THROW({
                // Dst index overflow
                test1.erase(6, 6);
            }, std::out_of_range);
    }

} // namespace
