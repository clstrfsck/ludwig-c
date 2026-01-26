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
{   Copyright (C) 2002                                                 }
{   Martin Sandiford, Adelaide, Australia                              }
{   All rights reserved.                                               }
{                                                                      }
{**********************************************************************/

/**
! Name:         VERSION
!
! Description:  Current version number.
!**/

#ifndef VERSION_H
#define VERSION_H

#include <string_view>

inline constexpr std::string_view LUDWIG_VERSION{
#ifdef DEBUG
    "X5.0-006                       "
#else
    "V5.0-006                       "
#endif
};

#endif // !define(VERSION_H)
