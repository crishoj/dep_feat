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
#include "letterfunc.h"
#include <string.h>
#if UNICODE_CAPABLE
#include "letter.h"


bool isAlpha(int kar)
    {
    int ind = kar % ARRSIZE;
    if((int)Letter[ind].Unfolded == kar)
        return true;
    /*
    TODO:
    Code for alphabets without upper/lower case distinction
    */
    return kar > 255; // Assume Unicode positions outside Latin-1 is all alphabetic
    }

bool isUpper(int kar)
    {
    int ind = kar % ARRSIZE;
    return ((int)Letter[ind].Unfolded == kar) && Letter[ind].Unfolded == Letter[ind].Capital;
    }

bool isLower(int kar)
    {
    int ind = kar % ARRSIZE;
    return ((int)Letter[ind].Unfolded == kar) && Letter[ind].Unfolded == Letter[ind].Simple;// && Letter[ind].Capital != 0;
    }

unsigned int lowerEquivalent(int kar)
    {
    int ind = kar % ARRSIZE;
    return ((int)Letter[ind].Unfolded == kar) && (Letter[ind].Simple) ? Letter[ind].Simple : kar;
    }

unsigned int upperEquivalent(int kar)
    {
    int ind = kar % ARRSIZE;
    return ((int)Letter[ind].Unfolded == kar) && (Letter[ind].Capital) ? Letter[ind].Capital : kar;
    }

int folded::C()
    {
    if(inFull)
        {
        if(i > 2 || w[i] == 0)
            inFull = false;
        else
            return w[i++];
        }
    int kar = c();
    int ind = kar % ARRSIZE;
    if((int)Letter[ind].Unfolded == kar)
        {
        tri * Tri = Letter[ind].Full;
        if(  Tri 
          && (w = Tri->w)[1] /* If the second element is zero, we have
                               a Turkic form, not a fully folded form. */
          )
            {
            inFull = true;
            i = 1;
            return w[0];
            }
        return Letter[ind].Simple;
        }
    return kar;
    }

class charfolded:public folded // ISO
    {
    private:
        const char * s;
    protected:
        virtual int c()
            {
            return *s++;
            }
    public:
        charfolded(const char * s):s(s)
            {
            }
        virtual ~charfolded()
            {
            }
    };

class wcharfolded:public folded // UTF16
    {
    private:
        const wchar_t * s;
    protected:
        virtual int c()
            {
            return *s++;
            }
    public:
        wcharfolded(const wchar_t * s):s(s)
            {
            }
        virtual ~wcharfolded()
            {
            }
    };

int strCaseCmp(const wchar_t *s, const char *p)
    {
    wcharfolded S(s);
    charfolded P(p);
    int iS,iP;
    while(true)
        {
        iS = S.C();
        iP = P.C();
        if(iS)
            {
            if(iP)
                {
                if(iS != iP)
                    return iS - iP;
                }
            else
                return 1;
            }
        else
            {
            if(iP)
                return -1;
            else
                return 0;
            }
        }
    }

#endif
