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
#if defined PROGLEMMATISE
#include "applyrules.h"
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef COUNTOBJECTS
int base::COUNT = 0;
int node::COUNT = 0;
int type::COUNT = 0;
int flex::COUNT = 0;

#include "basefrm.h"
#include "basefrmpntr.h"
#include "dictionary.h"
#include "field.h"
#include "freqfile.h"
#include "functio.h"
#include "functiontree.h"
#include "text.h"
#include "word.h"
#include "lemmatiser.h"
#include "option.h"
#endif

void Strrev(char * s)
    {
    char * e = s + strlen(s);
    while(s < --e)
        {
        char t = *s;
        *s++ = *e;
        *e = t;
        }
    }

flex Flex;

int base::nn = 0;
int base::mutated = 0;
int node::mutated = 0;
int type::mutated = 0;
long flex::CutoffRefcount = 0L;
bool flex::showRefcount = false;


static char EMPTY[] = "";

/*extern*/ bool training = false;

base * base::remove()
    {
    base * ret = m_next;
    m_next = 0;
    delete this;
    return ret;
    }

void node::cut(int c)
    {
    strcpy(m_tail,m_tail+c);
    m_len -= c;
    ++mutated;
    }

node * node::remove()
    {
    node * ret = m_next;
    m_next = 0;
    delete this;
    return ret;
    }

bool node::Baseform(char * invertedWord,base *& bf,int & ln)
    {
    int cmp = strncmp(m_tail,invertedWord,m_len);
    if(!cmp)
        return BaseformSub(invertedWord,bf,ln);
    if(m_next && *m_tail < *invertedWord)
        {
        return m_next->Baseform(invertedWord,bf,ln);
        }
    else
        return false;
    }


base * node::add(char * tail,char * baseform,bool fullWord,node *& prev,bool empty)
    {
    int n;
    // tail is inverted: last character first
    for(n = 0;tail[n] && this->m_tail[n] == tail[n];++n)
        ;
    if(n) // overlap
        {
        return addsub(tail,n,baseform,fullWord,prev);
        }
    else
        {
        if(!this->m_tail[0])
            {
            printf("ASSERT FAILED\n");
            }
        assert(this->m_tail[0]); // We do not allow empty tails. 
        // The rule [] (word remains unchanged) can therefore not be stored 
        // as a rule. It is assumed to be valid a priori.
        if(tail[0] && (!this->m_tail[0] || this->m_tail[0] > tail[0]))
            {
            prev = new node(this,tail,baseform,fullWord,false);
            return prev->Base();
            }
/*
ending(tail,baseform,fullWord,next,empty)
  node(next,tail,baseform,fullWord,empty)
*/
        else if(m_next)
            return m_next->add(tail,baseform,fullWord,m_next,empty);
        else 
            {
            m_next = new node(0,tail,baseform,fullWord,empty);
            return m_next->Base();
            }
        }
    }
//-----------------
bool node::BaseformSub(char * invertedWord,base *& bf,int & ln)
    {
    if(m_len && m_sub)
        {
        if(   m_sub->Baseform(invertedWord+m_len,bf,ln) 
           /* Bart 20030909. The lemma should not reduce to nothing, so either
              the word must have some remaining "stem" or the replacement
              string must not be empty. */
          && (training || invertedWord[m_len] || *bf->bf())
          )
            {
            ln += m_len;
            return true;
            }
        }
    if(  basef
           /* Bart 20030909. The lemma should not reduce to nothing, so either
              the word must have some remaining "stem" or the replacement
              string must not be empty. */
      && (training || invertedWord[m_len] || *basef->bf())
      )
        {
        bf = basef;
        ln = m_len;
        return true;
        }
    return false;
    }

//----------------

node::node(node * next,char * tail,char * baseform,bool fullWord,bool empty)
:m_next(next),m_sub(0)
    {
    m_len = strlen(tail);
    this->m_tail = new char[m_len+1];
    strcpy(this->m_tail,tail);
    if(!*tail && !empty)
        {
        printf("TAIL IS EMPTY BUT empty IS FALSE. (Found text:\"%s\")\n",baseform);
        }
    else if(*tail && empty)
        printf("TAIL IS NOT EMPTY BUT empty IS TRUE (Found text:\"%s\")\n",baseform);
    basef = new base(baseform,fullWord);
    ++mutated;
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }


