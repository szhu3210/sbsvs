//Phase 21
#define PHASE 21

//This version uses the special market data and SSD. It is based on Phase20 and going to analyze the behavior of stock scheme.
//Created by Troy on 3/16/2016

//Methodology: Automatic Greedy Hill Climbing

//Test Information

//Global Variable:
//int globalVariable = 0;

//Global Strategy:
#define MBS 20 //the maximum number of element of strategy (that of BS0, BS1 have one more element than strategy to store the F.Asset)
double globalstrategy[MBS];

#define STN 10 //the maximum number of element of strategy table.

//*********************************************************

int BMN = 1; //# of benchmark
#define BM ""
#define ON "test"  //outname
#define ABSTRACT "This version is to finalize the scoring system."

//*********************************************************

#define _DARWIN_UNLIMITED_STREAMS
#define _CRT_SECURE_NO_WARNINGS
#define MAXSTOCK 3000

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//changed INDEXMAXP to 85!
#define INDEXMAXP 85

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// In this phase I added EPS data and changed MAXP back to 31
#define MAXP 31

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define SCOREP 40
#define MAXINDEX 10
#define BN 100 //# of best results
#define SMP 15 //# of parameters in stockm[][]
#define MS 20  //# of max stocks in position
#define STARTYEAR 2000 //start from this year
#define CURRENTYEAR 2016 //current year, used to calculate annual trade time(in printstockm function)
#define MAXCURVERECORD 10000 // max number of curve record
#define INITIALCURRENCY 10000 //initial currency
#define MARKETCONDITION 40 //This is the market position control condition, e.g. 40 means ma40 and the label for w is (40+4=44), which is shown in the code
#define CURVEP 3 // the number of curve parameters

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <sys/resource.h>

double score(double w[][INDEXMAXP],double x[][MAXP],double y[][10],int i,int print,FILE *out);

void resety(double stocklist[][10],double y[][10],int j)
{
    int i;
    for (i=1; i<10; i++) {
        y[(int)round(stocklist[j][1])][i]=0;
    }
}

void sell(double stock[],double stocklist[][10],double gain,int i)
{
    //valid bit set -1 (temporarily, for printstocklist to print the sold stock, and then change into 0)
    stocklist[i][0]=-1;
    
    //currency
    stock[2] += stocklist[i][4] * stocklist[i][2] * 99.8;                     //currency increases with fees and taxes(0.2%) deducted
    
    //fees and taxes and counters
    stock[5] += stocklist[i][4] * stocklist[i][2] * 0.2;                          //fees and taxes sum up
    if (stocklist[i][4] > stocklist[i][3]) stock[6]++;                      //success counter+1
    else stock[7]++;                                                        //fail counter+1
}

void initialdata(char folder[], FILE *filep[], double x[][MAXP], double y[][10], int *filenum,double w[][INDEXMAXP],int flag[])
{
    int i, j;
    
    //initialize flag
    for (i=0;i<MAXSTOCK;i++)
        flag[i]=0;
    
    //initialize w
    for (i = 0; i < MAXINDEX; i++)
        for (j = 0; j < INDEXMAXP; j++)
            w[i][j] = 0;						//0 means that element is invalid
    
    //initialize x
    for (i = 0; i < MAXSTOCK; i++)
        for (j = 0; j < MAXP; j++)
            x[i][j] = 0;						//0 means that element is invalid
    
    //initialize y
    for (i = 0; i < MAXSTOCK; i++)
        for (j = 0; j < 10; j++)
            y[i][j] = 0;						//0 means that element is invalid
    
    //search stock files and save to file pointer array
    char a[255],b[255],s[255];
    j = 0;										//stock counter
    FILE *t=NULL;
    sprintf(a,"%sfilename.txt",folder);
    FILE *filename=fopen(a,"r");
    
    while (fscanf(filename,"%s\n",b)>0)
    {
        //printf("%s\n",b);
        sprintf(a, "%s%s", folder, b);	//create the file directory
        //printf("%s\n",a);
        t= fopen(a, "r");
        if (t != NULL)
        {
            filep[j]=t;
            if (b[3]=='0') b[3]='7';
            strncpy(s, b+3, 6);
            //printf("%s\n",s);
            sscanf(s,"%d",&i);
            y[j][0] = i;
            
            j++;
        }
    }
    
    if (filename!=NULL) fclose(filename);
    *filenum = j;
    
}

void printstocklist(FILE *out,double stock[10],double stocklist[][10],double y[][10])
{
    int i,j;
    char a[255];
    fprintf(out,"\nStock      Status   Stock#   Currency   P.Value   Asset   Taxes&Fees S.Counter  F.Counter  P.Limit  S.Limit\n      ");
    for (i=0;i<10;i++)
        fprintf(out, "%10.2f",stock[i]);
    fprintf(out, "\n");
    
    fprintf(out, "Stocklist\n");
    fprintf(out,"Stock      Status StockLabel   Hand        BP        CP    Value       P.Ratio    Gain    %%Gain      Score\n");
    //print current stocks in position
    for (i=0; i<stock[1]; i++) {
        
        sprintf(a, "%d", (int)y[(int)round(stocklist[i][1])][0]);
        if (a[0] == '7') a[0] = '0';
        fprintf(out,"%s",a);
        
        for (j=0; j<10; j++)
            fprintf(out, "%10.2f",stocklist[i][j]);
        fprintf(out, "\n");
    }
    fprintf(out, "--------\n");
    //print sold stocks and reset the valid bit back to 0
    for (i=2*MS-1; stocklist[i][0]==-1; i--) {
        
        sprintf(a, "%d", (int)y[(int)round(stocklist[i][1])][0]);
        if (a[0] == '7') a[0] = '0';
        fprintf(out,"%s",a);
        
        for (j=0; j<10; j++)
            fprintf(out, "%10.2f",stocklist[i][j]);
        fprintf(out, "\n");
        
        //reset the validbit back to 0
        stocklist[i][0]=0;
        
    }
    fprintf(out, "end.\n\n----------------------------------------------------------------------------------------------------------\n\n");
}

int isinposition(int i,double stocklist[][10])
{
    int j;
    for (j=0; j<MS; j++) {
        if (((int)round(stocklist[j][0])==1)&(i==(int)round(stocklist[j][1]))) return j;
    }
    return -1;
}

int stocknum(double x)
{
    return (int)(x/100000+3 > globalstrategy[7] ? globalstrategy[7] : x/100000+3);
}

void swapstocklist(double stocklist[][10],int i,int j)
{
    int k;
    double temp;
    for (k=0; k<10; k++) {
        temp=stocklist[i][k];
        stocklist[i][k]=stocklist[j][k];
        stocklist[j][k]=temp;
    }
}

void sortstocklist(double stocklist[][10],double stock[])
{
    int i,j,n;
    n=(int)round(stock[1]);
    
    //sort validbit
    for (i=0;i<2*MS-1; i++) {
        for (j=i+1; j<2*MS; j++) {
            if (stocklist[i][0]<stocklist[j][0]) {
                swapstocklist(stocklist,i,j);
            }
        }
    }
    
    //sort %Gain
    for (i=0; i<n-1; i++) {
        for (j=i+1; j<n; j++) {
            if (stocklist[i][8]<stocklist[j][8]) {
                swapstocklist(stocklist,i,j);
            }
        }
    }
}

