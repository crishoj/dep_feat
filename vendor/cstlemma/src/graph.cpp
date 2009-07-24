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
#define _CRT_SECURE_NO_DEPRECATE

#include "graph.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <math.h>
#include "utf8func.h"
#include "isofunc.h"

int (*comp)(const vertex * a,const vertex * b) = 0;

int visitno;

int ambivalentWords;
int alternatives;
int allwords;

int NodeCount = 0;

int VertexPointerCount = 0;
int VertexCount = 0;
int TrainingPairCount = 0;
int HashCount = 0;
int RulePairCount = 0;
int StrngCount = 0;
int RuleTemplateCount = 0;
int ShortRulePairCount = 0;
int FullRulePairCount = 0;

int VCount = 0;

bool building = true;

bool strongerCondition(char ** A,char ** B)
    {
    // Return true if all B are contained in an A.
    // The loop exits when there are no more A's.
    // If there still is a B, then this B must be unmatched, and so the
    // function must return false.
    // If there is no B left, it may still be the case that the last
    // B is not contained in an A, so we also have to check for the
    // result from the last call to strstr.
    char * sub = 0;
    char * a = *A;
    char * b = *B;
    while(b && a)
        {
        //printf("*A [%s] *B [%s] ",a,b);
        sub = 0;
        while(a)
            {
            sub = strstr(a,b);
            if(sub)
                {
                a = sub + strlen(b);
                if(!*a)
                    a = *++A;
                break;
                }
            a = *++A;
            }
        b = *++B;
        }
//    printf("%s\n",(!*B && sub) ? "strongerCondition":"not strongerCondition");
    return !*B && sub;
    }

void cut(char * s,char ** a)
    {
    *a = s;
    while(*s && *s != '*')
        ++s;
    if(*s)
        {
        *s++ = '\0';
        cut(s,++a);
        }
    else
        *++a = 0;
    }

void uncut(char ** a)
    {
    if(*++a)
        {
        (*a)[-1]= '*';
        uncut(a);
        }
    }

bool compatibleCondition(char ** A,char ** B)
    {
    char * a = *A;
    char * b = *B;
    // Check compatibility at start of pattern
    if(a[0] == '^' && b[0] == '^')
        {
        int i;
        for(i = 1;a[i] && b[i];++i)
            if(a[i] != b[i])
                return false;
        }
    while(A[1])
        ++A;
    while(B[1])
        ++B;
    if(a != *A || b != *B)
        {
        a = *A;
        b = *B;

        // Check compatibility at end of pattern
        size_t alast = strlen(a) - 1;
        size_t blast = strlen(b) - 1;
        if(a[alast] == '$' && b[blast] == '$')
            {
            int i,j;
            for(i = (int)alast - 1,j = (int)blast - 1;i >= 0 && j >= 0;--i,--j)
                if(a[i] != b[j])
                    return false;
            }
        }
    return true;
    }


edif dif(char * Txt, char * s_Txt)
    {
    /* returns 1 if Txt is a stronger condition (further from the root) 
       than s_Txt.
       It returns -1 if Txt is weaker than s_Txt
       It retuns 0 if the conditions are incommensurable.
    */
    char * A[100];
    char * B[100];
    cut(Txt,A);
    cut(s_Txt,B);
    if(compatibleCondition(A,B))
        {
        bool AB = strongerCondition(A,B);
        bool BA = strongerCondition(B,A);

        edif res = (AB && !BA) ? dif_bigger : (BA && !AB) ? dif_smaller : (AB && BA) ? dif_equal : dif_incommensurable;
        uncut(A);
        uncut(B);
        assert(res == dif_incommensurable || res == dif_smaller || res == dif_bigger || res == dif_equal);
        return res;
        }
    else
        {
        uncut(A);
        uncut(B);
        return dif_incompatible;
        }
    }


edif rulePair::dif(rulePair * other)
    {
    return ::dif(this->itsPattern(),other->itsPattern());
    }


char * dup(const char * buf)
    {
    char * Txt = new char[strlen(buf)+1];
    strcpy(Txt,buf);
    return Txt;
    }

strng::strng(const char * buf)
    {
    Txt = dup(buf);
    ++StrngCount;
    }

bool strng::eq(const char * s) // returns Txt - s
    {
    return !strcmp(Txt,s);
    }



edif strng::dif(strng * s) // returns Txt - s
    {
    return ::dif(Txt,s->Txt);
    }

void trainingPair::addRule(vertex * V,bool InputRight,bool Right)
    {
    this->applicableRules = new vertexPointer(V,this->applicableRules,InputRight,Right);
    }

void trainingPair::deleteRules()
    {
    if(this->applicableRules)
        {
        this->applicableRules->deleteAll();
        this->applicableRules = 0;
        }
    }

void trainingPair::allDeleteRules()
    {
    trainingPair * x = this;
    while(x)
        {
        trainingPair * n = x->Next;
        x->deleteRules();
        x = n;
        }
    }


trainingPair::~trainingPair()
    {
    //printf("delete trainingPair %p %.*s %.*s\n",this, this->itsWordlength(),this->itsWord(), this->itsLemmalength(), this->itsLemmaHead());
    deleteRules();
    delete [] Lemma;
    Lemma = NULL;
    delete [] Mask;
    Mask = NULL;
    --TrainingPairCount;
    }

