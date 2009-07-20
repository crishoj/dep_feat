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
#include "applyrules.h"
#include "flex.h"
#include "caseconv.h"
#include "readlemm.h"
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


flex Flex;

int base::nn = 0;
int base::mutated = 0;
int node::mutated = 0;
int type::mutated = 0;
bool flex::baseformsAreLowercase = true;
long flex::CutoffRefcount = 0L;
bool flex::showRefcount = false;
//bool     printoe = false;

bool WRITE = 0;

#define TABASSEP 1 // Bart 20050905 Tab as separator between base form and full form is better than square brackets,
                   // because the base form might contain square brackets itself (as in  "CO[2]").


#ifdef FLEXIBLE
static char N[] = "N";
#endif
static char EMPTY[] = "";
static bool training = false;

bool changes()
    {
    if(base::mutated || node::mutated || type::mutated)
        {
        printf("Mutations: baseforms %d nodes %d types %d\n",
            base::mutated,node::mutated,type::mutated);
        return true;
        }
    return false;
    }

void unchanged()
    {
    base::mutated = 0;
    node::mutated = 0;
    type::mutated = 0;
    }

void Exit()
    {
    FILE * fp = fopen("tree.txt","w");
    Flex.write(fp,true);
    fclose(fp);
    exit(0);
    }

void Strrev(char * s)
    {
    char * e = s + strlen(s) - 1;
    while(s < e)
        {
        char t = *s;
        *s++ = *e;
        *e-- = t;
        }
    }

#define strrev Strrev

base::base(char * baseform,bool fullWord,
#if REFCNT
           char * Refcnt/*!*/,
#endif
           base * next):m_next(next),m_fullWord(fullWord),needed(true),added(true)
    {
    ++nn;
    m_n = nn;
//    if(base::nn < -1000)
   // printf("%d\n",n);
    this->m_baseform = new char[strlen(baseform)+1];
    strcpy(this->m_baseform,baseform);
#if REFCNT
    if(Refcnt)
        {
        long rfcnt = strtol(Refcnt,0,10);
//        printf("%d %s\n",rfcnt,baseform);
        if(rfcnt > 0)
            this->refcnt = rfcnt;
        else
            this->refcnt = 1;
        }
    else
#endif
        this->refcnt = 1;
    ++mutated;
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }

base * base::add(char * baseform,bool fullWord,base *& prev
#if REFCNT
                 ,char * Refcnt/*!*/
#endif
                 )
    {
    int cmp = strcmp(baseform,this->m_baseform);
    if(!cmp)
        {
        if(fullWord)
            {
//    if(base::nn < -1000)
//            printf("FULLWORD <%s>\n",baseform);
            this->m_fullWord = true;
            }
        incRefCount();
        return this;
        }
    if(cmp > 0)
        {
//    if(base::nn < -1000)
  //      printf("\\[%s]\n",baseform);
        prev = new base(baseform,fullWord,
#if REFCNT
            Refcnt,
#endif
            this); 
        return prev;
        }
    else if(m_next)
        return m_next->add(baseform,fullWord,m_next
#if REFCNT
        ,Refcnt
#endif
        );
    else
        {
//    if(base::nn < -1000)
  //      printf("#[%s]\n",baseform);
        m_next = new base(baseform,fullWord
#if REFCNT
            ,Refcnt
#endif
            );
        return m_next;
        }
    }

void base::removeNonFullWordsAsAlternatives()
    {
    while(m_next && !m_next->isFullWord())
        m_next = m_next->remove();
    if(m_next)
        m_next->removeNonFullWordsAsAlternatives();
    }

void base::removeUnusedPatterns(base *& prev/*,const char * Type,char buf[]*/)
    {
    if(refcnt == 0)
        {
        needed = false;
#if 1
        prev = remove();
        if(prev)
            prev->removeUnusedPatterns(prev/*,Type,buf*/);
#else
        if(m_next)
            m_next->removeUnusedPatterns(m_next/*,Type,buf*/);
#endif
        }
    else if(m_next)
        m_next->removeUnusedPatterns(m_next/*,Type,buf*/);
    }

#if TEST
void base::test(char * text,char * save,char * buf,const char * Type)
    {
    char * bf;
    int borrow;
    if(Flex.Baseform(buf,Type,bf,borrow))
        {
        if(strcmp(save,bf))
            {
            printf("%s Buf %s Difference: baseform should be: %s, found: %s\n",text,buf,save,bf);
            Exit();
            }
        }
    else
        {
        printf("%s Baseform %s not found (should have baseform %s)\n",text,buf,save);
        Exit();
        }
    free(save);
    }
#endif

/*
Suppose we have a rule
[oid]id     (to constuct base form of mongolid, remove 'id' and insert 'oid')
This rather special rule would give wrong results for many other words ending in 'id'
(*) blid    ->  bloid 


*/


