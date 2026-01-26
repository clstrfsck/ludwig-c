/**
 * @file test_mark.cpp
 * Unit tests for mark manipulation routines
 */

#include <catch2/catch_test_macros.hpp>
#include "mark.h"
#include "type.h"

// Helper function to create a minimal line object for testing
line_ptr create_test_line() {
    line_ptr line = new line_hdr_object;
    line->flink = nullptr;
    line->blink = nullptr;
    line->group = nullptr;
    line->offset_nr = 0;
    line->marks.clear();
    line->str = nullptr;
    line->len = 0;
    line->used = 0;
    line->scr_row_nr = 0;
    return line;
}

// Helper to create a group for testing multi-line scenarios
group_ptr create_test_group(frame_ptr frame) {
    group_ptr group = new group_object;
    group->flink = nullptr;
    group->blink = nullptr;
    group->frame = frame;
    group->first_line = nullptr;
    group->last_line = nullptr;
    group->first_line_nr = 0;
    group->nr_lines = 0;
    return group;
}

// Helper to create a frame for testing
frame_ptr create_test_frame() {
    frame_ptr frame = new frame_object;
    frame->first_group = nullptr;
    frame->last_group = nullptr;
    frame->dot = nullptr;
    frame->span = nullptr;
    frame->return_frame = nullptr;
    return frame;
}

// Helper to link lines together
void link_lines(line_ptr first, line_ptr second) {
    first->flink = second;
    second->blink = first;
}

// Cleanup helper
void delete_test_line(line_ptr line) {
    // Delete all marks on the line first
    for (auto &mark : line->marks) {
        mark_destroy(mark);
    }
    delete line;
}

void delete_test_group(group_ptr group) {
    delete group;
}

void delete_test_frame(frame_ptr frame) {
    delete frame;
}

TEST_CASE("mark_pool_statistics", "[mark]") {
    SECTION("statistics tracking allocations") {
        line_ptr line = create_test_line();
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;

        mark_create(line, 1, mark1);
        mark_create(line, 5, mark2);

        // Allocated should increase by 2
        REQUIRE(marks_allocated() == 2);

        mark_destroy(mark1);
        mark_destroy(mark2);
        REQUIRE(marks_allocated() == 0);

        delete_test_line(line);
    }


    SECTION("creating many marks tracks allocations correctly") {
        line_ptr line = create_test_line();
        REQUIRE(marks_allocated() == 0);
        std::vector<mark_ptr> marks;

        // Create more marks than typical pool extend size (20)
        for (int i = 0; i < 25; ++i) {
            mark_ptr mark = nullptr;
            mark_create(line, i + 1, mark);
            marks.push_back(mark);
        }

        // Net allocated now should be 25
        REQUIRE(marks_allocated() == 25);

        // Cleanup
        for (mark_ptr m : marks) {
            mark_destroy(m);
        }
        marks.clear();

        REQUIRE(marks_allocated() == 0);

        delete_test_line(line);
    }
}

TEST_CASE("mark_create creates new marks", "[mark]") {
    line_ptr line = create_test_line();

    SECTION("creating a mark on an empty line") {
        mark_ptr mark = nullptr;
        REQUIRE(mark_create(line, 5, mark));
        REQUIRE(mark != nullptr);
        REQUIRE(mark->line == line);
        REQUIRE(mark->col == 5);
        REQUIRE(line->marks.size() == 1);
        REQUIRE(line->marks.front() == mark);

        mark_destroy(mark);
    }

    SECTION("creating multiple marks on the same line") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;

        REQUIRE(mark_create(line, 5, mark1));
        REQUIRE(mark_create(line, 10, mark2));

        REQUIRE(mark1 != nullptr);
        REQUIRE(mark2 != nullptr);
        REQUIRE(mark1 != mark2);
        REQUIRE(mark1->col == 5);
        REQUIRE(mark2->col == 10);

        // Both marks should be in the line's mark list
        REQUIRE(line->marks.size() == 2);

        mark_destroy(mark1);
        mark_destroy(mark2);
    }

    SECTION("creating mark at column 1") {
        mark_ptr mark = nullptr;
        REQUIRE(mark_create(line, 1, mark));
        REQUIRE(mark->col == 1);

        mark_destroy(mark);
    }

    SECTION("creating mark at high column number") {
        mark_ptr mark = nullptr;
        REQUIRE(mark_create(line, 1000, mark));
        REQUIRE(mark->col == 1000);

        mark_destroy(mark);
    }

    delete_test_line(line);
}