void ogstock(double stock[],double stocklist[][10])
{
    //update stocklist
    int i;
    stock[1]=0;
    stock[3]=0;
    for (i=0; i<MS; i++) {
        if((int)round(stocklist[i][0])==1)
        {
            stocklist[i][5]=stocklist[i][2]*stocklist[i][4]*100;
            stocklist[i][7]=stocklist[i][2]*(stocklist[i][4]*0.998-stocklist[i][3])*100;
            stocklist[i][8]=100*(stocklist[i][4]*0.998/stocklist[i][3]-1);
            stock[1]++;
            stock[3]+=stocklist[i][5];
        }
    }
    
    //update stock
    if (stock[1]>0) stock[0]=1;
    else stock[0]=0;
    stock[4]=stock[2]+stock[3];
    stock[9]=(double)stocknum(stock[4]);
    
    //sort stocklist
    sortstocklist(stocklist,stock);
    
    //update position ratio
    for (i=0; i<stock[1]; i++) {
        stocklist[i][6]=stocklist[i][5]/stock[4]*100;
    }
    
}



void analysis(double w[][INDEXMAXP],double x[][MAXP],double y[][10],double stock[],double stocklist[][10], double gain,int filenum,int date,double cut,int print,double v,FILE *out)
{
    //market analysis
    
    //set total position limit according to the big board, market below MA50 cut all stocks.
    //if (w[1][4]<w[1][MARKETCONDITION+4]) stock[8]=0; else stock[8]=1;
    
    //count the good market signals and make the final control value.
    int k,  maXXaboveNum=0;
    
    for (k=0; k<MAXINDEX; k++) {
        
        if (w[k][4]>w[k][MARKETCONDITION+4]) maXXaboveNum++;
        
    }
    
    stock[8] = maXXaboveNum>= 10 ? 1 : 0 ;
    
    
    //stock analysis
    int i,j;
    
    for (i=0; i<filenum; i++)
        if ((int)round(x[i][0])==date)
        {
            if ((j=isinposition(i,stocklist))!=-1)
            {
                
                y[i][1]=0;
                
                //analyze if we need to sell it, thus y[i][1]={0,-1}
                
                //check if once reached the target gain line
                if (x[i][4]>(1+gain)*stocklist[j][3]) y[i][6]=1;
                
                //if strong enough to maintain on the gain line, wait the ma go beyong the gain line and use ma as sell boundary instead
                //ma20 prior to ma10 prior to go below the gain line sell
                
                if (x[i][9]>(1+gain)*stocklist[j][3])
                {
                    if (x[i][4]<x[i][9])
                    {
                        y[i][1]=-1;
                        if (print==1) fprintf(out,"Close broke MA50 after MA50 reached target gain.\n");
                    }
                    else y[i][1]=0;
                }
                
                else if (x[i][8]>(1+gain)*stocklist[j][3])
                {
                    if (x[i][4]<x[i][8])
                    {
                        y[i][1]=-1;
                        if (print==1) fprintf(out,"Close broke MA20 after MA20 reached target gain.\n");
                    }
                    else y[i][1]=0;
                }
                
                else if ((x[i][4]<(1+gain)*stocklist[j][3])&((int)round(y[i][6])==1))
                {
                    y[i][1]=-1;
                    if (print==1) fprintf(out,"Close broke target gain after once reached.\n");
                }
                
                //when stock goes bad: cut
                else if (x[i][4] <(1 - cut)*(stocklist[j][3]))
                {
                    y[i][1]=-1;
                    if (print==1) fprintf(out,"Cut.(close below cut line)\n");
                }
                
                //when stock goes plainly: sell if close < ma50 (SEEMS GOOD!)
                else if (x[i][4] <x[i][9])
                {
                    y[i][1]=-1;
                    if (print==1) fprintf(out,"Cut.(close below MA50)\n");
                }
                
                //when stock goes plainly: sell if market close < ma50 (SEEMS NOT GOOD!))
                //else if (w[1][4] <w[1][11]) y[i][1]=-1;NEED MODIFY
                
            }
            else
            {
                
                //for the rest stocks we analyze if we can buy it thus y[i][1]={0,1} y[i][2]=score of the stock
                y[i][2]=score(w,x,y,i,0,out);
                if (y[i][2]>=v) y[i][1]=1; else y[i][1]=0;
                
                //get position control(which is the possibility of success)
                y[i][5]=1/stock[9];
                
            }
        }
}

void updatestocklist(double stocklist[][10],double stock[],int date,double x[][MAXP])
{
    int i;
    for (i=0; i<MS; i++) {
        if ((int)round(stocklist[i][0])==1) {
            if((int)round(x[(int)round(stocklist[i][1])][0])==date)
                stocklist[i][4]=x[(int)round(stocklist[i][1])][4];
        }
    }
}

void readdata(int date, FILE *filep[], double x[][MAXP], double y[][10], int filenum, int flag[],double stock[],double stocklist[][10], double w[][INDEXMAXP],FILE *indexp[])
{
    int i, j;

    //char tempString[1024];
    for (i = 0; i < MAXINDEX; i++) while (round(w[i][0]) < date) {
        
        //fgets(tempString, 1024, indexp[i]);
        for (j = 0; j < INDEXMAXP; j++) {

            fscanf(indexp[i], "%lf", &w[i][j]);
	}
        //printf("%lf",w[i][2]);
        
    }
    

    for (i = 0; i < filenum; i++)
    {
        while ((round(x[i][0]) < date)&(flag[i]==0)) {
            
            //fgets(tempString, 1024, filep[i]);
            
            for (j = 0; j < MAXP; j++) {
                
                //in case there is an end
                if (fscanf(filep[i], "%lf", &x[i][j])<0)
                    flag[i] = 1;
                
            }
        }
    }
    
    //update stocklist
    updatestocklist(stocklist,stock,date,x);
    ogstock(stock,stocklist);

}

double min3(double a1,double a2,double a3)
{
    if (a1<a2) {
        if (a1<a3) {
            return a1;
        }
        else return a3;
    }
    else if (a2<a3) return a2;
    else return a3;
}

int buy(int stocklabel, double score, double bp, double stock[], double stocklist[][10], double maxposition)
{
    //assume stocklist is organized
    int a=0;
    if (((int)round(stock[1])<(int)round(stock[9]))&((stock[3]/stock[4])<stock[8])) {
        stocklist[MS-1][1]=stocklabel;
        stocklist[MS-1][2]=(a=(int)(min3(stock[4]*stock[8]-stock[3],stock[2],stock[4]*maxposition)/(bp*100)))>0 ? a : 0;
        stocklist[MS-1][3]=bp;
        stocklist[MS-1][4]=stocklist[MS-1][3];
        stocklist[MS-1][9]=score;
        stocklist[MS-1][0]=stocklist[MS-1][2]>0 ? 1 : 0;
        stock[2]-=stocklist[MS-1][2]*stocklist[MS-1][3]*100;
        a=stocklist[MS-1][0];
    }
    
    return a;
}

void writehighscore(double score,int i, double highscore[][2])
{
    highscore[MS-1][0]=score;
    highscore[MS-1][1]=i;
}

void sorthighscore(double highscore[][2])
{
    int i,j;
    double temp;
    //int temp;
    for (i=0; i<MS-1; i++) {
        for (j=i+1; j<MS; j++) {
            if (highscore[i][0]<highscore[j][0]) {
                temp=highscore[i][0];
                highscore[i][0]=highscore[j][0];
                highscore[j][0]=temp;
                temp=highscore[i][1];
                highscore[i][1]=highscore[j][1];
                highscore[j][1]=temp;
            }
        }
    }
}

void initialhighscore(double highscore[][2])
{
    int i,j;
    for (i=0; i<MS; i++) {
        for (j=0; j<2; j++) {
            highscore[i][j]=-1;
        }
    }
}

