/********************************************************************\
 * util.c -- utility functions that are used everywhere else for    *
 *           xacc (X-Accountant)                                    *
 * Copyright (C) 1997 Robin D. Clark                                *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, write to the Free Software      *
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.        *
 *                                                                  *
 *   Author: Rob Clark                                              *
 * Internet: rclark@cs.hmc.edu                                      *
 *  Address: 609 8th Street                                         *
 *           Huntington Beach, CA 92648-4632                        *
\********************************************************************/

#include "config.h"
#include "messages.h"
#include "util.h"

/** GLOBALS *********************************************************/
int loglevel = 1;

/********************************************************************\
 * DEBUGGING MEMORY ALLOCATION STUFF                                * 
\********************************************************************/
#if DEBUG_MEMORY
size_t core=0;
void
dfree( void *ptr )
  {
  core -= malloc_usable_size(ptr);
  free(ptr);
  }

void*
dmalloc( size_t size )
  {
  int i;
  char *ptr;
  ptr = (char *)malloc(size);
  for( i=0; i<size; i++ )
    ptr[i] = '.';
  
  core +=  malloc_usable_size(ptr);
  return (void *)ptr;
  }

size_t
dcoresize(void)
  {
  return core;
  }
#endif

/********************************************************************\
 * currency & locale related stuff.
 * first attempt at internationalization i18n of currency amounts
 * In the long run, amounts should be printed with punctuation
 * returned from the localconv() subroutine
\********************************************************************/

char * xaccPrintAmount (double val, short shrs) 
{
   static char buf[BUFSIZE];

   if (shrs & PRTSHR) {
      if (shrs & PRTSYM) {
         if (0.0 > val) {
            sprintf( buf, "-%.3f shrs", DABS(val) );
         } else {
            sprintf( buf, " %.3f shrs", val );
         }
      } else {
         if (0.0 > val) {
            sprintf( buf, "-%.3f", DABS(val) );
         } else {
            sprintf( buf, "%.3f", val );
         }
      }
   } else {

      if (shrs & PRTSYM) {
         if (0.0 > val) {
            sprintf( buf, "-%s %.2f", CURRENCY_SYMBOL, DABS(val) );
         } else {
            sprintf( buf, "%s %.2f", CURRENCY_SYMBOL, val );
         }
      } else {
         if (0.0 > val) {
            sprintf( buf, "-%.2f", DABS(val) );
         } else {
            sprintf( buf, "%.2f", val );
         }
      }
   }

   /* its OK to reurn buf, since we declared it static */
   return buf;
}

/************************* END OF FILE ******************************\
\********************************************************************/
