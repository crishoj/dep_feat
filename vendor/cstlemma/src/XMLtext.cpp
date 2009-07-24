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

/*

XML
option X has been extended
word, POS, lemma and lemma class can all be found in attribute values. 
Words can also be found in elements.

*/

#include "XMLtext.h"
#include "word.h"
#include "field.h"
#include "sgmltag.h"
#include "basefrm.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>

// -Xaw -Xllemma -Xpana -I"$w\s" -L -e1 -f flexrules.utf8 -t -p -l- -c"$B" -B"$w" -i text-context.xml -o text-context.lemma.xml


static void printXML(
#if STREAM
                     ostream * fpo
#else
                     FILE * fpo
#endif
                     ,const char * s)
    {
#if STREAM
    while(*s)
        {
        switch(*s)
            {
            case '<':
                *basefrm::m_fp << "&lt;";
                break;
            case '>':
                *basefrm::m_fp << "&gt;";
                break;
            case '&':
                if(strchr(s+1,';'))
                    *basefrm::m_fp << '&';
                else
                    *basefrm::m_fp << "&amp;";
                break;
            case '\'':
                *basefrm::m_fp << "&apos;";
                break;
            case '"':
                *basefrm::m_fp << "&quot;";
                break;
            default:
                *basefrm::m_fp << *s;
            }
        ++s;
        }
#else
    while(*s)
        {
        switch(*s)
            {
            case '<':
                fprintf(basefrm::m_fp,"&lt;");
                break;
            case '>':
                fprintf(basefrm::m_fp,"&gt;");
                break;
            case '&':
                if(strchr(s+1,';'))
                    fputc('&',basefrm::m_fp);
                else
                    fprintf(basefrm::m_fp,"&amp;");
                break;
            case '\'':
                fprintf(basefrm::m_fp,"&apos;");
                break;
            case '"':
                fprintf(basefrm::m_fp,"&quot;");
                break;
            default:
                fputc(*s,basefrm::m_fp);
            }
        ++s;
        }
#endif
    }


static const char * 
#if STREAM
copy(ostream * fp,const char * o,const char * end)
    {
    for(;o < end;++o)
        *fp << *o;
    return o;
    }
#else
copy(FILE * fp,const char * o,const char * end)
    {
    for(;o < end;++o)
        fputc(*o,fp);
    return o;
    }
#endif


class substring
    {
    private:
        char * start;
        char * end;
    public:
        substring():start(NULL),end(NULL)
            {
            }
        void set(char * start,char * end)
            {
            this->start = start;
            this->end = end;
            }
        char * getStart() const
            {
            return start;
            }
        char * getEnd() const
            {
            return end;
            }
    };

class token
    {
    public:
        substring Word;
        substring POS;
        substring Lemma;
        substring LemmaClass;
    };

class crumb
    {
    private:
        char * e;
        int L;
        crumb * next;
    public:
        crumb(const char * s,int len,crumb * Crumbs):L(len),next(Crumbs)
            {
            e = new char[len+1];
            strncpy(e,s,len);
            e[len] = '\0';
            }
        ~crumb()
            {
            delete [] e;
            }
        const char * itsElement()
            {
            return e;
            }
        crumb * pop(const char * until,int len)
            {
            crumb * nxt = next;
            if(L == len && !strncmp(e,until,len))
                {
                delete this;
                return nxt;
                }
            else
                {
                delete this;
                return nxt ? nxt->pop(until,len) : NULL;
                }
            }
        void print()
            {
            if(next)
                next->print();
            printf("->%s",e);
            }
        bool is(const char * elm)
            {
            return !strcmp(e,elm);
            }
    };

bool XMLtext::analyseThis()
    {
    if(element)
        {
        return Crumbs && Crumbs->is(element);
        }
    else
        {
        if(ancestor)
            {
            return Crumbs != NULL;
            }
        else
            {
            return true;
            }
        }
/*
    bool h;
    if(element)
        {
        h = Crumbs && Crumbs->is(element);
        }
    else
        {
        if(ancestor)
            {
            h = Crumbs;
            }
        else
            {
            h = true;
            }
        }
    if(h)
        {
        printf("analyse\n");
        }
    return h;
    */
    }


