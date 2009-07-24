/*
CSTLEMMA - trainable lemmatiser using word-end inflectional rules

Copyright (C) 2002, 2005, 2009  Center for Sprogteknologi, University of Copenhagen

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


static int PERC = 1;
static int RECURSE = 1;

#include "applyaffrules.h"
#include "graph.h"
#include "comp.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <limits.h>
#include <float.h>
#include "utf8func.h"


static const char * nil = "";
static const char * Start = "^";
static const char * End = "$";
static const int inil[1] = {0};
static const int iStart[2] = {'^',0};
static const int iEnd[2] = {'$',0};

union pointers
    {
    unsigned char * uchars;
    char * chars;
    const char * cchars;
    const short int * shorts;
    const long int * longs;
    };

class shortRulePair;
class fullRulePair : public rulePair
    {
    private:
        char Pattern[1000];
        char Replacement[1000];
    protected:
        virtual char * itsPattern(){return Pattern;}
        virtual char * itsReplacement(){return Replacement;}
    public:
        fullRulePair(const char * pat, const char * rep);
        fullRulePair(shortRulePair * Rule);
        virtual ~fullRulePair(){--FullRulePairCount;}
        void copy(fullRulePair * Rule);
        const char * pattern()
            {
            return Pattern;
            }
        const char * replacement()
            {
            return Replacement;
            }
    };


fullRulePair::fullRulePair(const char * pat, const char * rep)
    {
    strcpy(Pattern,pat);
    strcpy(Replacement,rep);
    ++FullRulePairCount;
    }

void fullRulePair::copy(fullRulePair * Rule)
    {
    strcpy(Pattern,Rule->Pattern);
    strcpy(Replacement,Rule->Replacement);
    }

class ruleTemplate;
class shortRulePair
    {
    private:
        char patternArray[1000];
        char replacementArray[1000];
        void trim();
    public:
        const char * itsPatternArray(){return patternArray;}
        const char * itsReplacementArray(){return replacementArray;}
        void copy(shortRulePair * Rule)
            {
            strcpy(patternArray,Rule->patternArray);
            strcpy(replacementArray,Rule->replacementArray);
            }
        shortRulePair(trainingPair * trainingpair,ruleTemplate * Template);
        bool checkRule(/*ruleTemplate * Template,*/trainingPair * trainingpair,rulePair * parentPat);
        ~shortRulePair(){--ShortRulePairCount;}
    };

class ruleTemplate
    {
    protected:
        char npatternArray[1000];
        char nreplacementArray[1000];
    public:
        const char * itsNPatternArray(){return npatternArray;}
        const char * itsNReplacementArray(){return nreplacementArray;}
        void copy(ruleTemplate * Template)
            {
            strcpy(npatternArray,Template->npatternArray);
            strcpy(nreplacementArray,Template->nreplacementArray);
            }
        bool makebigger(int countdown);
        ruleTemplate(){++RuleTemplateCount;}
        ~ruleTemplate(){--RuleTemplateCount;}
    };

static const int unequal = '#';
static const int equal = '=';
#if SUFFIXONLY
void suffix(char * mask)
    {
    char * m;
    for(m = mask;*m && *m == equal;++m)
        ;
    for(;*m;++m)
        *m = unequal;
    }
#endif



class similData: public ruleTemplate
    {
    private:
        char pattern[1000];
        char replacement[1000];
        char * psimilar;
        char * ppattern;
        char * preplacement;
        char * nppattern;
        char * npreplacement;
    public:
        similData(char * similarArray)
            {
            psimilar = similarArray;
            ppattern = pattern;
            preplacement = replacement;
            nppattern = npatternArray;
            npreplacement = nreplacementArray;
            }
        ptrdiff_t simil(
            const char * const s1,
            const char * const n1,
            const char * const s2,
            const char * const n2,
            const char * const start,
            const char * const end);
        ptrdiff_t isimil(
            const int * const s1,
            const int * const n1,
            const int * const s2,
            const int * const n2,
            const int * const start,
            const int * const end);
        void match(trainingPair * trainingpair)
            {
            //char * psimilarsave = psimilar;
            if(UTF8)
                {
                size_t wordLen = trainingpair->itsWordlength();
                size_t lemmaLen = trainingpair->itsLemmalength();
                /*FILE * f = fopen("LOG.txt","a");
                fprintf(f,"%.*s %.*s\n",wordLen,trainingpair->itsWord(),lemmaLen,trainingpair->itsLemmaHead());
                fclose(f);*/
                int * iWord = new int[wordLen+1];
                int * iLemmaHead = new int[lemmaLen+1];
                wordLen = Utf8ToUnicode(iWord,trainingpair->itsWord(),wordLen);
                assert(lemmaLen > 0);
                lemmaLen = Utf8ToUnicode(iLemmaHead,trainingpair->itsLemmaHead(),lemmaLen);
                isimil(iWord,iWord+wordLen,iLemmaHead,iLemmaHead+lemmaLen,iStart,iEnd);
                //fprintf(flog,"isimil returns [%s] [%s]\n",pattern,replacement);
                //fprintf(flog,"%.*s %.*s\n",wordLen,npatternArray,lemmaLen,nreplacementArray);
                /*
                fprintf(flog,"%.*s\t%.*s\t",wordLen,trainingpair->itsWord(),lemmaLen,trainingpair->itsLemmaHead());
                int k,l;
                bool star = false;
                char pat[1000];
                char rep[1000];
                char pat2[1000];
                char rep2[1000];
                char * ppat = pat;
                char * prep = rep;
                char * ppat2 = pat2;
                char * prep2 = rep2;
                k = 0;
                l = 0;
                while(k < wordLen)
                    {
                    if(npatternArray[k] == '=')
                        {
                        if(!star)
                            {
                            *ppat++ = '*';
                            *prep++ = '*';
                            star = true;
                            }
                        *ppat2++ = trainingpair->itsWord()[k];
                        *prep2++ = trainingpair->itsLemmaHead()[l];
                        ++k;
                        ++l;
                        }
                    else
                        {
                        *ppat2++ = '?';
                        *prep2++ = '?';
                        do
                            {
                            *ppat++ = trainingpair->itsWord()[k];
                            }
                        while(++k < wordLen && npatternArray[k] != '=');
                        while(l < lemmaLen && nreplacementArray[l] != '=')
                            {
                            *prep++ = trainingpair->itsLemmaHead()[l];
                            ++l;
                            }
                        star = false;
                        }
                    }
                *ppat = *prep = 0;
                *ppat2 = *prep2 = 0;
                fprintf(flog,"%s\t%s\t%s\t%s\n",pat2,rep2,pat,rep);
                */
                delete [] iWord;
                delete [] iLemmaHead;
                }
            else
                {
                simil(trainingpair->itsWord(),trainingpair->itsWord()+trainingpair->itsWordlength(),trainingpair->itsLemmaHead(),trainingpair->itsLemmaHead()+trainingpair->itsLemmalength(),Start,End);
                }
            *psimilar = *ppattern = *preplacement = '\0';
            *nppattern = *npreplacement = '\0';
            /*FILE * f = fopen("LOG.txt","a");
            fprintf(f,"similar [%s] pattern [%s] replacement [%s] nppattern [%s] npreplacement [%s]\n",
                psimilarsave,pattern,replacement,npatternArray,nreplacementArray);
            fclose(f);*/
            //assert(strlen(itsNPatternArray()) == trainingpair->itsWordlength());
            //assert(strlen(itsNReplacementArray()) == trainingpair->itsLemmalength());
            }
        bool mergeTemplates(const char * predef)
            {
            //printf("pattern %s\npredef  %s\n",npatternArray,predef);
            char * n = npatternArray;
            char * r = nreplacementArray - 1;
            const char * p = predef;
            int eqs = 0;
            int reqs = 0;
            bool changed = false;
            while(*n)
                {
                if(*n == equal)
                    {
                    ++eqs;
                    do
                        {
                        if(*++r == equal)
                            ++reqs;
                        }
                    while(reqs < eqs);
                    if(*p == unequal)
                        {
                        *n = unequal;
                        *r = unequal;
                        changed = true;
                        }
                    }
                ++n;
                ++p;
                }
#if SUFFIXONLY
            suffix(npatternArray);
            suffix(nreplacementArray);
#endif
            return changed;
            }
        void print(FILE *fp)
            {/*
            *psimilar = '\0';
            *ppattern = *preplacement = '\0';*/
            fprintf(fp,"\npattern:\n%s\nreplacement:\n%s\n",pattern,replacement);
            fprintf(fp,"npattern:\n%s\nnreplacement:\n%s\n",npatternArray,nreplacementArray);
            }
    };


const char * find
        (const char * string
        ,int c
        ,const char * end
        )
    {
    while(string < end && *string != c)
        {
        ++string;
        }
    return (string < end) ? string : NULL;
    }

struct aFile
    {
    union pointers file;
    union pointers * Lines;
    int filesize;
    int lines;
    long size;
    const char * eob;
#if 0    
    int * frequencylist(int & max0,char & k0,int & max1,char & k1)
        {
        static int cnt[256];
        for(int i = 0;i < 256;++i)
            cnt[i] = 0;
        for(const unsigned char * s = file.uchars + size;s >= file.uchars;--s)
            ++cnt[*s];
        max0 = 0;
        max1 = 0;
        for(int i = 0;i < 256;++i)
            if(cnt[i] > max0)
                {
                max1 = max0;
                k1 = k0;
                max0 = cnt[i];
                k0 = (char)i;
                }
            else if(cnt[i] > max1)
                {
                max1 = cnt[i];
                k1 = (char)i;
                }
        return cnt;
        }
    void max()
        {
        int max0;
        char k0;
        int max1;
        char k1;
        /*int * max =*/ frequencylist(max0,k0,max1,k1);
        if(k0 < 32)
            printf("max0 \\x%.2x %d %ld\n",k0,max0,(max0*100)/size);
        else
            printf("max0 %c %d %ld\n",k0,max0,(max0*100)/size);

        if(k1 < 32)
            printf("max1 \\x%.2x %d %ld\n",k1,max1,(max1*100)/size);
        else
            printf("max1 %c %d %ld\n",k1,max1,(max1*100)/size);
        }
#endif
    aFile(FILE * fp):Lines(NULL),filesize(0),lines(0),eob(NULL)
        {
        file.chars = NULL;
        size = 0;
        while(getc(fp) != EOF)
            {
            ++size;
            }
        file.chars = new char[size];
        rewind(fp);
        size = 0;
        int kar;
        while((kar = getc(fp)) != EOF)
            {
            file.chars[size++] = (char)kar;
            }

        eob = file.chars + size;

        const char * buf;
        int line;
        const char * nl;
        //const char * cr; 
        // Assumption: \n terminates a group of words that belong together because they share lemma of word form.
        // \r separates word-lemma pairs in such a group.
        for(buf = file.chars,line = 0;buf < eob;++line,buf = (nl ? nl + 1 : eob))
            {
            nl = find(buf,'\n',eob);
            /*cr = find(buf,'\r',eob);
            if(cr != NULL && cr < nl)
                nl = cr;*/
            }

        lines = line;
        //Lines = (pointers *)calloc(lines,sizeof(file));
        Lines = new pointers[lines];
        
        for(buf = file.cchars,line = 0;buf < eob;++line,buf = (nl ? nl + 1 : eob))
            {
            Lines[line].cchars = buf;
            nl = find(buf,'\n',eob);
            /*cr = find(buf,'\r',eob);
            if(cr != NULL && cr < nl)
                nl = cr;*/
            }
        }
    ~aFile()
        {
        delete [] file.chars;
        //free(Lines);
        delete [] Lines;
        }
    };
