#define BMN 11
#include <stdio.h>
#include <string.h>

void filenamegenerator(char s1[])
{
    FILE *fi=NULL;  //file pointer for read
    
    int i,j=0;
    
    char folder[255];
    char b[255];
    char fdir[255];
    char fname[255];
    
    sprintf(folder,"/home/troy/Stock Analysis/1Data/5stockdata-final/%s/",s1);
    
    printf("File Name Generator: Folder %s\n",s1);
    
    char outname[255];
    sprintf(outname, "%sfilename.txt",folder);
    FILE *fout=fopen(outname, "w");    //file pointer for print out filename
  
    //SH
    for (i = 600000; i < 605000; i++)
    {
        sprintf(fname, "SH#%d.txt", i);
        sprintf(fdir, "%s%s", folder, fname);
        fi = fopen(fdir, "r");
        if (fi != NULL)
        {
            fprintf(fout,"%s\n",fname);
            j++;fclose(fi);
        }
        
    }
    
    //SZ
    for (i = 700000; i < 703000; i++)
    {
        sprintf(b, "SZ#%d.txt", i);                 //adjust the file name
        b[3] = '0';
        sprintf(fdir, "%s%s", folder, b);			//create the file directory
        fi = fopen(fdir, "r");
        if (fi != NULL)
        {
            fprintf(fout, "%s\n", b);
            j++;fclose(fi);
        }
        
    }
    
    //SZ-CYB
    for (i = 300000; i < 301000; i++)
    {
        sprintf(fname, "SZ#%d.txt", i);
        sprintf(fdir, "%s%s", folder, fname);
        fi = fopen(fdir, "r");
        if (fi != NULL)
        {
            fprintf(fout, "%s\n", fname);
            j++;fclose(fi);
        }
        
    }
    
    //output status
    printf("Total searched file: %d\n",j);
    fclose(fout);
}

int main()
{
    int i;
    char temp[255];
    for (i=0; i<BMN; i++) {
        sprintf(temp, "Test%d",i);
        filenamegenerator(temp);
    }
    return 0;
}
