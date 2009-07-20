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
#include "field.h"
#include "text.h"
#include "word.h"
#include "dictionary.h"
#include "basefrm.h"
#include "caseconv.h"
#include "flex.h"
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "hash.h"

#if !TREE
hash<Word> * Hash = 0;
#endif

#ifdef COUNTOBJECTS
int taggedText::COUNT = 0;
#endif


static int cmpBaseforms_twf(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmpt(n2);
    if(ret)
        return ret;
    ret = n1->cmps(n2);
    return ret;
    }

static int cmpBaseforms_w(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmps(n2);
    return ret;
    }

static int cmpBaseforms_wf(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmps(n2);
    if(ret)
        return ret;
    ret = n1->cmpf(n2);
    return ret;
    }

static int cmpBaseforms_ftw(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmpf(n2);
    if(ret)
        return ret;
    ret = n1->cmpt(n2);
    if(ret)
        return ret;
    ret = n1->cmps(n2);
    return ret;
    }

static int cmpBaseforms_fwt(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmpf(n2);
    if(ret)
        return ret;
    ret = n1->cmps(n2);
    if(ret)
        return ret;
    ret = n1->cmpt(n2);
    return ret;
    }

static int cmpBaseforms_tfw(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmpt(n2);
    if(ret)
        return ret;
    ret = n1->cmpf(n2);
    if(ret)
        return ret;
    ret = n1->cmps(n2);
    return ret;
    }

static int cmpBaseforms_wft(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmps(n2);
    if(ret)
        return ret;
    ret = n1->cmpf(n2);
    if(ret)
        return ret;
    ret = n1->cmpt(n2);
    return ret;
    }

static int cmpBaseforms_wtf(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmps(n2);
    if(ret)
        return ret;
    ret = n1->cmpt(n2);
    if(ret)
        return ret;
    ret = n1->cmpf(n2);
    return ret;
    }

static int cmpBaseforms_wt(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmps(n2);
    if(ret)
        return ret;
    ret = n1->cmpt(n2);
    return ret;
    }

static int cmpBaseforms_fw(const basefrm * n1,const basefrm * n2)
    {
    int ret = n1->cmpf(n2);
    if(ret)
        return ret;
    ret = n1->cmps(n2);
    return ret;
    }

static int (*pcmpBaseforms)  (const basefrm * elem1, const basefrm * elem2) = cmpBaseforms_w;
static int (*pcmpBaseforms_f)(const basefrm * elem1, const basefrm * elem2) = cmpBaseforms_fw;

static int compareBaseforms( const void *arg1, const void *arg2 )
    {
    const basefrm * n1 = *(const basefrm * const *)arg1;
    const basefrm * n2 = *(const basefrm * const *)arg2;
    return pcmpBaseforms(n1,n2);
    }

static int compareBaseforms_f( const void *arg1, const void *arg2 )
    {

    const basefrm * n1 = *(const basefrm * const *)arg1;
    const basefrm * n2 = *(const basefrm * const *)arg2;
    if(!n1)
        if(n2)
            return 1;
        else
            return 0;
    else if(!n2)
        return -1;
    return pcmpBaseforms_f(n1,n2);
    }

static int sortBaseforms(basefrm ** pbf,int cnt)
    {
    qsort((void *)pbf,cnt,sizeof(basefrm *),compareBaseforms);
    int i = 0;
    int j = 1;
    int k = 0;
//    int deleted = 0;
    while(j < cnt)
        {
        if(!pcmpBaseforms(pbf[i],pbf[j])) // 20050826 Error: pcmpBaseforms was cmpBaseforms_w
            {
            pbf[j]->getAbsorbedBy(pbf[i]);
            pbf[j] = 0;
//            ++deleted;
            }
        else
            {
            if(k != i)
                {
                pbf[k] = pbf[i];
                pbf[i] = 0;
                }
            i = j;
            ++k;
            }
        ++j;
        }
    if(k != i)
        {
        pbf[k] = pbf[i];
        pbf[i] = 0;
        }
//    assert(k+1 == cnt - deleted);
    return k+1;
    }

static void sortBaseforms_f(basefrm ** pbf,int cnt)
    {
    qsort((void *)pbf,cnt,sizeof(basefrm *),compareBaseforms_f);
    }


void taggedText::AddField(field * fld)
    {
    if(fields == 0)
        fields = fld;
    else
        fields->addField(fld);
    }

