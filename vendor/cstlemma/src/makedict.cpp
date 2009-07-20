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
#include "makedict.h"
#include "freqfile.h"
#include "lem.h"
#include "readlemm.h"
#include "readfreq.h"
#include "caseconv.h"
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <ctype.h>
//#define NDEBUG
#include <assert.h>

class DictNode;
class Lemma;
typedef int tchildrencount; // type for variables that are optimal for counting
                            // small numbers, but the value of which evantually
                            // will be typecasted to tchildren.

static char *** pstrings = NULL;
static char ** strings = NULL;
static DictNode ** pLeafs = NULL;
static char * STRINGS0;
static char * STRINGS; // STRINGS0 == STRINGS - 1
static Lemma * LEMMAS;
static tcount istrings = 0;
static char nul[] = "";

static unsigned long notypematch = 0;
static int g_added = 0;
static int notadded = 0;
static char * globflexform;
static char * globbf;
static unsigned long totcnt = 0L;
static unsigned long notypematchcnt = 0L;
static unsigned long addedcnt = 0L;
static unsigned long notaddedcnt = 0L;


#ifdef COUNTOBJECTS
int LemmaCOUNT = 0;
int DictNodeCOUNT = 0;
#endif

class Lemma
    {
    private:
        DictNode * m_parent;
        Lemma * next;
        char * Type;
        char * BaseForm;
        tsundry S;
        /*toffset Offset;
        tfrequency frequency;*/ // Bart 20020905
public:
    const char * getbaseform()
        {
        return BaseForm;
        }
    const char * type()
        {
        return Type;
        }
    int Offset()
        {
        return S.Offset;
        }
    int frequency()
        {
        return S.frequency;
        }
    void move(tcount pos)
        {
        LEMMAS[pos].Type = Type;
        LEMMAS[pos].BaseForm = BaseForm;
//        LEMMAS[pos].S.Offset = S.Offset;
        LEMMAS[pos].S = S;
        if(next)
            {
            LEMMAS[pos].next = LEMMAS + pos + 1;
            next->move(pos + 1);
            }
        }
    int cmp(const Lemma * lt)
        {
        int d = Type - lt->Type;
        if(d)
            return d;
        d = BaseForm - lt->BaseForm;
        if(d)
            return d;
        d = S.Offset - lt->S.Offset;
        if(d)
            return d;
        d = S.frequency - lt->S.frequency;
        if(d)
            return d;
        if(next)
            {
            assert(lt->next);
            return next->cmp(lt->next);
            }
        else
            return 0;
        }
    void setParent(DictNode * parent)
        {
        this->m_parent = parent;
        }
    Lemma()
        {
        Type = NULL;
        BaseForm = NULL;
        S.Offset = 0;
        next = NULL;
        m_parent = NULL;
        S.frequency = 0;
#ifdef COUNTOBJECTS
        ++LemmaCOUNT;
#endif
        }
    Lemma(const char * lextype,const char * baseform,int offset,DictNode * parent)
        {
        Type = new char[strlen(lextype)+1];
        strcpy(Type,lextype);
        BaseForm = new char[strlen(baseform)+1];
        strcpy(BaseForm,baseform);
        S.Offset = offset;
        next = NULL;
        this->m_parent = parent;
        S.frequency = 0;
#ifdef COUNTOBJECTS
        ++LemmaCOUNT;
#endif
        }
    Lemma(char * lextype,char * baseform,int offset)
        {
        Type = new char[strlen(lextype)+1];
        strcpy(Type,lextype);
        BaseForm = new char[strlen(baseform)+1];
        strcpy(BaseForm,baseform);
        S.Offset = offset;
        next = NULL;
        m_parent = NULL;
        S.frequency = 0;
#ifdef COUNTOBJECTS
        ++LemmaCOUNT;
#endif
        }
    ~Lemma()
        {
//        delete [] Type;
//        delete [] BaseForm;
//        delete next;
#ifdef COUNTOBJECTS
        --LemmaCOUNT;
#endif
        }
    tchildrencount count()
        {
        if(next)
            return 1 + next->count();
        else
            return 1;
        }
    void Lemmacnt(tcount * nLemmas)
        {
        ++*nLemmas;
        if(next)
            next->Lemmacnt(nLemmas);
        }
    tcount strcnt()
        {
        tcount ret = 0;
        if(*Type)
            ret = 1;
        if(*BaseForm)
            ret += 1;
        if(next)
            ret += next->strcnt();
        return ret;
        }
    void getStrings()
        {
        if(*Type)
            pstrings[istrings++] = &Type;
        else
            {
            delete [] Type;
            Type = nul;
            }
        if(*BaseForm)
            pstrings[istrings++] = &BaseForm;
        else
            {
            delete [] BaseForm;
            BaseForm = nul;
            }
        if(next)
            next->getStrings();
        }
    bool add(char * lextype,char * baseform,int offset)
        {
        if(  strcmp(lextype,Type) 
          || strcmp(baseform,BaseForm)
          || (unsigned int)offset != S.Offset
          )
            {
            if(next)
                return next->add(lextype,baseform,offset);
            else
                {
                next = new Lemma(lextype,baseform,offset);
                return true;
                }
            }
        return false;
        }
    bool addFreq(char * lextype,int n,char * bf,bool relaxed,unsigned int offset)
        {
        bool added = false;
        int off = offset - S.Offset;
        if(!strcmp(lextype,Type) && (!bf || /*S.Offset == offset*/ off >= 0))
            {
            if(relaxed) 
                {
                assert(bf);
                char * bfoff = bf + S.Offset;
                int len = strlen(bfoff);
                if(!strncmp(BaseForm,bfoff,len) && BaseForm[len] == ',')
                    {
                    S.frequency += n;
                    added = true;
                    //return true;
                    }
                }
            else
                {
                if(!bf || !strcmp(bf + S.Offset,BaseForm))
                    {
                    S.frequency += n;
                    added = true;
                    //return true;
                    }
                }
            }
        /*else*/ // Full form may have several (equal or different) base forms
                 // all having the same type!
        if(next)
            // TODO Bart 20060829. Shouldn't the next line be 
            // return next->addFreq(lextype,n,bf,relaxed,offset) || added;
            // ?
            // Bart 20061114 added || added
            return next->addFreq(lextype,n,bf,relaxed,offset) || added;
        else
            {
            if(added)
                {
                return true;
                }
            else
                {
                notypematch++;
                notypematchcnt += n;
                return false;
                }
            }
        }
    tcount printLemmas(tcount pos,DictNode * parent,FILE * fp);
    int print(FILE * fp)
        {
        fprintf(fp,"%d %s %s %d",S.Offset,BaseForm,Type,S.frequency);
        if(next)
            {
            fprintf(fp,",");
            return 1 + next->print(fp);
            }
        return 1;
        }
    void binprint(FILE * fp)
        {
        tindex tmp;
        if(Type == nul)
            tmp = 0;
        else
            tmp = Type - STRINGS0;
//        printf("Type %s tmp %d\n",Type,tmp);
        fwrite(&tmp,sizeof(tmp),1,fp);
        if(BaseForm == nul)
            tmp = 0;
        else
            tmp = BaseForm - STRINGS0;
//        printf("BaseForm %s tmp %d Offset %d\n",BaseForm,tmp,Offset);
        fwrite(&tmp,sizeof(tmp),1,fp);
        fwrite(&S,sizeof(S),1,fp);
        /*fwrite(&Offset,sizeof(Offset),1,fp);
        fwrite(&frequency,sizeof(frequency),1,fp);*/
        }
    };