const char * XMLtext::convert(const char * s)
    {
    const char * p = strchr(s,'&');
    if(p && strchr(p+1,';'))
        {
        static char * ret[2] = {NULL,NULL};
        static size_t len[2] = {0,0};
        static int index = 0;
        index += 1;
        index %= 2;
        if(len[index] <= strlen(s))
            {
            delete ret[index];
            len[index] = strlen(s)+1;
            ret[index] = new char[len[index]];
            }
        char *p = ret[index];
        while(*s)
            {
            if(*s == '&')
                {
                ++s;
                if(!strncmp(s,"lt;",3))
                    {
                    *p++ = '<';
                    s += 3;
                    }
                else if(!strncmp(s,"gt;",3))
                    {
                    *p++ = '>';
                    s += 3;
                    }
                else if(!strncmp(s,"amp;",4))
                    {
                    *p++ = '&';
                    s += 4;
                    }
                else if(!strncmp(s,"apos;",5))
                    {
                    *p++ = '\'';
                    s += 5;
                    }
                else if(!strncmp(s,"quot;",5))
                    {
                    *p++ = '"';
                    s += 5;
                    }
                else
                    *p++ = '&';
                }
            else
                *p++ = *s++;
            }
        *p = '\0';
        return ret[index];
        }
    return s;
    }


void XMLtext::CallBackStartElementName()
    {
    startElement = ch;
    }

void XMLtext::CallBackEndElementName()
    {
    endElement = ch;
    //printf("element(%s):%.*s\n",ClosingTag ? "closing" : "",endElement - startElement,startElement);
    if(ClosingTag)
        {
        if(Crumbs)
            Crumbs = Crumbs->pop(startElement,endElement - startElement);
        ClosingTag = false;
        }
    else
        {
        if(  Crumbs
          || !this->ancestor
          ||    startElement + strlen(this->ancestor) == endElement 
             && !strncmp(startElement,this->ancestor,endElement - startElement)
          )
            Crumbs = new crumb(startElement,endElement - startElement,Crumbs);
        }
/*    if(Crumbs)
        {
        Crumbs->print();
        printf("\n");
        }*/
    }

void XMLtext::CallBackStartAttributeName()
    {
    if(analyseThis())
        {
        ClosingTag = false;
        startAttributeName = ch;
        }
    }

void XMLtext::CallBackEndAttributeNameCounting()
    {
    if(analyseThis())
        {
        endAttributeName = ch;
        if(  this->wordAttributeLen == endAttributeName - startAttributeName 
            && !strncmp(startAttributeName,this->wordAttribute,wordAttributeLen )
            )
            {
            ++total;
            }
        }
    }

void XMLtext::CallBackEndAttributeNameInserting()
    {
    if(analyseThis())
        {
        endAttributeName = ch;
//        printf("attribu:%.*s\n",endAttributeName - startAttributeName,startAttributeName);

        WordPosComing = false;
        POSPosComing = false;
        LemmaPosComing = false;
        LemmaClassPosComing = false;

        if(  this->wordAttributeLen == endAttributeName - startAttributeName 
            && !strncmp(startAttributeName,this->wordAttribute,wordAttributeLen )
            )
            {
            WordPosComing = true;
            }
        else if(  this->POSAttributeLen == endAttributeName - startAttributeName 
            && !strncmp(startAttributeName,this->POSAttribute,POSAttributeLen )
            )
            {
            POSPosComing = true;
            }
        else if(  this->lemmaAttributeLen == endAttributeName - startAttributeName 
            && !strncmp(startAttributeName,this->lemmaAttribute,lemmaAttributeLen)
            )
            {
            LemmaPosComing = true;
            }
        else if(  this->lemmaClassAttributeLen == endAttributeName - startAttributeName 
            && !strncmp(startAttributeName,this->lemmaClassAttribute,lemmaClassAttributeLen)
            )
            {
            LemmaClassPosComing = true;
            }
        }
    }

void XMLtext::CallBackStartValue()
    {
    if(analyseThis())
        {
        startValue = ch;
        }
    }

void XMLtext::CallBackEndValue()
    {
    if(analyseThis())
        {
        endValue = ch;            
//        printf("value  :%.*s\n",endValue - startValue,startValue);
        if(WordPosComing)
            {
            this->Token[total].Word.set(startValue,endValue);
            }
        else if(POSPosComing)
            {
            this->Token[total].POS.set(startValue,endValue);
            }
        else if(LemmaPosComing)
            {
            this->Token[total].Lemma.set(startValue,endValue);
            }
        else if(LemmaClassPosComing)
            {
            this->Token[total].LemmaClass.set(startValue,endValue);
            }
        }
    }

