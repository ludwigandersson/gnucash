/*
 * FILE:
 * Scrub.h
 *
 * FUNCTION:
 * Provides a set of functions and utilities for scrubbing clean 
 * single-entry accounts so that they can be promoted into 
 * self-consistent, clean double-entry accounts.
 *
 * HISTORY:
 * Created by Linas Vepstas December 1998
 * Copyright (c) 1998 Linas Vepstas
 */

#ifndef __XACC_SCRUB_H__
#define __XACC_SCRUB_H__

#include "Account.h"
#include "Group.h"

/* The ScrubOrphans() methods search for transacations that contain
 *    splits that do not have a parent account. These "orphaned splits"
 *    are placed into an "orphan account" which the user will have to 
 *    go into and clean up.  Kind of like the unix "Lost+Found" directory
 *    for orphaned inodes.
 *
 * The xaccAccountScrubOrphans() method performs this scrub only for the 
 *    indicated account, and not for any of its children.
 *
 * The xaccAccountTreeScrubOrphans() method performs this scrub for the 
 *    indicated account and its children.
 *
 * The xaccGroupScrubOrphans() method performs this scrub for the 
 *    child accounts of this group.
 */
void xaccAccountScrubOrphans (Account *acc);
void xaccAccountTreeScrubOrphans (Account *acc);
void xaccGroupScrubOrphans (AccountGroup *grp);

/* The xaccScrubImbalance() method searches for transactions that do
 *    not balance to zero. If any such transactions are found, a split
 *    is created to offset this amount and is added to an "imbalance"
 *    account.
 */
void xaccAccountScrubImbalance (Account *acc);
void xaccAccountTreeScrubImbalance (Account *acc);
void xaccGroupScrubImbalance (AccountGroup *grp);

#endif /* __XACC_SCRUB_H__ */