int action(int date,double w[][INDEXMAXP], double x[][MAXP], double y[][10], int filenum, double cut, double stock[],double stocklist[][10], FILE *out,int print,int printcm,double gain)
{
    char a[255];									//reserved for print out the specific trade list
    int act=0;
    
    //sell
    int i;
    for (i=0; i<MS; i++)
        if ((x[(int)round(stocklist[i][1])][0] == date)&(round(stocklist[i][0]) == 1))				//date valid test and valid bit test
        {
            if ((((date>=19961216)&(x[(int)round(stocklist[i][1])][7]>-9.9))||(date<19961216))&(y[(int)round(stocklist[i][1])][1]==-1))
                //check if fulfill the condition of sell
            {
                sell(stock,stocklist,gain,i);
                //The following are for print out the specific trade list
                if (print == 1)
                {
                    sprintf(a, "%d", (int)y[(int)stocklist[i][1]][0]);
                    if (a[0] == '7') a[0] = '0';
                    fprintf(out,"%d  %s  SELL \n", date, a);
                    
                }
                resety(stocklist,y,i);
                ogstock(stock, stocklist);
                act=1;
            }
        }
    //total position control
    i=MS-1;
    while ((stock[3]/stock[4])>stock[8]) {
        if ((round(stocklist[i][0]) == 1)&(((date>=19961216)&(x[(int)round(stocklist[i][1])][7]>-9.9))||(date<19961216))&(x[(int)round(stocklist[i][1])][0] == date))     //check valid bit, date, and trade limit
        {
            sell(stock,stocklist,gain,i);   //unconditional sell according to the total position limit from the stock with worst performance
            if (print == 1)
            {
                sprintf(a, "%d", (int)y[(int)stocklist[i][1]][0]);
                if (a[0] == '7') a[0] = '0';
                fprintf(out,"%d  %s  SELL \n", date, a);
                fprintf(out, "Position Control. Current Position Limit: %d%%\n",(int)ceil(stock[8]*100));
            }
            resety(stocklist,y,i);
            ogstock(stock, stocklist);
            act=1;
        }
        i--;
        if (i<0) break;
    }
    
    //buy
    int besti=-1;
    double highscore[MS][2];
    initialhighscore(highscore);
    int buyact;
    
    //find best stocks (highest score)
    for (i = 0; i < filenum ; i++)
        if ( (round(x[i][0]) == date) & (round(y[i][1])==1) &  (((date>=19961216)&(x[i][7]<9.9))||date<19961216))
            if (y[i][2]>highscore[MS-1][0]) {
                writehighscore(y[i][2],i,highscore);
                sorthighscore(highscore);
            }
    
    for (i=0; (i<MS) & ((highscore[i][0])>=0); i++){
        besti=highscore[i][1];
        if ((stock[1]<stock[9])&((stock[3]/stock[4])<stock[8]))
        {
            
            act=(buyact=buy(besti,highscore[i][0], x[besti][4], stock, stocklist, y[besti][5]))>0 ? 1: act; //function "buy" has included position control
            if (buyact==1) score(w, x, y, besti, printcm,out);
            ogstock(stock, stocklist);
            //The following are reserved for print out the specific trade list
            if ((print == 1)&(buyact))
            {
                sprintf(a, "%d", (int)y[besti][0]);
                if (a[0] == '7') a[0] = '0';
                fprintf(out,"%d  %s  BUY \n", date, a);
            }
        }
    }
    return act;
    
}

void inistock(double stock[],double stocklist[][10])
{
    //stock initialization
    stock[0]=0;         //position status
    stock[1]=0;         //# of stocks
    stock[2]=INITIALCURRENCY;     //currency
    stock[3]=0;         //position value
    stock[4]=stock[2];  //asset
    stock[5]=0;         //sum of fees and taxes
    stock[6]=0;         //success counter
    stock[7]=0;         //fail counter
    stock[8]=1;         //position limit(by big market,temporarily set to 1
    stock[9]=stocknum(stock[4]);
    
    //stocklist initialization
    int i,j;
    for (i=0; i<MS; i++) {
        for (j=0; j<10; j++) {
            stocklist[i][j]=0;
        }
    }
}

void closefile(FILE *filep[], int filenum)
{
    int k;
    for (k = 0; k < filenum; k++) if (filep[k]!=NULL) fclose(filep[k]);
}

void initialindex(FILE *indexp[])
{
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    //changed to SPECIALDATAFORPHASE9 !
    char s0[255]="/home/troy/Stock Analysis/1Data/6indexdata-FPP/";
    //char s0[255]="/Volumes/DATA_0/2016/3Stock Analysis/1Data/13StandardIndexData(Final)/";
    
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    char s[255];
    sprintf(s,"%sSH#000043.txt",s0);
    indexp[0]=fopen(s,"r");
    
    sprintf(s,"%sSH#000001.txt",s0);
    indexp[1]=fopen(s,"r");
    
    sprintf(s,"%sSZ#399300.txt",s0);
    indexp[2]=fopen(s,"r");
    
    sprintf(s,"%sSZ#399001.txt",s0);
    indexp[3]=fopen(s,"r");
    
    sprintf(s,"%sSZ#399106.txt",s0);
    indexp[4]=fopen(s,"r");
    
    sprintf(s,"%sSZ#399100.txt",s0);
    indexp[5]=fopen(s,"r");
    
    sprintf(s,"%sSZ#399005.txt",s0);
    indexp[6]=fopen(s,"r");
    
    sprintf(s,"%sSZ#399006.txt",s0);
    indexp[7]=fopen(s,"r");
    
    sprintf(s,"%sSZ#399101.txt",s0);
    indexp[8]=fopen(s,"r");
    
    sprintf(s,"%sSZ#399102.txt",s0);
    indexp[9]=fopen(s,"r");
}

void closeindex(FILE *indexp[])
{
    int i;
    for (i=0;i<MAXINDEX;i++)
        if (indexp[i]!=NULL) fclose(indexp[i]);
}

void savestock(double stock[],double stockresult[][10],int k)
{
    int l;
    for (l=0;l<10;l++) stockresult[k][l]=stock[l];
}

void initialannualgrowth(double annualgrowth[][5])
{
    int i,j;
    for (i=0; i<100; i++) {
        for (j=0; j<5; j++) {
            annualgrowth[i][j]=0;
        }
    }
}

void storeasset(int date,int date0,double stock[],double annualgrowth[][5],double w[][INDEXMAXP])
{
    int i;
    if ((date/10000-date0/10000)==1)
    {
        i=date0/10000-STARTYEAR;
        annualgrowth[i][0]=date0/10000;
        annualgrowth[i][1]=stock[4];
        annualgrowth[i][2]=(i>0)?((annualgrowth[i][1]/annualgrowth[i-1][1]-1)*100):0;
        annualgrowth[i][3]=w[1][4];//market close
        annualgrowth[i][4]=(i>0)?((annualgrowth[i][3]/annualgrowth[i-1][3]-1)*100):0;
    }
}

void printannualgrowth(double annualgrowth[][5],FILE *out)
{
    int i=0;
    fprintf(out, "\nAnnual Asset & Growth\n%5s%12s%10s%10s%15s\n","Year","Asset","Growth","Index","Market Growth");
    while (annualgrowth[i][0]!=0) {
        fprintf(out,"%5d",(int)round(annualgrowth[i][0]));
        fprintf(out,"%12.2f",annualgrowth[i][1]);
        fprintf(out,"%10.2f%%",annualgrowth[i][2]);
        fprintf(out,"%10.2f",annualgrowth[i][3]);
        fprintf(out,"%10.2f%%",annualgrowth[i][4]);
        fprintf(out, "\n");
        i++;
    }
}

