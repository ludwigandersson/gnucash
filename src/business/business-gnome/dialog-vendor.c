/*
 * dialog-vendor.c -- Dialog for Vendor entry
 * Copyright (C) 2001 Derek Atkins
 * Author: Derek Atkins <warlord@MIT.EDU>
 */

#include "config.h"

#include <gnome.h>

#include "dialog-utils.h"
#include "global-options.h"
#include "gnc-currency-edit.h"
#include "gnc-component-manager.h"
#include "gnc-ui.h"
#include "gnc-gui-query.h"
#include "gnc-ui-util.h"
#include "gnc-engine-util.h"
#include "window-help.h"
#include "dialog-search.h"
#include "search-param.h"

#include "gncAddress.h"
#include "gncVendor.h"
#include "gncVendorP.h"

#include "business-gnome-utils.h"
#include "dialog-vendor.h"
#include "dialog-job.h"
#include "dialog-order.h"
#include "dialog-invoice.h"
#include "dialog-payment.h"

#define DIALOG_NEW_VENDOR_CM_CLASS "dialog-new-vendor"
#define DIALOG_EDIT_VENDOR_CM_CLASS "dialog-edit-vendor"

typedef enum
{
  NEW_VENDOR,
  EDIT_VENDOR
} VendorDialogType;

struct _vendor_select_window {
  GNCBook *	book;
  QueryNew *	q;
};

struct _vendor_window {
  GtkWidget *	dialog;

  GtkWidget *	id_entry;
  GtkWidget *	company_entry;

  GtkWidget *	name_entry;
  GtkWidget *	addr1_entry;
  GtkWidget *	addr2_entry;
  GtkWidget *	addr3_entry;
  GtkWidget *	addr4_entry;
  GtkWidget *	phone_entry;
  GtkWidget *	fax_entry;
  GtkWidget *	email_entry;
  GtkWidget *	terms_menu;
  GtkWidget *	currency_edit;

  GtkWidget *	active_check;
  GtkWidget *	taxincluded_menu;
  GtkWidget *	notes_text;

  GtkWidget *	taxtable_check;
  GtkWidget *	taxtable_menu;

  GncTaxIncluded taxincluded;
  GncBillTerm *	terms;
  VendorDialogType	dialog_type;
  GUID		vendor_guid;
  gint		component_id;
  GNCBook *	book;
  GncVendor *	created_vendor;

  GncTaxTable *	taxtable;
};

static void
gnc_vendor_taxtable_check_cb (GtkToggleButton *togglebutton,
				gpointer user_data)
{
  VendorWindow *vw = user_data;

  if (gtk_toggle_button_get_active (togglebutton))
    gtk_widget_set_sensitive (vw->taxtable_menu, TRUE);
  else
    gtk_widget_set_sensitive (vw->taxtable_menu, FALSE);
}

static GncVendor *
vw_get_vendor (VendorWindow *vw)
{
  if (!vw)
    return NULL;

  return gncVendorLookup (vw->book, &vw->vendor_guid);
}

static void gnc_ui_to_vendor (VendorWindow *vw, GncVendor *vendor)
{
  GncAddress *addr;

  addr = gncVendorGetAddr (vendor);

  gnc_suspend_gui_refresh ();
  gncVendorBeginEdit (vendor);

  gncVendorSetID (vendor, gtk_editable_get_chars
		    (GTK_EDITABLE (vw->id_entry), 0, -1));
  gncVendorSetName (vendor, gtk_editable_get_chars
		      (GTK_EDITABLE (vw->company_entry), 0, -1));

  gncAddressSetName (addr, gtk_editable_get_chars
		     (GTK_EDITABLE (vw->name_entry), 0, -1));
  gncAddressSetAddr1 (addr, gtk_editable_get_chars
		      (GTK_EDITABLE (vw->addr1_entry), 0, -1));
  gncAddressSetAddr2 (addr, gtk_editable_get_chars
		      (GTK_EDITABLE (vw->addr2_entry), 0, -1));
  gncAddressSetAddr3 (addr, gtk_editable_get_chars
		      (GTK_EDITABLE (vw->addr3_entry), 0, -1));
  gncAddressSetAddr4 (addr, gtk_editable_get_chars
		      (GTK_EDITABLE (vw->addr4_entry), 0, -1));
  gncAddressSetPhone (addr, gtk_editable_get_chars
		      (GTK_EDITABLE (vw->phone_entry), 0, -1));
  gncAddressSetFax (addr, gtk_editable_get_chars
		    (GTK_EDITABLE (vw->fax_entry), 0, -1));
  gncAddressSetEmail (addr, gtk_editable_get_chars
		      (GTK_EDITABLE (vw->email_entry), 0, -1));

  gncVendorSetActive (vendor, gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON (vw->active_check)));
  gncVendorSetTaxIncluded (vendor, vw->taxincluded);
  gncVendorSetNotes (vendor, gtk_editable_get_chars
		       (GTK_EDITABLE (vw->notes_text), 0, -1));
  gncVendorSetTerms (vendor, vw->terms);
  gncVendorSetCurrency (vendor,
			gnc_currency_edit_get_currency (GNC_CURRENCY_EDIT
							   (vw->currency_edit)));

  gncVendorSetTaxTableOverride
    (vendor, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (vw->taxtable_check)));
  gncVendorSetTaxTable (vendor, vw->taxtable);

  gncVendorCommitEdit (vendor);
  gnc_resume_gui_refresh ();
}