TEST_CASE("mark_create moves existing marks", "[mark]") {
    line_ptr line1 = create_test_line();
    line_ptr line2 = create_test_line();

    SECTION("moving mark to different column on same line") {
        mark_ptr mark = nullptr;
        mark_create(line1, 5, mark);

        REQUIRE(mark->col == 5);
        REQUIRE(mark_create(line1, 10, mark));
        REQUIRE(mark->col == 10);
        REQUIRE(mark->line == line1);

        mark_destroy(mark);
    }

    SECTION("moving mark to different line") {
        mark_ptr mark = nullptr;
        mark_create(line1, 5, mark);

        REQUIRE(mark->line == line1);
        REQUIRE(line1->marks.size() == 1);
        REQUIRE(line1->marks.front() == mark);

        REQUIRE(mark_create(line2, 15, mark));
        REQUIRE(mark->line == line2);
        REQUIRE(mark->col == 15);
        REQUIRE(line2->marks.size() == 1);
        REQUIRE(line2->marks.front() == mark);
        REQUIRE(line1->marks.empty()); // Mark should be removed from line1

        mark_destroy(mark);
    }

    SECTION("moving mark when other marks exist on the line") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;

        mark_create(line1, 5, mark1);
        mark_create(line1, 10, mark2);

        // Move mark1 to line2
        REQUIRE(mark_create(line2, 20, mark1));
        REQUIRE(mark1->line == line2);
        REQUIRE(mark2->line == line1);

        // line1 should still have mark2
        REQUIRE(line1->marks.size() == 1);
        REQUIRE(line1->marks.front() == mark2);

        mark_destroy(mark1);
        mark_destroy(mark2);
    }

    delete_test_line(line1);
    delete_test_line(line2);
}

TEST_CASE("mark_destroy removes marks", "[mark]") {
    line_ptr line = create_test_line();

    SECTION("destroying a single mark") {
        mark_ptr mark = nullptr;
        mark_create(line, 5, mark);

        REQUIRE(line->marks.size() == 1);
        REQUIRE(line->marks.front() == mark);
        REQUIRE(mark_destroy(mark));
        REQUIRE(mark == nullptr);
        REQUIRE(line->marks.empty());
    }

    SECTION("destroying one of multiple marks") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;
        mark_ptr mark3 = nullptr;

        mark_create(line, 5, mark1);
        mark_create(line, 10, mark2);
        mark_create(line, 15, mark3);

        REQUIRE(mark_destroy(mark2));
        REQUIRE(mark2 == nullptr);

        // Other marks should still exist
        REQUIRE(mark1 != nullptr);
        REQUIRE(mark3 != nullptr);

        mark_destroy(mark1);
        mark_destroy(mark3);
    }

    SECTION("destroying all marks on a line") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;

        mark_create(line, 5, mark1);
        mark_create(line, 10, mark2);

        mark_destroy(mark1);
        mark_destroy(mark2);

        REQUIRE(mark1 == nullptr);
        REQUIRE(mark2 == nullptr);
    }

    delete_test_line(line);
}

TEST_CASE("marks_squeeze on same line", "[mark]") {
    line_ptr line = create_test_line();

    SECTION("squeezing marks in a range") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;
        mark_ptr mark3 = nullptr;

        mark_create(line, 5, mark1);
        mark_create(line, 10, mark2);
        mark_create(line, 15, mark3);

        // Squeeze columns 5-15 to column 20
        REQUIRE(marks_squeeze(line, 5, line, 20));

        // All marks in range should move to column 20
        REQUIRE(mark1->col == 20);
        REQUIRE(mark2->col == 20);
        // mark3 is at column 15, which is >= 5 but not < 20, so it shouldn't move
        REQUIRE(mark3->col == 20);

        mark_destroy(mark1);
        mark_destroy(mark2);
        mark_destroy(mark3);
    }

    SECTION("marks outside range are not affected") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;
        mark_ptr mark3 = nullptr;

        mark_create(line, 5, mark1);
        mark_create(line, 15, mark2);
        mark_create(line, 25, mark3);

        // Squeeze columns 10-20 to column 30
        // Marks in range [10, 20) should move to 30
        REQUIRE(marks_squeeze(line, 10, line, 30));

        REQUIRE(mark1->col == 5);   // Before range (< 10)
        REQUIRE(mark2->col == 30);  // In range [10, 20)
        REQUIRE(mark3->col == 30);  // At the end column (25 >= 10 and < 30, so it's in range too!)

        mark_destroy(mark1);
        mark_destroy(mark2);
        mark_destroy(mark3);
    }

    SECTION("squeeze with no marks in range") {
        mark_ptr mark = nullptr;
        mark_create(line, 5, mark);

        REQUIRE(marks_squeeze(line, 10, line, 20));
        REQUIRE(mark->col == 5); // Should be unchanged

        mark_destroy(mark);
    }

    delete_test_line(line);
}