/*
static void pr(const char * s,const char * e)
    {
    while(s < e)
        putchar(*s++);
    }
*/

static const int unequalBit = 1 << 0;
static const int equalBit = 1 << 1;
static const int transition = unequalBit | equalBit;

static bool glob_wildcards = false;

ptrdiff_t similData::simil(
    const char * const s1,
    const char * const n1,
    const char * const s2,
    const char * const n2,
    const char * const start,
    const char * const end)
{
/*#define TOLOWER(c) (c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c)*/
const char * ls1;
const char * s1l = NULL;
const char * s1r = NULL;
const char * s2l = NULL;
const char * s2r = NULL;
ptrdiff_t max;
/*

d o o d l a c h t e n            d o o d l a c h e n
^                   ^            ^                 ^
s1                  n1           s2                n2



d o o d l a c h t e n            d o o d l a c h e n
^                   ^            ^                 ^
s1                  n1           s2                n2
^               ^                ^               ^
ls1             lls1             ls2             lls2
^---------------^
       dif
       max

       
       
d o o d l a c h t e n            d o o d l a c h e n
^                   ^            ^                 ^
s1                  n1           s2                n2
^               ^                ^               ^
ls1             lls1             ls2             lls2
^---------------^
       dif
       max
^               ^                ^               ^
s1l             s1r              s2l             s2r
*/


//printf("simil(");pr(s1,n1);printf(",");pr(s2,n2);printf(")\n");

/* beschouw elk teken van s1 als mogelijk startpunt voor match */
for(max = 0,ls1 = s1;ls1 + max < n1;ls1++)
    {
//    printf("L ");pr(s1,ls1);
    const char * ls2;
    /* vergelijk met s2 */
    for(ls2 = s2;ls2 + max < n2;ls2++)
        {
        const char * lls1;
        const char * lls2;
//        printf("R   ");pr(s2,ls2);
        /* bepaal lengte gelijke stukken */
        for(lls1 = ls1,lls2 = ls2
           ;
//          lls1 < n1 && lls2 < n2 && lowerEquivalent[(unsigned char)*lls1] == lowerEquivalent[(unsigned char)*lls2];
            lls1 < n1 && lls2 < n2 && *lls1 == *lls2
           ;lls1++,lls2++
           )
                ;
        /* pas evt score aan */
        ptrdiff_t dif = lls1 - ls1;
        if(dif > max)
            {
            max = dif;
            /* onthou eindpunten van linkerstrings en
            beginpunten rechterstrings */
            s1l = ls1;
            s1r = lls1;
            s2l = ls2;
            s2r = lls2;
            }
        }
    }
if(max)
    {
    /*
    The longest sequence of characters occurring in s1 and s2 is found.
    The number of characters in the common substring is <max>.
    */
    if(s1 == s1l)
        {
        /*Potential problematic pattern: a replacement for no pattern.
        (Insertion).
        That only works if the replacement is at the start or the end
        of the word. (prefix or suffix)*/
        if(!*start)
            {
            /*Problem: empty pattern somewhere in the middle.
            Solution: borrow to the left or to the right.*/
            s1l++;
            s2l++;
            /*Caveat: By borrowing the left and right may touch. 
            (max == 1 or 2)*/
            --max;
            }
        }
    }
if(max)
    {
    if(s1r == n1)
        {
        if(!*end)
            {
            s1r--;
            s2r--;
            --max;
            }
        }
    }

if(max)
    {
    if(s1 != s1l || s2 != s2l)
        {
        /* Recurse if the common substring does not start at the beginning of 
        at least one of s1 and s2. */
        if(max)
            max += simil(s1,s1l,s2,s2l,start,nil);

        }
    else 
        {
        if(*start && glob_wildcards)
            {
            *psimilar++ = *start;
            /*
            *nppattern++ = equal;
            *glob_npreplacement++ = equal;
            */
            }
        }


    //*glob_nppattern++ = '(';
    //*glob_npreplacement++ = '(';
    for(const char * s = s1l;s < s1r;++s)
        /* This is the longest common substring. */
        {
        //*glob_psimilar++ = *s;
        *nppattern++ = equal;
        //*glob_nppattern++ = *s;
        *npreplacement++ = equal;
        //*glob_npreplacement++ = *s;
        }
    //*glob_nppattern++ = ')';
    //*glob_npreplacement++ = ')';
    

    if(s1r != n1 || s2r != n2)
        {
        /* Recurse if the common substring does not end at the end of 
        at least one of s1 and s2. */
        max += simil(s1r,n1,s2r,n2,nil,end);
        }
    else 
        {
        if(*end && glob_wildcards)
            {
            *psimilar++ = *end;
            /*
            *glob_nppattern++ = equal;
            *glob_npreplacement++ = equal;
            */
            }
        }
    }
else
    {
    /*The strings s1 and s2 are completely different*/
    if(*start)
        {
        if(s1 < n1)
            *ppattern++ = *start; // ^ge
        if(s2 < n2)
            *preplacement++ = *start; // ^over
        }
    else if(!(ppattern == pattern && preplacement == replacement))
        {
        *ppattern++ = '*';
        *preplacement++ = '*';
        }

    if(s1 < n1) // pattern not nothing
        {
        for(const char * s = s1;s < n1;++s)
            {
            *ppattern++ = *s;
            *nppattern++ = unequal;
            //*glob_nppattern++ = *s;
            }
        }

    if(s2 < n2)
        {
        for(const char * s = s2;s < n2;++s)
            {
            *preplacement++ = *s;
            *npreplacement++ = unequal;
            //*glob_npreplacement++ = *s;
            }
        }
    if(!*start && !*end && glob_wildcards)
        {
        *psimilar++ = '?';
        }

    if(*end)
        {
        *ppattern++ = *end;
        *preplacement++ = *end;
        }
    }
return max;
}

ptrdiff_t similData::isimil(
            const int * const s1,
            const int * const n1,
            const int * const s2,
            const int * const n2,
            const int * const start,
            const int * const end)
{
/*#define TOLOWER(c) (c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c)*/
const int * ls1;
const int * s1l = NULL;
const int * s1r = NULL;
const int * s2l = NULL;
const int * s2r = NULL;
ptrdiff_t max;
/*

d o o d l a c h t e n            d o o d l a c h e n
^                   ^            ^                 ^
s1                  n1           s2                n2



d o o d l a c h t e n            d o o d l a c h e n
^                   ^            ^                 ^
s1                  n1           s2                n2
^               ^                ^               ^
ls1             lls1             ls2             lls2
^---------------^
       dif
       max

       
       
d o o d l a c h t e n            d o o d l a c h e n
^                   ^            ^                 ^
s1                  n1           s2                n2
^               ^                ^               ^
ls1             lls1             ls2             lls2
^---------------^
       dif
       max
^               ^                ^               ^
s1l             s1r              s2l             s2r
*/


//printf("simil(");pr(s1,n1);printf(",");pr(s2,n2);printf(")\n");

/* beschouw elk teken van s1 als mogelijk startpunt voor match */
for(max = 0,ls1 = s1;ls1 + max < n1;ls1++)
    {
//    printf("L ");pr(s1,ls1);
    const int * ls2;
    /* vergelijk met s2 */
    for(ls2 = s2;ls2 + max < n2;ls2++)
        {
        const int * lls1;
        const int * lls2;
//        printf("R   ");pr(s2,ls2);
        /* bepaal lengte gelijke stukken */
        for(lls1 = ls1,lls2 = ls2
           ;
//          lls1 < n1 && lls2 < n2 && lowerEquivalent[(unsigned char)*lls1] == lowerEquivalent[(unsigned char)*lls2];
            lls1 < n1 && lls2 < n2 && *lls1 == *lls2
           ;lls1++,lls2++
           )
                ;
        /* pas evt score aan */
        ptrdiff_t dif = lls1 - ls1;
        if(dif > max)
            {
            max = dif;
            /* onthou eindpunten van linkerstrings en
            beginpunten rechterstrings */
            s1l = ls1;
            s1r = lls1;
            s2l = ls2;
            s2r = lls2;
            }
        }
    }
if(max)
    {
    /*
    The longest sequence of characters occurring in s1 and s2 is found.
    The number of characters in the common substring is <max>.
    */
    if(s1 == s1l)
        {
        /*Potential problematic pattern: a replacement for no pattern.
        (Insertion).
        That only works if the replacement is at the start or the end
        of the word. (prefix or suffix)*/
        if(!*start)
            {
            /*Problem: empty pattern somewhere in the middle.
            Solution: borrow to the left or to the right.*/
            s1l++;
            s2l++;
            /*Caveat: By borrowing the left and right may touch. 
            (max == 1 or 2)*/
            --max;
            }
        }
    }
if(max)
    {
    if(s1r == n1)
        {
        if(!*end)
            {
            s1r--;
            s2r--;
            --max;
            }
        }
    }

if(max)
    {
    if(s1 != s1l || s2 != s2l)
        {
        /* Recurse if the common substring does not start at the beginning of 
        at least one of s1 and s2. */
        if(max)
            max += isimil(s1,s1l,s2,s2l,start,inil);

        }
    else 
        {
        if(*start && glob_wildcards)
            {
            //*psimilar++ = *start;
            psimilar += UnicodeToUtf8(*start,psimilar,1000);
            /*
            *nppattern++ = equal;
            *glob_npreplacement++ = equal;
            */
            }
        }


    //*glob_nppattern++ = '(';
    //*glob_npreplacement++ = '(';
    for(const int * s = s1l;s < s1r;++s)
        /* This is the longest common substring. */
        {
        //*glob_psimilar++ = *s;
        //*nppattern++ = equal;
        nppattern += UnicodeToUtf8(equal,nppattern,1000);
        //*glob_nppattern++ = *s;
        //*npreplacement++ = equal;
        npreplacement += UnicodeToUtf8(equal,npreplacement,1000);
        //*glob_npreplacement++ = *s;
        }
    //*glob_nppattern++ = ')';
    //*glob_npreplacement++ = ')';
    

    if(s1r != n1 || s2r != n2)
        {
        /* Recurse if the common substring does not end at the end of 
        at least one of s1 and s2. */
        max += isimil(s1r,n1,s2r,n2,inil,end);
        }
    else 
        {
        if(*end && glob_wildcards)
            {
            //*psimilar++ = *end;
            psimilar += UnicodeToUtf8(*end,psimilar,1000);
            /*
            *glob_nppattern++ = equal;
            *glob_npreplacement++ = equal;
            */
            }
        }
    }
else
    {
    /*The strings s1 and s2 are completely different*/
    if(*start)
        {
        if(s1 < n1)
            {
            //*ppattern++ = *start; // ^ge
            ppattern += UnicodeToUtf8(*start,ppattern,1000);// ^ge
            }
        if(s2 < n2)
            {
            //*preplacement++ = *start; // ^over
            preplacement += UnicodeToUtf8(*start,preplacement,1000);// ^over
            }
        }
    else if(!(ppattern == pattern && preplacement == replacement))
        {
        //*ppattern++ = '*';
        ppattern += UnicodeToUtf8('*',ppattern,1000);
        //*preplacement++ = '*';
        preplacement += UnicodeToUtf8('*',preplacement,1000);// ^over
        }

    if(s1 < n1) // pattern not nothing
        {
        for(const int * s = s1;s < n1;++s)
            {
            //*ppattern++ = *s;
            ppattern += UnicodeToUtf8(*s,ppattern,1000);
            //*nppattern++ = unequal;
            nppattern += UnicodeToUtf8(unequal,nppattern,1000);
            //*glob_nppattern++ = *s;
            }
        }

    if(s2 < n2)
        {
        for(const int * s = s2;s < n2;++s)
            {
            //*preplacement++ = *s;
            preplacement += UnicodeToUtf8(*s,preplacement,1000);
            //*npreplacement++ = unequal;
            npreplacement += UnicodeToUtf8(unequal,npreplacement,1000);
            //*glob_npreplacement++ = *s;
            }
        }
    if(!*start && !*end && glob_wildcards)
        {
        //*psimilar++ = '?';
        psimilar += UnicodeToUtf8('?',psimilar,1000);
        }

    if(*end)
        {
        //*ppattern++ = *end;
        ppattern += UnicodeToUtf8(*end,ppattern,1000);
        //*preplacement++ = *end;
        preplacement += UnicodeToUtf8(*end,preplacement,1000);
        }
    }
return max;
}