bool base::removeUnneededPatterns(base *& prev,const char * Type,char tailBuffer[],node * parent)
    {
//    printf("[%s]%s ",baseform,tailBuffer);
//    if(*baseform && *baseform == *tailBuffer && tailBuffer[1] /*We do not want to reduce 
  //                                                  the tail to nothing*/)
    int len = parent->itsLen();
    if(  !strncmp(m_baseform,tailBuffer,len) 
      && tailBuffer[len] /*We do not want to reduce the tail to nothing*/
      )
        // The deepest level of the tree makes no difference: the first 'len'
        // characters of the tail pattern are equal to the first 'len'
        // characters of the base form. Consider the possibility that we can
        // do without this last level. Perhaps we first have to get rid of a
        // seldomly used rule higher up in the tree that gives a wrong
        // prediction.
        // Example: can we 'shorten' the rule [bid]bid ?
        // On the other hand, we don't want to shorten the rule [b]b, as 
        // nothing would remain.
        {
//        printf("[%s]%s %d %c\n",baseform,tailBuffer,len,tailBuffer[len]);
        int offset;
        base * BaseOfHigherUpRule;
        if(Flex.Baseform2(tailBuffer+1,Type,BaseOfHigherUpRule,offset))
            // BaseOfHigherUpRule and offset now contain the prediction based 
            // on the left-trimmed tailBuffer. Offset is counted from the end
            // of the input (tailBuffer+1). BaseOfHigherUpRule is closer to
            // the root of the tree than this object, because the tail pattern
            // is shorter.
            {
//            printf("tailBuffer+1 %s BaseOfHigherUpRule %s offset %d\n",tailBuffer+1,BaseOfHigherUpRule->m_baseform,offset);
#if 1
            int wlen = strlen(tailBuffer+1); // the length of the left-trimmed tail
            int borrow = wlen - offset; //the number of characters that weren't
                            //used for deciding the base form by Flex.Baseform2
            if(  !BaseOfHigherUpRule->Next() // we only want to employ 
                                            // singe-valued baseforms
              && !strncmp(tailBuffer+1,m_baseform+1,borrow) // Apart from 
                 // the very first characters, also the borrowed characters 
                 // of the remaining tail must be equal to the base form.
                 // compare stems
              && !strcmp(m_baseform+1+borrow,BaseOfHigherUpRule->bf()) 
                // The prediction based on the shortened rule must also match.
              )
                {
                // Wow, we can just delete this rule; a rule higher up will
                // overtake.
                needed = false;
                BaseOfHigherUpRule->refcnt += refcnt/* - 1*/;
//                printf("BaseOfHigherUpRule %s this %s tailbuffer %s\n",BaseOfHigherUpRule->bf(),bf(),tailBuffer);
//                getchar();
#if TEST
                char * save = strdup(baseform);
#endif
                prev = remove();
                if(prev)
                    prev->removeUnneededPatterns(prev,Type,tailBuffer,parent);
#if TEST
                test("Should be present already",save,tailBuffer,Type);
#endif
                return true;
                }
            else
                {
                /*
                If we just remove the current rule, we get false predictions.
                We will have to consider removing an odd rule higher up that
                spoils the prediction.
                */
//                printf("<%.*s|%s>\n",borrow,wrd,wrd+borrow);
                if(borrow > 0) // This ensures that we do not shorten the tail
                            //if all of the word was used to find the baseform.
                            //(Doing so would create ambivalent base forms.)
                    {
                    // There is room for creating a rule between this level 
                    // and the level of BaseOfHigherUpRule. So we do not have
                    // to delete BaseOfHigherUpRule!
#if 0
                    // Why don't we do anything? Answer: the program doesn't 
                    // terminate if we add a new rule here.
                    base * newbase = Flex.add(Type,tailBuffer+1,baseform+1,false);
                    needed = false;
                    if(newbase)
                        newbase->refcnt += refcnt - 1;
                    char * save = strdup(baseform);
                    prev = remove();
                    if(prev)
                        prev->removeUnneededPatterns(prev,Type,tailBuffer);
                    test("Shorten tail",save,tailBuffer,Type);
                    return true;
#else
                    if(m_next)
                        return m_next->removeUnneededPatterns(m_next,Type,tailBuffer,parent);
                    else
                        return false;
#endif
                    }
                else
                    if(  !BaseOfHigherUpRule->Next()    // We do not consider removing rules predicting ambiguous values, because they have nowhere to go.
                       && !BaseOfHigherUpRule->m_fullWord  // Also, rules that cannot be made longer have nowhere to go when deleted.
//                       && !strncmp(tailBuffer+1,baseform+1,borrow)
                       )
                    {
                    // NB: We cannot 'jump' over a level (borrow > 0): 
                    // the program doesn't terminate.
                    // Reason: the intention is that the odd high-up rule
                    // is resurrected at a lower level then were it was found.
                    // However, we can only garantee that it can be recreated 
                    // at the level just below its current level; it may then
                    // have the fullWord property. If it lands on the same 
                    // or lower level as the more general rule that we are
                    // going to create, then the problem isn't solved at all.
                    int rcnt = refcnt;
//                    bool higher = false;
                    parent->unmark();
                    /*
                    See whether there are sister-rules that also would benifit
                    from deleting a higher up odd rule. How often are they
                    used?
                        [bid]bid
                        [did]did
                        [fid]fid
                        [gid]gid
                        [lid]lid
                        [oid]oid
                        [pid]pid
                        [rid]rid
                        [sid]sid
                        [uid]uid
                        [vid]vid
                    */
                    for(node * nxt = parent->Next();nxt;nxt = nxt->Next())
                        {
                        base * nbase = nxt->basef;
                        if(nbase && !nbase->m_next) // Homographs are not allowed to vote, they must stay put
                            {
                            char * nbaseform = nbase->m_baseform;
                            char * ntail = nxt->m_tail;
                            strrev(ntail);
                            if(  !strncmp(ntail,nbaseform,nxt->m_len)
                              && !strcmp(nbaseform+nxt->m_len,m_baseform+len)
                              )
                                {
//                                if(nbase->refcnt > BaseOfHigherUpRule->refcnt)
  //                                  higher = true;
                                rcnt += nbase->refcnt;
                                nxt->mark();
                                }
                            strrev(ntail);
                            }
                        }
                    // Compare usage. If the higher-up rule is not used
                    // as often as this rule and its sisters, them the
                    // higher-up rule has to give up.
                    if(BaseOfHigherUpRule->refcnt < rcnt/*refcnt*/)
                        {
                        // Delete existing rule having relatively few 
                          // applications. It will resurrect in longer format,
                          // deeper into the tree.
                        /*
                        printf("BEFORE:\n");
                        parent->print(0);
                        */
                        Flex.remove(BaseOfHigherUpRule);
                        base * newbase = Flex.add(Type,tailBuffer+1/*+borrow*/,m_baseform+1/*+borrow*/,false);
                        if(newbase)
                            {
                            newbase->refcnt = rcnt;
                            parent = parent->removeAllMarked();
                            }
                        needed = false;
#if TEST
                        char * save = strdup(baseform);
#endif
                        prev = remove();
                        /*
                        printf("AFTER:\n");
                        parent->print(0);
                        */
//                        exit(0);
//#error remove print
                        if(prev)
                            prev->removeUnneededPatterns(prev,Type,tailBuffer,parent);
#if TEST
                        test("Replace incourant rule",save,tailBuffer,Type);
#endif
                        return true;
                        }
                    else 
                        {
//                        if(!higher && rcnt > BaseOfHigherUpRule->refcnt)
  //                          printf("rcnt %d refcnt %d BaseOfHigherUpRule->refcnt %d\n",rcnt,refcnt,BaseOfHigherUpRule->refcnt);

                        if(m_next)
                            return m_next->removeUnneededPatterns(m_next,Type,tailBuffer,parent);
                        else
                            return false;
                        }
                    }
                else if(m_next)
                    return m_next->removeUnneededPatterns(m_next,Type,tailBuffer,parent);
                else
                    return false;
                }
#else
            if(next)
                return next->removeUnneededPatterns(next,Type,tailBuffer);
            else
                return false;
#endif
            }
        else
            {
#if 1
            WRITE = true;
//            printf("?\n");
            base * newbase = Flex.add(Type,tailBuffer+1,m_baseform+1,false);
            WRITE = false;
            needed = false;
            if(newbase)
                newbase->refcnt += refcnt - 1;
#if TEST
            char * save = strdup(baseform);
#endif
            prev = remove();
            if(prev)
                prev->removeUnneededPatterns(prev,Type,tailBuffer,parent);
#if TEST
            test("new rule",save,tailBuffer,Type);
#endif
            return true;
#else
            if(m_next)
                return m_next->removeUnneededPatterns(m_next,Type,tailBuffer);
            else
                return false;
#endif
            }
        }
    else 
        {
//        printf("!\n");
        if(m_next)
            return m_next->removeUnneededPatterns(m_next,Type,tailBuffer,parent);
        else
            return false;
        }
    }
#if 0
int doit = 0;
void base::removeUnusedAlternatives()// remove baseform objects with zero count from ambiguous sequences
    {
            if(doit == 2)
                {
                printf(" %s %c\n",bf(),isFullWord()?'F':' ');
                }
    while(next && next->RefCount() == 0)
        {
            if(doit == 2)
                {
                printf("-%s %c\n",bf(),isFullWord()?'F':' ');
                }
        next = next->remove();
        }
    if(next)
        next->removeUnusedAlternatives();
    }
#endif

int base::Count()
    {
    if(m_next)
        return 1 + m_next->Count();
    else
        return 1;
    }

int base::sumRefCount()
    {
    if(m_next)
        return refcnt + m_next->sumRefCount();
    else
        return refcnt;
    }

void base::resetAdded()
    {
    added = false;
    if(m_next)
        m_next->resetAdded();
    }

void base::resetRefCount()
    {
    needed = true;
    refcnt = 0;
    if(m_next)
        m_next->resetRefCount();
    }

base * base::remove()
    {
    base * ret = m_next;
    m_next = 0;
    delete this;
    return ret;
    }

void base::print(int n)
    {
    printf("%*c%s] %d\n",n,'[',m_baseform,refcnt);
    if(m_next)
        m_next->print(n);
    }

void base::write(FILE * fp,const char * Type,const char * ending,int indent)
    {
//    fprintf(fp,"%6d ",n);

    if(added)
        fputc('+',fp);
    else
        fputc(' ',fp);

    if(needed)
        fputc(' ',fp);
    else
        fputc('#',fp);

    if(refcnt == 0)
        {
        if(*ending == ' ')
            fputc('%',fp);
        else
            fputc('&',fp);
        }
    else
        fputc(' ',fp);
            
    if(m_fullWord)
        fputc('*',fp);
    else
        fputc(' ',fp);
    if(indent < 0) // second, third,... baseform
        {
        fprintf(fp,"|%5d%15s %*c%s]%s\n",refcnt,Type,-indent,'[',m_baseform,ending);
        if(m_next)
            m_next->write(fp,Type,ending,indent);
        }
    else
        {
        if(m_next)
            {
            fprintf(fp,"|%5d%15s %*c%s]%s\n",refcnt,Type,indent,'[',m_baseform,ending);
            m_next->write(fp,Type,ending,-indent);
            }
        else
            {
            fprintf(fp," %5d%15s %*c%s]%s\n",refcnt,Type,indent,'[',m_baseform,ending);
            }
        }
    }

