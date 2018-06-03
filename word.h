#ifndef WORD_H
#define WORD_H

#include "type.h"

bool word_fill(leadparam rept, int count, bool from_span);
bool word_centre(leadparam rept, int count, bool from_span);
bool word_justify(leadparam rept, int count, bool from_span);
bool word_squeeze(leadparam rept, int count, bool from_span);
bool word_right(leadparam rept, int count, bool from_span);
bool word_left(leadparam rept, int count, bool from_span);
bool word_advance_word(leadparam rept, int count, bool from_span);
bool word_delete_word(leadparam rept, int count, bool from_span);

#endif // !defined(WORD_H)
