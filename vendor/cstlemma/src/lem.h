/*
CSTLEMMA - trainable lemmatiser using word-end inflectional rules

Copyright (C) 2002, 2005  Center for Sprogteknologi, University of Copenhagen

This file is part of CSTLEMMA.

CSTLEMMA is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

CSTLEMMA is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CSTLEMMA; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef LEM_H
#define LEM_H
// types that ar used for serializing (writing to file) the dictionary

#include <stddef.h>
#include "defines.h"

typedef unsigned char tchildren;
//typedef unsigned int tchildren;
typedef ptrdiff_t tindex;
typedef size_t tlength;
typedef long tcount;
//typedef short int tfrequency; // One could consider a smaller type and a logarithmic scale
//typedef short int toffset;
typedef struct
    {
    unsigned int Offset:8;       // String length max 255
    unsigned int frequency:24;   // Max frequency 16.777.215
    } tsundry;
//typedef char tcount; // tcount must be signed
#endif
