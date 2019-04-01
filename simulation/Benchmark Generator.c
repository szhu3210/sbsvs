//
//  Benchmark Generator.c
//  Phase 5
//
//  Modified by Shengxiang Zhu on 12/23/15.
//  Copyright © 2015 Shengxiang Zhu. All rights reserved.
//

#define MAXFILE 3000
#define BMN 11
#define STOCKNUM 2500
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
//#include <bsd/stdlib.h>


void filecopy(FILE *fi,char fdir[]) //copy file content into text in fdir
{

    FILE* fo=fopen(fdir, "w");
    char s[500];
    
    while(fgets(s, 500, fi) != NULL)
    {
        fprintf(fo,"%s\n",s);
    }
    
    fclose(fo);
}

void initialfile(FILE *filep[],char filedir[][255],int *filenum,int k)
{
    int i, j;
    FILE *t;
    char folder[]="/home/troy/Stock Analysis/1Data/5stockdata-final/Test0/";
    char folderout[]="/home/troy/Stock Analysis/1Data/5stockdata-final/";
    //search stock files and save to file pointer array
    char a[255];
    j = 0;										//stock counter
    
    //SH
    for (i = 600000; i < 605000; i++)
    {
        sprintf(a, "%sSH#%d.txt", folder, i);	//create the file directory
        t = fopen(a, "r");
        if (t != NULL)
        {
            filep[j] = t;
            sprintf(filedir[j],"%sTest%d//SH#%d.txt",folderout,k,i);
            j++;
        }
    }
    //printf("Stock #: %d\n", j);
    //SZ
    char b[255];
    for (i = 700000; i < 703000; i++)
    {
        sprintf(b, "SZ#%d.txt", i);				//adjust the file name
        b[3] = '0';
        sprintf(a, "%s%s", folder, b);			//create the file directory
        t = fopen(a, "r");
        if (t != NULL)
        {
            filep[j] = t;
            sprintf(filedir[j],"%sTest%d//%s",folderout,k,b);
            j++;
        }
    }
    //printf("Stock #: %d\n", j);
    //SZ-CYB
    for (i = 300000; i < 301000; i++)
    {
        sprintf(a, "%sSZ#%d.txt", folder, i);
        t = fopen(a, "r");
        if (t != NULL)
        {
            filep[j] = t;
            sprintf(filedir[j],"%sTest%d//SZ#%d.txt",folderout,k,i);
            j++;
        }
    }
    
    *filenum = j;
    printf("Stock #: %d\n", j);
}

void closefile(FILE *filep[],int filenum)
{
    int i;
    for (i=0;i<filenum;i++)
        if (filep[i]!=NULL) fclose(filep[i]);
}

int main()
{
    FILE *filep[MAXFILE];
    char filedir[MAXFILE][255];
    int filemark[MAXFILE];
    int filenum,i,j,r,k;

    srand(time(NULL));
    //printf("!\n");
    for (k=1;k<BMN;k++)
    {
        initialfile(filep,filedir,&filenum,k);

        for (i=0;i<MAXFILE;i++) filemark[i]=0;

        for (j=0;j<STOCKNUM;)
        {

            r=rand() % filenum;

            if (filemark[r]==0)
            {

                filemark[r]=1;
//printf("!2\n");
                filecopy(filep[r], filedir[r]);

                j++;

            }
            
        }
//printf("!close\n");
        closefile(filep,filenum);
    }
}