/*
static int bigrand()
    {
    int res = 0;
    for(int i = 0;i < 20;++i)
        res += rand();
    return res;
    }
*/

static struct aFile * readFile(char * fname)
    {
    printf("reading file %s ...",fname);
    FILE * fp = fopen(fname,"rb");
    if(fp)
        {
        aFile * a_file = new aFile(fp);
        fclose(fp);
        printf("\rreading file %s DONE\n",fname);
        return a_file;
        }
    printf("\rreading file %s FAILED\n",fname);
    return NULL;
    }

fullRulePair::fullRulePair(shortRulePair * Rule)
    {
    char * ppat = Pattern;
    char * prep = Replacement;

    size_t patlen = strlen((const char*)Rule->itsPatternArray());
    size_t replen = strlen((const char*)Rule->itsReplacementArray());
    if(*Rule->itsPatternArray() != '^')
        {
        if(*Rule->itsReplacementArray() != '^')
            {
            strcpy(Pattern,"^*");
            strcpy(Replacement,"^*");
            ppat += 2;
            prep += 2;
            }
        else
            {
            strcpy(Pattern,"^");
            ++ppat;
            }
        }
    else if(*Rule->itsReplacementArray() != '^')
        {
        strcpy(Replacement,"^");
        ++prep;
        }
    strcpy(ppat,Rule->itsPatternArray());
    strcpy(prep,Rule->itsReplacementArray());
    ppat += patlen - 1;
    prep += replen - 1;
    if(*ppat == '*' && *prep == '*')
        {
        strcpy(++ppat,"$");
        strcpy(++prep,"$");
        }
    else
        {
        if(*ppat != '$')
            strcpy(++ppat,"*$");
        if(*prep != '$')
            strcpy(++prep,"*$");
        }
    ++FullRulePairCount;
    }


bool rulePair::apply(trainingPair * trainingpair,size_t lemmalength,char * lemma,char * mask) 
    {
    char wrd[100];
    sprintf(wrd,"^%.*s$",trainingpair->itsWordlength(),trainingpair->itsWord());
    char * p = itsPattern();
    char * r = itsReplacement();
    if(!*p) // root
        {
        p = "^*$";
        r = p;
        }
    char * w = wrd;
    char * d = lemma;
    char * last = lemma + lemmalength - 7;
    char * m = mask;
    int inc;
    int P = UTF8char(p,UTF8);
    int W = UTF8char(w,UTF8);
    int R = UTF8char(r,UTF8);
//    while(*p && *r)
    while(P && R)
        {
        //if(*p == *w)
        if(P == W)
            {
            *m = unequal;
            while(R && R != '*' && d < last)
            //while(*r && *r != '*')
                {
                inc = copyUTF8char(r,d);
                r += inc;
                d += inc;
                //*d++ = *r++;
                R = UTF8char(r,UTF8);
                }
            do
                {
                *++m = unequal;
                p += skipUTF8char(p);
                w += skipUTF8char(w);
                P = UTF8char(p,UTF8);
                W = UTF8char(w,UTF8);
                //++p;
                //++w;
                }
            while(P && P != '*' && P == W);
//            while(*p && *p != '*' && *p == *w);
//            if(*p != *r)
            if(P != R)
                {
#if SUFFIXONLY
                suffix(mask);
#endif
                return false;
                }
            }
        else if(R == '*')
        //else if(*r == '*')
            {
            //assert(*p == '*');
            //++p;
            //++r;
            p += skipUTF8char(p);
            r += skipUTF8char(r);
            P = UTF8char(p,UTF8);
            R = UTF8char(r,UTF8);
            char * ep = strchr(p,'*');
            if(ep)
                *ep = '\0';
            char * sub = strstr(w,p);
            if(sub)
                {
                while(w < sub && d < last)
                    {
                    *m++ = equal;
                    //*d++ = *w++;
                    inc = copyUTF8char(w,d);
                    w += inc;
                    d += inc;
                    }
                W = UTF8char(w,UTF8);
                }
            else
                {
                if(ep)
                    *ep = '*';
                d = lemma;
                break;
                }
            if(ep)
                *ep = '*';
            }
        else
            {
            d = lemma;
            break;
            }
        }
    //if(*p || *r)
    if(P || R)
        {
#if SUFFIXONLY
        suffix(mask);
#endif
        return false;
        }
    *--m = '\0';
    for(m = mask;*m;++m)
        m[0] = m[1];
    *d = '\0';
    d = lemma;
    char * oldd = d;
    inc = skipUTF8char(d);
    d += inc;
    while(*d)
        {
        *oldd++ = *d++;
        }
    *oldd = '\0';
    if(oldd > lemma + 1)
        oldd[-1] = '\0';
    /*
    while(*d)
        {
        *d = *(d+1);
        ++d;
        }
    if(d > lemma + 2)
        d[-2] = '\0';
    */
#if SUFFIXONLY
    suffix(mask);
#endif
    return true;
    }
/*
static void rev(const char * s,const char * e)
    {
    char * S = (char*)s;
    char * E = (char*)e;
    while(--E > S)
        {
        int k = *S;
        *S++ = *E;
        *E = k;
        }
    }
*/

shortRulePair::shortRulePair(trainingPair * trainingpair,ruleTemplate * Template)
    {
    bool star = false;
    int i = 0;
    int i2 = 0;
    int j = 0;
    int j2 = 0;
    int k = 0;
    int k2 = 0;
    int inc;
    /*
    char * tmp = patternArray;
    char * tmp2 = replacementArray;
    */
    while(Template->itsNPatternArray()[i] && Template->itsNReplacementArray()[i2])
        {
        while(UTF8 && Template->itsNPatternArray()[i] && Template->itsNPatternArray()[i] != equal)
            {
            star = false;
            inc = copyUTF8char(trainingpair->itsWord()+k,patternArray+j);
            j += inc;
            k += inc;
            ++i;
            }
        while(Template->itsNPatternArray()[i] && Template->itsNPatternArray()[i] != equal)
            {
            star = false;
            patternArray[j] = trainingpair->itsWord()[k];
            ++j;
            ++k;
            ++i;
            }
        while(UTF8 && Template->itsNReplacementArray()[i2] && Template->itsNReplacementArray()[i2] != equal)
            {
            star = false;
            inc = copyUTF8char(trainingpair->itsLemmaHead()+k2,replacementArray+j2);
            j2 += inc;
            k2 += inc;
            ++i2;
            }
        while(Template->itsNReplacementArray()[i2] && Template->itsNReplacementArray()[i2] != equal)
            {
            star = false;
            replacementArray[j2] = trainingpair->itsLemmaHead()[k2];
            ++j2;
            ++k2;
            ++i2;
            }
        while(Template->itsNPatternArray()[i] == equal && Template->itsNReplacementArray()[i2] == equal)
            {
            if(!star)
                {
                patternArray[j++] = '*';
                replacementArray[j2++] = '*';
                patternArray[j] = '\0';
                replacementArray[j2] = '\0';
                star = true;
                }
            k += skipUTF8char(trainingpair->itsWord()+k);
            k2 += skipUTF8char(trainingpair->itsLemmaHead()+k2);
            i++;
            i2++;
            }
        }
    while(UTF8 && Template->itsNPatternArray()[i])
        {
        inc = copyUTF8char(trainingpair->itsWord()+k,patternArray+j);
        j += inc;
        k += inc;
        ++i;
        }
    while(Template->itsNPatternArray()[i])
        {
        patternArray[j] = trainingpair->itsWord()[k];
        ++j;
        ++k;
        ++i;
        }
    while(UTF8 && Template->itsNReplacementArray()[i2])
        {
        inc = copyUTF8char(trainingpair->itsLemmaHead()+k2,replacementArray+j2);
        j2 += inc;
        k2 += inc;
        ++i2;
        }
    while(Template->itsNReplacementArray()[i2])
        {
        replacementArray[j2] = trainingpair->itsLemmaHead()[k2];
        ++j2;
        ++k2;
        ++i2;
        }
    patternArray[j] = '\0';
    replacementArray[j2] = '\0';
    /*fclose(flog);
    flog = fopen("flog.txt","a");
    fprintf(flog,"Voor trim patternArray %s replacementArray %s\n",patternArray,replacementArray);*/
    trim();
/*    fprintf(flog,"Na trim   patternArray %s replacementArray %s\n",patternArray,replacementArray);
    fclose(flog);
    flog = fopen("flog.txt","a");*/
    ++ShortRulePairCount;
    }