static gboolean check_entry_nonempty (GtkWidget *dialog, GtkWidget *entry, 
				      const char * error_message)
{
  const char *res = gtk_entry_get_text (GTK_ENTRY (entry));
  if (safe_strcmp (res, "") == 0) {
    if (error_message)
      gnc_error_dialog_parented (GTK_WINDOW (dialog), "%s", error_message);
    return TRUE;
  }
  return FALSE;
}

static void
gnc_vendor_window_ok_cb (GtkWidget *widget, gpointer data)
{
  VendorWindow *vw = data;

  /* Check for valid company name */
  if (check_entry_nonempty (vw->dialog, vw->company_entry,
		   _("You must enter a company name.\n"
		     "If this vendor is an individual (and not a company) "
		     "you should set the \"company name\" and \"contact name\" "
		     "the same.")))
    return;

  /* Make sure we have an address */
  if (check_entry_nonempty (vw->dialog, vw->addr1_entry, NULL) &&
      check_entry_nonempty (vw->dialog, vw->addr2_entry, NULL) &&
      check_entry_nonempty (vw->dialog, vw->addr3_entry, NULL) &&
      check_entry_nonempty (vw->dialog, vw->addr4_entry, NULL)) {
    const char *msg = _("You must enter a payment address.");
    gnc_error_dialog_parented (GTK_WINDOW (vw->dialog), msg);
    return;
  }

  /* Check for valid id and set one if necessary */
  if (safe_strcmp (gtk_entry_get_text (GTK_ENTRY (vw->id_entry)), "") == 0)
    gtk_entry_set_text (GTK_ENTRY (vw->id_entry),
			g_strdup_printf ("%.6lld",
					 gncVendorNextID(vw->book)));

  /* Now save it off */
  {
    GncVendor *vendor = vw_get_vendor (vw);
    if (vendor) {
      gnc_ui_to_vendor (vw, vendor);
    }
    vw->created_vendor = vendor;
    vw->vendor_guid = *xaccGUIDNULL ();
  }

  gnc_close_gui_component (vw->component_id);
}

static void
gnc_vendor_window_cancel_cb (GtkWidget *widget, gpointer data)
{
  VendorWindow *vw = data;

  gnc_close_gui_component (vw->component_id);
}

static void
gnc_vendor_window_help_cb (GtkWidget *widget, gpointer data)
{
  char *help_file = HH_VENDOR;

  helpWindow(NULL, NULL, help_file);
}

static void
gnc_vendor_window_destroy_cb (GtkWidget *widget, gpointer data)
{
  VendorWindow *vw = data;
  GncVendor *vendor = vw_get_vendor (vw);

  gnc_suspend_gui_refresh ();

  if (vw->dialog_type == NEW_VENDOR && vendor != NULL) {
    gncVendorBeginEdit (vendor);
    gncVendorDestroy (vendor);
    vw->vendor_guid = *xaccGUIDNULL ();
  }

  gnc_unregister_gui_component (vw->component_id);
  gnc_resume_gui_refresh ();

  g_free (vw);
}

