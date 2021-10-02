#ifndef NEWWORD_H
#define NEWWORD_H

#include "type.h"

bool newword_advance_word(leadparam rept, int count);
bool newword_delete_word(leadparam rept, int count);
bool newword_advance_paragraph(leadparam rept, int count);
bool newword_delete_paragraph(leadparam rept, int count);

#endif // !defined(NEWWORD_H)
