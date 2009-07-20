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
#ifndef UTF8_H
#define UTF8_H

#include "letterfunc.h"
#include <stddef.h>
#if UNICODE_CAPABLE
bool isAllUpper(const char * s,int len);
bool isUpperUTF8(const char * s);
int UnicodeToUtf8(int w,char * s,int len);
int Utf8ToUnicode(int * w,const char * s,size_t len);
void AllToUpper(char * s);
const char * allToLowerUTF8(const char * s);
// Simple case folding, used for simple "case insensitive" comparison.
// Allocates enough memory to hold the returned value.
// This memory is deallocated the next time allToLowerUTF8 is called, so
// do not delete!
void AllToLowerUTF8(char * s);
// Simple case folding, used for simple "case insensitive" comparison.
// Overwrites s!
int strCaseCmp(const char *s, const char *p);
int UTF8char(const char * s,bool & UTF8); 
    // Sets UTF8 to false if s isn't UTF8
    // returns character (starting at) s
int getUTF8char(const char *& s,bool & UTF8); 
    // Same as UTF8char, but also post-increments s!
const char * Inc(const char *& s);
int copyUTF8char(const char * source,char * dest);
    // Copies one character from source to destination.
    // Returns number of bytes copied.
    // Makes no check of validity of UTF8!
int skipUTF8char(const char * s);
    // Makes no check of validity of UTF8!
extern bool UTF8;
#endif

#endif
