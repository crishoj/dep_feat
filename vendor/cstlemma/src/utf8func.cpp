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
#include "utf8func.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#if UNICODE_CAPABLE
#include "letter.h"

bool UTF8 = true;

bool isUpperUTF8(const char * s)
    {
    int S = UTF8char(s,UTF8);
    return upperEquivalent(S) == (unsigned int)S;
    }

bool isAllUpper(const char * s,int len)
    {
    bool UTF8 = true;
    int S;
    if(len > 0)
        {
        while((S = getUTF8char(s,UTF8)) != 0 && len-- > 0)
            {
            if(upperEquivalent(S) != (unsigned int)S)
                return false;
            }
        }
    else
        {
        while((S = getUTF8char(s,UTF8)) != 0)
            {
            if(upperEquivalent(S) != (unsigned int)S)
                return false;
            }
        }
    return true;
    }

int UnicodeToUtf8(int w,char * s,size_t len)
// Converts Unicode character w to 1,2,3 or 4 bytes of UTF8 in s.
// Returns number of bytes written.
    {
    // Convert unicode to utf8
    int bytes;
    if ( w < 0x0080 )
        {
        if(len < 1)
            return 0;
        *s = (char)w;
        bytes = 1;
        }
    else 
        {
        if(w < 0x0800) // 7FF = 1 1111 111111
            {
            if(len < 2)
                return 0;
            *s++ = (char)(0xc0 | (w >> 6 ));
            bytes = 2;
            }
        else
            {
            if(w < 0x10000) // FFFF = 1111 111111 111111
                {
                if(len < 3)
                    return 0;
                *s++ = (char)(0xe0 | (w >> 12));
                bytes = 3;
                } 
            else // w < 110000
                { // 10000 = 010000 000000 000000, 10ffff = 100 001111 111111 111111
                if(len < 4)
                    return 0;
                *s++ = (char)(0xf0 | (w >> 18));
                *s++ = (char)(0x80 | ((w >> 12) & 0x3f));
                bytes = 4;
                } 
            *s++ = (char)(0x80 | ((w >> 6) & 0x3f));
            }
        *s = (char)(0x80 | (w & 0x3f));
        }
    return bytes;
    }

void AllToUpper(char * s)
    {
    /*
    The only character that will become longer when uppercased is the Turcic i
    i i İ U130>0x80  69
    The only character that will become shorter when uppercased is the Turcic ı
    ı ı I U49<0x80  131
    */
    size_t len = strlen(s);
    size_t L = len;
    char * start = new char[len + 1];
    char * dest = start;
    const char * cs = s;
    int S;
    while((S = getUTF8char(cs,UTF8)) != 0)
        {
        int n = UnicodeToUtf8(upperEquivalent(S),dest,L);
        if(n == 0)
            break;
        L -= n;
        dest += n;
        }
    *dest = '\0';
    if(S == 0)
        {
        strcpy(s,start);
        }
    else
        {
        fprintf(stderr,"UpperEquivalent(%s) [%s..] longer than %s\n",s,dest,s);
        }
    delete [] start;
    }


const char * allToLowerUTF8(const char * s)
    {
/* No characters will become longer when lowercased.
Some will become shorter:
İ i İ L69<0x80  130 (LATIN CAPITAL LETTER I WITH DOT ABOVE)
ſ s ſ L73<0x80  17f (LATIN SMALL LETTER LONG S)
ι ι ι L3b9<0x800  1fbe (GREEK PROSGEGRAMMENI)
Ω ω Ω L3c9<0x800  2126 (OHM SIGN)
K k K L6b<0x800  212a (KELVIN SIGN)
Å å Å Le5<0x800  212b (ANGSTROM SIGN)
*/
    static char buf[256];
    static char * ret = buf;
    if(ret != buf)
        delete [] ret;
    size_t len = strlen(s) + 1;
    if(len > 255)
        ret = new char[len+1];
    else
        ret = buf;

    size_t L = len;
    char * dest = ret;
    const char * cs = s;
    int S;
    while((S = getUTF8char(cs,UTF8)) != 0)
        {
        int n = UnicodeToUtf8(lowerEquivalent(S),dest,L);
        if(n == 0)
            break;
        L -= n;
        dest += n;
        }
    *dest = '\0';
    assert(S == 0);
    /*
    if(S != 0)
        {
        fprintf(stderr,"LowerEquivalent(%s) [%s..] longer than %s\n",s,dest,s);
        }
        */
    return ret;
    }

void AllToLowerUTF8(char * s)
    {
/* No characters will become longer when lowercased.
Some will become shorter:
İ i İ L69<0x80  130 (LATIN CAPITAL LETTER I WITH DOT ABOVE)
ſ s ſ L73<0x80  17f (LATIN SMALL LETTER LONG S)
ι ι ι L3b9<0x800  1fbe (GREEK PROSGEGRAMMENI)
Ω ω Ω L3c9<0x800  2126 (OHM SIGN)
K k K L6b<0x800  212a (KELVIN SIGN)
Å å Å Le5<0x800  212b (ANGSTROM SIGN)
*/
    size_t len = strlen(s) + 1;
    char * ret = new char[len+1];

    size_t L = len;
    char * dest = ret;
    const char * cs = s;
    int S;
    while((S = getUTF8char(cs,UTF8)) != 0)
        {
        int n = UnicodeToUtf8(lowerEquivalent(S),dest,L);
        if(n == 0)
            break;
        L -= n;
        dest += n;
        }
    *dest = '\0';
    assert(S == 0);
    strcpy(s,ret);
    delete [] ret;
    }