void base::write(FILE * fp,const char * Type,const char * ending)
    {
    if(refcnt > flex::CutoffRefcount)
        {
        if(flex::showRefcount)
#if TABASSEP
            fprintf(fp,"%s\t%s#%d\t%s\n",Type,m_baseform,refcnt,ending);
#else
            fprintf(fp,"%s\t[%s#%d]%s\n",Type,m_baseform,refcnt,ending);
#endif
        else
#if TABASSEP
            fprintf(fp,"%s\t%s\t%s\n",Type,m_baseform,ending);
#else
            fprintf(fp,"%s\t[%s]%s\n",Type,m_baseform,ending);
#endif
        }
    if(m_next)
        m_next->write(fp,Type,ending);
    }

/* Does not work as expected. Idea was to delete everything deeper than 
    n levels, n=1,2,3... and to rebuild the tree.

bool node::consolidate(node *& prev)
    {
    bool done = false;
    if(!consolidated)
        {
        if(!fixed)
            {
            delete sub;
            sub = 0;
            if(basef)
                consolidated = true;
            else
                {
                prev = remove();
                if(prev)
                    prev->consolidate(prev);
                return false;
                }
            }
        else
            {
            consolidated = true;
            if(sub)
                done = sub->consolidate(sub);
            }
        }
    else if(sub)
        done = sub->consolidate(sub);
    else
        done = true;

    if(next)
        done = next->consolidate(next) && done;

    return done;
    }

bool node::fix() 
    {
    if(!fixed)
        {
        if(sub)
            {
            fixed = sub->fix();
            }

        if(!fixed && basef && basef->Next())
            {
            fixed = true;
            }
        }
    // It should suffice to call fix only once. The paths to ambivalent 
    // baseforms are established after the first time 
    // extractFlexPatternsFromTaggedText is looped ready and they are never 
    // deleted.
    if(next)
        // If next is fixed, this node need not necessarily be fixed (it is 
        // not in the path to next)!
        return next->fix() || fixed;
    else
        return fixed;
    }
*/

bool node::remove(base * bf)
    {
    if(basef == bf)
        {
        basef = basef->remove();
        return true;
        }
    if(m_sub && m_sub->remove(bf))
        return true;
    if(m_next && m_next->remove(bf))
        return true;
    return false;
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

node * node::removeAllMarked()
    {
    if(marked)
        {
        assert(basef);
        delete basef;
        basef = 0;
        if(!m_sub)
            {
            node * nxt = m_next;
            m_next = 0;
            delete this;
            return nxt ? nxt->removeAllMarked() : 0;
            }
        }

    if(m_next)
        m_next = m_next->removeAllMarked();
    return this;
    }

bool node::Baseform(char * invertedWord,base *& bf,int & ln)
    {
//    if(base::nn < -1000 /*&& *tail*/)
  //      printf("%s =?= %s %d\n",tail,invertedWord,len);
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

base * node::add(char * tail,char * baseform,bool fullWord,node *& prev,bool empty
#if REFCNT
                 ,char * Refcnt/*!*/
#endif
                 )
    {
//    static int cnt = 0;
//    ++cnt;
    int n;
//    printf("%d    [%s]%s <%s>\n",cnt,baseform,tail,this->m_tail);
    // tail is inverted: last character first
    for(n = 0;tail[n] && this->m_tail[n] == tail[n];++n)
        ;
    if(n) // overlap
        {
        return addsub(tail,n,baseform,fullWord,prev
#if REFCNT
            ,Refcnt
#endif
            );
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
//            prev = new ending(tail,baseform,fullWord,this,false);
            prev = new node(this,tail,baseform,fullWord,false
#if REFCNT
                ,Refcnt
#endif
                );
            return prev->Base();
            }
/*
ending(tail,baseform,fullWord,next,empty)
  node(next,tail,baseform,fullWord,empty)
*/
        else if(m_next)
            return m_next->add(tail,baseform,fullWord,m_next,empty
#if REFCNT
            ,Refcnt
#endif
            );
        else 
            {
            m_next = new node(0,tail,baseform,fullWord,empty
#if REFCNT
                ,Refcnt
#endif
                );
            return m_next->Base();
//            return makeend(tail,baseform,fullWord,empty);
            }
        }
    }
//-----------------
bool node::BaseformSub(char * invertedWord,base *& bf,int & ln)
    {
    if(m_len && m_sub)
        {
//        if(base::nn < -1000)
  //          printf("sub %s\n",tail);
        if(   m_sub->Baseform(invertedWord+m_len,bf,ln) 
           /* Bart 20030909. The lemma should not reduce to nothing, so either
              the word must have some remaining "stem" or the replacement
              string must not be empty. */
//20040504           && (invertedWord[m_len] || *bf->bf())
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
//20040504      && (invertedWord[m_len] || *basef->bf())
      && (training || invertedWord[m_len] || *basef->bf())
      )
        {
//        if(base::nn < -1000)
  //          printf("OK\n");
        bf = basef;
        ln = m_len;
        return true;
        }
    return false;
    }

//-------------------
void node::removeNonFullWordsAsAlternatives()
    {
    if(m_sub)
        m_sub->removeNonFullWordsAsAlternatives();
    if(basef)
        {
        base * nxt = basef;
        while(nxt && !nxt->isFullWord())
            {
            nxt = nxt->Next();
            }
        if(nxt)
            {
            while(!basef->isFullWord())
                {
                basef = basef->remove();
                }
            basef->removeNonFullWordsAsAlternatives();
            }
        }
    if(m_next)
        m_next->removeNonFullWordsAsAlternatives();
    }
//------------------
void node::removeUnusedPatterns(node *& prev/*,const char * Type,char tailBuffer[]*/,bool first)
    {
/*    int len = strlen(tailBuffer);
    if(sub || basef)
        strcpy(tailBuffer+len,tail);*/
    if(m_sub)
        m_sub->removeUnusedPatterns(m_sub,/*Type,tailBuffer,*/true);
    if(basef)
        {
//        strrev(tailBuffer);
        basef->removeUnusedPatterns(basef/*,Type,tailBuffer*/);
//        strrev(tailBuffer);
        }
//    tailBuffer[len] = '\0';
    if(!m_sub && !basef)
        {
        prev = remove();
        if(prev)
            prev->removeUnusedPatterns(prev/*,Type,tailBuffer*/,first);
        }
    else if(m_next)
        m_next->removeUnusedPatterns(m_next/*,Type,tailBuffer*/,false);
    }
//------------------
void node::removeAmbiguous(node *& prev)
    {
    if(basef)
        {
        if(basef->Next())
            {
            delete basef;
            basef = 0;
            }
        }
    
    if(m_sub)
        m_sub->removeAmbiguous(m_sub);

    if(!m_sub && !basef)
        {
        prev = remove();
        if(prev)
            prev->removeAmbiguous(prev);
        }
    else if(m_next)
        m_next->removeAmbiguous(m_next);
    }
//------------------
void node::removeUnneededPatterns(node *& prev,const char * Type,char tailBuffer[],bool first,traverseTp how)
    {
    bool done = false;
    bool ambivalent = false;

    int len = strlen(tailBuffer);
    if(m_sub || basef)
        strcpy(tailBuffer+len,m_tail);
    
    if(basef)
        {
        if(!basef->Next())
            {
            if(how == normal/* || how == notdeeper*/) // Homographs can not be shortened
                {
                strrev(tailBuffer);
                done = basef->removeUnneededPatterns(basef,Type,tailBuffer,this);
                strrev(tailBuffer);
                }
            }
        else
            ambivalent = true;
        }
    
    if(!done && m_sub && (how == normal || how == onlydeeper))
        m_sub->removeUnneededPatterns(m_sub,Type,tailBuffer,true,ambivalent ? onlydeeper : normal);

    tailBuffer[len] = '\0';
    if(!m_sub && !basef)
        {
        prev = remove();
        if(prev)
            prev->removeUnneededPatterns(prev,Type,tailBuffer,first,how);
        }
    else if(m_next)
        m_next->removeUnneededPatterns(m_next,Type,tailBuffer,false,how);
    }
//---------------------
#if 0
void node::removeUnusedAlternatives(char * tailBuffer)
// remove baseform objects with zero count from ambiguous sequences
// If all counts are zero, then the last alternative is kept! (Hm...)
    {
    
    int len = strlen(tailBuffer);
    if(sub || basef)
        strcpy(tailBuffer+len,tail);
        
    if(sub)
        sub->removeUnusedAlternatives(tailBuffer);
    if(basef)
        {

        base * b;
        doit = 0;
        for(b = basef;b && b->Next();b=b->Next())
            {
            if(b->isFullWord())
                doit |= 1;
            if(b->RefCount() == 0)
                doit |= 2;
            }
        if(b->isFullWord())
            doit |= 1;
        if(doit == 2)
            printf("%s\n",tailBuffer);
        while(basef->RefCount() == 0 && basef->Next())
            {
            if(doit == 2)
                {
                printf("-%s %c\n",basef->bf(),b->isFullWord()?'F':' ');
                }
            basef = basef->remove();
            }
        if(basef->RefCount() != 0)
            basef->removeUnusedAlternatives();
        if(doit == 2)
            getchar();
        }
    tailBuffer[len] = '\0';
    if(next)
        next->removeUnusedAlternatives(tailBuffer);
    }
#endif
//---------------
int node::Count()
    {
    int cnt = 0;
    if(m_sub)
        cnt = m_sub->Count();
    if(basef)
        cnt += basef->Count();
    if(m_next)
        cnt += m_next->Count();
    return cnt;
    }
//------------------
int node::sumRefCount()
    {
    int cnt = 0;
    if(m_sub)
        cnt = m_sub->sumRefCount();
    if(basef)
        cnt += basef->sumRefCount();
    if(m_next)
        cnt += m_next->sumRefCount();
    return cnt;
    }
//------------------
void node::resetAdded()
    {
    if(m_sub)
        m_sub->resetAdded();
    if(basef)
        basef->resetAdded();
    if(m_next)
        m_next->resetAdded();
    }

void node::resetRefCount()
    {
    if(m_sub)
        m_sub->resetRefCount();
    if(basef)
        basef->resetRefCount();
    if(m_next)
        m_next->resetRefCount();
    }
//----------------

node::node(node * next,char * tail,char * baseform,bool fullWord,bool empty
#if REFCNT
           ,char * Refcnt/*!*/
#endif
           ):m_next(next),m_sub(0)//,fixed(false),consolidated(false)
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
//    printf(">[%s]%s\n",baseform,tail);
    basef = new base(baseform,fullWord
#if REFCNT
        ,Refcnt
#endif
        );
    ++mutated;
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }

node::node(node * next,char * tail,int n,char * baseform,bool fullWord,bool empty,node * sub
#if REFCNT
           ,char * Refcnt/*!*/
#endif
           ):m_next(next),m_sub(sub)//,fixed(false),consolidated(false)
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
//    printf("<[%s]%s\n",baseform,tail);
    basef = new base(baseform,fullWord
#if REFCNT
        ,Refcnt
#endif
        );
    ++mutated;
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    }