static void shiftleft(char * a)
    {
    while(*a)
        {
        a[0] = a[1];
        ++a;
        }
    }

static void shiftright(char * A)
    {
    char * a = A + strlen(A);
    a[1] = '\0';
    while(a > A)
        {
        a[0] = a[-1];
        --a;
        }
    }

static int cnt(char * s, int k)
    {
    int ret = 0;
    while(*s)
        {
        if(*s == k)
            ++ret;
        ++s;
        }
    return ret;
    }

void shortRulePair::trim()
    {
    char * A = patternArray;
    char * B = replacementArray;
    if(A[0] == '*' && B[0] == '*')
        {
        int ca = cnt(A,'*');
        int cb = cnt(B,'*');
        if(ca == cb)
            {
            shiftleft(A);
            shiftleft(B);
            }
        else if(ca < cb)
            shiftleft(B);
        else
            shiftleft(A);

        }
    else 
        {
        if(A[0] != '*' && A[0] != '^')
            {
            shiftright(A);
            A[0] = '^';
            }
        if(B[0] != '*' && B[0] != '^')
            {
            shiftright(B);
            B[0] = '^';
            }
        }
    size_t la = strlen(A);
    size_t lb = strlen(B);
    if(la && A[la - 1] == '*' && lb && B[lb - 1] == '*')
        {
        A[la - 1] = '\0';
        B[lb - 1] = '\0';
        }
    else
        {
        if(la)
            {
            if(A[la - 1] == '$')
                {
                if(!lb || (lb && B[lb - 1] == '*'))
                    {
                    B[lb] = '$';
                    B[lb + 1] = '\0';
                    }
                }
            else if(A[la - 1] != '*')
                {
                A[la] = '$';
                A[la + 1] = '\0';
                if(!lb || (lb && B[lb - 1] == '*'))
                    {
                    B[lb] = '$';
                    B[lb + 1] = '\0';
                    }
                }
            }
        if(lb)
            {
            if(B[lb - 1] == '$')
                {
                if(!la || (la && A[la - 1] == '*'))
                    {
                    A[la] = '$';
                    A[la + 1] = '\0';
                    }
                }
            else if(B[lb - 1] != '*')
                {
                B[lb] = '$';
                B[lb + 1] = '\0';
                if(!la || (la && A[la - 1] == '*'))
                    {
                    A[la] = '$';
                    A[la + 1] = '\0';
                    }
                }
            }
        }
    }
#if 1
bool ruleTemplate::makebigger(int countdown)
    {
    /*
    Change a = to a # if
    1) The next byte in replacement is # or
    2) The previous byte in pattern is = and the current byte is #
    */
    if(countdown == 0)
        return false;
    char * pattern = npatternArray;
    char * replacement = nreplacementArray;
    //printf("in:\n%s\n%s\n",npatternArray,nreplacementArray);
    while(countdown)
        {
        if(*pattern == equal) // =
            {
            // scan replacement until corresponding equal byte found.
            while(*replacement == unequal)
                {
                ++replacement;
                }
            // Corresponding equal byte found in replacement.
            assert(*replacement);
            if(  pattern[1] == unequal
              || replacement[1] == unequal
              || pattern > npatternArray && pattern[-1] == unequal
              || replacement > nreplacementArray && replacement[-1] == unequal
#if SUFFIXONLY
#else
              // Start and end are also transition points:
              // (This gives a markedly better result, at least for Dutch)
              || pattern[1] == '\0'
              || replacement[1] == '\0'
              || pattern == npatternArray
              || replacement == nreplacementArray
#endif
              )
                {
                if(countdown == 1)
                    {
                    *pattern = unequal; // #
                    *replacement = unequal; // Replace the =s with #s and return
                    return true;
                    }
                --countdown;
                }
            ++replacement; // Go past this = in replacement.
            }
        ++pattern;
        if(!*pattern)
            {
            return false; // signal to caller that countdown has reached maxvalue.
            }
        }
    return false;
    }

#else
bool ruleTemplate::makebigger(int countdown)
    {
    /*
    Change a = to a # if
    1) The next byte in replacement is # or
    2) The previous byte in pattern is = and the current byte is #
    */
    if(countdown == 0)
        return false;
    int state = 0; // has two bits: unequalBit and equalBit. 
    // If both bits are set we have a transition.
    //int patternEqual = 0; // counts number of = in pattern (not used)
    int replacementEqual = 0;// counts number of = in replacement. Only used to know whether # is at start of replacement or not.
    int lastpatternEqual = 0;// counts number of = in pattern since last #
    char * pattern = npatternArray;
    char * replacement = nreplacementArray;
    //printf("in:\n%s\n%s\n",npatternArray,nreplacementArray);
    while(countdown)
        {
        if(*pattern == equal) // =
            {
            //++patternEqual;
            ++lastpatternEqual;
            //<
            // scan replacement until corresponding equal byte found.
            while(*replacement == unequal)
                {
                if(!replacementEqual) // ^====$ ^#====$
                    state |= unequalBit;
                ++replacement;
                }
            // Corresponding equal byte found in replacement.
            ++replacementEqual;
            assert(*replacement);
            if(replacement[1] == unequal)
                {
                state |= unequalBit;
                }
            //>
            // We have = in pattern as well as in replacement, so they are equal.
            state |= equalBit;
            if(state == transition) // We have seen a # at the start of 
                                    // the replacement or in the next position.
                {
                if(countdown == 1)
                    {
                    *pattern = unequal; // #
                    *replacement = unequal; // Replace the =s with #s and return
                    return true;
                    }
                --countdown;
                }
            state = equalBit; // No longer a transition.
            ++replacement; // Go past this = in replacement. (Pattern does the same further down.)
            }
        else if(*pattern == unequal)
            { // No need to look at replacement here. 
              // Maybe there is a corresponding #, maybe there isn't.
            state |= unequalBit;
            if(  state == transition // Is the previous byte in pattern a = ?
              && lastpatternEqual > 1 // ^==#=#==$ The middle = is changed 
                                // when the second # is scanned, not the first.
              )
                {
                lastpatternEqual = 0;
                if(countdown == 1)
                    {
                    pattern[-1] = unequal;
                    replacement[-1] = unequal; // Change the patternEqual'th =
                    // character in the replacement string to #.
                    return true;
                    }
                --countdown;
                }
            state = unequalBit; // If the next byte in pattern is #, we have a transition. 
                                // See start of loop.
            }
        ++pattern;
        if(!*pattern)
            {
            return false; // signal to caller that countdown has reached maxvalue.
            /*
            loc_nppattern = pattern;
            loc_npreplacement = replacement;
            patternEqual = 0;
            replacementEqual = 0;
            */
            }
        }
    return false;
    }
#endif

bool shortRulePair::checkRule(/*ruleTemplate * Template,*/trainingPair * trainingpair,rulePair * parentPat)
    {
    char Lemma[100] = "";
    char Mask[100] = "";
    fullRulePair FullRule(this);
    bool ret;
    /*
    if(parentPat)
        fprintf(flog,"parent:pat %s rep %s\n",parentPat->pattern(),parentPat->replacement());
    fprintf(flog,"this  :pat %s rep %s\n",FullRule.pattern(),FullRule.replacement());
    fprintf(flog,"trainingpair:");
    trainingpair->print(flog);
    fprintf(flog,"\n");
    */
    if(  (!parentPat || FullRule.dif(parentPat) == dif_bigger) 
      && FullRule.apply(trainingpair,sizeof(Lemma),Lemma,Mask)
      )
        {
        ret = trainingpair->isCorrect(Lemma);
        /*
        fprintf(flog,"ret:%s\n",ret ? "true" : "false");
        */
        }
    else
        {
        /*
        fprintf(flog,"ret is false\n");
        */
        ret = false;
        }
    return ret;
    }

static int storeRule(hash * Hash,shortRulePair * Rule,vertex *& V)
    {
    fullRulePair FullRule(Rule);
    bool New;
    V = Hash->getVertex(&FullRule,New);
    /*
    V->print1(flog);
    fprintf(flog,"\n");
    */
    if(New)
        {
        return 1;
        }
    else
        {
        return 0;
        }
    }

int trainingPair::makeCorrectRules(hash * Hash,ruleTemplate * Template,const char * similar,vertex * parent,int mlow, int recurse)
    {
    /*In the template, replace one = with a # and construct a rule based on
    this new template. If the rule succeeds, store it. Repeat this for all
    places were replacement is allowed. (What should be allowed is a matter
    of heuristics.)*/
    int ret = 0;
    bool different = true;
    ruleTemplate locTemplate;
    locTemplate.copy(Template);
    for( int m = mlow
       ; locTemplate.makebigger(m)
       ;   ++m
         , locTemplate.copy(Template)
       )
        {
        shortRulePair Rule(this,&locTemplate);
        if(Rule.checkRule(/*&locTemplate,*/this,parent))
            {
            different = false;
            vertex * e;
            int New = storeRule(Hash,&Rule,e);
            /** /
            fprintf(flog,"makeCorrectRules:");
            e->print1(flog);
            fprintf(flog,"\n");
            */
            ret += New;
            }
        }
    /*If no rule with one more character in the search pattern succeeds,
    repeat the procedure with two instead of one additional character,
    and so on.*/
    if(different || --recurse > 0)
        {
        locTemplate.copy(Template);
        for( int m = mlow
           ; locTemplate.makebigger(m)
           ;   ++m
             , locTemplate.copy(Template)
           )
            {
            /*Recurse with template that has one more #*/
            ret += makeCorrectRules(Hash,&locTemplate,similar,parent,m+1,recurse);
            }
        }
    /*else
        assert(ret);*/
    return ret;
    }

