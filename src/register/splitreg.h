/********************************************************************\
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
\********************************************************************/

/*
 * FILE:
 * splitreg.h
 *
 * FUNCTION:
 * Implements a basic display register/ledger.
 * This object makes specific cells have specific properties
 * (price, text, date, etc.) and specific names that correspond.
 * It also determines the actual physical layout, arrangement
 * of columns, etc.
 *
 * Handles splits
 *
 * Hack alert -- finish documenting this
 *
 * The xaccConfigSplitRegister() subroutine allows the configuration 
 * of the register to be changed on the fly (dynamically).  In particular,
 * the register type, and/or the flags controlling the register display
 * can be changed on the fly. 
 *
 * DESIGN HOPES:
 * Should probably move at least some of the layout to a config 
 * file.  Might make good sense to use scheme/guile for the layout.
 *
 * HISTORY:
 * Copyright (c) 1998, 1999, 2000 Linas Vepstas
 */

#ifndef __XACC_SPLITREG_H__
#define __XACC_SPLITREG_H__

#include "basiccell.h"
#include "cellblock.h"
#include "combocell.h"
#include "datecell.h"
#include "quickfillcell.h"
#include "pricecell.h"
#include "numcell.h"
#include "recncell.h"
#include "table-allgui.h"

/* defined register types */
/* "registers" are single-account display windows.
 * "ledgers" are multiple-account display windows */
typedef enum
{
  BANK_REGISTER       =  1,
  CASH_REGISTER       =  2,
  ASSET_REGISTER      =  3,
  CREDIT_REGISTER     =  4,
  LIABILITY_REGISTER  =  5,
  INCOME_REGISTER     =  6,
  EXPENSE_REGISTER    =  7,
  EQUITY_REGISTER     =  8,
  STOCK_REGISTER      =  9,
  CURRENCY_REGISTER   = 10,

  GENERAL_LEDGER      = 11,
  INCOME_LEDGER       = 12,
  PORTFOLIO_LEDGER    = 13,
  SEARCH_LEDGER       = 14
} SplitRegisterType;

/* These values are used to identify the cells in the register. */
typedef enum
{
  NO_CELL    = -1,
  DATE_CELL  =  0,
  NUM_CELL   =  1,
  DESC_CELL  =  2,
  RECN_CELL  =  3,
  SHRS_CELL  =  4,
  BALN_CELL  =  5,
  ACTN_CELL  =  6,
  XFRM_CELL  =  7,
  XTO_CELL   =  8,
  MEMO_CELL  =  9,
  CRED_CELL  = 10,
  DEBT_CELL  = 11,
  PRIC_CELL  = 12,
  VALU_CELL  = 13,

  /* NCRED & NDEBT handle minus the usual quantities */
  NCRED_CELL = 14,
  NDEBT_CELL = 15,

  /* MXFRM is the "mirrored" transfer-from account */
  MXFRM_CELL = 16
} CellType;

/*
 * enumerated display styles 
 * REG_SINGLE_LINE    -- show one line per transaction
 * REG_DOUBLE_LINE    -- show two lines per transaction
 * REG_MULTI_LINE     -- show multiple lines per transaction
 * REG_SINGLE_DYNAMIC -- dynamically expand edited transaction,
 *                       all other transactions on one line
 * REG_DOUBLE_DYNAMIC -- dynamically expand edited transaction,
 *                       all other transactions on two lines
 */
typedef enum
{
  REG_SINGLE_LINE    = 1,
  REG_DOUBLE_LINE    = 2,
  REG_MULTI_LINE     = 3,
  REG_SINGLE_DYNAMIC = 4,
  REG_DOUBLE_DYNAMIC = 5
} SplitRegisterStyle;

/* modified flags -- indicate which cell values have been modified by user */
#define MOD_NONE   0x0000
#define MOD_DATE   0x0001
#define MOD_NUM    0x0002
#define MOD_DESC   0x0004
#define MOD_RECN   0x0008

#define MOD_ACTN   0x0010
#define MOD_XFRM   0x0020
#define MOD_MXFRM  0x0040
#define MOD_XTO    0x0080
#define MOD_MEMO   0x0100
#define MOD_AMNT   0x0200
#define MOD_NAMNT  0x0400
#define MOD_PRIC   0x0800
#define MOD_VALU   0x1000
#define MOD_NEW    0x2000
#define MOD_ALL    0x3fff

/* Types of cursors */
typedef enum
{
  CURSOR_SPLIT,
  CURSOR_TRANS,
  CURSOR_NONE
} CursorType;

/* The value of NUM_CELLS should be larger than the number of 
 * cells defined in the structure below!
 */
#define NUM_CELLS 25

typedef struct _SplitRegisterBuffer SplitRegisterBuffer;
typedef struct _SplitRegister SplitRegister;

struct _SplitRegister {
   /* the table itself that implements the underlying GUI. */
   Table         * table;