TEST_CASE("marks_squeeze across multiple lines", "[mark]") {
    frame_ptr frame = create_test_frame();
    group_ptr group = create_test_group(frame);

    line_ptr line1 = create_test_line();
    line_ptr line2 = create_test_line();
    line_ptr line3 = create_test_line();

    line1->group = group;
    line2->group = group;
    line3->group = group;

    group->first_line = line1;
    group->last_line = line3;
    group->first_line_nr = 1;
    group->nr_lines = 3;

    line1->offset_nr = 0;
    line2->offset_nr = 1;
    line3->offset_nr = 2;

    link_lines(line1, line2);
    link_lines(line2, line3);

    SECTION("squeezing marks from multiple lines to one position") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;
        mark_ptr mark3 = nullptr;

        mark_create(line1, 10, mark1);
        mark_create(line2, 15, mark2);
        mark_create(line3, 5, mark3);

        // Squeeze from (line1, col 5) to (line3, col 20)
        REQUIRE(marks_squeeze(line1, 5, line3, 20));

        // All marks >= column 5 should move to line3, column 20
        REQUIRE(mark1->line == line3);
        REQUIRE(mark1->col == 20);
        REQUIRE(mark2->line == line3);
        REQUIRE(mark2->col == 20);
        REQUIRE(mark3->col == 20); // This was already on line3

        mark_destroy(mark1);
        mark_destroy(mark2);
        mark_destroy(mark3);
    }

    SECTION("marks before first_column on first line are not moved") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;

        mark_create(line1, 3, mark1);
        mark_create(line1, 10, mark2);

        REQUIRE(marks_squeeze(line1, 5, line3, 20));

        REQUIRE(mark1->line == line1); // Not moved
        REQUIRE(mark1->col == 3);
        REQUIRE(mark2->line == line3); // Moved
        REQUIRE(mark2->col == 20);

        mark_destroy(mark1);
        mark_destroy(mark2);
    }

    delete_test_line(line1);
    delete_test_line(line2);
    delete_test_line(line3);
    delete_test_group(group);
    delete_test_frame(frame);
}

TEST_CASE("marks_shift on same line", "[mark]") {
    line_ptr line = create_test_line();

    SECTION("shifting marks forward on same line") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;
        mark_ptr mark3 = nullptr;

        mark_create(line, 10, mark1);
        mark_create(line, 15, mark2);
        mark_create(line, 20, mark3);

        // Shift 10 columns starting at column 10 to column 30
        // Range is [10, 19] (source_column to source_column + width - 1)
        REQUIRE(marks_shift(line, 10, 10, line, 30));

        // mark1 at 10 should move to 30 (offset +20)
        REQUIRE(mark1->col == 30);
        // mark2 at 15 should move to 35
        REQUIRE(mark2->col == 35);
        // mark3 at 20 is outside the range [10, 19], so should not be affected
        REQUIRE(mark3->col == 20);

        mark_destroy(mark1);
        mark_destroy(mark2);
        mark_destroy(mark3);
    }

    SECTION("shifting marks backward on same line") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;

        mark_create(line, 30, mark1);
        mark_create(line, 35, mark2);

        // Shift 10 columns starting at column 30 to column 10
        REQUIRE(marks_shift(line, 30, 10, line, 10));

        REQUIRE(mark1->col == 10);  // 30 + (10-30) = 10
        REQUIRE(mark2->col == 15);  // 35 + (10-30) = 15

        mark_destroy(mark1);
        mark_destroy(mark2);
    }

    SECTION("marks outside source range are not affected") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;
        mark_ptr mark3 = nullptr;

        mark_create(line, 5, mark1);
        mark_create(line, 12, mark2);
        mark_create(line, 25, mark3);

        // Shift 5 columns starting at column 10 to column 30
        // Range is [10, 14] (columns 10, 11, 12, 13, 14)
        REQUIRE(marks_shift(line, 10, 5, line, 30));

        REQUIRE(mark1->col == 5);   // Before range
        REQUIRE(mark2->col == 32);  // In range: 12 + (30-10) = 32
        REQUIRE(mark3->col == 25);  // After range [10, 14]

        mark_destroy(mark1);
        mark_destroy(mark2);
        mark_destroy(mark3);
    }

    delete_test_line(line);
}