static void
gnc_vendor_name_changed_cb (GtkWidget *widget, gpointer data)
{
  VendorWindow *vw = data;
  char *name, *id, *fullname, *title;

  if (!vw)
    return;

  name = gtk_entry_get_text (GTK_ENTRY (vw->company_entry));
  if (!name || *name == '\0')
    name = _("<No name>");

  id = gtk_entry_get_text (GTK_ENTRY (vw->id_entry));

  fullname = g_strconcat (name, " (", id, ")", NULL);

  if (vw->dialog_type == EDIT_VENDOR)
    title = g_strconcat (_("Edit Vendor"), " - ", fullname, NULL);
  else
    title = g_strconcat (_("New Vendor"), " - ", fullname, NULL);

  gtk_window_set_title (GTK_WINDOW (vw->dialog), title);

  g_free (fullname);
  g_free (title);
}

static void
gnc_vendor_window_close_handler (gpointer user_data)
{
  VendorWindow *vw = user_data;

  gnome_dialog_close (GNOME_DIALOG (vw->dialog));
}

static void
gnc_vendor_window_refresh_handler (GHashTable *changes, gpointer user_data)
{
  VendorWindow *vw = user_data;
  const EventInfo *info;
  GncVendor *vendor = vw_get_vendor (vw);

  /* If there isn't a vendor behind us, close down */
  if (!vendor) {
    gnc_close_gui_component (vw->component_id);
    return;
  }

  /* Next, close if this is a destroy event */
  if (changes) {
    info = gnc_gui_get_entity_events (changes, &vw->vendor_guid);
    if (info && (info->event_mask & GNC_EVENT_DESTROY)) {
      gnc_close_gui_component (vw->component_id);
      return;
    }
  }
}

static gboolean
find_handler (gpointer find_data, gpointer user_data)
{
  const GUID *vendor_guid = find_data;
  VendorWindow *vw = user_data;

  return(vw && guid_equal(&vw->vendor_guid, vendor_guid));
}

