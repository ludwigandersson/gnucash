/********************************************************************\
 * util.c -- utility functions that are used everywhere else for    *
 *           xacc (X-Accountant)                                    *
 * Copyright (C) 1997 Robin D. Clark                                *
 * Copyright (C) 1997-2000 Linas Vepstas <linas@linas.org>          *
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
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
 *   Author: Rob Clark (rclark@cs.hmc.edu)                          *
 *   Author: Linas Vepstas (linas@linas.org)                        *
\********************************************************************/

#include <malloc.h>
#include <math.h>
#include <string.h>
#include <locale.h>
#include <limits.h>
#include <ctype.h>

/* #include <glib.h> */

#include "config.h"
#include "messages.h"
#include "gnc-common.h"
#include "util.h"

/* hack alert -- stpcpy prototype is missing, use -DGNU */
char * stpcpy (char *dest, const char *src);

/** GLOBALS *********************************************************/
/* 
   0 == disable all messages
   1 == enble only error messages
   2 == print warnings
   3 == print info messages
   4 == print debugging messages
 */
int loglevel[MODULE_MAX] =
{0,      /* DUMMY */
 2,      /* ENGINE */
 2,      /* IO */
 2,      /* REGISTER */
 2,      /* LEDGER */
 2,      /* HTML */
 2,      /* GUI */
 2,      /* SCRUB */
 2,      /* GTK_REG */
 2,      /* GUILE */
 4,      /* BACKEND */
 2,      /* QUERY */
};

/********************************************************************\
\********************************************************************/
/* prettify() cleans up subroutine names.
 * AIX/xlC has the habit of printing signatures not names; clean this up.
 * On other operating systems, truncate name to 30 chars.
 * Note this routine is not thread safe. Note we wouldn't need this
 * routine if AIX did something more reasonable.  Hope thread safety
 * doesn't poke us in eye.
 */
char *
prettify (const char *name)
{
   static char bf[35];
   char *p;
   strncpy (bf, name, 29); bf[28] = 0;
   p = strchr (bf, '(');
   if (p)
   {
      *(p+1) = ')';
      *(p+2) = 0x0;
   }
   else
   {
      strcpy (&bf[26], "...()");
   }
   return bf;
}


/********************************************************************\
 * DEBUGGING MEMORY ALLOCATION STUFF                                * 
\********************************************************************/
#if DEBUG_MEMORY

// #if defined (__NetBSD__) || defined(__FreeBSD__)

#ifndef HAVE_MALLOC_USABLE_SIZE
#define malloc_usable_size(ptr) 0
#endif

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
\********************************************************************/

int 
safe_strcmp (const char * da, const char * db)
{
   SAFE_STRCMP (da, db);
   return 0;
}

/********************************************************************\
\********************************************************************/
/* inverse of strtoul */

#define MAX_DIGITS 50

char *
ultostr (unsigned long val, int base)
{
  char buf[MAX_DIGITS];
  unsigned long broke[MAX_DIGITS];
  int i;
  unsigned long places=0, reval;
  
  if ((2>base) || (36<base)) return NULL;

  /* count digits */
  places = 0;
  for (i=0; i<MAX_DIGITS; i++) {
     broke[i] = val;
     places ++;
     val /= base;
     if (0 == val) break;
  }

  /* normalize */
  reval = 0;
  for (i=places-2; i>=0; i--) {
    reval += broke[i+1];
    reval *= base;
    broke[i] -= reval;
  }

  /* print */
  for (i=0; i<places; i++) {
    if (10>broke[i]) {
       buf[places-1-i] = 0x30+broke[i];  /* ascii digit zero */
    } else {
       buf[places-1-i] = 0x41-10+broke[i];  /* ascii capaital A */
    }
  }
  buf[places] = 0x0;

  return strdup (buf);
}

/********************************************************************\
 * utility function to convert floating point value to a string
\********************************************************************/

static int
util_fptostr(char *buf, double val, int prec)
{
  int  i;
  char formatString[10];
  char prefix[]  = "%0.";
  char postfix[] = "f";

  /* This routine can only handle precision between 0 and 9, so
   * clamp precision to that range */
  if (prec > 9) prec = 9;
  if (prec < 0) prec = 0;

  /* Make sure that the output does not resemble "-0.00" by forcing
   * val to 0.0 when we have a very small negative number */
  if ((val <= 0.0) && (val > -pow(0.1, prec+1) * 5.0))
    val = 0.0;

  /* Create a format string to pass into sprintf.  By doing this,
   * we can get sprintf to convert the number to a string, rather
   * than maintaining conversion code ourselves.  */
  i = 0;
  strcpy(&formatString[i], prefix);
  i += strlen(prefix);
  formatString[i] = '0' + prec;  /* add prec to ASCII code for '0' */
  i += 1;
  strcpy(&formatString[i], postfix);
  i += strlen(postfix);

  sprintf(buf, formatString, val);

  return strlen(buf);
}