typedef enum {casesensitive,caseinsensitive} Case;

class DictNode
    {
    private:
    static tcount iLeafs;

    DictNode * m_parent;
    DictNode * next;
    char * m_flexform;
/*
    class Bi
        {
        private:
            union
                {
                DictNode * sub; // if leaf == false
                Lemma * type;   // if leaf == true
                } u;
            bool leaf;
        public:
            bool isLeaf()
                {
                return leaf;
                }
            
        };
    Bi bi;
*/
    union
        {
        DictNode * sub; // if leaf == false
        Lemma * type;   // if leaf == true
        } u;
    bool leaf;
    tchildrencount m_n; // number of subnodes or types (max 256)
public:
    void copy(DictNode * nd)
        {
        assert(leaf);
        assert(nd->leaf);
        delete u.type;
        u.type = nd->u.type;
        }
    tcount moveLemma(tcount pos)
        {
        assert(leaf);
        u.type->move(pos);
        delete u.type;
        u.type = LEMMAS + pos;
        return pos + m_n;
        }
    int cmp(const DictNode * n2) const
        {
        assert(leaf);
        int d = m_n - n2->m_n;
        if(d)
            return d;
        return u.type->cmp(n2->u.type);
        }
    int numberOfLextypes()
        {
        assert(leaf);
        return m_n;
        }
    tchildrencount count()
        {
        if(leaf)
            m_n = u.type->count();
        else
            m_n = u.sub->count();
        if(next)
            return 1 + next->count();
        else
            return 1;
        }
    tcount LeafCount(tcount * nLemmas)
        {
        tcount ret;
        if(leaf)
            {
            ret = 1;
            u.type->Lemmacnt(nLemmas);
            }
        else
            ret = u.sub->LeafCount(nLemmas);

        if(next)
            ret += next->LeafCount(nLemmas);
        return ret;
        }
    tcount strcnt()
        {
        tcount ret;
        if(*m_flexform)
            ret = 1;
        else
            ret = 0;
        if(leaf)
            ret += u.type->strcnt();
        else
            ret += u.sub->strcnt();
        if(next)
            ret += next->strcnt();
        return ret;
//        return (leaf ? u.type->strcnt() : u.sub->strcnt()) + (next ? next->strcnt() + 1 : 1);
        }
    void getLemmas()
        {
        if(leaf)
            pLeafs[iLeafs++] = this;
        else
            u.sub->getLemmas();
        if(next)
            next->getLemmas();
        }
    void getStrings()
        {
        if(*m_flexform)
            pstrings[istrings++] = &m_flexform;
        else
            {
            delete [] m_flexform;
            m_flexform = nul;
            }
        if(leaf)
            u.type->getStrings();
        else
            u.sub->getStrings();
        if(next)
            next->getStrings();
        }
    void init(const char * flexform,DictNode * parent)
        {
        this->m_flexform = new char[strlen(flexform)+1];
        strcpy(this->m_flexform,flexform);
        next = NULL;
        this->m_parent = parent;
        }
    DictNode(const char * flexform,const char * lextype,const char * baseform,int offset,DictNode * parent = NULL)
        {
        init(flexform,parent);
        leaf = true;
        u.type = new Lemma(lextype,baseform,offset,this);
#ifdef COUNTOBJECTS
        ++DictNodeCOUNT;
#endif
        }
    DictNode(const char * flexform,Lemma * lextype,DictNode * parent = NULL)
        {
        init(flexform,parent);
        leaf = true;
        u.type = lextype;
        u.type->setParent(this);
#ifdef COUNTOBJECTS
        ++DictNodeCOUNT;
#endif
        }
    DictNode(const char * flexform,DictNode * subnode,DictNode * parent = NULL)
        {
        init(flexform,parent);
        leaf = false;
        u.sub = subnode;
        u.sub->m_parent = this;
#ifdef COUNTOBJECTS
        ++DictNodeCOUNT;
#endif
        }
    ~DictNode()
        {
//        delete [] flexform;
        if(leaf)
            {
//            delete u.type;
            }
        else
            delete u.sub;
        delete next;
#ifdef COUNTOBJECTS
        --DictNodeCOUNT;
#endif
        }
    void rename(int cut)
        {
        char * newstrng = new char[strlen(m_flexform + cut)+1];
        strcpy(newstrng,m_flexform + cut);
        delete [] m_flexform;
        m_flexform = newstrng;
        }
    bool add(char * flexform,char * lextype,char * baseform,int offset);
    bool addFreq(char * flexform,char * lextype,int n,char * bf,unsigned int offset,Case cse);
    tcount printLemmas(tcount pos,FILE * fp)
        {
        if(leaf)
            {
            pos = u.type->printLemmas(pos,this,fp);
            fprintf(fp,"\n");
            }
        else
            {
            pos = u.sub->printLemmas(pos,fp);
            }
        if(next)
            return next->printLemmas(pos,fp);
        else
            return pos;
        }

    tcount BreadthFirst_position(tcount Pos,tchildrencount length);

    void BreadthFirst_print(int indent,tchildrencount length,FILE * fp);
    void BreadthFirst_print(int indent,tchildrencount length,FILE * fp,char * wrd);
    void BreadthFirst_printBin(FILE * fp);
    void print(int indent,FILE * fp);
    void print(int indent,FILE * fp,char * wrd);
    int printLeaf()
        {
        assert(leaf);
        printf("%s",m_flexform);
//        printf("[%ld]",n);
        printf("[%d]",m_n);
        printf("(");
        int ret = u.type->print(stdout);
        printf(")\n");
        return ret;
        }
    };