node::node(node * next,char * tail,int n,char * baseform,bool fullWord,bool empty,node * sub)
        :m_next(next),m_sub(sub)
    {
    m_len = n;
    this->m_tail = new char[m_len+1];
    strncpy(this->m_tail,tail,n);
    this->m_tail[n] = '\0';
    if(!*tail && !empty)
        {
        printf("TAIL IS EMPTY BUT empty IS FALSE %s\n",baseform);
        }
    else if(*tail && empty)
        printf("TAIL IS NOT EMPTY BUT empty IS TRUE %s\n",baseform);
    basef = new base(baseform,fullWord);
    ++mutated;
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }


node::node(node * next,char * tail,int n,node * sub):m_next(next),basef(0),m_sub(sub)
    {
    this->m_tail = new char[n+1];
    strncpy(this->m_tail,tail,n);
    this->m_tail[n] = '\0';
    m_len = n;
    ++mutated;
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }



base::base(char * baseform,bool fullWord,base * next):m_next(next)
#if defined PROGMAKESUFFIXFLEX
           ,m_fullWord(fullWord),needed(true),added(true)
#endif
    {
    ++nn;
    this->m_baseform = new char[strlen(baseform)+1];
    strcpy(this->m_baseform,baseform);
    this->refcnt = 1;
    ++mutated;
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }


base * base::add(char * baseform,bool fullWord,base *& prev)
    {
    int cmp = strcmp(baseform,this->m_baseform);
    if(!cmp)
        {
#if defined PROGMAKESUFFIXFLEX
        if(fullWord)
            {
            this->m_fullWord = true;
            }
#endif
        incRefCount();
        return this;
        }
    if(cmp > 0)
        {
        prev = new base(baseform,fullWord,this); 
        return prev;
        }
    else if(m_next)
        return m_next->add(baseform,fullWord,m_next);
    else
        {
        m_next = new base(baseform,fullWord);
        return m_next;
        }
    }

//---------------------
base * node::addsub(char * tail,int n,char * baseform,bool fullWord,node *& prev)
    {
    if(tail[n])
        {
        if(this->m_tail[n])
            {
            // insert new tail before this one, move both to subtree 
            if(this->m_tail[n] > tail[n])
                {
                cut(n);
                prev = new node(m_next,tail,n,new node(this,tail+n,baseform,fullWord,false));
                m_next = 0;
                return prev->m_sub->Base();
                }
            else
                {
                cut(n);
                prev = new node(m_next,tail,n,this);
                m_next = new node(0,tail+n,baseform,fullWord,false);
                return m_next->Base();
                }
            }
        else
            {
            if(m_sub)
                return m_sub->add(tail+n,baseform,fullWord,m_sub,false);
            else
                {
                m_sub = new node(0,tail+n,baseform,fullWord,false);
                return m_sub->Base();
                }
            }
        }
    else
        {
        if(this->m_tail[n])
            {
            cut(n);
            prev = new node(m_next,tail,n,baseform,fullWord,false,this);
            m_next = 0;
            return prev->Base();
            }
        else
            {
            if(basef)// exact match, word with more than one baseform
                {
                basef->add(baseform,fullWord,basef);
                }
            else
                {
                basef = new base(baseform,fullWord,0);
                }
            return basef;
            }
        }
    }

type::type(const char * tp,char * tail,char * baseform,bool fullWord,type * next):m_next(next)
    {
    this->m_tp = new char[strlen(tp)+1];
    strcpy(this->m_tp,tp);
    if(*tail)
        end = new node(0,tail,baseform,fullWord,false);
    else
        {
        printf("type::type TYPE %s TAIL 0\n",tp);
        end = new node(0,tail,baseform,fullWord,false);
        }
    ++mutated;
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }

type * type::remove()
    {
    type * ret = m_next;
    m_next = 0;
    delete this;
    return ret;
    }

base * type::add(const char * tp,char * tail,char * baseform,bool fullWord,type *& prev)
    {
    int cmp = strcmp(this->m_tp,tp);
    if(!cmp)
        {
        if(*tail)
            {
            return end->add(tail,baseform,fullWord,end,false);
            }
        else
            {
            return end->add(tail,baseform,fullWord,end,true);
            }
        }
    else if(cmp > 0)
        {
        prev = new type(tp,tail,baseform,fullWord,this);
        return prev->Base();
        }
    else if(m_next)
        return m_next->add(tp,tail,baseform,fullWord,m_next);
    else
        {
        m_next = new type(tp,tail,baseform,fullWord);
        return m_next->Base();
        }
    }

