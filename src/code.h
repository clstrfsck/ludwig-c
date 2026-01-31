#ifndef CODE_H
#define CODE_H

#include "type.h"

void code_discard(code_ptr &code_head);

bool code_compile(span_object &span, bool from_span);
bool code_interpret(leadparam rept, int count, code_ptr code_head, bool from_span);

// Visible for testing
using execute_fn = bool (*)(commands, leadparam, int, tpar_ptr, bool);
bool code_interpret_execute(
    execute_fn execute,
    leadparam rept,
    int count,
    code_ptr code_head,
    bool from_span
);

#endif // !defined(CODE_H)