void XMLtext::CallBackNoMoreAttributes()
    {
    if(analyseThis())
        {
        token * Tok = Token + total;
        if(Tok->Word.getStart() != NULL && Tok->Word.getEnd() != Tok->Word.getStart())
            { // Word as attribute
            char * EW = Tok->Word.getEnd();
            char savW = *EW;
            *EW = '\0';
            if(Tok->POS.getStart() != NULL)
                {
                char * EP = Tok->POS.getEnd();
                char savP = *EP;
                *EP = '\0';
                insert(Tok->Word.getStart(),Tok->POS.getStart());
                *EP = savP;
                }
            else
                {
                insert(Tok->Word.getStart());
                }
            *EW = savW;
            }
        }
    }

void XMLtext::CallBackEndTag()
    {
    ClosingTag = true;
    }

void XMLtext::CallBackEmptyTag()
    {
    if(Crumbs)
        Crumbs = Crumbs->pop(startElement,endElement - startElement);
    /*if(Crumbs)
        {
        Crumbs->print();
        printf("\n");
        }*/
    }


class wordReader
    {
    private:
        field * format;
        field * nextfield;
        field * wordfield;
        field * tagfield;
        unsigned long newlines;
        unsigned long lineno;
        const char * tag;
        int kar;
        int lastkar;
        bool treatSlashAsAlternativesSeparator;
        XMLtext * Text;
        char kars[2];
    public:
        unsigned long getNewlines()
            {
            return newlines;
            }
        const char * getTag()
            {
            return tag;
            }
        unsigned long getLineno()
            {
            return lineno;
            }
        void initWord()
            {
            nextfield = format;
            newlines = 0;
            format->reset();
            kars[1] = '\0';
            if(lastkar)
                {
                if(lastkar == '\n')
                    ++newlines;
                kars[0] = (char)lastkar;
                lastkar = 0;
                nextfield->read(kars,nextfield);
                }
            token * Tok = Text->getCurrentToken();
            if(Tok)
                {
                if(kars[0])
                    Tok->Word.set(Text->ch-1,Text->ch-1);
                else
                    Tok->Word.set(Text->ch,Text->ch);
                }
            }
        wordReader(field * format,field * wordfield,field * tagfield,bool treatSlashAsAlternativesSeparator,XMLtext * Text)
            :format(format)
            ,nextfield(format)
            ,wordfield(wordfield)
            ,tagfield(tagfield)
            ,newlines(0)
            ,lineno(0)
            ,tag(NULL)
            ,kar(EOF)
            ,lastkar(0)
            ,treatSlashAsAlternativesSeparator(treatSlashAsAlternativesSeparator)
            ,Text(Text)
            {
            initWord();
            assert(wordfield);
            if(lastkar)
                {
                assert(lastkar != EOF);
                if(lastkar == '\n')
                    ++newlines;
                kars[0] = (char)lastkar;
                lastkar = 0;
                nextfield->read(kars,nextfield);
                }
            }
#ifndef CONSTSTRCHR
        const 
#endif
        char * readChar(int kar)
            {
            if(!nextfield)
                {
                initWord();
                }
            if(kar == '\n')
                ++newlines;
            kars[0] = kar == EOF ? '\0' : (char)kar;
            char * plastkar = nextfield->read(kars,nextfield);
            if(kar == '\0')
                lastkar = '\0';
            else if(plastkar)
                lastkar = *plastkar;

            if(nextfield)
                return NULL;
            else
                {
                lineno += newlines;
                if(tagfield)
                    tag = tagfield->getString();
                return wordfield->getString();
                }
            }
#ifndef CONSTSTRCHR
        const 
#endif
        char * count(int kar)
            {
#ifndef CONSTSTRCHR
            const 
#endif
            char * w = readChar(kar);
            if(Text->analyseThis())
                {
                if(!Text->wordAttribute && w && *w)
                    {
                    Text->incTotal();
                    if(treatSlashAsAlternativesSeparator)
                        {
                        Text->incTotal(findSlashes(w));
                        }
                    }
                }
            return w;
            }
#ifndef CONSTSTRCHR
        const 
#endif
        char * read(int kar)
            {
#ifndef CONSTSTRCHR
            const 
#endif
            char * w = readChar(kar);
            if(Text->analyseThis())
                {
                if(!Text->wordAttribute && w && *w)
                    {
                    token * Tok = Text->getCurrentToken();
                    char * start = Tok->Word.getStart();
                    Tok->Word.set(start,start+strlen(w));
                    char * POS = Tok->POS.getStart();
                    if(POS != NULL)
                        {
                        char * EP = Tok->POS.getEnd();
                        char savP = *EP;
                        *EP = '\0';
                        if(treatSlashAsAlternativesSeparator && findSlashes(w))
                            Text->createTaggedAlternatives(w,POS);
                        else
                            Text->createTagged(w,POS);
                        *EP = savP;
                        }
                    else if(tag)
                        {
                        if(treatSlashAsAlternativesSeparator && findSlashes(w))
                            Text->createTaggedAlternatives(w,tag);
                        else
                            Text->createTagged(w,tag);
                        }
                    else
                        {
                        if(treatSlashAsAlternativesSeparator && findSlashes(w))
                            Text->createUnTaggedAlternatives(w);
                        else
                            Text->createUnTagged(w);
                        }
                    }
                }
            return w;
            }
    };

