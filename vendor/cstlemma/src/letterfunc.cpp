/*
CSTLEMMA - trainable lemmatiser using word-end inflectional rules

Copyright (C) 2002, 2005, 2009  Center for Sprogteknologi, University of Copenhagen

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

bool Turcic = false;

bool isAlpha(int kar)
    {
/*    int ind = kar % ARRSIZE;
    if((int)Letter[ind].Unfolded == kar)
        return true;*/

/* From http://www.w3.org/TR/xml11/#NT-NameStartChar:
NameStartChar	   ::=   	":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] 
        | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] | [#x37F-#x1FFF] 
        | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] 
        | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] 
        | [#x10000-#xEFFFF]
*/
    // 20090702
    return 'A' <= kar && kar <= 'Z'
        || 'a' <= kar && kar <= 'z'
        || 0xC0 <= kar && kar <= 0xD6
        || 0xD8 <= kar && kar <= 0xF6
        || 0xF8 <= kar && kar <= 0x2FF
        || 0x370 <= kar && kar <= 0x37D
        || 0x37F <= kar && kar <= 0x1FFF
        || 0x200C <= kar && kar <= 0x200D 
        || 0x2070 <= kar && kar <= 0x218F 
        || 0x2C00 <= kar && kar <= 0x2FEF 
        || 0x3001 <= kar && kar <= 0xD7FF 
        || 0xF900 <= kar && kar <= 0xFDCF 
        || 0xFDF0 <= kar && kar <= 0xFFFD
        || 0x10000 <= kar && kar <= 0xEFFFF;
    //return kar > 255; // Assume Unicode positions outside Latin-1 is all alphabetic
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
    if(kar == 'I' && !Turcic)
        return 'i';
    int ind = kar % ARRSIZE;
    return ((int)Letter[ind].Unfolded == kar) && (Letter[ind].Simple) ? Letter[ind].Simple : kar;
    }

unsigned int upperEquivalent(int kar)
    {
    if(kar == 'i' && !Turcic)
        return 'I';
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