static char * globIformat = 0;
field * taggedText::translateFormat(char * Iformat,field *& wordfield,field *& tagfield)
    {
    globIformat = Iformat;
    bool escape = false;
    bool afield = false;
    char * pformat;
    field * litteral = 0;
    for(pformat = Iformat;*pformat;++pformat)
        {
        if(afield)
            {
            afield = false;
            switch(*pformat)
                {
                case 'w':
                    if(wordfield)
                        {
                        printf("Invalid format string \"%s\"\n",Iformat);
                        printf("                        %*c\n",(int)(strlen(Iformat) - strlen(pformat)),'^');
                        exit(0);
                        }
                    wordfield = new readValue();
                    litteral = 0;
                    AddField(wordfield);
                    break;
                case 't':
                    if(tagfield)
                        {
                        printf("Invalid format string \"%s\"\n",Iformat);
                        printf("                        %*c\n",(int)(strlen(Iformat) - strlen(pformat)),'^');
                        exit(0);
                        }
                    tagfield = new readValue();
                    litteral = 0;
                    AddField(tagfield);
                    break;
                case 'd':
                    litteral = 0;
                    AddField(new readValue());
                    break;
                default:
                    {
                    printf("Invalid format string \"%s\"\n",Iformat);
                    printf("                        %*c\n",(int)(strlen(Iformat) - strlen(pformat)),'^');
                    exit(0);
                    }
                }
            }
        else if(escape)
            {
            escape = false;
            switch(*pformat)
                {
                case 's':
                    litteral = 0;
                    AddField(new readWhiteSpace);
                    break;
                case 'S':
                    litteral = 0;
                    AddField(new readAllButWhiteSpace);
                    break;
                case 't':
                    litteral = 0;
                    AddField(new readTab);
                    break;
                case 'n':
                    litteral = 0;
                    AddField(new readNewLine);
                    break;
                default:
                    {
                    printf("Invalid format string \"%s\"\n",Iformat);
                    printf("                        %*c\n",(int)(strlen(Iformat) - strlen(pformat)),'^');
                    exit(0);
                    }
                }
            }
        else if(*pformat == '\\')
            {
            escape = true;
            }
        else if(*pformat == '$')
            {
            afield = true;
            }
        else
            {
            if(!litteral)
                {
                litteral = new readLitteral(*pformat);
                AddField(litteral);
                }
            else
                litteral->add(*pformat);
            }
        }
    return fields;
    }

static int sanityCheck(int slashFound,const char * buf)
    {
    if(!slashFound)
        return 0;
    if(*buf == '/')
        return 0; // list of alternatives does not start with "/"
    const char * p = buf;
    while(  is_Alpha(*p) // list of alternatives does only have alphabetic characters
         ||    *p == '/' 
            && *(p - 1) != '/' // list of alternatives does not contain "//"
         ) 
        ++p;
    if(*p)
        return 0;
    if(*(p - 1) == '/')
        return 0; // list of alternatives does not end with "/"
    return slashFound;
    }

int findSlashes(const char * buf)
    {
    if(!*buf || *buf == '/')
        return 0;
    const char * p = buf;
    int ret = 0;
    while((p = strchr(p,'/')) != 0)
        {
        ++p;
        if(*p == '/')
            return 0;
        ++ret;
        }
    if(ret && buf[strlen(buf + 1)] == '/')
        return 0;
    return ret;
    }

#if STREAM
static 
#ifndef CONSTSTRCHR
const 
#endif
char * getwordI(istream * fpin,const char *& tag,field * format,field * wordfield,field * tagfield,unsigned long & lineno)
#else
static 
#ifndef CONSTSTRCHR
const 
#endif
char * getwordI(FILE * fpin,const char *& tag,field * format,field * wordfield,field * tagfield,unsigned long & lineno)
#endif
    {
    format->reset();
    assert(wordfield);
    int kar = EOF;
    char kars[2];
    kars[1] = '\0';
    char line[256];
    static char lastkar = '\0';
    int i = 0;
    field * nextfield = format;
    if(lastkar)
        {
        if(lastkar == '\n')
            ++lineno;
        kars[0] = (char)lastkar;
        if(i == sizeof(line) - 1)
            {
            printf("Unable to apply input format specification to this text.\n");
            printf("Input format:'%s'\n",globIformat);
            printf("Text:'%s...'\n",line);
            printf("Field:%s\n",nextfield->isA());
            printf("Line:%ld\n",lineno);
            printf("Last:[%s]\n",line);
            exit(0);
            }
        line[i++] = (char)lastkar;
        line[i] = '\0';
        nextfield->read(kars,nextfield);
        }
#if STREAM
    while(nextfield && (kar = fpin->get()) != 0 && !fpin->eof())
#else
    while(nextfield && (kar = fgetc(fpin)) != EOF)
#endif
        {
        if(kar == '\n')
            ++lineno;
        kars[0] = (char)kar;
        if(i == sizeof(line) - 1)
            {
            printf("Unable to apply input format specification to this text.\n");
            printf("Input format:'%s'\n",globIformat);
            printf("Text:'%s...'\n",line);
            printf("Field:%s\n",nextfield->isA());
            printf("Line:%ld\n",lineno);
            printf("Last:[%s]\n",line);
            exit(0);
            }
        line[i++] = (char)kar;
        line[i] = '\0';
        char * plastkar = nextfield->read(kars,nextfield);
        if(plastkar)
            lastkar = *plastkar;
        }
    if(kar == EOF)
        {
        tag = 0;
        format->reset();
        return 0;
        }
    if(tagfield)
        tag = tagfield->getString();
//    format->reset();
    return wordfield->getString();
    }

