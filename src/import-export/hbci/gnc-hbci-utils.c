/********************************************************************\
 * gnc-hbci-utils.c -- hbci utility functions                       *
 * Copyright (C) 2002 Christian Stimming                            *
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
#include "gnc-hbci-utils.h"

#include <gnome.h>
#include <errno.h>
#include <gwenhywfar/directory.h>

#include "gnc-ui.h"
#include "gnc-hbci-kvp.h"
#include "gnc-ui-util.h"
#include "gnc-engine-util.h" 
#include "global-options.h"

#include "hbci-interaction.h"
#include <aqbanking/version.h>

/* static short module = MOD_IMPORT; */

/* Globale variables for AB_BANKING caching. */
static AB_BANKING *gnc_AB_BANKING = NULL;
static int gnc_AB_BANKING_refcnt = 0;
static GNCInteractor *gnc_hbci_inter = NULL;


AB_BANKING * gnc_AB_BANKING_new_currentbook (GtkWidget *parent, 
					     GNCInteractor **inter)
{
  if (gnc_AB_BANKING == NULL) {
    /* No API cached -- create new one. */
    AB_BANKING *api = NULL;
  
    api = AB_Banking_new ("gnucash", 0);
    g_assert(api);
    {
      int r = AB_Banking_Init(api);
      if (r != 0)
	printf("gnc_AB_BANKING_new: Warning: Error %d on AB_Banking_init\n", r);
    }
    
    gnc_hbci_inter = gnc_AB_BANKING_interactors (api, parent);
    gnc_AB_BANKING = api;
    
    if (inter)
      *inter = gnc_hbci_inter;

    gnc_AB_BANKING_refcnt = 1;
    return gnc_AB_BANKING;
  } else {
    /* API cached. */

    /* Init the API again. */
    if (gnc_AB_BANKING_refcnt == 0)
      AB_Banking_Init(gnc_AB_BANKING);

    if (inter) {
      *inter = gnc_hbci_inter;
      GNCInteractor_reparent (*inter, parent);
    }
    
    gnc_AB_BANKING_refcnt++;
    return gnc_AB_BANKING;
  }
}

void gnc_AB_BANKING_delete (AB_BANKING *api)
{
  if (api == 0)
    api = gnc_AB_BANKING;

  if (api) {
    if (api == gnc_AB_BANKING) {
      gnc_AB_BANKING = NULL;
      gnc_hbci_inter = NULL;
      if (gnc_AB_BANKING_refcnt > 0)
	AB_Banking_Fini(api);
    }

    AB_Banking_free(api);
  }
}


int gnc_AB_BANKING_fini (AB_BANKING *api) 
{
  if (api == gnc_AB_BANKING) {
    gnc_AB_BANKING_refcnt--;
    if (gnc_AB_BANKING_refcnt == 0)
      return AB_Banking_Fini(api);
  }
  else
    return AB_Banking_Fini(api);
  return 0;
}




AB_ACCOUNT *
gnc_hbci_get_hbci_acc (const AB_BANKING *api, Account *gnc_acc) 
{
  int account_uid = 0;
  AB_ACCOUNT *hbci_acc = NULL;
  const char *bankcode = NULL, *accountid = NULL;

  bankcode = gnc_hbci_get_account_bankcode (gnc_acc);
  accountid = gnc_hbci_get_account_accountid (gnc_acc);
  account_uid = gnc_hbci_get_account_uid (gnc_acc);
  if (account_uid > 0) {
    /*printf("gnc_hbci_get_hbci_acc: gnc_acc %s has blz %s and ccode %d\n",
      xaccAccountGetName (gnc_acc), bankcode, countrycode);*/
    hbci_acc = AB_Banking_GetAccount(api, account_uid);

    if (!hbci_acc && bankcode && (strlen(bankcode)>0) &&
	accountid && (strlen(accountid) > 0)) {
      printf("gnc_hbci_get_hbci_acc: No AB_ACCOUNT found for UID %d, trying bank code\n", account_uid);
      hbci_acc = AB_Banking_GetAccountByCodeAndNumber(api, bankcode, accountid);
    }
    /*printf("gnc_hbci_get_hbci_acc: return HBCI_Account %p\n", hbci_acc);*/
    return hbci_acc;
  } else if (bankcode && (strlen(bankcode)>0) && accountid && (strlen(accountid) > 0)) {
    hbci_acc = AB_Banking_GetAccountByCodeAndNumber(api, bankcode, accountid);
    return hbci_acc;
  }

  return NULL;
}