int trainingPair::makeRuleEx(hash * Hash,vertex * parent)
    {
    /*
    this->print(flog);
    */
    char similarArray[1000];
    similData data(similarArray);
    data.match(this);
    /*
    data.print(flog);
    */
    const char * predef = getMask();
    /*
    fprintf(flog,"predef %s\n",predef);
    */
    data.mergeTemplates(predef);
    /*
    fprintf(flog,"mergeTemplates DONE:\n");
    data.print(flog);
    */
    shortRulePair Rule(this,&data);
    if(Rule.checkRule(/*&data,*/this,parent))
        {
        vertex * e;
        int ret = storeRule(Hash,&Rule,e);
        /** /
        fprintf(flog,"makeRuleEx:");
        e->print1(flog);
        fprintf(flog,"\n");
        */
        return ret;
        }
    return makeCorrectRules(Hash,&data,similarArray,parent,1,RECURSE);
    }

static bool isSpace(int a)
    {
    switch(a)
        {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;
        default:
            return false;
        }
    }

static int readLines(int lines,struct aFile * afile,trainingPair * TrainingPair,const char * columns) 
// "123456" means Word, Lemma, Wordfreq, Lemmafreq, Wordclass, Lemmaclass
    {
    int pairs = 0;
    int line;
    pointers * Lines = afile->Lines;
    for(line = 0;line < lines;++line)
        {
        const char * cols[8];
        const char * Word = NULL;
        ptrdiff_t lengths[8];
        size_t wordlength = 0;
#if WORDCLASS
        const char * WordClass = NULL; // unused
        size_t wordclasslength = 0; // unused
#endif
        const char * LemmaHead = NULL;
        size_t lemmalength = 0;
#if LEMMACLASS
        const char * LemmaClass = NULL;
        size_t lemmaclasslength = 0;
#endif
#if LEMMAINL
        long Inl = 0;
        long Lemma_Inl = 0;
#endif
        const char * limit = (line < lines - 1) ? Lines[line+1].cchars : afile->eob;

        const char * q = Lines[line].cchars;
        cols[0] = q;
        unsigned int ii = 0;
        while(  (q = find(q,'\t',limit)) != NULL
            && (ii < sizeof(cols)/sizeof(cols[0]) - 1)
            )
            {
            lengths[ii] = q - cols[ii];
            cols[++ii] = ++q;
            }
        lengths[ii] = limit - cols[ii] - 1;
        unsigned int maxii = ++ii;
        cols[maxii] = q ? q : limit;
        const char * column;
        for(column = columns,ii = 0
            ;    *column 
            && (ii < maxii)
            ;++column,++ii
            )
            {
            switch(*column)
                {
                case '1':
                    Word = cols[ii];
                    wordlength = lengths[ii];
                    break;
                case '2':
                    LemmaHead = cols[ii];
                    lemmalength = lengths[ii];
                    break;
#if LEMMAINL
                case '3':
                    Inl = strtol(cols[ii],NULL,10);
                    break;
                case '4':
                    Lemma_Inl = strtol(cols[ii],NULL,10);
                    break;
#endif
#if WORDCLASS
                case '5':
                    WordClass = cols[ii];
                    wordclasslength = lengths[ii];
                    break;
#endif
#if LEMMACLASS
                case '6':
                    LemmaClass = cols[ii];
                    lemmaclasslength = lengths[ii];
                    break;
#endif
                }
            }
        if(Word)
            {
            while(lemmalength > 0 && isSpace(LemmaHead[lemmalength - 1]))
                --lemmalength;
#if LEMMACLASS
            while(lemmaclasslength > 0 && isSpace(LemmaClass[lemmaclasslength - 1]))
                --lemmaclasslength;
#endif
            TrainingPair[pairs].init(Word,LemmaHead
#if LEMMACLASS
                ,LemmaClass
#endif
                ,wordlength,lemmalength
#if LEMMACLASS
                ,lemmaclasslength
#endif
#if LEMMAINL
                ,Inl,Lemma_Inl
#endif
                );
            ++pairs;
            }
        }
    return pairs;
    }

static int compare(const void * arg1, const void * arg2)
    {
    const trainingPair * A = *(const trainingPair * const *)arg1;
    const trainingPair * B = *(const trainingPair * const *)arg2;
    int ret = A->cmpWord(B);
    if(!ret)
        {
        ret = A->cmpLemma(B);
#if LEMMACLASS
        if(!ret)
            {
            ret = A->cmpLemmaClass(B);
            }
#endif
        }
    return ret;
    }

static int markAmbiguous(int allPairs,trainingPair * TrainingPair,FILE * famb)
    {
    trainingPair ** pTrainingPair = new trainingPair * [allPairs];
    int j;
    for(j = 0;j < allPairs;++j)
        pTrainingPair[j] = TrainingPair + j;
    qsort((void *)pTrainingPair, allPairs, sizeof(trainingPair *), compare);
    
    for(j = 0;j < allPairs;++j)
        // Mark words containing space for skipping
        {
        size_t nn = pTrainingPair[j]->itsWordlength();
        const char * s;
        // do not use words containing space
        for(s = pTrainingPair[j]->itsWord();nn-- > 0;)
            {
            if(s[nn] == ' ')
                {
                pTrainingPair[j]->set(b_skip);
                }
            }
        }
    int n = 0;

    for(j = 0;j < allPairs;// - 1;
#if AMBIGUOUS
#else
        ++j
#endif
        )
        {
        int k;
#if AMBIGUOUS
        trainingPair * pAlt = pTrainingPair[j];
        if(famb)
            {
            if(!pTrainingPair[j]->isset(b_skip))
                pTrainingPair[j]->fprint(famb);
            }
#endif
        for(k = j+1
           ;    k < allPairs
             && !pTrainingPair[j]->isset(b_skip)
             && !pTrainingPair[j]->cmpWord(pTrainingPair[k])
           ; ++k
           )
            {
            if(pTrainingPair[j]->cmpLemma(pTrainingPair[k]) /*|| pTrainingPair[j]->cmpLemmaClass(pTrainingPair[k])*/) //TODO Tags!
                {
#if AMBIGUOUS
                if(!pTrainingPair[k]->isset(b_skip))
                    {
                    pTrainingPair[k]->set(b_ambiguous);
                    pAlt->Alt = pTrainingPair[k];
                    pAlt = pTrainingPair[k];
                    if(famb)
                        {
                        fprintf(famb,"HOMOGRAPH:");
                        pTrainingPair[k]->fprint(famb);
                        }
                    }
#else
                switch(pTrainingPair[j]->cmpFreq(pTrainingPair[k]))
                    {
                    case 1:
                        pTrainingPair[k]->set(b_skip);
                        break;
                    case -1:
                        pTrainingPair[j]->set(b_skip);
                        break;
                    default:
                        /* Take the first one */
                        if(!pTrainingPair[k]->isset(b_ambiguous|b_skip))
                            {
                            pTrainingPair[k]->set(b_ambiguous);
                            if(famb)
                                pTrainingPair[k]->fprint(famb);
                            ++n;
                            }
                    }
#endif
                }
            else
                pTrainingPair[k]->set(b_doublet|b_skip);
            }
#if AMBIGUOUS
        // close the ring of homographs
        if(pAlt != pTrainingPair[j])
            {
            pTrainingPair[j]->set(b_ambiguous); // ALL members in the ring are marked b_ambiguous!
            pAlt->Alt = pTrainingPair[j];
            }
        j = k;
#else
//        if(allFile && j < allPairs)
  //          pTrainingPair[j]->fprintAll(allFile);
#endif
        }
    FILE * allFile = fopen("allFile.txt","wb");
    if(allFile)
        {
        for(j = 0;j < allPairs;++j)
            {
            if(!pTrainingPair[j]->isset(b_skip))
                pTrainingPair[j]->fprintAll(allFile);
            }
        fclose(allFile);
        }
    delete [] pTrainingPair; // Bart 20081008
    return n;
    }

#if PESSIMISTIC
static int compare2(const void * arg1, const void * arg2)
    {
    const trainingPair * A = *(const trainingPair * const *)arg1;
    const trainingPair * B = *(const trainingPair * const *)arg2;
    int ret = A->cmpLemma(B);
    if(!ret)
        {
        ret = A->cmpLemmaClass(B);
        if(!ret)
            {
            ret = A->cmpWord(B);
            }
        }
    return ret;
    }

static int markParadigms(int allPairs,trainingPair * TrainingPair,FILE * fparadigms)
    {
//    FILE * allFile = fopen("allFile.txt","wb");
    trainingPair ** pTrainingPair = new trainingPair * [allPairs];
    int j;
    for(j = 0;j < allPairs;++j)
        pTrainingPair[j] = TrainingPair + j;
    qsort((void *)pTrainingPair, allPairs, sizeof(trainingPair *), compare2);
    
    int n = 0;
    for(j = 0;j < allPairs/* - 1*/;)
        {
        int k;
        trainingPair * pAlt = pTrainingPair[j];
        if(fparadigms)
            {
            if(!pTrainingPair[j]->isset(b_skip))
                pTrainingPair[j]->fprint(fparadigms);
            }
        for(k = j+1
           ;    k < allPairs
             && !pTrainingPair[j]->isset(b_skip)
             && !pTrainingPair[j]->cmpLemma(pTrainingPair[k])
           ; ++k
           )
            {
            if(!pTrainingPair[k]->isset(b_skip))
                {
                pAlt->AltLemma = pTrainingPair[k];
                pAlt = pTrainingPair[k];
                if(fparadigms)
                    {
                    fprintf(fparadigms,"PARADIGM:");
                    pTrainingPair[k]->fprint(fparadigms);
                    }
                }
            }
        if(pAlt != pTrainingPair[j])
            {
            pAlt->AltLemma = pTrainingPair[j];
            }
        j = k;
        }
    delete [] pTrainingPair; // Bart 20081008
    return n;
    }
#endif

