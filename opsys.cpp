/**********************************************************************}
{                                                                      }
{            L      U   U   DDDD   W      W  IIIII   GGGG              }
{            L      U   U   D   D   W    W     I    G                  }
{            L      U   U   D   D   W ww W     I    G   GG             }
{            L      U   U   D   D    W  W      I    G    G             }
{            LLLLL   UUU    DDDD     W  W    IIIII   GGGG              }
{                                                                      }
{**********************************************************************}
{                                                                      }
{   Copyright (C) 1981, 1987                                           }
{   Department of Computer Science, University of Adelaide, Australia  }
{   All rights reserved.                                               }
{   Reproduction of the work or any substantial part thereof in any    }
{   material form whatsoever is prohibited.                            }
{                                                                      }
{**********************************************************************/

/**
! Name:         OPSYS
!
! Description:  This routine executes a command in a subprocess and
!               transfers the result into the current frame.
*/

#include "opsys.h"

#include "ch.h"
#include "line.h"
#include "filesys.h"

bool opsys_command(const tpar_object &command, line_ptr &first, line_ptr &last, int &actual_cnt) {
    first = nullptr;
    last = nullptr;
    actual_cnt = 0;
    file_object mbx;
    //with mbx^ do
    mbx.valid       = false;
    mbx.first_line  = nullptr;
    mbx.last_line   = nullptr;
    mbx.line_count  = 0;
    mbx.output_flag = false;
    if (command.len <= FILE_NAME_LEN) {
        mbx.fnm.copy(command.str, 1, FILE_NAME_LEN);
    } else {
        mbx.fnm.fill(0);
        mbx.fns = 0;
    }
    mbx.zed         = 'Z';

    if (!filesys_create_open(&mbx, nullptr, false))
        return false;
    bool opsys_result = false;
    while (!mbx.eof) {
        str_object result;
        strlen_range outlen;
        if (filesys_read(&mbx, result, outlen)) {
            line_ptr line;
            line_ptr line_2;
            if (!lines_create(1, line, line_2))
                goto l98;
            if (!line_change_length(line, outlen)) {
                lines_destroy(line, line_2);
                goto l98;
            }
            ch_fillcopy(&result, 1, outlen, line->str, 1, line->len, ' ');
            line->used  = outlen;
            line->blink = last;
            if (last != nullptr)
                last->flink = line;
            else
                first = line;
            last = line;
            actual_cnt += 1;
        } else if (!mbx.eof) { // Something terrible has happened!
            goto l98;
        }
    }
    opsys_result = true;
l98:;
    filesys_close(&mbx, 0, false);
    return opsys_result;
}
