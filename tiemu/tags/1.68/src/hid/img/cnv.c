/*  tilp - a linking program for TI graphing calculators
 *  Copyright (C) 1999-2002  Romain Lievin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmt.h"


/**********************************/
/* Conversion & utility functions */ 
/**********************************/

/* Pass from 1 bit/pixel to 1 uint8_t/pixel and fill a colormap */
int convert_bitmap_to_bytemap(Image *img) // tested: OK (12/05)
{
  uint8_t *src = img->bitmap;
  uint8_t *dst = NULL;
  int row, col;
  uint8_t mask, data;
  int bit, pixel;
  uint8_t *cmp;

  //DISPLAY("convert_bitmap_to_bytemap\n");
  img->bytemap = (uint8_t *)malloc(img->width * img->height * sizeof(uint8_t));
  if(img->bytemap == NULL)
    fprintf(stdout, "Malloc error.\n");
  dst = img->bytemap;

  memset(dst, 0, img->width * img->height); // thanks, JB ! -- You're welcome, Roms :-)
  for(row=0; row<img->height; row++)
    {
      for(col=0; col<(img->width)>>3; col++)
	{
	  data = src[((img->width) >> 3) * row + col];
	  mask=0x80;
	  for(bit=0; bit<8; bit++)
	    {
	      pixel = data & mask;
	      if(pixel)
		dst[row*(img->width)+8*col+bit] = 1; // Black entry 
	      else
		dst[row*(img->width)+8*col+bit] = 0; // White entry
	      mask>>=1;
	    }
	}
    }
  //delete_bitmap(img);

  alloc_colormap(img);
  cmp = img->colormap;
  cmp[3*0+0] = 0xff; cmp[3*0+1] = 0xff; cmp[3*0+2] = 0xff;
  cmp[3*1+0] = 0x00; cmp[3*1+2] = 0x00; cmp[3*1+2] = 0x00;

  img->encoding = IMG_COL_TYPE;

  return 0;
}

/* Pass from 1 byte/pixel to 1 bit/pixel (if possible) */
int convert_bytemap_to_bitmap(Image *img) // tested: OK (12/05)
{
  uint8_t *src = img->bytemap;
  uint8_t *dst = NULL;
  int row, col;
  uint8_t mask, data;
  int bit;

  //DISPLAY("convert_bytemap_to_bitmap\n");
  if((img->encoding == IMG_BW_TYPE) || (img->depth > 255) || (img->depth < 2))
    return -1;

  img->bitmap = (uint8_t *)malloc((img->width * img->height) >> 3);
  if(img->bitmap == NULL)
    fprintf(stdout, "Malloc error.\n");
  dst = img->bitmap;

  mask=0;
  for(row=0; row<img->height; row++)
    {
      for(col=0; col<img->width; col+=8)
        {
          data = src[(img->width) * row + col];
	  mask=0;
          for(bit=0; bit<8; bit++)
            {
	      data = src[(img->width) * row + col + bit];
	      if(data) mask |= 1;
	      mask <<= 1;
            }
	  dst[((img->width)<<3)*row+(col<<3)] = mask;
        }
    }
  delete_bytemap(img);
  
  img->encoding = IMG_BW_TYPE;

  return 0;
}

/* Invert the bitmap */
void invert_bitmap(Image *img)
{
  int row, col;
  unsigned char *ptr;

  ptr = img->bitmap;
  if(ptr == NULL) return;

  for(row=0; row<img->height; row++)
    {
      for(col=0; col<img->width >> 3; col++)
	{
	  *ptr = ~(*ptr);
	  ptr++;
	}
    }
}

/* Invert the bytemap */
void invert_bytemap(Image *img) //tested: OK (14/05)
{
  int row, col;
  unsigned char *ptr;

  ptr = img->bytemap;
  if(ptr == NULL) return;

  for(row=0; row<img->height; row++)
    {
      for(col=0; col<img->width; col++)
	{
	  *ptr = ~(*ptr);
	  ptr++;
	}
    }
}

/*
  Try to 'palettize' the image, 256 colors max !
  The source image is in rgbmap and the destination image will be placed
  in bytemap and the palette in colormap
*/
int compute_colormap(Image *img)
{
  unsigned char tab[256];
  int i, k=0;
  unsigned char *src, *dst;
  unsigned char *cmp;
  int r, g, b;
  int row, col;

  //DISPLAY("compute_colormap: ");
  src = img->rgbmap;
  alloc_bytemap(img);
  dst = img->bytemap;
  alloc_colormap(img);
  cmp = img->colormap;

  k = 0;
  for(i=0; i<256; i++)
    tab[i] = 0;
  for(i=0; i<256; i++)
    cmp[i] = 0;

  i = 0;
  for(row=0; row<img->height; row++)
    {
      for(col=0; col<img->width; col++)
	{	
	  r = src[3*(row*(img->width)+col)+0];
	  g = src[3*(row*(img->width)+col)+1];
	  b = src[3*(row*(img->width)+col)+2];
	
	  while(tab[i])
	    { // search an existant colormap entry
	      if( (cmp[3*i+0] == r) && (cmp[3*i+1] == g) && (cmp[3*i+2] == b) )
		{ // found, exit
		  dst[row*(img->width)+col] = i;
		  break;
		}
	      else
		i++;
	    }
	  if(!tab[i])
	    { // new color -> put in palette
	      cmp[3*i+0] = r;
	      cmp[3*i+1] = g;
	      cmp[3*i+2] = b;
	      tab[i] = 1;
	      k++;
	      if(k>255)
		{
		  fprintf(stderr, "Too many colors (<256)\n");
		  img->depth = k;
		  return 1;
		}
	    }
	  i = 0;
	}
    }
  
  img->depth = k;
  //DISPLAY("%i colors\n", k);
  delete_rgbmap(img);  
  return 0;
}

