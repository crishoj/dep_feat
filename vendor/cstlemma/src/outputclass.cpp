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
#include "outputclass.h"
#include "functiontree.h"
#include "functio.h"
#include "comparison.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef COUNTOBJECTS
int OutputClass::COUNT = 0;
#endif


const char * OutputClass::Format(const char * format,getFunction gfnc,functionTree & tree,const char * allFormat,bool & SortInput,int & testType)
    {
    int loctestType = 0;
    int locloctestType = 0;
    if(!format)
        {
        printf("format string missing.\n");
        exit(0);
        return NULL;
        }
    char buf[100];
    char * str = NULL;
    const char * f;
    bool block = false;
    for(f = format;*f && *f != '$' && *f != '[' && *f != ']';++f)
        {
        int s = *f;
        if(s == '\\')
            {
            ++f;
            switch(*f)
                {
                case 'n':s='\n';break;
                case 't':s='\t';break;
                case 'r':s='\r';break;
                case 'v':s='\v';break;
                case 'b':s='\b';break;
                default: s= *f;
                }
            }
        if(!str)
            str = buf;
        *str++ = (char)s;
        }
    if(str)
        {
        *str = '\0';
        tree.setFunction(new functionString(buf));
        }
    else if(*f == '$')
        {
        ++f;
        function * tmp = gfnc(*f,SortInput,loctestType);
        if(!tmp)
            {
            printf("unknown field %c in format \"%s\"\n",*f,allFormat);
            printf("                            %*c\n",(int)(strlen(allFormat) - strlen(f)),'^');
            exit(0);
            return NULL;
            }
        tree.setFunction(tmp);
        ++f;
        }
    /*
    else if(*f == '|')
        {
        ++f;
        tree.setOr();
        }*/
    else if(*f == '[')
        {
        tree.setComp(etest);
        const char * newf = f + 1;
        newf = Format(newf,gfnc,tree.addChild(),allFormat,SortInput,locloctestType);
        if(!newf || *newf != ']')
            {
            printf("No matching ] in format \"%s\"\n",allFormat);
            printf("                          %*c\n",(int)(strlen(allFormat) - strlen(f)),'^');
            exit(0);
            }
        f = newf+1;
        block = true;
        }
    else if(*f == ']')
        {
        if(testType == 0)
            {
            printf("No countable expression found in format \"%s\"\n",allFormat);
            printf("                                          %*c\n",(int)(strlen(allFormat) - strlen(f)),'^');
            exit(0);
            }
        return f;
        }
    else if(!*f)
        {
        return NULL;
        }

    //add condition
    int condition = -1;
    if(*f == '<' || *f == '>' || *f == '~')
        {
        condition = 1;
        tree.setComp(*f == '<' ? eless : *f == '>' ? emore : enotequal);
        ++f;
        if(!isdigit(*f))
            {
            printf("format \"%s\" must have one or more digits after %c.\n",allFormat,*--f);
            printf("         %*c\n",(int)(strlen(allFormat) - strlen(f)),'^');
            exit(0);
            return NULL;
            }
        else
            {
            int nmbr = *f - '0';
            do
                {
                condition++;
                ++f;
                if(isdigit(*f))
                    {
                    nmbr *= 10;
                    nmbr += *f - '0';
                    }
                else 
                    break;
                }
            while(true);
            tree.setNmbr(nmbr);
            }
        }
    else if(isdigit(*f))
        {
        condition = 0;
        tree.setComp(eequal);
        int nmbr = *f - '0';
        do
            {
            condition++;
            ++f;
            if(isdigit(*f))
                {
                nmbr *= 10;
                nmbr += *f - '0';
                }
            else 
                break;
            }
        while(true);
        tree.setNmbr(nmbr);
        }
    else if(*f == '+')
        {
        condition = 1;
        tree.setComp(emore);
        tree.setNmbr(0);
        ++f;
        }
    else if(*f == '*')
        {
        condition = 1;
        tree.setComp(eany);
        ++f;
        }
    else if(*f == '?')
        {
        condition = 1;
        tree.setComp(eany);
        tree.hide();
        ++f;
        }
    else if(block)
        locloctestType = 0;
    loctestType |= locloctestType;

    if(condition > -1 && !(loctestType & NUMBERTEST))
        {
        printf("format \"%s\" has illegal test.\n",allFormat);
        printf("         %*c\n",(int)(strlen(allFormat) - strlen(f) - condition),'^');
        exit(0);
        return NULL;
        }

    testType |= loctestType;
    return Format(f,gfnc,tree.addNext(),allFormat,SortInput,testType);
    }

