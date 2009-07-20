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
#ifndef TEXT_H
#define TEXT_H

#include "defines.h"
#if STREAM
# include <iostream>
# ifndef __BORLANDC__
using namespace std;
# endif
#else
# include <stdio.h>
#endif

class Word;
class taggedWord;
class basefrm;

struct tallyStruct
    {
    unsigned long int totcnt;
    unsigned long int totcntTypes;
    unsigned long int newcnt;
    unsigned long int newcntTypes;
    unsigned long int newhom;
    unsigned long int newhomTypes;
    tallyStruct()
        {
        totcnt = 0;
        totcntTypes = 0;
        newcnt = 0;
        newcntTypes = 0;
        newhom = 0;
        newhomTypes = 0;
        }
    };

class field;

class taggedText
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
#if TREE
        Word * Root;
#else
        int N;
        Word ** Root;
#endif
        const Word ** tunsorted;
        unsigned long int * Lines;
        bool sorted;
        unsigned long int lineno;
        unsigned long int total;
        unsigned long int reducedtotal;
        basefrm ** basefrmarrD;
        basefrm ** basefrmarrL;
        bool InputHasTags;
        field * fields;
        void AddField(field * fld);
        field * translateFormat(char * Iformat,field *& wordfield,field *& tagfield);
        void insert(const char * w);
        void insert(const char * w, const char * tag);
    public:
        basefrm ** ppD;
        basefrm ** ppL;
        int cntD;
        int cntL;
        int newcnt;
        int newcntTypes;
        int aConflict;
        int aConflictTypes;
        static bool setFormat(const char * format,const char * bformat,const char * Bformat,bool InputHasTags);
#if STREAM
        void Lemmatise(ostream * fpo,/*FILE * fpnew,FILE * fpconflict,*/const char * Sep,
            tallyStruct * tally,
            unsigned int SortOutput,int UseLemmaFreqForDisambiguation,bool nice,bool DictUnique,bool baseformsAreLowercase,int listLemmas);
        taggedText(istream * fpi,bool InputHasTags,char * Iformat,int keepPunctuation,bool nice,unsigned long int size,bool treatSlashAsAlternativeSeparator);
#else
        void Lemmatise(FILE * fpo,/*FILE * fpnew,FILE * fpconflict,*/const char * Sep,
            tallyStruct * tally,
            unsigned int SortOutput,int UseLemmaFreqForDisambiguation,bool nice,bool DictUnique,bool baseformsAreLowercase,int listLemmas);
        taggedText(FILE * fpi,bool InputHasTags,char * Iformat,int keepPunctuation,bool nice,unsigned long int size,bool treatSlashAsAlternativeSeparator);
#endif
        ~taggedText();
        void createUnTaggedAlternatives(
#ifndef CONSTSTRCHR
            const 
#endif
            char * w);
        void createUnTagged(const char * w);
        void createTaggedAlternatives(
#ifndef CONSTSTRCHR
            const 
#endif

            char * w, const char * tag);
        void createTagged(const char * w, const char * tag);
    };


#endif
