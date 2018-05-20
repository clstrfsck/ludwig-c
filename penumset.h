/** @file penumset.h
 * Declarations for class to support enumerated sets.
 */

#ifndef PENUMSET_H
#define PENUMSET_H

#include <bitset>

// FIXME: Required enum to be consecutive and have a value "last_entry".

template <typename T>
class penumset {

public:
    penumset add(T elt) {
        m_value.set(elt);
    }

    penumset remove(T elt) {
        m_value.remove(elt);
    }

private:
    std::bitset<static_cast<int>(T::last_entry)> m_value;
};

#endif // !defined(PENUMSET_H)
