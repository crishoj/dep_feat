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
#include "readlemm.h"
#include "fieldfnc.h"
#include <string.h>

static int s_bm = 0;
static int la = 0;
static int le = 0;
static char s_baseform[1000];
static char flexform[1000];
static char lextype[1000] = "_";
static int kar = 0;
static bool s_CollapseHomographs;


bool removeCardinal(char * baseform,int bm)
    {
//    bool nmbr = false;
//    while(bm > 0 && isdigit(baseform[bm])) isdigit('�') sometimes true (at least in VC6)!
    while(bm > 0 && '0' <= baseform[bm] && baseform[bm] <= '9')
        {
//        done = true;
//        nmbr = true;
//        baseform[bm--] = '\0';
        --bm;
        }
    if(/*nmbr && */bm > 0 && baseform[bm] == ',')
        {
        baseform[bm] = '\0';
        return true;
        }
    return false;
    }


static bool readBaseform(FILE * fpin)
    {
    while(true)
        {
//        int kar;
        kar = fgetc(fpin);
        if(kar == EOF)
            {
            s_baseform[s_bm] = '\0';
//            if(s_bm > 0 && la > 0 && le > 0)
//                add(s_baseform,flexform,lextype);
            return true;
            }
        if(kar == '\n' || kar == '\t')
            {
            s_baseform[s_bm] = '\0';
            if(s_CollapseHomographs)
                removeCardinal(s_baseform,s_bm - 1);
//Cardinal must stay in dictionary!            removeCardinal(s_baseform,s_bm - 1);
            break;
            }
        if(kar != '\r')// Was commented out Bart 20050216
            s_baseform[s_bm++] = (char)kar;
        }
    return false;
    }

static bool readFullform(FILE * fpin)
    {
    while(true)
        {
//        int kar;
        kar = fgetc(fpin);
        if(kar == EOF)
            {
            flexform[la] = '\0';
            return true;
            }
        if(kar == '\n' || kar == '\t')
            {
            flexform[la] = '\0';
            return false;
            }
        if(kar != '\r')// Was commented out Bart 20050216
            flexform[la++] = (char)kar;
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
            lextype[le] = '\0';
            return true;
            }
        if(kar == '\n' || kar == '\t')
            {
            lextype[le] = '\0';
//            add(s_baseform,flexform,lextype);
            return false;
            }
        if(kar != '\r')// Was commented out Bart 20050216
            lextype[le++] = (char)kar;
        }
    }

static bool readDummyType(FILE * fpin)
    {
    le = 1;
    while(true)
        {
        kar = fgetc(fpin);
        if(kar == EOF)
            {
            return true;
            }
        if(kar == '\n' || kar == '\t')
            {
            return false;
            }
        }
    }

int readLemmas(FILE * fpin,const char * format,adder func,bool CollapseHomographs,int & failed)
    {
    ::s_CollapseHomographs = CollapseHomographs;
    fieldfnc FIELDS[5];
    unsigned int ind = 0;
    while(ind < sizeof(FIELDS)/sizeof(FIELDS[0]))
        {
        FIELDS[ind++] = NULL;
        }
    //Bart 20030113
    int neededFields = 0;
    bool T = false;
    for(unsigned int n = 0;n < sizeof(FIELDS)/sizeof(FIELDS[0]) && format[n];++n)
        {
        switch(format[n])
            {
            case 'F':
            case 'f':
                if(neededFields & 1)
                    {
                    printf("Invalid format string %s (duplicate f or F)\n",format);
                    return 0;
                    }
                neededFields |= 1;
                FIELDS[n] = readFullform;
                break;
            case 'B':
            case 'b':
                if(neededFields & 2)
                    {
                    printf("Invalid format string %s (duplicate b or B)\n",format);
                    return 0;
                    }
                neededFields |= 2;
                FIELDS[n] = readBaseform;
                break;
            case 'T':
            case 't':
                if(T)
                    {
                    printf("Invalid format string %s (duplicate t or T)\n",format);
                    return 0;
                    }
                T = true;
                FIELDS[n] = readType;
                break;
            case '?':
                FIELDS[n] = readDummyType;
                break;
            default:
                {
                printf("Invalid format string \"%s\"\n (Only characters fbtFBT? are allowed, but found %.2x=\"%c\")\n",format,format[n],format[n]);
                return 0;
                }
            }
        }

    switch(neededFields)
        {
        case 0:
            printf("Invalid format string %s (F and B not specified)\n",format);
            return 0;
        case 1:
            printf("Invalid format string %s (B not specified)\n",format);
            return 0;
        case 2:
            printf("Invalid format string %s (F not specified)\n",format);
            return 0;
        }

    int cnt = 0;
    failed = 0;
    FILE * err = fopen("discarded","wb");
    bool eof = false;
    while(!eof)
        {
        unsigned int ind = 0;
        while(!eof && ind < sizeof(FIELDS)/sizeof(FIELDS[0]) && FIELDS[ind])
            {
            if(FIELDS[ind++](fpin))
                eof = true;
            }
        if(s_bm > 0 && la > 0 && (!T || le > 0))
            {
            if(func(s_baseform,flexform,lextype))
                cnt++;
            else
                {
                failed++;
                fprintf(err,"%d:%s %s %s\n",cnt+failed,s_baseform,flexform,lextype);
                }
            }
        s_bm = la = le = 0;
        while(!eof && kar != '\n')
            {
            kar = fgetc(fpin);
            if(kar == EOF)
                eof = true;
            }
        }
    s_bm = la = le = 0; // Bart 20050728
    fclose(err);
    if(!failed)
        remove("discarded");
    return cnt;
    }

bool removeBogus(char * dictFlexform)
    {
    // mineralvand,2erne --> mineralvanderne
    char * komma = strchr(dictFlexform,',');
    if(komma)
        {
        char * num = komma+1;
        while(*num && '0' <= *num && *num <= '9')
            ++num;
        while(*num)
            *komma++ = *num++;
        return true;
        }
    return false;
    }
