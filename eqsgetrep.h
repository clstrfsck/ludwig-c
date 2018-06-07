#ifndef EQSGETREP_H
#define EQSGETREP_H

#include "type.h"

bool eqsgetrep_eqs(leadparam rept, int count, tpar_object tpar, bool from_span);
bool eqsgetrep_get(leadparam rept, int count, tpar_object tpar, bool from_span);
bool eqsgetrep_rep(leadparam rept, int count, tpar_object tpar, tpar_object tpar2, bool from_span);

#endif // !defined(EQSGETREP_H)