#if STREAM
static char * getword(istream * fp,const char *& tag,bool InputHasTags,int keepPunctuation, int & slashFound, unsigned long & lineno)
#else
static char * getword(FILE * fp,const char *& tag,bool InputHasTags,int keepPunctuation, int & slashFound, unsigned long & lineno)
#endif
// lineno is incremented when the current word is followed by a new line \n
    {
    static int punct = 0;
    static char buf[1000];
    static char buf2[256]; // tag
    static int eof = false;
    static int prevkar = 0;
    slashFound = 0;
    if(punct)
        {
        buf[0] = (char)punct;
        buf[1] = '\0';
        punct = 0;
        return buf;
        }
    int kar;
    char *p;
    p = buf;
    tag = 0;
    if(eof)
        {
        eof = false;
        return 0;
        }
    while(true)
        {
        if(prevkar)
            {
            kar = prevkar;
            prevkar = 0;
            }
        else
            {
#if STREAM
            kar = fp->get();
            if(fp->eof())
#else
            kar = fgetc(fp);
            if(kar == EOF)
#endif
                {
                *p = '\0';
                eof = true;
                slashFound = sanityCheck(slashFound,buf);
                return buf;
                }
            }
        if(InputHasTags)
            {
            if(kar == '/')
                {// tag follows
			    // TODO: How is a slash in the original text tagged? Answer: as a slash
                char * slash = p; // slash points at / in buf (the word)
                *p = '\0';        // the / is replaced by zero
                p = buf2;         // p points at the start of the tag buffer
                tag = buf2;
                while(true)
                    {
#if STREAM
                    kar = fp->get();
                    if(fp->eof())
#else
                    kar = fgetc(fp);
                    if(kar == EOF)
#endif
                        {
                        *p = '\0';
                        eof = true;
                        break;
                        }
                    if(isSpace(kar))
                        {
                        if(kar == '\n')
                            ++lineno;
                        // Bart 20050823 We need to look for new line after the blank. If we wait, the first word of the next line will become the last word of the current line.
                        // If there is no new line character, the first character of the next word will be read.
                        // Put this in a safe place: prevkar.
                        else 
                            {
                            do
                                {
#if STREAM
                                kar = fp->get();
                                if(fp->eof())
#else
                                kar = fgetc(fp);
                                if(kar == EOF)
#endif
                                    {
                                    *p = '\0';
                                    eof = true;
                                    break;
                                    }
                                }
                            while(isSpace(kar) && kar != '\n');
                            if(kar == '\n')
                                ++lineno;
                            else if(!eof)
                                prevkar = kar;
                            }
                        if(p > buf2/*Bart 20030225. Sometimes slash and tag are on different lines*/)
                            {
                            *p = '\0';
                            break;
                            }
                        else
                            continue;
                        }
                    if(kar == '/') // oops, word contains slash
                        {
                        ++slashFound; // Bart 20030801. Token may need special treatment as "/"-separated alternatives.
                        *slash = '/'; // put the slash back into the word (somewhere in buf)
                        *p = '\0';    // prepare buf2 for being copied back to buf
                        strcpy(slash+1,buf2); // do the copying
                        slash += p - buf2 + 1; // let slash point at end of buf (the nul byte)
                        p = buf2; // let p start again at start of buf2
                        }
                    else
                        {
                        if(p - buf2 == sizeof(buf2) - 1)
                            {
                            *p = '\0';
                            printf("BUFFER OVERFLOW A [%s]\n",buf2);
                            break;
                            }
                        *p++ = (char)kar;
                        }
                    }
                slashFound = sanityCheck(slashFound,buf);
                return buf;
                }
            }
        else if(  keepPunctuation != 1
               && p > buf /*Bart 20030225 + 1*/
               && ispunct(kar) 
		       && kar != '-' /*gør-det-selv*/ 
		       && kar != '\'' /*bli'r*/
		       )
            {
            if(keepPunctuation == 0)
                {
                *p = '\0';
                slashFound = sanityCheck(slashFound,buf);
                return buf;
                }
            else //if(keepPunctuation == 2)
                {
                punct = kar;
                *p = '\0';
                slashFound = sanityCheck(slashFound,buf);
                return buf;
                }
            }
        if(isSpace(kar))
            {
            if(kar == '\n')
                ++lineno;
            *p = '\0';
            slashFound = sanityCheck(slashFound,buf);
            return buf;
            }
        if(p - buf == sizeof(buf) - 1)
            {
            *p = '\0';
            printf("BUFFER OVERFLOW B [%s]\n",buf);
            slashFound = sanityCheck(slashFound,buf);
            return buf; // overflow?
            }
        if(kar == '/')
            ++slashFound; // Bart 20030801.
        *p++ = (char)kar;
        }
    }