#if 0
static void *
print_list_int_cb (int value, void *user_data)
{
  printf("%d, ", value);
  return NULL;
}
static void 
print_list_int (const list_int *list)
{
  g_assert(list);
  list_int_foreach (list, &print_list_int_cb, NULL);
  printf ("\n");
}
static void *
get_resultcode_error_cb (int value, void *user_data)
{
  int *tmp_result = user_data;
  if (value > *tmp_result)
    *tmp_result = value;
  if (value >= 9000)
    return (void*) value;
  else
    return NULL;
}
static int
get_resultcode_error (const list_int *list)
{
  int tmp_result = 0, cause = 0;
  g_assert (list);
  cause = (int) list_int_foreach (list, &get_resultcode_error_cb, &tmp_result);
  return MAX(tmp_result, cause);
}
#endif
int
gnc_hbci_debug_outboxjob (AB_JOB *job, gboolean verbose)
{
/*   list_int *list; */
/*   const char *msg; */
  int cause = 0;
  
  g_assert (job);
/*   if (AB_JOB_status (job) != HBCI_JOB_STATUS_DONE) */
/*     return; */
/*   if (AB_JOB_result (job) != HBCI_JOB_RESULT_FAILED) */
/*     return; */

  if (verbose) {
    printf("OutboxJob status: %s", AB_Job_Status2Char(AB_Job_GetStatus(job)));

    printf(", result: %s", AB_Job_GetResultText(job));
    printf("\n");
  }

#if 0  
  list = AB_JOB_resultCodes (job);
  if (list_int_size (list) > 0) {

    cause = get_resultcode_error (list);

    if (verbose) {
      printf("OutboxJob resultcodes: ");
      print_list_int (list);

      switch (cause) {
      case 9310:
	msg = "Schluessel noch nicht hinterlegt";
	break;
      case 9320:
	msg = "Schluessel noch nicht freigeschaltet";
	break;
      case 9330:
	msg = "Schluessel gesperrt";
	break;
      case 9340:
	msg = "Schluessel falsch";
	break;
      default:
	msg = "Unknown";
      }
      printf("Probable cause of error was: code %d, msg: %s\n", cause, msg);
    }
  } else {
    if (verbose)
      printf("OutboxJob's resultCodes list has zero length.\n");
  }
  list_int_delete (list);
#endif

  return cause;
}