void XMLtext::printUnsorted(
#if STREAM
                            ostream * fpo
#else
                            FILE * fpo            
#endif
                            )
    {
    if(this->alltext)
        {
        unsigned int k;
        const char * o = this->alltext;
        for(k = 0;k < total;++k)
            {
            if(tunsorted[k])
                {
                const char * start = Token[k].Word.getStart();
                const char * end = Token[k].Word.getEnd();
                const char * lemmastart = Token[k].Lemma.getStart();
                const char * lemmaend = Token[k].Lemma.getEnd();
                const char * lemmaclassstart = Token[k].LemmaClass.getStart();
                const char * lemmaclassend = Token[k].LemmaClass.getEnd();
                if(lemmastart == NULL)
                    { // replace word with lemma or write lemma together with word
                    if(lemmaclassstart)
                        {
                        copy(fpo,o,lemmaclassstart);
                        tunsorted[k]->printLemmaClass();
                        o = lemmaclassend;
                        }
                    if(start)
                        {
                        copy(fpo,o,start);
                        o = end;
                        }
                    tunsorted[k]->print();
                    }
                else
                    {
                    if(lemmaclassstart == NULL)
                        {
                        copy(fpo,o,lemmastart);
                        o = lemmaend;
                        tunsorted[k]->print();
                        }
                    else if(lemmaclassstart < lemmastart)
                        {
                        copy(fpo,o,lemmaclassstart);
                        tunsorted[k]->printLemmaClass();
                        copy(fpo,lemmaclassend,lemmastart);
                        tunsorted[k]->print();
                        o = lemmaend;
                        }
                    else
                        {
                        copy(fpo,o,lemmastart);
                        tunsorted[k]->print();
                        copy(fpo,lemmaend,lemmaclassstart);
                        tunsorted[k]->printLemmaClass();
                        o = lemmaclassend;
                        }
                    }
                }
            }
        o = copy(fpo,o,o+strlen(o));
        }
    }

token * XMLtext::getCurrentToken()
    {
    return Token ? Token + total : NULL;
    }

void CallBackStartElementName(void *arg)
    {
    ((XMLtext *)arg)->CallBackStartElementName();
    }

void CallBackEndElementName(void *arg)
    {
    ((XMLtext *)arg)->CallBackEndElementName();
    }

void CallBackStartAttributeName(void *arg)
    {
    ((XMLtext *)arg)->CallBackStartAttributeName();
    }

static void (XMLtext::*CallBackEndAttributeNames)();

void CallBackEndAttributeName(void *arg)
    {
    (((XMLtext *)arg)->*CallBackEndAttributeNames)();
    }

void CallBackStartValue(void *arg)
    {
    ((XMLtext *)arg)->CallBackStartValue();
    }

void CallBackEndValue(void *arg)
    {
    ((XMLtext *)arg)->CallBackEndValue();
    }

void CallBackNoMoreAttributes(void *arg)
    {
    ((XMLtext *)arg)->CallBackNoMoreAttributes();
    }

void CallBackEndTag(void * arg)
    {
    ((XMLtext *)arg)->CallBackEndTag();
    }

void CallBackEmptyTag(void * arg)
    {
    ((XMLtext *)arg)->CallBackEmptyTag();
    }

#if STREAM
XMLtext::XMLtext(istream * fpi,bool a_InputHasTags,char * Iformat,int keepPunctuation,bool nice,
           unsigned long int size,bool treatSlashAsAlternativesSeparator,bool XML
           ,const char * ancestor // if not null, restrict lemmatisation to elements that are offspring of ancestor
           ,const char * element // if null, analyse all PCDATA that is text
           ,const char * wordAttribute // if null, word is PCDATA
           ,const char * POSAttribute // if null, POS is PCDATA
           ,const char * lemmaAttribute // if null, Lemma is PCDATA
           ,const char * lemmaClassAttribute // if null, lemma class is PCDATA
           )