void initialCurve(double curve[][CURVEP])
{
    int i,j;
    for (i=0; i<MAXCURVERECORD; i++) {
        for (j=0; j<CURVEP; j++) {
            curve[i][j]=0;
        }
    }
}

void storeCurve(double curve[][CURVEP],int date, double stock[],double w[][INDEXMAXP])
{
    int i;
    for (i=0;i<MAXCURVERECORD;i++) {
        if ( curve[i][0] == 0 ) {
            curve[i][0] = date;
            curve[i][1] = stock[4];
            curve[i][2] = w[1][4];
            break;
        }
    }
}

void printCurve(double curve[][CURVEP], FILE *out2)
{
    int i;
    fprintf(out2,"Asset curve\n   date     asset   market\n\n");
    for (i=0; i<MAXCURVERECORD; i++) {
        if (curve[i][0] != 0) {
            fprintf(out2, "%d/%d/%d  %10.2f%10.2f\n",(int)(round(curve[i][0]))/10000,(int)(round(curve[i][0]))/100%100,(int)(round(curve[i][0]))%100,curve[i][1],curve[i][2]);
        } else {
            break;
        }
    }
    //fprintf(out2,"\n\n------------------------------\n\n");
}

void test(double scheme[10],char benchmark[],double stockresult[][10],FILE *out)
{
    double w[MAXINDEX][INDEXMAXP];      //used to read index
    double x[MAXSTOCK][MAXP];			//used to read data
    double y[MAXSTOCK][10];				//analysis of data
    int flag[MAXSTOCK];					//used to mark the ending of a file
    double stock[10];					//overall asset and position information
    double stocklist[2*MS][10];         //stocklist
    
    FILE *d;							//pointer of date.txt
    FILE *filep[MAXSTOCK];				//store all the file pointers
    FILE *indexp[MAXINDEX];             //store all the file pointers
    int filenum;						//file number
    
    double cut, gain;		//set cut and target gain overhead
    
    int date,date0=0;
    int k;
    double v;
    
    cut = scheme[0];
    gain = scheme[1];
    v = scheme[2];
    
    double annualgrowth[100][5];
    initialannualgrowth(annualgrowth);
    
    double curve[MAXCURVERECORD][CURVEP];
    FILE *out2=NULL;
    if (scheme[4] == 1) {
        out2=fopen("/home/troy/Desktop/AssetCurve.txt", "w");
    }
    
    char folder[BMN][255];
    for (k=0;k<BMN;k++) sprintf(folder[k],"/home/troy/Stock Analysis/1Data/5stockdata-final/Test%d/",k);
    
    if (globalstrategy[MBS-1]==1) printf("Results:\n\n");
    if (globalstrategy[MBS-1]==1) fprintf(out,"Results:\n\n");
    

    int mark;
    //Test a bunch of test datas
    for (k=0;k<BMN;k++)
    {
        //set date pointer
        d = fopen("/home/troy/Stock Analysis/1Data/1date/date.txt", "r");
        
        //skip some years and start from STARTYEAR
        while (fscanf(d, "%d", &date) >=0) {
            
            if (date/10000 >= STARTYEAR) break;
            
        }
        
        //Initialize indexp
        initialindex(indexp);
        
        //Initialize w,x,y, search txt files from folder and set filep, get the filenum
        initialdata(folder[k], filep, x, y, &filenum, w,flag);
        
        //Initialize stock and stocklist
        inistock(stock,stocklist);
        
        //Initialize curve
        initialCurve(curve);
      

        //read date and read stock data and action
        while (fscanf(d, "%d", &date) >=0)
        {
            
            //printf("readdata:%d\n",date);
            storeasset(date,date0,stock,annualgrowth,w);
            
            //in a new day execute the following code
            

            //read the data of a designated date into x and y
            readdata(date, filep, x, y, filenum, flag,stock,stocklist,w,indexp);
            

            //analyze the data
            analysis(w,x,y,stock,stocklist,gain,filenum,date,cut,scheme[7],v,out);
            

            //sell and buy and output trade list
            if (action(date,w, x, y, filenum, cut, stock,stocklist, out,scheme[9],scheme[8],gain))
              
 
            //print out stock list
            if (scheme[6]==1) printstocklist(out, stock, stocklist,y);
            
            //store curve data
            if (scheme[4]==1) storeCurve(curve,date,stock,w);
            
            
            date0=date;//save the old date
            
        }
        

        //close date pointer
        if (d!=NULL) fclose(d);
        //close files
        closefile(filep, filenum);
        //close indexp
        closeindex(indexp);
        
        //print result
        if (globalstrategy[MBS-1]==1) printf("Test %d : File#:%4d   F.Asset: %6.2f\n",k,filenum,stock[4]);
        if (globalstrategy[MBS-1]==1) fprintf(out,"Test %d : File#:%4d   F.Asset: %6.2f\n",k,filenum,stock[4]);
        if (globalstrategy[MBS-1]==1) if (scheme[5]==1) printannualgrowth(annualgrowth, out);
        if (globalstrategy[MBS-1]==1) if (round(scheme[9])==1) fprintf(out,"\n***********************************************************************************************\n\n\n");
        
        //print curve
        if (globalstrategy[MBS-1]==1) if (scheme[4]==1) printCurve(curve,out2);
        
        //save stock into stockresult
        savestock(stock,stockresult,k);
        
    }
    
}

void initialstockm(double stockm[][SMP])
{
    int i, j;
    for (i = 0; i < BN; i++)
        for (j = 0; j < SMP; j++)
            stockm[i][j] = 0;
}

void writenew(double stockavg[], double i, double j, double stockresult[][10],double stockm[][SMP])
{
    int k;
    for (k = 0; k < 11; k++)
        stockm[BN-1][k] = stockavg[k];
    stockm[BN-1][11] = i;
    stockm[BN-1][12] = j;
    stockm[BN-1][13] = stockresult[BMN-1][4];
}

void swapstockm(int i, int j, double stockm[][SMP])
{
    int k;
    double temp[SMP];
    for (k = 0; k < SMP; k++)
        temp[k] = stockm[i][k];
    for (k = 0; k < SMP; k++)
        stockm[i][k] = stockm[j][k];
    for (k = 0; k < SMP; k++)
        stockm[j][k] = temp[k];
}

void sortstockm(double stockm[][SMP])
{
    int i, j;
    for (i = 0; i < BN-1; i++)
        for (j = i+1; j < BN;j++)
            if (stockm[i][4] < stockm[j][4])
                swapstockm(i, j, stockm);
}

void storem(double i, double j, double stockavg[],double stockresult[][10],double stockm[][SMP])
{
    //store
    if (stockavg[4] > stockm[BN-1][4]) writenew(stockavg, i, j,stockresult, stockm); else return;
    
    //sort
    sortstockm(stockm);
}