/********************************************************************\
 * returns GNC_T if the string is a number, possibly with whitespace
\********************************************************************/

gncBoolean
gnc_strisnum(const char *s)
{
  if (s == NULL) return GNC_F;
  if (*s == 0) return GNC_F;

  while (*s && isspace(*s))
    s++;

  if (*s == 0) return GNC_F;
  if (!isdigit(*s)) return GNC_F;

  while (*s && isdigit(*s))
    s++;

  if (*s == 0) return GNC_T;

  while (*s && isspace(*s))
    s++;

  if (*s == 0) return GNC_T;

  return GNC_F;
}

/********************************************************************\
 * stpcpy for those platforms that don't have it.
\********************************************************************/

#if !HAVE_STPCPY
char *
stpcpy (char *dest, const char *src)
{
   strcpy(dest, src);
   return(dest + strlen(src));
}
#endif


/********************************************************************\
 * currency & locale related stuff.
\********************************************************************/

static void
gnc_lconv_set(char **p_value, char *default_value)
{
  char *value = *p_value;

  if ((value == NULL) || (value[0] == 0))
    *p_value = default_value;
}

static void
gnc_lconv_set_char(char *p_value, char default_value)
{
  if ((p_value != NULL) && (*p_value == CHAR_MAX))
    *p_value = default_value;
}

struct lconv *
gnc_localeconv()
{
  static struct lconv lc;
  static gncBoolean lc_set = GNC_F;

  if (lc_set)
    return &lc;

  lc = *localeconv();

  gnc_lconv_set(&lc.decimal_point, ".");
  gnc_lconv_set(&lc.thousands_sep, ",");
  gnc_lconv_set(&lc.int_curr_symbol, "USD ");
  gnc_lconv_set(&lc.currency_symbol, CURRENCY_SYMBOL);
  gnc_lconv_set(&lc.mon_decimal_point, ".");
  gnc_lconv_set(&lc.mon_thousands_sep, ",");
  gnc_lconv_set(&lc.negative_sign, "-");

  gnc_lconv_set_char(&lc.frac_digits, 2);
  gnc_lconv_set_char(&lc.int_frac_digits, 2);
  gnc_lconv_set_char(&lc.p_cs_precedes, 1);
  gnc_lconv_set_char(&lc.p_sep_by_space, 0);
  gnc_lconv_set_char(&lc.n_cs_precedes, 1);
  gnc_lconv_set_char(&lc.n_sep_by_space, 0);
  gnc_lconv_set_char(&lc.p_sign_posn, 1);
  gnc_lconv_set_char(&lc.n_sign_posn, 1);

  lc_set = GNC_T;

  return &lc;
}

char *
gnc_locale_default_currency()
{
  static char currency[4];
  gncBoolean got_it = GNC_F;
  struct lconv *lc;
  int i;

  if (got_it)
    return currency;

  for (i = 0; i < 4; i++)
    currency[i] = 0;

  lc = gnc_localeconv();

  strncpy(currency, lc->int_curr_symbol, 3);

  got_it = GNC_T;

  return currency;
}

