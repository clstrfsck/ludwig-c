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
!
! $Log: opsys.pas,v $
! Revision 4.8  2002/07/21 02:18:47  martin
! The fpc port uses the same command line setup code as unix. MPS
!
! Revision 4.7  1990/10/24 14:56:42  ludwig
! Fix call to filesys_close.   KBN
!
! Revision 4.6  90/01/18  17:43:02  ludwig
! Entered into RCS at revision level 4.6
!
! Revision History:
! 4-001 Mark R. Prior                                           Apr-1987
!       Original code
! 4-002 Mark R. Prior                                        20-Feb-1988
!       Strings passed to ch routines are now passed using conformant
!         arrays, or as type str_object.
!               string[offset],length -> string,offset,length
!       Where conformant arrays are not implemented and the array is not
!         of type str_object, separate routines are provided for each
!         type.
! 4-003 Jeff Blows                                              Jul-1989
!       IBM PC developments incorporated into main source code.
! 4-004 Kelvin B. Nicolle                                    12-Jul-1989
!       VMS include files renamed from ".ext" to ".h", and from ".inc"
!       to ".i".  Remove the "/nolist" qualifiers.
! 4-005 Kelvin B. Nicolle                                    13-Sep-1989
!       Add includes etc. for Tower version.
! 4-006 Kelvin B. Nicolle                                    25-Oct-1989
!       Correct the includes for the Tower version.
!**/

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
        mbx.filename = std::string(command.str.data(), command.len);
    } else {
        mbx.filename.clear();
    }
    mbx.zed = 'Z';

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
