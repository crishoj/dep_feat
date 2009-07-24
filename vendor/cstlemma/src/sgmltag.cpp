#include "sgmltag.h"

void dummyCallBack(void *){}

estate (html_tag_class::*tagState)(wint_t kar);

estate html_tag_class::def(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            return tag;
        default:
            return notag;
        }
    }
estate html_tag_class::lt(wint_t kar)
    {
    switch(kar)
        {
        case '>':
            tagState = &html_tag_class::def;
            return notag;
        case '!':
            tagState = &html_tag_class::markup;
            return tag;
        case '?':
            tagState = &html_tag_class::script;
            return tag;
        case '/':
            CallBackEndTag(arg);
            tagState = &html_tag_class::endtag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return tag;
        default:
            if('A' <= kar && kar <= 'Z' || 'a' <= kar && kar <= 'z' || (kar & 0x80))
                {
                tagState = &html_tag_class::element;
                CallBackStartElementName(arg);
                return tag;
                }
            else
                {
                tagState = &html_tag_class::def;
                return notag;
                }
        }
    }

estate html_tag_class::element(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackEndElementName(arg);
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackEndElementName(arg);
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case '-':
        case '_':
        case ':':
        case '.':
            return tag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            CallBackEndElementName(arg);
            tagState = &html_tag_class::atts;
            return tag;
        case '/':
            CallBackEndElementName(arg);
            CallBackNoMoreAttributes(arg);
            CallBackEmptyTag(arg);
            tagState = &html_tag_class::emptytag;
            return tag;
        default:
            if('0' <= kar && kar <= '9' || 'A' <= kar && kar <= 'Z' || 'a' <= kar && kar <= 'z' || (kar & 0x80))
                return tag;
            else
                {
                tagState = &html_tag_class::def;
                return notag;
                }
        }
    }

estate html_tag_class::elementonly(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackEndElementName(arg);
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackEndElementName(arg);
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case '-':
        case '_':
        case ':':
        case '.':
            return tag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            CallBackEndElementName(arg);
            tagState = &html_tag_class::gt;
            return tag;
        default:
            if('0' <= kar && kar <= '9' || 'A' <= kar && kar <= 'Z' || 'a' <= kar && kar <= 'z' || (kar & 0x80))
                return tag;
            else
                {
                tagState = &html_tag_class::def;
                return notag;
                }
        }
    }

estate html_tag_class::gt(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return tag;
        default:
            tagState = &html_tag_class::def;
            return notag;
        }
    }

estate html_tag_class::emptytag(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            //CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            //CallBackNoMoreAttributes(arg);
            return endoftag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return tag;
        default:
            tagState = &html_tag_class::def;
            return notag;
        }
    }

estate html_tag_class::atts(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return tag;
        case '/':
            CallBackNoMoreAttributes(arg);
            CallBackEmptyTag(arg);
            tagState = &html_tag_class::emptytag;
            return tag;
        default:
            if('A' <= kar && kar <= 'Z' || 'a' <= kar && kar <= 'z' || (kar & 0x80))
                {
                CallBackStartAttributeName(arg);
                tagState = &html_tag_class::name;
                return tag;
                }
            else
                {
                tagState = &html_tag_class::def;
                return notag;
                }
        }
    }

estate html_tag_class::name(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackEndAttributeName(arg);
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackEndAttributeName(arg);
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case '-':
        case '_':
        case ':':
        case '.':
            return tag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            CallBackEndAttributeName(arg);
            tagState = &html_tag_class::atts_or_value;
            return tag;
        case '/':
            CallBackEndAttributeName(arg);
            CallBackEmptyTag(arg);
            tagState = &html_tag_class::emptytag;
            return tag;
        case '=':
            CallBackEndAttributeName(arg);
            tagState = &html_tag_class::value;
            return tag;
        default:
            if('0' <= kar && kar <= '9' || 'A' <= kar && kar <= 'Z' || 'a' <= kar && kar <= 'z' || (kar & 0x80))
                return tag;
            else
                {
                tagState = &html_tag_class::def;
                return notag;
                }
        }
    }

estate html_tag_class::value(wint_t kar)
    {
    switch(kar)
        {
        case '>':
        case '/':
        case '=':
            tagState = &html_tag_class::def;
            return notag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return tag;
        case '\'':
            tagState = &html_tag_class::singlequotes;
            return tag;
        case '"':
            tagState = &html_tag_class::doublequotes;
            return tag;
        /*case '-':
        case '_':
        case ':':
        case '.':
            tagState = &html_tag_class::invalue; 
            return tag;*/
        default:
            if('0' <= kar && kar <= '9' || 'A' <= kar && kar <= 'Z' || 'a' <= kar && kar <= 'z' || (kar & 0x80))
                {
                CallBackStartValue(arg);
                tagState = &html_tag_class::invalue;
                return tag;
                }
            else
                {
                tagState = &html_tag_class::def;
                return notag;
                }
        }
    }

estate html_tag_class::atts_or_value(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case '-':
        case '_':
        case ':':
        case '.':
            tagState = &html_tag_class::def;
            return notag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return tag;
        case '/':
            CallBackNoMoreAttributes(arg);
            CallBackEmptyTag(arg);
            tagState = &html_tag_class::emptytag;
            return tag;
        case '=':
            tagState = &html_tag_class::value;
            return tag;
        default:
            if('A' <= kar && kar <= 'Z' || 'a' <= kar && kar <= 'z' || (kar & 0x80))
                {
                CallBackStartAttributeName(arg);
                tagState = &html_tag_class::name;
                return tag;
                }
            else
                {
                tagState = &html_tag_class::def;
                return notag;
                }
        }
    }

