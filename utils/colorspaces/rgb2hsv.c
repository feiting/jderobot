/*
 *  Copyright (C) 2007 Roberto Calvo Palomino
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Authors : Roberto Calvo Palomino <rocapal@gsyc.es> , Jose Maria Caas Plaza <jmplaza@gsyc.es>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#include "colorspaces.h"

struct HSV * LUT_RGB2HSV [64][64][64];

const int MAX_BITS = 8;
const int SIGNIFICATIVE_BITS = 6;
const int MAX_RGB = 255;

/* Condicional variable:
 *   0: The table RGB2HSV don't exists.
 *   1: The table RGB2HSV exists.
 */
int isInitTableHSV;

/* mutex */
pthread_mutex_t mutex;

void rgb2hsv_wiki (double r, double g, double b, double *H, double *S, double *V)
{
	double min, max;

	// Calculamos el minimo
	if ((r <= g) && (r <= b))
		min = r;
	else if ((g <= r) && (g <= b))
		min = g;
	else
		min = b;

	// Calculamos el mximo
	if ((r >= g) && (r >= b))
		max = r;
	else if ((g >= r) && (g >= b))
		max = g;
	else
		max = b;

	//printf("min=%.1f - max=%.1f - r=%.1f - g=%.1f - b=%.1f\n",min,max,r,g,b);
	// Calculamos valor de H
	if (max==min)
	{
		*H=.0;					 // En estos casos, H no tiene sentido
	}
	else if (max==r && g>=b)
	{		
		*H=60*((g-b)/(max-min));
	}
	else if (max==r && g<b)
	{		
		*H=60*((g-b)/(max-min))+360;
	}
	else if (max==g)
	{
		*H=60*((b-r)/(max-min))+120;
	}
	else if (max==b)
	{
		*H=60*((r-g)/(max-min))+240;
	}

	// Calculamos el valor de S
	if (max==0)
		*S=0.0;
	else
		*S= 1-(min/max);

	// Calculamos el valor si V
	*V=max;
}



/// \brief Function to print unsiged int in binary
void print_status(unsigned long status)
{

	//const int BITS_PACK = 4;

	unsigned int t = 8;
	unsigned int i;
	unsigned long int j= 1 << (t - 1);

	for (i= t; i > 0; --i)
	{
		printf("%d", (status & j) != 0);
		j>>= 1;

		//if ((i - 1) % BITS_PACK == 0)
		if (i==3)
			printf(" ");
	}

	printf(" (%lu)\n",status);
}


void RGB2HSV_destroyTable ()
{

	int r,g,b;
	int pos_r, pos_g, pos_b;
	int count = 4;

	printf("Destroy Table LUT_RGB2HSV .... OK\n");

	for (b=0;b<=MAX_RGB;b=b+count)
		for (g=0;g<=MAX_RGB;g=g+count)
			for (r=0;r<=MAX_RGB;r=r+count)
			{
				if (r==0) pos_r=0; else pos_r = r/4;
				if (g==0) pos_g=0; else pos_g = g/4;
				if (b==0) pos_b=0; else pos_b = b/4;

				if (LUT_RGB2HSV[pos_r][pos_g][pos_b])
				{
					free(LUT_RGB2HSV[pos_r][pos_g][pos_b]);
					//RGB2HSI[pos_r][pos_g][pos_b]=NULL;
				}
			}
	pthread_mutex_lock(&mutex);
	isInitTableHSV = 0;
	pthread_mutex_unlock(&mutex);
}


void RGB2HSV_init()
{
	/* Checking exist one instance */
	pthread_mutex_lock(&mutex);
        if (isInitTableHSV==1)
        {
                pthread_mutex_unlock(&mutex);
                return;
        }
        pthread_mutex_unlock(&mutex);

	printf("Init %s v%s ... \n",NAME,VERSION);
	pthread_mutex_lock(&mutex);
	isInitTableHSV = 0;
	pthread_mutex_unlock(&mutex);
}


/// @TODO: Calculate values for create a generic table
void RGB2HSV_createTable()
{

	int r,g,b;
	int count, index;
	int pos_r, pos_g, pos_b;

	struct HSV* newHSV;
	
	/* Checking exist one instance */
	pthread_mutex_lock(&mutex);
	if (isInitTableHSV==1)
	{
		pthread_mutex_unlock(&mutex);
		return;
	}
	pthread_mutex_unlock(&mutex);
	

	count = 4;
	index = 0;

	for (b=0;b<=MAX_RGB;b=b+count)
		for (g=0;g<=MAX_RGB;g=g+count)
			for (r=0;r<=MAX_RGB;r=r+count)
			{
				newHSV = (struct HSV*) malloc(sizeof(struct HSV));
				if (!newHSV)
				{
					printf("Allocated memory error\n");
					exit(-1);
				}

				rgb2hsv_wiki(r,g,b,&(newHSV->H),&(newHSV->S),&(newHSV->V));

				if (r==0) pos_r=0; else pos_r = r/4;
				if (g==0) pos_g=0; else pos_g = g/4;
				if (b==0) pos_b=0; else pos_b = b/4;

				//printf("[%d,%d,%d] RGB=%d,%d,%d - %.1f,%.1f,%.1f \n",pos_r,pos_g,pos_b,r,g,b,newHSI->H,newHSI->S,newHSI->I);
				LUT_RGB2HSV[pos_r][pos_g][pos_b] = newHSV;

				index++;
			}

	printf("Table 'LUT_RGB2HSV' create with 6 bits (%d values)\n",index);
	
	pthread_mutex_lock(&mutex);
	isInitTableHSV=1;
	pthread_mutex_unlock(&mutex);
}


void RGB2HSV_printHSV (struct HSV* hsv)
{
	printf("HSV: %.1f,%.1f,%.1f\n",hsv->H,hsv->S,hsv->V);
}


void RGB2HSV_test (void)
{
	int r,g,b;
	const struct HSV* myHSV=NULL;
	struct HSV myHSV2;
	char line[16];
 
	while (1)
	{

		printf("\nIntroduce R,G,B: ");
		fgets(line,16,stdin);
		if ( sscanf(line,"%d,%d,%d",&r,&g,&b)!= 3)
			break;

		myHSV = RGB2HSV_getHSV(r,g,b);
		
		if (myHSV==NULL)
		{
			printf ("Error in myHSV=NULL\n");
			continue;
		}
		
		printf("[Table] RGB: %d,%d,%d -- HSV: %.1f,%.1f,%.1f\n",r,g,b,myHSV->H,myHSV->S,myHSV->V);

		rgb2hsv_wiki(r,g,b,&myHSV2.H,&myHSV2.S,&myHSV2.V);

		printf("[Algor] RGB: %d,%d,%d -- HSI: %.1f,%.1f,%.1f\n",r,g,b,myHSV2.H,myHSV2.S,myHSV2.V);

	}
}