node::node(node * next,char * tail,int n,node * sub):m_next(next),basef(0),m_sub(sub)//,fixed(false),consolidated(false)
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
/*
select::select(node * next,node * sub,char * tail,int n):node(next,tail,n,sub)
    {
    }

ending::ending(char * tail,char * baseform,bool fullWord,node * next,bool empty):node(next,tail,baseform,fullWord,empty)
    {
    }
*/
//-----------------
void node::print(int n)
    {
    printf("%*c%s}\n",n,'{',m_tail);
    if(m_sub)
        m_sub->print(n+2);
    if(basef)
        basef->print(n);
    if(m_next)
        m_next->print(n);
    }
//-----------------
void node::write(FILE * fp,const char * Type,int indent,char tailBuffer[])
    {
    if(m_sub)
        {
//        fprintf(fp,"%*c> {%s} %d\n",indent,'-',tail,len);
        int Len = strlen(tailBuffer);
        strcpy(tailBuffer+Len," ");
        strcpy(tailBuffer+Len+1,m_tail);
        m_sub->write(fp,Type,indent+2,tailBuffer);
        tailBuffer[Len] = '\0';
        }
    if(basef)
        {
//        fprintf(fp,"%*c= {%s} %d\n",indent,'=',tail,len);
        int Len = strlen(tailBuffer);
        strcpy(tailBuffer+Len," ");
        strcpy(tailBuffer+Len+1,m_tail);
        strrev(tailBuffer);
        basef->write(fp,Type,tailBuffer,indent);
        strrev(tailBuffer);
        tailBuffer[Len] = '\0';
        }
    if(m_next)
        m_next->write(fp,Type,indent,tailBuffer);
    }

void node::write(FILE * fp,const char * Type,char tailBuffer[])
    {
    if(m_sub)
        {
        int Len = strlen(tailBuffer);
        strcpy(tailBuffer+Len,m_tail);
        m_sub->write(fp,Type,tailBuffer);
        tailBuffer[Len] = '\0';
        }
    if(basef)
        {
        int Len = strlen(tailBuffer);
        strcpy(tailBuffer+Len,m_tail);
        strrev(tailBuffer);
        basef->write(fp,Type,tailBuffer);
        strrev(tailBuffer);
        tailBuffer[Len] = '\0';
        }
    if(m_next)
        m_next->write(fp,Type,tailBuffer);
    }
//---------------------
base * node::addsub(char * tail,int n,char * baseform,bool fullWord,node *& prev
#if REFCNT
                    ,char * Refcnt/*!*/
#endif
                    )
    {
    if(tail[n])
        {
        if(this->m_tail[n])
            {
            // insert new tail before this one, move both to subtree 
            if(this->m_tail[n] > tail[n])
                {
                cut(n);
                prev = new node(m_next,tail,n,new node(this,tail+n,baseform,fullWord,false
#if REFCNT
                    ,Refcnt
#endif
                    ));
                m_next = 0;
                return prev->m_sub->Base();
                }
            else
                {
                cut(n);
                prev = new node(m_next,tail,n,this);
                m_next = new node(0,tail+n,baseform,fullWord,false
#if REFCNT
                    ,Refcnt
#endif
                    );
                return m_next->Base();
                }
            }
        else
            {
            if(m_sub)
                return m_sub->add(tail+n,baseform,fullWord,m_sub,false
#if REFCNT
                ,Refcnt
#endif
                );
            else
                {
                m_sub = new node(0,tail+n,baseform,fullWord,false
#if REFCNT
                    ,Refcnt
#endif
                    );
                return m_sub->Base();
                }
            }
        }
    else
        {
        if(this->m_tail[n])
            {
            cut(n);
            prev = new node(m_next,tail,n,baseform,fullWord,false,this
#if REFCNT
                ,Refcnt
#endif
                );
            m_next = 0;
            return prev->Base();
            }
        else
            {
            if(basef)// exact match, word with more than one baseform
                {
                basef->add(baseform,fullWord,basef
#if REFCNT
                    ,Refcnt
#endif
                    );
                }
            else
                {
                basef = new base(baseform,fullWord
#if REFCNT
                    ,Refcnt
#endif
                    ,0);
                }
            return basef;
            }
        }
#if 0
    if(tail[n] && !this->m_tail[n])// the new tail is longer, 
                    //add the new [baseform]tail combination to the subtree
        {
        if(sub)
            sub->add(tail+n,baseform,fullWord,sub,false);
        else
            sub = new node(0,tail+n,baseform,fullWord,false);
        }
    else if(tail[n] && this->m_tail[n])
        {
        // insert new tail before this one, move both to subtree 
        if(this->m_tail[n] > tail[n])
            {
            cut(n);
//            prev = new select(next,new ending(tail+n,baseform,fullWord,this,false),tail,n);
//            prev = new node(next,tail,n,new ending(tail+n,baseform,fullWord,this,false));
            prev = new node(next,tail,n,new node(this,tail+n,baseform,fullWord,false));
/*
ending(tail,baseform,fullWord,next,empty)
  node(next,tail,baseform,fullWord,empty)
*/
/*
select(next,sub,tail,n):
  node(next,tail,n,sub)
*/

            next = 0;
            }
        else
            {
            cut(n);
//            prev = new select(next,this,tail,n);
            prev = new node(next,tail,n,this);
//            next = new ending(tail+n,baseform,fullWord,0,false);
            next = new node(0,tail+n,baseform,fullWord,false);
            }
        }
    else if(!tail[n] && this->m_tail[n])
        {
        cut(n);
        prev = new node(next,tail,n,baseform,fullWord,false,this);
        next = 0;
        }
    else if(!tail[n] && !this->m_tail[n])
        {
//        if(base::nn < -1000)
  //          printf("![%s]%s\n",baseform,tail);
        if(basef)// exact match, word with more than one baseform
            {
            basef->add(baseform,fullWord,basef);
            }
        else
            {
            basef = new base(baseform,fullWord,0);
            }
        }
#endif
    }