estate html_tag_class::invalue(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackEndValue(arg);
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackEndValue(arg);
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            CallBackEndValue(arg);
            tagState = &html_tag_class::atts;
            return tag;
        default:
            if('0' <= kar && kar <= '9' || 'A' <= kar && kar <= 'Z' || 'a' <= kar && kar <= 'z' || (kar & 0x80))
                {
                return tag;
                }
            else
                {
                tagState = &html_tag_class::def;
                return notag;
                }
        }
    }

estate html_tag_class::singlequotes(wint_t kar)
    {
    CallBackStartValue(arg);
    switch(kar)
        {
        case '\'':
            CallBackEndValue(arg);
            tagState = &html_tag_class::endvalue;
            return tag;
        default:
            tagState = &html_tag_class::insinglequotedvalue;
            return tag;
        }
    }

estate html_tag_class::doublequotes(wint_t kar)
    {
    CallBackStartValue(arg);
    switch(kar)
        {
        case '\"':
            CallBackEndValue(arg);
            tagState = &html_tag_class::endvalue;
            return tag;
        default:
            tagState = &html_tag_class::indoublequotedvalue;
            return tag;
        }
    }

estate html_tag_class::insinglequotedvalue(wint_t kar)
    {
    switch(kar)
        {
        case '\'':
            CallBackEndValue(arg);
            tagState = &html_tag_class::endvalue;
            return tag;
        default:
            return tag;
        }
    }

estate html_tag_class::indoublequotedvalue(wint_t kar)
    {
    switch(kar)
        {
        case '\"':
            CallBackEndValue(arg);
            tagState = &html_tag_class::endvalue;
            return tag;
        default:
            return tag;
        }
    }


estate html_tag_class::endvalue(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            tagState = &html_tag_class::atts;
            return tag;
        case '/': // 20090708
            CallBackNoMoreAttributes(arg);
            CallBackEmptyTag(arg);
            tagState = &html_tag_class::emptytag;
            return tag;
        default:
            tagState = &html_tag_class::def;
            return notag;
        }
    }

estate html_tag_class::markup(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case '-':
            tagState = &html_tag_class::h1;
            return tag;
        case '[':
            tagState = &html_tag_class::CDATA1;
            return tag;
        default:
            return tag;
        }
    }

estate html_tag_class::script(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case '?':
//            CallBackEndTag(arg);
            tagState = &html_tag_class::endtag;
            return tag;
        default:
            return tag;
        }
    }

estate html_tag_class::CDATA1(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case 'C':
            tagState = &html_tag_class::CDATA2;
            return tag;
        default:
            tagState = &html_tag_class::markup;
            return tag;
        }
    }

estate html_tag_class::CDATA2(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            return endoftag_startoftag;
            //CallBackNoMoreAttributes(arg);
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case 'D':
            tagState = &html_tag_class::CDATA3;
            return tag;
        default:
            tagState = &html_tag_class::markup;
            return tag;
        }
    }

estate html_tag_class::CDATA3(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case 'A':
            tagState = &html_tag_class::CDATA4;
            return tag;
        default:
            tagState = &html_tag_class::markup;
            return tag;
        }
    }

estate html_tag_class::CDATA4(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case 'T':
            tagState = &html_tag_class::CDATA5;
            return tag;
        default:
            tagState = &html_tag_class::markup;
            return tag;
        }
    }

estate html_tag_class::CDATA5(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case 'A':
            tagState = &html_tag_class::CDATA6;
            return tag;
        default:
            tagState = &html_tag_class::markup;
            return tag;
        }
    }

estate html_tag_class::CDATA6(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case '[':
            tagState = &html_tag_class::CDATA7;
            return tag;
        default:
            tagState = &html_tag_class::markup;
            return tag;
        }
    }

estate html_tag_class::CDATA7(wint_t kar)
    {
    switch(kar)
        {
        case ']':
            tagState = &html_tag_class::CDATA8;
            return tag;
        default:
            return tag;
        }
    }

estate html_tag_class::CDATA8(wint_t kar)
    {
    switch(kar)
        {
        case ']':
            tagState = &html_tag_class::CDATA9;
            return tag;
        default:
            return tag;
        }
    }

estate html_tag_class::CDATA9(wint_t kar)
    {
    switch(kar)
        {
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        default:
            tagState = &html_tag_class::CDATA7;
            return tag;
        }
    }


estate html_tag_class::h1(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            return notag;
        case '>':
            tagState = &html_tag_class::def;
            return notag;
        case '-':
            tagState = &html_tag_class::h2;
            return tag;
        default:
            tagState = &html_tag_class::markup;
            return tag;
        }
    }

estate html_tag_class::h2(wint_t kar)
    {
    switch(kar)
        {
        case '-':
            tagState = &html_tag_class::h3;
            return tag;
        default:
            return tag;
        }
    }

estate html_tag_class::h3(wint_t kar)
    {
    switch(kar)
        {
        case '-':
            tagState = &html_tag_class::markup;
            return tag;
        default:
            tagState = &html_tag_class::h2;
            return tag;
        }
    }

estate html_tag_class::endtag(wint_t kar)
    {
    switch(kar)
        {
        case '<':
            tagState = &html_tag_class::lt;
            CallBackNoMoreAttributes(arg);
            return endoftag_startoftag;
        case '>':
            tagState = &html_tag_class::def;
            CallBackNoMoreAttributes(arg);
            return endoftag;
        case 0xA0:
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return tag;
        default:
            if('A' <= kar && kar <= 'Z' || 'a' <= kar && kar <= 'z' || (kar & 0x80))
                {
                tagState = &html_tag_class::elementonly;
                CallBackStartElementName(arg);
                return tag;
                }
            else
                {
                tagState = &html_tag_class::def;
                return notag;
                }
        }
    }
