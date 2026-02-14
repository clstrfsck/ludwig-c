#ifndef CH_H
#define CH_H

#include "type.h"

int ch_compare_str(
    const str_object &target,
    strlen_range st1,
    strlen_range len1,
    const str_object &text,
    strlen_range st2,
    strlen_range len2,
    bool exactcase,
    strlen_range &nch_ident
);
void ch_reverse_str(const str_object &src, str_object &dst, strlen_range len);
char ch_toupper(char ch);
[[nodiscard]] bool ch_search_str(
    const str_object &target,
    strlen_range st1,
    strlen_range len1,
    const str_object &text,
    strlen_range st2,
    strlen_range len2,
    bool exactcase,
    bool backwards,
    strlen_range &found_loc
);

void ch_fillcopy(
    str_ptr src,
    strlen_range srcofs,
    size_t srclen,
    str_ptr dst,
    strlen_range dstofs,
    size_t dstlen,
    char fill
);

#endif // !defined(CH_H)
