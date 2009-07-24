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


#include "flex.h"
#include "caseconv.h"
#include "readlemm.h"
#include "applyrules.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>



bool flex::baseformsAreLowercase = true;




void base::print(int n)
    {
    printf("%*c%s] %d\n",n,'[',m_baseform,refcnt);
    if(m_next)
        m_next->print(n);
    }

//------------------
void node::removeAmbiguous(node *& prev)
    {
    if(basef)
        {
        if(basef->Next())
            {
            delete basef;
            basef = 0;
            }
        }
    
    if(m_sub)
        m_sub->removeAmbiguous(m_sub);

    if(!m_sub && !basef)
        {
        prev = remove();
        if(prev)
            prev->removeAmbiguous(prev);
        }
    else if(m_next)
        m_next->removeAmbiguous(m_next);
    }
//------------------
void node::print(int n)
    {
    printf("%*c%s}\n",n,'{',m_tail);
    if(m_sub)
        m_sub->print(n+2);
    if(basef)
        basef->print(n);
    if(m_next)
        m_next->print(n);
    }
//-----------------

char * type::Baseform(char * invertedWord,base *& bf,int & ln)
    {
    base * BF = 0;
    int LN = -1;
    char * TP = 0;
    if(m_next)
        TP = m_next->Baseform(invertedWord,BF,LN);
    if(end->Baseform(invertedWord,bf,ln))
        {
        if(TP && BF && LN > ln) // choose the longest 
                        // (what if there are more than one?)
                // "longest" rule (rule that looks at most characters of 
                // word's ending to decide on the proper baseform) works 
                // better than both refcount and "shortest" rule. A priory, 
                // the latter should be better, because shorter rules should
                // be more general than longer rules. But then again, the 
                // chance for mistakes is greater if the rule looks at fewer
                // characters before deciding.
            {
            bf = BF;
            ln = LN;
            return TP;
            }
        else
            return this->m_tp;
        }
    else if(TP)
        {
        bf = BF;
        ln = LN;
        return TP;
        }
    else
        return 0;
    }

void type::print()
    {
    printf("%s\n",m_tp);
    if(end)
        end->print(2);
    if(m_next)
        m_next->print();
    }

void type::removeAmbiguous(type *& prev)
    {
    if(end)
        {
        end->removeAmbiguous(end);
        if(!end)
            {
            prev = remove();
            if(prev)
                prev->removeAmbiguous(prev);
            return;
            }
        }
    if(m_next)
        m_next->removeAmbiguous(m_next);
    }

//------------------
void flex::removeAmbiguous()
    {
    if(newStyleRules())
        {
        oneAnswer = true;
        }
    else if(types)
        types->removeAmbiguous(types);
    }

void flex::print()
    {
    if(types)
        types->print();
    }

bool flex::Baseform(char * word,const char * tag,const char *& bf,int & borrow)
    {
    if(newStyleRules())
        {
        bf = applyRules(word,tag);
        return true;
        }

    if(types)
        {
        int offset = 0;
        int wlen = strlen(word);
        if(wlen > 256)
            {
            if(baseformsAreLowercase)
                AllToLower(word); // destructive, because overwriting word!
                                // But this is exceptional to begin with.
            bf = word;
            borrow = wlen;
            return true;
            }
        static char aWord[256];
        strcpy(aWord,word);
        if(baseformsAreLowercase)
            AllToLower(aWord);
        base * Base;
        Strrev(aWord);
        if(types->Baseform(aWord,tag,Base,offset))
            {
            borrow = wlen - offset;
            static char buf[256];
            char * b = buf;
            while(true)
                {
                strncpy(b,word,borrow);
                strcpy(b+borrow,Base->bf());

                if(baseformsAreLowercase)
                    AllToLower(b);
                else if(IsAllUpper(word)) // Bart 20090203: made UTF8-capable
                    {
                    allToUpper(b);
                    }
                else if(borrow == 0 && is_Upper(word))
                    {
                    toUpper(b);
                    }

                b += strlen(b);
                Base = Base->Next();
                if(Base)
                    {
                    *b++ = ' ';
                    }
                else
                    break;
                }
            bf = buf;
            return true;
            }
        else
            {
            return false;
            }
        }
    else
        return false;
    }

char * flex::Baseform(char * word,const char *& bf,int & borrow)
    {
    if(newStyleRules())
        {
        bf = applyRules(word);
        return "-";
        }


    if(types)
        {
        int offset = 0;
        int wlen = strlen(word);
        if(wlen > 256)
            {
            if(baseformsAreLowercase)
                AllToLower(word); // destructive, because overwriting word!
                                // But this is exceptional to begin with.
            bf = word;
            borrow = wlen;
            return 0;
            }
        static char aWord[256];
        strcpy(aWord,word);
        if(baseformsAreLowercase)
            AllToLower(aWord);
        base * Base;
        Strrev(aWord);
        char * tag = types->Baseform(aWord,Base,offset);
        if(tag)
            {
            borrow = wlen - offset;
            static char buf[256];
            char * b = buf;
            while(true)
                {
                strncpy(b,word,borrow);
                strcpy(b+borrow,Base->bf());
                if(baseformsAreLowercase)
                    AllToLower(b); // We have no lexical type information to 
                // decide whether capitals should be used, so we assume all 
                // lower case.

                b += strlen(b);
                Base = Base->Next();
                if(Base)
                    {
                    *b++ = ' ';
                    }
                else
                    break;
                }
            bf = buf;
            return tag;
            }
        else
            {
            return 0;
            }
        }
    else
        return 0;
    }

    bool flex::readFromFile(FILE * fpflex,const char * flexFileName)
        {
        if(!fpflex)
            {
            return false;
            }
        int start;
        fread(&start,sizeof(int),1,fpflex);
        rewind(fpflex);
        if(start == 0)
            { // new style flexrules 20080218
            if(!readRules(fpflex,flexFileName))
                return false;
            }
        else
            {
            return readFromFile(fpflex);
            }
        return true;
        }
