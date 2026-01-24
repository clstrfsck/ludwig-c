#ifndef TPAR_H
#define TPAR_H

#include "type.h"

void tpar_clean_object(tpar_object &tp_o);
void tpar_duplicate(tpar_ptr from_tp, tpar_ptr &to_tp);
bool tpar_to_mark(const tpar_object &strng, int &mark);
bool tpar_to_int(const tpar_object &strng, int &chpos, int &int_);
bool tpar_analyse(user_commands cmd, tpar_object &tran, int depth, tpcount_type this_tp);
bool tpar_get_1(tpar_ptr tpar, user_commands cmd, tpar_object &tran);
bool tpar_get_2(tpar_ptr tpar, user_commands cmd, tpar_object &trn1, tpar_object &trn2);

#endif // !define(TPAR_H)
