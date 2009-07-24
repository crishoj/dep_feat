#ifndef SGMLTAG_H
#define SGMLTAG_H

#include <wchar.h>

class html_tag_class;

typedef enum {notag,tag,endoftag,endoftag_startoftag} estate;
//typedef estate (html_tag_class::*t_state)(int kar);
extern estate (html_tag_class::*tagState)(wint_t kar);
extern void dummyCallBack(void *);
class html_tag_class
    {
    private:
        void * arg;
        void (*CallBackStartElementName)(void *);
        void (*CallBackEndElementName)(void *);
        void (*CallBackStartAttributeName)(void *);
        void (*CallBackEndAttributeName)(void *);
        void (*CallBackStartValue)(void *);
        void (*CallBackEndValue)(void *);
        void (*CallBackEndTag)(void *);
        void (*CallBackEmptyTag)(void *);
        void (*CallBackNoMoreAttributes)(void *);
    public:
        estate def(wint_t kar);
        estate lt(wint_t kar);
        estate element(wint_t kar);
        estate elementonly(wint_t kar);
        estate gt(wint_t kar);
        estate emptytag(wint_t kar);
        estate atts(wint_t kar);
        estate name(wint_t kar);
        estate value(wint_t kar);
        estate atts_or_value(wint_t kar);
        estate invalue(wint_t kar);
        estate insinglequotedvalue(wint_t kar);
        estate indoublequotedvalue(wint_t kar);
        estate singlequotes(wint_t kar);
        estate doublequotes(wint_t kar);
        estate endvalue(wint_t kar);
        estate markup(wint_t kar);
        estate script(wint_t kar);
        estate h1(wint_t kar);
        estate h2(wint_t kar);
        estate h3(wint_t kar);
        estate CDATA1(wint_t kar);
        estate CDATA2(wint_t kar);
        estate CDATA3(wint_t kar);
        estate CDATA4(wint_t kar);
        estate CDATA5(wint_t kar);
        estate CDATA6(wint_t kar);
        estate CDATA7(wint_t kar);
        estate CDATA8(wint_t kar);
        estate CDATA9(wint_t kar);
        estate endtag(wint_t kar);
        html_tag_class(void * arg):arg(arg)
            {
            CallBackStartElementName = CallBackEndElementName = 
            CallBackStartAttributeName = CallBackEndAttributeName = 
            CallBackStartValue = CallBackEndValue = dummyCallBack;
            tagState = &html_tag_class::def;
            }
        void setCallBackStartElementName(void (*f)(void *))
            {
            CallBackStartElementName = f;
            }
        void setCallBackEndElementName(void (*f)(void *))
            {
            CallBackEndElementName = f;
            }
        void setCallBackStartAttributeName(void (*f)(void *))
            {
            CallBackStartAttributeName = f;
            }
        void setCallBackEndAttributeName(void (*f)(void *))
            {
            CallBackEndAttributeName = f;
            }
        void setCallBackStartValue(void (*f)(void *))
            {
            CallBackStartValue = f;
            }
        void setCallBackEndValue(void (*f)(void *))
            {
            CallBackEndValue = f;
            }
        void setCallBackEndTag(void (*f)(void *))
            {
            CallBackEndTag = f;
            }
        void setCallBackEmptyTag(void (*f)(void *))
            {
            CallBackEmptyTag = f;
            }
        void setCallBackNoMoreAttributes(void (*f)(void *))
            {
            CallBackNoMoreAttributes = f;
            }
    };

#endif
