#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int day(int i, int j)
{
	switch (j) 
	{
	case 1:
		return 31;
		break; /* optional */
	case 2:
		if ((i % 4 == 0) & (i % 100 != 0)) return 29;
		else return 28;
		break; /* optional */
	case 3:
		return 31;
		break; /* optional */
	case 4:
		return 30;
		break; /* optional */
	case 5:
		return 31;
		break; /* optional */
	case 6:
		return 30;
		break; /* optional */
	case 7:
		return 31;
		break; /* optional */
	case 8:
		return 31;
		break; /* optional */
	case 9:
		return 30;
		break; /* optional */
	case 10:
		return 31;
		break; /* optional */
	case 11:
		return 30;
		break; /* optional */
	case 12:
		return 31;
		break; /* optional */
	
	default: /* Optional */
		return -1;
	}
}

void main()
{
	FILE *f = fopen("/home/troy/Stock Analysis/1Data/1date/date.txt", "w");
	int i,j,k;
	int d,dmax=20160415;

	for (i = 1990; i < 2017; i++)
		for (j = 1; j < 13; j++)
			for (k = 1; k <= day(i, j); k++)
				if ((d= 10000 * i + 100 * j + k)<dmax) fprintf(f, "%d\n", d);
	fclose(f);
}