bool DictNode::add(char * flexform,char * lextype,char * baseform,int offset)
    {
    int i;
    for(i = 0;flexform[i] && this->m_flexform[i] && flexform[i] == this->m_flexform[i];++i)
        ;
    if(i > 0)
        { // (partial) overlap
        if(!this->m_flexform[i])
            { // new string incorporates this string
            if(flexform[i])
                { // new string longer than this string
                if(leaf)
                    { // make this DictNode a non-terminal DictNode and chain a new node after the first subnode
                    Lemma * tmptype = u.type;
                    leaf = false;
                    u.sub = new DictNode("",tmptype,this); // The first subnode is a leaf containing this node's previously owned Lemma
                    u.sub->next = new DictNode(flexform + i,lextype,baseform,offset); // The next subnode is the new leaf
                    return true;
                    }
                else
                    {
                    if((unsigned char)flexform[i] < (unsigned char)u.sub->m_flexform[0])
                        { // 
                        DictNode * tmpnode = u.sub;
                        u.sub = new DictNode(flexform + i,lextype,baseform,offset,this); // The new leaf becomes the first in the sub-sequence
                        u.sub->next = tmpnode;
                        u.sub->next->m_parent = NULL;
                        return true;
                        }
                    else
                        return u.sub->add(flexform + i,lextype,baseform,offset); // The new leaf comes somewhere after the first node in the sub-sequence
                    }
                }
            else if(!leaf)
                { // new string equal to this string, but this string is not a leaf
                if(u.sub->m_flexform[0])
                    { // create leaf with empty string at start of sub-sequence
                    DictNode * tmpnode = u.sub;
                    u.sub = new DictNode("",lextype,baseform,offset,this);
                    u.sub->next = tmpnode;
                    u.sub->next->m_parent = NULL;
                    return true;
                    }
                else
                    { // The first node in the sub-sequence has a flexform that matches the new flexform
                    return u.sub->add(flexform + i,lextype,baseform,offset);
                    }
                }
            else // only difference between new string and this string is (possibly) lextype
                {
                return u.type->add(lextype,baseform,offset);
                // add new lextype (if not equal to existing type)
                }
            }
        else 
            {
            if(!flexform[i])
                { // new string is incorporated by this string. This string is longer
                  // The new node must become the first in the sub-sequence
                if(leaf)
                    {
                    Lemma * tmptype = u.type;
                    leaf = false;
                    u.sub = new DictNode("",lextype,baseform,offset,this);
                    u.sub->next = new DictNode(this->m_flexform + i,tmptype);
                    }
                else
                    {
                    DictNode * tmpsub = u.sub;
                    u.sub = new DictNode("",lextype,baseform,offset,this);
                    u.sub->next = new DictNode(this->m_flexform + i,tmpsub);
                    }
                }
            else
                {
                if((unsigned char)flexform[i] > (unsigned char)this->m_flexform[i])
                    { 
                    // The new node must come somewhere after the first node in the sub-sequence
                    if(leaf)
                        { // make this DictNode a non-terminal DictNode
                        Lemma * tmptype = u.type;
                        leaf = false;
                        u.sub = new DictNode(this->m_flexform + i,tmptype,this);
                        }
                    else
                        { // Insert a new level of nodes. Push the current sub-sequence one level down
                        DictNode * tmpsub = u.sub;
                        u.sub = new DictNode(this->m_flexform + i,tmpsub,this); 
                        }
                    // The new flexnode comes straight after the first node in the (new) subsequence
                    u.sub->next = new DictNode(flexform + i,lextype,baseform,offset);
                    }
                else
                    { // The new node is alphabetically before this node
                    if(leaf)
                        { // make this DictNode a non-terminal DictNode
                        Lemma * tmptype = u.type;
                        leaf = false;
                        u.sub = new DictNode(flexform + i,lextype,baseform,offset,this);
                        u.sub->next = new DictNode(this->m_flexform + i,tmptype);
                        }
                    else
                        {
                        DictNode * tmpsub = u.sub;
                        u.sub = new DictNode(flexform + i,lextype,baseform,offset,this);
                        u.sub->next = new DictNode(this->m_flexform + i,tmpsub); 
                        }
                    }
                }
            this->m_flexform[i] = '\0'; // shorten the flexform to the common string
            char * newstrng = new char[strlen(this->m_flexform)+1];
            strcpy(newstrng,this->m_flexform);
            delete [] this->m_flexform;
            this->m_flexform = newstrng;
            return true;
            }
        }
    else if(next)
        {
        if((unsigned char)next->m_flexform[0] > (unsigned char)flexform[0])
            { // The new node is alphabetically before the next node
            if((unsigned char)this->m_flexform[0] < (unsigned char)flexform[0])
                {// The new node is alphabetically after this node
                DictNode * tmpnode = next;
                next = new DictNode(flexform,lextype,baseform,offset);
                next->next = tmpnode;
                return true;
                }
            else
                return u.type->add(lextype,baseform,offset);
            }
        else // The new node is alphabetically after the next node
            return next->add(flexform,lextype,baseform,offset);
        }
    else
        {
        if(  !leaf 
          || strcmp(u.type->getbaseform(),baseform) 
          || strcmp(u.type->type(),lextype) 
          || strcmp(this->m_flexform,flexform) 
          || offset != u.type->Offset()
          )
            {
            next = new DictNode(flexform,lextype,baseform,offset);
            return true;
            }
        else
            {
//            printf("(%s:%s)(%s:%s)(%s:%s)(%d:%d)\n",this->m_flexform,flexform,u.type->baseform(),baseform,u.type->type(),lextype,offset,u.type->offset());
            return false;
            }
        }
//    return false;
    }

