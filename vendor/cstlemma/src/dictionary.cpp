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
#include "dictionary.h"
#include "caseconv.h"
#include <string.h>

#ifdef COUNTOBJECTS
int dictionary::COUNT = 0;
#endif


typedef int tchildrencount; // type for variables that are optimal for counting
                            // small numbers, but the value of which eventually
                            // will be typecasted to tchildren.

struct Nodes
    {
    tcount nnodes; // number of nodes = number of elements in initialchars, 
                   // strings and children
    tchildren ntoplevel; // number of nodes at top level (for other levels, 
                         // this number is given by numberOfChildren)
    char * initialchars; // (optimization) string with all first characters.
                         // First ntoplevel characters for each of the 
                         // ntoplevel nodes. May contain zero bytes!
                         // If first character of candidate string isn't in
                         // stretch of initialchars, then the candidate string
                         // cannot be matched.
                         // If first character of candidate string is in 
                         // stretch of initialchars, then the candidate string
                         // must be compared (strcmp) with precisely one 
                         // string.
                         // Position of string to compare with can be computed
                         // from position of character in initialchars 
    char ** strings; // array of strings. First ntoplevel strings are for 
                     // each of the ntoplevel nodes.
                     // Full forms are encoded by stringing together the 
                     // appropriate *strings needed to reach the final 
                     // 'lext' structure.
    tchildren * numberOfChildren; // If node is a leaf, numberOfChildren 
                                  // denotes number of consecutive elements in
                                  // LEXT, otherwise its meaning is analogous
                                  // to ntoplevel
    tindex * pos; // zero or positive: index into lext array (LEXT)
                  // negative: inverse of index into initialchars, strings,
                  // numberOfChildren and pos
    };

static char * STRINGS;
static char * STRINGS1; // STRINGS1 = STRINGS + 1
lext * LEXT;
static Nodes NODES;
static char EMPTY[] = "";

void dictionary::initdict(FILE * fpin)
    {
    if(fpin)
        {
        readStrings(fpin);
        readLeaves(fpin);
        readNodes(fpin);
        }
    }

dictionary::dictionary()
    {
    NODES.ntoplevel = 0;
#ifdef COUNTOBJECTS
    ++COUNT = 0;
#endif
    }

dictionary::~dictionary()
    {
    cleanup();
#ifdef COUNTOBJECTS
    --COUNT = 0;
#endif
    }

void dictionary::printall(FILE * fp)
    {
    for(tchildrencount i = 0;i < NODES.ntoplevel;++i)
        {
        printnode(0,i,fp);
        }
    }

void dictionary::printall2(FILE * fp)
    {
    for(tchildrencount i = 0;i < NODES.ntoplevel;++i)
        {
        printnode2(EMPTY,i,fp);
        }
    }

bool dictionary::findword(const char * word,tcount & Pos,int & Nmbr)
    {
    if(findwordSub(word,Pos,Nmbr))
        return true;
    else if(is_Upper(word))
        return findwordSub(allToLower(word),Pos,Nmbr);
    else
        return false;
    }

bool dictionary::findwordSub(const char * word,tcount & Pos,int & Nmbr)
    {
    int kar = 0xFF & *word;
    const char * w = word;
    int nmbr = NODES.ntoplevel;
//    int i = 0;
    tcount pos = 0;
    while(nmbr > 0)
        {
        int kar2 = 0xFF & NODES.initialchars[pos];
        if(kar2 < kar)
            {
            pos++;
            nmbr--;
            }
        else if(kar2 == kar)
            {
            if(kar)
                {
                char * s = NODES.strings[pos];
                while(*++s && *s == *++w)
                    ;
                if(*s)
                    {
                    return false;
                    }
                }
            nmbr = NODES.numberOfChildren[pos];
            pos = NODES.pos[pos];
            if(pos < 0) // not a leaf, descend further
                {
                pos = -pos; // Make it a valid index.
                kar = 0xFF & *++w; // Next initial character to find.
                }
            else if(*w && *++w)
                {
                return false;
                }
            else // This is a leaf. Do the baseform and type stuff.
                {
                Pos = pos;
                Nmbr = nmbr;
                return true;
                }
            }
        else // Initial character alphabetically greater than any of the
             // available candidates.
            {
            return false;
            }
        }
    return false;
    }


/*
Structure of dictionary file.
# indicates that a number is read from the file (binary,non portable!)
Read operations are indicated by {}.
Repeated read operations are idicated by {}*(n), where n indicates the number of repetitions

FILE     = {STRINGS1}{LEXT}{NODES}
STRINGS1 = {#STRINGS1}{STRINGS1}

*/

void dictionary::readStrings(FILE * fp)
    {
    tlength stringBufLen;
    fread(&stringBufLen,sizeof(stringBufLen),1,fp);
    STRINGS = new char[stringBufLen+1];
    STRINGS[0] = '\0';
    STRINGS1 = STRINGS + 1;
    fread(STRINGS1,stringBufLen,1,fp);
    }

