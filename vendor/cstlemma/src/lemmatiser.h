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
#include "defines.h"
#include "dictionary.h"


#if STREAM
# include <iostream>
# ifndef __BORLANDC__
using namespace std;
# endif
#endif



struct optionStruct;
class tagpairs;
struct tallyStruct;
//class istream;
//class std::ostream;

class Lemmatiser
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        int listLemmas;
        bool SortInput; // derived from other options

        static int instance;
        dictionary dict;
        optionStruct & Option;
        int status;
        static tagpairs * TextToDictTags;
        bool changed;
    public:
        // functions to change Option on the fly
        void setIformat(const char * format);            // -I
        void setBformat(const char * format);            // -B
        void setbformat(const char * format);            // -b
        void setcformat(const char * format);            // -c
        void setWformat(const char * format);            // -W
        void setSep(const char * a);
        void setUseLemmaFreqForDisambiguation(int n);    // -H 0, 1 or 2
        void setkeepPunctuation(bool b);
        void setsize(unsigned long int n);
        void settreatSlashAsAlternativesSeparator(bool b);
        void setUseLemmaFreqForDisambiguation(bool b);
        void setDictUnique(bool b);
        void setbaseformsAreLowercase(bool b);

        static const char * translate(const char * tag);
        int getStatus()
            {
            return status;
            }
        Lemmatiser(optionStruct & Option);
        ~Lemmatiser();
        int setFormats();
        int openFiles();
        void showSwitches();
        int MakeDict();
        int MakeFlexPatterns();
#if STREAM
        void LemmatiseText(istream * fpin,ostream * fpout,tallyStruct * tally);
#else
        void LemmatiseText(FILE * fpin,FILE * fpout,tallyStruct * tally);
#endif
        int LemmatiseFile();
        int LemmatiseInit();
        void LemmatiseEnd();
    };