static VendorWindow *
gnc_vendor_new_window (GNCBook *bookp, GncVendor *vendor)
{
  VendorWindow *vw;
  GladeXML *xml;
  GnomeDialog *vwd;
  GtkWidget *edit, *hbox;
  gnc_commodity *currency;

  /*
   * Find an existing window for this vendor.  If found, bring it to
   * the front.
   */
  if (vendor) {
    GUID vendor_guid;
    
    vendor_guid = *gncVendorGetGUID (vendor);
    vw = gnc_find_first_gui_component (DIALOG_EDIT_VENDOR_CM_CLASS,
				       find_handler, &vendor_guid);
    if (vw) {
      gtk_window_present (GTK_WINDOW(vw->dialog));
      return(vw);
    }
  }
  
  /* Find the default currency */
  if (vendor)
    currency = gncVendorGetCurrency (vendor);
  else
    currency = gnc_default_currency ();

  /*
   * No existing employee window found.  Build a new one.
   */
  vw = g_new0 (VendorWindow, 1);

  vw->book = bookp;

  /* Find the dialog */
  xml = gnc_glade_xml_new ("vendor.glade", "Vendor Dialog");
  vw->dialog = glade_xml_get_widget (xml, "Vendor Dialog");
  vwd = GNOME_DIALOG (vw->dialog);

  gtk_object_set_data (GTK_OBJECT (vw->dialog), "dialog_info", vw);

  /* default to ok */
  gnome_dialog_set_default (vwd, 0);

  /* Get entry points */
  vw->id_entry = glade_xml_get_widget (xml, "id_entry");
  vw->company_entry = glade_xml_get_widget (xml, "company_entry");

  vw->name_entry = glade_xml_get_widget (xml, "name_entry");
  vw->addr1_entry = glade_xml_get_widget (xml, "addr1_entry");
  vw->addr2_entry = glade_xml_get_widget (xml, "addr2_entry");
  vw->addr3_entry = glade_xml_get_widget (xml, "addr3_entry");
  vw->addr4_entry = glade_xml_get_widget (xml, "addr4_entry");
  vw->phone_entry = glade_xml_get_widget (xml, "phone_entry");
  vw->fax_entry = glade_xml_get_widget (xml, "fax_entry");
  vw->email_entry = glade_xml_get_widget (xml, "email_entry");

  vw->active_check = glade_xml_get_widget (xml, "active_check");
  vw->taxincluded_menu = glade_xml_get_widget (xml, "tax_included_menu");
  vw->notes_text = glade_xml_get_widget (xml, "notes_text");
  vw->terms_menu = glade_xml_get_widget (xml, "terms_menu");

  vw->taxtable_check = glade_xml_get_widget (xml, "taxtable_button");
  vw->taxtable_menu = glade_xml_get_widget (xml, "taxtable_menu");

  /* Currency */
  edit = gnc_currency_edit_new();
  gnc_currency_edit_set_currency (GNC_CURRENCY_EDIT(edit), currency);
  vw->currency_edit = edit;

  hbox = glade_xml_get_widget (xml, "currency_box");
  gtk_box_pack_start (GTK_BOX (hbox), edit, TRUE, TRUE, 0);

  /* Setup Dialog for Editing */
  gnome_dialog_set_default (vwd, 0);

  /* Attach <Enter> to default button */
  gnome_dialog_editable_enters (vwd, GTK_EDITABLE (vw->id_entry));
  gnome_dialog_editable_enters (vwd, GTK_EDITABLE (vw->company_entry));

  gnome_dialog_editable_enters (vwd, GTK_EDITABLE (vw->name_entry));
  gnome_dialog_editable_enters (vwd, GTK_EDITABLE (vw->addr1_entry));
  gnome_dialog_editable_enters (vwd, GTK_EDITABLE (vw->addr2_entry));
  gnome_dialog_editable_enters (vwd, GTK_EDITABLE (vw->addr3_entry));
  gnome_dialog_editable_enters (vwd, GTK_EDITABLE (vw->addr4_entry));
  gnome_dialog_editable_enters (vwd, GTK_EDITABLE (vw->phone_entry));
  gnome_dialog_editable_enters (vwd, GTK_EDITABLE (vw->fax_entry));
  gnome_dialog_editable_enters (vwd, GTK_EDITABLE (vw->email_entry));

  /* Set focus to company name */
  gtk_widget_grab_focus (vw->company_entry);

  /* Setup signals */
  gnome_dialog_button_connect
    (vwd, 0, GTK_SIGNAL_FUNC(gnc_vendor_window_ok_cb), vw);
  gnome_dialog_button_connect
    (vwd, 1, GTK_SIGNAL_FUNC(gnc_vendor_window_cancel_cb), vw);
  gnome_dialog_button_connect
    (vwd, 2, GTK_SIGNAL_FUNC(gnc_vendor_window_help_cb), vw);

  gtk_signal_connect (GTK_OBJECT (vw->dialog), "destroy",
		      GTK_SIGNAL_FUNC(gnc_vendor_window_destroy_cb), vw);

  gtk_signal_connect(GTK_OBJECT (vw->id_entry), "changed",
		     GTK_SIGNAL_FUNC(gnc_vendor_name_changed_cb), vw);

  gtk_signal_connect(GTK_OBJECT (vw->company_entry), "changed",
		     GTK_SIGNAL_FUNC(gnc_vendor_name_changed_cb), vw);

  gtk_signal_connect(GTK_OBJECT (vw->taxtable_check), "toggled",
		     GTK_SIGNAL_FUNC(gnc_vendor_taxtable_check_cb), vw);

  /* Setup initial values */
  if (vendor != NULL) {
    GncAddress *addr;
    const char *string;
    gint pos = 0;

    vw->dialog_type = EDIT_VENDOR;
    vw->vendor_guid = *gncVendorGetGUID (vendor);

    addr = gncVendorGetAddr (vendor);

    gtk_entry_set_text (GTK_ENTRY (vw->id_entry), gncVendorGetID (vendor));
    gtk_entry_set_text (GTK_ENTRY (vw->company_entry), gncVendorGetName (vendor));

    /* Setup Address */
    gtk_entry_set_text (GTK_ENTRY (vw->name_entry), gncAddressGetName (addr));
    gtk_entry_set_text (GTK_ENTRY (vw->addr1_entry), gncAddressGetAddr1 (addr));
    gtk_entry_set_text (GTK_ENTRY (vw->addr2_entry), gncAddressGetAddr2 (addr));
    gtk_entry_set_text (GTK_ENTRY (vw->addr3_entry), gncAddressGetAddr3 (addr));
    gtk_entry_set_text (GTK_ENTRY (vw->addr4_entry), gncAddressGetAddr4 (addr));
    gtk_entry_set_text (GTK_ENTRY (vw->phone_entry), gncAddressGetPhone (addr));
    gtk_entry_set_text (GTK_ENTRY (vw->fax_entry), gncAddressGetFax (addr));
    gtk_entry_set_text (GTK_ENTRY (vw->email_entry), gncAddressGetEmail (addr));

    /* Set toggle buttons */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (vw->active_check),
                                gncVendorGetActive (vendor));

    string = gncVendorGetNotes (vendor);
    gtk_editable_delete_text (GTK_EDITABLE (vw->notes_text), 0, -1);
    gtk_editable_insert_text (GTK_EDITABLE (vw->notes_text), string,
			      strlen(string), &pos);

    vw->component_id =
      gnc_register_gui_component (DIALOG_EDIT_VENDOR_CM_CLASS,
				  gnc_vendor_window_refresh_handler,
				  gnc_vendor_window_close_handler,
				  vw);

    vw->terms = gncVendorGetTerms (vendor);

  } else {
    vendor = gncVendorCreate (bookp);
    vw->vendor_guid = *gncVendorGetGUID (vendor);

    vw->dialog_type = NEW_VENDOR;
    vw->component_id =
      gnc_register_gui_component (DIALOG_NEW_VENDOR_CM_CLASS,
				  gnc_vendor_window_refresh_handler,
				  gnc_vendor_window_close_handler,
				  vw);

    /* XXX: Get the default Billing Terms */
    vw->terms = NULL;
  }

  /* I know that vendor exists here -- either passed in or just created */

  vw->taxincluded = gncVendorGetTaxIncluded (vendor);
  gnc_ui_taxincluded_optionmenu (vw->taxincluded_menu, &vw->taxincluded);
  gnc_ui_billterms_optionmenu (vw->terms_menu, bookp, TRUE, &vw->terms);

  vw->taxtable = gncVendorGetTaxTable (vendor);
  gnc_ui_taxtables_optionmenu (vw->taxtable_menu, bookp, TRUE, &vw->taxtable);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (vw->taxtable_check),
                                gncVendorGetTaxTableOverride (vendor));
  gnc_vendor_taxtable_check_cb (GTK_TOGGLE_BUTTON (vw->taxtable_check), vw);

  gnc_gui_component_watch_entity_type (vw->component_id,
				       GNC_VENDOR_MODULE_NAME,
				       GNC_EVENT_MODIFY | GNC_EVENT_DESTROY);

  gtk_widget_show_all (vw->dialog);

  return vw;
}