bool DictNode::addFreq(char * flexform,char * lextype,int n,char * bf,unsigned int offset,Case cse)
    {
    int i;
    if(cse == casesensitive)
        for(i = 0;flexform[i] && this->m_flexform[i] && flexform[i] == this->m_flexform[i];++i)
            ;
    else
        for(i = 0;flexform[i] && this->m_flexform[i] && Lower(flexform[i]) == Lower(this->m_flexform[i]);++i)
            ;
    if(i > 0 || flexform[i] == this->m_flexform[i])
        { // (partial) overlap
        if(!this->m_flexform[i])
            { // looked-for string incorporates this string
            if(flexform[i])
                { // new string longer than this string
                if(leaf)
                    {
                    return false; // Nothing to do; word isn't in dictionary.
                    }
                else
                    {
                    if((unsigned char)flexform[i] < (unsigned char)u.sub->m_flexform[0])
                        {
                        return false; // Nothing to do; word isn't in dictionary.
                        }
                    else
                        {
                        return u.sub->addFreq(flexform + i,lextype,n,bf,offset,cse);
                        }
                    }
                }
            else if(!leaf)
                { // new string equal to this string, but this string is not a leaf
                        return u.sub->addFreq(flexform + i,lextype,n,bf,offset,cse);
//                return false; // Nothing to do; word isn't in dictionary.
                }
            else // only difference between new string and this string is (possibly) lextype
                {
                if(u.type->addFreq(lextype,n,bf,false,offset))
                    return true;
                if(bf == NULL)
                    return false;
                return u.type->addFreq(lextype,n,bf,true,offset);
                // add frequency to one of the lemmas
                }
            }
        else 
            {
            return false; // Nothing to do; word isn't in dictionary.
            }
        }
    else if(next && (unsigned char)flexform[0] > (unsigned char)this->m_flexform[0])
        {
        return next->addFreq(flexform,lextype,n,bf,offset,cse);
        }
    return false; // Nothing to do; word isn't in dictionary.
    }

        