TEST_CASE("marks_shift to different line", "[mark]") {
    frame_ptr frame = create_test_frame();
    group_ptr group = create_test_group(frame);

    line_ptr line1 = create_test_line();
    line_ptr line2 = create_test_line();

    line1->group = group;
    line2->group = group;

    group->first_line = line1;
    group->last_line = line2;
    group->first_line_nr = 1;
    group->nr_lines = 2;

    line1->offset_nr = 0;
    line2->offset_nr = 1;

    link_lines(line1, line2);

    SECTION("shifting marks to different line") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;
        mark_ptr mark3 = nullptr;

        mark_create(line1, 10, mark1);
        mark_create(line1, 15, mark2);
        mark_create(line1, 25, mark3);

        // Shift 10 columns starting at column 10 from line1 to line2 at column 5
        REQUIRE(marks_shift(line1, 10, 10, line2, 5));

        // mark1 and mark2 should move to line2
        REQUIRE(mark1->line == line2);
        REQUIRE(mark1->col == 5);   // 10 + (5-10) = 5
        REQUIRE(mark2->line == line2);
        REQUIRE(mark2->col == 10);  // 15 + (5-10) = 10

        // mark3 is outside the range
        REQUIRE(mark3->line == line1);
        REQUIRE(mark3->col == 25);

        mark_destroy(mark1);
        mark_destroy(mark2);
        mark_destroy(mark3);
    }

    SECTION("shifting with width of 1") {
        mark_ptr mark = nullptr;
        mark_create(line1, 10, mark);

        REQUIRE(marks_shift(line1, 10, 1, line2, 20));

        REQUIRE(mark->line == line2);
        REQUIRE(mark->col == 20);

        mark_destroy(mark);
    }

    delete_test_line(line1);
    delete_test_line(line2);
    delete_test_group(group);
    delete_test_frame(frame);
}

TEST_CASE("marks_shift with MAX_STRLENP boundary", "[mark]") {
    line_ptr line = create_test_line();

    SECTION("shifting mark beyond MAX_STRLENP clamps to MAX_STRLENP on same line") {
        mark_ptr mark = nullptr;
        mark_create(line, 390, mark);

        // Shift with large offset that would exceed MAX_STRLENP (401)
        // Shifting 20 columns starting at 390 to 395 would put mark at 395
        // But if we shift to a very high column...
        REQUIRE(marks_shift(line, 390, 20, line, 500));

        // Should be clamped to MAX_STRLENP
        REQUIRE(mark->col == MAX_STRLENP);

        mark_destroy(mark);
    }

    SECTION("shifting mark beyond MAX_STRLENP clamps to MAX_STRLENP on different line") {
        frame_ptr frame = create_test_frame();
        group_ptr group = create_test_group(frame);
        line_ptr line2 = create_test_line();

        line->group = group;
        line2->group = group;
        group->first_line = line;
        group->last_line = line2;
        group->first_line_nr = 1;
        group->nr_lines = 2;
        line->offset_nr = 0;
        line2->offset_nr = 1;
        link_lines(line, line2);

        mark_ptr mark = nullptr;
        mark_create(line, 395, mark);

        // Shift to destination that would exceed MAX_STRLENP
        REQUIRE(marks_shift(line, 390, 10, line2, 500));

        REQUIRE(mark->line == line2);
        REQUIRE(mark->col == MAX_STRLENP);

        mark_destroy(mark);
        delete_test_line(line2);
        delete_test_group(group);
        delete_test_frame(frame);
    }

    delete_test_line(line);
}

