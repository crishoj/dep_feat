#include <stdio.h>
#include <string.h>
#include <assert.h>


void Strrev(char * s)
    {
    char * e = s + strlen(s);
    while(s < --e)
        {
        char t = *s;
        *s++ = *e;
        *e = t;
        }
    }


FILE * fm = NULL;

static void printpat(char ** fields,int findex,char * start,char * end)
    {
    sprintf(start+strlen(start),"%.*s",fields[1] - fields[0] - 1,fields[0]);
    fprintf(fm,"%s",start);
    for(int m = 1;2*m+3 < findex;++m)
        {
        int M = 2*m+3;
        fprintf(fm,"*%.*s",fields[M] - fields[M-1] - 1,fields[M-1]);//,vars[m].e - vars[m].s,vars[m].s);
        }
    if(findex > 2)
        {
        Strrev(end);
        int L = strlen(end);
        sprintf(end+L,"%.*s",fields[3] - fields[2] - 1,fields[2]);
        Strrev(end+L);
        Strrev(end);
        fprintf(fm,"*%s\t-->\t",end);
        /*Strrev(end);
        end[L] = 0;
        Strrev(end);*/
        }
    else
        fprintf(fm,"%s\t-->\t",end);

    fprintf(fm,"%.*s",fields[2] - fields[1] - 1,fields[1]);
    for(int m = 1;2*m+3 < findex;++m)
        {
        int M = 2*m+3;
        fprintf(fm,"*%.*s",fields[M+1] - fields[M] - 1,fields[M]);//,vars[m].e - vars[m].s,vars[m].s);
        }
    if(findex > 2)
        {
        fprintf(fm,"*%.*s\n",fields[4] - fields[3] - 1,fields[3]);
        }
    else
        fprintf(fm,"\n");
    }

static char * printrules(char * rules,int indent,char * max,char * start,char * end);

static char * printlist(char * p,char * e,int indent,char * start,char * end)
    {
    int stlen = strlen(start);
    int endlen = strlen(end);
    while(p < e)
        {
        p = printrules(p,indent+1,e,start,end);
        start[stlen] = '\0';
        Strrev(end);
        end[endlen] = 0;
        Strrev(end);
        }
    return e;
    }

static char * printrules(char * rules,int indent,char * max,char * start,char * end)
    {
    char * fields[44];
    int index = *(int *)rules;
    if(index == 0)
        index = max - rules;
    char * p = rules + sizeof(int);
    unsigned int byt = *p;
    int slen = strlen(start);
    int elen = strlen(end);
    if(byt < 4)
        {
        // fork
        fprintf(fm,"%*s",2+indent*2,"(\n");
        char * ret = "";
        p += sizeof(int); // skip word containing 1, 2 or 3
        start[slen] = 0;
        end[elen] = 0;
        switch(byt)
            {
            case 1:
                ret = printlist(p,max,indent,start,end);
                fprintf(fm,"%*s|\n%*s(parent)\n",2+indent*2,"",2+indent*2,"");
                break;
            case 2:
                fprintf(fm,"%*s(parent)\n%*s|\n",2+indent*2,"",2+indent*2,"");
                ret = printlist(p,max,indent,start,end);
                break;
            case 3:
                {
                char * next = p + *(int *)p;
                p += sizeof(int);
                printlist(p,next,indent,start,end);
                start[slen] = 0;
                end[elen] = 0;
                fprintf(fm,"%*s",2+indent*2,"|\n");
                ret = printlist(next,max,indent,start,end);
                break;
                }
            }
        fprintf(fm,"%*s",2+indent*2,")\n");
        return ret;
        }
    fields[0] = p;
    int findex = 1;
    while(*p != '\n')
        {
        if(*p == '\t')
            fields[findex++] = ++p;
        else
            ++p;
        }
    fields[findex] = ++p; // p is now within 3 bytes from the next record.
    fprintf(fm,"%*s",indent*2,"");
    printpat(fields,findex,start,end);

    /*start[slen] = 0;
    end[elen] = 0;*/
    

    long nxt = p - rules;
    nxt += sizeof(int) - 1;
    nxt /= sizeof(int);
    nxt *= sizeof(int);
    p = rules+nxt;
    return printlist(p,rules+index,indent,start,end);
    }

bool readRules(FILE * flexrulefile,char *& buf,int & end)
    {
    fseek(flexrulefile,0,SEEK_END);
    end = ftell(flexrulefile);
    buf = new char[end];
    rewind(flexrulefile);
    fread(buf,1,end,flexrulefile);
    fclose(flexrulefile);
    return true;
    }


bool readRules(const char * filename,char *& buf,int & end)
    {
    FILE * f = fopen(filename,"rb");
    if(f)
        {
        return readRules(f,buf,end);
        }
    return false;
    }

void parse(char * buf,char * len,int & sibling,char *& p,char *& e,char *& child/*,char *& field_1,char *& field_3*/,char *& mmax)
    {
    assert(len > buf);
    sibling = *(int *)buf;
    p = buf + sizeof(int);
    e = p;
    //int t = 0;
    while(*e != '\n')
        {
        /*if(*e == '\t')
            {
            ++t;
            if(t == 2)
                field_1 = ++e;
            else if(t == 4)
                field_3 = ++e;
            else
                ++e;
            }
        else*/
            ++e;
        }
    int ichild = e - buf;
    ichild += sizeof(int);
    ichild /= sizeof(int);
    ichild *= sizeof(int);
    child = buf + ichild;
    mmax = sibling ? buf + sibling : len;
    }

