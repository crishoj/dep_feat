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
#ifndef XMLTEXT_H
#define XMLTEXT_H

#include "text.h"

class token;
class crumb;

class XMLtext : public text
    {
    private:
        token * Token;
        char * startElement;
        char * endElement;
        char * startAttributeName;
        char * endAttributeName;
        char * startValue;
        char * endValue;
        const char * ancestor; // if not null, restrict lemmatisation to elements that are offspring of ancestor
        const char * element; // if not null, analyse only element's attributes and/or PCDATA
        const char * POSAttribute; // if null, POS is PCDATA
        const char * lemmaAttribute; // if null, Lemma is PCDATA
        const char * lemmaClassAttribute; // if null, lemma class is PCDATA
        int wordAttributeLen;
        int POSAttributeLen;
        int lemmaAttributeLen;
        int lemmaClassAttributeLen;
        crumb * Crumbs;
        bool ClosingTag;
        bool WordPosComing;
        bool POSPosComing;
        bool LemmaPosComing;
        bool LemmaClassPosComing;
        char * alltext;
        virtual const char * convert(const char * s);
    public:
        char * ch;
        const char * wordAttribute; // if null, word is PCDATA
    public:
        bool analyseThis();
        token * getCurrentToken();
        void CallBackStartElementName();
        void CallBackEndElementName();
        void CallBackStartAttributeName();
        void CallBackEndAttributeNameInserting();
        void CallBackEndAttributeNameCounting();
        void CallBackStartValue();
        void CallBackEndValue();
        void CallBackEndTag();
        void CallBackEmptyTag();
        void CallBackNoMoreAttributes();
        XMLtext(
#if STREAM
            istream * fpi
#else
            FILE * fpi
#endif
            ,bool InputHasTags
            ,char * Iformat
            ,int keepPunctuation
            ,bool nice
            ,unsigned long int size
            ,bool treatSlashAsAlternativeSeparator
            ,bool XML
            ,const char * ancestor // if not null, restrict lemmatisation to elements that are offspring of ancestor
            ,const char * element // if null, analyse all PCDATA that is text
            ,const char * wordAttribute // if null, word is PCDATA
            ,const char * POSAttribute // if null, POS is PCDATA
            ,const char * lemmaAttribute // if null, Lemma is PCDATA
            ,const char * lemmaClassAttribute // if null, lemma class is PCDATA
            );
        ~XMLtext(){}
        virtual void printUnsorted(
#if STREAM
            ostream * fpo
#else
            FILE * fpo            
#endif
            );
    };

#endif
