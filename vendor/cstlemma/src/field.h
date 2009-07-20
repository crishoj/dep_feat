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
#ifndef FIELD_H
#define FIELD_H

#include "defines.h"

class field
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    protected:
        field * next;
    public:
        virtual const char * isA() = 0; //20080213
        field();
        virtual ~field()
            {
            delete next;
#ifdef COUNTOBJECTS
            --COUNT;
#endif
            };
        virtual char * read(char * kar,field *& nextfield) = 0; // return NULL : kar accepted 
        void addField(field * fld)
            {
            if(next)
                next->addField(fld);
            else
                next = fld;
            }
        virtual void add(char kar){}
        virtual 
#ifndef CONSTSTRCHR
            const 
#endif
            char * getString();
        virtual void reset(){if(next)next->reset();}
    };

class readValue : public field
    {
    private:
        char * word;
        int len;
        int pos;
    public:
        const char * isA() //20080213
            {
            return "Value";
            }
        readValue()
            {
            len = 20;
            word = new char[len];
            word[0] = '\0';
            pos = 0;
            }
        ~readValue()
            {
            delete word;
            }
        void reset()
            {
            pos = 0;
            field::reset();
            }
#ifndef CONSTSTRCHR
        const 
#endif
            char * getString()
            {
//            printf("%s\n",word);
            if(pos == 0)
                return "";
//                word[pos] = '\0';
            return word;
            }
        virtual char * read(char * kar,field *& nextfield);
    };

class readWhiteSpace : public field
    {
#ifdef COUNTOBJECTS
    public:
        static int COUNT;
#endif
    private:
        bool found;
    public:
        const char * isA() //20080213
            {
            return "WhiteSpace";
            }
        readWhiteSpace():found(false){}
        ~readWhiteSpace(){}
        virtual char * read(char * kar,field *& nextfield);
        void reset()
            {
            found = false;
            field::reset();
            }
    };

class readAllButWhiteSpace : public field
    {
    private:
        bool found;
    public:
        const char * isA() //20080213
            {
            return "AllButWhiteSpace";
            }
        readAllButWhiteSpace():found(false){}
        ~readAllButWhiteSpace(){}
        virtual char * read(char * kar,field *& nextfield);
        void reset()
            {
            found = false;
            field::reset();
            }
    };

class readTab : public field
    {
    public:
        const char * isA() //20080213
            {
            return "Tab";
            }
        readTab(){}
        ~readTab(){}
        virtual char * read(char * kar,field *& nextfield);
    };

class readNewLine : public field
    {
    public:
        const char * isA() //20080213
            {
            return "NewLine";
            }
        readNewLine(){}
        ~readNewLine(){}
        virtual char * read(char * kar,field *& nextfield);
    };

class readLitteral : public field
    {
    private:
        char * litteral;
        char * matched;
        char * giveback;
        int len;
        int pos;
        int givebacklen;
    public:
        const char * isA() //20080213
            {
            return "Litteral";
            }
        readLitteral(char first);
        ~readLitteral()
            {
            delete litteral;
            delete giveback;
            }
        void add(char kar);
        virtual char * read(char * kar,field *& nextfield);
        void reset()
            {
            pos = 0;
            field::reset();
            }
    };
#endif
