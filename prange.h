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
    typedef int type;

    static constexpr type min() {
        return min_;
    }

    static constexpr type max() {
        return max_;
    }

    static constexpr size_t size() {
        return max_ - min_ + 1;
    }

    static constexpr size_t zero_based(type n) {
        return n - min_;
    }

    prange() {
        m_value = min_;
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
        return *this;
    }

    prange &operator=(int rhs) {
        m_value = check_range(rhs);
        return *this;
    }

    prange &operator+=(int rhs) {
        m_value = check_range(m_value + rhs);
        return *this;
    }

    prange &operator++() {
        m_value = check_range(m_value + 1);
        return *this;
    }

    prange operator++(int) {
        prange old(*this);
        m_value = check_range(m_value + 1);
        return old;
    }

    prange &operator-=(int rhs) {
        m_value = check_range(m_value - rhs);
        return *this;
    }

    prange &operator--() {
        m_value = check_range(m_value - 1);
        return *this;
    }

    prange operator--(int) {
        prange old(*this);
        m_value = check_range(m_value - 1);
        return old;
    }

private:
    int m_value;
};

#endif // !defined(PRANGE_H)
