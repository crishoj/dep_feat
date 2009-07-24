#include "utf16func.h"
#include <string.h>
#if UNICODE_CAPABLE
#include "letter.h"

/*
int strCaseCmp(const wchar_t *s, const char *p)
    {
    wcharfolded S(s);
    charfolded P(p);
    int iS,iP;
    while(true)
        {
        iS = S.C();
        iP = P.C();
        if(iS)
            {
            if(iP)
                {
                if(iS != iP)
                    return iS - iP;
                }
            else
                return 1;
            }
        else
            {
            if(iP)
                return -1;
            else
                return 0;
            }
        }
    }
*/

#endif
