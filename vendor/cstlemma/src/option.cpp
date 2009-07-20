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

#include "option.h"
#include "freqfile.h"
#include "caseconv.h"
#include "argopt.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#ifdef COUNTOBJECTS
int optionStruct::COUNT = 0;
#endif

const char optionStruct::DefaultSep[] = "|";
//const char DefaultCFormat[] = "$w\\t[$b]1[$b0$B][$b>1$B]\\t$t\\n";
const char optionStruct::DefaultCFormat[] = "$w\\t$b1[[$b?]~1$B]\\t$t\\n";
//const char DefaultCFormat_NoTags[] = "$w\\t[$b]1[$b0$B][$b>1$B]\\n";
const char optionStruct::DefaultCFormat_NoTags[] = "$w\\t$b1[[$b?]~1$B]\\n";
const char optionStruct::Default_b_format[] = "$w";
const char * optionStruct::Default_B_format = optionStruct::Default_b_format;

static char opts[] = "?@:A:b:B:c:d:De:f:FH:hi:I:k:l:Lm:n:N:o:p:q:s:t:u:U:v:W:x:y:z:R:C:" /* GNU: */ "wr";
/*
static char ** poptions = NULL;
static char * options = NULL;
*/
static char *** Ppoptions = NULL;
static char ** Poptions = NULL;
static int optionSets = 0;

char * dupl(const char * s)
    {
    char * d = new char[strlen(s) + 1];
    strcpy(d,s);
    return d;
    }

optionStruct::optionStruct()
    {
    defaultbformat = true;
    defaultBformat = true;
    defaultCformat = true;
    dictfile = NULL;
    v = NULL;
    x = NULL;
    z = NULL;
    flx = NULL;
    InputHasTags = true;
    CollapseHomographs = true;
    keepPunctuation = 1;
    Sep = dupl(DefaultSep);
    whattodo = LEMMATISE;
    argi = NULL;
    argo = NULL;
    arge = NULL;
    cformat = dupl(DefaultCFormat);
    Wformat = NULL;
    bformat = dupl(Default_b_format);
    Bformat = dupl(Default_B_format);
    freq = NULL;
    nice = false;
    SortOutput = 0;
    RulesUnique = true;
    DictUnique = true;
    Iformat = NULL;
    UseLemmaFreqForDisambiguation = 0;
    baseformsAreLowercase = true;
    size = ULONG_MAX;
    treatSlashAsAlternativesSeparator = false;
#ifdef COUNTOBJECTS
    ++COUNT;
#endif
    showRefcount = false;
    CutoffRefcount = 0;
    }

optionStruct::~optionStruct()
    {
    for(int i = 0;i < optionSets;++i)
        {
        delete [] Poptions[i];
        delete [] Ppoptions[i];
        }
    delete [] Poptions;
    delete [] Ppoptions;
    delete [] cformat;
    delete [] bformat;
    delete [] Bformat;
    delete [] Wformat;
    delete [] Iformat;
    delete [] Sep;
#ifdef COUNTOBJECTS
    --COUNT;
#endif
    }

