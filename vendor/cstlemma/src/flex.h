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
#ifndef FLEX_H
#define FLEX_H
#endif

#include <stdio.h>
#include "lext.h"

#define TEST 0
#define REFCNT 0 // 1: use base::refcnt as measure to decide which type 
                 // unknown words have. Assumption: frequencies of endings in
                 // dictionary are proportional to the frequencies of endings 
                 // in texts.
                 // 0: don't do this. The assumption is far from valid.

typedef enum {normal,/*notdeeper,*/onlydeeper} traverseTp;

class node;

class base
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        int m_n;
        char * m_baseform;
        base * m_next; //If next != 0, then this baseform is only one of 
                     // several that are derivable from the same ending
                     // (ambiguity, very unwelcome)
        int refcnt;
        bool m_fullWord; // if true: there exists at least one word having this 
        // baseform for which the ending extends over all of the word
        // example : baseform 'sindet' ending == word 'sindede'
        // If fullWord is true then this baseform object cannot be deleted. 
        // If fullWord is false, and if the same tail has more than one 
        // baseform, then the object can be deleted. In the next pass,
        // the tail will be extended to disambiguate between the baseforms.
        bool needed;
        bool added;
    public:
        static int nn;
        static int mutated;
#if TEST
        void test(char * text,char * save,char * buf,const char * type);
#endif
        base(char * baseform,bool fullWord,
#if REFCNT
            char * Refcnt,
#endif
            base * next = 0);
        ~base()
            {
            ++mutated;
            delete [] m_baseform;
            delete m_next;
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            }
        char * bf(){return m_baseform;}
        base * add(char * baseform,bool fullWord,base *& prev
#if REFCNT
            ,char * Refcnt
#endif
            );
        void print(int n);
        void write(FILE * fp,const char * type,const char * ending,int indent);
        void write(FILE * fp,const char * type,const char * ending);
        base * Next(){return m_next;}
        void resetRefCount();
        void resetAdded();
        void incRefCount(){++refcnt;}
        int RefCount(){return refcnt;}
        int sumRefCount();
        int Count();
        void removeNonFullWordsAsAlternatives();
        void removeUnusedPatterns(base *& prev/*,const char * type,char buf[]*/);
        bool removeUnneededPatterns(base *& prev,const char * type,char buf[],node * parent);
#if 0
        void removeUnusedAlternatives();// remove baseform objects with zero count from ambiguous sequences
#endif
        bool isFullWord(){return m_fullWord;}
        void setFullWord(){m_fullWord = true;}
        base * remove();
    };

class node
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
    public:
        static int mutated;
        node * m_next;
        char * m_tail;
        int m_len;
        base * basef;
        node * m_sub;
//        bool consolidated;
//        bool fixed; // set to true if node is in path to ambivalence
        bool marked;
    public:
        void unmark(){marked = false;if(m_next)m_next->unmark();}
        void mark(){marked = true;}
        node * removeAllMarked();
        base * Base(){return basef ? basef : m_sub ? m_sub->Base() : 0;}
        node(node * next,char * tail,char * baseform,bool fullWord,bool empty
#if REFCNT
            ,char * Refcnt
#endif
            );
        node(node * next,char * tail,int n,char * baseform,bool fullWord,bool empty,node * sub
#if REFCNT
            ,char * Refcnt
#endif
            );
        node(node * next,char * tail,int n,node * sub);
        ~node()
            {
            delete [] m_tail;
            delete m_next;
            delete m_sub;
            delete basef;
            ++mutated;
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            }
//        bool consolidate(node *& prev);
//        bool fix();
        node * Next(){return m_next;}
        int itsLen(){return m_len;}
        base * addsub(char * tail,int n,char * baseform,bool fullWord,node *& prev
#if REFCNT
            ,char * Refcnt
#endif
            );
        bool Baseform(char * invertedWord,base *& bf,int & ln);
        bool BaseformSub(char * word,base *& bf,int & ln);
        void print(int n);
        void write(FILE * fp,const char * type,int indent,char buf[]);
        void write(FILE * fp,const char * type,char buf[]);
        void removeUnusedPatterns(node *& prev,/*const char * type,char buf[],*/bool first);
        void removeUnneededPatterns(node *& prev,const char * type,char buf[],bool first,traverseTp how);
        void removeAmbiguous(node *& prev);// Remove all rules that have equally good competitors, e.g.
                                //  ADJ		[lille]små
                                //  ADJ		[liden]små

        base * add(char * tail,char * baseform,bool fullWord,node *& prev,bool empty
#if REFCNT
            ,char * Refcnt
#endif
            );
//        bool empty(){return *tail == '\0';}
        void cut(int c);
        void resetRefCount();
        void resetAdded();
        int sumRefCount();
        int Count();
        void removeNonFullWordsAsAlternatives();
#if 0
        void removeUnusedAlternatives(char * tailBuffer);// remove baseform objects with zero count for ambiguous sequences
#endif
        node * remove();
        bool remove(base * bf);
    };

class type
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        char * m_tp;
        node * end;
        type * m_next;
        bool fixed;
    public:
        base * Base(){return end ? end->Base():0;}
        static int mutated;
        type(const char * tp,char * tail,char * baseform,bool fullWord,
#if REFCNT
            char * Refcnt,
#endif
            type * next = 0);
        ~type()
            {
            delete [] m_tp;
            delete end;
            delete m_next;
            ++mutated;
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            }
//        bool consolidate();
//        void fix();
        base * add(const char * tp,char * tail,char * baseform,bool fullWord,type *& prev
#if REFCNT
            ,char * Refcnt
#endif
            );
        bool Baseform(char * invertedWord,const char * tag,base *& bf,int & ln);
        char * Baseform(char * invertedWord,base *& bf,int & ln);
        void print();
        void write(FILE * fp,bool nice);
        void resetRefCount();
        void resetAdded();
        int sumRefCount();
        int Count();
        void removeNonFullWordsAsAlternatives();
        void removeUnusedPatterns(type *& prev);
        void removeUnneededPatterns(type *& prev);
        void removeAmbiguous(type *& prev);