/* Utility function for printing non-negative amounts */
static int
PrintAmt(char *buf, double val, int prec,
         gncBoolean use_separators,
         gncBoolean monetary,
         int min_trailing_zeros)
{
  int i, stringLength, numWholeDigits, sepCount;
  struct lconv *lc = gnc_localeconv();
  char tempBuf[50];
  char *bufPtr = buf;

  /* check if we're printing infinity */
  if (!finite(val)) {
    strcpy (buf, "inf");
    return 3;
  }

  if (val < 0.0)
    val = DABS(val);

  util_fptostr(tempBuf, val, prec);

  /* Here we strip off trailing decimal zeros per the argument. */
  if (prec > 0)
  {
    int max_delete;
    char *p;

    max_delete = prec - min_trailing_zeros;

    p = tempBuf + strlen(tempBuf) - 1;

    while ((*p == '0') && (max_delete > 0))
    {
      *p-- = 0;
      max_delete--;
    }

    if (*p == '.')
      *p = 0;
  }

  if (!use_separators)
  {
    /* fix up the decimal place, if there is one */
    stringLength = strlen(tempBuf);
    numWholeDigits = -1;
    for (i = stringLength - 1; i >= 0; i--) {
      if (tempBuf[i] == '.') {
        if (monetary)
          tempBuf[i] = lc->mon_decimal_point[0];
        else
          tempBuf[i] = lc->decimal_point[0];
        break;
      }
    }

    strcpy(buf, tempBuf);
  }
  else
  {
    /* Determine where the decimal place is, if there is one */
    stringLength = strlen(tempBuf);
    numWholeDigits = -1;
    for (i = stringLength - 1; i >= 0; i--) {
      if ((tempBuf[i] == '.') || (tempBuf[i] == lc->decimal_point[0])) {
        numWholeDigits = i;
        if (monetary)
          tempBuf[i] = lc->mon_decimal_point[0];
        else
          tempBuf[i] = lc->decimal_point[0];
        break;
      }
    }

    if (numWholeDigits < 0)
      numWholeDigits = stringLength;  /* Can't find decimal place, it's
                                       * a whole number */

    /* We now know the number of whole digits, now insert separators while
     * copying them from the temp buffer to the destination */
    bufPtr = buf;
    for (i = 0; i < numWholeDigits; i++, bufPtr++) {
      *bufPtr = tempBuf[i];
      sepCount = (numWholeDigits - i) - 1;
      if ((sepCount % 3 == 0) &&
          (sepCount != 0))
      {
        bufPtr++;
        if (monetary)
          *bufPtr = lc->mon_thousands_sep[0];
        else
          *bufPtr = lc->thousands_sep[0];
      }
    }

    strcpy(bufPtr, &tempBuf[numWholeDigits]);
  } /* endif */

  return strlen(buf);
}

int
xaccSPrintAmountGeneral (char * bufp, double val, short shrs, int precision,
                         int min_trailing_zeros, const char *curr_sym)
{
   struct lconv *lc;

   char *orig_bufp = bufp;
   const char *currency_symbol;
   const char *sign;

   char cs_precedes;
   char sep_by_space;
   char sign_posn;

   gncBoolean print_sign = GNC_T;

   if (!bufp) return 0;

   lc = gnc_localeconv();

   if (DEQ(val, 0.0))
     val = 0.0;

   if (shrs & PRTSHR)
   {
     currency_symbol = "shrs";
     cs_precedes = 0;  /* currency symbol follows amount */
     sep_by_space = 1; /* they are separated by a space  */
   }
   else if (shrs & PRTEUR)
   {
     currency_symbol = "EUR";
     cs_precedes = 1;  /* currency symbol precedes amount */
     sep_by_space = 0; /* they are not separated by a space  */
   }
   else
   {
     if (curr_sym == NULL)
     {
       currency_symbol = lc->currency_symbol;
     }
     else
     {
       currency_symbol = curr_sym;
     }

     if (val < 0.0)
     {
       cs_precedes  = lc->n_cs_precedes;
       sep_by_space = lc->n_sep_by_space;
     }
     else
     {
       cs_precedes  = lc->p_cs_precedes;
       sep_by_space = lc->p_sep_by_space;
     }
   }

   if (val < 0.0)
   {
     sign = lc->negative_sign;
     sign_posn = lc->n_sign_posn;
   }
   else
   {
     sign = lc->positive_sign;
     sign_posn = lc->p_sign_posn;
   }

   if ((val == 0.0) || (sign == NULL) || (sign[0] == 0))
     print_sign = GNC_F;

   /* See if we print sign now */
   if (print_sign && (sign_posn == 1))
     bufp = stpcpy(bufp, sign);

   /* Now see if we print currency */
   if (cs_precedes)
   {
     /* See if we print sign now */
     if (print_sign && (sign_posn == 3))
       bufp = stpcpy(bufp, sign);

     if (shrs & PRTSYM)
     {
       bufp = stpcpy(bufp, currency_symbol);
       if (sep_by_space)
         bufp = stpcpy(bufp, " ");
     }

     /* See if we print sign now */
     if (print_sign && (sign_posn == 4))
       bufp = stpcpy(bufp, sign);
   }

   /* Now see if we print parentheses */
   if (print_sign && (sign_posn == 0))
     bufp = stpcpy(bufp, "(");

   /* Now print the value */
   bufp += PrintAmt(bufp, DABS(val), precision, shrs & PRTSEP,
                    !(shrs & PRTNMN), min_trailing_zeros);

   /* Now see if we print parentheses */
   if (print_sign && (sign_posn == 0))
     bufp = stpcpy(bufp, ")");

   /* Now see if we print currency */
   if (!cs_precedes)
   {
     /* See if we print sign now */
     if (print_sign && (sign_posn == 3))
       bufp = stpcpy(bufp, sign);

     if (shrs & PRTSYM)
     {
       if (sep_by_space)
         bufp = stpcpy(bufp, " ");
       bufp = stpcpy(bufp, currency_symbol);
     }

     /* See if we print sign now */
     if (print_sign && (sign_posn == 4))
       bufp = stpcpy(bufp, sign);
   }

   /* See if we print sign now */
   if (print_sign && (sign_posn == 2))
     bufp = stpcpy(bufp, sign);

   /* return length of printed string */
   return (bufp - orig_bufp);
}