/*
void type::fix()
    {
    if(!fixed)
        {
        if(end)
            end->fix();
        fixed = true;
        }
    if(next)
        next->fix();
    }

bool type::consolidate()
    {
    bool done = false;
    if(end)
        done = end->consolidate(end);
    if(next)
        done = next->consolidate() && done;
    return done;
    }
*/

void type::remove(base * bf)
    {
    if(end && end->remove(bf))
        return;
    if(m_next)
        m_next->remove(bf);
    }

type::type(const char * tp,char * tail,char * baseform,bool fullWord,
#if REFCNT
           char * Refcnt/*!*/,
#endif
           type * next):m_next(next),fixed(false)
    {
    this->m_tp = new char[strlen(tp)+1];
    strcpy(this->m_tp,tp);
    if(*tail)
//        end = new ending(tail,baseform,fullWord,0,false);
        end = new node(0,tail,baseform,fullWord,false
#if REFCNT
        ,Refcnt
#endif
        );
    else
        {
        printf("type::type TYPE %s TAIL 0\n",tp);
//        end = new ending(tail,baseform,fullWord,0,false);
        end = new node(0,tail,baseform,fullWord,false
#if REFCNT
            ,Refcnt
#endif
            );
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

base * type::add(const char * tp,char * tail,char * baseform,bool fullWord,type *& prev
#if REFCNT
                 ,char * Refcnt/*!*/
#endif
                 )
    {
//    printf("add %s [%s]%s\n",tp,baseform,tail);
    int cmp = strcmp(this->m_tp,tp);
    if(!cmp)
        {
        if(*tail)
            {
//    if(base::nn < -1000)
  //                  printf("x[%s]%s\n",baseform,tail);
            return end->add(tail,baseform,fullWord,end,false
#if REFCNT
                ,Refcnt
#endif
                );
            }
        else
            {
//    if(base::nn < -1000)
  //          printf("type::add <%s,%s,%s,%d>\n",tp,tail,baseform,fullWord);
            return end->add(tail,baseform,fullWord,end,true
#if REFCNT
                ,Refcnt
#endif
                );
            }
        }
    else if(cmp > 0)
        {
//            if(base::nn < -1000)
  //                  printf("add1[%s]%s\n",baseform,tail);

        prev = new type(tp,tail,baseform,fullWord,
#if REFCNT
            Refcnt,
#endif
            this);
        return prev->Base();
        }
    else if(m_next)
        return m_next->add(tp,tail,baseform,fullWord,m_next
#if REFCNT
        ,Refcnt
#endif
        );
    else
        {
//            if(base::nn < -1000)
  //                  printf("add2[%s]%s\n",baseform,tail);
        m_next = new type(tp,tail,baseform,fullWord
#if REFCNT
            ,Refcnt
#endif
            );
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

char * type::Baseform(char * invertedWord,base *& bf,int & ln)
    {
    base * BF = 0;
    int LN = -1;
    char * TP = 0;
    if(m_next)
        TP = m_next->Baseform(invertedWord,BF,LN);
    if(end->Baseform(invertedWord,bf,ln))
        {
#if REFCNT
        if(TP && BF && bf->RefCount() < BF->RefCount())
#else
        if(TP && BF && LN > ln) // choose the longest 
                        // (what if there are more than one?)
                // "longest" rule (rule that looks at most characters of 
                // word's ending to decide on the proper baseform) works 
                // better than both refcount and "shortest" rule. A priory, 
                // the latter should be better, because shorter rules should
                // be more general than longer rules. But then again, the 
                // chance for mistakes is greater if the rule looks at fewer
                // characters before deciding.
#endif
            {
            bf = BF;
            ln = LN;
            return TP;
            }
        else
            return this->m_tp;
        }
    else if(TP)
        {
        bf = BF;
        ln = LN;
        return TP;
        }
    else
        return 0;
    }

void type::print()
    {
    printf("%s\n",m_tp);
    if(end)
        end->print(2);
    if(m_next)
        m_next->print();
    }

void type::write(FILE * fp,bool nice)
    {
    char tailBuffer[256];
    tailBuffer[0] = '\0';
    if(end)
        {
        if(nice)
            end->write(fp,m_tp,1,tailBuffer);
        else
            end->write(fp,m_tp,tailBuffer);
        }
    if(m_next)
        m_next->write(fp,nice);
    }

void type::removeNonFullWordsAsAlternatives()
    {
    if(end)
        end->removeNonFullWordsAsAlternatives();
    if(m_next)
        m_next->removeNonFullWordsAsAlternatives();
    }

void type::removeUnusedPatterns(type *& prev)
    {
/*    char tailBuffer[256];
    tailBuffer[0] = '\0';*/
    if(end)
        {
        end->removeUnusedPatterns(end/*,tp,tailBuffer*/,true);
        if(!end)
            {
            prev = remove();
            if(prev)
                prev->removeUnusedPatterns(prev);
            }
        else if(m_next)
            m_next->removeUnusedPatterns(m_next);
        }
    else if(m_next)
        m_next->removeUnusedPatterns(m_next);
    }

void type::removeAmbiguous(type *& prev)
    {
    if(end)
        {
        end->removeAmbiguous(end);
        if(!end)
            {
            prev = remove();
            if(prev)
                prev->removeAmbiguous(prev);
            return;
            }
        }
    if(m_next)
        m_next->removeAmbiguous(m_next);
    }

void type::removeUnneededPatterns(type *& prev)
    {
    char tailBuffer[256];
    tailBuffer[0] = '\0';
    if(end)
        {
        end->removeUnneededPatterns(end,m_tp,tailBuffer,true,normal);
        if(!end)
            {
            prev = remove();
            if(prev)
                prev->removeUnneededPatterns(prev);
            }
        else if(m_next)
            m_next->removeUnneededPatterns(m_next);
        }
    else if(m_next)
        m_next->removeUnneededPatterns(m_next);
    }

#if 0
void type::removeUnusedAlternatives()// remove baseform objects with zero count from ambiguous sequences
    {
    char tailBuffer[256];
    tailBuffer[0] = '\0';
    if(end)
        end->removeUnusedAlternatives(tailBuffer);
    if(next)
        next->removeUnusedAlternatives();
    }
#endif

int type::Count()
    {
    int cnt = 0;
    if(end)
        cnt = end->Count();
    if(m_next)
        cnt += m_next->Count();
    return cnt;
    }

int type::sumRefCount()
    {
    int cnt = 0;
    if(end)
        cnt = end->sumRefCount();
    if(m_next)
        cnt += m_next->sumRefCount();
    return cnt;
    }

void type::resetAdded()
    {
    if(end)
        end->resetAdded();
    if(m_next)
        m_next->resetAdded();
    }

void type::resetRefCount()
    {
    if(end)
        end->resetRefCount();
    if(m_next)
        m_next->resetRefCount();
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
        "taggedText\n%d "
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
        taggedText::COUNT,
        Word::COUNT
        );
    getchar();
#endif
    }


void flex::remove(base * bf)
    {
    if(types)
        types->remove(bf);
    }

void flex::removeNonFullWordsAsAlternatives()
    {
    if(types)
        types->removeNonFullWordsAsAlternatives();
    }

void flex::removeUnusedPatterns()
    {
    if(types)
        types->removeUnusedPatterns(types);
    }

//------------------
void flex::removeAmbiguous()
    {
    if(types)
        types->removeAmbiguous(types);
    }

void flex::removeUnneededPatterns()
    {
    if(types)
        types->removeUnneededPatterns(types);
    }

#if 0
void flex::removeUnusedAlternatives()// remove baseform objects with zero count from ambiguous sequences
    {
    if(types)
        types->removeUnusedAlternatives();
    }
#endif

int flex::Count()
    {
    if(types)
        return types->Count();
    else
        return 0;
    }

int flex::sumRefCount()
    {
    if(types)
        return notadded + types->sumRefCount();
    else
        return notadded;
    }

void flex::resetAdded()
    {
//    unchanged();
    if(types)
        types->resetAdded();
    }

void flex::resetRefCount()
    {
    notadded = 0;
    unchanged();
    if(types)
        types->resetRefCount();
    }


void flex::trim(char * s)
    {
    for(char * t = s + strlen(s) - 1;t >= s && isSpace(*t);--t)
        *t = '\0';
    }

void flex::print()
    {
    if(types)
        types->print();
    }

void flex::write(FILE * fp,bool nice)
    {
    if(types)
        types->write(fp,nice);
    }

bool flex::Baseform2(char * word,const char * tag,base *& bf,int & offset)
    {
    if(types)
        {
        offset = 0;
        strrev(word);
        bool ret = types->Baseform(word,tag,bf,offset);
        strrev(word);
        return ret;
        }
    else
        return false;
    }

bool flex::Baseform(char * word,const char * tag,const char *& bf,int & borrow)
    {
    if(newStyleRules())
        {
        bf = applyRules(word,tag);
        return true;
        }

    if(types)
        {
        int offset = 0;
        int wlen = strlen(word);
        if(wlen > 256)
            {
            if(baseformsAreLowercase)
                AllToLower(word); // destructive, because overwriting word!
                                // But this is exceptional to begin with.
            bf = word;
            borrow = wlen;
            return true;
            }
        static char aWord[256];
        strcpy(aWord,word);
        if(baseformsAreLowercase)
            AllToLower(aWord);
        base * Base;
        strrev(aWord);
        if(types->Baseform(aWord,tag,Base,offset))
            {
            borrow = wlen - offset;
//            strrev(word);
//            printf("WORD %s",word);
            static char buf[256];
            char * b = buf;
            while(true)
                {
                strncpy(b,word,borrow);
                strcpy(b+borrow,Base->bf());
//                printf(" BASEFORM %s",b);

                if(baseformsAreLowercase)
                    AllToLower(b);
                else if(isAllUpper(word))
                    {
                    AllToUpperISO(b);
//                    printf("AllToUpperISO: %s\n",buf);
                    }
                else if(borrow == 0 && is_Upper(word))
                    {
                    toUpper(b);
//                    printf("toUpper: %s\n",buf);
                    }

                b += strlen(b);
                Base = Base->Next();
                if(Base)
                    {
                    *b++ = ' ';
//                    printf(" |");
                    }
                else
                    break;
                }
            bf = buf;
//            printf("\n");
            return true;
            }
        else
            {
//            strrev(word);
            return false;
            }
        }
    else
        return false;
    }

char * flex::Baseform(char * word,const char *& bf,int & borrow)
    {
    if(newStyleRules())
        {
        bf = applyRules(word);
        return "-";
        }


    if(types)
        {
        int offset = 0;
        int wlen = strlen(word);
        if(wlen > 256)
            {
            if(baseformsAreLowercase)
                AllToLower(word); // destructive, because overwriting word!
                                // But this is exceptional to begin with.
            bf = word;
            borrow = wlen;
            return 0;
            }
        static char aWord[256];
        strcpy(aWord,word);
        if(baseformsAreLowercase)
            AllToLower(aWord);
        base * Base;
        strrev(aWord);
        char * tag = types->Baseform(aWord,Base,offset);
        if(tag)
            {
            borrow = wlen - offset;
//            strrev(word);
//            printf("WORD %s",word);
            static char buf[256];
            char * b = buf;
            while(true)
                {
                strncpy(b,word,borrow);
                strcpy(b+borrow,Base->bf());
//                printf(" BASEFORM %s",b);

/*                if(isAllUpper(word))
                    {
                    AllToUpperISO(b);
//                    printf("AllToUpperISO: %s\n",buf);
                    }
                else if(borrow == 0 && isUpper(word))
                    {
                    toUpper(b);
//                    printf("toUpper: %s\n",buf);
                    }*/
                if(baseformsAreLowercase)
                    AllToLower(b); // We have no lexical type information to 
                // decide whether capitals should be used, so we assume all 
                // lower case.

                b += strlen(b);
                Base = Base->Next();
                if(Base)
                    {
                    *b++ = ' ';
//                    printf(" |");
                    }
                else
                    break;
                }
            bf = buf;
//            printf("\n");
            return tag;
            }
        else
            {
//            strrev(word);
            return 0;
            }
        }
    else
        return 0;
    }


base * flex::update(char * baseForm,char * word,const char * tag,bool partial)
    {
    //bool tempAmbiguos = false;
/*    printoe = false;
    if(!partial && !strcmp(word,"øer") && !strcmp(baseForm,"ø"))
        {
        printf("[%s]%s %s\n",baseForm,word,tag);
        printoe = true;
        }*/
    base * ret = 0;
    size_t i = 0;
//    if(base::nn < -1000)
    if(WRITE)
        printf("[%s]%s\n",baseForm,word);
//    if(base::nn == 109)
  //      Exit();
   // if(base::nn < 1000)
//    if(WRITE)
//        printf("update [%s]%s %s\n",baseForm,word,tag);

    // Find the shortest [basef]end combination that is distinct from
    // already existing combinations by removing a common "stem" from the start
    // such that stem+basef = baseForm and stem+end = word.
    if(types)
        {
        int wlen = strlen(word);
        int offset = 0;
        strrev(word);
        // Now the full form word is inverted!
        base * Base;
        // Retrieve current prediction into Base and offset. 
        // The length of the matched tail is stored in offset.
        if(types->Baseform(word,tag,Base,offset))
            {
//    if(base::nn < 1000)
//    if(WRITE)
/*        if(printoe)
        printf("found\n");*/
            strrev(word);
            // The full form is back to normal again.
            i = wlen - offset; // compute the length of the full form that 
                                    // isn't touched by the rule (the 'stem').
            if(  i > strlen(baseForm) // The true base form is shorter than
                    // the implied stem, or, in other words, the stem as  
                    // implied by the rule is too long.
              || strncmp(baseForm,word,i) // The implied stem is not a stem 
                                // at all: the first i characters of the
                                // word and the true base form are not equal
              )
                {
                i = 0;
                // compute the stem's (maximum) length
                while(word[i] && word[i] == baseForm[i])
                    {
                    ++i;
                    }
                }
            else
                {
#if 1
                if(i == 0 && !partial)
                    {
                    while(Base)
                        {
                        if(!strcmp(Base->bf(),baseForm + i))
                            // Agreement!
                            {
                            //if(WRITE)
/*                                if(printoe)
                                printf("X nothing to do i %d baseForm %s [%s] == [%s]\n",i,baseForm,baseForm + i,Base->bf());*/
                            Base->setFullWord();
                            Base->incRefCount();
                            return Base; // nothing to do
                            }
                        
                        //printf(" [%s]\n",Base->bf());
                        Base = Base->Next(); // Look at the next prediction 
                                            // (in case of ambiguous rule)
                        }
//                    tempAmbiguos = true;
                    }
                else
                    {
                    // If Base holds a genuine ambiguity (two whole words) and
                    // this word is longer then we have to add a longer rule!
                    int cnt = 0;
                    base * Base2 = Base;
                    while(Base2)
                        {
                        if(Base2->isFullWord())
                            ++cnt;
                        Base2 = Base2->Next(); // Look at the next prediction 
                                        // (in case of ambiguous rule)
                        }
                    if(cnt < 2)
                        {
                        while(Base)
                            {
                            if(!strcmp(Base->bf(),baseForm + i))
                                // Agreement!
                                {
                                //if(WRITE)
/*                                    if(printoe)
                                        printf("Y nothing to do i %d baseForm %s [%s] == [%s]\n",i,baseForm,baseForm + i,Base->bf());*/
                                Base->incRefCount();
                                return Base; // nothing to do
                                }
                            Base = Base->Next(); // Look at the next prediction 
                                                // (in case of ambiguous rule)
                            }
                        }
                    }
                // The predicted stem is compatible with the true base form.
                // Now check that the predicted base form is good.
#else
    /*
    This code produces a smaller rule set, but does not disambiguate fully.
    Some non-genuine ambiguity is left.
    */
                while(Base)
                    {
                    if(!strcmp(Base->bf(),baseForm + i))
                        // Agreement!
                        {
                        if(WRITE)
                            printf("nothing to do [%s] == [%s]\n",baseForm + i,Base->bf());
                        if(i == 0 && !partial)
                            {
                            Base->setFullWord();
                            }
                        Base->incRefCount();
                        return Base; // nothing to do
                        }
                    Base = Base->Next(); // Look at the next prediction 
                                        // (in case of ambiguous rule)
                    }
                // None of the constructed baseforms is correct. Extend the tail of
                // the non-baseform ending with one character and store it with the
                // correct baseform ending.
#endif
                if(i > 0)
                    // No agreement. We need a more specific rule.
                    --i; // shorten the stem, or, make the tail pattern longer.
                else
                    {
               //     printf("CALAMITY %s %s\n",baseForm,word);
                 //   sBase->print(0);
                    }
                }
            }
        else // Could not predict a base form. Possible causes: tag isn't 
            // known yet or no tail pattern matches end of word.
            {
            strrev(word);
            // Full form back to normal.
            // compute the stem's (maximum) lenght
            while(word[i] && word[i] == baseForm[i])
                {
                ++i;
                }
            }
        }
    else // Just beginning: we do not have any rules at all.
        {
        // compute the stem's (maximum) lenght
        while(word[i] && word[i] == baseForm[i])
            {
            ++i;
            }
        }

//    printf(" i==%d ",i);
    char * end = word+i;
    if(!*end && i && baseForm[i])
        {           // do not allow entries such as [e] , i.e. a rule saying 
                    // that, per default, a word can be extended with 'e'.
                    // The rule [] (a word remains unchanged) is not in the 
                    // rule base, but assumed a priori.
        --i;
        --end;
        }
    if(  i == 0 && !partial // We can allow an empty ending of the full form 
                            // or the base form if the stem has zero length 
                            // and the word is a complete word.
      || *end 
      || baseForm[i]
      )
        {
//        if(base::nn < 1000)
  //          printf("+[%s]%s\n",baseForm+i,end);
/*        if(tempAmbiguos)
            {
            printf("+[%s]%s\n",baseForm+i,end);
            exit(0);
            }*/
/*        if(printoe)
            printf("+[%s]%s\n",baseForm+i,end);*/
        ret = add(tag,end,baseForm+i,i == 0 && !partial);
        /*
        strrev(end);
        if(types)
            {
            ret = types->add(tag,end,baseForm+i,i == 0 && !partial,types);
            }
        else
            {
//            if(base::nn < -1000)
    //                printf("update[%s]%s\n",baseForm+i,end);
            types = new type(tag,end,baseForm+i,i == 0 && !partial);
            ret = types->Base();
            }
        strrev(end);
        */
        }
    else
        notadded++;
    return ret;
    }

base * flex::add(const char * tag,char * end,char * baseform,bool fullWord)
    {
    base * ret;
    strrev(end);
    if(types)
        {
        ret = types->add(tag,end,baseform,fullWord,types
#if REFCNT
            ,0
#endif
            );
        }
    else
        {
//            if(base::nn < -1000)
//                printf("update[%s]%s\n",baseForm+i,end);
        types = new type(tag,end,baseform,fullWord,0);
        ret = types->Base();
        }
    strrev(end);
    return ret;
    }

#ifdef FLEXIBLE
base * flex::add(char * line)
    {
//    printf("line %s\n",line);
    base * ret = 0;
    static int cnt = 0;
    ++cnt;
    if(!line)
        return 0;
    if(*line != '\t' && *line != ' ' && !is_Alpha(*line))
        return 0;
    bool plus = strchr(line,'+') != 0;
    bool bracket = strchr(line,'[') != 0;
    if(!plus && !bracket)
        return 0;

    char * tp;
    if('A' <= *line && *line <= 'Z')
        {
        char * c = line;
        while(*c && !isSpace(*c))
            ++c;
        if(!*c)
            return 0;
        *c = '\0';
        tp = line;
        line = c + 1;
        }
    else
        tp = "N";
    while(*line && isSpace(*line))
        ++line;
    if(!*line)
        return 0;

    bool minus = strchr(line,'-') != 0;
    char * end;
    char * basef;
    if(plus && minus)
        {
        // 	-mænd 	+ mand
        //	-skibe	+skib	
        end = strchr(line,'-');
        assert(end);
        while(*++end && isSpace(*end))
            ;
        for(line = end;*++line && !isSpace(*line);)
            ;
        if(!*line)
            return 0;
        *line++ = '\0';
        basef = strchr(line,'+');
        assert(basef);
        while(*++basef && isSpace(*basef))
            ;
        for(line = basef;*++line && !isSpace(*line);)
            ;
        *line = '\0';
        strrev(end);
        // In fact, we have no way to know whether baseform and end are 
        // complete words or not. To stay on the save side, we assume that
        // they are. (fullWord = true)
        if(types)
            {
            ret = types->add(tp,end,basef,true,types
#if REFCNT
                ,0
#endif
                );
            }
        else
            {
            types = new type(tp,end,basef,true,0);
            ret = types->Base();
            }
        strrev(end);
        }
    else
        {
        //N	+en,[og]øger,[og]øgerne
        //N	+en,+,+ene
        char * next;
        for(;line;line = next)
            {
            next = strchr(line,',');
            if(next)
                *next++ = '\0';
            basef = strchr(line,'[');
            if(basef)
                {
                ++basef;
                char * close = strchr(basef,']');
                if(!close)
                    {
                    printf("missing ']' in line %d\n",cnt);
                    return 0;
                    }
                *close++ = '\0';
                end = close;
                }
            else
                {
                basef = "";
                end = line;
                if(*end == '+')
                    ++end;
                else
                    continue;
                }
            trim(tp);
            trim(end);
            trim(basef);
        // In fact, we have no way to know whether baseform and end are 
        // complete words or not. To stay on the save side, we assume that
        // they are. (fullWord = true)
#if REFCNT
            char * Refcnt = strchr(end,'\t'); // added 20020819, optional Refcnt info
            if(Refcnt)
                {
                *Refcnt++ = '\0';
//                printf("end %s Refcnt %s\n",end,Refcnt);
                }
#endif
            strrev(end);
            if(types)
                {
//                printf("BBB ");
#if REFCNT
                if(Refcnt)
                    ret = types->add(tp,end,basef,true,types,Refcnt);
                else
#endif
                    ret = types->add(tp,end,basef,true,types
#if REFCNT
                    ,0
#endif
                    );
                }
            else
                {
#if REFCNT
                if(Refcnt)
                    types = new type(tp,end,basef,true,Refcnt);
                else
#endif
                    types = new type(tp,end,basef,true,0);
                ret = types->Base();
                }
            strrev(end);
            }
        }
    return ret;
    }
#else // FLEXIBLE not defined
base * flex::add(char * line)
    {
    if(strchr(line,' '))
        return 0; // 20080221
    base * ret/* = 0*/;
    static int cnt = 0;
    ++cnt;
//    bool bracket = strchr(line,'[') != 0;
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
#if !TABASSEP
    while(*line && isSpace(*line)) // skip white space following word class
        ++line;
#endif
    if(!*line)
        return 0; // no flex rule following white space.

    char * end;
    char * basef;
    //N	[og]øger
#if TABASSEP
    basef = line;
#else
    basef = strchr(line,'[');
#endif
    if(basef)
        {
#if TABASSEP
        char * close = strchr(basef,'\t');
#else
        ++basef;
        char * close = strchr(basef,']');
#endif
        if(!close)
            {
#if TABASSEP
            printf("missing '\\t' (TAB) in line %d\n",cnt);
#else
            printf("missing ']' in line %d\n",cnt);
#endif
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
//    trim(end);
    char * t;
    for(t = end;*t && !isSpace(*t);++t)
        ;
    if(*t)
        *t = '\0'; // This allows for a comment after the word ending, e.g. a count.
    
    trim(basef);
// In fact, we have no way to know whether baseform and end are 
// complete words or not. To stay on the safe side, we assume that
// they are. (fullWord = true)
#if REFCNT
    char * Refcnt = strchr(end,'\t'); // added 20020819, optional Refcnt info
    if(Refcnt)
        {
        *Refcnt++ = '\0';
        }
#endif
        {
        strrev(end);
        if(types)
            {
#if REFCNT
            if(Refcnt)
                ret = types->add(tp,end,basef,true,types,Refcnt);
            else
#endif
                ret = types->add(tp,end,basef,true,types
#if REFCNT
                ,0
#endif
                );
            }
        else
            {
#if REFCNT
            if(Refcnt)
                types = new type(tp,end,basef,true,Refcnt);
            else
#endif
                types = new type(tp,end,basef,true,0);
            ret = types->Base();
            }
        strrev(end);
        }
    return ret;
    }
#endif



#if 0 // Bart 20030910 Not used
#if 1
    bool flex::updateFlexRulesIfNeeded(lext * Plext,int nmbr,// The dictionary's available 
        // lexical information for this (tagged) word.
        char * word,char * tag)// word and tag as found in the text
        {
        lext * plext = Plext;
        char * w;
        if(tag)
            {
            const char * Tp = Lemmatiser->translate(tag); // tag as found in the text
            /*
            for(int i = 0;i < tagcnt;++i)
                {
                if(!strcmp(tag,textTags[i]))
                    {
                    Tp = dictTags[i]; // translate tag to dictionary-type
                    break;
                    }
                }
            */
            // See whether the word's tag can be found in the 
            // dictionary's lexical information.
            for(int n = nmbr;n;--n,++plext)
                {
                if(!strcmp(Tp,plext->Type)) // Word is in dictionary,
                    // and type info matches.
                    {
                    char correctBaseForm[1000];
                    char * cBF = correctBaseForm;
                    int off = plext->S.Offset; 
                    for(w = word;*w && off;--off)
                        {
                        *cBF++ = *w++;
                        }
                    strcpy(cBF,plext->BaseFormSuffix);
//                    printf(" correctBaseForm %s\n",correctBaseForm);
//                    putchar('A');
                    update(correctBaseForm,word,tag,false);
                    //break; // disregard other readings
                    }
                }
            
            }
        return true;
        }
#else
    bool flex::updateFlexRulesIfNeeded(lext * Plext,int nmbr,// The dictionary's available 
        // lexical information for this (tagged) word.
        char * word,char * tag)// word and tag as found in the text
        {
        lext * plext = Plext;
        char * w;
        if(tag)
            {
            char * Tp = tag; // tag as found in the text
            for(int i = 0;i < tagcnt;++i)
                {
                if(!strcmp(tag,textTags[i]))
                    {
                    Tp = dictTags[i]; // translate tag to dictionary-type
                    break;
                    }
                }
            
            char * predictedBaseform;// = baseform(word,tag);
            if(!Baseform(word,tag,predictedBaseform))
                predictedBaseform = word;
//            printf("word %s predictedBaseform %s ",word,predictedBaseform);
            
            // See whether the word's tag can be found in the 
            // dictionary's lexical information.
            for(int n = nmbr;n;--n,++plext)
                {
                if(!strcmp(Tp,plext->Type)) // Word is in dictionary,
                    // and type info matches.
                    {
                    char correctBaseForm[1000];
                    char * cBF = correctBaseForm;
                    int off = plext->Offset; 
                    for(w = word;*w && off;--off)
                        {
                        *cBF++ = *w++;
                        }
                    strcpy(cBF,plext->BaseForm);
//                    printf(" correctBaseForm %s\n",correctBaseForm);
                    char * x = strstr(predictedBaseform,correctBaseForm);
                    int kar;
                    if(!x || (kar = x[strlen(correctBaseForm)],kar && !isSpace(kar)))
                        // Check exact match (next character must be blank or '\0')
                        {
//                    putchar('B');
                        update(correctBaseForm,word,tag);
                        }
                    //break; // disregard other readings
                    }
                }
            
            }
        return true;
        }
#endif
#endif

    int flex::updateFlexRulesIfNeeded(char * dictBaseform,char * dictFlexform, char * dictType)
        {
//        char tmp[256];
        if(removeBogus(dictFlexform))
            {
            static int boguscnt = 0;
            boguscnt++;
            printf("%d Comma detected in flex form. Type: %s Baseform: %s Flexform: %s\n",boguscnt,dictType,dictBaseform,dictFlexform);
            }
//        removeCardinal(dictBaseform,strlen(dictBaseform) - 1);
        /*
		int i;
        for(i = 0;dictBaseform[i];++i)
            {
            if(dictBaseform[i] == ',')
                {
                dictBaseform[i] = '\0';
                break;
                }

//            tmp[i] = dictBaseform[i];
            }*/
//        tmp[i] = '\0';

//        strcpy(tmp,dictBaseform);
//                    putchar('D');
        AllToLowerISO(dictFlexform);
        AllToLowerISO(dictBaseform);// should be lower already, but some characters,
                        //notably Ü (U umlaut), are not converted to lower case
        update(dictBaseform,dictFlexform,dictType,false);
//        update(tmp,dictFlexform,dictType,false);
//                    putchar('E');
//        update(tmp,dictBaseform,dictType,false);
        return 2;
        }

    bool flex::readFromFile(FILE * fpflex,const char * flexFileName)
        {
        if(!fpflex)
            {
            return false;
            }
        int start;
        fread(&start,sizeof(int),1,fpflex);
        rewind(fpflex);
        if(start == 0)
            { // new style flexrules 20080218
            if(!readRules(fpflex,flexFileName))
                return false;
            }
        else
            {
            char s[256];

            while((fgets(s,256,fpflex)) != 0)
                {
                Flex.add(s);
                }
            }
        return true;
        }


static int cnt = 0;
bool  addrule(char * baseform,char * flexform,char * lextype)
    {
    cnt += Flex.updateFlexRulesIfNeeded(baseform,flexform,lextype);
    return true;
    }
/*
int flex::extractFlexPatternsFromTaggedText(FILE * fpdict,const char * format)
    {
    readLemmas(fpdict,format,addrule,true);
    return cnt;
    }
*/
void flex::makeFlexRules
        (FILE * fpdict
        ,FILE * fpflex
        ,bool nice
        ,const char * format
        ,int & failed
        ,long CutoffRefcount
        ,bool showRefcount
        )
    {
    int count;
    char name[256];
    flex::CutoffRefcount = CutoffRefcount;
    flex::showRefcount = showRefcount;
    training = true;
    for(int SRep = 0;true/*see test below*/;++SRep)
        {
        for(int Rep = 0;true;++Rep)
            {
            for(int rep = 0;true;++rep)
                {
                rewind(fpdict);
                resetRefCount();

//                extractFlexPatternsFromTaggedText(fpdict,format);
                readLemmas(fpdict,format,addrule,true,failed);
                if(nice)
                    {
                    count = Count();
                    printf("rep %d.%d.%d\n",SRep,Rep,rep);
                    //              printf("%d INPUTS TO FORM FLEXPATTERNS\n",cnt);
                    //                printf("%d SUM OF REFCOUNT\n",refcnt);
                    printf("%d COUNT\n\n",count);
                    }
                if(!changes())
                    {
                    break;
                    }
                }

#if 0
            // Unused alternatives are removed by removeNonFullWordsAsAlternatives!
            removeUnusedAlternatives();
            
            if(nice)
                {
                count = Count();
                printf("removeUnusedAlternatives\n");
                //              printf("%d SUM OF REFCOUNT\n",refcnt);
                printf("%d COUNT\n\n",count);
                }
#endif            
            removeNonFullWordsAsAlternatives();
            
            if(nice)
                {
                count = Count();
                printf("removeNonFullWordsAsAlternatives\n");
                //                printf("%d SUM OF REFCOUNT\n",refcnt);
                printf("%d COUNT\n\n",count);
                }
            if(!changes())
                {
                break;
                }
            }
        
        if(nice)
            {
            sprintf(name,"before%d.txt",SRep);
            FILE * fpflexTMP = fopen(name,"w");
            if(fpflexTMP)
                {
                write(fpflexTMP,nice);
                fclose(fpflexTMP);
                }
            }
        
        resetAdded();
        
        removeUnneededPatterns();
        
        
        if(nice)
            {
            count = Count();
            printf("removeUnneededPatterns\n");
            //    printf("%d SUM OF REFCOUNT\n",refcnt);
            printf("%d COUNT\n\n",count);
            
            sprintf(name,"after%d.txt",SRep);
            FILE *fpflexTMP = fopen(name,"w");
            if(fpflexTMP)
                {
                write(fpflexTMP,nice);
                fclose(fpflexTMP);
                }
            }
        
        if(!changes())
            {
            break;
            }
        resetAdded();
        }
    if(nice)
        {
        printf("Remove unused\n");
        }
    
    removeUnusedPatterns();
    write(fpflex,nice);
    }
