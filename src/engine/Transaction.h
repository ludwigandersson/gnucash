/********************************************************************\
 * Transaction.h -- defines transaction for xacc (X-Accountant)     *
 * Copyright (C) 1997 Robin D. Clark                                *
 * Copyright (C) 1997, 1998 Linas Vepstas                           *
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

#ifndef __XACC_TRANSACTION_H__
#define __XACC_TRANSACTION_H__

#include "config.h"
#include "date.h"   /* for Date */

/* Values for the reconciled field in Transaction: */
#define CREC 'c'              /* The transaction has been cleared        */
#define YREC 'y'              /* The transaction has been reconciled     */
#define FREC 'f'              /* frozen into accounting period           */
#define NREC 'n'              /* not reconciled or cleared               */

/** STRUCTS *********************************************************/
/* The debit & credit pointers are used to implement a double-entry 
 * accounting system.  Basically, the idea with double entry is that
 * there is always an account that is debited, and another that is
 * credited.  These two pointers identify the two accounts. 
 */

/* A split transaction is one which shows up as a credit (or debit) in
 * one account, and peices of it show up as debits (or credits) in other
 * accounts.  Thus, a single credit-card transaction might be split
 * between "dining", "tips" and "taxes" categories.
 */

typedef struct _account       Account;
typedef struct _account_group AccountGroup;
typedef struct _split         Split;
typedef struct _transaction   Transaction;


/** PROTOTYPES ******************************************************/
/*
 * The xaccTransSetDate() method will modify the date of the 
 *    transaction.  It will also make sure that the transaction
 *    is stored in proper date order in the accounts.
 */

Transaction * xaccMallocTransaction (void);       /* mallocs and inits */
void          xaccInitTransaction (Transaction *);/* clears a trans struct */

/* freeTransaction only does so if the transaction is not part of an
 * account. (i.e. if none of the member splits are in an account). */
void          xaccFreeTransaction (Transaction *);

void          xaccTransBeginEdit (Transaction *);
void          xaccTransCommitEdit (Transaction *);

void          xaccTransSetDate (Transaction *, int day, int mon, int year);
void          xaccTransSetDateStr (Transaction *, char *);

/* set the transaction date to the current system time. */
void          xaccTransSetDateToday (Transaction *);

void          xaccTransSetNum (Transaction *, const char *);
void          xaccTransSetDescription (Transaction *, const char *);
void          xaccTransSetMemo (Transaction *, const char *);
void          xaccTransSetAction (Transaction *, const char *);
void          xaccTransSetReconcile (Transaction *, char);

void          xaccTransAppendSplit (Transaction *, Split *);
void          xaccTransRemoveSplit (Transaction *, Split *);

/* 
 * HACK ALERT *** this algorithm is wrong. Needs fixing.
 * The xaccSplitRebalance() routine is an important routine for 
 * maintaining and ensuring that double-entries balance properly.
 * This routine forces the sum-total of the values of all the 
 * splits in a transaction to total up to exactly zero.  
 *
 * It is worthwhile to understand the algorithm that this routine
 * uses to acheive balance.  It goes like this:
 * If the indicated split is a destination split, then the
 * total value of the destination splits is computed, and the
 * value of the source split is adjusted to be minus this amount.
 * (the share price of the source split is not changed).
 * If the indicated split is the source split, then the value
 * of the very first destination split is adjusted so that
 * the blanace is zero.   If there is not destination split,
 * one of two outcomes are possible, depending on whether
 * "forced_double_entry" is enabled or disabled.
 * (1) if forced-double-entry is disabled, the fact that
 *     the destination is missing is ignored.
 * (2) if force-double-entry is enabled, then a destination
 *     split that exactly mirrors the ource split is created,
 *     and credited to the same account as the source split.
 *     Hopefully, the user will notice this, and reparent the 
 *     destination split properly.
 *
 * The xaccTransRebalance() routine merely calls xaccSplitRebalance()
 * on the source split.
 */
 
void xaccSplitRebalance (Split *);
void xaccTransRebalance (Transaction *);

