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
/** @file kvp_frame.h
    @brief A key-value frame system for gnucash
    @author Copyright (C) 2000 Bill Gribble
*/

#ifndef KVP_FRAME_H
#define KVP_FRAME_H

#include <glib.h>

#include "gnc-date.h"
#include "gnc-numeric.h"
#include "guid.h"

/** a KvpFrame is a set of associations between character strings
 * (keys) and KvpValue structures.  A KvpValue is a union with
 * possible types enumerated in the KvpValueType enum, and includes, 
 * among other things, ints, doubles, strings, guid's, lists, time
 * and numeric values.  KvpValues may also be other frames, so
 * KVP is inherently heirarchical.
 * 
 * Values are stored in a 'slot' associated with a key.
 * Pointers passed as arguments into set_slot and get_slot are the
 * responsibility of the caller.  Pointers returned by get_slot are
 * owned by the kvp_frame.  Make copies as needed.
 * 
 * kvp_frame_delete and kvp_value_delete are deep (recursive) deletes.
 * kvp_frame_copy and kvp_value_copy are deep value copies. 
 */
typedef struct _KvpFrame KvpFrame;

/** A KvpValue is a union with possible types enumerated in the
 * KvpValueType enum. */
typedef struct _KvpValue KvpValue;
 
/** Enum to enumerate possible types in the union KvpValue */
typedef enum {
  KVP_TYPE_GINT64,
  KVP_TYPE_DOUBLE,
  KVP_TYPE_NUMERIC,
  KVP_TYPE_STRING,
  KVP_TYPE_GUID,
  KVP_TYPE_TIMESPEC,
  KVP_TYPE_BINARY, 
  KVP_TYPE_GLIST,
  KVP_TYPE_FRAME
} KvpValueType;


/** @name KvpFrame Constructors */
/*@{*/

/** Return a new empty instance of KvpFrame */
KvpFrame   * kvp_frame_new(void);

/** Perform a deep (recursive) delete of the frame and any subframes. */
void         kvp_frame_delete(KvpFrame * frame);

/** Perform a deep (recursive) value copy, copying the fraame, 
 *  subframes, and the values as well. */
KvpFrame   * kvp_frame_copy(const KvpFrame * frame);

/** Return TRUE if the KvpFrame is empty */
gboolean     kvp_frame_is_empty(KvpFrame * frame);

/*@}*/


/** Internal helper rouotines, you probably shouldn't be using these. */
gchar* kvp_frame_to_string(const KvpFrame *frame);
gchar* binary_to_string(const void *data, guint32 size);
gchar* kvp_value_glist_to_string(const GList *list);
GHashTable* kvp_frame_get_hash(const KvpFrame *frame);


/** @name KvpFrame Value Storing */
/*@{*/
/** The kvp_frame_set_slot() routine copies the value into the frame,
 *    associating it with 'key'.
 *
 * Pointers passed as arguments into set_slot are the responsibility
 * of the caller.
 */
void          kvp_frame_set_slot(KvpFrame * frame, 
                                 const char * key, const KvpValue * value);
/**
 * The kvp_frame_set_slot_nc() routine puts the value (without copying
 *    it) into the frame, associating it with 'key'.  This routine is
 *    handy for avoiding excess memory allocations & frees.
 */
void          kvp_frame_set_slot_nc(KvpFrame * frame, 
                                    const char * key, KvpValue * value);

/** The kvp_frame_set_slot_path() routines walk the hierarchy,
 * using the key values to pick each branch.  When the terminal node
 * is reached, the value is copied into it.
 */
void          kvp_frame_set_slot_path (KvpFrame *frame,
                                       const KvpValue *value,
                                       const char *first_key, ...);

/** The kvp_frame_set_slot_path() routines walk the hierarchy,
 * using the key values to pick each branch.  When the terminal node
 * is reached, the value is copied into it.
 */
void          kvp_frame_set_slot_path_gslist (KvpFrame *frame,
                                              const KvpValue *value,
                                              GSList *key_path);
/*@}*/


/** @name KvpFrame Value Retrieval */
/*@{*/
/** Returns the KvpValue in the given KvpFrame 'frame' that is associated with 'key'.
 *
 * Pointers passed as arguments into get_slot are the responsibility
 * of the caller.  Pointers returned by get_slot are owned by the
 * kvp_frame.  Make copies as needed.
 */
KvpValue   * kvp_frame_get_slot(KvpFrame * frame, 
                                 const char * key);

/** This routine return the last frame of the path.
 * If the frame path doesn't exist, it is created.  
 */
KvpFrame    * kvp_frame_get_frame (KvpFrame *frame, const char *,...);

/** This routine return the last frame of the path.
 * If the frame path doesn't exist, it is created.  
 */
KvpFrame    * kvp_frame_get_frame_gslist (KvpFrame *frame,
                                           GSList *key_path);

/** This routine return the last frame of the path.
 * If the frame path doesn't exist, it is created.  
 *
 * The kvp_frame_get_frame_slash() routine takes a single string
 *    where the keys are separated by slashes; thus, for example:
 *    /this/is/a/valid/path  and///so//is////this/
 *    Multiple slashes are compresed.  Leading slash is optional.
 *    The pointers . and .. are *not* followed/obeyed.  (This is 
 *    arguably a bug that needs fixing).
 *
 * 
 */
KvpFrame    * kvp_frame_get_frame_slash (KvpFrame *frame,
                                          const char *path);

/** This routine return the value at the end of the
 * path, or NULL if any portion of the path doesn't exist.
 */