#if 0
        void removeUnusedAlternatives();// remove baseform objects with zero count from ambiguous sequences
#endif
        type * remove();
        void remove(base * bf);
    };

class flex
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        type * types;
        int notadded;
        bool consolidated;
    public:
        static bool baseformsAreLowercase;
        static long CutoffRefcount;
        static bool showRefcount;
        flex():types(0),consolidated(false)
            {
#ifdef COUNTOBJECTS
            ++COUNT;
#endif
            };
        ~flex();
//        void consolidate();
//        void fix();
        void trim(char * s);
        base * add(char * line);
        base * update(char * baseForm,char * word,const char * tag,bool partial);
        base * add(const char * tp,char * tail,char * baseform,bool fullWord);
        bool Baseform2(char * word,const char * tag,base *& bf,int & offset);
        bool Baseform(char * word,const char * tag,const char *& bf,int & borrow);
        char * Baseform(char * word,const char *& bf,int & borrow); // returns tag
        void print();
        void write(FILE * fp,bool nice);
/* Bart 20030910
This function is not used
        bool updateFlexRulesIfNeeded(lext * Plext,int nmbr,// The dictionary's available 
        // lexical information for this (tagged) word.
        char * word,char * tag);// word and tag as found in the text
*/
        int updateFlexRulesIfNeeded(char * dictBaseform,char * dictFlexform, char * dictType);
        void resetRefCount();
        void resetAdded();
        int sumRefCount();
        int Count();
        void removeNonFullWordsAsAlternatives();
        void removeUnusedPatterns();
        void removeUnneededPatterns();
#if 0
        void removeUnusedAlternatives();// remove baseform objects with zero count from ambiguous sequences
#endif
        void removeAmbiguous();
        void remove(base * bf);
//        int extractFlexPatternsFromTaggedText(FILE * fpdict,const char * format);
        void makeFlexRules
            (FILE * fpdict
            ,FILE * fpflex
            ,bool nice
            ,const char * format
            ,int & failed
            ,long CutoffRefcount
            ,bool showRefcount
            );
        bool readFromFile(FILE * fpflex,const char * flexFileName);
    };

extern flex Flex;
bool changes();
void unchanged();

/*

    1            ADJ  [øderativ]ederativ t 
   10            ADJ      [knyt]k n y t 
    1            ADJ      [ny] n y t 
    0            ADJ    [yt] y t                    <----- can be removed
  598            ADJ  [] t 


        |
        |
        V


    1            ADJ  [øderativ]ederativ t 
   10            ADJ      [knyt]k n y t 
    1            ADJ      [ny] n y t 
  598            ADJ  [] t 

Now, we would like to promote the second line
   10            ADJ      [knyt]k n y t 
by cutting off the 'k'

        |
        |
        V

    1            ADJ  [øderativ]ederativ t 
   10            ADJ      [nyt] n y t 
  598            ADJ  [] t 

but
    1            ADJ      [ny] n y t 

has no place to go, instead creating an extra baseform on the same ending

        |
        |
        V

    1            ADJ  [øderativ]ederativ t 
   10            ADJ      [nyt|ny] n y t 
  598            ADJ  [] t 


+++++++++++++++

     12            ADJ      [alid]a l i d           <----- remove
*    12            ADJ      [blid]b l i d           <----- remove
      1            ADJ        [goloid]g o l i d
     12            ADJ        [olid] o l i d        <----- remove
      0            ADJ      [loid] l i d            <----- replace stem by [lid], because alid, blid and olid are more frequent than loid and their stem+1 is equal
    227            ADJ    [id] i d 


      1            ADJ      [goloid]go l i d 
      x            ADJ      [lid] l i d 
    227            ADJ    [id] i d 




      1            ADJ        [goloid]g o l i d 
    227            ADJ    [id] i d 


*/

/*
cycle

read lemmas

 34500    *    12            ADJ       [bred]b r e d 
 31955    *    12            ADJ       [vred]v r e d 
 15091          2            ADJ     [rede]r e d 

shorten [rede]r e d to [ede]e d (unoccupied as yet)

 34500    *    12            ADJ       [bred]b r e d 
 31955    *    12            ADJ       [vred]v r e d 
 46525 +        2            ADJ   [ede]e d 

reread lemmas. fed, hed and led are exceptions to the now prominent [ede]e d rule

 48549 +  *    12            ADJ     [fed]f e d 
 48752 +  *    12            ADJ     [hed]h e d 
 49003 +  *    12            ADJ     [led]l e d 
 34500    *    12            ADJ       [bred]b r e d 
 31955    *    12            ADJ       [vred]v r e d 
 46525          2            ADJ   [ede]e d 

replace [ede]e d by the more general [ed]e d rule, derived from [fed]f e d. All other entries fall in.

 49499 +       56            ADJ   [ed]e d 

reread lemmas. [rede]r e d comes in as an exception to the [ed]e d rule, followed by [bred]b r e d and [vred]v r e d.

 51131 +  *    12            ADJ       [bred]b r e d 
 51093 +  *    12            ADJ       [vred]v r e d 
 50553 +        2            ADJ     [rede]r e d 
 49499         36            ADJ   [ed]e d 

remove the now unnecessary [ed]e d rule (incorporated by the [d]d rule).

 51131    *    12            ADJ       [bred]b r e d 
 51093    *    12            ADJ       [vred]v r e d 
 50553          2            ADJ     [rede]r e d 
*/
