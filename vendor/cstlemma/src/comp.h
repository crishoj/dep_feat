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
#ifndef COMP_H
#define COMP_H

class vertex;
/*
extern int comp_fairly_good(const vertex * a,const vertex * b);
extern int comp_even_better(const vertex * a,const vertex * b);
extern int comp_affiksFEW3(const vertex * a,const vertex * b);
extern int comp_affiksFEW(const vertex * a,const vertex * b);
extern int comp_affiksFEW2(const vertex * a,const vertex * b);
extern int comp_affiksFEW2org(const vertex * a,const vertex * b);
extern int comp_fixNA(const vertex * a,const vertex * b);
extern int comp_fruit(const vertex * a,const vertex * b);
extern int comp_ice(const vertex * a,const vertex * b);
extern int comp_pisang(const vertex * a,const vertex * b);
extern int comp_kiwi(const vertex * a,const vertex * b);
extern int comp_carrot(const vertex * a,const vertex * b);
extern int comp_peen(const vertex * a,const vertex * b);
extern int comp_sugar(const vertex * a,const vertex * b);
extern int comp_honey(const vertex * a,const vertex * b);
extern int comp_beet(const vertex * a,const vertex * b);
*/
extern int comp_koud(const vertex * a,const vertex * b);
/*
extern int comp_parms(const vertex * a,const vertex * b);
*/
extern bool compute_parms;

bool setCompetitionFunction(const char * functionname);
void init();
void brown(int iteration);
void printparms(int Nnodes);
void betterfound(int Nnodes);

extern int (*comp)(const vertex * a,const vertex * b);

#endif