#if !TREE
static int cmpUntagged( const void *arg1, const void *arg2 )
    {
    const Word * n1 = *(const Word * const *)arg1;
    const Word * n2 = *(const Word * const *)arg2;
    return (n1->*Word::cmp)(n2);
    }

static int cmpTagged( const void *arg1, const void *arg2 )
    {
    const taggedWord * n1 = *(const taggedWord * const *)arg1;
    const taggedWord * n2 = *(const taggedWord * const *)arg2;
    return (n1->*taggedWord::comp)(n2);
    }
#endif




#if STREAM
void taggedText::Lemmatise(ostream * fpo,const char * Sep,
                             tallyStruct * tally,
                             unsigned int SortOutput,
                             int UseLemmaFreqForDisambiguation,bool nice,bool DictUnique,bool baseformsAreLowercase,int listLemmas)
#else
void taggedText::Lemmatise(FILE * fpo,const char * Sep,
                             tallyStruct * tally,
                             unsigned int SortOutput,
                             int UseLemmaFreqForDisambiguation,bool nice,bool DictUnique,bool baseformsAreLowercase,int listLemmas)
#endif
    {
    flex::baseformsAreLowercase = baseformsAreLowercase;
    Word::DictUnique = DictUnique;
    baseformpointer::UseLemmaFreqForDisambiguation = UseLemmaFreqForDisambiguation;
    taggedWord::sep = Sep;
    basefrm::sep = Sep;
    if(InputHasTags)
        {
        pcmpBaseforms = cmpBaseforms_wt;
        switch(SortOutput)
            {
            case (SORTWORD<<4)+(SORTFREQ<<2)+SORTPOS:
            case (SORTWORD<<2)+SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_wft;
#if TREE
                taggedWord::inst = &taggedWord::insert_wft;
#else
                taggedWord::comp = &taggedWord::cmp_wft;
#endif
                break;
            case (SORTWORD<<4)+(SORTPOS<<2)+SORTFREQ:
            case (SORTWORD<<2)+SORTPOS:
            case SORTWORD:
                pcmpBaseforms_f = cmpBaseforms_wtf;
#if TREE
                taggedWord::inst = &taggedWord::insert_wtf;
#else
                taggedWord::comp = &taggedWord::cmp_wtf;
#endif
                break;
            case (SORTPOS<<4)+(SORTFREQ<<2)+SORTWORD:
            case (SORTPOS<<2)+SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_tfw;
#if TREE
                taggedWord::inst = &taggedWord::insert_tfw;
#else
                taggedWord::comp = &taggedWord::cmp_tfw;
#endif
                break;
            case (SORTPOS<<4)+(SORTWORD<<2)+SORTFREQ:
            case (SORTPOS<<2)+SORTWORD:
                pcmpBaseforms_f = cmpBaseforms_twf;
#if TREE
                taggedWord::inst = &taggedWord::insert_twf;
#else
                taggedWord::comp = &taggedWord::cmp_twf;
#endif
                break;
            case (SORTFREQ<<4)+(SORTWORD<<2)+SORTPOS:
            case (SORTFREQ<<2)+SORTWORD:
            case SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_fwt;
#if TREE
                taggedWord::inst = &taggedWord::insert_fwt;
#else
                taggedWord::comp = &taggedWord::cmp_fwt;
#endif
                break;
            case (SORTFREQ<<4)+(SORTPOS<<2)+SORTWORD:
            case (SORTFREQ<<2)+SORTPOS:
                pcmpBaseforms_f = cmpBaseforms_ftw;
#if TREE
                taggedWord::inst = &taggedWord::insert_ftw;
#else
                taggedWord::comp = &taggedWord::cmp_ftw;
#endif
                break;
            }
        }
    else
        {
        pcmpBaseforms = cmpBaseforms_w;
        switch(SortOutput)
            {
            case (SORTWORD<<4)+(SORTFREQ<<2)+SORTPOS:
            case (SORTWORD<<2)+SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_wf;
#if TREE
                Word::ins = &Word::insert_wf;
#else
                Word::cmp = &Word::comp_wf;
#endif
                break;
            case (SORTWORD<<4)+(SORTPOS<<2)+SORTFREQ:
            case (SORTWORD<<2)+SORTPOS:
            case SORTWORD:
                pcmpBaseforms_f = cmpBaseforms_wf;
#if TREE
                Word::ins = &Word::insert_wf;
#else
                Word::cmp = &Word::comp_wf;
#endif
                break;
            case (SORTPOS<<4)+(SORTFREQ<<2)+SORTWORD:
            case (SORTPOS<<2)+SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_fw;
#if TREE
                Word::ins = &Word::insert_fw;
#else
                Word::cmp = &Word::comp_fw;
#endif
                break;
            case (SORTPOS<<4)+(SORTWORD<<2)+SORTFREQ:
            case (SORTPOS<<2)+SORTWORD:
                pcmpBaseforms_f = cmpBaseforms_wf;
#if TREE
                Word::ins = &Word::insert_wf;
#else
                Word::cmp = &Word::comp_wf;
#endif
                break;
            case (SORTFREQ<<4)+(SORTWORD<<2)+SORTPOS:
            case (SORTFREQ<<2)+SORTWORD:
            case SORTFREQ:
                pcmpBaseforms_f = cmpBaseforms_fw;
#if TREE
                Word::ins = &Word::insert_fw;
#else
                Word::cmp = &Word::comp_fw;
#endif
                break;
            case (SORTFREQ<<4)+(SORTPOS<<2)+SORTWORD:
            case (SORTFREQ<<2)+SORTPOS:
                pcmpBaseforms_f = cmpBaseforms_fw;
#if TREE
                Word::ins = &Word::insert_fw;
#else
                Word::cmp = &Word::comp_fw;
#endif
                break;
            default:
                pcmpBaseforms_f = cmpBaseforms_wf;
#if TREE
                Word::ins = &Word::insert_wf;
#else
                Word::cmp = &Word::comp_wf;
#endif
                break;
            }
        }

//    int cnt = 0;
    this->aConflict = this->aConflictTypes = this->newcnt = this->newcntTypes = 0;
    if(tally)
        {
        tally->totcnt = total;
        tally->totcntTypes = reducedtotal;
        }
    unsigned long int k;
    cntD = 0;
    cntL = 0;
    if(nice)
        printf("looking up words\n");
    if(Root)
#if TREE
        Root->traverse(&Word::lookup,this);
#else
        {
        for(int i = 0;i < N;++i)
            Root[i]->lookup(this);
        }
#endif
    if(tally)
        {
        tally->newhom = this->aConflict;
        tally->newhomTypes = this->aConflictTypes;
        tally->newcnt = this->newcnt;
        tally->newcntTypes = this->newcntTypes;
        }
    basefrmarrD = new basefrm * [cntD];
    basefrmarrL = new basefrm * [cntL];
    ppD = &basefrmarrD[0];
    ppL = &basefrmarrL[0];
    if(Root)
        {
#if TREE
        Root->traverse(&Word::assign,this);
#else
        for(int i = 0;i < N;++i)
            Root[i]->assignTo(this->ppD,this->ppL);

#endif
        }
    assert(cntD == ppD - &basefrmarrD[0]);
    assert(cntL == ppL - &basefrmarrL[0]);
    sortBaseforms(basefrmarrD,cntD);
    sortBaseforms(basefrmarrL,cntL);

    if(UseLemmaFreqForDisambiguation != 2 /*Why?-> && lext::DictUnique*/)
        {
        if(nice)
            printf("disambiguation by lemma frequency\n");
        if(Root)
            {
#if TREE
            Root->traverse0(&Word::DissambiguateByLemmaFrequency);
            Root->traverse0(&Word::decFreq);
#else
            for(int i = 0;i < N;++i)
                Root[i]->DissambiguateByLemmaFrequency();
            for(int i = 0;i < N;++i)
                Root[i]->decFreq();
#endif
            }
        if(nice)
            printf("...disambiguated by lemma frequency\n");
        }
    if(TagFriends && InputHasTags)
        {
        if(nice)
            printf("disambiguation by tag friends\n");
//        printf("DissambiguateByTagFriends\n");
        if(Root)
            {
#if TREE
            ((taggedWord*)Root)->traverse0T(&taggedWord::DissambiguateByTagFriends);
#else
            for(int i = 0;i < N;++i)
                ((taggedWord**)Root)[i]->DissambiguateByTagFriends();
#endif
            }
        if(nice)
            printf("...disambiguated by tag friends\n");
        }

    tunsorted[0]->setFile(fpo);
    if(listLemmas) /* Make a list of lemmas, for each lemma listing all found word forms belonging to the same paradigm. 
                   Some word forms have ambiguous lemmas. Such word forms are listed under all possible lemmas. 
                   Lemma frequencies can therefore be too high.
                   */
        {
        if(pcmpBaseforms_f != cmpBaseforms_wf && pcmpBaseforms_f != cmpBaseforms_wtf)
            {
            sortBaseforms_f(basefrmarrD,cntD);
            sortBaseforms_f(basefrmarrL,cntL);
            }
        if(nice)
            printf("listing lemmas\n");
        basefrmarrD[0]->setFile(fpo);
        if(listLemmas & 1 && listLemmas & 2)
            {
            int d = 0;
            int l = 0;
            while(d < cntD && basefrmarrD[d] && l < cntL && basefrmarrL[l])
                {
                if(pcmpBaseforms_f(basefrmarrD[d],basefrmarrL[l]) < 0)
                    {
                    if(basefrmarrD[d]->lemmaFreq())
                        basefrmarrD[d]->printb();
                    d++;
                    }
                else
                    {
                    if(basefrmarrL[l]->lemmaFreq())
                        basefrmarrL[l]->printB();
                    l++;
                    }
                }
            while(d < cntD && basefrmarrD[d])
                {
                if(basefrmarrD[d]->lemmaFreq())
                    basefrmarrD[d]->printb();
                d++;
                }
            while(l < cntL && basefrmarrL[l])
                {
                if(basefrmarrL[l]->lemmaFreq())
                    basefrmarrL[l]->printB();
                l++;
                }
            }
        else if(listLemmas & 1)
            {
            for(int K = 0;K < cntD && basefrmarrD[K];++K)
                basefrmarrD[K]->printb();
            }
        else if(listLemmas & 2)
            {
            for(int K = 0;K < cntL && basefrmarrL[K];++K)
                basefrmarrL[K]->printB();
            }
        if(nice)
            printf("...listed lemmas\n");
        }
    else/* Make a list of word forms, for each word form listing all possible lemmas. */
        {
    //    unsorted[0]->setFile(fpo);
        if(nice)
            printf("listing words\n");
        if(SortOutput)
            {
            if(Root)
                {
#if TREE
                if(pcmpBaseforms_f != cmpBaseforms_wf && pcmpBaseforms_f != cmpBaseforms_wtf)
                    {
                    Word * Root_f = 0;
                    Root_f = Root->sort_f(Root_f); // re-order the word tree.
                    Root_f->traverse0C(&Word::print);
                    delete Root_f;
                    }
                else
                    {
                    Root->traverse0C(&Word::print);
                    }
#else
                if(InputHasTags)
                    qsort(Root,N,sizeof(Word *),cmpTagged);
                else
                    qsort(Root,N,sizeof(Word *),cmpUntagged);
                for(int i = 0;i < N;++i)
                    Root[i]->print();
#endif
                }
            }
        else
            {
            int line = 0;
            for(k = 0;k < total;++k)
                {
                if(k == Lines[line])
                    {
                    Word::LastWordOfLine = true;
                    ++line;
                    }
                if(tunsorted[k])
                    tunsorted[k]->print(/*fpo*/);
                Word::LastWordOfLine = false;
                }
            }
        if(nice)
            printf("...listed words\n");
        }
#if 0
    unsorted[0]->setFile(fpnew);
    for(k = 0;k < reducedtotal;++k)
        {
        u.unTaggedWords[k]->printnew(/*fpnew*/);
        }
    unsorted[0]->setFile(fpconflict);
    for(k = 0;k < reducedtotal;++k)
        {
        u.unTaggedWords[k]->printConflict(/*fpconflict*/);
        }
#endif
//    totcnt = k;
//    return cnt;
    if(nice)
        printf("...text processed\n");
    delete [] basefrmarrD;
    delete [] basefrmarrL;
    
    if(Root)
        {
#if TREE
        Root->traverse0(&Word::deleteSecondaryStuff);
#else
        for(int i = 0;i < N;++i)
            Root[i]->deleteSecondaryStuff();
#endif
        }
    }