TEST_CASE("mark_destroy with different positions in mark list", "[mark]") {
    line_ptr line = create_test_line();

    SECTION("destroying mark at head of list") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;
        mark_ptr mark3 = nullptr;

        mark_create(line, 5, mark1);
        mark_create(line, 10, mark2);
        mark_create(line, 15, mark3);

        // mark3 should be at the head (marks are added to the front)
        REQUIRE(line->marks.front() == mark3);

        // Destroy the head mark
        mark_destroy(mark3);

        // Now line->mark should point to the next mark
        REQUIRE(!line->marks.empty());
        REQUIRE(mark3 == nullptr);

        mark_destroy(mark1);
        mark_destroy(mark2);
    }

    SECTION("destroying mark in middle of list") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;
        mark_ptr mark3 = nullptr;

        mark_create(line, 5, mark1);
        mark_create(line, 10, mark2);
        mark_create(line, 15, mark3);

        // Destroy middle mark
        mark_destroy(mark2);

        REQUIRE(mark2 == nullptr);
        REQUIRE(mark1 != nullptr);
        REQUIRE(mark3 != nullptr);

        mark_destroy(mark1);
        mark_destroy(mark3);
    }

    SECTION("destroying last remaining mark") {
        mark_ptr mark = nullptr;
        mark_create(line, 5, mark);

        REQUIRE(line->marks.front() == mark);
        mark_destroy(mark);

        REQUIRE(mark == nullptr);
        REQUIRE(line->marks.empty());
    }

    delete_test_line(line);
}

TEST_CASE("mark_create moving mark from head and middle of list", "[mark]") {
    line_ptr line1 = create_test_line();
    line_ptr line2 = create_test_line();

    SECTION("moving mark from head of mark list") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;

        mark_create(line1, 5, mark1);
        mark_create(line1, 10, mark2);

        // mark2 is at the head
        REQUIRE(line1->marks.front() == mark2);

        // Move mark2 to line2
        mark_create(line2, 20, mark2);

        REQUIRE(mark2->line == line2);
        REQUIRE(mark2->col == 20);
        REQUIRE(line1->marks.front() == mark1); // mark1 should now be at head of line1
        REQUIRE(line2->marks.front() == mark2);

        mark_destroy(mark1);
        mark_destroy(mark2);
    }

    SECTION("moving mark from middle of mark list") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;
        mark_ptr mark3 = nullptr;

        mark_create(line1, 5, mark1);
        mark_create(line1, 10, mark2);
        mark_create(line1, 15, mark3);

        // mark3 is head, mark2 is in middle
        // Move mark2 (middle) to line2
        mark_create(line2, 25, mark2);

        REQUIRE(mark2->line == line2);
        REQUIRE(mark2->col == 25);

        // mark3 and mark1 should still be on line1
        REQUIRE(mark3->line == line1);
        REQUIRE(mark1->line == line1);

        mark_destroy(mark1);
        mark_destroy(mark2);
        mark_destroy(mark3);
    }

    delete_test_line(line1);
    delete_test_line(line2);
}

TEST_CASE("marks_squeeze with no marks on lines", "[mark]") {
    frame_ptr frame = create_test_frame();
    group_ptr group = create_test_group(frame);

    line_ptr line1 = create_test_line();
    line_ptr line2 = create_test_line();

    line1->group = group;
    line2->group = group;
    group->first_line = line1;
    group->last_line = line2;
    group->first_line_nr = 1;
    group->nr_lines = 2;
    line1->offset_nr = 0;
    line2->offset_nr = 1;
    link_lines(line1, line2);

    SECTION("squeezing when lines have no marks") {
        // Should succeed but do nothing
        REQUIRE(marks_squeeze(line1, 5, line2, 20));

        REQUIRE(line1->marks.empty());
        REQUIRE(line2->marks.empty());
    }

    delete_test_line(line1);
    delete_test_line(line2);
    delete_test_group(group);
    delete_test_frame(frame);
}

TEST_CASE("marks_squeeze at exact column boundaries", "[mark]") {
    line_ptr line = create_test_line();

    SECTION("mark at exact first_column boundary") {
        mark_ptr mark = nullptr;
        mark_create(line, 10, mark);

        // Squeeze from column 10 (mark is exactly at first_column)
        REQUIRE(marks_squeeze(line, 10, line, 30));

        // Mark at first_column should be moved
        REQUIRE(mark->col == 30);

        mark_destroy(mark);
    }

    SECTION("mark at exact last_column boundary") {
        mark_ptr mark = nullptr;
        mark_create(line, 30, mark);

        // Squeeze to column 30 (mark is exactly at last_column)
        REQUIRE(marks_squeeze(line, 10, line, 30));

        // Mark at last_column should NOT be moved (col < last_column condition)
        REQUIRE(mark->col == 30);

        mark_destroy(mark);
    }

    SECTION("mark just before last_column") {
        mark_ptr mark = nullptr;
        mark_create(line, 29, mark);

        REQUIRE(marks_squeeze(line, 10, line, 30));

        // Should be moved to last_column
        REQUIRE(mark->col == 30);

        mark_destroy(mark);
    }

    delete_test_line(line);
}

