/********************************************************************\
 * dialog-pass.c -- dialog for password entry                       *
 * Copyright (C) 2002 Christian Stimming                            *
 * heavily copied from Dave Peticolas <dave@krondo.com>             *
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
\********************************************************************/

#include "config.h"

#include <gnome.h>

#include "dialog-utils.h"
#include "gnc-ui.h"
#include "dialog-pass.h"

gboolean
gnc_hbci_get_password (GtkWidget *parent,
		       const char *heading,
		       const char *initial_password,
		       char **password)
{
  GtkWidget *dialog;
  GtkWidget *heading_label;
  GtkWidget *password_entry;
  GladeXML *xml;
  gint result;

  g_return_val_if_fail (password != NULL, FALSE);

  xml = gnc_glade_xml_new ("hbcipass.glade", "Password Dialog");

  dialog = glade_xml_get_widget (xml, "Password Dialog");

  if (parent)
    gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (parent));

  heading_label  = glade_xml_get_widget (xml, "heading_label");
  password_entry = glade_xml_get_widget (xml, "password_entry");
  g_assert(heading_label && password_entry);

  if (heading)
    gtk_label_set_text (GTK_LABEL (heading_label), heading);

  if (initial_password)
    gtk_entry_set_text (GTK_ENTRY (password_entry), initial_password);

  result = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);

  if (result == GTK_RESPONSE_OK)
  {
    *password = gtk_editable_get_chars (GTK_EDITABLE (password_entry), 0, -1);
    return TRUE;
  }

  *password = NULL;
  return FALSE;
}


gboolean
gnc_hbci_get_initial_password (GtkWidget *parent,
			       const char *heading,
			       char **password)
{
  GtkWidget *dialog;
  GtkWidget *heading_label;
  GtkWidget *password_entry;
  GtkWidget *confirm_entry;
  GladeXML *xml;
  gint result;

  g_return_val_if_fail (password != NULL, FALSE);

  xml = gnc_glade_xml_new ("hbcipass.glade", "Initial Password Dialog");

  dialog = glade_xml_get_widget (xml, "Initial Password Dialog");

  if (parent)
    gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (parent));

  heading_label  = glade_xml_get_widget (xml, "heading_label");
  password_entry = glade_xml_get_widget (xml, "password_entry");
  confirm_entry = glade_xml_get_widget (xml, "confirm_entry");
  g_assert(heading_label && password_entry && confirm_entry);

  if (heading)
    gtk_label_set_text (GTK_LABEL (heading_label), heading);

  while (TRUE) {
    result = gtk_dialog_run (GTK_DIALOG (dialog));
    
    if (result == GTK_RESPONSE_OK)
      {
	char *pw, *confirm;
	pw = gtk_editable_get_chars (GTK_EDITABLE (password_entry), 0, -1);
	confirm = gtk_editable_get_chars (GTK_EDITABLE (confirm_entry), 
					  0, -1);
	if (strcmp (pw, confirm) == 0) {
	  *password = pw;
	  g_free (confirm);
	  gtk_widget_destroy (GTK_WIDGET (dialog));
	  return TRUE;
	}
	g_free (pw);
	g_free (confirm);
      }
    else
      break;

    /* strings didn't match */
    if (gnc_ok_cancel_dialog (parent, GTK_RESPONSE_OK,
			      _("The two passwords didn't match. \n"
				"Please try again."))
	!= GTK_RESPONSE_OK)
      break;
  }
  *password = NULL;
  gtk_widget_destroy (GTK_WIDGET (dialog));
  return FALSE;
}