OptReturnTp optionStruct::doSwitch(int c,char * locoptarg,char * progname)
    {
    switch (c)
        {
// GNU >>
        case 'w':
            printf("11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n");
            printf("FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n");
            printf("OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n");
            printf("PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n");
            printf("OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n");
            printf("MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n");
            printf("TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n");
            printf("PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n");
            printf("REPAIR OR CORRECTION.\n");
            return Leave;
        case 'r':
            printf("12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n");
            printf("WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n");
            printf("REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n");
            printf("INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n");
            printf("OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n");
            printf("TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n");
            printf("YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n");
            printf("PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n");
            printf("POSSIBILITY OF SUCH DAMAGES.\n");
            return Leave;
// << GNU
        case '@':
            readOptsFromFile(locoptarg,progname);
            break;
        case 'A':
            if(locoptarg && *locoptarg == '-')
                {
                treatSlashAsAlternativesSeparator = false;
                }
            else
                {
                treatSlashAsAlternativesSeparator = true;
                }	    
            break;
        case 'D':
            whattodo = MAKEDICT;
            break;
        case 'F':
            whattodo = MAKEFLEXPATTERNS;
            break;
        case 'L':
            whattodo = LEMMATISE; // default action
            break;
/*            case 'a':
            if(locoptarg && *locoptarg == '-')
                OutputHasFullForm = false;
            else
                OutputHasFullForm = true;
            break;*/
        case 'b':
            bformat = dupl(locoptarg); 
            defaultbformat = false;
            break;
        case 'B':
            Bformat = dupl(locoptarg); 
            defaultBformat = false;
            break;
        case 'c':
            cformat = dupl(locoptarg);
            defaultCformat = false;
            break;
        case 'W':
            Wformat = dupl(locoptarg);
            break;
        case 'd':
            dictfile = locoptarg;
            break;
        case 'f':
            flx = locoptarg;
            break;
        case 'H':
            if(locoptarg)
                {
                UseLemmaFreqForDisambiguation = *locoptarg - '0';
                if(UseLemmaFreqForDisambiguation < 0 || UseLemmaFreqForDisambiguation > 2)
                    {
                    printf("-H option: specify -H0, -H1 or -H2 (found -H%s)\n",locoptarg);
                    return Error;
                    }
                }
            else
                {   
                printf("-H option: specify -H0, -H1 or -H2\n");
                return Error;
                }
            break;
        case 'h':
        case '?':
            printf("usage:\n");
            printf("============================\n");
            printf("    Create binary dictionary\n");
            printf("%s -D \\\n",progname);
            printf("         -c<format> [-N<frequency file> -n<format>] [-y[-]] \\\n"
                   "        [-i<lemmafile>] [-o<binarydictionary>]\n"
                   "    -c  column format of dictionary (tab separated), e.g. -cBFT, which means:\n"
                   "        1st column B(ase form), 2nd column F(ull form), 3rd column T(ype)\n"
                   "    -n  column format of frequency file (tab separated)\n"
                   "        Example: -nN?FT, which means:\n"
                   "        1st column N(frequency), 2nd column irrelevant,\n"
                   "        3rd column F(ull form), 4th column T(ype)\n"
                   "    -y  test output\n    -y- release output (default)\n"
                   "    -k  collapse homographs (remove \",n\" endings)(default)\n"
                   "    -k- do not collapse homographs (keep \",n\" endings)\n");
//                printf("--More--");getchar();
            printf("===============================\n");
            printf("    Create or add flex patterns\n");
            printf("%s -F \\\n",progname);
            printf("         -c<format> [-y[-]] [-i<lemmafile>] \\\n"
                   "        [-f<old flexpatterns>] [-o<new flexpatterns>]\n"
                   "    -c  column format, e.g. -cBFT, which means:\n"
                   "        1st column B(aseform), 2nd column F(ullform), 3rd column T(ype)\n"
                   "        For lemmatising untagged text, suppress lexical type information by\n"
                   "        specifying '?' for the column containing the type.\n"
                   "    -y  test output\n    -y- release output (default)\n");
            printf("    -R- Do not append refcount to base form (default)\n");// Bart 20050905
            printf("    -R  Append refcount to base form (format: [<base form>#<refcount>])\n");// Bart 20050905
            printf("    -C- Include all rules in output (default)\n");// Bart 20050905
            printf("    -C<n> Do not include rules with refcount <= <n>\n");// Bart 20050905
//            printf("--More--");getchar();
            printf("=============\n");
            printf("    Lemmatise\n");
//                printf("%s [-L] -c<format> -b<format> -B<format> [-s[<sep>]] [-u[-]] -d<binarydictionary> -f<flexpatterns> [-z<type conversion table>] [-i<input text>] [-o<output text>] [-m<conflicts>] [-n<newlemmas>] [-x<Lexical type translation table>]\n",argv[0]);
            printf("%s [-L] \\\n",progname);
            printf("         -f<flex patterns> [-d<binary dictionary>] [-u[-]] [-v[-]] \\\n"
                   "         [-I<input format>] [-i<input text>] [-o<output text>] \\\n"
                   "         [-c<format>] [-b<format>] [-B<format>] [-W<format>] [-s[<sep>]] \\\n"
                   "         [-x<Lexical type translation table>] [-v<tag friends file>] \\\n"
                   "         [-z<type conversion table>] [-@<option file>]\n");
            printf("    -i<input text>\tIf -t- defined: any flat text. Otherwise: words must be\n"
                   "        followed by tags, separated by '/'. Default: standard input.\n");  
            printf("    -I<format>\tInput format (if not word/tag (-t) or word (-t-)).\n" 
                   "        $w word to be lemmatised\n" 
                   "        $t tag\n" 
                   "        $d dummy\n" 
                   "        \\t tab\n" 
                   "        \\n new line\n" 
                   "        \\s white space\n" 
                   "        \\S all except white space\n"); 
            printf("    -o<output text>\tOutput format dependent on -b, -B, -c and -W arguments.\n"
                   "        Default output: standard output\n");  
            printf("    -d<binarydictionary>\tDictionary as produced with the -D option set.\n"  
                   "        If no dictionary is specified, only the flex patterns are used.\n"  
                   "        Without dictionary, wrong tags in the input can not be corrected.\n");  
            printf("    -f<flexpatterns>\tFile with flex patterns. (see -F). Best results for\n"
                   "        untagged input are obtained if the rules are made without lexical type\n"
                   "        information. See -c option above.\n");  
            printf("    -b<format string>\tdefault:" commandlineQuote "%s" commandlineQuote "\n",Default_b_format);  
            printf("        Output format for data pertaining to the base form, according to the\n"
                   "        dictionary:\n"
                   "        $f sum of frequencies of the words $W having the base form $w\n"
                   "           (lemmafrequency).\n");
            /*
            printf("        $f base form type or token frequency.\n");
            printf("           (The frequency of the base form type is given if you have\n");
            printf("            (a) specified $f in the -c<format> argument, or\n");
            printf("            (b) specified a -W<format> argument, or\n");
            printf("            (c) specified a -H0 or -H1 argument.\n");
            printf("            Otherwise, base form token frequency is given.)\n");
            */
#if FREQ24
            printf("        $n frequency of the full form $w/$t in \"standard\" corpus.\n");
#endif
//                printf("        $p probability of this lexical type (%%) = 100x$n/sum($n).\n");
            printf("        $t lexical type\n");
            printf("        $w base form\n");
            printf("        $W full form(s)\n");
            printf("        \\$ dollar\n");
            printf("        \\[ [\n");
            printf("        \\] ]\n");
            printf("        Example: -b" commandlineQuote "$f $w/$t" commandlineQuote "\n");
            printf("    -B<format string>\tdefault:" commandlineQuote "%s" commandlineQuote "\n",Default_B_format);  
            printf("        Output format for data pertaining to the base form, as predicted by\n");
            printf("        flex pattern rules. See -b\n");
//            printf("--More--");getchar();
            printf("    -W<format string>\tdefault: not present.\n");  
            printf("        Output format:\n");
            printf("        $w full form\n");
            printf("        $t lexical type(s) according to dictionary\n");
            printf("        $f full form type frequency\n");
            printf("        $i info:  -    full form not in dictionary\n");
            printf("                  +    full form in dictionary, but other type\n");
            printf("               (blank) full form in dictionary\n");
            printf("        \\t tab\n");
            printf("        $X?, [X]? Do not output X. (X can be tested, though).\n");
            printf("        [X]+  Output X only if X occurs at least once. (X is an expression\n");
            printf("              containing $b or $B)\n");
            printf("        [X]>n Output X only if X occurs more than n times.\n");
            printf("        [X]n  Output X only if X occurs exactly n times.\n");
            printf("        [X]<n Output X only if X occurs less than n times.\n");
            printf("        [X]   Output X if all nested conditions are met, or if X occurs\n");
            printf("              at least once. ([X] itself is always met!)\n");
            printf("        Example: -b" commandlineQuote "$w ($W)[>1[$W?]>1]" commandlineQuote "\n");
            printf("                 -W" commandlineQuote "$w\\n" commandlineQuote "\n");
            printf("                (Output lemma (full form|full form..)>1\n"
                   "                 if different words have same base form)\n");
//            printf("--More--");getchar();
            printf("    -c<format string>\tdefault:\t" commandlineQuote "%s" commandlineQuote "\n",DefaultCFormat);// word/lemma/tag lemma: if dictionary gives 1 solution, take dictionary, otherwise rules
            printf("        Output format:\n");
            printf("        $w full form\n");
            printf("        $b base form(s) according to dictionary.\n"
                   "           (You also need to specify -b<format>)\n"
                   "           (If the full form is found in the dictionary and tag=lexical type,\n"
                   "            then only one base form is output.\n"
                   "            Otherwise all base forms are output)\n");
            printf("        $B base form(s) according to flex pattern rules\n"
                   "           (You also need to specify -B<format>)\n"
                   "           (only if full form not in dictionary, or in dictionary,\n"
                   "            but with other lexical type.)\n");
            printf("        $s word separator: new line character when the current word is the last\n"
                   "           word before a line break, blank otherwise\n");
            printf("        $t lexical type(s) according to dictionary\n");
            printf("        $f full form frequency\n");
            printf("        $i info: indicates - full form not in dictionary\n");
            printf("                           + full form in dictionary, but other type\n");
            printf("                           * full form in dictionary\n");
            printf("        \\t tab\n");
            printf("        $X?, [X]? Do not output X. (X can be tested, though).\n");
            printf("        $b and $B are variables: they can occur any number of times,\n");
            printf("        including zero. This number can be tested in conditions:\n");
            printf("        $bn   Output $b only if $b occurs exactly n-times (n >= 0).\n");
            printf("        $Bn   Output $B only if $B occurs exactly n-times (n >= 0).\n");
            printf("        [X]+  Output X only if X occurs at least once. (X is an expression\n");
            printf("              containing $b or $B)\n");
            printf("        [X]>n Output X only if X occurs more than n times.\n");
            printf("        [X]n  Output X only if X occurs exactly n times.\n");
            printf("        [X]<n Output X only if X occurs less than n times.\n");
            printf("        [X]   Output X if all nested conditions are met, or if X occurs\n");
            printf("              at least once. ([X] itself is always met!)\n");
            printf("        Example: -c" commandlineQuote "[+$b?]>0[-$b0]$w\\n" commandlineQuote "\n");
            printf("                 -b" commandlineQuote "$w\t/$t" commandlineQuote "\n");
            printf("                (Output +lemma if the word is found in the dictionary,\n"
                   "                 otherwise -lemma)\n");
//            printf("--More--");getchar();
            printf("    -l  force lemma to all-lowercase (default)\n");
            printf("    -l- make case of lemma similar to full form's case\n");
            printf("    -p  keep punctuation (default)\n");
            printf("    -p- ignore punctuation (only together with -t- and no -W format)\n");
            printf("    -p+ treat punctuation as tokens (only together with -t- and no -W format)\n");
            printf("    -q  sort output\n");
            printf("    -q- do not sort output (default)\n");
            printf("    -q# sort output by frequency\n");
            printf("    -s<sep> multiple base forms (-b -B) are <sep>-separated. Example: -s" commandlineQuote " | " commandlineQuote "\n");
            printf("    -s  multiple base forms (-b -B) are " commandlineQuote "%s" commandlineQuote "-separated (default)\n",DefaultSep);
            printf("    -t  input text is tagged (default)\n    -t- input text is not tagged\n");
            printf("    -U  enforce unique flex rules (default)\n");
            printf("    -U- allow ambiguous flex rules\n");
            printf("    -u  enforce unique dictionary look-up (default)\n");
            printf("    -u- allow ambiguous dictionary look-up\n");
            printf("    -Hn n = 0: use lemma frequencies for disambiguation (default)\n");
            printf("        n = 1: use lemma frequencies for disambiguation,\n");
            printf("               show candidates for pruning between << and >>\n");
            printf("        n = 2: do not use lemma frequencies for disambiguation.\n");
            printf("    -v<tag friends file>: Use this to coerce the nearest fit between input\n"
                   "        tag and the dictionary's lexical types if the dictionary has more than\n"
                   "        one readings of the input word and none of these has a lexical type\n"
                   "        that exactly agrees with the input tag. Format:\n"
                   "             {<dict type> {<space> <tag>}* <newline>}*\n"
                   "        The more to the left the tag is, the better the agreement with the\n"
                   "        dictionary'e lexical type\n");
            printf("    -x<Lexical type translation table>: Use this to handle tagged texts with\n"
                   "        tags that do not occur in the dictionary. Format:\n"
                   "             {<dict type> {<space> <tag>}* <newline>}*\n");
            printf("    -z<type conversion table>: Use this to change the meaning of $t in -b and\n"
                   "        -B formats. Without conversion table, $t is the lexical type of the\n"
                   "        full form. With conversion table, $t is the lexical type of the base\n"
                   "        form, as defined by the table. Format:\n"
                   "             {<base form type> <space> <full form type> <newline>}*\n");
            printf("    -m<size>: Max. number of words in input. Default: 0 (meaning: unlimited)\n");
            printf("    -A  Treat / as separator between alternative words.\n"); // Bart 20030108
            printf("    -A- Do not treat / as separator between alternative words (default)\n");// Bart 20030108
            printf("    -e<n> ISO8859 Character encoding. 'n' is one of 1,2,7 and 9 (ISO8859-1,2, etc).\n");// Bart 20080219
            printf("    -eU Unicode (UTF8) input.\n");// Bart 20081106
            printf("    -e  Don't use case conversion.\n");// Bart 20080219
            return Leave;
        case 'e':
            arge = locoptarg;
            switch(*arge)
                {
                case '0':
                case '1':
                case '2':
                case '7':
                case '9':
                    setEncoding(*arge - '0');
                    break;
                case 'u':
                case 'U':
                    setEncoding(ENUNICODE);
                    break;
                }
            break;
        case 'i':
            argi = locoptarg;
            break;
        case 'I':
            Iformat = dupl(locoptarg); 
            break;
            /*
        case 'm': // file containing conflicts
            argm = locoptarg;
            break;
        case 'n': // file containing new words
            argn = locoptarg;
            break;
            */
        case 'k':
            if(locoptarg && *locoptarg == '-')
                {
                CollapseHomographs = false;
                }
            else
                {
                CollapseHomographs = true;
                }
            break;
        case 'l':
            if(locoptarg && *locoptarg == '-')
                {
                baseformsAreLowercase = false;
                }
            else
                {
                baseformsAreLowercase = true;
                }
            break;
            
        case 'm':
            if(locoptarg)
                {
                size = strtoul(locoptarg,NULL,10);
                printf("size %lu\n",size);
                if(size == 0)
                    size = ULONG_MAX;
                printf("size %lu\n",size);
                }
            else
                size = ULONG_MAX;
            break;
        case 'n':
//Bart 20021223            if(freq)
                {
                if(!freq)
                    {
                    freq = new FreqFile();
                    }
                (freq)->addFormat(locoptarg);
                }
            break;
        case 'N':
//Bart 20021223            if(freq)
                {
                if(!freq)
                    {
                    freq = new FreqFile();
                    }
                (freq)->addName(locoptarg);
                }
            break;
        case 'o':
            argo = locoptarg;
            break;
        case 'p':
            if(locoptarg)
                {
                if(*locoptarg == '-')
                    {
                    keepPunctuation = 0;
                    }
                else if(*locoptarg == '+')
                    {
                    keepPunctuation = 2;
                    }
                else if(*locoptarg == '\0')
                    {
                    keepPunctuation = 1;
                    }
                else
                    {
                    printf("Invalid argument %s for -p option.\n",locoptarg);
                    return Error;
                    }
                }
            else
                {
                keepPunctuation = 1;
                }
            break;
        case 'q':
            if(!locoptarg)
                locoptarg = "w#";
            else if(*locoptarg == '-')
                {
                SortOutput = 0;
                break;
                }

            SortOutput = 0;
            while(*locoptarg)
                {
                SortOutput <<= 2;
                switch(*locoptarg)
                    {
                    case '#':
                    case 'f':
                    case 'F':
                    case 'n':
                    case 'N':
                        SortOutput += SORTFREQ;
                        break;
                    case 'l':
                    case 'L':
                    case 'w':
                    case 'W':
                        SortOutput += SORTWORD;
                        break;
                    case 'p':
                    case 'P':
                    case 't':
                    case 'T':
                        SortOutput += SORTPOS;
                        break;
                    default:
                        SortOutput = SORTWORD;
                        break;
                    }
                ++locoptarg;
                }
            break;
        case 's':
            if(locoptarg && *locoptarg)
                {
                for(char * p = locoptarg;*p;)
                    {
                    if(*p == '\\')
                        {
                        switch(*(p + 1))
                            {
                            case 't':
                                *p++ = '\t';
                                memmove(p,p+1,strlen(p));
                                break;
                            case 'n':
                                *p++ = '\n';
                                memmove(p,p+1,strlen(p));
                                break;
                            default:
                                *p = *(p+1);
                                ++p;
                                memmove(p,p+1,strlen(p));
                                break;
                            }
                        }
                    else
                        ++p;
                    }
                Sep = dupl(locoptarg);
                }
            else
                Sep = dupl(DefaultSep);
            break;
        case 't':
            if(locoptarg && *locoptarg == '-')
                InputHasTags = false;
            else
                InputHasTags = true;
            break;
        case 'U':
            if(locoptarg && *locoptarg == '-')
                RulesUnique = false;
            else
                RulesUnique = true;
            break;
        case 'u':
            if(locoptarg && *locoptarg == '-')
                DictUnique = false;
            else
                DictUnique = true;
            break;
        case 'v':
            v = locoptarg;
            break;
        case 'x':
            x = locoptarg;
            break;
        case 'y':
            if(locoptarg && *locoptarg == '-')
                nice = false;
            else
                nice = true;
            break;
        case 'R':
            if(locoptarg && *locoptarg == '-')
                showRefcount = false;
            else
                showRefcount = true;
            break;
        case 'C':
            if(locoptarg && *locoptarg == '-')
                CutoffRefcount = 0;
            else
                CutoffRefcount = strtol(locoptarg,NULL,10);
            break;
        case 'z':
            z = locoptarg;
            break;
        }
    return GoOn;
    }