gboolean
gnc_hbci_Error_retry (GtkWidget *parent, int error, 
		      GNCInteractor *inter)
{
  int code = error;

  switch (code) {
#if 0
  case AB_ERROR_PIN_WRONG:
    GNCInteractor_erasePIN (inter);
    return gnc_verify_dialog_parented (parent,
				       TRUE,
				       _("The PIN you entered was wrong.\n"
					 "Do you want to try again?"));
  case AB_ERROR_PIN_WRONG_0:
    GNCInteractor_erasePIN (inter);
    return gnc_verify_dialog_parented (parent,
				       TRUE,
				       _("The PIN you entered was wrong.\n"
					 "ATTENTION: You have zero further wrong retries left!\n"
					 "Do you want to try again?"));
  case AB_ERROR_PIN_WRONG_1:
    GNCInteractor_erasePIN (inter);
    return gnc_verify_dialog_parented (parent,
				       TRUE,
				       _("The PIN you entered was wrong.\n"
					 "You have one further wrong retry left.\n"
					 "Do you want to try again?"));
  case AB_ERROR_PIN_WRONG_2:
    GNCInteractor_erasePIN (inter);
    return gnc_verify_dialog_parented (parent,
				       TRUE,
				       _("The PIN you entered was wrong.\n"
					 "You have two further wrong retries left.\n"
					 "Do you want to try again?"));
  case AB_ERROR_PIN_ABORTED:
    /*     printf("gnc_hbci_Error_feedback: PIN dialog was aborted.\n"); */
    return FALSE;
  case AB_ERROR_PIN_TOO_SHORT:
    GNCInteractor_erasePIN (inter);
    return gnc_verify_dialog_parented (parent,
				       TRUE,
				       _("The PIN you entered was too short.\n"
					 "Do you want to try again?"));
  case AB_ERROR_CARD_DESTROYED:
    GNCInteractor_hide (inter);
    gnc_error_dialog_parented
      (parent,
       _("Unfortunately you entered a wrong PIN for too many times.\n"
	 "Your chip card is therefore destroyed. Aborting."));
    return FALSE;
  case AB_ERROR_FILE_NOT_FOUND:
    /*     printf("gnc_hbci_Error_feedback: File not found error.\n"); */
    return FALSE;
  case AB_ERROR_NO_CARD:
    return gnc_verify_dialog_parented (parent,
				       TRUE,
				       _("No chip card has been found in the chip card reader.\n"
					 "Do you want to try again?"));
  case AB_ERROR_JOB_NOT_SUPPORTED:
    GNCInteractor_hide (inter);
    gnc_error_dialog_parented 
      (parent,
       _("Unfortunately this HBCI job is not supported \n"
	 "by your bank or for your account. Aborting."));
    return FALSE;
#endif
  case AB_ERROR_NETWORK:
    GNCInteractor_hide (inter);
    gnc_error_dialog_parented
      (GTK_WINDOW (parent),
       _("The server of your bank refused the HBCI connection.\n"
	 "Please try again later. Aborting."));
    return FALSE;
#if 0
  case AB_ERROR_MEDIUM:
    gnc_error_dialog_parented 
      (parent,
       _("There was an error when loading the plugin for your security medium \n"
	 "(see log window). Probably the versions of your currently installed \n"
	 "OpenHBCI library and of the plugin do not match. In that case you need \n"
	 "to recompile and reinstall the plugin again. Aborting now."));
    GNCInteractor_hide (inter);
    return FALSE;
  case AB_ERROR_BAD_MEDIUM:
    gnc_error_dialog_parented
      (parent,
       _("Your security medium is not supported. No appropriate plugin \n"
	 "has been found for that medium. Aborting."));
    GNCInteractor_hide (inter);
    return FALSE;
#endif
      
  default:
    return FALSE;
  }
  
}

#if 0
/* Prints all results that can be found in the outbox into the interactor */
static void gnc_hbci_printresult(HBCI_Outbox *outbox, GNCInteractor *inter)
{
  /* Got no sysid. */
  GWEN_DB_NODE *rsp, *n;
  g_assert(outbox);
  if (!inter) 
    return;
  
  rsp = HBCI_Outbox_response(outbox);
  n = GWEN_DB_GetFirstGroup(rsp);
  while(n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "msgresult")==0) {
      GWEN_DB_NODE *r = GWEN_DB_GetFirstGroup(n);
      while (r) {
	if (strcasecmp(GWEN_DB_GroupName(r), "result") == 0) {
	  gchar *logtext;
	  int resultcode;
	  const char *text, *elementref, *param;
	  
	  resultcode = GWEN_DB_GetIntValue(r, "resultcode", 0, 0);
	  text = GWEN_DB_GetCharValue(r, "text", 0, "Response without text");
	  elementref = GWEN_DB_GetCharValue(r, "elementref", 0, "");
	  param = GWEN_DB_GetCharValue(r, "param", 0, "");

	  if (strlen(elementref)>0 || strlen(param) > 0)
	    logtext = g_strdup_printf("%s (%d; Elementref %s; Param %s)", text, 
				      resultcode, elementref, param);
	  else
	    logtext = g_strdup_printf("%s (%d)", text, resultcode);
	  GNCInteractor_add_log_text(inter, logtext);
	  g_free(logtext);
	}
	r = GWEN_DB_GetNextGroup(r);
      }
    } 
    else if (strcasecmp(GWEN_DB_GroupName(n), "segresult")==0) {
      GWEN_DB_NODE *r = GWEN_DB_GetFirstGroup(n);
      while (r) {
	if (strcasecmp(GWEN_DB_GroupName(r), "result") == 0) {
	}
	r = GWEN_DB_GetNextGroup(r);
      }
    } 
    n=GWEN_DB_GetNextGroup(n);
  } // while

  GWEN_DB_Group_free(rsp);
}
#endif

