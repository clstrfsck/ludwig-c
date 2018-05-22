/** @file penumset.h
 * Declarations for class to support enumerated sets.
 */

#ifndef PENUMSET_H
#define PENUMSET_H

#include <bitset>

// FIXME: Required enum to be consecutive and have a value "last_entry".

template <typename T>
class penumset {

    // FIXME: This is pretty horrible
    static constexpr T next_enum(T val) {
        return static_cast<T>(static_cast<int>(val) + 1);
    }

    static constexpr size_t range() {
        return static_cast<size_t>(T::last_entry);
    }

    static constexpr size_t bit(T val) {
        return static_cast<size_t>(val);
    }

public:
    penumset() {
        // Nothing to do
    }

    penumset(std::initializer_list<T> elts) {
        add(elts);
    }

    penumset &add(T elt) {
        m_value.set(bit(elt));
        return *this;
    }

    penumset &add(std::initializer_list<T> elts) {
        for (T elt : elts) {
            add(elt);
        }
        return *this;
    }

    penumset &add_range(T begin, T endi) {
        while (begin != endi) {
            add(begin);
            begin = next_enum(begin);
        }
        add(begin);
        return *this;
    }

    penumset &remove(T elt) {
        m_value.remove(elt);
        return *this;
    }

    penumset &clear() {
        m_value.reset();
        return *this;
    }

private:
    std::bitset<range()> m_value;
};

#endif // !defined(PENUMSET_H)
