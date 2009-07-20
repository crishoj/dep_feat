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
#include "readfreq.h"
#include "fieldfnc.h"
#include <string.h>
#include <stdlib.h>

static int baseformindex = 0;
static int flexformindex = 0;
static int lextypeindex = 0;
static char baseform[1000];
static char flexform[1000];
static char lextype[1000];
static int lexfreq;
static int kar = 0;
static int line = 0;

static bool readBaseform(FILE * fpin)
    {
    while(true)
        {
//        int kar;
        kar = fgetc(fpin);
        if(kar == EOF)
            {
            baseform[baseformindex] = '\0';
            return true;
            }
        if(kar == '\n' || kar == '\t')
            {
            if(kar == '\n')
                ++line;
            baseform[baseformindex] = '\0';
            return false;
            }
        if(kar == '\r')
            continue;
        baseform[baseformindex++] = (char)kar;
        }
    }

static bool readFullform(FILE * fpin)
    {
    while(true)
        {
//        int kar;
        kar = fgetc(fpin);
        if(kar == EOF)
            {
            flexform[flexformindex] = '\0';
            return true;
            }
        if(kar == '\n' || kar == '\t')
            {
            if(kar == '\n')
                ++line;
            flexform[flexformindex] = '\0';
            return false;
            }
        if(kar == '\r')
            continue;
        flexform[flexformindex++] = (char)kar;
        }
    }

static bool readType(FILE * fpin)
    {
    while(true)
        {
//        int kar;
        kar = fgetc(fpin);
        if(kar == EOF)
            {
            lextype[lextypeindex] = '\0';
            return true;
            }
        if(kar == '\n' || kar == '\t')
            {
            if(kar == '\n')
                ++line;
            lextype[lextypeindex] = '\0';
//            add(baseform,flexform,lextype);
            return false;
            }
        if(kar == '\r')
            continue;
        lextype[lextypeindex++] = (char)kar;
        }
    }

static bool readNothing(FILE * fpin)
    {
    while(true)
        {
//        int kar;
        kar = fgetc(fpin);
        if(kar == EOF)
            {
            return true;
            }
        if(kar == '\n' || kar == '\t')
            {
            if(kar == '\n')
                ++line;
            return false;
            }
        if(kar == '\r')
            continue;
        }
    }

static bool readFreq(FILE * fpin)
    {
    while(true)
        {
//        int kar;
        kar = fgetc(fpin);
        if(kar == EOF)
            {
            return true;
            }
        if(kar == '\n' || kar == '\t')
            {
            if(kar == '\n')
                ++line;
            return false;
            }
        if(kar == '\r')
            continue;
        if(kar < '0' || kar > '9')
            {
            printf("Expected digit in frequency file (line %d), but found '%c'\n",line,kar);
            exit(0);
            }
        lexfreq = 10*lexfreq + (char)kar - '0';
        }
    }

void readFrequencies(FILE * fpin,const char * format,adderFreq func)
    {
    int nflds = strlen(format);
    fieldfnc * fieldFncs = new fieldfnc[nflds];
    bool hasBaseform = false;
    bool hasFullform = false;
    bool hasFreq = false;
    bool hasType = false;
    line = 0;
    for(int n = 0;n < nflds;++n)
        {
        switch(format[n])
            {
            case 'B':
            case 'b':
                fieldFncs[n] = readBaseform;
                hasBaseform = true;
                break;
            case 'F':
            case 'f':
                fieldFncs[n] = readFullform;
                hasFullform = true;
                break;
            case 'N':
            case 'n':
                fieldFncs[n] = readFreq;
                hasFreq = true;
                break;
            case 'T':
            case 't':
                fieldFncs[n] = readType;
                hasType = true;
                break;
            case '?':
                fieldFncs[n] = readNothing;
                break;
            default:
                {
                printf("Invalid format string \"%s\"\n (Only characters ?bfntBFNT are allowed, but found %.2x=\"%c\")\n",format,format[n],format[n]);
                delete [] fieldFncs;
                return;
                }
            }
        }
    char * basef = NULL;
    if(hasBaseform)
        {
        printf("Using base forms\n");
        basef = baseform;
        }
    else
        {
        printf("Not using base forms\n");
        }
    if(!hasFullform)
        {
        printf("Format string %s lacks F (full form)\n",format);
        delete [] fieldFncs;
        return;
        }
    if(!hasFreq)
        {
        printf("Format string %s lacks N (frequency)\n",format);
        delete [] fieldFncs;
        return;
        }
    if(!hasType)
        {
        printf("Format string %s lacks T (type)\n",format);
        delete [] fieldFncs;
        return;
        }

    for(int a = 0;a < nflds;++a)
        for(int b = a + 1;b < nflds;++b)
            if(fieldFncs[a] == fieldFncs[b] && fieldFncs[a] != readNothing)
                {
                printf("Invalid format string %s (duplicates)\n",format);
                delete [] fieldFncs;
                return;
                }
    
    while(true)
        {
        for(int i = 0;i < nflds;++i)
            if(fieldFncs[i](fpin))
                {
                if(flexformindex > 0 && lextypeindex > 0 && (!hasBaseform || baseformindex > 0))
                    func(lexfreq,flexform,lextype,basef);
                delete [] fieldFncs;
                return;
                }
        if(flexformindex > 0 && lextypeindex > 0)
            {
            if(flexformindex == 0 || lextypeindex == 0 || (hasBaseform && baseformindex == 0))
                {
                fgets(lextype,sizeof(lextype),fpin);
                fgets(lextype,sizeof(lextype),fpin);
                printf("The format %s does not fit the contents of the file\nExample:\n%s\n",format,lextype);
                return;
                }
            func(lexfreq,flexform,lextype,basef);
            }
        flexformindex = lextypeindex = baseformindex = 0;
        lexfreq = 0;
        while(kar != '\n')
            {
            kar = fgetc(fpin);
            if(kar == EOF)
                {
                delete [] fieldFncs;
                return;
                }
            }
        }
    }