KvpValue   * kvp_frame_get_slot_path (KvpFrame *frame,
                                       const char *first_key, ...);

/** This routine return the value at the end of the
 * path, or NULL if any portion of the path doesn't exist.
 */
KvpValue   * kvp_frame_get_slot_path_gslist (KvpFrame *frame,
                                              GSList *key_path);

/**
 * Similar returns as strcmp.
 **/
gint          kvp_frame_compare(const KvpFrame *fa, const KvpFrame *fb);
/*@}*/


gint          double_compare(double v1, double v2);

/** list convenience funcs. */
gint        kvp_glist_compare(const GList * list1, const GList * list2);

/** kvp_glist_copy() performs a deep copy of a <b>GList of
 *     kvp_frame's</b> (not to be confused with GLists of something
 *     else): same as mapping kvp_value_copy() over the elements and
 *     then copying the spine.
 */
GList     * kvp_glist_copy(const GList * list);

/** kvp_glist_delete() performs a deep delete of a <b>GList of
 *     kvp_frame's</b> (not to be confused with GLists of something
 *     else): same as mapping * kvp_value_delete() over the elements
 *     and then deleting the GList.
 */
void        kvp_glist_delete(GList * list);


/** @name KvpValue Constructors */
/*@{*/
/** The following routines are constructors for kvp_value.
 *    Those with pointer arguments copy in the value.
 *    The *_nc() versions do *not* copy in thier values, 
 *    but use them directly.
 */
KvpValue   * kvp_value_new_gint64(gint64 value);
KvpValue   * kvp_value_new_double(double value);
KvpValue   * kvp_value_new_gnc_numeric(gnc_numeric value);
KvpValue   * kvp_value_new_string(const char * value);
KvpValue   * kvp_value_new_guid(const GUID * guid);
KvpValue   * kvp_value_new_timespec(Timespec timespec);
KvpValue   * kvp_value_new_binary(const void * data, guint64 datasize);
/** Creates a KvpValue from a <b>GList of kvp_value's</b>! (Not to be
 * confused with GList's of something else!) */
KvpValue   * kvp_value_new_glist(const GList * value);
KvpValue   * kvp_value_new_frame(const KvpFrame * value);

/** value constructors (non-copying - KvpValue takes pointer ownership)
   values *must* have been allocated via glib allocators! (gnew, etc.) */
KvpValue   * kvp_value_new_binary_nc(void * data, guint64 datasize);
/** Creates a KvpValue from a <b>GList of kvp_value's</b>! (Not to be
 * confused with GList's of something else!) 
 *
 * This value constructor is non-copying (KvpValue takes pointer
 * ownership). The values *must* have been allocated via glib
 * allocators! (gnew, etc.) */
KvpValue   * kvp_value_new_glist_nc(GList *lst);
/** value constructors (non-copying - KvpValue takes pointer ownership)
   values *must* have been allocated via glib allocators! (gnew, etc.) */
KvpValue   * kvp_value_new_frame_nc(KvpFrame * value);

/** This is a deep (recursive) delete. */
void          kvp_value_delete(KvpValue * value);
/** This is a deep value copy. */
KvpValue   * kvp_value_copy(const KvpValue * value);
/*@}*/


/** @name KvpValue Value access */
/*@{*/
/** Value accessors. Those for GUID, binary, GList, KvpFrame and
  string are non-copying -- the caller can modify the value directly.

  Note that the above non-copying list did not include the
  get_string() function. But in fact that function has always been a
  non-copying one -- therefore don't free the result unless you want
  to delete the whole KvpFrame by yourself. */
KvpValueType kvp_value_get_type(const KvpValue * value);

gint64      kvp_value_get_gint64(const KvpValue * value);
double      kvp_value_get_double(const KvpValue * value);
gnc_numeric kvp_value_get_numeric(const KvpValue * value);
/** Value accessor. This one is non-copying -- the caller can modify
 * the value directly. */
char        * kvp_value_get_string(const KvpValue * value);
/** Value accessor. This one is non-copying -- the caller can modify
 * the value directly. */
GUID        * kvp_value_get_guid(const KvpValue * value);
/** Value accessor. This one is non-copying -- the caller can modify
 * the value directly. */
void        * kvp_value_get_binary(const KvpValue * value,
                                   guint64 * size_return); 
/** Returns the GList of kvp_frame's (not to be confused with GList's
 * of something else!) from the given kvp_frame.  This one is
 * non-copying -- the caller can modify the value directly. */
GList       * kvp_value_get_glist(const KvpValue * value);
/** Value accessor. This one is non-copying -- the caller can modify
 * the value directly. */
KvpFrame   * kvp_value_get_frame(const KvpValue * value);
Timespec    kvp_value_get_timespec(const KvpValue * value);

/**
 * Similar returns as strcmp.
 **/
gint          kvp_value_compare(const KvpValue *va, const KvpValue *vb);
/*@}*/


gchar* KvpValueTypeo_string(const KvpValue *val);

/** Manipulator: 
 *
 * copying - but more efficient than creating a new KvpValue manually. */
gboolean kvp_value_binary_append(KvpValue *v, void *data, guint64 size);

/** @name  Iterators */
/*@{*/
/** Traverse all of the slots in the given kvp_frame.  This function
   does not descend recursively to traverse any kvp_frames stored as
   slot values.  You must handle that in proc, with a suitable
   recursive call if desired. */
void kvp_frame_for_each_slot(KvpFrame *f,
                             void (*proc)(const char *key,
                                          KvpValue *value,
                                          gpointer data),
                             gpointer data);
/*@}*/

#endif
/** @} */