tcount DictNode::BreadthFirst_position(tcount Pos,tchildrencount length)
    {
    // Unnecessary complex function. Previously used to compute index of each
    // DictNode in array. Now only used to return the size of the array.
    tcount subPos;
    subPos = Pos + length;
    for(DictNode * nxt = this;nxt;nxt = nxt->next)
        {
        if(!nxt->leaf)
            subPos = nxt->u.sub->BreadthFirst_position(subPos,nxt->m_n);
        }
    return subPos;
    }

void DictNode::BreadthFirst_print(int indent,tchildrencount length,FILE * fp,char * wrd)
    {
    DictNode * nxt;
    fprintf(fp,"%d\n",(int)length);
    int len = strlen(wrd);
    for(nxt = this;nxt;nxt = nxt->next)
        {
        for(int i = indent;i;--i)
            fputc(' ',fp);
        fprintf(fp,"%s%s",wrd,nxt->m_flexform);
//        fprintf(fp,"[%ld]",nxt->n);
        fprintf(fp,"[%d]",nxt->m_n);
        if(nxt->leaf)
            {
            fprintf(fp,"(");
            nxt->u.type->print(fp);
            fprintf(fp,")");
            }
        fprintf(fp,"\n");
        }
    for(nxt = this;nxt;nxt = nxt->next)
        {
        if(!nxt->leaf)
            {
            strcpy(wrd+len,nxt->m_flexform);
            nxt->u.sub->BreadthFirst_print(indent + strlen(nxt->m_flexform),nxt->m_n,fp,wrd);
            }
        }
    wrd[len] = '\0';
    }

void DictNode::BreadthFirst_print(int indent,tchildrencount length,FILE * fp)
    {
    DictNode * nxt;
    fprintf(fp,"%d\n",(int)length);
    for(nxt = this;nxt;nxt = nxt->next)
        {
        for(int i = indent;i;--i)
            fputc(' ',fp);
        fprintf(fp,"%s",nxt->m_flexform);
//        fprintf(fp,"[%ld]",nxt->n);
        fprintf(fp,"[%d]",nxt->m_n);
        if(nxt->leaf)
            {
            fprintf(fp,"(");
            nxt->u.type->print(fp);
            fprintf(fp,")");
            }
        fprintf(fp,"\n");
        }
    for(nxt = this;nxt;nxt = nxt->next)
        {
        if(!nxt->leaf)
            {
            nxt->u.sub->BreadthFirst_print(indent + strlen(nxt->m_flexform),nxt->m_n,fp);
            }
        }
    }

void DictNode::BreadthFirst_printBin(FILE * fp)
    {
    DictNode * nxt;
    for(nxt = this;nxt;nxt = nxt->next)
        {
        tindex tmp;
        if(nxt->m_flexform == nul)
            tmp = 0;
        else
            tmp = nxt->m_flexform - STRINGS0;
//        printf("tmp %d nxt->n %d\n",tmp,nxt->n);
        fwrite(&tmp,sizeof(tmp),1,fp);
        tchildren nxtn = tchildren(nxt->m_n);
        fwrite(&nxtn,sizeof(nxtn),1,fp);
        if(nxt->leaf)
            tmp = nxt->u.type - LEMMAS;
        else
            tmp = -1; // index to first child DictNode. reconstructed upon reading
//        printf("tmp %d\n",tmp);
        fwrite(&tmp,sizeof(tmp),1,fp);
        }
    for(nxt = this;nxt;nxt = nxt->next)
        {
        if(!nxt->leaf)
            {
            nxt->u.sub->BreadthFirst_printBin(fp);
            }
        }
    }