   /* the cursors that define the currently edited row */
   CellBlock     * single_cursor;
   CellBlock     * double_cursor;
   CellBlock     * trans_cursor;
   CellBlock     * split_cursor;
   CellBlock     * header;

   DateCell      * dateCell;
   NumCell       * numCell;
   QuickFillCell * descCell;
   RecnCell      * recnCell;   /* main transaction line reconcile */
   PriceCell     * shrsCell;
   PriceCell     * balanceCell;
   BasicCell     * nullCell;

   ComboCell     * actionCell;
   ComboCell     * xfrmCell;
   ComboCell     * mxfrmCell;
   ComboCell     * xtoCell;
   QuickFillCell * memoCell;
   PriceCell     * creditCell;
   PriceCell     * debitCell;
   PriceCell     * priceCell;
   PriceCell     * valueCell;

   PriceCell     * ncreditCell;
   PriceCell     * ndebitCell;

   /* The type of the register, must be one of the enumerated types
    * named *_REGISTER, *_LEDGER, above */
   SplitRegisterType type;

   SplitRegisterStyle style;

   /* some private data; outsiders should not access this */
   int num_cols;
   int num_header_rows;

   int num_phys_rows;
   int num_virt_rows;

   int cursor_phys_row;
   int cursor_virt_row;

   BasicCell *header_label_cells[NUM_CELLS];

   /* user_data allows users of this object to hang
    * private data onto it */
   void *user_data;

   /* The destroy callback gives user's a chance 
    * to free up any associated user_hook data */
   void (* destroy) (SplitRegister *);
};


typedef struct _SplitRegisterColors SplitRegisterColors;

struct _SplitRegisterColors
{
  guint32 single_cursor_active_bg_color;
  guint32 single_cursor_passive_bg_color;
  guint32 single_cursor_passive_bg_color2;

  guint32 double_cursor_active_bg_color;
  guint32 double_cursor_passive_bg_color;
  guint32 double_cursor_passive_bg_color2;

  gboolean double_alternate_virt;

  guint32 trans_cursor_active_bg_color;
  guint32 trans_cursor_passive_bg_color;

  guint32 split_cursor_active_bg_color;
  guint32 split_cursor_passive_bg_color;

  guint32 header_bg_color;
};

typedef char* (*SRStringGetter) (SplitRegisterType);

void            xaccSplitRegisterSetDebitStringGetter(SRStringGetter getter);
void            xaccSplitRegisterSetCreditStringGetter(SRStringGetter getter);

SplitRegister * xaccMallocSplitRegister (SplitRegisterType type,
                                         SplitRegisterStyle style);
void            xaccInitSplitRegister (SplitRegister *reg,
                                       SplitRegisterType type,
                                       SplitRegisterStyle style);
void            xaccConfigSplitRegister (SplitRegister *reg,
                                         SplitRegisterType type,
                                         SplitRegisterStyle style);
void            xaccDestroySplitRegister (SplitRegister *reg);

void            xaccSetSplitRegisterColors (SplitRegisterColors reg_colors);
void            xaccSplitRegisterConfigColors (SplitRegister *reg);

/* returns non-zero value if updates have been made to data */
guint32         xaccSplitRegisterGetChangeFlag (SplitRegister *reg);

/* Clears all change flags in the register. Does not alter values */
void            xaccSplitRegisterClearChangeFlag (SplitRegister *reg);

/* Returns the type of the current cursor */
CursorType      xaccSplitRegisterGetCursorType (SplitRegister *reg);

/* Returns the type of the cursor at the given virtual row and column. */
CursorType      xaccSplitRegisterGetCursorTypeRowCol (SplitRegister *reg,
                                                      int virt_row,
                                                      int virt_col);
/* Returns the type of the current cell */
CellType        xaccSplitRegisterGetCellType (SplitRegister *reg);

/* Returns the type of the cell at the given physical row and column. */
CellType        xaccSplitRegisterGetCellTypeRowCol (SplitRegister *reg,
                                                    int phys_row,
                                                    int phys_col);

/* Returns the physical row and column in the current cursor of the
 * given cell using the pointer values. The function returns true if
 * the given cell type is in the current cursor, false otherwise. */
gboolean        xaccSplitRegisterGetCellRowCol (SplitRegister *reg,
                                                CellType cell_type,
                                                int *p_phys_row,
                                                int *p_phys_col);

/* Functions for working with split register buffers */
SplitRegisterBuffer * xaccMallocSplitRegisterBuffer ();
void xaccDestroySplitRegisterBuffer (SplitRegisterBuffer *srb);

void xaccSplitRegisterSaveCursor(SplitRegister *sr, SplitRegisterBuffer *srb);
void xaccSplitRegisterRestoreCursorChanged(SplitRegister *sr,
                                           SplitRegisterBuffer *srb);


#endif /* __XACC_SPLITREG_H__ */

/* ============ END OF FILE ===================== */
