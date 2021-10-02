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

    penumset(T elt) {
        add(elt);
    }

    penumset(std::initializer_list<T> elts) {
        add(elts);
    }

    bool operator==(const penumset &other) const {
        return m_value == other.m_value;
    }

    penumset &add(T elt) {
        m_value.set(bit(elt));
        return *this;
    }

    penumset &add(const penumset &other) {
        m_value |= other.m_value;
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

    penumset &set(T elt) {
        clear();
        return add(elt);
    }

    penumset &set(std::initializer_list<T> elts) {
        clear();
        return add(elts);
    }

    penumset &set_range(T begin, T endi) {
        clear();
        return add_range(begin, endi);
    }

    penumset &remove(T elt) {
        m_value.reset(bit(elt));
        return *this;
    }

    penumset &remove(const penumset &other) {
        m_value &= ~other.m_value;
        return *this;
    }

    penumset &remove(std::initializer_list<T> elts) {
        for (T elt : elts) {
            remove(elt);
        }
        return *this;
    }

    penumset &remove_range(T begin, T endi) {
        while (begin != endi) {
            remove(begin);
            begin = next_enum(begin);
        }
        remove(begin);
        return *this;
    }

    penumset &clear() {
        m_value.reset();
        return *this;
    }

    bool contains(T elt) const {
        return m_value.test(bit(elt));
    }

    penumset set_union(const penumset &other) const {
        penumset copy(*this);
        copy.m_value |= other.m_value;
        return copy;
    }

    penumset set_difference(const penumset &other) const {
        penumset copy(*this);
        copy.m_value &= ~other.m_value;
        return copy;
    }

    penumset set_intersection(const penumset &other) const {
        penumset copy(*this);
        copy.m_value &= other.m_value;
        return copy;
    }

    bool empty() const {
        return m_value.none();
    }

private:
    std::bitset<range()> m_value;
};

#endif // !defined(PENUMSET_H)
