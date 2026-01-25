/** @file perange.h
 * Declarations for class to support Pascal-style enum ranges.
 */

#ifndef PERANGE_H
#define PERANGE_H

// FIXME: Could use some range checking, possibly optional.

template <typename E, E min_ = static_cast<E>(0), E max_ = static_cast<E>(static_cast<int>(E::last_entry) - 1)>
class perange {
    static_assert(min_ < max_, "min must be strictly less than max");

    static constexpr E check_range(E value) {
        if (value < min_ || value > max_)
            throw std::out_of_range("out of range");
        return value;
    }

public:
    using type = E;

    static constexpr type min() {
        return min_;
    }

    static constexpr type max() {
        return max_;
    }

    static constexpr size_t size() {
        return static_cast<int>(max_) - static_cast<int>(min_) + 1;
    }

    static constexpr size_t zero_based(type n) {
        return static_cast<int>(n) - static_cast<int>(min_);
    }

    perange() : m_value{ min_ } {
        // Nothing
    }

    perange(E value) : m_value{ check_range(value) } {
        // Nothing
    }

    perange(const perange &other) : m_value{ other.m_value } {
        // Nothing
    }

    perange &operator=(const perange &rhs) {
        m_value = rhs.m_value;
        return *this;
    }

    E value() const {
        return m_value;
    }

private:
    E m_value;
};

#endif // !defined(PERANGE_H)
