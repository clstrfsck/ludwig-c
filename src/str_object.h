/** @file str_object.h
 * Declarations for class to support Ludwig str_object.
 */

#if !defined(STR_OBJECT_H)
#define STR_OBJECT_H

#include "const.h"

#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <stdexcept>

class str_object {
private:
    static void check_index(size_t index, size_t offset = 0) {
        if (offset > std::numeric_limits<size_t>::max() - index) {
            throw std::out_of_range("index + offset overflow");
        }

        size_t combined_index = index + offset;

        if (combined_index < MIN_INDEX || combined_index > MAX_INDEX) {
            throw std::out_of_range("index out of range");
        }
    }

    static size_t adjust_index(size_t index, size_t offset = 0) {
        check_index(index, offset);
        return index + offset - MIN_INDEX;
    }

public:
    static constexpr size_t MAX_STRLEN = ::MAX_STRLEN;
    static constexpr size_t MIN_INDEX = 1;
    static constexpr size_t MAX_INDEX = MAX_STRLEN;

    using iterator = std::array<char, MAX_STRLEN>::iterator;
    using const_iterator = std::array<char, MAX_STRLEN>::const_iterator;

public:
    explicit str_object(char elt = ' ') {
        m_array.fill(elt);
    }

    // Initialize with repeating values
    explicit str_object(std::initializer_list<char> values) {
        auto i = values.begin();
        if (i == values.end()) {
            // Empty initializer_list— just fill with spaces
            m_array.fill(' ');
        } else {
            for (size_t j = 0; j < MAX_STRLEN; ++j) {
                m_array[j] = *i++;
                if (i == values.end()) {
                    i = values.begin();
                }
            }
        }
    }

    str_object(const str_object &other) = default;
    ~str_object() = default;

    bool operator==(const str_object &rhs) const = default;
    auto operator<=>(const str_object &rhs) const = default;

    char &operator[](size_t index) {
        return m_array[adjust_index(index)];
    }

    const char &operator[](size_t index) const {
        return m_array[adjust_index(index)];
    }

    str_object &operator=(str_object rhs) noexcept {
        std::swap(m_array, rhs.m_array);
        return *this;
    }

    const_iterator cbegin() const noexcept {
        return m_array.cbegin();
    }

    const_iterator cend() const noexcept {
        return m_array.cend();
    }

    iterator begin() noexcept {
        return m_array.begin();
    }

    iterator end() noexcept {
        return m_array.end();
    }

    const char *data(size_t index = MIN_INDEX) const {
        return m_array.data() + adjust_index(index);
    }

    char *data(size_t index = MIN_INDEX) {
        return m_array.data() + adjust_index(index);
    }

    str_object &apply_n(const std::function<char(char)> &f, size_t n, size_t beg = MIN_INDEX) {
        if (n > 0) {
            check_index(beg, n - 1);
            size_t ibeg = adjust_index(beg);
            std::transform(m_array.begin() + ibeg, m_array.begin() + ibeg + n, m_array.begin() + ibeg, f);
        }
        return *this;
    }

    str_object &copy(
        const str_object &src,
        size_t src_offset,
        size_t count,
        size_t dst_offset = MIN_INDEX
    ) {
        if (count > 0) {
            check_index(dst_offset, count - 1); // Last index used
            src.check_index(src_offset, count - 1);
            auto srcb = src.m_array.begin() + src.adjust_index(src_offset);
            auto srce = srcb + count;
            auto dst = m_array.begin() + adjust_index(dst_offset);
            std::copy(srcb, srce, dst);
        }
        return *this;
    }

    str_object &copy_n(const char *src, size_t count, size_t dst_offset = MIN_INDEX) {
        if (count > 0) {
            check_index(dst_offset, count - 1); // Last index used
            auto dst = m_array.begin() + adjust_index(dst_offset);
            std::copy(src, src + count, dst);
        }
        return *this;
    }

    str_object &erase(size_t n, size_t from) {
        if (n > 0) {
            check_index(from, n - 1);
            auto d = m_array.begin() + adjust_index(from);
            auto b = d + n;
            auto e = m_array.end();
            std::copy(b, e, d);
        }
        return *this;
    }

    str_object &fill(char value, size_t beg = MIN_INDEX, size_t end = MAX_INDEX) {
        size_t ibeg = adjust_index(beg);
        size_t iend = adjust_index(end) + 1;
        if (ibeg < iend) {
            auto b = m_array.begin() + ibeg;
            auto e = m_array.begin() + iend;
            std::fill(b, e, value);
        }
        return *this;
    }

    str_object &fill_n(char value, size_t n, size_t beg = MIN_INDEX) {
        if (n > 0) {
            check_index(beg, n - 1);
            auto b = m_array.begin() + adjust_index(beg);
            std::fill(b, b + n, value);
        }
        return *this;
    }

    str_object &fillcopy(
        const char *src,
        size_t src_len,
        size_t dst_index,
        size_t dst_len,
        char value
    ) {
        if (dst_len > 0) {
            check_index(dst_index, dst_len - 1);
            size_t len = std::min(src_len, dst_len);
            size_t dst = adjust_index(dst_index);
            if (len != 0) {
                std::copy(src, src + len, m_array.begin() + dst);
            }
            if (dst_len > len) {
                auto db = m_array.begin() + dst + len;
                auto de = db + dst_len - len;
                std::fill(db, de, value);
            }
        }
        return *this;
    }

    str_object &insert(size_t n, size_t at) {
        if (n > 0) {
            check_index(at, n - 1);
            auto b = m_array.begin() + adjust_index(at);
            auto e = m_array.end() - n;
            auto de = m_array.end();
            std::copy_backward(b, e, de);
        }
        return *this;
    }

    size_t length(char value, size_t from = MAX_INDEX) const {
        size_t last = adjust_index(from);
        auto rbeg = std::make_reverse_iterator(m_array.begin() + last + 1);
        auto rend = m_array.rend();

        auto it = std::find_if(rbeg, rend, [value](char c) { return c != value; });

        return it.base() - m_array.begin();
    }

private:
    std::array<char, MAX_STRLEN> m_array;
};

#endif // !defined(STR_OBJECT_H)
