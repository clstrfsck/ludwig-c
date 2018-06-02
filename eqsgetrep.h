#ifndef EQSGETREP_H
#define EQSGETREP_H

#include "type.h"

bool eqsgetrep_eqs(leadparam rept, int count, const tpar_object &tpar, bool from_span);
bool eqsgetrep_get(leadparam rept, int count, const tpar_object &tpar, bool from_span);
bool eqsgetrep_rep(leadparam rept, int count, const tpar_object &tpar, const tpar_object &tpar2, bool from_span);

#endif // !defined(EQSGETREP_H)