void DictNode::print(int indent,FILE * fp,char * wrd)
    {
    int len = strlen(wrd);
    for(int i = indent;i;--i)
        fputc(' ',fp);
    fprintf(fp,"%s|%s",wrd,m_flexform);
//    fprintf(fp,"[%ld]",m_n);
    fprintf(fp,"[%d]",m_n);
    if(leaf)
        {
        fprintf(fp,"(");
        u.type->print(fp);
        fprintf(fp,")\n");
        }
    else
        {
        fprintf(fp,"\n");
        strcpy(wrd+len,m_flexform);
        u.sub->print(indent + strlen(m_flexform),fp,wrd);
        wrd[len] = '\0';
        }
    if(next)
        next->print(indent,fp,wrd);
    }

void DictNode::print(int indent,FILE * fp)
    {
    for(int i = indent;i;--i)
        fputc(' ',fp);
    fprintf(fp,"%s",m_flexform);
//    fprintf(fp,"[%ld]",m_n);
    fprintf(fp,"[%d]",m_n);
    if(leaf)
        {
        fprintf(fp,"(");
        u.type->print(fp);
        fprintf(fp,")\n");
        }
    else
        {
        fprintf(fp,"\n");
        u.sub->print(indent + strlen(m_flexform),fp);
        }
    if(next)
        next->print(indent,fp);
    }

tcount DictNode::iLeafs = 0;

tcount Lemma::printLemmas(tcount pos,DictNode * parent,FILE * fp)
    {
    if(parent)
        {
///        parent->setType(pos);
        fprintf(fp,"%ld: ",pos);
        }
    fprintf(fp,"%d %s %s %d",S.Offset,BaseForm,Type,S.frequency);
    if(next)
        {
        fprintf(fp,",");
        return next->printLemmas(pos + 1,NULL,fp);
        }
    else
        return pos + 1;
    }

static DictNode * root;

static bool add(char * baseform,char * flexform,char * lextype)
    {
    int i;
    if(removeBogus(flexform))
        {
        static int boguscnt = 0;
        boguscnt++;
        printf("%d Comma detected in flex form. Type: %s Baseform: %s Flexform: %s\n",boguscnt,lextype,baseform,flexform);
        }
    AllToLowerISO(baseform);// should be lower already, but some characters,
                      // notably Ü (U umlaut), are not converted to lower case
    for(i = 0;baseform[i] && flexform[i] == baseform[i];++i)
        ;
    if(root->add(flexform,lextype,baseform+i,i))
        return true;
    else
        {
//        printf("%s %s %s\n",baseform,flexform,lextype);
  //      root->add(flexform,lextype,baseform+i,i);
        return false;
        }
    }

static void addFreq(int n,char * flexform,char * lextype,char * bf)
    {
    //AllToLower(flexform);
    globflexform = flexform;
    globbf = bf;
    totcnt += n;
    int i = 0;
    if(bf)
        {
        AllToLowerISO(bf);// should be lower already, but some characters,
                          // notably Ü (U umlaut), are not converted to lower case
        for(i = 0;bf[i] && flexform[i] == bf[i];++i)
            ;
        }
    if(root->addFreq(flexform,lextype,n,bf ? bf/*+i*/ : NULL,i,casesensitive))
        {
//        printf("+++%s/%s %d %s\n",flexform,lextype,n,bf);
        g_added++;
        addedcnt += n;
        }
    else
        {
        if(bf)
            {
            for(i = 0;bf[i] && Lower(flexform[i]) == (unsigned int)(unsigned char)bf[i];++i)
                ;
            }
        if(root->addFreq(flexform,lextype,n,bf ? bf/*+i*/ : NULL,i,caseinsensitive))
            {
    //        printf("+++%s/%s %d %s\n",flexform,lextype,n,bf);
            g_added++;
            addedcnt += n;
            }
        else
            {
    //        printf("---%s/%s %d %s\n",flexform,lextype,n,bf);
            notadded++;
            notaddedcnt += n;
            }
        }
    }


static int compare(const void * arg1, const void * arg2)
    {
    return strcmp(**(const char * const * const * )arg1, **(const char * const * const * )arg2);
    }