/*
LEXT                   = {#LEXT}{LEXT[0]LEXT[1]...LEXT[#LEXT-1]}

where

LEXT[i]                = {#TypeIndex}{#BaseFormSuffixIndex}{#Offset}

such that

LEXT[i].Type           = STRINGS1[TypeIndex-1]
LEXT[i].BaseFormSuffix = STRINGS1[BaseFormSuffixIndex-1]
*/
void dictionary::readLeaves(FILE * fp)
    {
    tcount leafBufLen;
    fread(&leafBufLen,sizeof(leafBufLen),1,fp);
    LEXT = new lext[leafBufLen];
    for(tcount i = 0;i < leafBufLen;++i)
        {
        tindex tmp;
        fread(&tmp,sizeof(tmp),1,fp);
        LEXT[i].Type = tmp + STRINGS;
        fread(&tmp,sizeof(tmp),1,fp);
        LEXT[i].BaseFormSuffix = tmp + STRINGS;
        fread(&LEXT[i].S,sizeof(LEXT[i].S),1,fp);
        /*
        fread(&LEXT[i].Offset,sizeof(LEXT[i].Offset),1,fp);
        fread(&LEXT[i].frequency,sizeof(LEXT[i].frequency),1,fp);
        */
        /*if(LEXT[i].S.frequency > 0)
            printf("%d %s %s %d\n",LEXT[i].S.Offset,LEXT[i].BaseFormSuffix,LEXT[i].Type,LEXT[i].S.frequency);*/
        }
    }

/*
NODES = {#nodeBufLen}{#ntoplevel}NODE*(ntoplevel)
NODE  = {#stringsIndex}{#numberOfChildren}{#pos}NODE*(numberOfChildren)

where N is ntoplevel for the first NODE and numberOfChildren for child NODES.
The tree is built in depth-first fashion.
*/
tcount dictionary::readStretch(tchildren length,tcount pos,FILE * fp)
    {
    tchildrencount i;
    for(i = 0;i < length;++i)
        {
        tindex tmp;
        fread(&tmp,sizeof(tmp),1,fp);
        fread(&NODES.numberOfChildren[pos + i],sizeof(NODES.numberOfChildren[pos + i]),1,fp);
        fread(&NODES.pos[pos + i],sizeof(NODES.pos[pos + i]),1,fp);
        NODES.strings[pos + i] = STRINGS + tmp;
        }
    tcount curr = pos + length;
    for(i = 0;i < length;++i)
        {
        if(NODES.pos[pos + i] < 0)
            {
            NODES.pos[pos + i] = -curr;
            curr = readStretch(NODES.numberOfChildren[pos + i],curr,fp);
            }
        }
    return curr;
    }

void dictionary::readNodes(FILE * fp)
    {
    tcount nodeBufLen;
    fread(&nodeBufLen,sizeof(nodeBufLen),1,fp);
    NODES.nnodes = nodeBufLen;
    NODES.initialchars = new char[nodeBufLen];
    NODES.strings = new char * [nodeBufLen];
    NODES.numberOfChildren = new tchildren[nodeBufLen];
    NODES.pos = new tindex[nodeBufLen];
    tchildren length;
    fread(&length,sizeof(length),1,fp);
    NODES.ntoplevel = length;
    readStretch(NODES.ntoplevel,0,fp);
    for(tcount i = 0;i < nodeBufLen;++i)
        {
        NODES.initialchars[i] = *NODES.strings[i];
//        putchar(NODES.initialchars[i]);
        }
    }

void dictionary::cleanup()
    {
    delete [] STRINGS;
    delete [] LEXT;
    delete [] NODES.initialchars;
    delete [] NODES.strings;
    delete [] NODES.numberOfChildren;
    delete [] NODES.pos;
    }

void dictionary::printlex(long pos, FILE * fp)
    {
    fprintf(fp,"%s %s %d %d",LEXT[pos].BaseFormSuffix,LEXT[pos].Type,LEXT[pos].S.Offset,LEXT[pos].S.frequency);
    }

void dictionary::printlex2(char * head,long pos, FILE * fp)
    {
    fprintf(fp,"%.*s%s/%s %d",(int)LEXT[pos].S.Offset,head,LEXT[pos].BaseFormSuffix,LEXT[pos].Type,LEXT[pos].S.frequency);
    }

void dictionary::printnode(int indent, long pos, FILE * fp)
    {
    tchildren n = NODES.numberOfChildren[pos];
    tchildrencount i;
    for(int j = indent;j;--j)
        fputc(' ',fp);
    fprintf(fp,"%s",NODES.strings[pos]);
    if(NODES.pos[pos] < 0)
        {
        fprintf(fp,"\n");
        for(i = 0;i < n;++i)
            {
            printnode(indent + 2,i - NODES.pos[pos],fp);
            }
        }
    else
        {
        fprintf(fp,"(");
        for(i = 0;i < n;++i)
            {
            printlex(NODES.pos[pos] + i,fp);
            if(i < NODES.numberOfChildren[pos] - 1)
                fprintf(fp,",");
            }
        fprintf(fp,")\n");
        }
    }

void dictionary::printnode2(char * head, long pos, FILE * fp)
    {
    int len = strlen(head);
    strcpy(head+len,NODES.strings[pos]);
    tchildren n = NODES.numberOfChildren[pos];
    tchildrencount i;
    if(NODES.pos[pos] < 0)
        {
        for(i = 0;i < n;++i)
            {
            printnode2(head,i - NODES.pos[pos],fp);
            }
        }
    else
        {
        fprintf(fp,"%s\t(",head);
        for(i = 0;i < n;++i)
            {
            printlex2(head,NODES.pos[pos] + i,fp);
            if(i < NODES.numberOfChildren[pos] - 1)
                fprintf(fp,",");
            }
        fprintf(fp,")\n");
        }
    head[len] = '\0';
    }