void taggedText::insert(const char * w)
    {
#if TREE
    if(Root)
        {
        Root->insert(w,tunsorted[total]);
        }
    else
        {
        Word * wrd = new Word(w);
        tunsorted[total] = wrd;
        Root = wrd;
        }
#else
    if(!Hash)
        {
        Hash = new hash<Word>(&Word::itsWord,1000);
        }
    void * v;
    Word * wrd = Hash->find(w,v);
    if(wrd)
        {
        wrd->inc();
        }
    else
        {
        wrd = new Word(w);
        Hash->insert(wrd,v);
        }
    tunsorted[total] = wrd;
#endif
    ++total;
    Lines[lineno - 1] = total;
    }

void taggedText::insert(const char * w, const char * tag)
    {
#if TREE
    if(Root)
        {
        ((taggedWord*)Root)->insert(w,tag,(const taggedWord*&)tunsorted[total]);
        }
    else
        {
        taggedWord * wrd = new taggedWord(w,tag);
        Root = wrd;
        tunsorted[total] = wrd;
        }
#else
    if(!Hash)
        {
        Hash = (hash<Word> *)new hash<taggedWord>(&Word::itsWord,1000);
        }
    void * v;
    taggedWord * wrd;
    for ( wrd = (taggedWord *)Hash->find(w,v)
        ; wrd && strcmp(wrd->m_tag,tag)
        ; wrd = (taggedWord *)Hash->findNext(w,v)
        )
        ;

    if(wrd)
        {
        wrd->inc();
        }
    else
        {
        wrd = new taggedWord(w,tag);
        Hash->insert((Word *)wrd,v);
        }
    tunsorted[total] = wrd;
#endif
    ++total;
    Lines[lineno - 1] = total;
    }

