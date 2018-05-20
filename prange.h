/** @file prange.h
 * Declarations for class to support Pascal-style ranges.
 */

#ifndef PRANGE_H
#define PRANGE_H

#include <stdexcept>

template <int min_, int max_>
class prange {
    static_assert(min_ < max_, "min must be strictly less than max");

    constexpr int check_range(int value) {
        if (value < min() || value > max())
            throw std::out_of_range("out of range");
        return value;
    }

public:
    static constexpr int min() {
        return min_;
    }

    static constexpr int max() {
        return max_;
    }

    static constexpr size_t size() {
        return max() - min() + 1;
    }

    prange() {
        m_value = min();
    }

    prange(int value) {
        m_value = check_range(value);
    }

    prange(const prange &other) {
        m_value = other.m_value;
    }

    operator int() const {
        return m_value;
    }

    prange &operator=(const prange &rhs) {
        m_value = rhs.m_value;
    }

    prange &operator=(int rhs) {
        m_value = check_range(rhs);
    }

private:
    int m_value;
};

#endif // !defined(PRANGE_H)
