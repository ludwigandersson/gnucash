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
/** @addtogroup Engine
    @{ */
/** @file gnc-book.h
 * @brief dataset access (set of accounting books)
 * Encapsulate all the information about a gnucash dataset.
 * See src/docs/books.txt for implementation overview.
 *
 * HISTORY:
 * Created by Linas Vepstas December 1998
 * @author Copyright (c) 1998, 1999, 2001 Linas Vepstas <linas@linas.org>
 * @author Copyright (c) 2000 Dave Peticolas
 */

#ifndef GNC_BOOK_H
#define GNC_BOOK_H

#include "GNCId.h"
#include "gnc-engine.h"
#include "gnc-pricedb.h"
#include "kvp_frame.h"

/** Allocate, initialise and return a new GNCBook.  The new book will
    contain a newly allocated AccountGroup */
GNCBook * gnc_book_new (void);

/** End any editing sessions associated with book, and free all memory 
    associated with it. */
void      gnc_book_destroy (GNCBook *book);

/** \return The Entity table for the book. */
GNCEntityTable      * gnc_book_get_entity_table (GNCBook *book);

/** \return The GUID for the book. */
const GUID          * gnc_book_get_guid (GNCBook *book);

/** \return The kvp data for the book */
kvp_frame           * gnc_book_get_slots (GNCBook *book);

/** \return The top-level group in the book.*/
AccountGroup        * gnc_book_get_group (GNCBook *book);

/** \return The pricedb of the book. */
GNCPriceDB          * gnc_book_get_pricedb (GNCBook *book);

/** \return The commodity table of the book.  */
gnc_commodity_table * gnc_book_get_commodity_table(GNCBook *book);

/** \return A GList of the scheduled transactions in the book.  */
GList               * gnc_book_get_schedxactions( GNCBook *book );

/** \return The template AccountGroup of the book.  */
AccountGroup        * gnc_book_get_template_group( GNCBook *book );

/** The gnc_book_set_data() allows
 *    arbitrary pointers to structs to be stored in GNCBook.
 *    This is the "prefered" method for extending GNCBook to hold
 *    new data types.
 */
void gnc_book_set_data (GNCBook *book, const char *key, gpointer data);

/** Retreives arbitrary pointers to structs stored by gnc_book_set_data. */
gpointer gnc_book_get_data (GNCBook *book, const char *key);

/** DOCUMENT ME! */
gpointer gnc_book_get_backend (GNCBook *book);

/** gnc_book_not_saved() will return TRUE if any 
 *    data in the book hasn't been saved to long-term storage.
 *    (Actually, that's not quite true.  The book doesn't know 
 *    anything about saving.  Its just that whenever data is modified,
 *    the 'dirty' flag is set.  This routine returns the value of the 
 *    'dirty' flag.  Its up to the backend to periodically reset this 
 *    flag, when it acutally does save the data.)
 */
gboolean gnc_book_not_saved (GNCBook *book);

/** Call this function when you change the book kvp, to make sure the book
 * is marked 'dirty'. */
void gnc_book_kvp_changed (GNCBook *book);

/** The gnc_book_equal() method returns TRUE if the engine data
 * in the two given books is equal. */
gboolean gnc_book_equal (GNCBook *book_1, GNCBook *book_2);

/** \warning XXX FIXME gnc_book_count_transactions is a utility function, needs 
 * to be moved to some utility/support file.  */
guint gnc_book_count_transactions(GNCBook *book);

/** This will 'get and increment' the named counter for this book.
 * The return value is -1 on error or the incremented counter.
 */
gint64 gnc_book_get_counter (GNCBook *book, const char *counter_name);

/** Book parameter names */
/**@{*/ 

#define BOOK_KVP		"kvp"

/**@}*/
 
#endif /* GNC_BOOK_H */
/** @} */