int getUTF8char(const char *& s,bool & UTF8) 
/* Returns *s if s isn't UTF8 and increments s with 1.
   Otherwise returns character and shifts s to start of next character.
   Returns 0 if end of string reached. In that case, s becomes invalid
   (pointing past the zero).
*/
    {
    const unsigned char * save = (const unsigned char *)s;
    int k[6];
    int kk;
    if((kk = *s++) != 0)
        {
        k[0] = kk;
        if((k[0] & 0xc0) == 0xc0) // 11bbbbbb
            {
            int i = 1;
            if((k[0] & 0xfe) == 0xfe)
                {
                s = (const char *)save + 1;
                UTF8 = false;
                return *save;
                }
            // Start of multibyte
            
            for(int k0 = k[0];(k0 << i) & 0x80;++i)
                {
                k[i] = *s++;
                
                if((k[i] & 0xc0) != 0x80) // 10bbbbbb
                    {
                    s = (const char *)save + 1;
                    UTF8 = false;
                    return *save;
                    }
                }
            int K = ((k[0] << i) & 0xff) << (5 * i - 6);
            int I = --i;
            while(i > 0)
                {
                K |= (k[i] & 0x3f) << ((I - i) * 6);
                --i;
                }
            if(K <= 0x7f)
                {
                s = (const char *)save + 1;
                UTF8 = false;
                return *save; // overlong UTF-8 sequence
                }
            return K;
            }
        else
            return k[0];
        }
    else
        return 0;
    }

int copyUTF8char(const char * s,char * dest)
// Makes no check of validity of UTF8!
    {
    *dest = *s;
    if((*s & 0xc0) == 0xc0) // 11bbbbbb
        {
        // Start of multibyte
        int i = 1;
        while((*++s & 0xc0) == 0x80) // 10bbbbbb
            {
            ++i;
            *++dest = *s;
            }
        return i;
        }
    else
        return 1;
    }

int skipUTF8char(const char * s)
// Makes no check of validity of UTF8!
    {
    if(UTF8 && (*s & 0xc0) == 0xc0) // 11bbbbbb
        {
        // Start of multibyte
        int i = 1;
        while((*++s & 0xc0) == 0x80) // 10bbbbbb
            {
            ++i;
            }
        return i;
        }
    else
        return 1;
    }


int Utf8ToUnicode(int * w,const char * s,int len)
    {
    int i = 0;
    int S;
    const char * end = s + len;
    while((S = getUTF8char(s,UTF8)) != 0 && s <= end)
        {
        *w++ = S;
        ++i;
        }
    *w = 0;
    return i;
    }


int UTF8char(const char * s,bool & UTF8) 
/* Returns *s if s isn't UTF8.
   Otherwise returns UTF8 character.
   Returns 0 if end of string reached.
   Does not increment s;
*/
    {
    if(!UTF8)
        return *s;

    const unsigned char * save = (const unsigned char *)s;
    int k[6];
    int kk;
    if((kk = *s++) != 0)
        {
        k[0] = kk;
        if((k[0] & 0xc0) == 0xc0) // 11bbbbbb
            {
            int i = 1;
            if((k[0] & 0xfe) == 0xfe)
                {
                UTF8 = false;
                return *save;
                }
            // Start of multibyte
            
            for(int k0 = k[0];(k0 << i) & 0x80;++i)
                {
                k[i] = *s++;
                
                if((k[i] & 0xc0) != 0x80) // 10bbbbbb
                    {
                    UTF8 = false;
                    return *save;
                    }
                }
            int K = ((k[0] << i) & 0xff) << (5 * i - 6);
            int I = --i;
            while(i > 0)
                {
                K |= (k[i] & 0x3f) << ((I - i) * 6);
                --i;
                }
            if(K <= 0x7f)
                {
                UTF8 = false;
                return *save; // overlong UTF-8 sequence
                }
            return K;
            }
        else
            return k[0];
        }
    else
        return 0;
    }

const char * Inc(const char *& s)
    /* essentially *s++
       but:
            UTF8 sequences count as one character
            if *s is zero, s is not incremented
    */
    {
    if(!UTF8)
        return s++;

    const char * save = s;
    int k[6];
    int kk;
    if((kk = *s++) != 0)
        {
        k[0] = kk;
        if((k[0] & 0xc0) == 0xc0) // 11bbbbbb
            {
            int i = 1;
            if((k[0] & 0xfe) == 0xfe)
                {
                s = save + 1;
                return s;
                }
            // Start of multibyte
            
            for(int k0 = k[0];(k0 << i) & 0x80;++i)
                {
                k[i] = *s++;
                
                if((k[i] & 0xc0) != 0x80) // 10bbbbbb
                    {
                    s = save + 1;
                    return s;
                    }
                }
            int K = ((k[0] << i) & 0xff) << (5 * i - 6);
            int I = --i;
            while(i > 0)
                {
                K |= (k[i] & 0x3f) << ((I - i) * 6);
                --i;
                }
            if(K <= 0x7f)
                {
                s = save + 1;
                return s; // overlong UTF-8 sequence
                }
            return s;
            }
        else
            return s;
        }
    else
        return save;
    }


class mcharfolded:public folded // UTF8
    {
    private:
        const char * s;
        bool UTF8;
    protected:
        virtual int c()
            {
            if(UTF8)
                {
                return getUTF8char(s,UTF8);
                }
            return *s++;
            }
    public:
        mcharfolded(const char * s):s(s),UTF8(true)
            {
            }
        virtual ~mcharfolded()
            {
            }
    };

int strCaseCmp(const char *s, const char *p)
    {
    /* Assumes that s and p are UTF8. However, if they aren't, 
       the algorithm assumes that s and p are one byte per character.
    */
    mcharfolded S(s);
    mcharfolded P(p);
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
