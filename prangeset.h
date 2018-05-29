/** @file prangeset.h
 * Declarations for class to support ranged sets.
 */

#ifndef PRANGESET_H
#define PRANGESET_H

#include <bitset>

template <typename T>
class prangeset {
    static_assert(T::min() < T::max(), "min must be strictly less than max");
public:
    typedef T element_type;

    prangeset<T> &add(T elt) {
        m_value.set(adjust_value(elt));
        return *this;
    }

    prangeset<T> &add(const prangeset<T> &other) {
        m_value |= other.m_value;
        return *this;
    }

    prangeset<T> &add(std::initializer_list<T> elts) {
        for (T elt : elts) {
            add(elt);
        }
        return *this;
    }

    prangeset<T> &add_range(T begin, T endi) {
        while (begin != endi) {
            add(begin);
            ++begin;
        }
        add(begin);
        return *this;
    }

    prangeset<T> &set(std::initializer_list<T> elts) {
        clear();
        return add(elts);
    }

    prangeset<T> &set_range(T begin, T endi) {
        clear();
        return add_range(begin, endi);
    }

    prangeset<T> &remove(T elt) {
        m_value.remove(adjust_value(elt));
        return *this;
    }

    prangeset<T> &remove(const prangeset<T> &other) {
        m_value &= ~other.m_value;
        return *this;
    }

    prangeset<T> &remove(std::initializer_list<T> elts) {
        for (T elt : elts) {
            remove(elt);
        }
        return *this;
    }

    prangeset<T> &remove_range(T begin, T endi) {
        while (begin != endi) {
            remove(begin);
            ++begin;
        }
        remove(begin);
        return *this;
    }

    prangeset<T> &clear() {
        m_value.reset();
        return *this;
    }

    bool contains(T elt) const {
        return m_value.test(adjust_value(elt));
    }

private:
    std::bitset<T::size()> m_value;

    constexpr int adjust_value(T elt) const {
        return elt - T::min();
    }
};

#endif // !defined(PRANGESET_H)