#else
XMLtext::XMLtext(FILE * fpi,bool a_InputHasTags,char * Iformat,int keepPunctuation,bool nice,
           unsigned long int size,bool treatSlashAsAlternativesSeparator,bool XML
           ,const char * ancestor // if not null, restrict lemmatisation to elements that are offspring of ancestor
           ,const char * element // if null, analyse all PCDATA that is text
           ,const char * wordAttribute // if null, word is PCDATA
           ,const char * POSAttribute // if null, POS is PCDATA
           ,const char * lemmaAttribute // if null, Lemma is PCDATA
           ,const char * lemmaClassAttribute // if null, lemma class is PCDATA
           )
#endif
           :text(/*fpi,*/a_InputHasTags/*,Iformat*/,keepPunctuation,nice/*,size,treatSlashAsAlternativesSeparator*/)
           ,Token(NULL)
           ,ancestor(ancestor)
           ,element(element)
           ,POSAttribute(POSAttribute)
           ,lemmaAttribute(lemmaAttribute)
           ,lemmaClassAttribute(lemmaClassAttribute)
           ,Crumbs(NULL)
           ,ClosingTag(false)
           ,WordPosComing(false)
           ,POSPosComing(false)
           ,LemmaPosComing(false)
           ,LemmaClassPosComing(false)
           ,alltext(NULL)
           ,wordAttribute(wordAttribute)
    {
    wordAttributeLen = wordAttribute ? strlen(wordAttribute) : 0;
    POSAttributeLen = POSAttribute ? strlen(POSAttribute) : 0;
    lemmaAttributeLen = lemmaAttribute ? strlen(lemmaAttribute) : 0;
    lemmaClassAttributeLen = lemmaClassAttribute ? strlen(lemmaClassAttribute) : 0;
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    //fields = 0;
    reducedtotal = 0;
    Root = 0;
//    basefrmarrD = 0;
//    basefrmarrL = 0;

#ifndef CONSTSTRCHR
//    const 
#endif
//        char * w;
    //?int total = 0;
//    const char * Tag;
    field * wordfield;
    field * tagfield;
    field * format = 0;
    //int slashFound = 0;
    if(nice)
        printf("counting words\n");
    lineno = 0;
//    unsigned long newlines;
    
    if(XML && Iformat)
        {
#if STREAM
        fpi->clear();
        fpi->seekg(0, ios::end);
        int kar;
        streamoff filesize = fpi->tellg();
        fpi->clear();
        fpi->seekg(0, ios::beg);
        if(filesize > 0)
            {
            alltext = new char[filesize+1];
            char * p = alltext;
            while((kar = fpi->get()) && !fpi->eof())
                *p++ = kar;
            *p = '\0';
            fpi->clear();
            fpi->seekg(0, ios::beg);
            }
#else
        fseek(fpi,0,SEEK_END);
        int kar;
        long filesize = ftell(fpi);
        rewind(fpi);
        if(filesize > 0)
            {
            alltext = new char[filesize+1];
            char * p = alltext;
            while((kar = getc(fpi)) != EOF)
                *p++ = (char)kar;
            *p = '\0';
            rewind(fpi);
            }
#endif
        }
    if(alltext)
        {
        html_tag_class html_tag(this);
        CallBackEndAttributeNames = &XMLtext::CallBackEndAttributeNameCounting;
        if(element||ancestor)
            {
            html_tag.setCallBackStartElementName(::CallBackStartElementName);
            html_tag.setCallBackEndElementName(::CallBackEndElementName);
            html_tag.setCallBackEndTag(::CallBackEndTag);
            html_tag.setCallBackEmptyTag(::CallBackEmptyTag);
            }
        else
            {
            html_tag.setCallBackStartElementName(::dummyCallBack);
            html_tag.setCallBackEndElementName(::dummyCallBack);
            html_tag.setCallBackEndTag(::dummyCallBack);
            html_tag.setCallBackEmptyTag(::dummyCallBack);
            }
        if(wordAttribute||lemmaAttribute||POSAttribute||lemmaClassAttribute)
            {
            html_tag.setCallBackStartAttributeName(::CallBackStartAttributeName);
            html_tag.setCallBackEndAttributeName(::CallBackEndAttributeName);
            print = printXML;
            }
        else
            {
            html_tag.setCallBackStartAttributeName(::dummyCallBack);
            html_tag.setCallBackEndAttributeName(::dummyCallBack);
            }
        html_tag.setCallBackStartValue(::dummyCallBack);
        html_tag.setCallBackEndValue(::dummyCallBack);
        html_tag.setCallBackNoMoreAttributes(::dummyCallBack);
        ch = alltext;
        char * curr_pos = alltext;
        char * endpos = alltext;
        estate Seq = notag;
#ifndef CONSTSTRCHR
        const 
#endif
        char * (wordReader::*fnc)(int kar);
        int loop;
        fnc = &wordReader::count;
        if(Iformat)
            {
            wordfield = 0;
            tagfield = 0;
            format = translateFormat(Iformat,wordfield,tagfield);
            if(!wordfield)
                {
                printf("Input format %s must specify '$w'.\n",Iformat);
                exit(0);
                }
            }
        for(loop = 1;loop <= 2;++loop)
            {
            wordReader WordReader(format,wordfield,tagfield,treatSlashAsAlternativesSeparator,this);
            while(*ch)
                {
                while(  *ch 
                     && (( Seq = (html_tag.*tagState)(*ch)) == tag 
                        || Seq == endoftag_startoftag
                        )
                     )
                    {
                    ch++;
                    }
                if(Seq == notag)
                    { // Not an HTML tag. Backtrack.
                    endpos = ch;
                    ch = curr_pos;
                    //printf("NOTAG:");  // TEXT
                    while(ch < endpos)
                        {
                        (WordReader.*fnc)(*ch);
                        //putchar(*ch);  // TEXT
                        ch++;
                        }
                    }
                else if(Seq == endoftag)
                    ++ch; // skip >
                if(*ch)
                    {
//                    printf("[");  // TEXT START
                    while(  *ch 
                        && (Seq = (html_tag.*tagState)(*ch)) == notag
                        )
                        {
                        (WordReader.*fnc)(*ch);
                        //putchar(*ch); // TEXT
                        ++ch;
                        }
                    (WordReader.*fnc)('\0');
                    if(Seq == tag)
                        {
                        (WordReader.*fnc)('\0');
                        curr_pos = ch++; // skip <
                        //printf("]");  // TEXT END
                        }
                    }
                }
            if(Seq == tag)
                { // Not an SGML tag. Backtrack.
                endpos = ch;
                ch = curr_pos;
                //printf("NOTAG:");  // TEXT
                while(ch < endpos)
                    {
                    (WordReader.*fnc)(*ch);
                    //putchar(*ch); // TEXT
                    ch++;
                    }
                }
            if(loop == 1)
                {
                lineno = WordReader.getLineno();
                if(nice)
                    {
                    printf("... %lu words counted in %lu lines\n",total,lineno);
                    printf("allocating array of pointers to words\n");
                    }
                tunsorted =  new const Word * [total];
                Token = new token[total + 1]; // 1 extra for the epilogue after the last token
                if(nice)
                    printf("allocating array of line offsets\n");
                Lines =  new unsigned long int [lineno+1];
                for(int L = lineno+1;--L >= 0;)
                    Lines[L] = 0;
                if(nice)
                    printf("...allocated array\n");
                /** /
                FILE * ft = fopen("wordlist.txt","wb");
                / **/

                total = 0;
                if(nice)
                    printf("reading words\n");
                lineno = 0;
                ch = alltext;
                fnc = &wordReader::read;
                CallBackEndAttributeNames = &XMLtext::CallBackEndAttributeNameInserting;
                if(wordAttribute||POSAttribute||lemmaAttribute||lemmaClassAttribute)
                    {
                    html_tag.setCallBackStartAttributeName(::CallBackStartAttributeName);
                    html_tag.setCallBackEndAttributeName(::CallBackEndAttributeName);
                    html_tag.setCallBackStartValue(::CallBackStartValue);
                    html_tag.setCallBackEndValue(::CallBackEndValue);
                    html_tag.setCallBackNoMoreAttributes(::CallBackNoMoreAttributes);
                    }
                }
            }
        }
    makeList();
    /*
    Root = Hash->convertToList(N);
    delete Hash;
    */
    //        qsort(Root,N,sizeof(Word *),cmpUntagged);
    if(nice)
        printf("...read words from XML file\n");
    /** /
    for(unsigned int g = 0;g < total;++g)
        {
        if(tunsorted[g] && tunsorted[g]->m_word)
            fprintf(ft,"%s\n",tunsorted[g]->m_word);
        }
    fclose(ft);
    / **/
    sorted = false;
    }