void printstockm(FILE *out,double stockm[][SMP])
{
    int i;
    fprintf(out,"\n\n\n###############################################################################################\n\n");
    fprintf(out, "\n\n                 The Best %d Schemes\n\nVariable   Deviation   F.Asset       Success    fail      S.Rate  Taxes&Fees Least F.Asset  All Gain?  Avg.Profit  Avg.T.Times\n",BN);
    for (i = 0; i < BN; i++)
    {
        fprintf(out, "%6.2f", stockm[i][11]);
        fprintf(out, "%10.2f%%", stockm[i][12]);
        fprintf(out, "%15.2f", stockm[i][4]);
        fprintf(out, "%10d", (int)stockm[i][6]);
        fprintf(out, "%10d", (int)stockm[i][7]);
        fprintf(out, "%11.2f%%", 100*(stockm[i][6]/(stockm[i][6]+ stockm[i][7])));
        fprintf(out, "%12.2f", stockm[i][5]);
        fprintf(out, "%12.2f", stockm[i][13]);
        
        if (round(stockm[i][10])==1) fprintf(out, "    ALL GAIN"); else fprintf(out, "            ");
        
        //Print out Avg.Profit
        fprintf(out, "%11.2f%%", (pow((stockm[i][4]/INITIALCURRENCY),(1/(stockm[i][6]+stockm[i][7])))-1)*100);
        
        //Print out A.T.Times (annual trade times)
        fprintf(out, "%12.2f", (stockm[i][6]+stockm[i][7])/(CURRENTYEAR-STARTYEAR));
        
        fprintf(out, "\n");
    }
}

void initialresult(double stockresult[][10],double stockavg[])
{
    int k,l;
    for (k=0;k<BMN;k++)
        for (l=0;l<10;l++)
            stockresult[k][l]=0;
    for (l=0;l<11;l++)
        stockavg[l]=0;
}

int allgain(double stockresult[][10])
{
    int x=1,i;
    for (i=0;i<BMN;i++)
        if (stockresult[i][4]<10000)
        {
            x=0;
            break;
        }
    return x;
}

void swapstockresult(double stockresult[][10],int k,int l)
{
    double t[10];
    int i;
    for (i=0;i<10;i++)
        t[i]=stockresult[k][i];
    for (i=0;i<10;i++)
        stockresult[k][i]=stockresult[l][i];
    for (i=0;i<10;i++)
        stockresult[l][i]=t[i];
}

void calstockresult(double stockresult[][10], double stockavg[], double *deviation)
{
    int k,l;
    //sort stockresult
    for (k=1;k<BMN-1;k++)
        for (l=k+1;l<BMN;l++)
            if (stockresult[k][4]<stockresult[l][4]) swapstockresult(stockresult,k,l);
    //calculate stockavg;
    for (k=0;k<BMN;k++)
        for (l=0;l<10;l++)
            stockavg[l]+=stockresult[k][l];
    for (l=0;l<10;l++)
        stockavg[l]=stockavg[l]/(BMN);
    for (k=0;k<BMN;k++)
        *deviation+=stockavg[4]>stockresult[k][4] ? pow(stockavg[4]-stockresult[k][4],2) : 0;
    *deviation=pow(*deviation/BMN,0.5)/stockavg[4]*100;
    if (allgain(stockresult)==1) stockavg[10]=1;
}

void printscreen(double stockavg[])
{
    if (round(stockavg[10])==1) printf("----------------------------------------------- ALL GAIN!\n");
    printf("\nFinal Average Asset:%10.2f  \nAnnual growth:       %5.2f%% \nAvg.Success:    %d \nAvg.Fail:       %d \nTax&Fees:       %.2f \n", stockavg[4],100*(pow(stockavg[4]/INITIALCURRENCY, 1.0/(CURRENTYEAR-STARTYEAR))-1),(int)stockavg[6],(int)stockavg[7],stockavg[5]);
}

void printfile(double stockavg[],FILE *out,double deviation,double stockresult[][10])
{
    if (round(stockavg[10])==1) fprintf(out,"----------------------------------------------- All Gain!\n");
    fprintf(out, "\nLeast F.Asset : %10.2f  (%5.2f%%)\n    A F.Asset : %10.2f  (%5.2f%%)\n\nFinal Average Asset:  %10.2f \nAsset Deviation:       %9.2f \nAverage Annual growth: %5.2f%%  \nAvg.Success:      %d \nAvg.Fail:         %d \nTax&Fees:     %.2f \n",stockresult[BMN-1][4],100*(pow(stockresult[BMN-1][4]/INITIALCURRENCY, 1.0/(CURRENTYEAR-STARTYEAR))-1),stockresult[0][4],100*(pow(stockresult[0][4]/INITIALCURRENCY, 1.0/(CURRENTYEAR-STARTYEAR))-1),stockavg[4],deviation,100*(pow(stockavg[4]/INITIALCURRENCY, 1.0/(CURRENTYEAR-STARTYEAR))-1),(int)stockavg[6],(int)stockavg[7],stockavg[5]);
}

void printscheme(double scheme[],char schemename[][255],FILE *out)
{
    int i;
    printf("\n\n\nCurrent testing scheme:\n");
    fprintf(out,"\n\n========================================================================\n\n\nCurrent testing scheme:\n");
    for (i=0;i<10;i++)
    {
        printf("%3d%12s:%6.2f\n",i,schemename[i],scheme[i]);
        fprintf(out,"%3d%12s:%6.2f\n",i,schemename[i],scheme[i]);
    }
    //printf("\nMARKETCONDITION+4: %d   (ma%d)\n\n",MARKETCONDITION+4,MARKETCONDITION);
    //fprintf(out,"\nMARKETCONDITION: %d   (ma%d)\n\n",MARKETCONDITION+4,MARKETCONDITION);
}

void printbasicinfo(FILE *out,char outname[])
{
    printf("Basic Information:\nBenchmark Dir: %s\nBenchmark #:   %d\nOutput File:%s\n\n",BM,BMN,outname);
    fprintf(out,"Basic Information:\nBenchmark Dir:%s \nBenchmark #:   %d\nOutput File:%s\n\nAbstract:%s\n\n",BM,BMN,outname,ABSTRACT);
}

