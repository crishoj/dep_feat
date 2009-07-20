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
#ifndef LETTERFUNC_H
#define LETTERFUNC_H

#define UNICODE_CAPABLE 1

#if UNICODE_CAPABLE

class folded
    {
    private:
        unsigned int * w;
        int i;
        bool inFull;
    protected:
        virtual int c() = 0;
    public:
        folded():w(0),i(-1),inFull(false){}
        virtual ~folded(){}
        int C(); // full case folding of w
    };


bool isAlpha(int kar);
bool isUpper(int kar);
bool isLower(int kar);
unsigned int lowerEquivalent(int kar);
unsigned int upperEquivalent(int kar);
//bool isAllUpper(const char * s,int len);
int strCaseCmp(const wchar_t *s, const char *p);
//int strCaseCmp(const char *s, const char *p);
//int UTF8char(const char * s,bool & UTF8); 
    // Sets UTF8 to false if s isn't UTF8
    // returns character (starting at) s
//int getUTF8char(const char *& s,bool & UTF8); 
    // Same as UTF8char, but also increments s!
//const char * Inc(const char *& s);
#endif

#endif