static tlength compressStrings(tcount nstrings,tcount * nUniqueStrings)
    {
    pstrings = new char ** [nstrings];
    root->getStrings();
    printf("sorting %ld strings...",nstrings);
    fflush(stdout);
    qsort( (void *)pstrings, nstrings, sizeof( char * ), compare );
    printf("done\n");
    tcount i,j,k;
    tlength len = strlen(*pstrings[0]) + 1;
    for(i = 0, j = 1;j < nstrings;++j)
        {
        if(strcmp(*pstrings[i],*pstrings[j]))
            {
            i = j;
            len += strlen(*pstrings[i]) + 1;
            }
        }
//    printf("len %ld j %ld\n",len,j);
    STRINGS = new char[len];
    STRINGS0 = STRINGS - 1;
    tlength pos;
    strcpy(STRINGS,*pstrings[0]);
//    printf("0 %ld <%s>\n",pos,STRINGS+pos);
    delete [] *pstrings[0];
    *pstrings[0] = STRINGS;
    pos = strlen(*pstrings[0])+1;
//    printf("%ld %s ",0,*pstrings[0]);
    printf("compacting strings...");
    fflush(stdout);
    for(i = 0, j = 1,k = 1;j < nstrings;++j)
        {
//        printf("%ld %s ",j,*pstrings[j]);
//        printf("i %ld j %ld %s %s\n",i,j,*pstrings[i],*pstrings[j]);
        if(strcmp(*pstrings[i],*pstrings[j]))
            {
            i = j;
            strcpy(STRINGS+pos,*pstrings[i]);
//            printf("%ld %ld <%s>\n",k,pos,STRINGS+pos);
            delete [] *pstrings[i];
            *pstrings[i] = STRINGS+pos;
            pos += strlen(*pstrings[i])+1;
            pstrings[k++] = pstrings[j];
            }
        else
            {
            delete [] *pstrings[j];
            *pstrings[j] = *pstrings[i];
//            pstrings[j] = NULL;
            }
        }
    if(k < nstrings)
        pstrings[k] = NULL;
    strings = new char * [k];
    for(i = 0;i < k;++i)
        strings[i] = *pstrings[i];
    delete [] pstrings;
    pstrings = NULL;
    /*
    j = 0;
    */
    *nUniqueStrings = k;
    printf("resulting to %ld strings\n",k);
    return len;
    }

static int compareLeaf(const void *arg1, const void * arg2)
    {
    const DictNode * n1 = *(const DictNode * const *)arg1;
    const DictNode * n2 = *(const DictNode * const *)arg2;
    return n1->cmp(n2);
    }

static tcount compressLeafs(tcount nLeaf,tcount * nUniqueLemmas)
    {
    pLeafs = new DictNode * [nLeaf];
    root->getLemmas();
    tcount i,j,k;
    printf("sorting %ld leafs...",nLeaf);
    fflush(stdout);
    qsort( (void *)pLeafs, nLeaf, sizeof( DictNode * ), compareLeaf );
    printf("done\n");
    tcount len = pLeafs[0]->numberOfLextypes();
    for(i = 0, j = 1;j < nLeaf;++j)
        {
        if(pLeafs[i]->cmp(pLeafs[j]))
            {
            i = j;
            len += pLeafs[i]->numberOfLextypes();
            }
        }
//    printf("LEMMAS len %ld j %ld\n",len,j);
    LEMMAS = new Lemma[len];
    tcount pos = 0;
    pos = pLeafs[0]->moveLemma(pos);
//    printf("%ld ",0);pLeafs[0]->printLeaf();
    printf("compacting leafs...");
    fflush(stdout);
    for(i = 0, j = 1,k = 1;j < nLeaf;++j)
        {
//        printf("%ld ",j);pLeafs[j]->printLeaf();
        if(pLeafs[i]->cmp(pLeafs[j]))
            {
            i = j;
            pos = pLeafs[i]->moveLemma(pos);
            pLeafs[k++] = pLeafs[j];
//            printf("            %ld ",k-1);pLeafs[k-1]->printLeaf();
            }
        else
            {
            pLeafs[j]->copy(pLeafs[i]);
            }
        }
    if(k < nLeaf)
        pLeafs[k++] = NULL;
    /*
    j = 0;
    */
    *nUniqueLemmas = k;
    printf("resulting to %ld leafs\n",k);
    return len;
    }
    