// Mark all pairs that are going to be used for testing after the training has completed. (If there is such testing.)
static void markTest(int allPairs,trainingPair * TrainingPair,int percent,FILE * ftrain,FILE * ftestp,FILE * ftests)
    {
    int pairs = 0;
    for(pairs = 0;pairs < allPairs;++pairs)
        {
        if(!TrainingPair[pairs].isset(
#if !AMBIGUOUS
            b_ambiguous|
#endif
            b_doublet|b_skip))
            {
            if(rand() % 100 <= percent)
                {
                TrainingPair[pairs].set(b_test);
                TrainingPair[pairs].print(ftestp);
                fprintf(ftestp,"\n");
                fprintf(ftests,"%.*s\n",TrainingPair[pairs].itsWordlength(),TrainingPair[pairs].itsWord());
                }
            else
                {
                TrainingPair[pairs].print(ftrain);
                fprintf(ftrain,"\n");
                }
            }
        }
    }



static void writeAvailableTrainingData(int allPairs,trainingPair * TrainingPair,FILE * ftrainavailable
#if WRITEINVERTED
                                       ,FILE * fpinverted
#endif
                                       )
    {
    int pairs = 0;
    for(pairs = 0;pairs < allPairs;++pairs)
        {
#if AMBIGUOUS
        if(!TrainingPair[pairs].isset(b_doublet|b_skip))
#else
        if(!TrainingPair[pairs].isset(b_ambiguous|b_doublet|b_skip))
#endif
            {
            TrainingPair[pairs].fprintTraining(ftrainavailable
#if WRITEINVERTED
                ,fpinverted
#endif
                );
            }
        }
    }

int trainingPair::makeNextTrainingSet
        ( int allPairs
        , trainingPair * TrainingPair
        , FILE * train
        , FILE * done
        , FILE * combined
        , FILE * disamb
        )
    {
    int pairs = 0;
    int donepairs = 0;
    for(pairs = 0;pairs < allPairs;++pairs)
        {
#if AMBIGUOUS
        if(!TrainingPair[pairs].isset(b_doublet|b_skip))
#else
        if(!TrainingPair[pairs].isset(b_ambiguous|b_doublet|b_skip))
#endif
            {
            if(!TrainingPair[pairs].isset(b_ambiguous))
                {
                TrainingPair[pairs].printMore(train);
                TrainingPair[pairs].printMore(disamb);
                fprintf(combined,"\t");
                TrainingPair[pairs].printMore(combined);
                }
            else if(TrainingPair[pairs].isset(b_bench))
                {
                TrainingPair[pairs].printMore(train);
                fprintf(combined,"\t");
                TrainingPair[pairs].printMore(combined);
                }
            else
                {
                donepairs++;
                TrainingPair[pairs].printMore(done);
                TrainingPair[pairs].printMore(disamb);
                fprintf(combined,"*\t");
                TrainingPair[pairs].printMore(combined);
                }
            }
        }
    return donepairs;
    }

void trainingPair::makeChains(int allPairs,trainingPair * TrainingPair,trainingPair ** train,trainingPair ** test)
    {
    int pairs = 0;
    trainingPair ** ptrain = train;
    trainingPair ** ptest = test;
    for(pairs = 0;pairs < allPairs;++pairs)
        {
#if AMBIGUOUS
        if(!TrainingPair[pairs].isset(b_doublet|b_skip))
#else
        if(!TrainingPair[pairs].isset(b_ambiguous|b_doublet|b_skip))
#endif
            {
            if(TrainingPair[pairs].isset(b_test))
                {
                *ptest = TrainingPair + pairs;
                ptest = &TrainingPair[pairs].Next;
                }
            else
                {
                *ptrain = TrainingPair + pairs;
                ptrain = &TrainingPair[pairs].Next;
                }
            }
        }
    }

#if DOTEST
// Testing. (Not called during training!)
static void showResults(trainingPair * TrainingPair,int & wrong,int & right,int & both,FILE * fr)
    {
    for(trainingPair * tp = TrainingPair;tp;tp = tp->next())
        {
        if(!tp->isset(b_ambiguous|b_doublet|b_skip)) // b_ambiguous is ok if AMBIGUOUS == 1, or what?
            {
                {
                if(tp->isset(b_wrong))
                    {
                    if(tp->isset(b_ok))
                        {
                        if(fr)
                            {
                            fprintf(fr,"/ ");
                            tp->fprint(fr);
                            }
                        both++;
                        }
                    else
                        {
                        if(fr)
                            {
                            fprintf(fr,"- ");
                            tp->fprint(fr);
                            }
                        wrong++;
                        }
                    }
                else if(tp->isset(b_ok))
                    {
                    if(fr)
                        {
                        fprintf(fr,"+ ");
                        tp->fprint(fr);
                        }
                    right++;
                    }
                tp->unset(b_ok|b_wrong);
                }
            }
        }
    }

// Testing. (Not called during training!)
static void lemmatise(node * tree,trainingPair * TrainingPair)
    {
    /*
    printf("lemmatise\n");
    */
    for(trainingPair * tp = TrainingPair;tp;tp = tp->next())
        {
        if(!tp->isset(b_ambiguous|b_doublet|b_skip)) // Skip all homographs, even if AMBIGUOUS == 1 !
            {
            //tp->print(stdout);printf("\n");
            tree->lemmatise(tp);
            }
        }
    /*
    printf("lemmatise DONE\n");
    */
    }
static FILE * fresults = NULL;
static double maxCorrectness = 0.0;
#endif

static trainingPair * globTrainingPair;
static int globlines;



static trainingPair * readTrainingPairs(struct aFile * afile,int & pairs,const char * columns)
    {
    globlines = afile->lines;
    trainingPair * TrainingPair = new trainingPair[afile->lines];
    globTrainingPair = TrainingPair;
    pairs = readLines(afile->lines,afile,TrainingPair,columns);
    printf("\n%ld characters and %d lines read\n",afile->size,afile->lines);
    return TrainingPair;
    }

static void markTheAmbiguousPairs(trainingPair * TrainingPair,const char * ext,int pairs)
    {
    char filename[256];
    sprintf(filename,"ambiguouspairs_%s.txt",ext);
    FILE * famb = fopen(filename,"wb");
    /*int ambi =*/ markAmbiguous(pairs,TrainingPair,famb);
    if(famb)
        fclose(famb);
#if PESSIMISTIC
    sprintf(filename,"paradigms_%s.txt",ext);
    FILE * fparadigms = fopen(filename,"wb");
    markParadigms(pairs,TrainingPair,fparadigms);
    if(fparadigms)
        fclose(fparadigms);
#endif
    printf("%s written\n",filename);
    }

static void writeAllAvailablePairs(trainingPair * TrainingPair,const char * ext,int pairs)
    {
    char train[256];
    sprintf(train,"availabletrainingpairs_%s.txt",ext);
    FILE * ftrain = fopen(train,"wb");

#if WRITEINVERTED
    sprintf(train,"availabletrainingpairs_inverted_%s.txt",ext);
    FILE * fpinverted = fopen(train,"wb");
#endif
    if(ftrain 
#if WRITEINVERTED
        && fpinverted
#endif
        )
        {
        writeAvailableTrainingData(pairs,TrainingPair,ftrain
#if WRITEINVERTED
            ,fpinverted
#endif
            );
        }
    if(ftrain)
        {
        fclose(ftrain);
        ftrain = NULL;
        }
#if WRITEINVERTED
    if(fpinverted)
        {
        fclose(fpinverted);
        ftrain = NULL;
        }
#endif
    //printf("FILES WRITTEN\n");
    //getchar();
    }

static void writeAllTestPairs(trainingPair * TrainingPair,const char * ext,int pairs)
    {
    char train[256];
    char testp[256];
    char tests[256];
    sprintf(train,"trainingpairs_%s.txt",ext);
    FILE * ftrain = fopen(train,"wb");

    sprintf(testp,"testpairs_%s.txt",ext);
    FILE * ftestp = fopen(testp,"wb");
    sprintf(tests,"tests_%s.txt",ext);
    FILE * ftests = fopen(tests,"wb");
    if(ftrain && ftestp && ftests)
        {
        markTest(pairs,TrainingPair,PERC,ftrain,ftestp,ftests);
        printf("%s %s %s written\n",train,testp,tests);
        }
    if(ftrain)
        fclose(ftrain);
    if(ftestp)
        fclose(ftestp);
    if(ftests)
        fclose(ftests);
    }
/*
static void checkTrainingPairIntegrity()
    {
    int ii;
    for(ii = 0;ii < globlines;++ii)
        if(!globTrainingPair[ii].isset(b_ambiguous|b_doublet|b_test|b_skip))
            globTrainingPair[ii].checkIntegrity();
    }
*/
static void doTheRules(hash * Hash,trainingPair * TrainingPair,node ** top)
    {
    fullRulePair ROOT("^*$","^*$");
    bool New;
    vertex * best = Hash->getVertex(&ROOT,New);

    *top = new node(best);
    trainingPair * Right = NULL;
    (*top)->init(&Right,&TrainingPair,0/*,0,0*/);
    (*top) = (*top)->cleanup(NULL);
    }