/* 
   Convert the rgbmap buffer to a bytemap+colormap buffer 
   (true colors -> colormapped)
*/
int convert_rgbmap_to_bytemap(Image *img)
{
  //DISPLAY("rgbmap_to_bytemap\n");
  compute_colormap(img);

  return 0;
}

/* Convert the bytemap buffer to a rrggbb buffer (colormapped -> true colors) */
int convert_bytemap_to_rgbmap(Image *img) // tested: OK (14/05)
{
  int row, col;
  uint8_t *src, *dst, *cmp;
  int r, g, b;

  cmp = img->colormap;
  src = img->bytemap;
  alloc_rgbmap(img);
  dst = img->rgbmap;

  // could be greatly optimized using memset...
  //memset(dts, 0, (img->width * img->heigth));

  for(row=0; row<img->height; row++)
    {
      for(col=0; col<img->width; col++)
	{
	  r = cmp[3*src[row*img->width+col]+0];
	  g = cmp[3*src[row*img->width+col]+1];
	  b = cmp[3*src[row*img->width+col]+2];
	  
	  dst[3*(row*img->width+col)+0] = r;
	  dst[3*(row*img->width+col)+1] = g;
	  dst[3*(row*img->width+col)+2] = b;
	}
    }
  delete_colormap(img);
  delete_bytemap(img);

  return 0;
}


/************************/
/* Allocation & freeing */
/************************/

/* Destroy the bitmap */
void delete_bitmap(Image *img)
{
  if(img->bitmap != NULL)
    {
      free(img->bitmap);
      img->bitmap = NULL;
    }
}

void alloc_bitmap(Image *img)
{
  //(img->bitmap) = (uint8_t *)malloc(((img->width) * (img->height)) >> 3);
	(img->bitmap) = (uint8_t *)calloc(((img->width) * (img->height)) >> 3, sizeof(uint8_t));
  if(img->bitmap == NULL)
    exit(-1);
}

/* Destroy the bytemap */
void delete_bytemap(Image *img)
{
  //DISPLAY("delete_bytemap\n");
	if(img->bytemap != NULL)
    {
      free(img->bytemap);
      img->bytemap = NULL;
    }
}

void alloc_bytemap(Image *img)
{
  //(img->bytemap) = (uint8_t *)malloc((img->width) * (img->height));
	(img->bytemap) = (uint8_t *)calloc((img->width) * (img->height), sizeof(uint8_t));
  if(img->bytemap == NULL)
    exit(-1);
}

/* Destroy the colormap(Image *img) */
void delete_colormap(Image *img)
{
  //DISPLAY("delete_colormap\n");
  if(img->colormap != NULL)
    {
      free(img->colormap);
      img->colormap = NULL;
    }
}

void alloc_colormap(Image *img)
{
  //img->colormap = (uint8_t *)malloc(3*256 * sizeof(uint8_t));
	img->colormap = (uint8_t *)calloc(3*256, sizeof(uint8_t));
  if(img->colormap == NULL)
    exit(-1);
}

/* Destroy the pixmap created with the previous function */
void delete_pixmap(Image *img)
{
  int i = 0;
  char **pixmap = img->pixmap;
  
  //DISPLAY("delete_pixmap\n");
  if(pixmap == NULL) return;
  for(i=0; pixmap[i] != NULL; i++)
    {
      if(pixmap[i] != NULL) free(pixmap[i]);
    }
  free(pixmap);
  pixmap = NULL;
}

/* Destroy the bytemap */
void delete_rgbmap(Image *img)
{
  //DISPLAY("delete_rgbmap\n");
  if(img->rgbmap != NULL)
    {
      free(img->rgbmap);
      img->rgbmap = NULL;
    }
}

/* Allocate an RGB buffer (rrggbb) */
void alloc_rgbmap(Image *img)
{
  //(img->rgbmap) = (uint8_t *)malloc(3 * (img->width) * (img->height));
	(img->rgbmap) = (uint8_t *)calloc(3 * (img->width) * (img->height), sizeof(uint8_t));
  if(img->rgbmap == NULL)
    exit(-1);
}

/* Destroy the entire image 
	Note: the bitmap must NOT be deleted. This is done by 
	the libticables library.
*/
void free_image(Image *img)
{
  //DISPLAY("delete_image\n");
  //delete_bitmap(img);
  delete_bytemap(img);
  delete_colormap(img);
  delete_pixmap(img);
  delete_rgbmap(img);
}

void delete_image(Image *img)
{
  free_image(img);
}


/* Blurry a bitmap by transforming the bitmap into a bytemap
   and modifying colormap entries
*/
int blurry_bitmap(Image *img) // tested: OK (09/04/2002)
{
  int err;
  uint8_t *cmp;

  err = convert_bitmap_to_bytemap(img);

  cmp = img->colormap;

  cmp[3*0+0] = 0xa8; cmp[3*0+1] = 0xb4; cmp[3*0+2] = 0xa8;
  cmp[3*1+0] = 0x00; cmp[3*1+1] = 0x00; cmp[3*1+2] = 0x34;

  return err;
}

void clear_image(Image *img)
{
  memset((void *)img, 0, sizeof(Image));
}
