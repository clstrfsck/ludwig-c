/** @file parray.h
 * Declarations for class to support Pascal-style ranged arrays.
 */

#ifndef PARRAY_H
#define PARRAY_H

#include "prange.h"

#include <vector>
#include <algorithm>
#include <type_traits>

// FIXME: Could use some range checking, possibly optional.

template <class T, class R>
class parray : public R {
    static_assert(R::min() < R::max(), "min must be strictly less than max");

    constexpr size_t adjust_index(typename R::type index) const {
        return R::zero_based(index);
    }

    typedef std::vector<T> array_type;

public:
    explicit parray(const T &init_value = T())
        : m_array(R::size(), init_value) {
        // Nothing more to do here
    }

    explicit parray(const T *values)
        : m_array(values, values + R::size()) {
        // Nothing more to do here
    }

    explicit parray(size_t size, const T &init_value = T())
        : m_array(size, init_value) {
        // Nothing more to do here.  Be careful with this!
    }

    parray(const parray &other) : m_array(other.m_array) {
        // Nothing more to do here
    }

    ~parray() {
        // Nothing to do here
    }

    typename array_type::reference operator[] (typename R::type index) {
        return m_array[adjust_index(index)];
    }

    typename array_type::const_reference operator[] (typename R::type index) const {
        return m_array[adjust_index(index)];
    }

    parray &copy(typename array_type::const_pointer src, size_t count, typename R::type dst_offset = R::min()) {
        size_t offset_index = adjust_index(dst_offset);
        typename array_type::iterator i = std::next(m_array.begin(), offset_index);
        std::copy(src, src + count, i);
        return *this;
    }

    parray &fill(const T &value, typename R::type begin = R::min(), typename R::type endi = R::max()) {
        size_t ibeg = adjust_index(begin);
        size_t iend = adjust_index(endi) + 1;
        typename array_type::iterator b = std::next(m_array.begin(), ibeg);
        typename array_type::iterator e = std::next(m_array.begin(), iend);
        std::fill(b, e, value);
        return *this;
    }

    typename array_type::const_pointer data() const {
        return m_array.data();
    }

    parray &operator=(parray rhs) {
        std::swap(m_array, rhs.m_array);
        return *this;
    }

private:
    array_type m_array;
};

#endif // !defined(PARRAY_H)
