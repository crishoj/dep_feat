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

class FreqFile;
typedef enum {MAKEDICT,MAKEFLEXPATTERNS,LEMMATISE} whattodoTp;
typedef enum {GoOn = 0,Leave = 1,Error = 2} OptReturnTp;

#if defined _WIN32
#define commandlineQuote "\""
#else
#define commandlineQuote "\'"
#endif

struct optionStruct
    {
#ifdef COUNTOBJECTS
    public:
    static int COUNT;
#endif
    static const char DefaultSep[]; // -s
    static const char DefaultCFormat[]; // -c
    static const char DefaultCFormat_NoTags[]; // -c
    static const char Default_b_format[]; // -b
    static const char * Default_B_format; // -B

    // program task
    whattodoTp whattodo; // -D, -F, -L

    // -D: Make dictionary
    bool CollapseHomographs; // -k makedict
    FreqFile * freq; // -n, -N makedict

    // -L
    // linguistic resources
    const char * dictfile;  // -d
    const char * flx;       // -f
    const char * v;         // -v
    const char * x;         // -x
    const char * z;         // -z

    // pre-run rule treatment
    bool RulesUnique; // -U removeAmbiguous

    // input text info
    bool InputHasTags;                      // -t taggedText::taggedText
    char * Iformat;                         // -I taggedText::taggedText
    int keepPunctuation;                    // -p taggedText::taggedText
    bool nice;                              // -y makedict, taggedText::taggedText, taggedText::Lemmatise
    unsigned long int size;                 // -m taggedText::taggedText
    bool treatSlashAsAlternativesSeparator; // -A taggedText::taggedText

    // input and output
    const char * argi; // -i
    const char * argo; // -o
    const char * arge; // -e

    // output format
    char * cformat; // -c (also option for -D and -F tasks)
    char * Wformat; // -W
    char * bformat; // -b
    char * Bformat; // -B

    // output switches
    char * Sep;                             // -s  taggedText::Lemmatise
    unsigned int SortOutput;                // -q  taggedText::Lemmatise
//    unsigned int SortFreq;                // -q#wp taggedText::Lemmatise
    int UseLemmaFreqForDisambiguation;      // -H taggedText::Lemmatise
    bool DictUnique;                        // -u taggedText::Lemmatise
    bool baseformsAreLowercase;             // -l taggedText::Lemmatise


    bool defaultbformat;
    bool defaultBformat;
    bool defaultCformat;

    long CutoffRefcount;
    bool showRefcount;

    bool Defaultbformat()
        {
        return defaultbformat;
        }
    bool DefaultBformat()
        {
        return defaultBformat;
        }
    bool DefaultCformat()
        {
        return defaultCformat;
        }
    optionStruct();
    ~optionStruct();
    OptReturnTp doSwitch(int c,char * locoptarg,char * progname);
    OptReturnTp readOptsFromFile(char * locoptarg,char * progname);
    OptReturnTp readArgs(int argc, char * argv[]);
    void setIformat(const char * format);            // -I
    void setBformat(const char * format);            // -B
    void setbformat(const char * format);            // -b
    void setcformat(const char * format);            // -c
    void setWformat(const char * format);            // -W
    void setSep(const char * format);                // -s
    void setUseLemmaFreqForDisambiguation(int n);    // -H 0, 1 or 2
    void setkeepPunctuation(bool b);
    void setsize(unsigned long int n);
    void settreatSlashAsAlternativesSeparator(bool b);
    void setUseLemmaFreqForDisambiguation(bool b);
    void setDictUnique(bool b);
    void setbaseformsAreLowercase(bool b);
    };
