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
#include "word.h"
#include "basefrm.h"
#include "functio.h"
#include "functiontree.h"
#include "flex.h"
#include "lemmtags.h"
#include "tags.h"
#include "caseconv.h"
#include "lemmatiser.h"
#include "text.h"
#include "dictionary.h"
#include <assert.h>
#include <stdlib.h>
#include <limits.h>

#ifdef COUNTOBJECTS
int Word::COUNT = 0;
#endif

Word * Word::Root = 0;



#if STREAM
ostream * Word::fp = 0;
#else
FILE * Word::fp = 0;
#endif
functionTree * Word::funcs = 0;
functionTree * Word::bfuncs = 0;
functionTree * Word::Bfuncs = 0;
bool Word::hasb = false;
bool Word::hasB = false;
const char * Word::sep;
bool Word::DictUnique = true;
int Word::NewLinesAfterWord = 0;
int Word::LineNumber = 1;
cmp_f Word::cmp = &Word::cmpword;
cmp_ft taggedWord::comp = &taggedWord::cmptaggedword;

/*
The next function makes an >>uneducated<< guess at the type of the word.
Only used for untagged texts.
We could use several techniques to improve: return the type that is
most probable, given the word's ending (then we would need to have a most
probable type for the case that NO rule aplies, 
i.e. the unkown word is in base form already.)
*/
const char * baseform(char * word,const char ** tag /*return value!*/)
    { // construct baseform by applying general rules (e.g. removing endings)
    const char * wrd;
    int borrow;
    assert(tag);
    *tag = Flex.Baseform(word,wrd,borrow);
    if(*tag)
        return wrd;
    else
        return allToLower(word); 
    }


const char * baseform(char * word,const char * tag)
    { // construct baseform by applying general rules (e.g. removing endings)
    const char * wrd;
    int borrow;
    if(Flex.Baseform(word,tag,wrd,borrow))
        return wrd;
    else if(flex::baseformsAreLowercase)
        return allToLower(word); 
    else
        return /*Bart 20021001 allToLower*/(word); 
    }



#if STREAM
void Word::setFile(ostream * a_fp)
#else
void Word::setFile(FILE * a_fp)
#endif
    {
    Word::fp = a_fp;
    basefrm::setFile(a_fp);
    }

void Word::deleteStaticMembers()
    {
    delete funcs;funcs = 0;
    delete bfuncs;bfuncs = 0;
    delete Bfuncs;Bfuncs = 0;
    }


function * Word::getUnTaggedWordFunction(int character,bool & SortInput,int & testType)
    {
    switch(character)
        {
        case 'i':
            return new functionNoArg(&Word::i,0);
        case 'f':
            SortInput = true;
            return new functionNoArg(&Word::f,0);
        case 'w':
            return new functionNoArg(&Word::w,0);
        case 'b':
            hasb = true;
            testType |= NUMBERTEST;
            return new functionNoArg(&Word::b,&Word::countBaseForms);
        case 'B':
            hasB = true;
            testType |= NUMBERTEST;
            return new functionNoArg(&Word::B,&Word::countBaseFormsL);
        case 's':
            return new functionNoArg(&Word::s,0);
        default:
            return 0;
        }
    }

function * Word::getUnTaggedWordFunctionNoBb(int character,bool & SortInput,int & testType)
    {
    switch(character)
        {
        case 'i':
            return new functionNoArg(&Word::i,0);
        case 'f':
            SortInput = true;
            return new functionNoArg(&Word::f,0);
        case 'w':
            return new functionNoArg(&Word::w,0);
        case 's':
            return new functionNoArg(&Word::s,0);
        default:
            return 0;
        }
    }

void Word::print()const
    {
    funcs->printIt(this);
    }

void Word::printLemmaClass()const
    {
    if(pbfD)
        pbfD->printFn(fp,&basefrm::T,sep);
    else if(pbfL)
        pbfL->printFn(fp,&basefrm::T,sep);
    }

int Word::addBaseFormsL()
    {
    const char * tag = 0;
    const char * wrd = baseform(m_word,&tag);
    if(!tag)
        tag = NOT_KNOWN;// TODO do something better (NUM, XX, TEGN, etc)
//        return 0;
    if(!*wrd)
        wrd = allToLower(m_word);
    return addBaseFormL(wrd,LemmaTag(tag));
    }

