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
#ifndef DEFINES_H
#define DEFINES_H
#define STREAM 0
#define FREQ24 0 /* $n field in baseform. This is a meaningless field
                     (not used for disambiguation) */
#define PFRQ 0 /* frequency field in baseformpointer. This keeps the
        frequency as stored in the dictionary. Dictionary frequency is a
        relation n(full form, base form). The baseformpointer is the right
        place to store this value, but it is difficult to get at it:
        If you list lemmas per full form, then it should be printed together
        with the lemmas. If, on the other hand, you list full forms per
        lemma, then it should be printed with the full forms. In the
        current architecture, the baseformpointer connecting full form and
        base form is not part of the known context in the printIt function.
        */ 


#define TREE 0

//#define COUNTOBJECTS  // Bart 20050916: for finding memory leak. 
                       // Printout of counts in ~flex()

#define SORTWORD 1
#define SORTFREQ 2
#define SORTPOS  3
/*#define SORTFREQ1 (SORTFREQ << 2)
#define SORTWORD1 (SORTWORD << 2)
#define SORTPOS1  (SORTPOS  << 2)*/

#define SortFreq(i) (i & SORTFREQ)
/*#define SortWord(i) (i & SORTWORD)
#define SortPos(i)  (i & SORTPOS )
#define SortFreq1(i) (i & SORTFREQ1)
#define SortWord1(i) (i & SORTWORD1)
#define SortPos1(i)  (i & SORTPOS1 )*/

#if defined __BORLANDC__
#define CONSTSTRCHR
/* Borland C++ 5.02 defines
const char *strchr(const char *s, int c);
char *strchr( char *s, int c);

Standard C++ defines 
char *strchr( const char *string, int c );

The Borland prototype is more picky and forces other functions to return
non-const character strings.
*/
#elif defined _MSC_VER
#if _MSC_VER >= 1400
#define CONSTSTRCHR
#endif
#endif
#endif