int makedict(FILE * fpin,FILE * fpout,bool nice,const char * format,const FreqFile * freq,bool CollapseHomographs)
    {
    root = new DictNode("","","",0);
    printf("reading lemmas\n");
    int failed;
    int cnt = readLemmas(fpin,format,add,CollapseHomographs,failed);
    printf("%d lemmas read, %d discarded\n",cnt,failed);
    if(failed)
        printf("(see file \"discarded\")\n");

    while(freq)
        {
        if(!freq->itsName())
            {
            printf("No file name matching format %s\n",freq->itsFormat());
            break;
            }
        if(!freq->itsFormat())
            {
            printf("No format matching file name %s\n",freq->itsName());
            break;
            }

        FILE * ffreq = fopen(freq->itsName(),"r");
        if(ffreq)
            {
            printf("reading frequencies from %s with format %s\n",freq->itsName(),freq->itsFormat());
            readFrequencies(ffreq,freq->itsFormat(),addFreq);
            }
        else
            printf("*** CANNOT OPEN %s\n",freq->itsName());
        freq = freq->Next();
        }
    printf("counting children\n");
    tchildrencount nroot = root->count();
//    printf("nroot %ld\n",nroot);
//    root->print(0,stdout);
//    char woord[1000];

    printf("counting strings\n");
    tcount nstrings = root->strcnt();
    tcount nUniqueStrings = 0;
    printf("compressing strings\n");
    tlength stringBufferLen = compressStrings(nstrings,&nUniqueStrings);
    tcount nLemmas = -1; // compensate for root
    printf("counting leafs\n");
    tcount nLeaf = root->LeafCount(&nLemmas);


    tcount nUniqueLemmas = 0;
    printf("compressing leafs\n");
    tcount LemmaBufferLen = compressLeafs(nLeaf,&nUniqueLemmas);
//    printf("LemmaBufferLen %ld\n",LemmaBufferLen);
//    root->print(0,stdout);
/*
    woord[0] = '\0';
    FILE * fpt = fopen("root.txt","w");
    root->print(0,fpt,woord);
    fclose(fpt);
*/
    printf("writing strings\n");
    tcount i;
    if(nice)
        {
        fprintf(fpout,"%ld\n",(long)stringBufferLen);
        for(i = 0;i < nUniqueStrings;++i)
            {
//            fprintf(fpout,"%ld %ld %s\n",i,strings[i] - STRINGS,strings[i]);
            fprintf(fpout,"%ld %d %s\n",i,strings[i] - STRINGS,strings[i]);
            }
        }
    /*
    else if(portable)
        {
        //    printf("stringBufferLen %d\n",stringBufferLen);
        fprintf(fpout,"%d\n",stringBufferLen);
        fwrite(STRINGS,stringBufferLen,1,fpout);
        fprintf(fpout,"\n");
        }*/
    else
        {
        //    printf("stringBufferLen %d\n",stringBufferLen);
        fwrite(&stringBufferLen,sizeof(stringBufferLen),1,fpout);
        fwrite(STRINGS,stringBufferLen,1,fpout);
        }
    printf("writing lemmas\n");
//    printf("nUniqueLemmas %ld\n",nUniqueLemmas);
    if(nice)
        {
        fprintf(fpout,"%ld\n",LemmaBufferLen);
        for(i = 0;i < nUniqueLemmas;++i)
            {
            fprintf(fpout,"%ld ",i);
            LEMMAS[i].print(fpout);
            fprintf(fpout,"\n");
            }
        }
    /*
    else if(portable)
        {
        //    printf("stringBufferLen %d\n",stringBufferLen);
        fprintf(fpout,"%d\n",LemmaBufferLen);
        for(i = 0;i < LemmaBufferLen;++i)
            LEMMAS[i].portableprint(fpout);
        }
        */
    else
        {
        //    printf("LemmaBufferLen %d\n",LemmaBufferLen);
        fwrite(&LemmaBufferLen,sizeof(LemmaBufferLen),1,fpout);
        for(i = 0;i < LemmaBufferLen;++i)
            LEMMAS[i].binprint(fpout);
        }
    printf("strings: %lu unique: %lu\n",nstrings,nUniqueStrings);
    printf("flexforms: %lu lemmas: %lu unique: %lu\n",nLeaf,nLemmas,nUniqueLemmas);
    tcount nnodes = root->BreadthFirst_position(0,nroot);
    printf("writing nodes\n");
    if(nice)
        {
        fprintf(fpout,"nodes %ld\n",nnodes);
        root->BreadthFirst_print(0,nroot,fpout);
        /*
        woord[0] = '\0';
        root->BreadthFirst_print(0,nroot,fpout,woord);
        */
        }
    else
        {
        //    printf("nnodes %d nroot %d\n",nnodes,nroot);
        fwrite(&nnodes,sizeof(nnodes),1,fpout);
        tchildren nrootwrite = (tchildren)nroot;
        fwrite(&nrootwrite,sizeof(nrootwrite),1,fpout);
        root->BreadthFirst_printBin(fpout);
        }
//    root->print(0,fpout);
    delete root;
    delete [] strings;
    delete [] STRINGS;
    delete [] LEMMAS;

/*
    fclose(fpin);
    fclose(fpout);
*/
    if(totcnt > 0)
        {
        printf("frequencies added from %d words (%f%% of reference text)\n",g_added,(double)addedcnt*100.0/(double)totcnt);
        printf("frequencies from %ld words are not added because they weren't found in the dictionary (%f%% of reference text)\n",notadded - notypematch,(double)notaddedcnt*100.0/(double)totcnt);
        printf("frequencies from %ld words are not added because the types didn't agree. (%f%% of reference text)\n",notypematch,(double)notypematchcnt*100.0/(double)totcnt);
        }
    return 0;
    }

/*
int main(int argc, char * argv[])
    {
    FILE * fpin;
    FILE * fpout;
    if(!readArgs(argc,argv,&fpin,&fpout))
        return -1;
    return makedict(fpin,fpout);
    }
*/

