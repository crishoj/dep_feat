/*
CSTLEMMA - trainable lemmatiser using word-end inflectional rules

Copyright (C) 2002, 2008  Center for Sprogteknologi, University of Copenhagen

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

/*
D:\projects\lemmatiser\bin\vc6\Release\cstlemma.exe -L -e1 -p+ -q- -t- -U- -H2 -fD:\projects\tvarsok\ru\rules_0utf8.lem -B$w -l -c$w/$B$s -i D:\dokumenter\russisk\russisk.txt -o D:\dokumenter\russisk\russisk.lemmatised.txt -m0
*/
#include "hash.h"
#include "applyrules.h"
#include "flex.h"
#include "caseconv.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct var
    {
    const char * s;
    const char * e;
    };

static const char * flexFileName;
static char * buf = "\0x00\0x00\0x00\0x00\0x09\0x09\0x09\0x0a";
static long end = 8;
static char * result = 0;
static bool NewStyle = false;


static char * readRules(FILE * flexrulefile,long & end)
    {
    if(flexrulefile)
        {
        int start;
        fread(&start,sizeof(int),1,flexrulefile);
        if(start != 0)
            {
            return 0; // not the new format
            }
        fseek(flexrulefile,0,SEEK_END);
        end = ftell(flexrulefile);
        char * buf = new char[end];
        if(buf)
            {
            rewind(flexrulefile);
            fread(buf,1,end,flexrulefile);
            NewStyle = true;
            }
        return buf;
        }
    return 0;
    }

class rules
    {
    private:
        char * TagName;
        char * Buf;
        long End;
    public:
        rules(const char * TagName)
            {
            this->TagName = new char[strlen(TagName)+1];
            strcpy(this->TagName,TagName);
            char * filename = new char[strlen(TagName)+strlen(flexFileName)+2];
            sprintf(filename,"%s.%s",flexFileName,TagName);
            FILE * f = fopen(filename,"rb");
            if(f)
                {
                Buf = readRules(f, End);
                fclose(f);
                }
            else
                {
                Buf = 0;
                End = 0;
                }
            }
        const char * tagName() const {return TagName;}
        const char * buf(){return Buf;}
        long end(){return End;}
        void print(){}
    };

static hash<rules> * Hash = NULL;


bool newStyleRules()
    {
    return NewStyle;
    }

static const char * samestart(const char ** fields,const char * s,const char * we)
    {
    const char * f = fields[0];
    const char * e = fields[1]-1;
    while((f < e) && (s < we) && (*f == *s))
        {
        ++f;
        ++s;
        }
    return f == e ? s : 0;
    }

static const char * sameend(const char ** fields,const char * s,const char * wordend)
    {
    const char * f = fields[2];
    const char * e = fields[3]-1;
    const char * S = wordend - (e-f);
    if(S >= s)
        {
        s = S;
        while(f < e && *f == *S)
            {
            ++f;
            ++S;
            }
        }
    return f == e ? s : 0;
    }

static bool substr(const char ** fields,int k,const char * w,const char * wend,var * vars,int vindex)
    {
    if(w == wend)
        return false;
    const char * f = fields[k];
    const char * e = fields[k+1]-1;
    const char * p = w;
    assert(f != e);
    const char * ff;
    const char * pp;
    do
        {
        while((p < wend) && (*p != *f))
            {
            ++p;
            }
        if(p == wend)
            return false;
        pp = ++p;
        ff = f+1;
        while(ff < e)
            {
            if(pp == wend)
                return false;
            if(*pp != *ff)
                break;
            ++pp;
            ++ff;
            }
        }
    while(ff != e);
    vars[vindex].e = p-1;
    vars[vindex+1].s = pp;
    return true;
    }

/*
static void printpat(const char ** fields,int findex)
    {
    printf("findex %d {%.*s]",findex,fields[1] - fields[0] - 1,fields[0]);//,vars[0].e - vars[0].s,vars[0].s);
    for(int m = 1;2*m+3 < findex;++m)
        {
        int M = 2*m+3;
        printf("[%.*s]",fields[M] - fields[M-1] - 1,fields[M-1]);//,vars[m].e - vars[m].s,vars[m].s);
        }
    if(findex > 2)
        printf("[%.*s}\n",fields[3] - fields[2] - 1,fields[2]);
    else
        printf("\n");
    }
*/

