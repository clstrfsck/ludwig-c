#ifndef EQSGETREP_H
#define EQSGETREP_H

#include "type.h"

[[nodiscard]] bool eqsgetrep_eqs(leadparam rept, tpar_object tpar);
[[nodiscard]] bool eqsgetrep_get(int count, tpar_object tpar, bool from_span);
[[nodiscard]] bool eqsgetrep_rep(leadparam rept, int count, tpar_object tpar, tpar_object tpar2, bool from_span);

#endif // !defined(EQSGETREP_H)
