/*
 * gnc-amount-edit.h -- amount editor widget
 *
 * Copyright (C) 2000 Dave Peticolas <dave@krondo.com>
 * All rights reserved.
 *
 * GnuCash is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Gnucash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, contact:
 *
 * Free Software Foundation           Voice:  +1-617-542-5942
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652
 * Boston, MA  02111-1307,  USA       gnu@gnu.org
 *
 */
/*
  @NOTATION@
 */

#ifndef GNC_AMOUNT_EDIT_H
#define GNC_AMOUNT_EDIT_H 

#include <gnome.h>

#include "gnc-numeric.h"
#include "gnc-ui-util.h"

BEGIN_GNOME_DECLS


#define GNC_AMOUNT_EDIT(obj)          GTK_CHECK_CAST (obj, gnc_amount_edit_get_type(), GNCAmountEdit)
#define GNC_AMOUNT_EDIT_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, gnc_amount_edit_get_type(), GNCAmountEditClass)
#define GNC_IS_AMOUNT_EDIT(obj)       GTK_CHECK_TYPE (obj, gnc_amount_edit_get_type ())

typedef struct
{
  GtkHBox hbox;

  GtkWidget *amount_entry;

  gboolean need_to_parse;

  GNCPrintAmountInfo print_info;

  gnc_numeric amount;

  int fraction;

  gboolean evaluate_on_enter;

} GNCAmountEdit;

typedef struct
{
  GtkHBoxClass parent_class;
  void (*amount_changed) (GNCAmountEdit *gae);
} GNCAmountEditClass;

guint     gnc_amount_edit_get_type        (void);

GtkWidget *gnc_amount_edit_new            (void);

GtkWidget *gnc_amount_edit_gtk_entry      (GNCAmountEdit *gae);

void      gnc_amount_edit_set_amount      (GNCAmountEdit *gae,
                                           gnc_numeric amount);
void      gnc_amount_edit_set_damount     (GNCAmountEdit *gae,
                                           double amount);

gnc_numeric gnc_amount_edit_get_amount    (GNCAmountEdit *gae);
double      gnc_amount_edit_get_damount   (GNCAmountEdit *gae);

gboolean  gnc_amount_edit_evaluate        (GNCAmountEdit *gae);

void      gnc_amount_edit_set_print_info  (GNCAmountEdit *gae,
                                           GNCPrintAmountInfo print_info);

void      gnc_amount_edit_set_fraction    (GNCAmountEdit *gae, int fraction);

void      gnc_amount_edit_set_evaluate_on_enter (GNCAmountEdit *gae,
                                                 gboolean evaluate_on_enter);

END_GNOME_DECLS

#endif