/* ------------- gets --------------- */
/* return pointer to the source split */
Split *       xaccTransGetSourceSplit (Transaction *);
Split *       xaccTransGetDestSplit (Transaction *trans, int i);

char *        xaccTransGetNum (Transaction *);
char *        xaccTransGetDescription (Transaction *trans);
Date *        xaccTransGetDate (Transaction *);
char *        xaccTransGetDateStr (Transaction *);

/* return the number of destination splits */
int           xaccTransCountSplits (Transaction *trans);

/* returns non-zero value if split is source split */
int           xaccTransIsSource (Transaction *, Split *);


Split       * xaccMallocSplit (void);
void          xaccInitSplit   (Split *);    /* clears a split struct */
void          xaccFreeSplit   (Split *);    /* frees memory */
int           xaccCountSplits (Split **sarray);

void          xaccSplitSetMemo (Split *, const char *);
void          xaccSplitSetAction (Split *, const char *);
void          xaccSplitSetReconcile (Split *, char);


/* 
 * The following four functions set the prices and amounts.
 * All of the routines always maintain balance: that is, 
 * invoking any of them will cause other splits in the transaction
 * to be modified so that the net value of the transaction is zero. 
 *
 * The xaccSetShareAmount() method sets the number of shares
 *     that the split should have.  
 *
 * The xaccSetSharePrice() method sets the price of the split.
 *
 * The xaccSetValue() method adjusts the number of shares in 
 *     the split so that the number of shares times the share price
 *     equals the value passed in.
 *
 * The xaccSetSharePriceAndAmount() method will simultaneously
 *     update the share price and the number of shares. This 
 *     is a utility routine that is equivalent to a xaccSplitSetSharePrice()
 *     followed by and xaccSplitSetAmount(), except that it incurs the
 *     processing overhead of balancing only once, instead of twise.
 */

void         xaccSplitSetSharePriceAndAmount (Split *, double price,
                                              double amount);
void         xaccSplitSetShareAmount (Split *, double);
void         xaccSplitSetSharePrice (Split *, double);
void         xaccSplitSetValue (Split *, double);


/* The following four subroutines return the running balance up
 * to & including the indicated split.
 * 
 * The balance is the currency-denominated balance.  For accounts
 * with non-unit share prices, it is correctly adjusted for
 * share prices.
 * 
 * The share-balance is the number of shares. 
 * Price fluctuations do not change the share balance.
 * 
 * The cleared-balance is the currency-denominated balance 
 * of all transactions that have been marked as cleared or reconciled.
 * It is correctly adjusted for price fluctuations.
 * 
 * The reconciled-balance is the currency-denominated balance
 * of all transactions that have been marked as reconciled.
 */

double xaccSplitGetBalance (Split *);
double xaccSplitGetClearedBalance (Split *);
double xaccSplitGetReconciledBalance (Split *);
double xaccSplitGetShareBalance (Split *);

/* return teh parent transaction of the split */
Transaction * xaccSplitGetParent (Split *);

/* return the value of the reconcile flag */
char *        xaccSplitGetMemo (Split *split);
char *        xaccSplitGetAction (Split *split);

char          xaccSplitGetReconcile (Split *split);
double        xaccSplitGetShareAmount (Split * split);
double        xaccSplitGetSharePrice (Split * split);
double        xaccSplitGetValue (Split * split);

Account *     xaccSplitGetAccount (Split *);

/********************************************************************\
 * sorting comparison function
 *
 * returns a negative value if transaction a is dated earlier than b,        
 * returns a positive value if transaction a is dated later than b,
 * returns zero if both transactions are on the same date.
 *
\********************************************************************/

int  xaccTransOrder (Transaction **ta, Transaction **tb);
int  xaccSplitOrder (Split **sa, Split **sb);

/*
 * count the number of transactions in the null-terminated array
 */
int xaccCountTransactions (Transaction **tarray);

/** GLOBALS *********************************************************/

#endif /* __XACC_TRANSACTION_H__ */