void vertex::computeImpedance()
    {
    /* Regard the part of the pattern that is enclosed in * as a series of condensors.
    The capacity of each condensor is equal to the length of the pattern string.

    For example, the impedance of ^tvö*k*nur$ is 1/len("k") = 1
    impedance(^*ê*à*ête$) = (1/len("ê")+1/len("à")) = 2
    impedance(^a*ge*etste$) = 1/2
    impedance(^ge*etste$) = 0
    Hypothesis: Rules with low impedance are better (less fluffy) than rules with high impedance.
    */
    impedance = 0.0;
    
    const char * p = Pattern->itsTxt();
    while(*p && *p != '*')
        {
        //++p;
        Inc(p);
        --impedance; // characters at start (or end) diminish impedance.
        }
    if(*p)
        { // *p == '*', the first star
        double len = 0.0;
        //while(*++p)
        int k;
        while((k = getUTF8char(p,UTF8)) != 0)
            {
            if(k == '*') // the second, third, ... star
                {
                impedance += 1.0/len;
                len = 0.0;
                }
            else
                ++len;
            }
        impedance -= len; // characters at end diminish impedance.
        }
        
    }

void vertex::print1(FILE * f)
    {
    fprintf(f,"{%s\t%s}",Pattern->itsTxt(),Replacement->itsTxt());
    }

void vertex::print(FILE * f,int level)
    {
        fprintf(f,"%*s|\t{%s\t%s}\n",level,"",Pattern->itsTxt(),Replacement->itsTxt());
    }

void vertex::printRule(FILE * f,int level,int nr)
    {
    fprintf(f,"%d\t%d\t%.*s\t%.*s\n",
        nr,
        level+1, // 1-based
        strlen(Pattern->itsTxt()) - 2,Pattern->itsTxt()+1,
        strlen(Replacement->itsTxt()) - 2,Replacement->itsTxt()+1);
    }


void vertex::destroy()
    {
    decRefCnt();
    int rc = refCount();
//    printf("destroy vertex %p %s %s %d %d\n",this,Pattern->itsTxt(),Replacement->itsTxt(),refCount(),this->Relations);
    if(rc == 0)
        {
        deleteThis();
        }
    }

vertex::~vertex()
    {
    //printf("|%s\t%s\n",Pattern->itsTxt(),Replacement->itsTxt());

    //this->print2(0);
//    printf("~vertex %p %s %s\n",this,Pattern->itsTxt(),Replacement->itsTxt());
    delete Pattern;
    delete Replacement;
    --VertexCount;
    }

vertex::vertex(rulePair * Rule,hash * Hash):
        Head(0),RefCount(0),/*bits(0),*/Relations(0),Hash(Hash)
        ,R__R(0)
        ,R__W(0)
#if _NA
        ,R__NA(0)
#endif
        ,W__R(0)
        ,W__W(0)
#if _NA
        ,W__NA(0)
#endif

        {
        assert(Hash);
        Pattern = new strng(Rule->pattern());
        Replacement = new strng(Rule->replacement());
        ++VertexCount;
//        printf("vertex %p %s %s\n",this,Pattern->itsTxt(),Replacement->itsTxt());
        }

bool trainingPair::isCorrect(const char * lemma) const
    {
    const char * a = lemma;
    const char * b = LemmaHead;
    const char * e = LemmaHead + lemmalength;
    while(b < e && *a == *b)
        {
        ++a;
        ++b;
        }
    return !*a && b == e;
    }

int trainingPair::printAll(FILE * f,const char * h)
    {
    trainingPair * p = this;
    int n = 0;
    if(p)
        {
        fprintf(f,"%s",h);
        while(p)
            {
            ++n;
            p->print(f);
            if(p->Next)
                fprintf(f,",");
            p = p->Next;
            }
        fprintf(f,"\n");
        }
    return n;
    }

void trainingPair::print(FILE * f)
    {
    fprintf(f,"%.*s\t%.*s",wordlength,Word,lemmalength,LemmaHead);
    }

void trainingPair::printMore(FILE * f)
    {
    fprintf(f,"%.*s\t%.*s\n",wordlength,Word,lemmalength,LemmaHead);
    }

void trainingPair::printSep(FILE * f)
    {
    trainingPair * tp = this;
    do
        {
        ++allwords;
        fprintf(f,"%.*s\t%.*s\t\n",tp->wordlength,tp->Word,tp->lemmalength,tp->LemmaHead);
#if AMBIGUOUS
        if(tp->Alt && tp->Alt != tp)
            {
            ++ambivalentWords;
            trainingPair * p = tp->Alt;
            do
                {
                ++alternatives;
                fprintf(f,"%.*s\t%.*s\t@@%s\n",p->wordlength,p->Word,p->lemmalength,p->LemmaHead,p->isset(b_test) ? "t" : "");
                p = p->Alt;
                }
                while(p != tp);
            }
#endif
        tp = tp->Next;
        }
    while(tp);
    }


// Testing only. (Not called during training!)
matchResult node::lemmatise(trainingPair * pair)
    {
    char * lemma;
    matchResult r = V->lemmatise(pair,0,&lemma);
    if(r == failure)
        {
        if(IfPatternFails)
            return IfPatternFails->lemmatise(pair);
        }
    else 
        {
        pair->setLemma(lemma);
        if(IfPatternSucceeds)
            {
            matchResult rr = IfPatternSucceeds->lemmatise(pair);
            if(rr != failure)
                {
                return rr;
                }
            }
        }
    if(r == right /*|| r == rightBut*/)
        {
        pair->setVertex(V);
        pair->set(b_ok);
        }
    else if(r == wrong)
        {
        pair->setVertex(V);
        pair->set(b_wrong);
        }
    return r;
    }


matchResult vertex::lemmatise(trainingPair * pair,char ** pmask,char ** plemma)
    {
    static char lemma[100];
    static char mask[100];
#if AMBIGUOUS
    pair->unset(b_ok);
    pair->unset(b_wrong);
#endif
    if(apply(pair,sizeof(lemma),lemma,mask))
        {
        if(plemma)
            *plemma = lemma;
        if(pmask)
            *pmask = mask;
        if(pair->isCorrect(lemma))
            {
#if AMBIGUOUS
            pair->set(b_ok);
#endif
            return right;
            }
        else
            {
#if AMBIGUOUS
            pair->set(b_wrong);
#endif
            return wrong;
            }
        }
    else
        {
        return failure;
        }
    }

