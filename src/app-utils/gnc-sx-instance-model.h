/* 
 * gnc-sx-instance-model.h
 *
 * Copyright (C) 2006 Josh Sled <jsled@asynchronous.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, contact:
 *
 * Free Software Foundation           Voice:  +1-617-542-5942
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652
 * Boston, MA  02110-1301,  USA       gnu@gnu.org
 */

#ifndef _GNC_SX_INSTANCE_MODEL_H
#define _GNC_SX_INSTANCE_MODEL_H

#include "config.h"
#include <glib.h>
#include <glib-object.h>
#include "gnc-numeric.h"
#include "SchedXaction.h"

G_BEGIN_DECLS

#define GNC_TYPE_SX_INSTANCE_MODEL	      (gnc_sx_instance_model_get_type ())
#define GNC_SX_INSTANCE_MODEL(obj)	      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GNC_TYPE_SX_INSTANCE_MODEL, GncSxInstanceModel))
#define GNC_SX_INSTANCE_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GNC_TYPE_SX_INSTANCE_MODEL, GncSxInstanceModelClass))
#define GNC_IS_SX_INSTANCE_MODEL(obj)	      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNC_TYPE_SX_INSTANCE_MODEL))
#define GNC_IS_SX_INSTANCE_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GNC_TYPE_SX_INSTANCE_MODEL))
#define GNC_SX_INSTANCE_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GNC_TYPE_SX_INSTANCE_MODEL, GncSxInstanceModelClass))

typedef struct _GncSxInstanceModel
{
     GObject parent;
     gboolean disposed;

     /* private */
     gint qof_event_handler_id;

     /* signals */
     /* void (*added)(SchedXaction *sx); // gpointer user_data */
     /* void (*updated)(SchedXaction *sx); // gpointer user_data */
     /* void (*removing)(SchedXaction *sx); // gpointer user_data */

     /* public */
     GDate range_end;
     GList *sx_instance_list; /* <GncSxInstances*> */
} GncSxInstanceModel;

typedef struct _GncSxInstanceModelClass
{
     GObjectClass parent;

     guint removing_signal_id;
     guint updated_signal_id;
     guint added_signal_id;
} GncSxInstanceModelClass;

typedef struct _GncSxInstances
{
     SchedXaction *sx;
     GHashTable /** <name:char*,GncSxVariable*> **/ *variable_names;
     gboolean variable_names_parsed;
     
     GDate next_instance_date;
     
     /** GList<GncSxInstance*> **/
     GList *list; /* @fixme: s/list/?/ */
} GncSxInstances;

typedef enum 
{
     SX_INSTANCE_STATE_IGNORED,
     SX_INSTANCE_STATE_POSTPONED,
     SX_INSTANCE_STATE_TO_CREATE,
     SX_INSTANCE_STATE_REMINDER,
     SX_INSTANCE_STATE_CREATED,
     SX_INSTANCE_STATE_MAX_STATE
} GncSxInstanceState;

typedef struct _GncSxVariable
{
     gchar *name;
     gnc_numeric value; /**< only numeric values are supported. **/
     gboolean editable;
} GncSxVariable;

typedef struct _GncSxInstance
{
     GncSxInstances *parent; /**< the parent instances collection. **/
     void *temporal_state; /**< the sx creation temporal state. **/
     GncSxInstanceState orig_state; /**< the original state at generation time. **/
     GncSxInstanceState state; /**< the current state of the instance (during editing) **/
     GDate date; /**< the instance date. **/
     GHashTable *variable_bindings; /**< variable bindings. **/
} GncSxInstance;

GType gnc_sx_instance_model_get_type(void);

GncSxInstanceModel* gnc_sx_get_instances(GDate *range_end);

/**
 * Regenerates and updates the GncSxInstances* for the given SX.  Model
 * consumers are probably going to call this in response to seeing the
 * "update" signal, unless they need to be doing something else like
 * finishing an iteration over an existing GncSxInstances*.
 **/
void gnc_sx_instance_model_update_sx_instances(GncSxInstanceModel *model, SchedXaction *sx);
void gnc_sx_instance_model_remove_sx_instances(GncSxInstanceModel *model, SchedXaction *sx);

/** @return GList<GncSxVariable*> **/
GList *gnc_sx_instance_get_variables(GncSxInstance *inst);

Account* gnc_sx_get_template_transaction_account(SchedXaction *sx);

/**
 * @return caller-owned.
 **/
GHashTable* gnc_sx_instance_get_variables_for_parser(GHashTable *instance_var_hash);

GncSxVariable* gnc_sx_variable_new_full(gchar *name, gnc_numeric value, gboolean editable);

G_END_DECLS

#endif // _GNC_SX_INSTANCE_MODEL_H