VendorWindow *
gnc_ui_vendor_new (GNCBook *bookp)
{
  VendorWindow *vw;

  /* Make sure required options exist */
  if (!bookp) return NULL;

  vw = gnc_vendor_new_window (bookp, NULL);
  return vw;
}

VendorWindow *
gnc_ui_vendor_edit (GncVendor *vendor)
{
  VendorWindow *vw;

  if (!vendor) return NULL;

  vw = gnc_vendor_new_window (gncVendorGetBook(vendor), vendor);

  return vw;
}

/* Functions for vendor selection widgets */

static void
invoice_vendor_cb (gpointer *vendor_p, gpointer user_data)
{
  struct _vendor_select_window *sw = user_data;
  GncOwner owner;
  GncVendor *vendor;

  g_return_if_fail (vendor_p && user_data);

  vendor = *vendor_p;

  if (!vendor)
    return;

  gncOwnerInitVendor (&owner, vendor);
  gnc_invoice_search (NULL, &owner, sw->book);
  return;
}

static void
order_vendor_cb (gpointer *vendor_p, gpointer user_data)
{
  struct _vendor_select_window *sw = user_data;
  GncOwner owner;
  GncVendor *vendor;

  g_return_if_fail (vendor_p && user_data);

  vendor = *vendor_p;

  if (!vendor)
    return;

  gncOwnerInitVendor (&owner, vendor);
  gnc_order_search (NULL, &owner, sw->book);
  return;
}

static void
jobs_vendor_cb (gpointer *vendor_p, gpointer user_data)
{
  struct _vendor_select_window *sw = user_data;
  GncOwner owner;
  GncVendor *vendor;

  g_return_if_fail (vendor_p && user_data);

  vendor = *vendor_p;

  if (!vendor)
    return;

  gncOwnerInitVendor (&owner, vendor);
  gnc_job_search (NULL, &owner, sw->book);
  return;
}