OptReturnTp optionStruct::readOptsFromFile(char * locoptarg,char * progname)
    {
    char ** poptions;
    char * options;
    FILE * fpopt = fopen(locoptarg,"r");
    OptReturnTp result = GoOn;
    if(fpopt)
        {
        char * p;
        char line[1000];
        int lineno = 0;
        int bufsize = 0;
        while(fgets(line,sizeof(line) - 1,fpopt))
            {
            lineno++;
            int off = strspn(line," \t");
            if(line[off] == ';')
                continue; // comment line
            if(line[off] == '-')
                {
                off++;
                if(line[off])
                    {
                    char * optarg2 = line + off + 1;
                    int off2 = strspn(optarg2," \t");
                    if(!optarg2[off2])
                        optarg2 = NULL;
                    else
                        optarg2 += off2;
                    if(optarg2)
                        {
                        for(p = optarg2 + strlen(optarg2) - 1;p >= optarg2;--p)
                            {
                            if(!isspace(*p))
                                break;
                            *p = '\0';
                            }
                        bool string = false;
                        if(*optarg2 == '\'' || *optarg2 == '"')
                            {

                            // -x 'jhgfjhagj asdfj\' hsdjfk' ; dfaasdhfg
                            // -x 'jhgfjhagj asdfj\' hsdjfk' ; dfa ' asdhfg
                            // -x "jhgfjhagj \"asdfj hsdjfk" ; dfaasdhfg
                            // -x "jhgfjhagj \"asdfj hsdjfk" ; dfa " asdhfg
                            for(p = optarg2 + strlen(optarg2) - 1;p > optarg2;--p)
                                {
                                if(*p == *optarg2)
                                    {
                                    string = true;
                                    for(char * q = p + 1;*q;++q)
                                        {
                                        if(*q == ';')
                                            break;
                                        if(!isspace(*q))
                                            {
                                            string = false;
                                            }
                                        }
                                    if(string)
                                        {
                                        *p = '\0';
                                        ++optarg2;
                                        }
                                    break;
                                    }
                                }
                            }
                        if(!*optarg2 && !string)
                            optarg2 = NULL;
                        }
                    if(optarg2)
                        {
                        bufsize += strlen(optarg2) + 1;
                        }
                    char * optpos = strchr(opts,line[off]);
                    if(optpos)
                        {
                        if(optpos[1] != ':')
                            {
                            if(optarg2)
                                {
                                printf("Option argument %s provided for option letter %c that doesn't use it on line %d in option file \"%s\"\n",optarg2,line[off],lineno,locoptarg);
                                exit(1);
                                }
                            }
                        }
                    }
                else
                    {
                    printf("Missing option letter on line %d in option file \"%s\"\n",lineno,locoptarg);
                    exit(1);
                    }
                }
            }
        rewind(fpopt);

        poptions = new char * [lineno];
        options = new char[bufsize];
        // update stacks that keep pointers to the allocated arrays.
        optionSets++;
        char *** tmpPpoptions = new char **[optionSets];
        char ** tmpPoptions = new char *[optionSets];
        int g;
        for(g = 0;g < optionSets - 1;++g)
            {
            tmpPpoptions[g] = Ppoptions[g];
            tmpPoptions[g] = Poptions[g];
            }
        tmpPpoptions[g] = poptions;
        tmpPoptions[g] = options;
        delete [] Ppoptions;
        Ppoptions = tmpPpoptions;
        delete [] Poptions;
        Poptions = tmpPoptions;

        lineno = 0;
        bufsize = 0;
        while(fgets(line,sizeof(line) - 1,fpopt))
            {
            poptions[lineno] = options+bufsize;
            int off = strspn(line," \t");
            if(line[off] == ';')
                continue; // comment line
            if(line[off] == '-')
                {
                off++;
                if(line[off])
                    {
                    char * optarg2 = line + off + 1;
                    int off2 = strspn(optarg2," \t");
                    if(!optarg2[off2])
                        optarg2 = NULL;
                    else
                        optarg2 += off2;
                    if(optarg2)
                        {
                        for(p = optarg2 + strlen(optarg2) - 1;p >= optarg2;--p)
                            {
                            if(!isspace(*p))
                                break;
                            *p = '\0';
                            }
                        bool string = false;
                        if(*optarg2 == '\'' || *optarg2 == '"')
                            {

                            // -x 'jhgfjhagj asdfj\' hsdjfk' ; dfaasdhfg
                            // -x 'jhgfjhagj asdfj\' hsdjfk' ; dfa ' asdhfg
                            // -x "jhgfjhagj \"asdfj hsdjfk" ; dfaasdhfg
                            // -x "jhgfjhagj \"asdfj hsdjfk" ; dfa " asdhfg
                            for(p = optarg2 + strlen(optarg2) - 1;p > optarg2;--p)
                                {
                                if(*p == *optarg2)
                                    {
                                    string = true;
                                    for(char * q = p + 1;*q;++q)
                                        {
                                        if(*q == ';')
                                            break;
                                        if(!isspace(*q))
                                            {
                                            string = false;
                                            }
                                        }
                                    if(string)
                                        {
                                        *p = '\0';
                                        ++optarg2;
                                        }
                                    break;
                                    }
                                }
                            }
                        if(!*optarg2 && /*Bart 20030905: allow empty string for e.g. -s option*/!string)
                            optarg2 = NULL;
                        }
                    if(optarg2)
                        {
                        strcpy(poptions[lineno],optarg2);
                        bufsize += strlen(optarg2) + 1;
                        }
                    /*else
                        optarg2 = "";
                    char * optpos = strchr(opts,line[off]);*/
                    OptReturnTp res = doSwitch(line[off],poptions[lineno],progname);
                    if(res > result)
                        result = res;
                    }
                }
            lineno++;
            }
        fclose(fpopt);
        }
    else
        {
        printf("Cannot open option file %s\n",locoptarg);
        }
    return result;
    }

