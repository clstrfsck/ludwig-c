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
class parray {
    static_assert(R::min() < R::max(), "min must be strictly less than max");

    constexpr size_t adjust_index(typename R::type index) const {
        return R::zero_based(index);
    }

    inline void check_index(typename R::type index) const {
        if (index < R::min() || index > R::max()) {
            throw std::out_of_range("index out of range");
        }
    }

    typedef std::vector<T> array_type;

public:
    typedef R                                    index_type;
    typedef typename array_type::value_type      value_type;
    typedef typename array_type::reference       reference;
    typedef typename array_type::const_reference const_reference;
    typedef typename array_type::pointer         pointer;
    typedef typename array_type::const_pointer   const_pointer;

    explicit parray(const_reference init_value = value_type())
        : m_array(index_type::size(), init_value) {
        // Nothing more to do here
    }

    explicit parray(const_pointer values)
        : m_array(values, values + R::size()) {
        // Nothing more to do here
    }

    explicit parray(size_t size, const_reference init_value = T())
        : m_array(size, init_value) {
        // Nothing more to do here.  Be careful with this!
    }

    parray(const parray &other) : m_array(other.m_array) {
        // Nothing more to do here
    }

    ~parray() {
        // Nothing to do here
    }

    reference operator[] (typename index_type::type index) {
        check_index(index);
        return m_array[adjust_index(index)];
    }

    const_reference operator[] (typename index_type::type index) const {
        check_index(index);
        return m_array[adjust_index(index)];
    }

    parray &copy(const_pointer src, size_t count, typename index_type::type dst_offset = index_type::min()) {
        if (count > 0) {
            check_index(dst_offset); // First index used
            check_index(dst_offset + count - 1); // Last index used
            size_t offset_index = adjust_index(dst_offset);
            typename array_type::iterator i = std::next(m_array.begin(), offset_index);
            std::copy(src, src + count, i);
        }
        return *this;
    }

    parray &fill(const T &value, typename index_type::type begin = index_type::min(),
                 typename index_type::type endi = index_type::max()) {
        check_index(begin);
        check_index(endi);
        size_t ibeg = adjust_index(begin);
        size_t iend = adjust_index(endi) + 1;
        if (ibeg < iend) {
            typename array_type::iterator b = std::next(m_array.begin(), ibeg);
            typename array_type::iterator e = std::next(m_array.begin(), iend);
            std::fill(b, e, value);
        }
        return *this;
    }

    parray &fill_n(const T &value, size_t n, typename index_type::type begin = index_type::min()) {
        if (n > 0) {
            check_index(begin);
            check_index(begin + n - 1);
            size_t ibeg = adjust_index(begin);
            typename array_type::iterator b = std::next(m_array.begin(), ibeg);
            typename array_type::iterator e = std::next(b, n);
            std::fill(b, e, value);
        }
        return *this;
    }

    parray &apply(std::function<T(T)> f, typename index_type::type beg = index_type::min(),
                  typename index_type::type endi = index_type::max()) {
        check_index(beg);
        check_index(endi);
        size_t ibeg = adjust_index(beg);
        size_t iend = adjust_index(endi) + 1;
        if (ibeg < iend) {
            typename array_type::iterator b = std::next(m_array.begin(), ibeg);
            typename array_type::iterator e = std::next(m_array.begin(), iend);
            std::transform(b, e, b, f);
        }
        return *this;
    }

    parray &apply_n(std::function<T(T)> f, size_t n, typename index_type::type beg = index_type::min()) {
        if (n > 0) {
            check_index(beg);
            check_index(beg + n - 1);
            size_t ibeg = adjust_index(beg);
            typename array_type::iterator b = std::next(m_array.begin(), ibeg);
            typename array_type::iterator e = std::next(b, n);
            std::transform(b, e, b, f);
        }
        return *this;
    }

    parray &fillcopy(const_pointer src, size_t src_len,
                     typename index_type::type dst_index, size_t dst_len, const_reference value) {
        if (dst_len > 0) {
            check_index(dst_index);
            check_index(dst_index + dst_len - 1);
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
        }
        return *this;
    }

    const_pointer data(typename index_type::type index = index_type::min()) const {
        check_index(index);
        return m_array.data() + adjust_index(index);
    }

    pointer data(typename index_type::type index = index_type::min()) {
        check_index(index);
        return m_array.data() + adjust_index(index);
    }

    size_t length(const_reference value, typename index_type::type from = index_type::max()) const {
        if (!m_array.empty()) {
            check_index(from);
            int last = adjust_index(from);
            while (last >= 0) {
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

    bool operator==(const parray &rhs) const {
        return m_array == rhs.m_array;
    }

    bool operator!=(const parray &rhs) const {
        return !operator==(rhs);
    }

    bool operator<(const parray &rhs) const {
        return std::lexicographical_compare(m_array.begin(), m_array.end(), rhs.m_array.begin(), rhs.m_array.end());
    }

private:
    array_type m_array;
};

#endif // !defined(PARRAY_H)