void vertex::nlemmatiseStart()
    {
    R__W = R__R = W__W = W__R = 0;
#if _NA
    R__NA = W__NA = 0;
#endif
    }

trainingPair * trainingPair::nth(int n)
    {
    trainingPair * pairs = this;
    while(pairs && n > 0)
        {
        pairs = pairs->next();
        --n;
        }
    return pairs;
    }


int vertex::nlemmatise(trainingPair * pair,int n,bool InputRight)
    {
    /*
    if(pair)
        {
        fprintf(flog,"\n");
        fprintf(flog,"nlemmatise:\n");
        this->print1(flog);
        fprintf(flog,"\n");
        }
        */
    int ret = 0;
    while(pair && n != 0)
        {
        ++ret;
        switch(this->lemmatise(pair,0,0))
            {
            case wrong:
                /*
                pair->print(flog);
                fprintf(flog," W\n");
                */
                if(!pair->isset(b_oksibling))
                    { // don't count homographs that have an ok sibling
                    pair->addRule(this,InputRight,false);
                    }
                else
                    {
                    //printf("b_oksibling\n");
                    }
                break;
            case right:
                /*
                pair->print(flog);
                fprintf(flog," R\n");
                */
                // do opportunistically count homographs that are ok
                pair->addRule(this,InputRight,true);
                break;
#if _NA
            default:
                /*
                pair->print(flog);
                fprintf(flog," NA\n");
                */
                if(InputRight)
                    ++R__NA;
                else
                    ++W__NA;
#else
            default:
                ;
#endif
            }
        pair = pair->next();
        --n;
        }
    return ret;//pair;
    }

long nextprime(unsigned long g)
    {
    int i;
    unsigned long smalldivisor;
    static int byte[12]=
        {1,  2,  2,  4,    2,    4,    2,    4,    6,    2,  6};
    /*2-3,3-5,5-7,7-11,11-13,13-17,17-19,19-23,23-29,29-1,1-7*/
    unsigned long bigdivisor;
    if(g & 1)
        ++g; /* even -> uneven */
    smalldivisor = 2;
    i = 0;
    while((bigdivisor = g / smalldivisor) >= smalldivisor)
        {
        if(bigdivisor * smalldivisor == g)
            {
            ++g;
            smalldivisor = 2;
            i = 0;
            }
        else
            {
            smalldivisor += byte[i];
            if(++i > 10)
                i = 3;
            }
        }
    return g;
    }

long casesensitivehash(const char * cp)
    {
    long hash_temp = 0;
    while (*cp != '\0')
        {
        if(hash_temp < 0)
            hash_temp = (hash_temp << 1) +1;
        else
            hash_temp = hash_temp << 1;
        hash_temp ^= *cp;
        ++cp;
        }
    return hash_temp;
    }


void hash::rehash(int loadFactor/*1-100*/)
    {
//    printf("rehashing %ld ...",hash_size);
    long oldsize = hash_size;
    hash_size = nextprime((100 * record_count)/loadFactor);
    vertex ** new_hash_table = new vertex * [hash_size];
    long i;
    for(i = 0;i < hash_size;++i)
        new_hash_table[i] = 0;
    if(hash_table)
        {
        for(i = oldsize;i > 0;)
            {
            vertex * r = hash_table[--i];
            while(r)
                {
                long Key = key(r->itsTxt());
                vertex ** head = new_hash_table + Key;
                vertex * Next = r->getNext();
                r->setNext(*head);
                r->setHead(head);
                *head = r;
                r = Next;
                }
            }
        }
    delete [] hash_table;
    hash_table = new_hash_table;
//    printf("      \r");
    }

hash::hash(long size):record_count(0),next(0)
    {
    //            LOG("               %p hash",this);
    hash_size = nextprime(size);
    hash_table = new vertex * [hash_size];
    long i;
    for(i = 0;i < hash_size;++i)
        hash_table[i] = 0;
    ++HashCount;
    }

hash::~hash()
    {
    //            LOG("               %p ~hash",this);
//    printf("~hash\n");
    int i;
    int n = 0;
    for(i = 0;i < hash_size;++i)
        {
        vertex * ps = hash_table[i];
        while(ps)
            {
            ++n;
            ps->Hash = 0;
            ps = ps->getNext();
            }
        }
    delete [] hash_table;
    // do we want to delete the strngs as well?
    delete next;
    --HashCount;
//    printf("~hash DONE\n");
    }

long hash::key(const char * ckey)
    {
    return ((unsigned long)casesensitivehash(ckey)) % hash_size;
    }

vertex * hash::find(const char * ckey,vertex **& head)
    {
    long Key = key(ckey);
    head = hash_table + Key;
    vertex * p;
    for(p = *head;p && strcmp(p->itsTxt(),ckey);p = p->getNext())
        ;
    return p;
    }

vertex ** hash::convertToList(int & N)
    {
    N = record_count;
    vertex ** ret = new vertex * [record_count];
    int i;
    int n = 0;
    for(i = 0;i < hash_size;++i)
        {
        vertex * ps = hash_table[i];
        while(ps)
            {
            ps->incRefCnt();
            ret[n++] = ps;
            ps = ps->getNext();
            }
        }
    return ret;
    }

int hash::forall(forallfunc fnc)
    {
    int i;
    int n = 0;
    for(i = 0;i < hash_size;++i)
        {
        vertex * ps = hash_table[i];
        while(ps)
            {
            ++n;
            (ps->*fnc)();
            ps = ps->getNext();
            }
        }
    return n;
    }