/* Create binary output */
static void rearrange
        ( char * filename
        , FILE * folel
#if RULESASTEXT
        , FILE * foleltxt
#endif
        )
    {
    FILE * fo = fopen(filename,"wb"); // output
    if(!fo)
        return;
    long end;
    end = ftell(folel);
    char * buf = new char[end+1]; // contents of folel, a textual file.
    rewind(folel);
    fread(buf,1,end,folel);
    unsigned int n = 0;
    int i;
    for(i = 0;i < end;++i)
        if(buf[i] == '\n')
            ++n;
    char ** pbuf = new char * [n+1]; // lines
    unsigned int * size = new unsigned int[n+1]; // binary size taken up by each rule
    unsigned int * cumsize = new unsigned int[n+1]; // idem, cumulative
    int * ind = new int[n+1]; // nesting levels, also used to store cumulative sizes
    pbuf[0] = buf+0;
    n = 0;
    bool doind = true;  // first field on line is an indentation (or nesting
                        // level) number
    ind[0] = 0;
    cumsize[0] = 0;
    size[0] = 0;
    for(i = 0;i < end;++i)
        {
        if(buf[i] == '\n')
            {
            size[n] += sizeof(int) + sizeof(int); // room for index and '\0' byte
            size[n] >>= 2;
            size[n] <<= 2; // rounded to nearest word boundary
            pbuf[++n] = buf+i+1; // start of next line
            cumsize[n] = size[n-1];
            cumsize[n] += cumsize[n-1];
            size[n] = 0;
            doind = true; // read first data on line as nesting level
            ind[n] = 0; // initialize nesting level to 0
            }
        else if(buf[i] == '\t')
            {
            if(doind)
                doind = false;
            else
                size[n]++;
            }
        else if(doind)
            {// read nesting level
            ind[n] *= 10;
            ind[n] += buf[i] - '0';
            }
        else
            size[n]++;
        }
    pbuf[n] = NULL;
    int lev[50];
    unsigned int j;
    for(j = 0;j < sizeof(lev)/sizeof(lev[0]);++j)
        lev[j] = 0;
    for(j = 0;j < n;++j)
        {
        int oldj = lev[ind[j]]; // find previous sibling
        if(oldj)
            ind[oldj] = cumsize[j]; // tell previous sibling where its next 
                                    // sibling is
        for(int k = ind[j]+1;k < 50 && lev[k];++k)
            {
            lev[k] = 0; // forget about previous sibling'c children
            }
        lev[ind[j]] = j; // update who's the current node at this level.
        ind[j] = 0; // the level information is not needed anymore. Space can 
                    // be reused to keep the position to the next sibling 
                    // (if any).
        }
    char * p = buf;
    for(j = 0;j < n;++j)
        {
        long pos = ftell(fo);
#ifdef _DEBUG
        long ppos = cumsize[j];
        assert(pos == ppos);
#endif
        if(ind[j] >= pos)
            ind[j] -= pos; // We only need to know how far to jump from here.
        fwrite(ind+j,sizeof(int),1,fo);
#if RULESASTEXT
        fprintf(foleltxt,"%d",ind[j]);
#endif
        unsigned int written = sizeof(int);

        while(*p && *p != '\t')
            ++p;
        assert(*p);
        ++p;
        while(*p != '\n')
            { // write the pattern and the replacement (which are intertwined)
#if RULESASTEXT
            fputc(*p,foleltxt);
#endif
            fputc(*p++,fo);
            written++;
            }
#if RULESASTEXT
        fputc(*p,foleltxt); 
#endif
        fputc(*p++,fo); // write the end-of-line marker
        written++;
        assert(written <= size[j]);
        assert(written > size[j]-sizeof(int));
        while(written < size[j]) // write until word boundary hit
            {
#if RULESASTEXT
            fputc('\n',foleltxt);
#endif
            fputc('\n',fo);
            written++;
            }
        }
    fclose(fo);
    delete [] buf;
    delete [] size;
    delete [] cumsize;
    delete [] ind;
    }

int Nnodes = 0;

static bool writeAndTest(node * tree,const char * ext,int threshold,char * nflexrules)
    {
    char filename[256];
#if RULESASTEXTINDENTED
    sprintf(filename,"tree_%d%s.txt",threshold,ext);
    FILE * foo = fopen(filename,"wb");
    if(foo)
#endif
        {
        Nnodes = 0;
#if RULESASTEXTINDENTED
        fprintf(foo,"threshold %d\n",threshold);
        int NnodesR = 0;
        int N = tree->print(foo,0,Nnodes,NnodesR);
        fprintf(foo,"threshold %d:%d words %d nodes %d nodes with words\n\n",threshold,N,Nnodes,NnodesR);
        fclose(foo);
        sprintf(filename,"rules_%d%s.txt",threshold,ext);
        FILE * foo = fopen(filename,"wb");
#else
        Nnodes = tree->count();
#endif
        sprintf(filename,"numberOfRules_%d.txt",threshold);
        FILE * fono = fopen(filename,"wb");
#if BRACMATOUTPUT
        sprintf(filename,"rules_%d%s.bra",threshold,ext);
        FILE * fobra = fopen(filename,"wb");
#endif
        sprintf(filename,"rules_%d%s.lel",threshold,ext);
        FILE * folel = fopen(filename,"wb+");
#if RULESASTEXT
        filename[strlen(filename)-1] += 2; // change ".lel" to ".len"
        FILE * foleltxt = fopen(filename,"wb");
        filename[strlen(filename)-1] -= 2; // change ".len" back to ".lel"
#endif
        if(    fono  
#if RULESASTEXTINDENTED
            && foo
#endif
#if BRACMATOUTPUT
            && fobra 
#endif
            && folel 
#if RULESASTEXT
            && foleltxt
#endif
            )
            {
#if RULESASTEXTINDENTED
            fprintf(foo,"tree={%d}\n",Nnodes); // "rules_%d%s.txt"
#endif
            /*
            if(compute_parms)
                {
                FILE * f = fopen("parms.txt","a");
                fprintf(f,"%d",Nnodes);
                fprintf(f,"\n");
                fclose(f);
                }
            */
            fprintf(fono,"%d",Nnodes); // "numberOfRules_%d.txt"
            fclose(fono);
            int nr = 0;
            strng L("");
            strng R("");
            tree->printRules
                ( tree
#if RULESASTEXTINDENTED
                , foo // "rules_%d%s.txt"
#endif
#if BRACMATOUTPUT
                , fobra // "rules_%d%s.bra"
#endif
                , folel // "rules_%d%s.lel"
                , 0
                , &L
                , &R
                , nr
                );
#if BRACMATOUTPUT
            fclose(fobra);
#endif
#if RULESASTEXTINDENTED
            fclose(foo);
#endif
            if(nflexrules)
                {
//                printf("rearrange %d %s\n",threshold,nflexrules);
                rearrange
                    ( nflexrules// the binary output of the training, 
                                // third command line argument
                    , folel     // .lel file, textual output with relative 
                                // positions
#if RULESASTEXT
                    , foleltxt  // .len file, textual output with absolute 
                                // positions, like in the binary output.
#endif
                    );
//                printf("rearrange done %d %s\n",threshold,nflexrules);
                }
            else
                {
                filename[strlen(filename)-1]++; // change ".lel" to ".lem"
//                printf("rearrange %d %s\n",threshold,filename);
                rearrange
                    ( filename  // .lem file, the binary output of the training
                    , folel     // .lel file, textual output with relative 
                                // positions
#if RULESASTEXT
                    , foleltxt  // .len file, textual output with absolute 
                                // positions, like in the binary output.
#endif
                    );
//                printf("rearrange done %d %s\n",threshold,filename);
                filename[strlen(filename)-1]--; // change ".lem" back to ".lel"
                }
            fclose(folel);
            if(remove(filename)) // del ".lel"
                {
                printf("cannot remove %s\n",filename);
                getchar();
                }
//            else
  //              printf("removed %s\n",filename);
#if RULESASTEXT
            fclose(foleltxt);
#endif
            return true;
            }
        else
            {
#if RULESASTEXTINDENTED
            if(foo)
                fclose(foo);
#endif
#if BRACMATOUTPUT
            if(fobra)
                fclose(fobra);
#endif
            if(folel)
                fclose(folel);
#if RULESASTEXT
            if(foleltxt)
                fclose(foleltxt);
#endif
            }
        }
    return false;
    }

#if DOTEST
static void testf(node * tree,trainingPair * TestPair,const char * ext,int threshold,char * nflexrules)
    {
    char filename[256];
    int wrong = 0;
    int right = 0;
    int both = 0;
    if(TestPair) // requires PERC > 0
        {
        lemmatise(tree,TestPair);
        sprintf(filename,"test_%d%s.txt",threshold,ext);
        FILE * ftest = fopen(filename,"ab+");
        showResults(TestPair,wrong,right,both,ftest);
        int tot = right+wrong+both;
        if(ftest)
            fprintf(ftest,"test pairs %d threshold %d vertices %d right %d (%f) wrong %d (%f) both %d (%f)\n\n",tot,threshold,tree->count(),right,(right*100.0)/tot,wrong,(wrong*100.0)/tot,both,(both*100.0)/tot);
        fresults = fopen("results.txt","ab+");
        if(fresults)
            {
            fprintf(fresults,"tot %d\tthreshold %d\ttree->count() %d\tright %d\t%% %f\twrong %d\t%% %f\n",tot,threshold,tree->count(),right,(right*100.0)/tot,wrong,(wrong*100.0)/tot);
            }
        if(maxCorrectness < (right*100.0)/tot)
            {
            maxCorrectness = (right*100.0)/tot;
            if(fresults)
                {
                fprintf(fresults,"\nhighest correctness %f\n",maxCorrectness);
                }
            }
        if(fresults)
            fclose(fresults);
        if(ftest)
            fclose(ftest);
        }
    else // requires PERC <= 0
        {
        sprintf(filename,"rules_%d%s.lem",threshold,ext);
        printf("readRules %d\n",threshold);
        if(readRules(filename) || readRules(nflexrules))
            {
            printf("readRules done %d\n",threshold);
            char word[100];
            printf("\nType first word:\n");
            while(gets(word)[0])
                {
                const char * result = applyRules(word);
                printf("%s\n",result);
                printf("\nType word:\n");
                }
            deleteRules();
            }
        }
    }
#endif

