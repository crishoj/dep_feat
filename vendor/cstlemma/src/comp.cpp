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

#include "comp.h"
#include "graph.h"

/*
ACL 2009 paper:
Icelandic   71.3    1.5 even_better (71,30 1,51 iflg. D:\dokumenter\tvärsök\even_better\icelandic.xls) peen 71,51 1,65 sugar 70,93 1,86 affiksFEW3 71,02 2,16 no pruning
Danish      92.8    0.2 peen sugar: 92,72 0,19 no pruning
Norwegian   87.6    0.3 affiksFEW2 sugar: 86,67 0,68
Greek       90.4    0.4 sugar no pruning
Slovene     86.7    0.3 affiksFEW3 affiksFEW2: 86,23 0,58 sugar: 86,27 0,41 peen:86,13 0,55  0,4
Swedish     92.3    0.3 sugar pruning 1
German      91.46   0.17 sugar no pruning
English     89.0    1.3 sugar pruning 2
Dutch       90.4    0.5 affiksFEW2 sugar: 90,17 0,31    0,3 no pruning
Polish      93.88   0.08 peen sugar: 93,88 0,08 (?) no pruning
*/

#if _NA
// IMPORTANT (20090511) R__NA and W__NA are not updated as sibling rules are 
// added and eat up the training pairs that earlier siblings did not handle.
// This error was detected after having used the weight functions for 
// the ACL-paper.