static gboolean hbci_Error_isOk(int err) {
  switch (err) {
  case 0:
    return TRUE;
  default:
    return FALSE;
  };
}

gboolean
gnc_AB_BANKING_execute (GtkWidget *parent, AB_BANKING *api,
			AB_JOB *job, GNCInteractor *inter)
{
  int err;
  int resultcode;
	  
  if (inter)
    GNCInteractor_show (inter);

  if (gnc_lookup_boolean_option("_+Advanced", 
				"HBCI Verbose Debug Messages", FALSE)) {
/*     GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug); */
/*     HBCI_Hbci_setDebugLevel (4); */
  }
/*   else */
/*     HBCI_Hbci_setDebugLevel (0); */

  do {
    if (inter) {
      GNCInteractor_show_nodelete (inter);
#if (AQBANKING_VERSION_MAJOR > 0) || (AQBANKING_VERSION_MINOR > 9) || \
  ((AQBANKING_VERSION_MINOR == 9) && \
   ((AQBANKING_VERSION_PATCHLEVEL > 6) || \
    ((AQBANKING_VERSION_PATCHLEVEL == 6) && (AQBANKING_VERSION_BUILD > 1))))
      AB_Banking_SetPinCacheEnabled (api, GNCInteractor_get_cache_valid(inter));
#endif
    }

    err = AB_Banking_ExecuteQueue (api);

    /* Print result codes to interactor */
/*     gnc_hbci_printresult(queue, inter); */
    
  } while (gnc_hbci_Error_retry (parent, err, inter));
  
  resultcode = gnc_hbci_debug_outboxjob (job, FALSE);
  if (!hbci_Error_isOk(err)) {
/*     char *errstr =  */
/*       g_strdup_printf("gnc_AB_BANKING_execute: Error at executeQueue: %s", */
/* 		      hbci_Error_message (err)); */
/*     printf("%s\n", errstr); */
/*     HBCI_Interactor_msgStateResponse (HBCI_Hbci_interactor  */
/* 				      (AB_BANKING_Hbci (api)), errstr); */
/*     g_free (errstr); */
    gnc_hbci_debug_outboxjob (job, TRUE);
    GNCInteractor_show_nodelete (inter);
    return FALSE;
  }

  GNCInteractor_set_cache_valid (inter, TRUE);
  if (resultcode <= 20) {
    return TRUE;
  }
  else {
/*     printf("gnc_AB_BANKING_execute: Some message at executeQueue: %s", */
/* 	   hbci_Error_message (err)); */
    GNCInteractor_show_nodelete (inter);
    return TRUE; /* <- This used to be a FALSE but this was probably
		  * as wrong as it could get. @�%$! */
  }
}

