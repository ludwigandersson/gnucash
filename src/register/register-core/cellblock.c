/********************************************************************\
 * cellblock.c -- group of cells that act as cursor within a table  *
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
\********************************************************************/

/*
 * FILE:
 * cellblock.c
 * 
 * FUNCTION:
 * implements a rectangular array of cells. See the header file for
 * additional documentation.
 *
 * HISTORY:
 * Copyright (c) 1998 Linas Vepstas
 * Copyright (c) 2000 Dave Peticolas
 */

#include "config.h"

#include "cellblock.h"

static void gnc_cellblock_init (CellBlock *cellblock, int rows, int cols);


CellBlock *
gnc_cellblock_new (int rows, int cols, const char *cursor_name)
{
  CellBlock *cellblock;

  g_return_val_if_fail (cursor_name != NULL, NULL);

  cellblock = g_new0(CellBlock, 1);

  gnc_cellblock_init (cellblock, rows, cols);

  cellblock->cursor_name = g_strdup (cursor_name);

  return cellblock;
}

static void
gnc_cellblock_cell_construct (gpointer _cb_cell, gpointer user_data)
{
  CellBlockCell *cb_cell = _cb_cell;

  cb_cell->cell = NULL;

  cb_cell->sample_text = NULL;
  cb_cell->alignment = CELL_ALIGN_LEFT;
  cb_cell->expandable = FALSE;
  cb_cell->span = FALSE;
}

static void
gnc_cellblock_cell_destroy (gpointer _cb_cell, gpointer user_data)
{
  CellBlockCell *cb_cell = _cb_cell;

  if (cb_cell == NULL)
    return;

  cb_cell->cell = NULL;

  g_free(cb_cell->sample_text);
  cb_cell->sample_text = NULL;
}

static void
gnc_cellblock_init (CellBlock *cellblock, int rows, int cols)
{
  /* record new size */
  cellblock->num_rows = rows;
  cellblock->num_cols = cols;

  cellblock->start_col = cols;
  cellblock->stop_col  = -1;

  /* malloc new cell table */
  cellblock->cb_cells = g_table_new (sizeof (CellBlockCell),
                                     gnc_cellblock_cell_construct,
                                     gnc_cellblock_cell_destroy, NULL);
  g_table_resize (cellblock->cb_cells, rows, cols);
}

void        
gnc_cellblock_destroy (CellBlock *cellblock)
{
   if (!cellblock) return;

   g_table_destroy (cellblock->cb_cells);
   cellblock->cb_cells = NULL;

   g_free (cellblock->cursor_name);
   cellblock->cursor_name = NULL;

   g_free (cellblock);
}

CellBlockCell *
gnc_cellblock_get_cell (CellBlock *cellblock, int row, int col)
{
  if (cellblock == NULL)
    return NULL;

  return g_table_index (cellblock->cb_cells, row, col);
}

gboolean
gnc_cellblock_changed (CellBlock *cursor, gboolean include_conditional)
{
  int r, c;

  if (!cursor)
    return FALSE;

  for (r = 0; r < cursor->num_rows; r++)
    for (c = 0; c < cursor->num_cols; c++)
    {
      CellBlockCell *cb_cell;

      cb_cell = gnc_cellblock_get_cell (cursor, r, c);
      if (cb_cell == NULL)
        continue;

      if (gnc_basic_cell_get_changed (cb_cell->cell))
        return TRUE;

      if (include_conditional &&
          gnc_basic_cell_get_conditionally_changed (cb_cell->cell))
        return TRUE;
    }

  return FALSE;
}

void
gnc_cellblock_clear_changes (CellBlock *cursor)
{
  int r, c;

  if (!cursor)
    return;

  for (r = 0; r < cursor->num_rows; r++)
    for (c = 0; c < cursor->num_cols; c++)
    {
      CellBlockCell *cb_cell;

      cb_cell = gnc_cellblock_get_cell (cursor, r, c);
      if (cb_cell == NULL)
        continue;

      gnc_basic_cell_set_changed (cb_cell->cell, FALSE);
      gnc_basic_cell_set_conditionally_changed (cb_cell->cell, FALSE);
    }
}

/* --------------- end of file ----------------- */