void strng::checkIntegrity()
    {
    assert(Txt);
    }

bool strng::hasWildCard()
    {
    return Txt[0] != '^' || Txt[strlen(Txt)-1] != '$' || strchr(Txt,'*');
    }


void vertex::deleteThis()
    {
    if(Hash)
        {
        while(*Head && *Head != this)
            Head = &(*Head)->Next;
        if(*Head)
            {
            *Head = Next;
            Next = 0;
            delete this;
            }
        }
    else
        delete this;
    }

vertex * hash::getVertex(rulePair * Rule,bool & New)
    {
    vertex * ret = 0;
    New = false;
    //if(*Rule->pattern() && *Rule->replacement())
        {
        vertex ** head;
        long lf = loadfactor();
        if(lf > 100)
            {
            rehash(60);
            }
        vertex * p = find(Rule->pattern(),head);
        if(p)
            p = p->findReplacement(Rule);
        if(p)
            {
            ret = p;
            }
        else
            {
            ret = new vertex(Rule,this);
            ret->setNext(*head);
            *head = ret;
            ret->setHead(head);
            New = true;
            inc();
            }
        }
  //  else
    //    ret = 0;
    return ret;
    }


void trainingPair::checkIntegrity(vertex * v,bool supportive)
    {
    }

void trainingPair::checkIntegrity()
    {
    }



int trainingPair::cmpWord(const trainingPair * B) const
    {
    int ret = (int)this->wordlength - (int)B->wordlength;
    if(!ret)
        ret = strncmp(this->Word,B->Word,this->wordlength);
    return ret;
    }

int trainingPair::cmpLemma(const trainingPair * B) const
    {
    int ret = (int)this->lemmalength - (int)B->lemmalength;
    if(!ret)
        ret = strncmp(this->LemmaHead,B->LemmaHead,this->lemmalength);
    return ret;
    }

#if LEMMACLASS
int trainingPair::cmpLemmaClass(const trainingPair * B) const
    {
    int ret = (int)this->lemmaclasslength - (int)B->lemmaclasslength;
    if(!ret)
        ret = strncmp(this->LemmaClass,B->LemmaClass,this->lemmaclasslength);
    return ret;
    }
#endif

#if LEMMAINL
int trainingPair::cmpFreq(const trainingPair * B) const
    {
    int ret = (int)this->Inl - (int)B->Inl;
    if(!ret)
        ret = (int)this->Lemma_Inl - (int)B->Lemma_Inl;
    return ret > 0 ? 1 : ret < 0 ? -1 : 0;
    }
#endif

void trainingPair::fprint(FILE * famb)
    {
    fprintf(famb,"%.*s\t%.*s\t",this->wordlength,this->Word,this->lemmalength,this->LemmaHead);
    if(this->Lemma && this->isset(b_wrong))
        {
        fprintf(famb,"%s\t",this->Lemma);
        if(V)
            {
            V->print1(famb);
            fprintf(famb,"\t");
            }
        }
//    fprintf(famb,"%.*s\t%d\n",this->lemmaclasslength,this->LemmaClass,this->Inl);
#if LEMMACLASS
    fprintf(famb,"%.*s\n",this->lemmaclasslength,this->LemmaClass);
#else
    fprintf(famb,"\n");
#endif
    }

#if WRITEINVERTED
char * strnrev(const char * s,size_t n)
    {
    if(n == 0)
        n = strlen(s);
    static char * ret = 0;
    static size_t N = 0;
    if(N < n+1)
        {
        if(N > 0)
            delete [] ret;
        N = n+1;
        ret = new char[N];
        }
    size_t i = n;
    char * d = ret;
    if(UTF8)
        {
        while(i > 0)
            {
            --i;
            if(s[i] & 0x80)
                {
                while(i > 0 && (s[--i] & 0xC0) != 0xC0)
                    ;
                size_t ii = i;
                do 
                    *d++ = s[i];
                while((s[++i] & 0xC0) != 0xC0 && s[i] & 0x80);
                i = ii;
                }
            else
                *d++ = s[i];
            }
        }
    else
        while(i > 0)
            *d++ = s[--i];
    *d = '\0';
    return ret;
    }

char * Strrev(char * s)
    {
    return strnrev(s,strlen(s));
    }
#endif