void printtime(int i,FILE *out)
{
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    if (i==1) fprintf(out,"Phase %d\nStart time: %d-%d-%d %d:%d:%2d\n\n", PHASE, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    if (i==2) fprintf(out,"\n\n\nEnd time: %d-%d-%d %d:%d:%2d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    
}

double score(double w[][INDEXMAXP],double x[][MAXP],double y[][10],int i,int print,FILE *out)
{
    double a[SCOREP];
    int j;
    double s=0;
    
    double weight[SCOREP]={ 5,1,globalstrategy[2],globalstrategy[3],1,   1,1,1,1,1,   1,1,1,1,1,   1,1,1,1,1,   1,1,1,1,1,   1,1,1,1,1,   1,1,1,1,1,   1,1,1,1,1 };
    
    //initialize
    for (j=0;j<SCOREP;j++) a[j]=0;
    
    //------------------------------------------------------------------------------------------------------
    int mark;
    // Part 1 : New High
    
    
    //    // 0 close new high (20)
    //    if (x[i][4]>x[i][17]) a[0]=1;
    //
    //    // 1 close new high (50)
    //    if (x[i][4]>x[i][18]) a[1]=1;
    //
    //    // 2 close new high (200)
    //    if (x[i][4]>x[i][19]) a[2]=1;
    //
    // 3 close new high (historical) After tests, this is the best condition among 0~3.
    
    // -----------Phase 10 - 9-------------temporarily changed----------------------
    
    // Make sure that it is not a new stock (has valid x[i][19] data) and pivot.
    if ((x[i][10+(int)globalstrategy[0]]>0)&(x[i][4]>x[i][10+(int)globalstrategy[0]])) a[0]=1;
    
    //if (x[i][4]>x[i][20]) a[3]=1; //The original one
    
    
    //
    //    // Part 2 : MA
    //
    
    //-----------------Phase 17-----------------------
    //Test1
    //if ((x[i][9]>0)&(x[i][4]>x[i][9])) a[6]=1;
    
    //Test2
    //if (x[i][11]>0) a[6] = 1;
    
    //Test3
    //if ((x[i][11]>0)&(x[i][9]>0)&(x[i][4]>x[i][9])) a[6]=1;
    
    //Test4
    //if ((x[i][8]>0)&(x[i][4]>x[i][8])) a[6]=1;
    
    //Test5
    // No ma20up data so cannot implement
    
    //Test6
    // No ma20up data so cannot implement
    
    //Test7
    //if ((x[i][10]>0)&(x[i][4]>x[i][10])) a[6]=1;
    
    //Test8
    //if (x[i][12]>0) a[6] = 1;
    
    //Test9
    //if ((x[i][12]>0)&(x[i][10]>0)&(x[i][4]>x[i][10])) a[6]=1;
    
    //Test10
    //if ((x[i][8]>x[i][9])&(x[i][9]>0)) a[6]=1;
    
    //Test11
    //if ((x[i][9]>x[i][10])&(x[i][10]>0)) a[6]=1;
    
    //Test12
    //if ((x[i][8]>x[i][9])&(x[i][9]>x[i][10])&(x[i][10]>0)) a[6]=1;
    //-------------------------Phase 17 end-------------------------------
    
    //
    //    // 5 close > ma20
    //    if (x[i][4]>x[i][8]) a[5]=1;
    //
    //    // 6 close > ma50
    //    if (x[i][4]>x[i][9]) a[6]=1;
    //
    //    // 7 close > ma200
    //    if (x[i][4]>x[i][10]) a[7]=1;
    //
    //    // 8 ma50 up rate > 0
    //    if (x[i][11]>0) a[8]=1;
    //
    //    // 9 ma200 up rate > 0
    //    if (x[i][12]>0) a[9]=1;
    //
    //    // 10 volume > vma50 * 1.5
    //    if (x[i][5]>x[i][14]*1.5) a[10]=1;
    
    // ----------------Phase 16 (It is proved to be useless in this case.)-----------------
    // a[10] = volume / vma20 or volume / vma50 ;
    // The higher the better?
    
    // The range of this score is in 0 or 1
//    if (x[i][13]>0) {
//        a[10] = x[i][5]/x[i][13] > globalVariable ? 1.0 : 0 ;
//    }
    
    //
    //    // 11 volume > vma50 * 2
    //    if (x[i][5]>x[i][14]*2.0) a[11]=1;
    //
    //    // 12 vma20 up rate > 0
    //    if (x[i][15]>0) a[12]=1;
    //
    //    // 13 vma50 up rate > 0
    //    if (x[i][16]>0) a[13]=1;
    //
    //    // Part 3 : Market
    //
    //    // 15 market close new high (20)
    //    if (w[1][4]>w[1][17]) a[15]=1;
    //
    //    // 16 market close new high (50)
    //    if (w[1][4]>w[1][18]) a[16]=1;
    //
    //    // 17 market close new high (200)
    //    if (w[1][4]>w[1][19]) a[17]=1;
    //
    //    // 18 market close new high (historical)
    //    if (w[1][4]>w[1][20]) a[18]=1;
    //
    //    // 19 market close > ma20
    //    if (w[1][4]>w[1][8]) a[19]=1;
    //
    //    // 20 market close > ma50
    //    if (w[1][4]>w[1][9]) a[20]=1;
    //
    //    // 21 market close > ma200
    //    if (w[1][4]>w[1][10]) a[21]=1;
    //
    //    // 22 market ma50 up rate > 0
    //    if (w[1][11]>0) a[22]=1;
    //
    //    // 23 market ma200 up rate > 0
    //    if (w[1][12]>0) a[23]=1;
    //
    //    // 24 market volume > market vma50
    //    if (w[1][5]>w[1][14]) a[24]=1;
    //
    //    // 25 market volume > market vma50 * 1.1
    //    if (w[1][5]>w[1][14]*1.1) a[25]=1;
    //
    //    // 26 market vma 20 up rate > 0
    //    if (w[1][15]>0) a[26]=1;
    //
    //    // 27 market stability in 20 days = highest high (20) / lowest low (20) < 1.2
    //    if ( w[1][17]/w[1][21] < 1.2 ) a[27]=1;
    //
    //    // 28 market stability in 50 days = highest high (50) / lowest low (50) < 1.3
    //    if ( w[1][18]/w[1][22] < 1.3 ) a[28]=1;
    //
    //    // 29 market stability in 200 days = highest high (200) / lowest low (200) < 1.4
    //    if ( w[1][19]/w[1][23] < 1.4 ) a[29]=1;
    //
    //    // Part 4 : Price Stability
    //
    
    //    ----------Phase 10 - 10-----------
    
    //    Definition: stability = lowest low / highest high ;   score is in (0,1)
    
    if (x[i][19+(int)globalstrategy[0]]>0 && x[i][10+(int)globalstrategy[0]]>0)   // make sure that the data is valid (not negative 1)
        
    {
        
        a[1] = pow(x[i][19+(int)globalstrategy[0]] / x[i][10+(int)globalstrategy[0]],2) ; // stability of 20,50,200,historical day price interval
        
    }
    
    
    
    //    // 30 stability in 20 days = highest high (20) / lowest low (20) < 1.2
    //    if ( x[i][17]/x[i][21] < 1.2 ) a[30]=1;
    //
    //    // 31 stability in 50 days = highest high (50) / lowest low (50) < 1.3
    //    if ( x[i][18]/x[i][22] < 1.3 ) a[31]=1;
    //
    //    // 32 stability in 200 days = highest high (200) / lowest low (200) < 1.4
    //    if ( x[i][19]/x[i][23] < 1.4 ) a[32]=1;
    //
    //    // 33 stability (historical) = highest high (historical) / lowest low (historical) < 1.5
    //    if ( x[i][20]/x[i][24] < 1.5 ) a[33]=1;
    //
    //    // Part 5 : Basics ( Liquidity + EPS )
    //
    //    // WARNING! EPS may be negative! Be careful when calculating up rate!
    //
    // ---------------------Phase 10 - 11--------------------------
    // Condition: EPS up rate > PE => 1 else 0
    
    
    // ---------------------Phase 18------------------------------
    
    //Test1 : Liquidity
//    if ((x[i][30]<globalVariable)&(x[i][30]>0)) {
//        a[34]=1;
//    }
    
    //Test2 : EPS
//        if ((x[i][25]>globalVariable)&(x[i][25]>0)) {
//            a[34]=1;
//        }
    
    //Test3 : EPS%
    //Cannot implement due to the lack of data.
    
    //Test4 : EPSTTM control EPSTTM = [0.00, 1.00]
//    if ((x[i][26]>globalVariable)&(x[i][26]>0)) {
//        a[34]=1;
//    }
    
    //Test5 : EPSTTM% control EPSTTM% = [0.00%, 100.00%]
//    if ((x[i][27]>globalVariable*100)&(x[i][27]>0)) {
//        a[34]=1;
//    }
    
    //Test6 : ROETTM control ROE = [0.00%, 40.00%]
//    if ((x[i][28]>globalVariable*40)&(x[i][28]>0)) {
//        a[34]=1;
//    }
    
    //Test7 : ROETTM% control ROE% = [0.00%, 100.00%]
//    if ((x[i][29]>globalVariable*100)&(x[i][29]>0)) {
//        a[34]=1;
//    }
    
    //Test8 : EPSTTM > 0 + EPSTTM% control EPSTTM% = [0.00%, 100.00%]
//        if ((x[i][27]>globalVariable*100)&(x[i][27]>0)&(x[i][26]>0)) {
//            a[34]=1;
//        }
    
    //Additional test of Phase 18, EPSTTM% > PE
//    if ((x[i][27]/x[i][4]/x[i][25]>globalVariable)&(x[i][27]>0)&(x[i][4]>0)&(x[i][25]>0)) {
//        a[34]=1;
//    }
    
    // =======================Phase 20=============================
    
    //double eps=0, roe=0;
    
    if ((x[i][28]>globalstrategy[1])&(x[i][29]>0)) {
        a[2] = (x[i][29]>100.0 ? 1 : x[i][29]/100.0);
    }
    
    if (x[i][30]>0) {
        a[3] = (x[i][30]/50>1 ? 1 : (x[i][30]/50));
    }
    
    //a[2] = 0.5 * (eps + roe);
    //printf("a[30]=%f\n",a[30]);
    
    // =======================Phase 20 end=========================
    
    //    // 34 Liquid share < 10 e9
    //    if ( x[i][30] < 10 ) a[34]=1;
    //
    //    // 35 EPS > 0.05
    //    if ( x[i][25] > 0.05 ) a[35]=1;
    //
    //    // 36 EPSTTM > 0.05
    //    if ( x[i][26] > 0.05 ) a[36]=1;
    //
    //    // 37 EPSTTM up rate > 10%
    //    if ( x[i][27] > 10 ) a[37]=1;
    //
    //    // 38 ROETTM > 0.05
    //    if ( x[i][28] > 0.05 ) a[38]=1;
    //
    //    // 39 ROETTM up rate > 10%
    //    if ( x[i][29] > 10 ) a[39]=1;
    
    
    //------------------------------------------------------------------------------------------------------
    
    
    //sum up the scores and weights
    for (j=0;j<SCOREP;j++) s+=a[j]*weight[j];
    
    
    //print committee machine
    if (print==1)
    {
        fprintf(out,"Score: %3.2f\n",s);
        for (j=0;j<SCOREP;j++) fprintf(out,"%3.2f ",a[j]);
        fprintf(out,"\n");
        for (j=0;j<SCOREP;j++) fprintf(out,"%3.2f ",weight[j]);
        fprintf(out,"\n");
    }
    
    return s;
    
}

//int main0()
//{
//    //printf("Setmaxstdio: %d\n", _setmaxstdio(2048));
//    struct rlimit rlp;
//    rlp.rlim_cur = 10000;
//    rlp.rlim_max = 10000;
//    setrlimit(RLIMIT_NOFILE, &rlp);
//    
//    srand((unsigned)time(NULL));
//    
//    double stockresult[BMN][10];        //store the results from test
//    double stockavg[11];                //that of benchmark for average calculation
//    double stockm[BN][SMP];				//store the best results
//    initialstockm(stockm);
//    
//    char s1[]=ON;
//    char outname[255];
//    sprintf(outname,"/Users/Troy/Desktop/Phase%d-Analysis-%s.txt",PHASE,s1);
//    FILE *out = fopen(outname, "w");
//    
//    double deviation=0;
//    
//    char schemename[10][255]={"Cut","T. Gain","Threshold","",   "P. Curve","P. Annualgrowth","P. Stocklist",   "P. SNote","P.  CM  ","P.  TL  "};
//    
//    /******Caution:Parameter******/
//    //double scheme[10]={0.06,0.25,3.30,0,  0,0,0,  0,0,0};
//    double scheme[10]={0.06,0.25,3.30,0,  0,1,1,  1,1,1};      // This shows the trade info that I can verify the correctness of a new strategy
//    
//    char benchmark[]=BM;
//    
//    printtime(1,out);
//    printbasicinfo(out,outname);
//    
//    int i;
//    //int j;
//    int schemecounter=0;
//    
////    double testArray[20]={0.2,0.5,0.8,0.9,1,1.1,1.2,1.5,2.0,5.0,10,15,20,30,40,0,0,0,0,0};
////    for (i=0; i<=9; i++) {
////        printf("%f ",testArray[i]);
////        printf("\n");
////    }
//    
//    /******Caution:Parameter******/
//    for (i=0; i<=0; i++)
//        
//         //for (j=1;j<=30;j++)
//    {
//        
//        /******Caution:Parameter******/
//        //scheme[1]=i*0.01;
//        scheme[2]=i*0.01+3.00;
//        //scheme[2]=i/10.0+1;
//        //globalVariable=testArray[i];
//        //globalVariable=i;
//        //printf("\n\nCurrent Test Value: %d\n",globalVariable);
//        
//        //output the parameter table (scheme)
//        printscheme(scheme,schemename,out);
//        
//        //initialization
//        initialresult(stockresult,stockavg);
//        
//        //input scheme and benchmark, output stockresult
//        test(scheme, benchmark, stockresult, out);
//        
//        //calculate stockresult from the test
//        calstockresult(stockresult,stockavg,&deviation);
//        
//        //output in file
//        printfile(stockavg,out,deviation,stockresult);
//        
//        //output on screen
//        printscreen(stockavg);
//        
//        //store in Best
//        
//        /******Caution:Parameter******/
//        storem(scheme[2],deviation,stockavg,stockresult,stockm);
//        
//        //schemecounter
//        schemecounter++;
//        
//    }
//    
//    //print out Best
//    printstockm(out,stockm);
//    
//    fprintf(out,"\n\nTested Scheme#: %d",schemecounter);
//    
//    printtime(2,out);
//    
//    fclose(out);
//    
//    return 0;
//}

double getResult()
{
    //printf("Setmaxstdio: %d\n", _setmaxstdio(2048));
    struct rlimit rlp;
    rlp.rlim_cur = 10000;
    rlp.rlim_max = 10000;
    setrlimit(RLIMIT_NOFILE, &rlp);
    
    srand((unsigned)time(NULL));
    
    double stockresult[BMN][10];        //store the results from test
    double stockavg[11];                //that of benchmark for average calculation
    double stockm[BN][SMP];				//store the best results
    initialstockm(stockm);
    
    char s1[]=ON;
    char outname[255];
    sprintf(outname,"/home/troy/Desktop/Phase%d-Analysis-%s.txt",PHASE,s1);
    FILE *out = fopen(outname, "w");
    
    double deviation=0;
    int i;
    
    char schemename[10][255]={"Cut","T. Gain","Threshold","",   "P. Curve","P. Annualgrowth","P. Stocklist",   "P. SNote","P.  CM  ","P.  TL  "};
    
    /******Caution:Parameter******/
    double scheme[10]={globalstrategy[4],globalstrategy[5],globalstrategy[6],0,  0,0,0,  0,0,0};
    if (globalstrategy[MBS-1]==1) {
        for (i=4; i<10; i++) {
            scheme[i]=1;
        }
    }
    
    
    
    char benchmark[]=BM;
    
    if (globalstrategy[MBS-1]==1) printtime(1,out);
    if (globalstrategy[MBS-1]==1) printbasicinfo(out,outname);
    
    
    //int j;
    int schemecounter=0;
    
    //    double testArray[20]={0.2,0.5,0.8,0.9,1,1.1,1.2,1.5,2.0,5.0,10,15,20,30,40,0,0,0,0,0};
    //    for (i=0; i<=9; i++) {
    //        printf("%f ",testArray[i]);
    //        printf("\n");
    //    }

    /******Caution:Parameter******/
    for (i=0; i<=0; i++)
        
        //for (j=1;j<=30;j++)
    {
        
        /******Caution:Parameter******/
        //scheme[1]=i*0.01;
        //scheme[2]=i*0.01+3.00;
        //scheme[2]=i/10.0+1;
        //globalVariable=testArray[i];
        //globalVariable=i;
        //printf("\n\nCurrent Test Value: %d\n",globalVariable);
        
        //output the parameter table (scheme)
        if (globalstrategy[MBS-1]==1) printscheme(scheme,schemename,out);
        
        //initialization
        initialresult(stockresult,stockavg);
        

        //input scheme and benchmark, output stockresult
        test(scheme, benchmark, stockresult, out);
        
        //calculate stockresult from the test
        calstockresult(stockresult,stockavg,&deviation);
        
        //output in file
        if (globalstrategy[MBS-1]==1) printfile(stockavg,out,deviation,stockresult);
        
        //output on screen
        if (globalstrategy[MBS-1]==1) printscreen(stockavg);
        
        //store in Best
        
        /******Caution:Parameter******/
        if (globalstrategy[MBS-1]==1) storem(scheme[2],deviation,stockavg,stockresult,stockm);
        
        //schemecounter
        if (globalstrategy[MBS-1]==1) schemecounter++;
        
    }
    
    //print out Best
    if (globalstrategy[MBS-1]==1) printstockm(out,stockm);
    
    if (globalstrategy[MBS-1]==1) fprintf(out,"\n\nTested Scheme#: %d",schemecounter);
    
    if (globalstrategy[MBS-1]==1) printtime(2,out);
    
    if (out!=NULL) fclose(out);
    
    return stockavg[4];
}

void initialglobalstrategy()
{
    
    double globalstrategyTemp[MBS] = {
        2.0,0,0,1,0.08,0.30,5.3,3,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,5000000
    };
    
    int i;
    for (i=0; i<MBS; i++) {
        globalstrategy[i]=globalstrategyTemp[i];
    }
    
}

void initialbs(double bs[])
{
    
    int i;
    for (i=0; i<MBS+1; i++) {
        bs[i]=0;
    }
    
}

void updatebs(double strategy[], double result, double bs[])
{
    
    int i;
    for (i=0; i<MBS; i++) {
        bs[i] = strategy[i];
    }
    
    bs[MBS] = result;
    
}

void printbs(double bs[])
{
    
    int i;
    printf("\nbest strategy:\n");
    for (i=0; i<MBS; i++) {
        printf("%.2f\t",bs[i]);
    }
    printf("\t\t%.2f\n",bs[MBS]);
    
}

void printglobalstrategy()
{
    
    int i;
    printf("global strategy:\n");
    for (i=0; i<MBS; i++) {
        printf("%.2f\t",globalstrategy[i]);
    }
    printf("\n");
    
}

void updateglobalstrategy(double bs[])
{
    
    int i;
    for (i=0; i<MBS; i++) {
        globalstrategy[i]=bs[i];
    }
    
}

void initialstrategytable(double strategytable[][STN])
{
    double strategytableTemp[MBS][STN] = {
        //pivot interval
        {0,1,2,3,4,5,6,7,8,-1},
        //eps condition
        {-100,0,0.01,0.02,0.03,0.04,0.05,0.06,0.08,0.10},
        //eps weight
        {0,0.1,0.2,0.3,0.4,0.5,0.8,1.0,1.2,1.5},
        //roe weight
        {0,0.1,0.2,0.3,0.4,0.5,0.8,1.0,1.2,1.5},
        //cut
        {0.01,0.02,0.03,0.04,0.05,0.06,0.08,0.10,0.15,0.20},
        //gain
        {0.01,0.02,0.05,0.10,0.15,0.20,0.25,0.30,0.40,0.50},
        //threshold
        {5.0,5.1,5.2,5.3,5.4,5.5,5.8,6.1,6.5,7.0},
        //stocknum
        {1,2,3,4,5,6,7,8,9,10},
        
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
        
    };
    
    int i,j;
    for (i=0; i<MBS; i++) {
        for (j=0; j<STN; j++) {
            strategytable[i][j] = strategytableTemp[i][j];
        }
    }
    
}

//void initialstrategytable(double strategytable[][STN]) //Caution! This one is for test use!
//{
//    double strategytableTemp[MBS][STN] = {
//        
//        //pivot interval
//        {5,6,-1,-1,-1,-1,-1,-1,-1,-1},
//        //eps condition
//        {0,0.05,-1,-1,-1,-1,-1,-1,-1,-1},
//        
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},
//        
//    };
//    
//    int i,j;
//    for (i=0; i<MBS; i++) {
//        for (j=0; j<STN; j++) {
//            strategytable[i][j] = strategytableTemp[i][j];
//        }
//    }
//    
//}

void greedyhillclimbing()
{
    
    double bs0[MBS+1], bs1[MBS+1];

    //initialize bs0 and bs1
    initialbs(bs0);
    initialbs(bs1);
    
    //test strategy array
    
    //flag indicates if the test should end (get the same result in two rounds), roundcounter counts the round number.
    int flag=0, roundcounter=1;
    int i,j,isResultSame=0;
    double result,result0=0;
    
    //strategytable stores all the test values.
    double strategytable[MBS][STN];
    
    //set strategy table contents
    initialstrategytable(strategytable);
    
    //initialize strategy array (set starting point)
    initialglobalstrategy();
    
    //initial bs1
    updatebs(globalstrategy, 0, bs1);
    
    while (flag!=1) {
        
        //---start round test---
        
        printf("\n---Round %d start.\n",roundcounter);

        for (i=0; i<MBS; i++) {
            
            //update globalstrategy from bs1
            updateglobalstrategy(bs1);
            
            for (j=0; j<STN; j++) {
                
                if (strategytable[i][j]!=-1) {
                    
                    globalstrategy[i] = strategytable[i][j];
                    
                    printf("\nTesting:\n");
                    printglobalstrategy();
                    
                    result = getResult();
                    printf("Result: %.2f\n",result);
                    
                    // check if it is the same as the last time, which means we don't need to verify.
                    if (result0 == result) {
                        isResultSame=1;
                        printf("Result is the same as last time so no verification.\n");
                    } else {
                        isResultSame=0;
                    }
                    result0=result;
                    
                    if ((result>bs1[MBS])&(isResultSame==0)) {
                        
                        // verify by BMN = 11
                        printf("Verifying...\n");
                        BMN = 11;
                        result = getResult();
                        
                        if (result > bs1[MBS]) {
                            
                            updatebs(globalstrategy, result, bs1);
                            printf("\nConguratulations! Benchmark test passed! New bs1 found!\n");
                            printbs(bs1);
                            
                        }
                        
                        // set BMN back to 1
                        BMN = 1;
                        
                    }
                    
                }
                
            }
            
        }
        
        //---round test over---
        
        //after a round test is over, we check if the results are the same.
        if (bs1[MBS]==bs0[MBS]) {
            flag = 1;
        }
        
        // test if the bs1 is better than bs0
        if (bs1[MBS]>bs0[MBS]) {
            //update bs0 by bs1
            updatebs(bs1, bs1[MBS], bs0);
        }
        
        // check if bs1 is worse than bs0 (it is an error)
        if (bs1[MBS]<bs0[MBS]) {
            //throw an error
            printf("ERROR!!! bs1 is smaller than bs0 !!!\n");
        }
        
        printf("\n---Round %d finished. bs1 is as follows:\n",roundcounter);
        printbs(bs1);

        roundcounter++;

    }
    
    //output bs0, both in file and on screen.
    printf("\n===All tests finished. bs0 is as follows:\n");
    printbs(bs0);
    
    //recalculate bs0 and print out all the details
    updateglobalstrategy(bs0);
    globalstrategy[MBS-1] = 1;
    getResult();
    
}

int main()
{
    //hill climbing
    greedyhillclimbing();
    
    //print a specific scheme
    //initialglobalstrategy();
    //BMN=11;
    //globalstrategy[MBS-1] = 1;

    //getResult();
}
