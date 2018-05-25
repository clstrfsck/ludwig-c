/** @file parray.h
 * Declarations for class to support Pascal-style ranged arrays.
 */

#ifndef PARRAY_H
#define PARRAY_H

#include "prange.h"

#include <vector>
#include <algorithm>
#include <functional>
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
        if (ibeg < iend) {
            typename array_type::iterator b = std::next(m_array.begin(), ibeg);
            typename array_type::iterator e = std::next(m_array.begin(), iend);
            std::fill(b, e, value);
        }
        return *this;
    }

    parray &fill_n(const T &value, size_t n, typename R::type begin = R::min()) {
        size_t ibeg = adjust_index(begin);
        typename array_type::iterator b = std::next(m_array.begin(), ibeg);
        typename array_type::iterator e = std::next(b, n);
        std::fill(b, e, value);
        return *this;
    }

    parray &apply(std::function<T(T)> f, typename R::type beg = R::min(), typename R::type endi = R::max()) {
        size_t ibeg = adjust_index(beg);
        size_t iend = adjust_index(endi) + 1;
        if (ibeg < iend) {
            typename array_type::iterator b = std::next(m_array.begin(), ibeg);
            typename array_type::iterator e = std::next(m_array.begin(), iend);
            std::transform(b, e, b, f);
        }
        return *this;
    }

    parray &apply_n(std::function<T(T)> f, size_t n, typename R::type beg = R::min()) {
        if (n > 0) {
            size_t ibeg = adjust_index(beg);
            typename array_type::iterator b = std::next(m_array.begin(), ibeg);
            typename array_type::iterator e = std::next(b, n);
            std::transform(b, e, b, f);
        }
        return *this;
    }

    parray &fillcopy(typename array_type::const_pointer src, size_t src_len,
                     typename R::type dst_index, size_t dst_len, const T &value) {
        size_t len = std::min(src_len, dst_len);
        size_t dst = adjust_index(dst_index);
        if (len != 0) {
            std::copy(src, src + len, std::next(m_array.begin(), dst));
        }
        if (dst_len > len) {
            typename array_type::iterator db = std::next(m_array.begin(), dst + len);
            typename array_type::iterator de = std::next(db, dst_len - len);
            std::fill(db, de, value);
        }
        return *this;
    }

    typename array_type::const_pointer data(typename R::type index = R::min()) const {
        return m_array.data() + adjust_index(index);
    }

    size_t length(const T &value, typename R::type from = R::max()) const {
        if (!m_array.empty()) {
            size_t last = adjust_index(from);
            while (last) {
                if (m_array[last] != value)
                    return last + 1;
                last -= 1;
            }
            return m_array[0] == value ? 0 : 1;
        }
        return 0;
    }

    parray &operator=(parray rhs) {
        std::swap(m_array, rhs.m_array);
        return *this;
    }

private:
    array_type m_array;
};

#endif // !defined(PARRAY_H)