void trainingPair::fprintTraining(FILE * fp
#if WRITEINVERTED
                                  ,FILE * fpinverted
#endif
                                  )
    {
    if(UTF8 
      ? isUpper(UTF8char(this->Word,UTF8)) && !isUpper(UTF8char(this->LemmaHead,UTF8)) 
//      : isUpper(*(const unsigned char *)this->Word) && !isUpper(*(const unsigned char *)this->LemmaHead)
      : isUpperISO(this->Word) && !isUpperISO(this->LemmaHead)
      )
        {
        char * lemma = new char[this->lemmalength+1];
        strncpy(lemma,this->LemmaHead,this->lemmalength);
        lemma[this->lemmalength] = '\0';
        if( UTF8 
          ? isAllUpper(this->Word,this->wordlength) && (AllToUpper(lemma),true)
          : isAllUpperISO(this->Word,this->wordlength) && (AllToUpperISO(lemma),true)
//          : isAllUpperISO(this->Word,this->wordlength) && (AllToUpperISO(lemma),true)
          )
            {
            }
        else
            {
            char * plemma = lemma;
            const char * pword = this->Word;
            size_t len = this->wordlength;
            if(UTF8)
                {
                while(len-- > 0 && *plemma)
                    {
                    char * lemma = plemma;
                    int LEMMA = upperEquivalent(getUTF8char((const char *&)plemma,UTF8));
                    int WORD =  getUTF8char(pword,UTF8);
                    if(LEMMA == WORD)
                        {
                        char NLEMMA[7];
                        int n = UnicodeToUtf8(LEMMA,NLEMMA,sizeof(NLEMMA)-1);
                        if(n == plemma - lemma)
                            {
                            strncpy(lemma,NLEMMA,n);
                            }
                        else
                            break;
                        }
                    else
                        break;
                    }
                }
            else
                {
                while(len-- > 0 && *plemma && isUpperISO(pword))
                    {
                    toUpperISO(plemma);
                    ++plemma;
                    ++pword;
                    }
                }
            }
        fprintf(fp,"%.*s\t%s\t",this->wordlength,this->Word,lemma);
#if WRITEINVERTED
        fprintf(fpinverted,"%s\t%s\t",strnrev(this->Word,this->wordlength),Strrev(lemma));         
#endif
        delete [] lemma;
        }
    else
        {
        fprintf(fp,"%.*s\t%.*s\t",this->wordlength,this->Word,this->lemmalength,this->LemmaHead);
        char * lemmainverted = new char[this->lemmalength + 1];
#if WRITEINVERTED
        strcpy(lemmainverted,strnrev(this->LemmaHead,this->lemmalength));
        fprintf(fpinverted,"%s\t%s\t",strnrev(this->Word,this->wordlength),lemmainverted);
#endif
        delete [] lemmainverted;
        }
#if LEMMACLASS
    fprintf(fp,"%.*s",this->lemmaclasslength,this->LemmaClass);
#endif
#if LEMMAINL
    fprintf(fp,"\t%d\t%d",this->Inl,this->Lemma_Inl);
#endif
    fprintf(fp,"\n");
#if WRITEINVERTED
#if LEMMACLASS
    fprintf(fpinverted,"%.*s",this->lemmaclasslength,this->LemmaClass);
#endif
#if LEMMAINL
    fprintf(fpinverted,"\t%d\t%d",this->Inl,this->Lemma_Inl);
#endif
    fprintf(fpinverted,"\n");
#endif
    }

void trainingPair::fprintAll(FILE * famb)
    {
    fprintf(famb,"%.*s\t%.*s\t",this->wordlength,this->Word,this->lemmalength,this->LemmaHead);
    if(this->Lemma && this->isset(b_wrong))
        {
        fprintf(famb,"%s\t",this->Lemma);
        if(V)
            {
            V->print1(famb);
            fprintf(famb,"\t");
            }
        }
#if LEMMACLASS
    fprintf(famb,"%.*s",this->lemmaclasslength,this->LemmaClass);
#endif
#if LEMMAINL
    fprintf(famb,"\t%d\t%d",this->Inl,this->Lemma_Inl);
#endif
    if(this->isset(b_ambiguous))
        fprintf(famb," ambiguous");
    if(this->isset(b_skip))
        fprintf(famb," skip");
    fprintf(famb,"\n");

    //fprintf(famb,"%.*s\n",this->lemmaclasslength,this->LemmaClass);
    }

void trainingPair::makeCandidateRules(hash * Hash,vertex * parent)
    {
    trainingPair * tp = this;
    while(tp)
        {
        /*
        fprintf(flog,"tp:");
        tp->print(flog);
        fprintf(flog,"\n");
        */
        tp->makeRuleEx(Hash,parent);
        /*
        fprintf(flog,"\n");
        */
        tp = tp->next();
        }
    }

int node::printRules(node * nd
#if RULESASTEXTINDENTED
                     ,FILE * fo
#endif
#if BRACMATOUTPUT
                     ,FILE * fobra
#endif
                     ,FILE * folem,int ind,strng * L,strng * R,int & nr)
    {
    int n = 0;
    while(nd)
        {
        strng * nLL = 0;
        strng * nRR = 0;
        strng * node;
        strng * pat = nd->V->itsstrngPattern()->substr(1,strlen(nd->V->itsPattern()) - 2);
        strng * rep = nd->V->itsstrngReplacement()->substr(1,strlen(nd->V->itsReplacement()) - 2);
        strng * patreps[100];
        unsigned int i;
        for(i = 0;i < sizeof(patreps)/sizeof(patreps[0]);++i)
            patreps[i] = 0;
        if(nd->IfPatternSucceeds)
            {
            node = nd->makeNodeAndTrim(nr,pat,rep,L,R,nLL,nRR,patreps);
#if BRACMATOUTPUT
            fprintf(fobra,"(");
#endif
            }
        else
            node = nd->makeNode(patreps,nr,pat,rep,L,R);
        for(i = 0;patreps[i];i+=2)
            ;
        /**/
        if(i > 4)
            {
            i -= 2;
            strng * tmppat = patreps[i];
            strng * tmprep = patreps[i+1];
            while(i > 2)
                {
                patreps[i] = patreps[i - 2];
                patreps[i+1] = patreps[i - 1];
                i -= 2;
                }
            patreps[2] = tmppat;
            patreps[3] = tmprep;
            }
        /**/
        fprintf(folem,"%d\t",ind);
        if(patreps[0])
            {
            for(i = 0;patreps[i+2];i+=2)
                {
                fprintf(folem,"%s\t%s\t",patreps[i]->itsTxt(),patreps[i+1]->itsTxt());
                }
            fprintf(folem,"%s\t%s",patreps[i]->itsTxt(),patreps[i+1]->itsTxt());
            }
        fprintf(folem,"\n");
        for(i = 0;patreps[i];++i)
            delete patreps[i];

        delete pat;
        delete rep;
#if RULESASTEXTINDENTED
        nd->V->printRule(fo,ind,nr);
#endif
#if BRACMATOUTPUT
        fprintf(fobra,"%s\n",node->itsTxt());
#endif
        delete node;
        if(nd->IfPatternSucceeds)
            {
#if BRACMATOUTPUT
            fprintf(fobra,",");
#endif
            strng nL(L);
            strng nR(nRR);
            nL.cat(nLL,0);
            nR.cat(R,0);
            n += nd->IfPatternSucceeds->printRules
                ( nd->IfPatternSucceeds
#if RULESASTEXTINDENTED
                , fo
#endif
#if BRACMATOUTPUT
                , fobra
#endif
                , folem
                , ind+1
                , &nL
                , &nR
                , nr
                );
#if BRACMATOUTPUT
            fprintf(fobra,")\n");
#endif
            }
        delete nLL;
        delete nRR;
        nd = nd->IfPatternFails;
/*
        if(nd->IfPatternFails)
            {
            n += nd->IfPatternFails->printRules(nd,fo,fobra,folem,ind,L,R,nr);
            }
*/
        }
    return n;
    }


