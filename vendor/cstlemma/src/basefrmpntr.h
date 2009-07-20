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
#ifndef BASEFORMPOINTER_H
#define BASEFORMPOINTER_H

#include "defines.h"
#if STREAM
# include <iostream>
# ifndef __BORLANDC__
using namespace std;
# endif
#else
# include <stdio.h>
#endif

class basefrm;
class functionTree;
class Word;

class baseformpointer
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        bool owning;
        basefrm * bf;
        baseformpointer * next;
        bool hidden;
#if PFRQ
        unsigned int pfrq;
#endif
    public:
        static int UseLemmaFreqForDisambiguation;
        int count()
            {
            return (hidden ? 0 : 1) + (next ? next->count() : 0);
            /*if(next)
                return 1 + next->count();
            else
                return 1;*/
            }
#if STREAM
        void printfbf(ostream *fp,functionTree * fns,const char * sep);
#else
        void printfbf(FILE *fp,functionTree * fns,const char * sep);
#endif
#if PFRQ || FREQ24
        baseformpointer(const char * s,const char * t,int len,/*int cnt,*/unsigned int frequency);
#else
        baseformpointer(const char * s,const char * t,int len);
#endif
        ~baseformpointer();
        void reassign(basefrm * bf);
#if PFRQ || FREQ24
        int addBaseForm(const char * s,const char * t,int len,/*int cnt,*/unsigned int frequency);
#else
        int addBaseForm(const char * s,const char * t,int len);
#endif
        void assignTo(basefrm **& pbf)
            {
            *pbf = bf;
            ++pbf;
            if(next)
                next->assignTo(pbf);
            }
        void addFullForm(Word * word);
        void DissambiguateByLemmaFrequency();
        void DissambiguateByTagFriends(const char * tag);
        void testPrint();
        void decFreq(Word * w);
    };


#endif
