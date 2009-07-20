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
#ifndef DICTIONARY_H
#define DICTIONARY_H


#include <stdio.h>
#include "lext.h"

/*
class flex;
class Word;
class taggedWord;
class basefrm;
class taggedWord;
class dictionary;
*/
extern lext * LEXT;

class dictionary
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        static void readStrings(FILE * fp);
        static void readLeaves(FILE * fp);
        static tcount readStretch(tchildren length,tcount pos,FILE * fp);
        static void readNodes(FILE * fp);
        static void cleanup();
        
        static void printlex(long pos, FILE * fp);
        static void printlex2(char * head,long pos, FILE * fp);
        static void printnode(int indent, long pos, FILE * fp);
        static void printnode2(char * head,long pos, FILE * fp);
        static bool findwordSub(const char * word,tcount & Pos,int & Nmbr);
    public:
        static bool findword(const char * word,tcount & Pos,int & Nmbr);
        void initdict(FILE * fpin);
        dictionary();
        ~dictionary();
        void printall(FILE * fp);
        void printall2(FILE * fp);
    };


#endif