#if PFRQ || FREQ24
int Word::addBaseFormL(const char * s,const char * t)
    {
    int cntL = 0;
    const char * l_sep;
    while((l_sep = strchr(s,' ')) != 0)
        {
        //       ++cntL;
        if(pbfL)
            cntL += pbfL->addBaseForm(s,t,l_sep - s,/*1,*/0/*freq*/);
        else
            {
            pbfL = new baseformpointer(s,t,l_sep - s,/*1,*/0/*freq*/);
            ++cntL;
            }
        s = l_sep + 1;
        }
    //++cntL;
    if(pbfL)
        cntL += pbfL->addBaseForm(s,t,strlen(s),/*1,*/0/*freq*/);
    else
        {
        pbfL = new baseformpointer(s,t,strlen(s),/*1,*/0/*freq*/);
        ++cntL;
        }
    return cntL;
    }
#else
int Word::addBaseFormL(const char * s,const char * t)
    {
    int cntL = 0;
    const char * l_sep;
    while((l_sep = strchr(s,' ')) != 0)
        {
        //       ++cntL;
        if(pbfL)
            cntL += pbfL->addBaseForm(s,t,l_sep - s);
        else
            {
            pbfL = new baseformpointer(s,t,l_sep - s);
            ++cntL;
            }
        s = l_sep + 1;
        }
    //++cntL;
    if(pbfL)
        cntL += pbfL->addBaseForm(s,t,strlen(s));
    else
        {
        pbfL = new baseformpointer(s,t,strlen(s));
        ++cntL;
        }
    return cntL;
    }
#endif

int Word::addBaseFormsDL(lext * Plext,int nmbr,// The dictionary's available
                                 // lexical information for this word.
                                 bool & ,int & cntD,int & )//
    {
    int n;
#define WRIT 1
#if WRIT
    int written = 0;// Bart 20030206 INT_MIN;
#endif
    unsigned int maxFreq = maxFrequency(Plext,nmbr,0,n);
    if(n > 1)
        {
        const char * tp = commonType(Plext,nmbr,maxFreq);
        unsigned int off;
        char * stem = commonStem(Plext,nmbr,tp,maxFreq,off);
        if(tp)
            {
            /*
            unsigned int off;
            char * stem = commonStem(Plext,nmbr,tp,maxFreq,off);
            */
            if(stem)
                {
                /*
                static char buf[256];
                const char * l_w;
                char * pbuf = buf;
                for(l_w = word;*l_w && off;--off)
                    {
                    *pbuf++ = Lower(*l_w);
                    l_w++;
                    }
                for(l_w = stem;*l_w;)
                    {
                    *pbuf++ = *l_w++;
                    }
                *pbuf = '\0';*/
#if PFRQ || FREQ24
                cntD += addBaseFormD(stem/*buf*/,/**/LemmaTag/**/(tp)/*tp*/,/*1,*/maxFreq);
#else
                cntD += addBaseFormD(stem/*buf*/,/**/LemmaTag/**/(tp)/*tp*/);
#endif
                FoundInDict = true;
                return cnt;
                }
            }
        else
            {
            if(stem && !strcmp(stem,Plext->constructBaseform(m_word))) 
                // Bart 20021105: if all baseforms are the same, then use that common baseform.
                {
#if PFRQ || FREQ24
                cntD += addBaseFormD(stem,NOT_KNOWN,0);
#else
                cntD += addBaseFormD(stem,NOT_KNOWN);
#endif
/*            
                cntD += addBaseFormD(allToLower(word),NOT_KNOWN,0);*/
                FoundInDict = true;
                return cnt;
                }
            else
                {
#if WRIT
                written = 0; // force flex rule application
#else
                cntD += addBaseFormD(allToLower(word),NOT_KNOWN,0);
                FoundInDict = true;
                return cnt;
#endif
                }
            }
        }

//    printf("word %s\n",word);
    lext * plext;
    // Lemmatiser adds tag from dictionary
    plext = Plext;
    while(true)
        {
        if(plext->S.frequency >= maxFreq)
            {
#if PFRQ || FREQ24
            cntD += addBaseFormD(plext->constructBaseform(m_word),/**/LemmaTag/**/(plext->Type),/*1,*/plext->S.frequency);
#else
            cntD += addBaseFormD(plext->constructBaseform(m_word),/**/LemmaTag/**/(plext->Type));
#endif
            FoundInDict = true;
#if WRIT
            written++;
#endif
            }
        // write the remainder of the baseform and the tag, as
        // found in the dictionary
        
        if(--nmbr)
            {
            ++plext;
            }
        else
            break;
        }
#if WRIT
    if(written > 1)
        {
        /*cntL +=*/ addBaseFormsL(); // Bart 20021105
        }
#endif
    return cnt;
    }