void node::splitTrainingPairList(trainingPair * all,trainingPair *** pNotApplicable,trainingPair *** pWrong,trainingPair *** pRight)
    {
    while(all)
        {
        trainingPair * nxt = all->next();
        all->setNext(0);
        char * mask = 0;
        matchResult res = V->lemmatise(all,&mask,0);
        if(mask)
            {
            all->takeMask(mask);
            assert(res != failure);
            }
        else
            {
            assert(res == failure);
            }
        switch(res)
            {
            case failure:
                **pNotApplicable = all;
                *pNotApplicable = all->pNext();
                break;
            case wrong:
                **pWrong = all;
                *pWrong = all->pNext();
                break;
            default:
                **pRight = all;
                *pRight = all->pNext();
            }
        all = nxt;
        }
    }


bool node::compatibleSibling(node * sib)
    {
    if(this->V->dif(sib->V) != dif_incompatible)
        return true;
    else if(IfPatternFails)
        return IfPatternFails->compatibleSibling(sib);
    else
        return false;
    }


node * node::cleanup(node * parent)
    {
//    int cnt;
//    int newcnt;
    if(this->IfPatternSucceeds) 
        {
//        cnt = countWords();
        this->IfPatternSucceeds = this->IfPatternSucceeds->cleanup(this);
//        newcnt = countWords();
//        assert(cnt == newcnt);
        }
    if(IfPatternFails)
        {
//        if(parent)
//            cnt = parent->countWords();
        IfPatternFails = IfPatternFails->cleanup(parent);
/*        if(parent)
            {
            newcnt = parent->countWords();
            assert(cnt == newcnt);
            }
*/
        }
    if(  IfPatternFails 
      && (  this->IfPatternSucceeds 
         || IfPatternFails->compatibleSibling(this)
         )
      )
        {
        return this;
        }
    else if(parent)
        {
        if(this->Right)
            {
            if(!parent->IfPatternFails && !parent->Right)
                {
                // remove parent, keep this.
                return this;
                }
//            assert(this->Right);
            trainingPair * R = this->Right;
            while(true)
                {
                matchResult res = parent->V->lemmatise(R,0,0);

                if(res != right)
                    {
                    return this;
                    }
                if(R->next())
                    R = R->next();
                else
                    break;
                }
//            printf("%d + %d = ",this->Right->count(),parent->Right->count());
            R->setNext(parent->Right);
            parent->Right = this->Right;
            this->Right = 0;
//            printf("%d\n",parent->Right->count());
            }
        node * ret = IfPatternFails;
        if(ret)
            {
//            assert(this->IfPatternSucceeds == 0);
            IfPatternFails = 0;
            }
        else
            {
            ret = this->IfPatternSucceeds;
            this->IfPatternSucceeds = 0;
            }
        delete this;
        return ret;
        }
    else
        {
        return this;
        }
    }

#if AMBIGUOUS
void trainingPair::makeWrongAmbiguousIfRightPresent(trainingPair *& Wrong,trainingPair *& Ambiguous)
    {
    trainingPair * p = this; // This is the head of a list of training pairs 
                             // that were all lemmatized wrongly.
                             // When this method returns, 'this' is at the
                             // end of either the new Wrong list or at the end
                             // of the Ambiguous list.
    Wrong = Ambiguous = 0;
    while(p)
        {
        trainingPair * nxt = p->Next;
        if(p->Alt != p) 
            { // there are homographs
            // perhaps one of its friends is ok.
            trainingPair * q = p->Alt;
            while(q != p && !q->isset(b_ok))
                {
                q = q->Alt;
                }
            if(q == p)
                { // No friend is ok
                // Put p at the head of the Wrong list
                // This inverts the order of training pairs in the Wrong list!
                p->Next = Wrong;
                Wrong = p;
                p->unset(b_oksibling);
                }
            else
                { // An ok friend is found.
                // Put p at the head of the Ambiguous list.
                q->unset(b_bench);
                p->Next = Ambiguous;
                Ambiguous = p;
                p->set(b_oksibling);
                }
            p->set(b_bench);
            }
        else // No homographs
            {
            // Put p at the head of the Wrong list
            // This inverts the order of training pairs in the Wrong list!
            p->Next = Wrong;
            Wrong = p;
            }
        p = nxt;
        }
    }
#endif

/*
static long checkPV(vertex ** pv,int N)
    {
    long R = pv[0]->R();
    long W = pv[0]->W();
    long res = R + W;
    for(int i = 1;i < N;++i)
        {
        assert(pv[i]->R() == R && pv[i]->W() == W);
        }
    return res;
    }

static void qqsort(vertex ** pv,size_t num,size_t width,int (__cdecl *compare )(const void *, const void *))
    {
    for(int i = 1;i < num;++i)
        {
        if(comp(*pv,pv[i]) > 0)
            {
            vertex * tmp = pv[0];
            pv[0] = pv[i];
            pv[i] = tmp;
            }
        }
    }
*/