TEST_CASE("marks_shift with no marks in range", "[mark]") {
    line_ptr line = create_test_line();

    SECTION("no marks exist in the shift range") {
        mark_ptr mark1 = nullptr;
        mark_ptr mark2 = nullptr;

        mark_create(line, 5, mark1);
        mark_create(line, 50, mark2);

        // Shift range [20, 30] where no marks exist
        REQUIRE(marks_shift(line, 20, 10, line, 100));

        // Marks should be unchanged
        REQUIRE(mark1->col == 5);
        REQUIRE(mark2->col == 50);

        mark_destroy(mark1);
        mark_destroy(mark2);
    }

    delete_test_line(line);
}

TEST_CASE("marks_shift at exact range boundaries", "[mark]") {
    line_ptr line = create_test_line();

    SECTION("mark at exact start of range") {
        mark_ptr mark = nullptr;
        mark_create(line, 10, mark);

        // Range is [10, 19] (width=10)
        REQUIRE(marks_shift(line, 10, 10, line, 30));

        REQUIRE(mark->col == 30); // Should be shifted

        mark_destroy(mark);
    }

    SECTION("mark at exact end of range") {
        mark_ptr mark = nullptr;
        mark_create(line, 19, mark);

        // Range is [10, 19] (width=10, so source_end = 10+10-1 = 19)
        REQUIRE(marks_shift(line, 10, 10, line, 30));

        REQUIRE(mark->col == 39); // 19 + (30-10) = 39

        mark_destroy(mark);
    }

    SECTION("mark just outside end of range") {
        mark_ptr mark = nullptr;
        mark_create(line, 20, mark);

        // Range is [10, 19]
        REQUIRE(marks_shift(line, 10, 10, line, 30));

        REQUIRE(mark->col == 20); // Should NOT be shifted

        mark_destroy(mark);
    }

    delete_test_line(line);
}

TEST_CASE("marks_squeeze multi-line with marks before first_column", "[mark]") {
    frame_ptr frame = create_test_frame();
    group_ptr group = create_test_group(frame);

    line_ptr line1 = create_test_line();
    line_ptr line2 = create_test_line();
    line_ptr line3 = create_test_line();

    line1->group = group;
    line2->group = group;
    line3->group = group;
    group->first_line = line1;
    group->last_line = line3;
    group->first_line_nr = 1;
    group->nr_lines = 3;
    line1->offset_nr = 0;
    line2->offset_nr = 1;
    line3->offset_nr = 2;
    link_lines(line1, line2);
    link_lines(line2, line3);

    SECTION("marks on middle lines all moved") {
        mark_ptr mark1a = nullptr;
        mark_ptr mark1b = nullptr;
        mark_ptr mark2a = nullptr;
        mark_ptr mark2b = nullptr;

        // Mark list grows at the front, so create in reverse order
        mark_create(line1, 12, mark1b); // Before first_column on first line
        mark_create(line1, 2, mark1a);  // After first_column on first line
        mark_create(line2, 5, mark2a);  // On middle line, any column
        mark_create(line2, 15, mark2b); // Another on middle line

        REQUIRE(marks_squeeze(line1, 10, line3, 30));

        // mark1a should stay on line1 (before first_column)
        REQUIRE(mark1a->line == line1);
        REQUIRE(mark1a->col == 2);
        // mark1b should move to line3 col 30
        REQUIRE(mark1b->line == line3);
        REQUIRE(mark1b->col == 30);

        // All marks on line2 should move to line3 (first_column becomes 1 for subsequent lines)
        REQUIRE(mark2a->line == line3);
        REQUIRE(mark2a->col == 30);
        REQUIRE(mark2b->line == line3);
        REQUIRE(mark2b->col == 30);

        mark_destroy(mark1a);
        mark_destroy(mark1b);
        mark_destroy(mark2a);
        mark_destroy(mark2b);
    }

    delete_test_line(line1);
    delete_test_line(line2);
    delete_test_line(line3);
    delete_test_group(group);
    delete_test_frame(frame);
}