/* Needed for the gnc_hbci_descr_tognc and gnc_hbci_memo_tognc. */
static void *gnc_list_string_cb (const char *string, void *user_data)
{
  gchar **res = user_data;
  gchar *tmp1, *tmp2;

  if (!string) return NULL;
  tmp1 = g_strdup (string);
  g_strstrip (tmp1);

  if (strlen (tmp1) > 0) {
    if (*res != NULL) {
      /* The " " is the separating string in between each two strings. */
      tmp2 = g_strjoin (" ", *res, tmp1, NULL);
      g_free (tmp1);
      
      g_free (*res);
      *res = tmp2;
    }
    else {
      *res = tmp1;
    }
  }
  
  return NULL;
}


char *gnc_hbci_descr_tognc (const AB_TRANSACTION *h_trans)
{
  /* Description */
  char *h_descr = NULL;
  char *othername = NULL;
  char *g_descr;
  const GWEN_STRINGLIST *h_purpose = AB_Transaction_GetPurpose (h_trans);
  const GWEN_STRINGLIST *h_remotename = AB_Transaction_GetRemoteName (h_trans);

  /* Don't use list_string_concat_delim here since we need to
     g_strstrip every single element of the string list, which is
     only done in our callback gnc_list_string_cb. The separator is
     also set there. */
  if (h_purpose)
    GWEN_StringList_ForEach (h_purpose,
			     &gnc_list_string_cb,
			     &h_descr);
  if (h_remotename)
    GWEN_StringList_ForEach (h_remotename,
			     &gnc_list_string_cb,
			     &othername);
  /*DEBUG("HBCI Description '%s'", h_descr);*/

  if (othername && (strlen (othername) > 0))
    g_descr = 
      ((h_descr && (strlen (h_descr) > 0)) ?
       g_strdup_printf ("%s; %s", 
			h_descr,
			othername) :
       g_strdup (othername));
  else
    g_descr = 
      ((h_descr && (strlen (h_descr) > 0)) ?
       g_strdup (h_descr) : 
       g_strdup (_("Unspecified")));

  free (h_descr);
  free (othername);
  return g_descr;
}

char *gnc_hbci_memo_tognc (const AB_TRANSACTION *h_trans)
{
  /* Memo in the Split. HBCI's transactionText contains strings like
   * "STANDING ORDER", "UEBERWEISUNGSGUTSCHRIFT", etc.  */
  /*   char *h_transactionText =  */
  /*     g_strdup (AB_TRANSACTION_transactionText (h_trans)); */
  const char *h_remoteAccountNumber = 
    AB_Transaction_GetRemoteAccountNumber (h_trans);
  const char *h_remoteBankCode = 
    AB_Transaction_GetRemoteBankCode (h_trans);
  char *h_otherAccountId =
    g_strdup (h_remoteAccountNumber ? h_remoteAccountNumber : _("unknown"));
  char *h_otherBankCode =
    g_strdup (h_remoteBankCode ? h_remoteBankCode : _("unknown"));
  char *g_memo;

  /*   g_strstrip (h_transactionText); */
  g_strstrip (h_otherAccountId);
  g_strstrip (h_otherBankCode);

  g_memo = 
    (h_otherAccountId && (strlen (h_otherAccountId) > 0) ?
     g_strdup_printf ("%s %s %s %s",
		      _("Account"), h_otherAccountId,
		      _("Bank"), h_otherBankCode) :
     g_strdup (""));
    
  g_free (h_otherAccountId);
  g_free (h_otherBankCode);
  return g_memo;
}


#if 0
/** Return the only customer that can act on the specified account, or
    NULL if none was found. */
const HBCI_Customer *
gnc_hbci_get_first_customer(const AB_ACCOUNT *h_acc)
{
  /* Get one customer. */
  const list_HBCI_User *userlist;
  const HBCI_Bank *bank;
  const HBCI_User *user;
  g_assert(h_acc);
  
  bank = AB_ACCOUNT_bank (h_acc);
  userlist = HBCI_Bank_users (bank);
  g_assert (userlist);
  user = choose_one_user(gnc_ui_get_toplevel (), userlist);
  g_assert (user);
  return choose_one_customer(gnc_ui_get_toplevel (), HBCI_User_customers (user));
}

