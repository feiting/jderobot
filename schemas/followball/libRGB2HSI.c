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
 *  Authors : Roberto Calvo Palomino <rocapal@gsyc.es> , Jose Maria Ca�as Plaza <jmplaza@gsyc.es>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libRGB2HSI.h"

const int MAX_BITS = 8;
const int SIGNIFICATIVE_BITS = 6;
const int MAX_RGB = 255;



// Conversor to HSI
void rgb2hsi (double r, double g, double b, double *H, double *S, double *I)
{
  const float PI = 3.141592654;
  double a, n, d;

   // the component I has benn hacked, the result is divided by 255.0
  *I = ((r + b + g) / 3.0)/255.0;
  
  // El minimo
  if ((r <= g) && (r <= b)){
    a = r;
  }
  else if ((g <= r) && (g <= b)){
    a = g;
  }
  else{
    a = b;
  }
  
  if ((r + b + g) == 0){
    *S = 1.0;
  }
  else{
    *S = 1.0 - (3.0 / (r + b + g)) * a;
  }
  
  n = .5 * ((r - g) + (r - b));
  d = sqrt ((r - g) * (r - g) + (r - b) * (g - b));
  if ((d == 0) || (*S == 1) || (*S == 0)){
    // En estos casos *H no tiene sentido
    *H = 0.0;
  }
  else{
    *H = acos (n / d);// Falta medio c�rculo
  }
  if (b < g){
    // C�rculo completo
    *H = (2*PI) - *H;
  }
  
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

void libRGB2HSI_destroyTable ()
{
  
  int r,g,b;
  int pos_r, pos_g, pos_b;
  int count = 4;

  printf("Destroy Table RGB2HSI .... OK\n");
  
  for (b=0;b<=MAX_RGB;b=b+count)
    for (g=0;g<=MAX_RGB;g=g+count)
      for (r=0;r<=MAX_RGB;r=r+count)
	{
	  if (r==0) pos_r=0; else pos_r = r/4;
	  if (g==0) pos_g=0; else pos_g = g/4;
	  if (b==0) pos_b=0; else pos_b = b/4;

	  if (RGB2HSI[pos_r][pos_g][pos_b])
	    {
	      free(RGB2HSI[pos_r][pos_g][pos_b]);
	      //RGB2HSI[pos_r][pos_g][pos_b]=NULL;
	    }
	}
}

void libRGB2HSI_init()
{
  printf("Init %s v%s ... \n",NAME,VERSION);
}

/// @TODO: Calculate values for create a generic table
void libRGB2HSI_createTable()
{
  
  int r,g,b;
  int count, index;
  int pos_r, pos_g, pos_b;

  struct HSI* newHSI;

  count = 4;
  index = 0; 

  for (b=0;b<=MAX_RGB;b=b+count)
    for (g=0;g<=MAX_RGB;g=g+count)
      for (r=0;r<=MAX_RGB;r=r+count)
	{
	  newHSI = (struct HSI*) malloc(sizeof(struct HSI));
	  if (!newHSI)
	    {
	      printf("Allocated memory error\n");
	      exit(-1);
	    }

	  rgb2hsi(r,g,b,&newHSI->H,&newHSI->S,&newHSI->I);

	  if (r==0) pos_r=0; else pos_r = r/4;
	  if (g==0) pos_g=0; else pos_g = g/4;
	  if (b==0) pos_b=0; else pos_b = b/4;

	  //printf("[%d,%d,%d] RGB=%d,%d,%d - %.1f,%.1f,%.1f \n",pos_r,pos_g,pos_b,r,g,b,newHSI->H,newHSI->S,newHSI->I);
	  RGB2HSI[pos_r][pos_g][pos_b] = newHSI;
	    
	  index++;	  
	}

  printf("Table 'RGB2HSI' create with 6 bits (%d values)\n",index);


}
void libRGB2HSI_printHSI (struct HSI* hsi)
{
  printf("HSI: %.1f,%.1f,%.1f\n",hsi->H,hsi->S,hsi->I);
}



void libRGB2HSI_test (void)
{
  int r,g,b;
  struct HSI* myHSI;
  char line[16];

  while (1)
    {
      
      printf("\nIntroduce R-G-B: ");
      gets(line);
      if ( sscanf(line,"%d,%d,%d",&r,&g,&b)!= 3)
	break;
      
      myHSI = libRGB2HSI_getHSI(r,g,b);

      printf("RGB: %d,%d,%d -- HSI: %.1f,%.1f,%.1f\n",r,g,b,myHSI->H,myHSI->S,myHSI->I);

    }
}

