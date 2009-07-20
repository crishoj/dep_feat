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
#ifndef CASECONV_H
#define CASECONV_H


#define DEFAULTENCODING 0
#define ISO8859_1 1 //Western European
#define ISO8859_2 2 //Eastern European
#define ISO8859_7 7 //Greek
#define ISO8859_9 9 //Turkish
#define ENUNICODE 'U' //Unicode


                // If you don't want case conversion and
                // assumptions about what are characters in the 
                // range >127, then set to DEFAULTENCODING.
void setEncoding(int encoding);
const char * allToLowerISO(const char * s);
void AllToLowerISO(char * s);
void AllToUpperISO(char * s);
void toLower(char * s);
void toUpper(char * s);
bool isAllUpper(const char * s);
extern const bool * space;
extern const bool * alpha;
#define isSpace(s) space[(int)(s) & 0xFF]
bool isAlphaISO(int s); 

#if 0
#if ENCODING == DEFAULTENCODING

#define isUpper(s) 1
#define Upper(k) k
#define Lower(k) k

#else

extern const unsigned char * upperEquivalent;
extern const unsigned char * lowerEquivalent;

#define isUpper(s) (upperEquivalent[(int)(*s & 0xFF)] == (int)(*s & 0xFF))
#define Upper(k) upperEquivalent[(int)(k & 0xFF)]
#define Lower(k) lowerEquivalent[(int)(k & 0xFF)]

#endif
#endif

extern bool (*is_Upper)(const char * s);
extern unsigned int (*Upper)(int k);
extern unsigned int (*Lower)(int k);
extern bool (*is_Alpha)(int s);
extern const char * (*allToLower)(const char * s);
extern void (*AllToLower)(char * s);
#endif
