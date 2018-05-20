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
    penumset<T> add(T elt) {
        m_value.set(adjust_value(elt));
    }

    penumset<T> remove(T elt) {
        m_value.remove(adjust_value(elt));
    }

private:
    std::bitset<T::size()> m_value;

    constexpr int adjust_value(T elt) {
        return elt - T::min();
    }
};

#endif // !defined(PRANGESET_H)