void taggedText::createUnTaggedAlternatives(
#ifndef CONSTSTRCHR
                                            const 
#endif
                                            char * w)
    {
    while(w && *w)
        {
        char * slash = strchr(w,'/');
        if(slash)
            *slash = '\0';
        insert(w);
        if(slash)
            {
            *slash = '/';
            w = slash + 1;
            }
        else
            w = 0;
        }
    }

void taggedText::createUnTagged(const char * w)
    {
    if(*w)
        {
        insert(w);
        }
    }

void taggedText::createTaggedAlternatives(
#ifndef CONSTSTRCHR
                                          const 
#endif
                                          char * w, const char * tag)
    {
    while(w && *w)
        {
        char * slash = strchr(w,'/');
        if(slash)
            {
            *slash = '\0';
            }
        insert(w, tag);
        if(slash)
            {
            *slash = '/';
            w = slash + 1;
            }
        else
            w = 0;
        }
    }

void taggedText::createTagged(const char * w, const char * tag)
    {
    insert(w, tag);
    }


#if STREAM
taggedText::taggedText(istream * fpi,bool a_InputHasTags,char * Iformat,int keepPunctuation,bool nice,
                             unsigned long int size,bool treatSlashAsAlternativesSeparator)
#else
taggedText::taggedText(FILE * fpi,bool a_InputHasTags,char * Iformat,int keepPunctuation,bool nice,
                             unsigned long int size,bool treatSlashAsAlternativesSeparator)
