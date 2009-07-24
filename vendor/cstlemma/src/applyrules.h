/*
CSTLEMMA - trainable lemmatiser using word-end inflectional rules

Copyright (C) 2002, 2008  Center for Sprogteknologi, University of Copenhagen

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
#ifndef APPLYRULES_H
#define APPLYRULES_H

#include <stdio.h>

bool newStyleRules();
bool readRules(FILE * flexrulefile,const char * flexFileName); // 2nd argument added 20081107
//bool readRules(FILE * flexrulefile);
bool readRules(const char * flexFileName); // 20081107
//bool readRules(const char * filename);
const char * applyRules(const char * word);
const char * applyRules(const char * word,const char * tag);
void deleteRules();
extern bool oneAnswer;

#endif
