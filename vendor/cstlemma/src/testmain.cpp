// Only for some tests!

// 1: Case insensitive string comparison with full case folding
// 2: Find characters that change in UTF8-length when uppercased or lowercased

#include "utf8func.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#if UNICODE_CAPABLE
#include "letter.h"


int main(int argc,char **argv)
    {
    FILE * f = fopen("foldingLenghts.txt","wb");
    int cmp = strCaseCmp("MASSE","Maﬂe");
    fprintf(f,"MASSE %s Maﬂe\n",cmp == 0? "==" : cmp < 0 ? "<" : ">");
    for(int y=0;y < 0x11000;++y)
        {
        int L = lowerEquivalent(y);
        int U = upperEquivalent(y);
        if(L == y && U == y)
            continue;
        char buf1[5];
        char buf2[5];
        char buf3[5];
        buf1[UnicodeToUtf8(y,buf1,5)] = '\0';
        buf2[UnicodeToUtf8(L,buf2,5)] = '\0';
        buf3[UnicodeToUtf8(U,buf3,5)] = '\0';
        if(y < 0x80)
            {
            if(L >= 0x80)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"L%x>0x80  %x\n",L,y);
                }
            if(U >= 0x80)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"U%x>0x80  %x\n",U,y);
                }
            }
        else if(y < 0x800)
            {
            if(L >= 0x800)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"L%x>0x800  %x\n",L,y);
                }
            else if(L < 0x80)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"L%x<0x80  %x\n",L,y);
                }
            if(U >= 0x800)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"U%x>0x800  %x\n",U,y);
                }
            else if(U < 0x80)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"U%x<0x80  %x\n",U,y);
                }
            }
        else if(y < 0x10000)
            {
            if(L >= 0x10000)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"L%x>0x10000 %x\n",L,y);
                }
            else if(L < 0x800)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"L%x<0x800  %x\n",L,y);
                }
            if(U >= 0x10000)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"U%x>0x10000 %x\n",U,y);
                }
            else if(U < 0x800)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"U%x<0x800  %x\n",U,y);
                }
            }
        else
            {
            if(L < 0x10000)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"L%x<0x10000  %x\n",L,y);
                }
            if(U < 0x10000)
                {
                fprintf(f,"%s %s %s ",buf1,buf2,buf3);
                fprintf(f,"U%x<0x10000  %x\n",U,y);
                }
            }
        }
    fclose(f);
    return 0;
    }

#endif