static bool doTraining
        ( struct aFile * afile
        , const char * ext
        , int cutoff
        , char * nflexrules
        , const char * columns
        , char * nexttrainname
        , char * donename
        , char * combinedname
        , char * disambtrainname
        )
    {
    bool moreToDo = false;
    char filename[256];
    VertexPointerCount = 0;

    int pairs;
    trainingPair * TrainingPair = readTrainingPairs(afile,pairs,columns);
    markTheAmbiguousPairs(TrainingPair,ext,pairs);
    writeAllAvailablePairs(TrainingPair,ext,pairs);
    if(PERC > 0)
        writeAllTestPairs(TrainingPair,ext,pairs);
    hash Hash(10);
    //int pairsStart = 0;
    trainingPair * train = NULL;
    trainingPair * test = NULL;
    // Split list of pairs in those that are to be used for training and those
    // that are to be used for testing.
    // Pairs that are doublets are not added to either list.
    // Nor are pairs that are not well-formed (e.g. contain a ' ').
    trainingPair::makeChains(pairs,TrainingPair,&train,&test);
#if DOTEST
    FILE * ftest;
    sprintf(filename,"test_%s.txt",ext);
    ftest = fopen(filename,"wb");
    if(ftest)
        fclose(ftest);
#endif
    node * top;
    doTheRules(&Hash,train,&top);

    FILE * nexttrain = nexttrainname ? fopen(nexttrainname,"wb") : NULL;
    FILE * done = donename ? fopen(donename,"wb") : NULL;
    FILE * combined = combinedname ? fopen(combinedname,"wb") : NULL;
    FILE * disamb = disambtrainname ? fopen(disambtrainname,"wb") : NULL;
    if(nexttrain && done && combined && disamb)
        {
        int donepairs = trainingPair::makeNextTrainingSet(pairs,TrainingPair,nexttrain,done,combined,disamb);
        if(donepairs)
            {
            printf("More training to do with file \"%s\"\n",nexttrainname);
            moreToDo = true;
            }
        else
            printf("No more training to do\n");
        }
    if(nexttrain)
        fclose(nexttrain);
    if(done)
        fclose(done);
    if(combined)
        fclose(combined);
    if(disamb)
        fclose(disamb);
    sprintf(filename,"words_%s.txt",ext);
    FILE * wordsFile = fopen(filename,"wb");
    if(wordsFile)
        {
        ambivalentWords = 0;
        alternatives = 0;
        allwords = 0;
        top->printSep(wordsFile,0);
        fprintf(wordsFile
            ,"\nAll words: %d    Words with alternative lemmas: %d    Words that are not lemmatized correctly: %d\n"
            ,allwords
            ,ambivalentWords
            ,alternatives);
        fclose(wordsFile);
        }
    FILE * fcounting = fopen("counting.tab","wb");
    if(fcounting)
        {
        fprintf(fcounting,"Bottom up left to right traversal of tree.\n");
        fprintf(fcounting,"Nodes\tPairs\tlog(Nodes)\tlog(Pairs)\n");

        int nodes = 0,pairs = 0;
        top->Counting(nodes,pairs,fcounting);
        fclose(fcounting);
        }
    if(cutoff >= 0 && nflexrules)
        {
        char naam[500];
        sprintf(naam,"%s0",nflexrules);
        writeAndTest(top,ext,0,naam);
#if DOTEST
        testf(top,test,ext,0,naam);
#endif
        for(int thresh = 1;thresh <= cutoff;thresh++)
            {
            top->pruneAll(thresh);
            top = top->cleanup(NULL);
            sprintf(naam,"%s%d",nflexrules,thresh);
            writeAndTest(top,ext,thresh,naam);
#if DOTEST
            testf(top,test,ext,0,naam);
#endif
            }
        }
    else
        {
        writeAndTest(top,ext,0,nflexrules);
#if DOTEST
        testf(top,test,ext,0,nflexrules);
#endif
        for(int thresh = 1;thresh < 3;thresh++)
            {
            top->pruneAll(thresh);
            writeAndTest(top,ext,thresh,nflexrules);
#if DOTEST
            testf(top,test,ext,thresh,nflexrules);
#endif
            }
        }
    delete top;
    building = false; // Signal to ~vertexPointer() to not access nodes.
    delete [] TrainingPair;
    building = true; // Signal to ~vertexPointer() to access nodes.
//    printf("delete [] TrainingPair DONE\n");
    return moreToDo;
    }

FILE * flog = NULL;



FILE *fplog;

int main(int argc,char **argv)
    {
    flog = fopen("flog.txt","wb");
    clock_t start, finish;
    double  duration = 0;
    char * nflexrules = NULL;
    int cutoff = -1;
    char * extra = NULL;
    //const char * columns = "12634"; // Word, Lemma, LemmaClass, WordFreq, LemmaFreq
    const char * columns = "123"; // Word, Lemma, other
    comp = comp_koud;
    if(argc < 2)
        {
        printf("usage: makeaffixrules <word list> [<cutoff> [<flexrules> [<extra> [<columns> [<compfunc>]]]]]\n");
        printf("columns:\n");
        printf("  1:Word\n");
        printf("  2:Lemma\n");
        printf("  3:Word Frequency\n");
        printf("  4:Lemma Frequency\n");
        printf("  5:Word Class\n");
        printf("  6:Lemma Class\n");
        printf("Compfuncs:\n");
        printf("  1:fairly_good\n");
        printf("  2:even_better (Icelandic)\n");
        printf("  3:affiksFEW3 (Slovene)\n");
        printf("  4:affiksFEW\n");
        printf("  5:affiksFEW2 (Dutch, Norwegian)\n");
        printf("  6:fixNA\n");
        printf("  7:fruit\n");
        printf("  8:ice\n");
        printf("  9:pisang\n");
        printf("  10:kiwi\n");
        printf("  11:carrot\n");
        printf("  12:peen (Danish, Polish)\n");
        printf("  13:beet\n");
        printf("  14:sugar (English, Greek, German, Swedish, Russian)\n");
        printf("  15:affiksFEW2org ()\n");
        printf("  16:honey ()\n");
        printf("  17:koud ()\n");
        }
    else 
        {
        printf("Wordlist:%s\n",argv[1]);
        if(argc > 2)
            {
            PERC = -1; // set to positive value if you want to set PERC percent
                       // of the avaliable data aside for testing.
            cutoff = *argv[2] - '0';
            if(cutoff > 9 || cutoff < 0)
                cutoff = -1;
            else if(argc > 3)
                {
                nflexrules = argv[3];
                }
            printf("cutoff:%d\n",cutoff);
            printf("flex rules:%s\n",nflexrules ? "automatically generated names" : nflexrules);
            if(argc > 4)
                {
                extra = argv[4];
                printf("extra name suffix:%s\n",extra);
                if(argc > 5)
                    {
                    columns = argv[5];
                    printf("columns:%s (1=word,2=lemma,3=other)\n",columns);
                    if(argc > 6)
                        {
                        printf("competition function:%s\n",argv[6]);
                        if(!setCompetitionFunction(argv[6]))
                        
                            {
                            printf("Unknown competition function %s\n",argv[6]);
                            exit(2);
                            }
                        }
                    }
                }
            }
        }

    start = clock();
    int iterations = 0;
    int currentNo = 0;
    int brownNo = 0;
    do
        {
        FILE * f = fopen("parms.txt","a");
        fprintf(f,"iteration:%d\n",iterations);
        fclose(f);
        bool moreToDo = true;
        if(compute_parms)
            {
            cutoff = 0;
            if(currentNo == 0)
                init();
            else
                brown(iterations);
            }
        char * fname = argv[1];
        int passes = 0;
        char nexttrainname[256] = "__nexttrain";
        char donename[256] = "__done";
        char combinedname[256] = "__combined";
        char disambtrainname[256] = "__disambtrain";
        char command[200];
        do
            {
            ++passes;
            disambtrainname[0] = combinedname[0] = donename[0] = (char)(passes+'0');
            struct aFile * afile = readFile(fname);
            nexttrainname[0] = (char)(passes+'0');
            char ext[100];
            ext[0] = '\0';
            if(extra)
                strcpy(ext,extra);
            sprintf(ext,"%s%d",extra ? extra : "",passes);

            if(afile)
                {
                moreToDo = doTraining(afile,ext,cutoff,nflexrules,columns,nexttrainname,donename,combinedname,disambtrainname);
                //printf("CNT = %d\n",CNT);
                delete afile;
                afile = NULL;
                if(SLOW && moreToDo && !compute_parms)
                    {
                    afile = readFile(disambtrainname);
                    if(doTraining(afile,ext,cutoff,nflexrules,columns,NULL,NULL,NULL,NULL))
                        {
                        printf("This should return 0\n");
                        getchar();
                        }
                    delete afile;
                    afile = NULL;
                    }
                }

            char filename[256];
            sprintf(filename,"statistics_%s.txt",ext);
            FILE * fo = fopen(filename,"ab");
            if(fo)
                {
                fprintf(fo,"VertexPointerCount          %d\n",VertexPointerCount          );
                fprintf(fo,"VertexCount                 %d\n",VertexCount                 );
                fprintf(fo,"TrainingPairCount           %d\n",TrainingPairCount           );
                fprintf(fo,"HashCount                   %d\n",HashCount                   );
                fprintf(fo,"RulePairCount               %d\n",RulePairCount               );
                fprintf(fo,"StrngCount                  %d\n",StrngCount                  );
                fprintf(fo,"RuleTemplateCount           %d\n",RuleTemplateCount           );
                fprintf(fo,"ShortRulePairCount          %d\n",ShortRulePairCount          );
                fprintf(fo,"FullRulePairCount           %d\n",FullRulePairCount           );

                finish = clock();
                duration = (double)(finish - start) / CLOCKS_PER_SEC;

                fprintf(fo,"%2.1f seconds\n", duration );
                fclose(fo);
                }
            for(int cut = 0;cut <= cutoff;++cut)
                {
                if(nflexrules)
                    {
                    sprintf(command,COPY "%s%d %s_%d%d",nflexrules,cut,nflexrules,cut,passes);
                    printf("%s\n",command);
                    system(command);
                    if(passes == 1)
                        {
                        sprintf(command,COPY "%s%d %s%d_ambi",nflexrules,cut,nflexrules,cut);
                        printf("%s\n",command);
                        system(command);
                        }
                    else
                        {
                        sprintf(command,COMBIFLEX "%s%d_ambi %s_%d%d %s%d_ambi",nflexrules,cut,nflexrules,cut,passes,nflexrules,cut);
                        printf("%s\n",command);
                        system(command);
                        }
                    }
                else //rules_0.lem
                    {
                    if(passes == 1)
                        {
                        printf("del rules.lem\n");
                        //system("del rules.lem");
                        remove("rules.lem");
                        printf("ren rules_01.lem rules.lem\n");
                        rename("rules_01.lem","rules.lem");
                        }
                    else
                        {
                        sprintf(command,COMBIFLEX "rules.lem rules_0%d.lem rules.lem2",passes);
                        printf("%s\n",command);
                        system(command);
                        sprintf(command,"del rules.lem");
                        printf("%s\n",command);
                        remove("rules.lem");
                        //system(command);
                        sprintf(command,"ren rules.lem2 rules.lem");
                        printf("%s\n",command);
                        //system(command);
                        rename("rules.lem2","rules.lem");
                        }
                    }
                }
            fname = nexttrainname;
            }
        while(moreToDo && passes < 2 && !compute_parms);
        for(int cut = 0;cut <= cutoff;++cut)
            {
            sprintf(command,COPY "%s%d_ambi %s%d",nflexrules,cut,nflexrules,cut);
            printf("%s\n",command);
            system(command);
            }
        if(compute_parms)
            {
            if(currentNo == 0)
                {
                currentNo = Nnodes;
                }
            else
                {
                brownNo = Nnodes;
                if(brownNo <= currentNo)
                    {
                    currentNo = brownNo;
                    betterfound(currentNo);
                    }
                }
            printparms(Nnodes);
            }
        }
    while(compute_parms && iterations++ < 20*64);
    //printf("XX: %d\n",XX); // If high, increase nStandardDev in graph.cpp
    printf( "%2.1f seconds\n", duration );
    if(flog)
        fclose(flog);
    if(argc < 3)
        getchar();
    printf("\nSIMIL OK\n");
//printf("AMBS %d RIGHTS %d\n",AMBS,RIGHTS);
    }