bool Word::setFormat(const char * cformat,const char * bformat,const char * Bformat,bool InputHasTags)
    {
    bool SortInput = false;
    getFunction gfnc = InputHasTags ? taggedWord::getTaggedWordFunction : Word::getUnTaggedWordFunction;
    if(funcs)
        delete funcs;
    funcs = new functionTree();
    int testType = 0;
    OutputClass::Format(cformat,gfnc,*funcs,cformat,SortInput,testType);
    if(hasb)
        {
        if(bformat)
            {
            bfuncs = basefrm::Format(bformat);
            }
        else
            {
            printf("Error: Missing -b pattern on commandline. (Output format %s specifies dictionary field $b)\n",cformat);
            exit(0);
            }
        }
    else if(bformat)
        {
        printf("Warning: -b pattern \"%s\"specified on commandline but not used. (Output format %s doesn't specify dictionary field $b)\n",bformat,cformat);
        }

    if(hasB)
        {
        if(Bformat)
            {
            Bfuncs = basefrm::Format(Bformat);
            }
        else
            {
            printf("Error: Missing -B pattern on commandline. (Output format %s specifies flexrule field $B)\n",cformat);
            exit(0);
            }
        }
    else if(bformat)
        {
        printf("Warning: -B pattern specified on commandline but not used. (Output format %s doesn't specify flexrule field $B)\n",cformat);
        }
    return SortInput;
    }

unsigned int Word::maxFrequency(lext * Plext,int nmbr,const char * a_type,int & n)// The dictionary's available
                               // lexical information for this word.
    {
    n = 1;
    if(!Word::DictUnique)
        return 0;
    if(nmbr < 2)
        return 0;
    unsigned int maxfreq = 0;
    //int maxi = -1;
    for(int j = 0;j < nmbr;++j)
        {
        if(!a_type || !strcmp(a_type,LemmaTag(Plext[j].Type)))
            {
            if(Plext[j].S.frequency > maxfreq)
                {
                n = 1;
                maxfreq = Plext[j].S.frequency;
        //            maxi = j;
                }
            else if(Plext[j].S.frequency == maxfreq)
                ++n;
            }
        }
    return maxfreq;//maxi;
    }

/*
Find the common stem by disregarding cardinal numbers, i.e. abcd,1 and abcd,2 have common stem abcd.
*/
char * Word::commonStem(lext * Plext,int nmbr,const char * a_type,unsigned int freq,unsigned int & off)
    {
    if(!Word::DictUnique)
        return 0;
    if(nmbr < 2)
        return 0;
    static char buf[256];
/*    for(int f = 0;f < 256;++f)
        buf[f] = '\0';*/
    buf[0] = '\0';
    off = 0;
    char * ret = 0;
    int ii;
    for(ii = 0;ii < nmbr;++ii)
        {
        if(freq == Plext[ii].S.frequency && (!a_type /*Bart 20021105*/ || !strcmp(a_type,/**/LemmaTag/**/(Plext[ii].Type))))
            {
            if(ret && off != Plext[ii].S.Offset)
                {
                return 0;
                }
            char * bf = Plext[ii].BaseFormSuffix;
            char * komma = strchr(bf,',');
            if(komma)
                {
                if(buf[0])
                    {
                    if(strncmp(buf,bf,komma-bf))
                        {
                        return 0;
                        }
                    }
                else
                    {
                    strncpy(buf,bf,komma-bf);
                    buf[komma-bf] = '\0';
                    ret = buf;
                    off = Plext[ii].S.Offset;
                    }
                }
            else
                {
                if(buf[0])
                    {
                    if(strcmp(buf,bf))
                        {
                        return 0;
                        }
                    }
                else
                    {
                    strcpy(buf,bf);
                    ret = buf;
                    off = Plext[ii].S.Offset;
                    }
                }
            }
        }
    
    ii = strlen(buf);
    unsigned int j = ii + off;
    buf[j] = '\0';
    while(j > off)
        {
        --j;
        buf[j] = buf[j - off];
        }
    const char * l_w;
    char * pbuf = buf;
    for(l_w = m_word;*l_w && off;--off)
        {
        *pbuf++ = (char)Lower(*l_w);
        l_w++;
        }
/*    for(l_w = stem;*l_w;)
        {
        *pbuf++ = *l_w++;
        }
    *pbuf = '\0';*/
    return buf;
    }


