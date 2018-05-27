#ifndef NEWWORD_H
#define NEWWORD_H

#include "type.h"

bool newword_advance_word(leadparam rept, int count, bool from_span);
bool newword_delete_word(leadparam rept, int count, bool from_span);
bool newword_advance_paragraph(leadparam rept, int count, bool from_span);
bool newword_delete_paragraph(leadparam rept, int count, bool from_span);

#endif // !defined(NEWWORD_H)