int
xaccSPrintAmount (char * bufp, double val, short shrs, const char *curr_code) 
{
   struct lconv *lc;
   int precision;
   int min_trailing_zeros;
   char curr_sym[5];

   lc = gnc_localeconv();

   if (curr_code && (strncmp(curr_code, lc->int_curr_symbol, 3) == 0))
     curr_code = NULL;
   else if (curr_code)
   {
     strncpy(curr_sym, curr_code, 3);
     curr_sym[3] = '\0';
     strcat(curr_sym, " ");
     curr_code = curr_sym;
   }

   if (curr_code && (strncmp(curr_code, "EUR", 3) == 0))
     shrs |= PRTEUR;

   if (shrs & PRTSHR)
   {
     precision = 4;
     min_trailing_zeros = 0;
   }
   else if (shrs & PRTEUR)
   {
     precision = 2;
     min_trailing_zeros = 2;
   }
   else
   {
     precision = lc->frac_digits;
     min_trailing_zeros = lc->frac_digits;
   }

   return xaccSPrintAmountGeneral(bufp, val, shrs, precision,
                                  min_trailing_zeros, curr_code);
}

char *
xaccPrintAmount (double val, short shrs, const char *curr_code) 
{
   /* hack alert -- this is not thread safe ... */
   static char buf[BUFSIZE];

   xaccSPrintAmount (buf, val, shrs, curr_code);

   /* its OK to return buf, since we declared it static */
   return buf;
}

char *
xaccPrintAmountArgs (double val, gncBoolean print_currency_symbol,
                     gncBoolean print_separators, gncBoolean is_shares_value,
                     const char *curr_code)
{
  short shrs = 0;

  if (print_currency_symbol) shrs |= PRTSYM;
  if (print_separators)      shrs |= PRTSEP;
  if (is_shares_value)       shrs |= PRTSHR;

  return xaccPrintAmount(val, shrs, curr_code);
}


/********************************************************************\
 * xaccParseAmount                                                  *
 *   parses amount strings using locale data                        *
 *                                                                  *
 * Args: str      -- pointer to string rep of num                   *
         monetary -- boolean indicating whether value is monetary   *
 * Return: double -- the parsed amount                              *
\********************************************************************/

double xaccParseAmount (const char * instr, gncBoolean monetary)
{
   struct lconv *lc = gnc_localeconv();
   char *mstr, *str, *tok;
   double amount = 0.0;
   char negative_sign;
   char thousands_sep;
   char decimal_point;
   int len;
   int isneg = 0;

   if (!instr) return 0.0;
   mstr = strdup (instr);
   str = mstr;

   negative_sign = lc->negative_sign[0];
   if (monetary)
   {
     thousands_sep = lc->mon_thousands_sep[0];
     decimal_point = lc->mon_decimal_point[0];
   }
   else
   {
     thousands_sep = lc->thousands_sep[0];
     decimal_point = lc->decimal_point[0];
   }

   /* strip off garbage at end of the line */
   tok = strchr (str, '\r');
   if (tok) *tok = 0x0;
   tok = strchr (str, '\n');
   if (tok) *tok = 0x0;

   /* search for a negative sign */
   tok = strchr (str, negative_sign);
   if (tok) {
      isneg = 1;
      str = tok + sizeof(char);
   }

   /* remove thousands separator */
   tok = strchr (str, thousands_sep);
   while (tok) {
      *tok = 0x0;
      amount *= 1000.0;
      amount += ((double) (1000 * atoi (str)));
      str = tok + sizeof(char);
      tok = strchr (str, thousands_sep);
   }

   /* search for a decimal point */
   tok = strchr (str, decimal_point);
   if (tok) {
      *tok = 0x0;
      amount += ((double) (atoi (str)));
      str = tok + sizeof(char);

      /* if there is anything trailing the decimal 
       * point, convert it  */
      if (str[0]) {

         /* strip off garbage at end of the line */
         tok = strchr (str, ' ');
         if (tok) *tok = 0x0;
   
         /* adjust for number of decimal places */
         len = strlen(str);
         if (6 == len) {
            amount += 0.000001 * ((double) atoi (str));
         } else
         if (5 == len) {
            amount += 0.00001 * ((double) atoi (str));
         } else
         if (4 == len) {
            amount += 0.0001 * ((double) atoi (str));
         } else
         if (3 == len) {
            amount += 0.001 * ((double) atoi (str));
         } else
         if (2 == len) {
            amount += 0.01 * ((double) atoi (str));
         } else 
         if (1 == len) {
            amount += 0.1 * ((double) atoi (str));
         } 
      }

   } else {
      amount += ((double) (atoi (str)));
   }

   if (isneg) amount = -amount;

   free (mstr);
   return amount;
}