OptReturnTp optionStruct::readArgs(int argc, char * argv[])
    {
    int c;
    SortOutput = 0;
    Wformat = NULL;
    OptReturnTp result = GoOn;
    while((c = getopt(argc,argv, opts)) != -1)
        {
        OptReturnTp res = doSwitch(c,optarg,argv[0]);
        if(res > result)
            result = res;
        }
    if(this->arge == NULL)
        {
        setEncoding(0);
        }
    return result;
    }

void optionStruct::setIformat(const char * format)  // -I
    {
    delete [] Iformat;
    Iformat = dupl(format);
    }

void optionStruct::setBformat(const char * format)  // -B
    {
    delete [] Bformat;
    Bformat = dupl(format);
    defaultBformat = format != Default_B_format;
    }

void optionStruct::setbformat(const char * format)  // -b
    {
    delete [] bformat;
    bformat = dupl(format);
    defaultbformat = format != Default_b_format;
    }

void optionStruct::setcformat(const char * format)  // -c
    {
    delete [] cformat;
    cformat = dupl(format);
    defaultCformat = format != DefaultCFormat && format != DefaultCFormat_NoTags;
    }

void optionStruct::setWformat(const char * format)  // -W
    {
    delete [] Wformat;
    Wformat = dupl(format);
    }

void optionStruct::setSep(const char * format)      // -s
    {
    delete [] Sep;
    Sep = dupl(format);
    }


void optionStruct::setUseLemmaFreqForDisambiguation(int n)    // -H 0, 1 or 2
    {
    assert(0 <= n && n <= 2);
    UseLemmaFreqForDisambiguation = n;
    }

void optionStruct::setkeepPunctuation(bool b)
    {
    keepPunctuation = b;
    }

void optionStruct::setsize(unsigned long int n)
    {
    size = n;
    }

void optionStruct::settreatSlashAsAlternativesSeparator(bool b)
    {
    treatSlashAsAlternativesSeparator = b;
    }

void optionStruct::setUseLemmaFreqForDisambiguation(bool b)
    {
    UseLemmaFreqForDisambiguation = b;
    }

void optionStruct::setDictUnique(bool b)
    {
    DictUnique = b;
    }

void optionStruct::setbaseformsAreLowercase(bool b)
    {
    baseformsAreLowercase = b;
    }