#endif
    :InputHasTags(a_InputHasTags)
    {
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    fields = 0;
    reducedtotal = 0;
    Root = 0;
    basefrmarrD = 0;
    basefrmarrL = 0;

#ifndef CONSTSTRCHR
    const 
#endif
        char * w;
    total = 0;
    const char * tag;
    field * wordfield;
    field * tagfield;
    field * format = 0;
    int slashFound = 0;
    if(nice)
        printf("counting words\n");
    lineno = 1;
    if(Iformat)
        {
        wordfield = 0;
        tagfield = 0;
        format = translateFormat(Iformat,wordfield,tagfield);
        if(!wordfield)
            {
            printf("Input format %s must specify '$w'.\n",Iformat);
            exit(0);
            }
        while(total < size && (w = getwordI(fpi,tag,format,wordfield,tagfield,lineno)) != 0)
            {
            if(*w)
                {
                ++total;
                if(treatSlashAsAlternativesSeparator)
                    {
                    total += findSlashes(w);
                    }
                }
            }
        }
    else
        {
        while(total < size && (w = getword(fpi,tag,InputHasTags,keepPunctuation,slashFound,lineno)) != 0)
            {
            if(*w)
                {
                ++total;
                if(slashFound && treatSlashAsAlternativesSeparator)
                    total += slashFound;
                }
            }
        }
    if(nice)
        {
        printf("... %lu words counted in %lu lines\n",total,lineno);
        }
#if STREAM
    fpi->clear();
    fpi->seekg(0, ios::beg);
#else
    rewind(fpi);
#endif
    if(nice)
        printf("allocating array of pointers to words\n");
    tunsorted =  new const Word * [total];
    if(nice)
        printf("allocating array of line offsets\n");
    Lines =  new unsigned long int [lineno];
    if(nice)
        printf("...allocated array\n");
    /** /
    FILE * ft = fopen("wordlist.txt","wb");
    / **/

    if(!InputHasTags)
        {
        total = 0;
        if(nice)
            printf("reading words\n");
        lineno = 1;
        if(format)
            {
            while(total < size && (w = getwordI(fpi,tag,format,wordfield,tagfield,lineno)) != 0)
                {
                if(treatSlashAsAlternativesSeparator && findSlashes(w))
                    createUnTaggedAlternatives(w);
                else
                    createUnTagged(w);
                }
            }
        else
            {
            while(total < size && (w = getword(fpi,tag,false,keepPunctuation,slashFound,lineno)) != 0)
                {
                if(slashFound && treatSlashAsAlternativesSeparator)
                    createUnTaggedAlternatives(w);
                else 
                    createUnTagged(w);
                }
            }
#if !TREE
        Root = Hash->convertToList(N);
        delete Hash;
//        qsort(Root,N,sizeof(Word *),cmpUntagged);
#endif
        if(nice)
            printf("...read words\n");
        }
    else
        {
        total = 0;
        if(nice)
            printf("reading words\n");
        lineno = 1;
        if(format)
            {
            while(total < size && (w = getwordI(fpi,tag,format,wordfield,tagfield,lineno)) != 0)
                {
                if(treatSlashAsAlternativesSeparator && findSlashes(w))
                    createTaggedAlternatives(w,tag);
                else if(*w)
                    createTagged(w,tag);
                }
            }
        else
            {
            while(total < size && (w = getword(fpi,tag,true,true,slashFound,lineno)) != 0)
                {
                if(*w)
                    {
                    if(!tag)
                        {
                        if(total > 1 && lineno > 1)
                            printf("Tag missing in word #%lu (\"%s\") (line #%lu).\n",total,w,lineno);
                        else
                            printf("Tag missing in word #%lu (\"%s\") (line #%lu). (Is the input text tagged?)\n",total,w,lineno);
                        exit(0);
                        }
                    if(slashFound && treatSlashAsAlternativesSeparator)
                        createTaggedAlternatives(w,tag);
                    else 
                        createTagged(w,tag);
                    }
                }
            reducedtotal = Word::reducedtotal;
            }
#if !TREE
        Root = Hash->convertToList(N);
        delete Hash;
//        qsort(Root,N,sizeof(Word *),cmpTagged);
#endif
        if(nice)
            printf("...read words\n");
        }
#if STREAM
    fpi->clear();
    fpi->seekg(0, ios::beg);
#else
    rewind(fpi);
#endif
    /** /
    for(unsigned int g = 0;g < total;++g)
        {
        if(tunsorted[g] && tunsorted[g]->m_word)
            fprintf(ft,"%s\n",tunsorted[g]->m_word);
        }
    fclose(ft);
    / **/
    sorted = false;
    }

taggedText::~taggedText()
    {
    delete fields;
    delete Root;
#ifdef COUNTOBJECTS
    --COUNT;
#endif
    }

bool taggedText::setFormat(const char * cformat,const char * bformat,const char * Bformat,bool a_InputHasTags)
    {
    return Word::setFormat(cformat,bformat,Bformat,a_InputHasTags);
    }