#if AMBIGUOUS
void node::init(trainingPair ** allRight,trainingPair ** allWrong/*,trainingPair ** allAmbiguous*/,int level)
#else
void node::init(trainingPair ** allRight,trainingPair ** allWrong,int level/*,vertex ** pvp,int Np*/)
#endif
    {
    /* At entry, this node has a rule (a pattern and a replacement).
       The parent's two sets of training pairs are given as arguments, so that
       this node, this node's siblings or this node's children may "steal" 
       them.
       The return value are the leftovers, the trainingpairs for which no
       rules could be made. (This can only be the case if the parent's rule's
       pattern matches all of a word. The leftovers consist of at most one
       training pair.)
    */
    /*
    this->V->print(flog,level);
    */

    trainingPair * NotApplicableRight;
    trainingPair * NotApplicableWrong;
    trainingPair * Wrong;

    trainingPair ** pNotApplicableRight = &NotApplicableRight; // Pattern fails.
    trainingPair ** pNotApplicableWrong = &NotApplicableWrong; // Pattern fails.
    trainingPair ** pWrong = &Wrong; // Pattern succeeds and replacement is wrong.
    trainingPair ** pRight = &this->Right; // Pattern succeeds and replacement is right.
    //if(pvp){checkPV(pvp,Np);}

    //int input = (*allRight ? (*allRight)->count() : 0) + (*allWrong ? (*allWrong)->count() : 0);
    this->splitTrainingPairList(*allRight,&pNotApplicableRight,&pWrong,&pRight);
    this->splitTrainingPairList(*allWrong,&pNotApplicableWrong,&pWrong,&pRight);
#if AMBIGUOUS
//    this->splitTrainingPairList(*allAmbiguous,&pNotApplicableWrong,&pWrong,&pRight);
#endif
    *pNotApplicableRight = 0;
    *pNotApplicableWrong = 0;
    *pWrong = 0;
    *pRight = 0; // Pattern succeeds and replacement is right.
    /*
    int output = (NotApplicableRight ? NotApplicableRight->count() : 0)
        + (NotApplicableWrong ? NotApplicableWrong->count() : 0)
        + (Wrong ? Wrong->count() : 0)
        + (this->Right ? this->Right->count() : 0);
    assert(input == output);
    printf("input %d\n",input);
*/
    *allRight = NotApplicableRight;
    *allWrong = NotApplicableWrong;

    //if(pvp){checkPV(pvp,Np);}

    this->Right->allDeleteRules();
    trainingPair * Ambiguous = 0;
    /** /
    if(this->Right)
        {
        fprintf(flog,"%.*s",level,"");
        this->Right->printAll(flog,"right:");
        }
    if(Wrong)
        {
        fprintf(flog,"%.*s",level,"");
        Wrong->printAll(flog,"wrong:");
        }
    */
    //if(pvp){checkPV(pvp,Np);}
    if(Wrong)
        {
//        printf("Wrong->allDeleteRules();\n");
        Wrong->allDeleteRules();
#if AMBIGUOUS
        Wrong->makeWrongAmbiguousIfRightPresent(Wrong,Ambiguous);
        /*if(Ambiguous)
            Ambiguous->printAll(stdout,"ambiguous");*/
#endif
        }
    //if(pvp){checkPV(pvp,Np);}
/*    output = (NotApplicableRight ? NotApplicableRight->count() : 0)
        + (NotApplicableWrong ? NotApplicableWrong->count() : 0)
        + (Wrong ? Wrong->count() : 0)
        + (this->Right ? this->Right->count() : 0)
        + (Ambiguous ? Ambiguous->count() : 0);
    assert(input == output);
*/
    if(Wrong)
        {
        /*
        int wc = Wrong->count();
        fprintf(flog,"Wrong:%d\n",wc);
        if(wc < 1000)
            Wrong->printSep(flog);
        fprintf(flog,"\n");
        if(this->Right)
            {
            int rc = this->Right->count();
            fprintf(flog,"Right:%d\n",rc);
            if(rc < 1000)
                this->Right->printSep(flog);
            fprintf(flog,"\n");
            }
        else
            fprintf(flog,"Right:0\n");
        */
        if(Ambiguous)
            {
//            printf("Wrong %d Ambiguous %d ",Wrong->count(),Ambiguous->count());
            Wrong = Ambiguous->append(Wrong);
//            printf("Sum %d ",Wrong->count());
            Ambiguous = 0;
            }
        int N;
        hash Hash(1000);
        /*
        fprintf(flog,"\nWrong going:\n");
        printf("Wrong->makeCandidateRules\n");
        */
        Wrong->makeCandidateRules(&Hash,this->V);
        /*
        vertex ** pvtemp = Hash.convertToList(N);
        assert(N > 0);
        fprintf(flog,"Rules taking care of wrong cases:\n");
        if(N < 1000)
            {
            for(int k = 0;k < N;++k)
                pvtemp[k]->print1(flog);
            fprintf(flog,"\n");
            }
        else
            fprintf(flog,"N = %d\n",N);

        for(int i = 0;i < N;++i)
            {
            pvtemp[i]->decRefCnt();
            }
        delete [] pvtemp;
        */
        /*
        fprintf(flog,"\nRight going:\n");
        printf("this->Right->makeCandidateRules\n");
        */
        this->Right->makeCandidateRules(&Hash,this->V);
        /*
        fprintf(flog,"\nRules made!\n");
        */
#if AMBIGUOUS
        //this->Ambiguous->makeCandidateRules(&Hash,this->V);
#endif

//        printf("convertToList\n");
        vertex ** pv = Hash.convertToList(N);
        assert(N > 0);
        /*
        fprintf(flog,"All Rules:\n");
        if(N < 1000)
            {
            for(int k = 0;k < N;++k)
                pv[k]->print1(flog);
            fprintf(flog,"\n");
            }
        else
            fprintf(flog,"N = %d\n",N);
        */
        int wpart = -1; // -1 indicating: no upper bound
        int rpart = -1;

        // hack if number of pairs is very big
        int ntestpairWrong = Wrong->count();
#if AMBIGUOUS
        //ntestpairWrong += this->Ambiguous->count();
#endif
        int ntestpairRight = this->Right ? this->Right->count() : 0;
        double fraction = (double)MAXPAIRS / (double)(ntestpairWrong + ntestpairRight);
        if(fraction < 1.0)
            { // Define upper bound to make it managable memory-wise
            printf("%d < %d N=   %d\n",MAXPAIRS,(ntestpairWrong + ntestpairRight),N);
            wpart = (int)(fraction * (double)wpart);
            rpart = MAXPAIRS - wpart;
            }
        //:hack

        // Test all pairs (up to upper bound) on all candidate rules.
        //printf("Test all pairs (up to upper bound) on all candidate rules. %d\n",N);
        printf("             %d  \r",N);
        // temp:
        //int wold = 0;
        //int w = 0;
        //int rold = 0;
        //int r = 0;
        //:temp
        //temptest = "niepó³murowany";
        for(int i = 0;i < N;++i)
            {
            printf("%d  \r",i);
            /** /
            fprintf(flog,"%.*s%d:",level,"",i);
            pv[i]->print1(flog);
            fprintf(flog,"\n");
            */
            // Reset all counters in all candidate rules.
            pv[i]->nlemmatiseStart();
            /*w =*/ pv[i]->nlemmatise(Wrong,wpart,false);
#if AMBIGUOUS
            //pv[i]->nlemmatise(this->Ambiguous,wpart,false); // TODO can wpart be used here?
#endif
            /*r =*/ pv[i]->nlemmatise(this->Right,rpart,true);
            //temptest = NULL;
            /*
            if(i == 0)
                {
                wold = w;
                rold = r;
                printf("wold:%d rold %d\n",w,r);
                }
            else
                {
                if(w != wold || r != rold)
                    printf("w:%d r %d\n",w,r);
                }
                */
            }
        //printf("check %ld\n",checkPV(pv,N));


//        printf("Test all pairs (up to upper bound) on all candidate rules. %d DONE\n",N);
        node ** pnode = &this->IfPatternSucceeds;
        int first = 0;
        do
            {
            if(first >= N)
                {
                fprintf(flog,"pv:");
                for(int i = 0;i < N;++i)
                    {
                    pv[i]->print1(flog);
                    }
                fprintf(flog,"\nWrong:");
                Wrong->printAll(flog,"");
                fclose(flog);
                printf("\nAFFIXTRAIN failed\n");
                exit(-1);
                }
            assert(first < N);
            //printf("check %ld\n",checkPV(pv,N));
            //printf("check %ld\n",checkPV(pv+first,N-first));
            //qqsort(pv+first,N-first,sizeof(pv[0]),comp);
            vertex ** pvf = pv+first;
            vertex ** pvN = pv+N;
            for(vertex ** pvi = pvf+1;pvi < pvN;++pvi)
                {
                if(comp(*pvf,*pvi) > 0)
                    {
                    vertex * tmp = *pvf;
                    *pvf = *pvi;
                    *pvi = tmp;
                    }
                }
            //printf("check %ld\n",checkPV(pv+first,N-first));
            *pnode = new node(pv[first]);
            //(*pnode)->printSep(stdout,level);
#if AMBIGUOUS
            (*pnode)->init(&this->Right,&Wrong,/* &this->Ambiguous,*/level+1);
#else
            (*pnode)->init(&this->Right,&Wrong,level+1/*,pv+first,N-first*/);
#endif
            //printf("check %ld\n",checkPV(pv+first,N-first));
            ++first;
            // hack:

            int CnT;
#if AMBIGUOUS
            // Take apart the wrongly lemmatised homographs that have a correctly lemmatised sibling. 
            Wrong->makeWrongAmbiguousIfRightPresent(Wrong,Ambiguous);
            /*if(Ambiguous)
                Ambiguous->printAll(stdout,"ambiguous");*/
#endif
            if(  wpart >= 0 
              && Wrong 
              &&    ( CnT = 
                      Wrong->count() 
                    + (this->Right ? this->Right->count() : 0) 
#if AMBIGUOUS
                    //+ (this->Ambiguous ? this->Ambiguous->count() : 0) 
#endif
                    )
                 <= MAXPAIRS
              )
                {
                printf("%d > %d N=%d\n",MAXPAIRS,CnT,N);
                // Test all remaining pairs on all remaining candidate rules.
        //temptest = "niepó³murowany";
                for(int i = first;i < N;++i)
                    {
                    // Reset all counters in remaining candidate rules.
                    pv[i]->nlemmatiseStart();
                    pv[i]->nlemmatise(Wrong,-1,false);
                    pv[i]->nlemmatise(this->Right,-1,true);
#if AMBIGUOUS
                    //pv[i]->nlemmatise(this->Ambiguous,-1,true);
#endif
        //temptest = NULL;
                    }
                wpart = -1;
                //rpart = -1;
                }
            // :hack
            pnode = &(*pnode)->IfPatternFails;
            }
        while(Wrong);
        //RIGHTS += this->Right->count();

        *pnode = 0;
        for(int i = 0;i < N;++i)
            {
            pv[i]->destroy();
            }
        delete [] pv;
        }
    else
        {
        //AMBS += Ambiguous ? Ambiguous->count() : 0;
        //RIGHTS += this->Right->count();
        }
    }

