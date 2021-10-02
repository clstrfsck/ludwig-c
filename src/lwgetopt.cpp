/*-
 * Adapted from FreeBSD getopt.h which is under the following license.
 *
 * Copyright (c) 1987, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "lwgetopt.h"

#include <stdio.h>
#include <string.h>

size_t      lwoptind = 1;     // index into parent argv vector
int         lwoptopt;         // character checked for validity
bool        lwoptreset;       // reset getopt
std::string lwoptarg;         // argument associated with option

#define BADCH   (int)'?'
#define BADARG  (int)':'
#define EMSG    ""

/*
 * lwgetopt --
 *  Parse argv argument vector.
 */
int lwgetopt(const std::vector<std::string> &nargv, const std::string &ostr) {
    static std::string place = EMSG;    // option letter processing

    if (lwoptreset || place.empty()) {
        // update scanning pointer
        lwoptreset = false;

        if (lwoptind >= nargv.size()) {
            // Argument is absent
            place = EMSG;
            return (-1);
        }
        place = nargv[lwoptind];
        if (place[0] != '-') {
            // Argument is not an option
            place = EMSG;
            return (-1);
        }
        place.erase(0, 1);
        if (place.empty()) {
            // Solitary '-', treat as a '-' option
            // if the program (eg su) is looking for it.
            place = EMSG;
            if (ostr.find('-') == std::string::npos)
                return (-1);
            lwoptopt = '-';
        }
        lwoptopt = place[0];
        place.erase(0, 1);
        if (lwoptopt == '-' && place.empty()) {
            // "--" => end of options
            ++lwoptind;
            place = EMSG;
            return (-1);
        }
    } else {
        lwoptopt = place[0];
        place.erase(0, 1);
    }

    // See if option letter is one the caller wanted...
    std::string::size_type oli = ostr.find(lwoptopt);
    if (lwoptopt == ':' || oli == std::string::npos) {
        if (place.empty())
            ++lwoptind;
        return (BADCH);
    }

    /* Does this option need an argument? */
    if (oli + 1 >= ostr.size() || ostr[oli + 1] != ':') {
        // don't need argument
        lwoptarg = "";
        if (place.empty()) {
            ++lwoptind;
        }
    } else {
        // Option-argument is either the rest of this
        // argument or the entire next argument.
        if (!place.empty()) {
            lwoptarg = place;
        } else if (nargv.size() > ++lwoptind) {
            lwoptarg = nargv[lwoptind];
        } else {
            // option-argument absent
            place = EMSG;
            if (ostr[0] == ':')
                return (BADARG);
            return (BADCH);
        }
        place = EMSG;
        ++lwoptind;
    }
    return (lwoptopt); // return option letter
}