static bool lemmatiseer(const char * word,const char * wordend,const char * buf,int pos,int maxpos)
    {
    do
        {
        var vars[20];
        const char * fields[44];
        // output=fields[1]+vars[0]+fields[5]+vars[1]+fields[7]+vars[2]+...+fields[2*n+3]+vars[n]+...+fields[3]
        const char * Record = buf+pos;
        const char * wend = wordend;
        pos = *(int*)Record;
        const char * p = Record + sizeof(int);
        fields[0] = p;
        int findex = 1;
        while(*p != '\n')
            {
            if(*p == '\t')
                fields[findex++] = ++p;
            else
                ++p;
            }
        fields[findex] = ++p; // p is now within 3 bytes from the next Record.
//        printpat(fields,findex);
        // check Lpat
        vars[0].s = samestart(fields,word,wend);
        if(vars[0].s)
            {
            // Lpat succeeded
            vars[0].e = wend;
            if(findex > 2)
                {
                const char * newend = sameend(fields,vars[0].s,wend);
                if(newend)
                    wend = newend;
                else
                    continue;

                int k;
                const char * w = vars[0].s;
                int vindex = 0;
                for(k = 4;k < findex;k += 2)
                    {
                    if(!substr(fields,k,w,wend,vars,vindex))
                        break;
                    ++vindex;
                    w = vars[vindex].s;
                    }
                if(k < findex)
                    continue;
                vars[vindex].e = newend;
                long nxt = p - buf;
                nxt += 3;
                nxt >>= 2;
                nxt <<=2;
                if(pos)
                    maxpos = pos;
                if(nxt >= maxpos || !lemmatiseer(vars[0].s,wend,buf,nxt,maxpos))
                    {
                    int resultlength = (fields[2] - fields[1] - 1) + (vars[0].e - vars[0].s) + (fields[4] - fields[3] - 1);
                    int m;
                    for(m = 1;2*m+3 < findex;++m)
                        {
                        int M = 2*m+3;
                        resultlength += (fields[M+1] - fields[M] - 1) + (vars[m].e - vars[m].s);
                        }
                    result = new char[resultlength+1];
                    int printed = sprintf(result,"%.*s%.*s",fields[2] - fields[1] - 1,fields[1],vars[0].e - vars[0].s,vars[0].s);

                    for(m = 1;2*m+3 < findex;++m)
                        {
                        int M = 2*m+3;
                        printed += sprintf(result+printed,"%.*s%.*s",fields[M+1] - fields[M] - 1,fields[M],vars[m].e - vars[m].s,vars[m].s);
                        }
                    sprintf(result+printed,"%.*s",fields[4] - fields[3] - 1,fields[3]);
                    }
                }
            else if(vars[0].e == vars[0].s)
                {
                result = new char[(fields[2] - fields[1] - 1)+1];
                sprintf(result,"%.*s",fields[2] - fields[1] - 1,fields[1]);
                }
            else
                continue;

    
            return true;
            }
        else
            {
            // Lpat failed
            continue;
            }
        }
    while(pos);
    return false;
    }


bool readRules(FILE * flexrulefile,const char * flexFileName)
    {
    if(flexFileName)
        {
        ::flexFileName = flexFileName;
        }
    if(flexrulefile)
        {
        fseek(flexrulefile,0,SEEK_END);
        end = ftell(flexrulefile);
        buf = new char[end];
        if(buf)
            {
            rewind(flexrulefile);
            fread(buf,1,end,flexrulefile);
            NewStyle = true;
            }
        return buf != 0;
        }
    return flexFileName != 0;
    }


bool readRules(const char * flexFileName)
    {
    if(flexFileName)
        {
        ::flexFileName = flexFileName;
        }
    if(Hash == NULL)
        Hash = new hash<rules>(&rules::tagName,10);
    return flexFileName != 0;
    }
/*
bool readRules(const char * filename)
    {
    FILE * f = fopen(filename,"rb");
    if(f)
        {
        buf = readRules(f);
        return buf != 0;
        }
    return false;
    }
*/


void deleteRules()
    {
    delete [] buf;
    buf = 0;
    delete [] result;
    result = 0;
    NewStyle = false;
    }


const char * applyRules(const char * word)
    {
    if(buf)
        {
        int len = strlen(word);
        char * loword = 0;
        if(flex::baseformsAreLowercase)
            {
            loword = new char[len+1];
            strcpy(loword,word);
            AllToLower(loword);
            word = loword;
            }

        delete [] result;
        result = 0;
        lemmatiseer(word,word+len,buf,0,end);
        delete [] loword;
        return result;
        }
    return 0;
    }

const char * applyRules(const char * word,const char * tag)
    {
    if(buf)
        {
        int len = strlen(word);
        char * loword = 0;
        if(flex::baseformsAreLowercase)
            {
            loword = new char[len+1];
            strcpy(loword,word);
            AllToLower(loword);
            word = loword;
            }
        delete [] result;
        result = 0;
        if(tag && *tag)
            {
            void * v;
            rules * Rules;
            if(!Hash)
                Hash = new hash<rules>(&rules::tagName,10);
            Rules = Hash->find(tag,v);
            if(!Rules)
                {
                Rules = new rules(tag);
                Hash->insert(Rules,v);
                /*
                Hash->remove(Rules,v);
                Hash->deleteThings();
                Hash->forall(&rules::print);
                int N;
                rules ** Rls = Hash->convertToList(N);
                */
                }
            if(Rules->buf())
                lemmatiseer(word,word+len,Rules->buf(),0,Rules->end());
            else
                lemmatiseer(word,word+len,buf,0,end);
            }
        else
            lemmatiseer(word,word+len,buf,0,end);
        delete [] loword;
        return result;
        }
    return 0;
    }

