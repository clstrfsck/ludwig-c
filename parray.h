/** @file parray.h
 * Declarations for class to support Pascal-style ranged arrays.
 */

#ifndef PARRAY_H
#define PARRAY_H

#include "prange.h"

#include <vector>
#include <type_traits>

// FIXME: Could use some range checking, possibly optional.

template <class T, class R>
class parray : public R {
    static_assert(R::min() < R::max(), "min must be strictly less than max");

    constexpr size_t adjust_index(int index) {
        return index - R::min();
    }

public:
    explicit parray(const T &init_value = T())
        : m_array(R::size(), init_value) {
        // Nothing more to do here
    }

    parray(const parray &other) : m_array(other.m_array) {
        // Nothing more to do here
    }

    ~parray() {
        // Nothing to do here
    }

    T &operator[] (int index) {
        return m_array[adjust_index(index)];
    }

    const T &operator[] (int index) const {
        return m_array[adjust_index(index)];
    }

    parray& operator=(parray rhs) {
        std::swap(m_array, rhs.m_array);
    }

private:
    std::vector<T> m_array;
};

#endif // !defined(PARRAY_H)