const char *bank_to_str (const HBCI_Bank *bank)
{
  g_assert (bank);
  return ((strlen(HBCI_Bank_name (bank)) > 0) ?
	  HBCI_Bank_name (bank) :
	  HBCI_Bank_bankCode(bank));
}


const HBCI_Bank *
choose_one_bank (gncUIWidget parent, const list_HBCI_Bank *banklist)
{
  const HBCI_Bank *bank;
  list_HBCI_Bank_iter *iter, *end;
  int list_size;
  g_assert (parent);
  g_assert (banklist);

  /*printf("%d banks found.\n", list_HBCI_Bank_size (banklist));*/
  list_size = list_HBCI_Bank_size (banklist);
  if (list_size == 0) 
    return NULL;

  if (list_size == 1) 
    {
      /* Get bank. */
      iter = list_HBCI_Bank_begin (banklist);
      bank = list_HBCI_Bank_iter_get (iter);
      list_HBCI_Bank_iter_delete (iter);
      return bank;
    }

  /* More than one bank available. */
  {
    int choice, i;
    GList *node;
    GList *radio_list = NULL;

    end = list_HBCI_Bank_end (banklist);
    for (iter = list_HBCI_Bank_begin (banklist);
	 !list_HBCI_Bank_iter_equal(iter, end); 
	 list_HBCI_Bank_iter_next(iter))
      {
	bank = list_HBCI_Bank_iter_get (iter);
	radio_list = g_list_append(radio_list, 
				   g_strdup_printf ("%s (%s)",
						    HBCI_Bank_name (bank),
						    HBCI_Bank_bankCode (bank)));
      }
    list_HBCI_Bank_iter_delete (iter);
      
    choice = gnc_choose_radio_option_dialog_parented
      (parent,
       _("Choose HBCI bank"), 
       _("More than one HBCI bank is available for \n"
	 "the requested operation. Please choose \n"
	 "the one that should be used."), 
       0, 
       radio_list);
      
    for (node = radio_list; node; node = node->next)
      g_free (node->data);
    g_list_free (radio_list);

    i = 0;
    for (iter = list_HBCI_Bank_begin (banklist);
	 !list_HBCI_Bank_iter_equal(iter, end); 
	 list_HBCI_Bank_iter_next(iter))
      if (i == choice)
	{
	  bank = list_HBCI_Bank_iter_get (iter);
	  list_HBCI_Bank_iter_delete (iter);
	  list_HBCI_Bank_iter_delete (end);
	  return bank;
	}
      else
	++i;
  }
  
  g_assert_not_reached();
  return NULL;
}

const HBCI_Customer *
choose_one_customer (gncUIWidget parent, const list_HBCI_Customer *custlist)
{
  const HBCI_Customer *customer;
  list_HBCI_Customer_iter *iter, *end;
  g_assert(parent);
  g_assert(custlist);
  
  if (list_HBCI_Customer_size (custlist) == 0) {
    printf ("choose_one_customer: oops, no customer found.\n");
    return NULL;
  }
  if (list_HBCI_Customer_size (custlist) == 1) 
    {
      /* Get one customer */
      iter = list_HBCI_Customer_begin (custlist);
      customer = list_HBCI_Customer_iter_get (iter);
      list_HBCI_Customer_iter_delete (iter);
      
      return customer;
    }

  /* More than one customer available. */
  {
    int choice, i;
    GList *node;
    GList *radio_list = NULL;

    end = list_HBCI_Customer_end (custlist);
    for (iter = list_HBCI_Customer_begin (custlist);
	 !list_HBCI_Customer_iter_equal(iter, end); 
	 list_HBCI_Customer_iter_next(iter))
      {
	customer = list_HBCI_Customer_iter_get (iter);
	radio_list = 
	  g_list_append(radio_list, 
			/* Translators: %s is the name of the
			 * customer. %s is the id of the customer. %s
			 * is the name of the bank. %s is the bank
			 * code. */
			g_strdup_printf (_("%s (%s) at bank %s (%s)"),
					 HBCI_Customer_name (customer),
					 HBCI_Customer_custId (customer),
					 bank_to_str (HBCI_User_bank(HBCI_Customer_user(customer))),
					 HBCI_Bank_bankCode (HBCI_User_bank(HBCI_Customer_user(customer)))));
      }
    list_HBCI_Customer_iter_delete (iter);
      
    choice = gnc_choose_radio_option_dialog_parented
      (parent,
       _("Choose HBCI customer"), 
       _("More than one HBCI customer is available for \n"
	 "the requested operation. Please choose \n"
	 "the one that should be used."), 
       0, 
       radio_list);
      
    for (node = radio_list; node; node = node->next)
      g_free (node->data);
    g_list_free (radio_list);

    i = 0;
    for (iter = list_HBCI_Customer_begin (custlist);
	 !list_HBCI_Customer_iter_equal(iter, end); 
	 list_HBCI_Customer_iter_next(iter))
      if (i == choice)
	{
	  customer = list_HBCI_Customer_iter_get (iter);
	  list_HBCI_Customer_iter_delete (iter);
	  list_HBCI_Customer_iter_delete (end);
	  return customer;
	}
      else
	++i;
  }
  
  g_assert_not_reached();
  return NULL;
}