int comp_fairly_good(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    //fairly good, Icelandic 71.270883
//AMBI:
    // French ok 85.767516 ambi1 1.156051 ambi2 0.955414 diff 12.121019 rules 7337.500000 2.731849% cutoff 2
    int A1 = a->W__R           + a->R__R;
    int B1 = b->W__R           + b->R__R;
    int A2 = a->W__R + a->W__W           + a->R__NA;
    int B2 = b->W__R + b->W__W           + b->R__NA;
    int A3 = a->W__R           + a->R__R + a->R__NA;
    int B3 = b->W__R           + b->R__R + b->R__NA;
/*  int A2 = a->R__NA - a->W__NA;
    int B2 = b->R__NA - b->W__NA;
    int A3 = a->W__R - a->R__W;
    int B3 = b->W__R - b->R__W;
*/
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_even_better(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    //even better, Icelandic 71.300716
    // BEST Icelandic 71.535870 +/- 1.919590 at 0.9856 of dataset, 17 iterations, 23209.882353 = 40.477646% rules, cutoff = 0
    // Icelandic 71.283167 +/- 1.714260 at 0.9856 of dataset, 17 iterations, 22719.470588 = 39.622376% rules, cutoff = 0, RECURSE == 4
//AMBI:
    // French ok 85.487261 ambi1 1.283439 ambi2 1.050955 diff 12.178344 rules 7360.125000 2.740283% cutoff 2

    int A1 = a->W__R           + a->R__R;// wr + rr
    int B1 = b->W__R           + b->R__R;
    int A2 = a->W__R           + a->R__R + a->R__NA;// wr + rr + rn - r = wr - rw
    int B2 = b->W__R           + b->R__R + b->R__NA;
    int A3 = a->W__R + a->W__W           + a->R__NA;// wr + ww + rn - w = -wn + rn
    int B3 = b->W__R + b->W__W           + b->R__NA;
//    int A2 = a->W__R - a->R__W;
//    int B2 = b->W__R - b->R__W;
//    int A3 = a->R__NA - a->W__NA;
//    int B3 = b->R__NA - b->W__NA;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_affiksFEW3(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    // Icelandic 65.781623, cutoff 1 (old lemmatizer 73.329356, cutoff 0)
    // Icelandic 66.544995 +/- 1.943469 at 0.9856 of dataset, 17 iterations, 11134.176471 = 19.417817% rules, cutoff = 1
    // English 87.863636, cutoff 2 (old 87.954545, cutoff 1)
    // English 87.806061 +/- 1.009323 at 0.9856 of dataset, 15 iterations, 1619.133333 = 2.152101% rules (cutoff = 2)
    // BEST Slovene 86.669776 +/- 0.331106 at 0.9856 of dataset, 9 iterations, 5650.777778 = 2.888237% rules (cutoff = 2)
    // Slovene-ambi (4.23%) 83.165661, cutoff 3 (3550 rules!) (old 82.017103, cutoff 1, 9377 rules) Better than _affiksFEW2, 82.780013, 6656 rules.
    // Danish 90.942165	+/- 0.589437 at 0.9856 of dataset, 5 iterations, 32327.400000 = 5.925881% rules, cutoff = 1
    // German 90.266461 +/-	0.509202 at 0.9856 of dataset, 7 iterations, 21539.428571 = 6.930653% rules, cutoff = 1
    // Greek 89.640779 +/- 0.402079 at 0.9856 of dataset, 5 iterations, 13377.200000 = 2.472132% rules, cutoff = 2
    // Dutch 87.817059 +/- 0.366236 at 0.9856 of dataset, 7 iterations, 23493.571429 = 7.895486% rules, cutoff = 1
    // Norwegian 85.788507 +/- 0.484921 at 0.9856 of dataset, 6 iterations, 14904.000000 = 3.157580% rules, cutoff = 2
    // Polish 93.203365 +/- 0.175436 at 0.9856 of dataset, 2 iterations, 50597.500000 = 1.491153% rules, cutoff = 2
    // Swedish 91.709042 +/- 0.170094 at 0.9856 of dataset, 6 iterations, 4407.666667 = 0.935737% rules, cutoff = 3
//AMBI:
    // French ok 82.754777 ambi1 2.353503 ambi2 1.805732 diff 13.085987 rules 7360.125000 2.740283% cutoff 2

    /* Interesting because it generates far less rules than the above 
       variables, only 20 % more than the old lemmatizer.
       Also interesting is that there are not many leaves with only one
       supporting training pair.
       Yet, the leaves with only one supporter are detrimentous to the overall
       result (cutoff has to be 1 or even 2).
    */
#if 1
    int A1 = a->W__R           + a->R__R + a->R__NA; // Good: previously wrong words got it right. Bad: previously right words got it wrong.
    int B1 = b->W__R           + b->R__R + b->R__NA;
    int A2 = a->W__R           + a->R__R; // Good: any rightly lemmatized word
    int B2 = b->W__R           + b->R__R;
    int A3 = a->W__R + a->W__W           + a->R__NA; // Good: previously right words that didn't match. They may return to the parent.
    int B3 = b->W__R + b->W__W           + b->R__NA; // Bad: previously wrong words that didn't match. They must be handled by siblings.
#else
    int A1 = a->W__R - a->R__W; // Good: previously wrong words got it right. Bad: previously right words got it wrong.
    int B1 = b->W__R - b->R__W;
    int A2 = a->W__R + a->R__R; // Good: any rightly lemmatized word
    int B2 = b->W__R + b->R__R;
    int A3 = a->W__R + a->W__W - a->R__R - a->R__W; // Good: previously right words that didn't match. They may return to the parent.
    int B3 = b->W__R + b->W__W - b->R__R - b->R__W; // Bad: previously wrong words that didn't match. They must be handled by siblings.
#endif
/*
    int A1 = a->W__R - a->R__W; // Good: previously wrong words got it right. Bad: previously right words got it wrong.
    int B1 = b->W__R - b->R__W;
    int A2 = a->W__R + a->R__R; // Good: any rightly lemmatized word
    int B2 = b->W__R + b->R__R;
    int A3 = a->R__NA - a->W__NA; // Good: previously right words that didn't match. They may return to the parent.
    int B3 = b->R__NA - b->W__NA; // Bad: previously wrong words that didn't match. They must be handled by siblings.
*/
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_affiksFEW(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
/**/
    //_affiksFEW
    // Dutch 88.138224, 39943.5 flexrules cutoff 1 (old 89.656164, 47277.75 flexrules, cutoff 1)
    // German 90.266461	+/-	0.509202 at 0.9856 of dataset, 7 iterations, 21539.428571 = 6.930653% rules, cutoff = 1
//AMBI:
    // French ok 82.617834 ambi1 2.455414 ambi2 1.872611 diff 13.054140 rules 7360.125000 2.740283% cutoff 2
    int N = a->W__W + a->W__R + a->W__NA + a->R__W + a->R__R + a->R__NA;

    int A1;
    int B1;
    int A2;
    int B2;
    int A3;
    int B3;

    // good for small numbers:
    if(N < 3)
        {
        A1 = a->W__R           + a->R__R;
        B1 = b->W__R           + b->R__R;
        A2 = a->W__R           + a->R__R + a->R__NA;
        B2 = b->W__R           + b->R__R + b->R__NA;
        A3 = a->W__R + a->W__W           + a->R__NA;
        B3 = b->W__R + b->W__W           + b->R__NA;
/*      A1 = a->W__R + a->R__R;
        B1 = b->W__R + b->R__R;
        A2 = a->W__R - a->R__W;
        B2 = b->W__R - b->R__W;
        A3 = a->R__NA - a->W__NA;
        B3 = b->R__NA - b->W__NA;
*/
        }
    // good for big numbers:
    else
        {
        A1 = a->W__R + a->R__R + a->R__NA; // Good: previously wrong words got it right. Bad: previously right words got it wrong.
        B1 = b->W__R + b->R__R + b->R__NA;
        A2 = a->W__R + a->R__R; // Good: any rightly lemmatized word
        B2 = b->W__R + b->R__R;
        A3 = a->R__NA + a->W__R + a->W__W; // Good: previously right words that didn't match. They may return to the parent.
        B3 = b->R__NA + b->W__R + a->W__W; // Bad: previously wrong words that didn't match. They must be handled by siblings.
/*      A1 = a->W__R - a->R__W; // Good: previously wrong words got it right. Bad: previously right words got it wrong.
        B1 = b->W__R - b->R__W;
        A2 = a->W__R + a->R__R; // Good: any rightly lemmatized word
        B2 = b->W__R + b->R__R;
        A3 = a->R__NA - a->W__NA; // Good: previously right words that didn't match. They may return to the parent.
        B3 = b->R__NA - b->W__NA; // Bad: previously wrong words that didn't match. They must be handled by siblings.
*/
        }
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_affiksFEW2(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    //_affiksFEW2
    // (OK) BEST Dutch 90.452096 +/- 0.655431 at 0.9856 of dataset, 7 iterations, 53607.714286 = 18.015948% rules, cutoff = 0
    // (OK) Norwegian 86.776860 +/- 0.642621 at 0.9856 of dataset, 6 iterations, 112374.000000 = 23.807698% rules, cutoff = 0
    // (OK) English 88.424242 +/- 1.191106 at 0.9856 of dataset, 15 iterations, 1383.000000 = 1.838240% rules, cutoff = 2
    // (OK) Icelandic 71.304226 +/- 1.453643 at 0.9856 of dataset, 17 iterations, 25635.000000 = 44.707011% rules, cutoff = 0
    // (OK) German 91.156762 +/- 0.348391 at 0.9856 of dataset, 7 iterations, 48816.571429 = 15.707506% rules, cutoff = 0
    // (OK) Slovene 86.537639 +/- 0.559484 at 0.9856 of dataset, 9 iterations, 40643.444444 = 20.773759% rules, cutoff = 0
    // (OK) Swedish 91.907598 +/- 0.224888 at 0.9856 of dataset, 6 iterations, 27958.000000 = 5.935415% rules, cutoff = 1
    // (OK) Greek 90.741209 +/- 0.312526 at 0.9856 of dataset, 5 iterations, 125306.400000 = 23.156860% rules, cutoff = 0
    // (OK) Danish 92.994605 +/- 0.210674 at 0.9856 of dataset, 5 iterations, 67278.800000 = 12.332763% rules, cutoff = 0
    
    // (?)ALMOST BEST Polish 93.398015 +/- 0.045642 at 0.9856 of dataset, 2 iterations, 165511.500000 = 4.877770% rules, cutoff = 1
//AMBI:
    // French ok 84.194268 ambi1 2.277070 ambi2 1.576433 diff 11.952229 rules 6453.250000 2.402640% cutoff 2

#if 1 // 20090511
    int A1 = a->W__R           + 2*a->R__R + a->R__NA; // good: all words that are lemmatised correctly. bad: all previously right words that got it wrong
    // wr + 2rr + rn - r = ww + rr - rw
    int B1 = b->W__R           + 2*b->R__R + b->R__NA;
    int A2 = a->W__R           +   a->R__R + a->R__NA;
    // wr + rr + rn - r = wr - rw
    int B2 = b->W__R           +   b->R__R + b->R__NA;
    int A3 = a->W__R + a->W__W             + a->R__NA;
    // wr + ww + rn - w = -wn + rn
    int B3 = b->W__R + b->W__W             + b->R__NA;
#else
    int A1 = a->W__R           + a->R__R - a->R__W; // good: all words that are lemmatised correctly. bad: all previously right words that got it wrong
    // wr + 2rr + rn - r = ww + rr - rw
    int B1 = b->W__R           + b->R__R - b->R__W;
    int A2 = a->W__R                     - a->R__W;
    // wr + rr + rn - r = wr - rw
    int B2 = b->W__R                     - b->R__W;
    int A3 = a->W__R + a->W__W - a->R__R - a->R__W;
    // wr + ww + rn - w = -wn + rn
    int B3 = b->W__R + b->W__W - b->R__R - b->R__W;
#endif
/*  int A1 = a->W__R + a->R__R - a->R__W; // good: all words that are lemmatised correctly. bad: all previously right words that got it wrong
    int B1 = b->W__R + b->R__R - b->R__W;
    int A2 = a->W__R - a->R__W;
    int B2 = b->W__R - b->R__W;
    int A3 = a->R__NA - a->W__NA;
    int B3 = b->R__NA - b->W__NA;
*/
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_affiksFEW2org(const vertex * a,const vertex * b)
    {
    // BEST Norwegian 87.494563 +/- 0.217147 at 0.9856 of dataset, 6 iterations, 101814.500000 = 21.570549% rules, cutoff = 0
    // English 88.260606 +/- 0.826699 at 0.9856 of dataset, 15 iterations, 7362.466667 = 9.785960% rules, cutoff = 1
    // Icelandic 70.651411 +/- 1.565857 at 0.9856 of dataset, 17 iterations, 23232.941176 = 40.517860% rules, cutoff = 0
    // German 90.307358 +/- 0.355867 at 0.9856 of dataset, 7 iterations, 50595.857143 = 16.280019% rules, cutoff = 0
    // Dutch 90.274675 +/- 0.462929 at 0.9856 of dataset, 7 iterations, 23452.142857 = 7.881563% rules, cutoff = 1    
    // Slovene 86.417162 +/- 0.540735 at 0.9856 of dataset, 9 iterations, 40847.666667 = 20.878142% rules, cutoff = 0
    // Swedish 91.982663 +/- 0.250703 at 0.9856 of dataset, 6 iterations, 28998.000000 = 6.156204% rules, cutoff = 1
    // Greek 90.258032 +/- 0.234665 at 0.9856 of dataset, 5 iterations, 43156.000000 = 7.975310% rules, cutoff = 1 (but exactly the same as cutoff = 0)
    // Danish 92.425041 +/- 0.374415 at 0.9856 of dataset, 5 iterations, 73177.800000 = 13.414099% rules, cutoff = 0
//AMBI:
    // French ok 84.761146 ambi1 2.015924 ambi2 1.665605 diff 11.557325 rules 7262.500000 2.703935% cutoff 2
    int A1 = a->W__R + a->R__R - a->R__W; // good: all words that are lemmatised correctly. bad: all previously right words that got it wrong
    int B1 = b->W__R + b->R__R - b->R__W;
    int A2 = a->W__R - a->R__W;
    int B2 = b->W__R - b->R__W;
    int A3 = a->R__NA - a->W__NA;
    int B3 = b->R__NA - b->W__NA;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_fixNA(const vertex * a,const vertex * b)
    {
    /*
    Icelandic 47.982267 (at 0.8488 of dataset)
    */
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    //_fixNA
//AMBI:
    // French: stopped because of very bad results. (> 25% wrong results) 

    int A1 = a->W__R + a->W__NA           + a->R__NA;
    int B1 = b->W__R + b->W__NA           + b->R__NA;
    int A2 = a->W__R            + a->R__R + a->R__NA;
    int B2 = b->W__R            + b->R__R + b->R__NA;
    int A3 = a->W__R                      + a->R__NA;
    int B3 = b->W__R                      + b->R__NA;

    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_fruit(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    //Icelandic 71.344041 at 0.939 of dataset
    //ALMOST BEST Icelandic 71.521831 +/- 1.988737 at 0.9856 of dataset, 17 iterations, 23539.352941 = 41.052237% rules
    //Slovene 85.900276 +/- 0.456532 at 0.9856 of dataset, 9 iterations, 42167.333333 = 21.552652% rules
    //English 87.626771	+/- 0.060148 at 0.4928 (!) of dataset, 3 iterations, 933.000000 = 2.480262% rules
//AMBI:
    // French ok 85.382166 ambi1 1.359873 ambi2 1.089172 diff 12.168790 rules 7259.125000 2.899075% cutoff 2
    int A1 = a->W__R            + a->R__R;
    int B1 = b->W__R            + b->R__R;
    int A2 = a->W__R            + a->R__R + a->R__NA;
    int B2 = b->W__R            + b->R__R + b->R__NA;
    int A3 = a->W__R + a->W__NA           + a->R__NA;
    int B3 = b->W__R + b->W__NA           + b->R__NA;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_ice(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    //Icelandic 60.242322 at 0.939 of dataset
//AMBI:
    // French ok 82.557325 ambi1 2.522293 ambi2 1.866242 diff 13.054140 rules 8556.625000 3.185757% cutoff 2
    int A1 = a->W__R            + a->R__R + a->R__NA;
    int B1 = b->W__R            + b->R__R + b->R__NA;
    int A2 = a->W__R            + a->R__R;
    int B2 = b->W__R            + b->R__R;
    int A3 = a->W__R + a->W__NA           + a->R__NA;
    int B3 = b->W__R + b->W__NA           + b->R__NA;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_pisang(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    //Icelandic 71.287687 at 0.939 of dataset
//AMBI:
    // French ok 85.414013 ambi1 1.359873 ambi2 1.085987 diff 12.140127 rules 7848.375000 2.922065% cutoff 2
    int A1 = a->W__R            + a->R__R;
    int B1 = b->W__R            + b->R__R;
    int A2 = a->W__R + a->W__NA           + a->R__NA;
    int B2 = b->W__R + b->W__NA           + b->R__NA;
    int A3 = a->W__R            + a->R__R + a->R__NA;
    int B3 = b->W__R            + b->R__R + b->R__NA;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_kiwi(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    //Icelandic 70.865032 at 0.939 of dataset
//AMBI:
    // French ok 85.410828 ambi1 1.378981 ambi2 1.035032 diff 12.175159 rules 7676.875000 2.858213% cutoff 2
    int A1 = a->W__R            + a->R__R;
    int B1 = b->W__R            + b->R__R;
    int A2 = a->W__R + a->W__NA + a->R__R + a->R__NA;
    int B2 = b->W__R + b->W__NA + b->R__R + b->R__NA;
    int A3 = a->W__R                      + a->R__NA;
    int B3 = b->W__R                      + b->R__NA;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_carrot(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    //Icelandic 71.090448 at 0.939 of dataset
//AMBI:
    // French ok 85.060510 ambi1 1.328025 ambi2 1.041401 diff 12.570064 rules 7241.625000 2.696163% cutoff 2
    int A1 = 4*(a->W__R        + a->R__R) + a->R__NA;
    int B1 = 4*(b->W__R        + b->R__R) + b->R__NA;
    int A2 = a->W__R            + a->R__R + a->R__NA;
    int B2 = b->W__R            + b->R__R + b->R__NA;
    int A3 = a->W__R + a->W__NA           + a->R__NA;
    int B3 = b->W__R + b->W__NA           + b->R__NA;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_peen(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    // Icelandic 71.344041 at 0.939 of dataset
    // ALMOST BEST Icelandic 71.507792 +/- 1.645702 at 0.9856 of dataset, 17 iterations, 25240.882353 = 44.019676% rules
    // Slovene 86.133458	+/- 0.549185 at 0.9856 of dataset, 9 iterations, 40898.777778 = 20.904266% rules
    // English 87.803261	+/- 0.106156 at 0.4928 (!) of dataset, 3 iterations, 889.333333 = 2.364179% rules
    // Dutch 89.837692 +/- 0.412795 at 0.9856 of dataset, 7 iterations, 56640.285714 = 19.035104% rules, cutoff = 0
    // ALMOST BEST German 91.288892 +/- 0.670828 at 0.9856 of dataset, 7 iterations, 50584.857143 = 16.276480% rules, cutoff = 0
    // Swedish 91.873698 +/- 0.367967 at 0.9856 of dataset, 6 iterations, 9066.166667 = 1.924725% rules, cutoff = 2
    // ALMOST BEST Norwegian 87.535644 +/- 0.344659 at 0.9856 of dataset, 6 iterations, 48468 = 10.268492% rules, cutoff = 1
    // ALMOST BEST Greek 90.414875+/- 0.385254 at 0.9856 of dataset, 5 iterations, 120691.4 = 22.303999% rules, cutoff = 0
    // BEST Danish 92.796387 +/- 0.214267 at 0.9856 of dataset, 5 iterations, 67807 = 12.429587% rules, cutoff = 0
    // ALMOST BEST Russian 80.484806 +/- 0.409391 at 0.9856 of dataset, 6 iterations, 54630 = 14.022614% rules, cutoff = 1
    // BEST Polish 93.880103 +/- 0.077021 at 0.9856 of dataset, 2 iterations, 344944.5	= 10.165818% rules, cutoff = 0
//AMBI:
    // French ok 84.993631 ambi1 1.388535 ambi2 1.085987 diff 12.531847 rules 7318.375000 2.724738% cutoff 2
    int A1 = 3*(a->W__R        + a->R__R) + a->R__NA;
    // 3wr + 3rr + rn - r = 3wr + 2rr - rw
    int B1 = 3*(b->W__R        + b->R__R) + b->R__NA;
    int A2 = a->W__R            + a->R__R + a->R__NA;
    //wr + rr + rn - r = wr - rw
    int B2 = b->W__R            + b->R__R + b->R__NA;
    int A3 = a->W__R + a->W__NA           + a->R__NA;
    // wr + wn + rn - w = -ww + rn
    int B3 = b->W__R + b->W__NA           + b->R__NA;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif
#if _NA
int comp_sugar(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    // Slovene 86.273367 +/- 0.410931 at 0.9856 of dataset, 9 iterations, 17254.777778 = 8.819297% rules (cutoff = 1)
    // BEST English 89.060606 +/- 1.320829 at 0.9856 of dataset, 3 iterations, 1318.266667 = 1.752199% rules, cutoff=2
    // Icelandic 70.925172 +/- 1.858255 at 0.9856 of dataset, 17 iterations, 27151.294118 = 47.351402% rules, cutoff = 0
    // Dutch 90.172822 +/- 0.307911 at 0.9856 of dataset, 7 iterations, 57761.142857 = 19.411791% rules, cutoff = 0
    // BEST Greek 90.422464 +/- 0.437009 at 0.9856 of dataset, 5 iterations, 132765.6 = 24.535334% rules, cutoff = 0
    // BEST German 91.461918 +/- 0.167574 at 0.9856 of dataset, 7 iterations, 50986 = 16.405554% rules, cutoff = 0
    // BEST Swedish 92.265969 +/- 0.277289 at 0.9856 of dataset, 6 iterations, 25935.333333 = 5.506008% rules, cutoff = 1
    // Norwegian 86.665700 +/- 0.676264 at 0.9856 of dataset, 6 iterations, 46685.5 = 9.890849% rules, cutoff = 1
    // Danish 92.585623 +/- 0.171327 at 0.9856 of dataset, 5 iterations, 30422.400000 = 5.576679% rules, cutoff = 1
    // BEST Russian 80.815622 +/- 0.450500 at 0.9856 of dataset, 6 iterations, 47079.166667 = 12.084440% rules, cutoff = 1		
//AMBI:
    // French ok 75.472316 ambi1 4.615600 ambi2 3.493266 diff 16.418818 rules 4162.909091 3.129560% cutoff 2

#if 1
    // next lines from affixFEW2
    int A1 = a->W__R           + 2*a->R__R + a->R__NA; // good: all words that are lemmatised correctly. bad: all previously right words that got it wrong
    // wr - rw + rr
    int B1 = b->W__R           + 2*b->R__R + b->R__NA;
    int A2 = a->W__R           +   a->R__R + a->R__NA;
    // wr - rw
    int B2 = b->W__R           +   b->R__R + b->R__NA;
    // next lines from peen
    int A3 = a->W__R + a->W__NA           + a->R__NA;
    // -ww + rn 
    int B3 = b->W__R + b->W__NA           + b->R__NA;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
#else
    //equivalent with:
    int AA1 = a->W__R - a->R__W + a->R__R;
    int AA2 = - a->R__R;
    int AA3 = - a->W__R - 2*a->W__W;//a->R__NA - a->W__W;
    int BB1 = b->W__R - b->R__W + b->R__R;
    int BB2 = - b->R__R;
    int BB3 = - b->W__R - 2*b->W__W;//b->R__NA - b->W__W;
#endif
    }
#endif

int comp_honey(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    // (OK) Dutch 90.179393 +/- 0.589662 at 0.9856 of dataset, 7 iterations, 73324.571429 = 24.642193% rules, cutoff = 0
    // (OK) Norwegian 87.272244 +/- 0.267729 at 0.9856 of dataset, 6 iterations, 141038.666667 = 29.880630% rules, cutoff = 0
    // (OK) English 88.315152 +/- 1.097312 at 0.9856 of dataset, 3 iterations, 5285.466667 = 7.025276% rules, cutoff=1
    // (OK) Icelandic 70.742665 +/- 1.686147 at 0.9856 of dataset, 17 iterations, 29857.000000 = 52.070108% rules, cutoff = 0
    // (?) Slovene 86.273367 +/- 0.410931 at 0.9856 of dataset, 9 iterations, 17254.777778 = 8.819297% rules (cutoff = 1)
    // (?) BEST Greek 90.422464 +/- 0.437009 at 0.9856 of dataset, 5 iterations, 132765.6 = 24.535334% rules, cutoff = 0
    // (?) BEST German 91.461918 +/- 0.167574 at 0.9856 of dataset, 7 iterations, 50986 = 16.405554% rules, cutoff = 0
    // (?) BEST Swedish 92.265969 +/- 0.277289 at 0.9856 of dataset, 6 iterations, 25935.333333 = 5.506008% rules, cutoff = 1
    // (?) Danish 92.585623 +/- 0.171327 at 0.9856 of dataset, 5 iterations, 30422.400000 = 5.576679% rules, cutoff = 1
    // (?) BEST Russian 80.815622 +/- 0.450500 at 0.9856 of dataset, 6 iterations, 47079.166667 = 12.084440% rules, cutoff = 1		
//AMBI:
    // French ok 84.477707 ambi1 2.251592 ambi2 1.426752 diff 11.843949 rules 7413.875000 2.760295% cutoff 2
    int A1 = a->W__R           + 2*a->R__R;
    int B1 = b->W__R           + 2*b->R__R;
    int A2 = a->W__R           +   a->R__R;
    int B2 = b->W__R           +   b->R__R;
    int A3 = a->W__R ;
    int B3 = b->W__R ;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }

#if _NA
int comp_beet(const vertex * a,const vertex * b)
    {
    //const vertex * a = *(const vertex **)A;
    //const vertex * b = *(const vertex **)B;
    //Icelandic 71.034094 at 0.939 of dataset
//AMBI:
    // French ok 85.057325 ambi1 1.283439 ambi2 1.057325 diff 12.601911 rules 7260.375000 2.703144% cutoff 2
    int A1 = 2*(a->W__R        + a->R__R) + a->R__NA;
    int B1 = 2*(b->W__R        + b->R__R) + b->R__NA;
    int A2 = a->W__R            + a->R__R + a->R__NA;
    int B2 = b->W__R            + b->R__R + b->R__NA;
    int A3 = a->W__R + a->W__NA           + a->R__NA;
    int B3 = b->W__R + b->W__NA           + b->R__NA;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }
#endif

int comp_koud(const vertex * a,const vertex * b)
    {
    // German 91.260578 +/- 0.363285 at 0.9856 of dataset, 7 iterations, 30890.714286 = 9.939577% rules, cutoff = 0
//AMBI:
    // French 86.356688 ambi1 0.996815 ambi2 0.796178 diff 11.850318 rules 3335.250000 1.241763% cutoff 3 (!)
    // French 85.250493 ambi1 2.333057 ambi2 2.161181 diff 10.255268 rules 28520.250000 10.618597% cutoff 0 (!) paradigms+homographs clumped
    // French 85.313973 ambi1 2.050694 ambi2 2.289517 diff 10.345816 rules 28509.250000 10.614432% cutoff 0 (!) homographs clumped
    // Dutch.clumped.ph 85.789838 ambi1 1.086067 ambi2 1.256060 diff 11.868035 rules 37400.142857 12.190637% cutoff 0 paradigms+homographs clumped
    // Dutch.clumped.h 85.818923 ambi1 1.095507 ambi2 1.060476 diff 12.025095 rules 37411.857143 12.192383% cutoff 0 homographs clumped
    // (Dutch.clumped.ph suffix, old algo:
    //                  83.532708 ambi1 1.948624 ambi2 2.719889 ambi3 0.107033 diff 11.691746 rules 73024.571429 23.802477% cutoff 0 paradigms+homographs clumped
    // (Dutch.clumped.h suffix, old algo:
    //                  83.624725 ambi1 1.859813 ambi2 2.611382 ambi3 0.162415 diff 11.741664 rules 72975.428571 23.782417% cutoff 0 paradigms+homographs clumped
    // Russian clumped ph 74.983460 ambi1 0.517762 ambi2 0.558033 diff 23.940745 rules 95077.500000 24.184389% cutoff 0 paradigms+homographs clumped
    // (old algo:)
    //                  79.485114 ambi1 0.218611 ambi2 0.342298 ambi3 0.005753 diff 19.948224 rules 94247.166667 23.973181% cutoff 0 paradigms+homographs clumped
    // The A1 vs B1 condition is pretty close to what later was found as the
    // best factors using automatic factor setting (comp_parms).
    // These factors were found by manual optimizing.
    int A1 = 6*a->W__R - 5*a->R__W + a->W__W;
    int B1 = 6*b->W__R - 5*b->R__W + b->W__W;
    int A2 = a->W__R - 6*a->R__W;
    int B2 = b->W__R - 6*b->R__W;
    int A3 = a->R__R - a->W__W;
    int B3 = b->R__R - b->W__W;
    return (A1>B1)?-1:(A1<B1)?1:(A2>B2)?-1:(A2<B2)?1:(A3>B3)?-1:(A3<B3)?1:0;
    }

// You can find a local optimum for the parameters by using comp_parms as the
// weight function and setting compute_parms = true. The parameters parms[] 
// can be seeded with non-zero values by hard coding. The file parms.txt
// will contain the currently best parameters.

// Optimal parameters == resulting in smallest rule set.
// Hypothesis: small rule sets give (almost) best lemmatization results.
// Optimizing for the size of rule sets is computationally MUCH cheaper
// than optimizing for accuracy.

// If you have found a good set of parameters (presumably with a small subset
// of the training data), you can hard code them (as is done below) and run 
// the program with the full set of the training data. In that case,
// set compute_parms = false

bool compute_parms = false;

#define FLOATINGPARMS 0

#if FLOATINGPARMS
//#define NPARMS 16
#define NPARMS 4
double parms[NPARMS] = 
#if 1
#if 1
#if 1
    {0};
#else
    {
0.001866,	0.512337,	-0.561650,	0.056732,
0.254143,	0.275942,	0.235856,	-0.165358,
0.000241,	-0.114895,	-0.145030,	-0.398223,
-0.040605,	0.017982,	0.015184,	-0.010743
    };
#endif
#else
    {
    0.090612,	0.786132,	-0.328493,	0.421432,
    -0.443032,	-0.945906,	0.330267,	0.672812,
    -0.244571,	-0.080198,	-0.658630,	0.612534,
    -0.188197,	0.423856,	-0.103047,	0.048760
    };
#endif
#else
    {
    -12.343588,	6.757362,	-0.124495,	-0.119445,
    3.420677,	-4.421679,	-3.634280,	7.381257,
    3.606583,	5.137671,	-15.996030,	5.509965,
    -0.206408,	3.749373,	2.990789,	-16.252610
    };
#endif

double best[NPARMS]     = {0};

double D[NPARMS]        = {0};

double inner(double * r1,double * r2,int cols)
    {
    double x2 = 0;
    for(int col = 0;col < cols;++col)
        x2 += r1[col]*r2[col];
    return x2;
    }

void plus(double * dest, double * term,double factor,int cols)
    {
    for(int col = 0;col < cols;++col)
        dest[col] += factor * term[col];
    }

void normal(double * dest, double * term,int cols)
    {
    double f = inner(term,dest,cols)/inner(term,term,cols);
    for(int col = 0;col < cols;++col)
        dest[col] -= f * term[col];
/*    f = inner(term,dest,cols);
    printf("F %f\n",f);*/
    }

void unit(double * dest,int cols)
    {
    double f = sqrt(inner(dest,dest,cols));
    if(f > 0.0)
        {
        for(int col = 0;col < cols;++col)
            dest[col] /= f;
        }
    }

void copy(double * dest,double * source,int cols)
    {
    for(int col = 0;col < cols;++col)
        dest[col] = source[col];
    }

void betterfound()
    {
    copy(best,parms,NPARMS);
    }

#if NPARMS == 16
/* Based on a useless idea:
Each leg of the tetrad defines a condition.
If the first condition ends in a tie, try the second, etc.
BUT: the first condition will never end in a tie if the condition's factors 
are floating point numbers and no rounding is applied.
If rounding is introduced: to which decimal?
*/
void tetrad(double * D)
    {
    /*
    printf("%f %f %f %f %f %f\t",
        inner(D+4,D,4),
        inner(D+8,D,4),
        inner(D+12,D,4),
        inner(D+8,D+4,4),
        inner(D+12,D+4,4),
        inner(D+12,D+8,4));
        */
    normal(D+4,D,4);
    normal(D+8,D,4);
    normal(D+12,D,4);
    normal(D+8,D+4,4);
    normal(D+12,D+4,4);
    normal(D+12,D+8,4);
    /*
    printf("%f %f %f %f %f %f\n",
        inner(D+4,D,4),
        inner(D+8,D,4),
        inner(D+12,D,4),
        inner(D+8,D+4,4),
        inner(D+12,D+4,4),
        inner(D+12,D+8,4));
        */
    }
#endif

void brown(int iteration)
    {
    double delta = 2.0*pow(0.99,iteration);
    FILE * f = fopen("parms.txt","a");
    for(int i = 0;i < NPARMS;++i)
        {
        int row,col;
        row = i / 4;
        col = i % 4;
        if(col >= row)
            {
            double ran = rand() / (double)RAND_MAX;
            ran = exp(-ran*ran);
            if(rand() > (RAND_MAX/2))
                ran = -ran;
            D[i] = ran;
            }
        else
            D[i] = 0.0;
        }

    //printf("D:");
    //tetrad(D);


    //if(inner(parms,parms,NPARMS) > 0.99)
    if(delta > 1.0)
        {
        copy(parms,D,NPARMS);
        }
    else
        {
        copy(parms,best,NPARMS);
        //normal(D,parms,NPARMS);
        unit(D,NPARMS);
        plus(parms,D,delta,NPARMS);
        }

    printf("parms:");
#if NPARMS == 16
    tetrad(parms);
#endif
    unit(parms,NPARMS);

    for(int i = 0;i < NPARMS;++i)
        {
        fprintf(f,"%f",parms[i]);
        if(((i+1) % 4) == 0)
            if(i == NPARMS - 1)
                fprintf(f,"\n");
            else
                fprintf(f,",\n");
        else
            fprintf(f,",\t");
        }
    fclose(f);
    }

void init()
    {
    double delta = 2.0;
    double x2 = 0;
    int i;
    for(i = 0;i < NPARMS;++i)
        {
        x2 += parms[i]*parms[i];
        }
    if(x2 < 0.123)
        brown(delta);
    else
        {
        //double x = sqrt(x2);
        FILE * f = fopen("parms.txt","a");
        for(i = 0;i < NPARMS;++i)
            {
            //parms[i] /= x;
            fprintf(f,"%f",parms[i]);
            if(((i+1) % 4) == 0)
                if(i == NPARMS - 1)
                    fprintf(f,"\n");
                else
                    fprintf(f,",\n");
            else
                fprintf(f,",\t");
            }
        fclose(f);
        }
    }

int pcnt[4] = {0,0,0,0};

void printparms(int Nnodes)
    {
    int i;
    FILE * f = fopen("parms.txt","a");
    fprintf(f,"/* %d */\n",Nnodes);
    double x2 = 0;
    for(i = 0;i < NPARMS;++i)
        {
        x2 += parms[i]*parms[i];
        }
    double x = sqrt(x2);
    for(i = 0;i < NPARMS;++i)
        {
        fprintf(f,"%d",(int)(100.0*parms[i]/x));
        if(((i+1) % 4) == 0)
            {
            fprintf(f,"\t%d\n",pcnt[i >> 2]);
            pcnt[i >> 2] = 0;
            }
        else
            fprintf(f,"\t");
        }
    fclose(f);
    }

#if 0
int comp_parms(const vertex * a,const vertex * b)
    {
    for(int o = 0;o < NPARMS;o += 4)
        {
        double A = parms[o]*a->R__R + parms[o+1]*a->W__R + parms[o+2]*a->R__W + parms[o+3]*a->W__W;
        double B = parms[o]*b->R__R + parms[o+1]*b->W__R + parms[o+2]*b->R__W + parms[o+3]*b->W__W;
        if(A != B)
            {
            ++pcnt[o >> 2]; // For counting the number of times the first, second, third or fourth condition has been used.
            // (Hypothesis: with parms as doubles the first condition is used and only in very special cases the second.
            //  Addendum: This hypothesis holds.)
            return A > B ? -1 : 1;
            }
        }
    return 0;
    }
#else
#if NPARMS == 16
int comp_parms(const vertex * a,const vertex * b)
    {
    double A = 0;
    double B = 0;
    int s = 0;
    for(int o = 0;o < NPARMS;o += 4,s += 2)
        {
        A += parms[o]*(a->R__R >> s) + parms[o+1]*(a->W__R >> s) + parms[o+2]*(a->R__W >> s) + parms[o+3]*(a->W__W >> s);
        B += parms[o]*(b->R__R >> s) + parms[o+1]*(b->W__R >> s) + parms[o+2]*(b->R__W >> s) + parms[o+3]*(b->W__W >> s);
        }
    if(A != B)
        {
        return A > B ? -1 : 1;
        }
    return 0;
    }
#elif NPARMS == 4
int comp_parms(const vertex * a,const vertex * b)
    {
    double A = 0;
    double B = 0;
    A += parms[0]*a->R__R + parms[1]*a->W__R + parms[2]*a->R__W + parms[3]*a->W__W;
    B += parms[0]*b->R__R + parms[1]*b->W__R + parms[2]*b->R__W + parms[3]*b->W__W;
    if(A != B)
        {
        return A > B ? -1 : 1;
        }
    return 0;
    }
#else
#error NPARMS invalid
#endif
#endif
#else // !FLOATINGPARMS
#define NPARMS 16
int parms[NPARMS]    = {0};

int best[NPARMS]     = 
    {
  0,	 10,	-10,	  0,
  0,	  0,	  0,	 10,
 10,	 10,	 10,	  0,
  0,	  0,	-10,	  0
    };

int D[NPARMS]        = {0};

int R[] = 
    {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1,
    1,-1,0,0,
    1,1,0,0,
    1,0,-1,0,
    1,0,1,0,
    1,0,0,1,
    1,0,0,-1,
    0,1,-1,0,
    0,1,1,0,
    0,1,0,-1,
    0,1,0,1,
    0,0,1,-1,
    0,0,1,1
    };

void plus(int * dest, int * term,int cols)
    {
    for(int col = 0;col < cols;++col)
        dest[col] += term[col];
    }

void copy(int * dest,int * source,int cols)
    {
    for(int col = 0;col < cols;++col)
        dest[col] = source[col];
    }

void betterfound(int Nnodes)
    {
    copy(best,parms,NPARMS);
    FILE * f = fopen("best.txt","a");
    if(f)
        {
        fprintf(f,"/* %d */\n",Nnodes);
        for(int i = 0;i < NPARMS;++i)
            {
            fprintf(f,"%d",parms[i]);
            if(((i+1) % 4) == 0)
                if(i == NPARMS - 1)
                    fprintf(f,"\n");
                else
                    fprintf(f,",\n");
            else
                fprintf(f,",\t");
            }
        fclose(f);
        }
    }


void brown(int iteration)
    {
    static int it = 0;
    FILE * f = fopen("parms.txt","a");
    D[rand() % NPARMS] += (rand() & 1) ? 1 : -1;

    int i;
    int T = it;
    int R0 = (T % 16) * 4;
    T /= 16;
    int P0 = (T % 4) * 4;
    T /= 4;
    int fac = (T & 1) ? -1 : 1;
    copy(parms,best,NPARMS);
    for(i = 0;i < 4;++i)
        parms[P0+i] += fac*R[R0+i];
    ++it;

    fclose(f);
    }

void init()
    {
    int x2 = 0;
    int i;
    for(i = 0;i < NPARMS;++i)
        {
        x2 += parms[i]*parms[i];
        }
    if(x2 == 0)
        {
        copy(parms,best,NPARMS);
        }
    }

int pcnt[4] = {0,0,0,0};

void printparms(int Nnodes)
    {
    int i;
    FILE * f = fopen("parms.txt","a");
    fprintf(f,"/* %d */\n",Nnodes);
    for(i = 0;i < NPARMS;++i)
        {
        fprintf(f,"%d",parms[i]);
        if(((i+1) % 4) == 0)
            {
            fprintf(f,"\t%d\n",pcnt[i >> 2]);
            pcnt[i >> 2] = 0;
            }
        else
            fprintf(f,"\t");
        }
    fclose(f);
    }

int comp_parms(const vertex * a,const vertex * b)
    {
    for(int o = 0;o < NPARMS;o += 4)
        {
        int A = parms[o]*a->R__R + parms[o+1]*a->W__R + parms[o+2]*a->R__W + parms[o+3]*a->W__W;
        int B = parms[o]*b->R__R + parms[o+1]*b->W__R + parms[o+2]*b->R__W + parms[o+3]*b->W__W;
        if(A != B)
            {
            ++pcnt[o >> 2]; // For counting the number of times the first, second, third or fourth condition has been used.
            // (Hypothesis: with parms as doubles the first condition is used and only in very special cases the second.
            //  Addendum: This hypothesis holds.)
            return A > B ? -1 : 1;
            }
        }
    return 0;
    }
#endif


struct funcstruct
    {
    bool compute_parms;
    char * number;
    char * name;
    int (*comp)(const vertex * a,const vertex * b);
    };

struct funcstruct funcstructs[] =
    {
#if _NA
        {false,"1","fairly_good",comp_fairly_good},
        {false,"2","even_better",comp_even_better},
        {false,"3","affiksFEW3",comp_affiksFEW3},
        {false,"4","affiksFEW",comp_affiksFEW},
        {false,"5","affiksFEW2",comp_affiksFEW2},
        {false,"6","fixNA",comp_fixNA},
        {false,"7","fruit",comp_fruit},
        {false,"8","ice",comp_ice},
        {false,"9","pisang",comp_pisang},
        {false,"10","kiwi",comp_kiwi},
        {false,"11","carrot",comp_carrot},
        {false,"12","peen",comp_peen},
        {false,"13","beet",comp_beet},
        {false,"14","sugar",comp_sugar},
        {false,"15","affiksFEW2org",comp_affiksFEW2org},
#endif
        {false,"16","honey",comp_honey},
        {false,"17","koud",comp_koud},
        {true,"18","parms",comp_parms},
        //makeaffix.exe mydata.txt 0 affixrules XX 123 parms
        {false,"19","parms0",comp_parms},
        {false,0,0,0}
    };


bool setCompetitionFunction(const char * functionname)
    {
    int i;
    for(i = 0;funcstructs[i].number;++i)
        if(!strcmp(functionname,funcstructs[i].number) || !strcmp(functionname,funcstructs[i].name))
            {
            comp = funcstructs[i].comp;
            compute_parms = funcstructs[i].compute_parms;
            return true;
            }
    return false;
    }