/********************************************************************\
 * xaccParseQIFAmount                                               * 
 *   parses monetary strings in QIF files                           *
 *                                                                  * 
 * Args:   str -- pointer to string rep of sum                      * 
 * Return: double -- the parsed amount                              * 
 *
 * Note: be careful changing this algorithm.  The Quicken-file-format
 * parser depends a lot on the ability of this routine to do what it's
 * doing.  Don't break it!
\********************************************************************/

/* The following tokens are used to define the US-style monetary
 * strings.  With a bit of cleverness, it should be possible to modify
 * these to handle various international styles ... maybe ... */

#define MINUS_SIGN '-'
#define K_SEP      ','      /* thousands separator */
#define DEC_SEP    '.'      /* decimal point */

double xaccParseQIFAmount (const char * instr) 
{
   char decimal_point = DEC_SEP;
   char thousands_sep = K_SEP;
   char *mstr, *str, *tok;
   double dollars = 0.0;
   int isneg = 0;
   int len;

   if (!instr) return 0.0;
   mstr = strdup (instr);
   str = mstr;

   /* strip off garbage at end of the line */
   tok = strchr (str, '\r');
   if (tok) *tok = 0x0;
   tok = strchr (str, '\n');
   if (tok) *tok = 0x0;

   /* search for a minus sign */
   tok = strchr (str, MINUS_SIGN);
   if (tok) {
      isneg = 1;
      str = tok+sizeof(char);
   }

   /* figure out separators */
   {
     char *tok1, *tok2;

     tok1 = strrchr(str, DEC_SEP);
     tok2 = strrchr(str, K_SEP);

     if (tok1 < tok2)
     {
       decimal_point = K_SEP;
       thousands_sep = DEC_SEP;
     }
   }

   /* remove comma's */
   tok = strchr (str, thousands_sep);
   while (tok) {
      *tok = 0x0;
      dollars *= 1000.0;
      dollars += ((double) (1000 * atoi (str)));
      str = tok+sizeof(char);
      tok = strchr (str, thousands_sep);
   }

   /* search for a decimal point */
   tok = strchr (str, decimal_point);
   if (tok) {
      *tok = 0x0;
      dollars += ((double) (atoi (str)));
      str = tok+sizeof(char);

      /* if there is anything trailing the decimal 
       * point, convert it  */
      if (str[0]) {

         /* strip off garbage at end of the line */
         tok = strchr (str, ' ');
         if (tok) *tok = 0x0;
   
         /* adjust for number of decimal places */
         len = strlen(str);
         if (6 == len) {
            dollars += 0.000001 * ((double) atoi (str));
         } else
         if (5 == len) {
            dollars += 0.00001 * ((double) atoi (str));
         } else
         if (4 == len) {
            dollars += 0.0001 * ((double) atoi (str));
         } else
         if (3 == len) {
            dollars += 0.001 * ((double) atoi (str));
         } else
         if (2 == len) {
            dollars += 0.01 * ((double) atoi (str));
         } else 
         if (1 == len) {
            dollars += 0.1 * ((double) atoi (str));
         } 
      }

   } else {
      dollars += ((double) (atoi (str)));
   }

   if (isneg) dollars = -dollars;

   free (mstr);
   return dollars;
}

/************************* END OF FILE ******************************\
\********************************************************************/