char * Word::commonType(lext * Plext,int nmbr,unsigned int freq)
    {
    if(!Word::DictUnique)
        return 0;
    if(nmbr < 2)
        return 0;
    static char buf[256];
    buf[0] = '\0';
    char * ret = 0;
    for(int ii = 0;ii < nmbr;++ii)
        {
        if(freq == Plext[ii].S.frequency)
            {
            const char * t = /**/LemmaTag/**/(Plext[ii].Type);
            if(buf[0])
                {
                if(strcmp(buf,t))
                    {
                    return 0;
                    }
                }
            else
                {
                strcpy(buf,t);
                ret = buf;
                }
            }
        }
    return ret;
    }

function * taggedWord::getTaggedWordFunction(int character,bool & SortInput,int & testType)
    {
    switch(character)
        {
        case 't':
            return new functionNoArgT(&taggedWord::t);
        }
    return Word::getUnTaggedWordFunction(character,SortInput,testType);
    }

function * taggedWord::getTaggedWordFunctionNoBb(int character,bool & SortInput,int & testType)
    {
    switch(character)
        {
        case 't':
            return new functionNoArgT(&taggedWord::t);
        }
    return Word::getUnTaggedWordFunctionNoBb(character,SortInput,testType);
    }

int taggedWord::addBaseFormsL()
    {
    const char *ttag = Lemmatiser::translate(m_tag); // Bart 20030910 hurtig hack
    const char * wrd = baseform(m_word,ttag);
    return addBaseFormL(wrd,LemmaTag(ttag /*eller tag?*/));
    }