const HBCI_User *
choose_one_user (gncUIWidget parent, const list_HBCI_User *userlist)
{
  const HBCI_User *user;
  list_HBCI_User_iter *iter, *end;
  g_assert(parent);
  g_assert(userlist);

  if (list_HBCI_User_size (userlist) == 0) {
    printf("choose_one_user: oops, no user found.\n");
    return NULL;
  }
  if (list_HBCI_User_size (userlist) == 1)
    {
      /* Get one User */
      iter = list_HBCI_User_begin (userlist);
      user = list_HBCI_User_iter_get (iter);
      list_HBCI_User_iter_delete (iter);

      return user;
    }

  /* More than one user available. */
  {
    int choice, i;
    GList *node;
    GList *radio_list = NULL;

    end = list_HBCI_User_end (userlist);
    for (iter = list_HBCI_User_begin (userlist);
	 !list_HBCI_User_iter_equal(iter, end); 
	 list_HBCI_User_iter_next(iter))
      {
	user = list_HBCI_User_iter_get (iter);
	radio_list = g_list_append
	    (radio_list, 
	     g_strdup_printf (_("%s (%s) at bank %s (%s)"),
			      HBCI_User_name (user),
			      HBCI_User_userId (user),
			      bank_to_str (HBCI_User_bank(user)),
			      HBCI_Bank_bankCode (HBCI_User_bank(user))));
      }
    list_HBCI_User_iter_delete (iter);
      
    choice = gnc_choose_radio_option_dialog_parented
      (parent,
       _("Choose HBCI user"), 
       _("More than one HBCI user is available for \n"
	 "the requested operation. Please choose \n"
	 "the one that should be used."), 
       0, 
       radio_list);
      
    for (node = radio_list; node; node = node->next)
      g_free (node->data);
    g_list_free (radio_list);

    i = 0;
    for (iter = list_HBCI_User_begin (userlist);
	 !list_HBCI_User_iter_equal(iter, end); 
	 list_HBCI_User_iter_next(iter))
      if (i == choice)
	{
	  user = list_HBCI_User_iter_get (iter);
	  list_HBCI_User_iter_delete (iter);
	  list_HBCI_User_iter_delete (end);
	  return user;
	}
      else
	++i;
  }
  
  g_assert_not_reached();
  return NULL;
}
#endif

char *gnc_AB_VALUE_toReadableString(const AB_VALUE *v)
{
  char tmp[100];
  if (v)
    sprintf(tmp, "%.2f %s", AB_Value_GetValue(v), AB_Value_GetCurrency(v));
  else
    sprintf(tmp, "%.2f", 0.0);
  return g_strdup(tmp);
}