bool type::Baseform(char * invertedWord,const char * tag,base *& bf,int & ln)
    {
    int cmp = strcmp(this->m_tp,tag);
    if(!cmp)
        {
        return end->Baseform(invertedWord,bf,ln);
        }
    else if(cmp < 0 && m_next)
        return m_next->Baseform(invertedWord,tag,bf,ln);
    else
        return false;
    }

/*
void flex::fix()
    {
    if(types)
        types->fix();
    }

void flex::consolidate()
    {
    if(!consolidated && types)
        consolidated = types->consolidate();
    }
*/



flex::~flex()
    {
    delete types;
#ifdef COUNTOBJECTS
    --COUNT;
    extern int LemmaCOUNT, DictNodeCOUNT;
    printf(
        "%d basefrm\n%d "
        "baseformpointer\n%d "
        "dictionary\n%d "
        "field\n%d "
        "base\n%d "
        "node\n%d "
        "type\n%d "
        "flex\n%d "
        "FreqFile\n%d "
        "function\n%d "
        "functionTree\n%d "
        "Lemmatiser\n%d "
        "lext\n%d "
        "Lemma\n%d "
        "DictNode\n%d "
        "optionStruct\n%d "
        "OutputClass\n%d "
        "text\n%d "
        "Word\n",

        
        basefrm::COUNT,
        baseformpointer::COUNT,
        dictionary::COUNT,
        field::COUNT,
        base::COUNT,
        node::COUNT,
        type::COUNT,
        flex::COUNT,
        FreqFile::COUNT,
        function::COUNT,
        functionTree::COUNT,
        Lemmatiser::COUNT,
        lext::COUNT,
        LemmaCOUNT,
        DictNodeCOUNT,
        optionStruct::COUNT,
        OutputClass::COUNT,
        text::COUNT,
        Word::COUNT
        );
    getchar();
#endif
    }

void flex::trim(char * s)
    {
    for(char * t = s + strlen(s) - 1;t >= s && isSpace(*t);--t)
        *t = '\0';
    }

bool flex::Baseform2(char * word,const char * tag,base *& bf,int & offset)
    {
    if(types)
        {
        offset = 0;
        Strrev(word);
        bool ret = types->Baseform(word,tag,bf,offset);
        Strrev(word);
        return ret;
        }
    else
        return false;
    }

base * flex::add(char * line)
    {
    if(strchr(line,' '))
        return 0; // 20080221
    base * ret/* = 0*/;
    static int cnt = 0;
    ++cnt;
    char * tp;
/*    if('A' <= *line && *line <= 'Z')
        { Bart 20050303 
This code is a rudiment from the time when the flex rules still were hand-
written and more guesswork was needed from the program to decide what was
a rule and what was not (a blank line, a comment, or something garbage-like.    
*/
        char * c = line;
        while(*c && !isSpace(*c))
            ++c;
        if(!*c)
            return 0; // line w/o flex rule
        *c = '\0';
        tp = line; // line starts with word class, followed by white space.
        line = c + 1;
/*        }
    else
        {
        tp = N;
        } Bart 20050303 */
    if(!*line)
        return 0; // no flex rule following white space.

    char * end;
    char * basef;
    basef = line;
    if(basef)
        {
        char * close = strchr(basef,'\t');
        if(!close)
            {
            printf("missing '\\t' (TAB) in line %d\n",cnt);
            return 0;
            }
        *close++ = '\0';
        end = close;
        }
    else
        {
        basef = EMPTY;
        end = line;
        }
    trim(tp);
    char * t;
    for(t = end;*t && !isSpace(*t);++t)
        ;
    if(*t)
        *t = '\0'; // This allows for a comment after the word ending, e.g. a count.
    
    trim(basef);
// In fact, we have no way to know whether baseform and end are 
// complete words or not. To stay on the safe side, we assume that
// they are. (fullWord = true)
        {
        Strrev(end);
        if(types)
            {
            ret = types->add(tp,end,basef,true,types);
            }
        else
            {
            types = new type(tp,end,basef,true,0);
            ret = types->Base();
            }
        Strrev(end);
        }
    return ret;
    }

bool flex::readFromFile(FILE * fpflex)
    {
    char s[256];

    while((fgets(s,256,fpflex)) != 0)
        {
        Flex.add(s);
        }
    return true;
    }