int taggedWord::addBaseFormsDL(lext * Plext,int nmbr,// The dictionary's available
                               // lexical information for this word.
                               bool & conflict,int & cntD,int & cntL)//
    {
    lext * plext;
    //   int cnt = 0;
    const char * Tp = Lemmatiser::translate(m_tag); // tag as found in the text
/*    if(TextToDictTags)
        tag = TextToDictTags->translate(tag);*/  // TODO Bart 20030910 Skal tag-member erstattes med translate(tag)?
    /*
    for(int ii = 0;ii < tagcnt;++ii)
        {
        if(!strcmp(tag,textTags[ii]))
            {
            Tp = dictTags[ii]; // translate tag to dictionary-a_type
            break;
            }
        }
        */
    // See whether the word's tag can be found in the
    // dictionary's lexical information.
    int written = 0;
    
    plext = Plext;
    int m;

    const char * baseTp = /**/LemmaTag/**/(Tp);

    unsigned int maxFreq = maxFrequency(Plext,nmbr,baseTp/*Tp*/,m);
    if(m > 1)
        {
        unsigned int off;
        char * stem = commonStem(Plext,nmbr,baseTp/*Tp*/,maxFreq,off);
        if(stem)
            {/*
            static char buf[256];
            const char * l_w;
            char * pbuf = buf;
            for(l_w = word;*l_w && off;--off)
                {
                *pbuf++ = Lower(*l_w);
                l_w++;
                }
            for(l_w = stem;*l_w;)
                {
                *pbuf++ = *l_w++;
                }
            *pbuf = '\0';*/
//            printf("word %s stem %s maxFreq %d baseTp %s\n",word,stem,maxFreq,baseTp);
#if PFRQ || FREQ24
            cntD += addBaseFormD(stem/*buf*/,baseTp/*LemmaTag(Tp)*/,/*1,*/maxFreq);
#else
            cntD += addBaseFormD(stem/*buf*/,baseTp/*LemmaTag(Tp)*/);
#endif
            FoundInDict = true;
            return cnt;
            }
        }

//    const char * baseTp = /**/LemmaTag/**/(Tp);
    for(int n = nmbr;n;--n,++plext)
        {
        if(plext->S.frequency >= maxFreq)
            {
#if 0
            // This test has the advantage that it is less sensitive for
            // POS tag errors in the input. On the other hand, it also
            // introduces too many lemmas, for example "skalle" in
            // 522 skalle/V (522 skal/V_PRES)
            // 522 skulle/V (522 skal/V_PRES)
            // In reality, only if "skal" has POS tag V_IMP the lemma
            // "skalle" is correct.

            if(!strcmp(baseTp/*Tp*/,/**/LemmaTag/**/(plext->Type))) // Word is in dictionary,
#else
            if(!strcmp(/*baseTp*/Tp,/*LemmaTag*/(plext->Type))) // Word is in dictionary,
#endif
                // and type info matches.
                {
/*            printf("word %s constructed %s freq %d maxFreq %d baseTp %s\n",
                word,plext->constructBaseform(word),plext->S.frequency,maxFreq,baseTp);*/
#if PFRQ || FREQ24
                cntD += addBaseFormD(plext->constructBaseform(m_word),baseTp,/*1,*/plext->S.frequency);
#else
                cntD += addBaseFormD(plext->constructBaseform(m_word),baseTp);
#endif
                ++written;
                FoundInDict = true;
                }
            }
        //There is a problem with the statistics if more than one lemma is added!
        }
    
    if(written > 1)
        /*cntL +=*/ addBaseFormsL(); // Let the rules decide if the word is a 
        // homograph, with more than one readings having the correct lexical 
        // type. First try other disambiguation tricks (frequency).
    else if(!written)
        {
        conflict = true;
        // There is a conflict between the tagger and the dictionary. 
        // The word is in the dictionary, but not with the lexical type assigned by the tagger.
        cntL += addBaseFormsL();
        
        if(nmbr)
            {
            maxFreq = maxFrequency(Plext,nmbr,0,m);
            if(m > 1)
                {
                char * tp = commonType(Plext,nmbr,maxFreq);
                if(tp)
                    {
                    unsigned int off;
                    char * stem = commonStem(Plext,nmbr,tp,maxFreq,off);
                    if(stem)
                        {/*
                        static char buf[256];
                        const char * l_w;
                        char * pbuf = buf;
                        for(l_w = word;*l_w && off;--off)
                            {
                            *pbuf++ = *l_w++;
                            }
                        for(l_w = stem;*l_w;)
                            {
                            *pbuf++ = *l_w++;
                            }
                        *pbuf = '\0';*/
#if PFRQ || FREQ24
                        addBaseFormD(stem/*buf*/,/*LemmaTag*/(tp),/*1,*/maxFreq);
#else
                        addBaseFormD(stem/*buf*/,/*LemmaTag*/(tp));
#endif
/*
                        printf("stem %s tp %s baseTp %s\n",stem,tp,baseTp);
stem fire tp V baseTp N
stem jens tp N baseTp EGEN
stem tabe tp V baseTp ADJ
stem stege tp V baseTp ADJ
stem m�tte tp V baseTp ADJ
stem ende tp V baseTp ADJ
*/
                        return cnt;
                        }
                    }
                }
            plext = Plext;
            while(true)
                {
                if(plext->S.frequency >= maxFreq)
#if PFRQ || FREQ24
                    /*cntD +=*/ addBaseFormD(plext->constructBaseform(m_word),/**/LemmaTag/**/(plext->Type),/*0,*/plext->S.frequency);
#else
                    /*cntD +=*/ addBaseFormD(plext->constructBaseform(m_word),/**/LemmaTag/**/(plext->Type));
#endif
                    // We choose not to count the dictionary lemmas if the constructed lemma already is counted on.
                    if(--nmbr)
                        ++plext;
                    else
                        break;
                }
            }
        }
    return cnt;
    }

void Word::lookup(text * txt)
    {
    bool conflict = false;
    tcount Pos;
    int Nmbr;
    if(dictionary::findword(itsWord(),Pos,Nmbr))
        {
        addBaseFormsDL(LEXT + Pos,Nmbr,conflict,txt->cntD,txt->cntL);
        //            files.write(LEXT + Pos,Nmbr,TaggedWords[k]->word,TaggedWords[k]->tag,conflict);
        if(conflict)
            {
            txt->aConflictTypes++;
            txt->aConflict += itsCnt();
            }
        }
    else
        {
        txt->newcntTypes++;
        txt->newcnt += itsCnt();
        txt->cntL += addBaseFormsL();
        //            files.writeWordAndTag(TaggedWords[k]->word,TaggedWords[k]->tag,text.Sorted() ? TaggedWords[k]->cnt:-1);
        }
    if(basefrm::hasW)
        addFullForm();
    }


int Word::reducedtotal = 0;