int merge(char * arr, 
           char * buf1,
           char * len1,
           char * buf2,
           char * len2,
           char * p,
           int indent
           )
    {
    int sibling1;
    int sibling2;
    char * p1;
    char * p2;
    char * e1;
    char * e2;
    char * child1;
    char * child2;
    /*    
    char * field_11;
    char * field_31;
    char * field_12;
    char * field_32;
    */
    char * mmax1;
    char * mmax2;

    parse(buf1,len1,sibling1,p1,e1,child1,/*field_11,field_31,*/mmax1);
    parse(buf2,len2,sibling2,p2,e2,child2,/*field_12,field_32,*/mmax2);

    char * k;
    int ind = 0;
//    printf("\n[%.*s] =?= [%.*s]\n",e1 - p1,p1,e2 - p2,p2);
    if(  e1 - p1 == e2 - p2 
      && !strncmp(p1,p2,e1-p1)
      )
        { // equal nodes
        // do children
        for(k = buf1;k < child1;++k)
            arr[ind++] = *k;
        //printf("\n%*s%.*s",indent,"",e1-p1,p1);
        char * arrind = arr+ind;
        *(int *)(arrind) = 0xBABE;// test
        if(child1 < mmax1)
            {
            if(child2 < mmax2)
                { // each of the two nodes has at least one child
                ind += merge(arr+ind,child1,mmax1,child2,mmax2,p1,indent+1);
                }
            else
                { // the second node has no children
                  // copy the children of the first node
                ind += sizeof(int);
                arr[ind] = 1;
                arr[ind+1] = 12;// test
                ind += sizeof(int);
                for(k = child1;k < mmax1;++k)
                    arr[ind++] = *k;
                *(int *)(arrind) = ind;
                }
            }
        else
            {
            if(child2 < mmax2)
                { // the first node has no children
                  // copy the children of the second node
                ind += sizeof(int);
                arr[ind] = 2;
                arr[ind+1] = 15;// test
                ind += sizeof(int);
                for(k = child2;k < mmax2;++k)
                    arr[ind++] = *k;
                *(int *)(arrind) = ind;
                }
            else
                ;
            }
        *(int*)(arr) = ind;
        // do siblings
        arrind = arr+ind;
        *(int *)(arrind) = 0xDEAD;// test
        if(sibling1)
            {
            if(sibling2)
                {
                ind += merge(arrind,buf1+sibling1,len1,buf2+sibling2,len2,p/*,field1,field3*/,indent);
                }
            else
                {
                ind += sizeof(int);
                arr[ind] = 1;
                arr[ind+1] = 11;// test
                ind += sizeof(int);
                for(k = buf1+sibling1;k < len1;++k)
                    arr[ind++] = *k;
                *(int *)(arrind) = ind;
                }
            }
        else
            {
            if(sibling2)
                {
                ind += sizeof(int);
                arr[ind] = 2;
                arr[ind+1] = 14; // test
                ind += sizeof(int);
                for(k = buf2+sibling2;k < len2;++k)
                    arr[ind++] = *k;
                *(int *)(arrind) = ind;
                }
            else
                ;
            }
        }
    else // nodes differ
        {
        ind += sizeof(int);
        arr[ind] = 3; //second word: 3 means that there are a lhs and a rhs
        ind += sizeof(int);
        int offset = len1 - buf1 + sizeof(int);
        *(int *)(arr+ind) = offset; // third word: number of bytes to rhs
        ind += sizeof(int);
        for(k = buf1/*+sibling1*/;k < len1;++k)
            {
            arr[ind++] = *k;
            }
        for(k = buf2/*+sibling2*/;k < len2;++k)
            arr[ind++] = *k;
        *(int *)arr = ind;
        }
    return ind;
    }

int main(int argc, char * argv[])
    {
    char * buf1;
    int end1;
    char * buf2;
    int end2;
    char * buf12;
    int end12;
    if(argc < 2)
        {
        printf("usage:\ncombiflex bestflexrules nextbestflexrules combinedflexrules\n");
        return 0;
        }
    if(!readRules(argv[1],buf1,end1))
        {
        printf("Error: Cannot open %s for reading\n",argv[1]);
        return -1;
        }
    char start[1000] = {0};
    char end[1000] = {0};
    fm = fopen("1","wb");
    printrules(buf1,0,buf1+end1,start,end);
    fclose(fm);
    if(argc < 3)
        return 0;
    if(!readRules(argv[2],buf2,end2))
        {
        printf("Error: Cannot open %s for reading\n",argv[2]);
        return -1;
        }
    fm = fopen("2","wb");
    printrules(buf2,0,buf2+end2,start,end);
    fclose(fm);
    //printf("end1 %d + end2 %d = %d\n",end1,end2,end1+end2);
    char * arr = new char[2*(end1 + end2)];
    FILE * f = fopen(argv[3],"wb");
    if(!f)
        {
        printf("Error: Cannot open %s for writing\n",argv[3]);
        return -1;
        }
    fm = fopen("merged","wb");
    int length = merge(arr,buf1,buf1+end1,buf2,buf2+end2,"\t\t\t\n",0);
    fclose(fm);
    //printf("length %d\n",length);
    *(int*)arr = 0;
    for(int i = 0;i < length;++i)
        fputc(arr[i],f);
    fclose(f);
    if(!readRules(argv[3],buf12,end12))
        return -1;
    //printf("end12 %d\n",end12);
    start[0] = 0;
    end[0] = 0;
    /*
    printf("\n>>\n");
    printrules(buf12,0,buf12+end12,start,end);
    printf("\n<<\n");
    */
    return 0;
    }