static void
payment_vendor_cb (gpointer *vendor_p, gpointer user_data)
{
  struct _vendor_select_window *sw = user_data;
  GncOwner owner;
  GncVendor *vendor;

  g_return_if_fail (vendor_p && user_data);

  vendor = *vendor_p;

  if (!vendor)
    return;

  gncOwnerInitVendor (&owner, vendor);
  gnc_ui_payment_new (&owner, sw->book);
  return;
}

static void
edit_vendor_cb (gpointer *vendor_p, gpointer user_data)
{
  GncVendor *vendor;

  g_return_if_fail (vendor_p && user_data);

  vendor = *vendor_p;

  if (!vendor)
    return;

  gnc_ui_vendor_edit (vendor);
  return;
}

static gpointer
new_vendor_cb (gpointer user_data)
{
  struct _vendor_select_window *sw = user_data;
  VendorWindow *vw;
  
  g_return_val_if_fail (user_data, NULL);

  vw = gnc_ui_vendor_new (sw->book);
  return vw_get_vendor (vw);
}

static void
free_vendor_cb (gpointer user_data)
{
  struct _vendor_select_window *sw = user_data;
  g_return_if_fail (sw);

  gncQueryDestroy (sw->q);
  g_free (sw);
}

GNCSearchWindow *
gnc_vendor_search (GncVendor *start, GNCBook *book)
{
  GNCIdType type = GNC_VENDOR_MODULE_NAME;
  struct _vendor_select_window *sw;
  QueryNew *q, *q2 = NULL;
  static GList *params = NULL;
  static GList *columns = NULL;
  static GNCSearchCallbackButton buttons[] = { 
    { N_("View/Edit Vendor"), edit_vendor_cb},
    { N_("Vendor's Jobs"), jobs_vendor_cb},
    //    { N_("Vendor Orders"), order_vendor_cb},
    { N_("Vendor's Bills"), invoice_vendor_cb},
    { N_("Pay Bill"), payment_vendor_cb},
    { NULL },
  };
  (void)order_vendor_cb;

  g_return_val_if_fail (book, NULL);

  /* Build parameter list in reverse order*/
  if (params == NULL) {
    params = gnc_search_param_prepend (params, _("Billing Contact"), NULL, type,
				       VENDOR_ADDR, ADDRESS_NAME, NULL);
    params = gnc_search_param_prepend (params, _("Vendor ID"), NULL, type,
				       VENDOR_ID, NULL);
    params = gnc_search_param_prepend (params, _("Company Name"), NULL, type,
				       VENDOR_NAME, NULL);
  }

  /* Build the column list in reverse order */
  if (columns == NULL) {
    columns = gnc_search_param_prepend (columns, _("Contact"), NULL, type,
					VENDOR_ADDR, ADDRESS_NAME, NULL);
    columns = gnc_search_param_prepend (columns, _("Company"), NULL, type,
					VENDOR_NAME, NULL);
    columns = gnc_search_param_prepend (columns, _("ID #"), NULL, type,
					VENDOR_ID, NULL);
  }

  /* Build the queries */
  q = gncQueryCreate ();
  gncQuerySetBook (q, book);

#if 0
  if (start) {
    q2 = gncQueryCopy (q);
    gncQueryAddGUIDMatch (q2, g_slist_prepend (NULL, QUERY_PARAM_GUID),
			  gncVendorGetGUID (start), QUERY_AND);
  }
#endif

  /* launch select dialog and return the result */
  sw = g_new0 (struct _vendor_select_window, 1);
  sw->book = book;
  sw->q = q;

  return gnc_search_dialog_create (type, params, columns, q, q2,
				   buttons, NULL,
				   new_vendor_cb, sw, free_vendor_cb);
}

GNCSearchWindow *
gnc_vendor_search_select (gpointer start, gpointer book)
{
  if (!book) return NULL;

  return gnc_vendor_search (start, book);
}

GNCSearchWindow *
gnc_vendor_search_edit (gpointer start, gpointer book)
{
  if (start)
    gnc_ui_vendor_edit (start);

  return NULL;
}
