/********************************************************************\
 * gncOwner.h -- Business Interface:  Object OWNERs                 *
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
/** @addtogroup Business
    @{ */
/** @addtogroup Owner
    @{ */
/** @file gncOwner.h
    @brief Business Interface:  Object OWNERs
    @author Copyright (C) 2001,2002 Derek Atkins <warlord@MIT.EDU>
    @author Copyright (c) 2005 Neil Williams <linux@codehelp.co.uk>
*/

#ifndef GNC_OWNER_H_
#define GNC_OWNER_H_

typedef struct gnc_owner_s GncOwner;

#define GNC_ID_OWNER   "gncOwner"

#include "qofinstance-p.h"
#include "gncCustomer.h"
#include "gncJob.h"
#include "gncVendor.h"
#include "gncEmployee.h"
#include "gnc-lot.h" 
#include "gnc-engine-util.h"

#define ENUM_OWNER_TYPE(_) \
  _(GNC_OWNER_NONE,)       \
  _(GNC_OWNER_UNDEFINED,)  \
  _(GNC_OWNER_CUSTOMER,)   \
  _(GNC_OWNER_JOB,)        \
  _(GNC_OWNER_VENDOR,)     \
  _(GNC_OWNER_EMPLOYEE,)

DEFINE_ENUM(GncOwnerType, ENUM_OWNER_TYPE)

/** \name QOF handling

Whilst GncOwner is not a formal QOF object, these functions
are still expected to be useful in making GncOwner transparent
to QOF as they can be used by objects like GncInvoice.
@{
*/
/** \brief Allow the type to be set separate from the union. */
void qofOwnerSetType(GncOwner *owner, const char* type_string);
/** \brief Allow the union to be set independently of the type. */
char* qofOwnerGetTypeString(GncOwner *owner);
/** return the type for the collection. */
QofIdType qofOwnerGetType(GncOwner *owner);
/** return the owner itself as an entity. */
QofEntity* qofOwnerGetOwner (GncOwner *owner);
/** set the owner from the entity. */
void qofOwnerSetEntity (GncOwner *owner, QofEntity *ent);

/** \brief QOF union set routine.

 If ::qofOwnerSetType has already been called, initialise the owner.\n
 If no type has been set, store the object in a temporary pointer.
 When qofOwnerSetType is then called, qofOwnerSetOwner is called again
 and uses the value in the pointer to initialise the owner.

QOF makes sure both owner and type set functions are called once.
*/
void qofOwnerSetOwner(GncOwner *owner, gpointer obj);

void qofOwnerSetUndefined (GncOwner *owner, gpointer obj);
void qofOwnerSetCustomer (GncOwner *owner, GncCustomer* customer);
void qofOwnerSetJob (GncOwner *owner, GncJob* job);
void qofOwnerSetVendor (GncOwner *owner, GncVendor* vendor);
void qofOwnerSetEmployee (GncOwner *owner, GncEmployee* employee);
/** \brief Set the parent owner. */
void qofOwnerSetEndOwner (GncOwner *owner, GncOwner *parent);
GncOwner* qofOwnerCreate (QofBook *book);

AS_STRING_DEC(GncOwnerType, ENUM_OWNER_TYPE)
FROM_STRING_DEC(GncOwnerType, ENUM_OWNER_TYPE)

gboolean
gncOwnerRegister(void);

/** @} */

/** \struct GncOwner */
struct gnc_owner_s {
  GncOwnerType     type;      /**< Customer, Job, Vendor, Employee or Undefined. */
  union {
    gpointer       undefined;
    GncCustomer *  customer;
    GncJob *       job;
    GncVendor *    vendor;
    GncEmployee *  employee;
  } owner;                   /**< holds the pointer to the owner object. */
  gpointer         qof_temp; /**< Set type independently of the owner. */
};

/** \name Setup routines
@{
*/
void gncOwnerInitUndefined (GncOwner *owner, gpointer obj);
void gncOwnerInitCustomer (GncOwner *owner, GncCustomer *customer);
void gncOwnerInitJob (GncOwner *owner, GncJob *job);
void gncOwnerInitVendor (GncOwner *owner, GncVendor *vendor);
void gncOwnerInitEmployee (GncOwner *owner, GncEmployee *employee);
/** @} */
/** \name Get routines.
@{
*/
GncOwnerType gncOwnerGetType (const GncOwner *owner);
gpointer gncOwnerGetUndefined (const GncOwner *owner);
GncCustomer * gncOwnerGetCustomer (const GncOwner *owner);
GncJob * gncOwnerGetJob (const GncOwner *owner);
GncVendor * gncOwnerGetVendor (const GncOwner *owner);
GncEmployee * gncOwnerGetEmployee (const GncOwner *owner);
/** @} */
void gncOwnerCopy (const GncOwner *src, GncOwner *dest);
gboolean gncOwnerEqual (const GncOwner *a, const GncOwner *b);
int gncOwnerCompare (const GncOwner *a, const GncOwner *b);

const char * gncOwnerGetName (GncOwner *owner);
gnc_commodity * gncOwnerGetCurrency (GncOwner *owner);

/** Get the GUID of the immediate owner */
const GUID * gncOwnerGetGUID (GncOwner *owner);
GUID gncOwnerRetGUID (GncOwner *owner);

gboolean gncOwnerIsValid (GncOwner *owner);

/**
 * Get the "parent" Owner or GUID thereof.  The "parent" owner
 * is the Customer or Vendor, or the Owner of a Job
 */
GncOwner * gncOwnerGetEndOwner (GncOwner *owner);
const GUID * gncOwnerGetEndGUID (GncOwner *owner);

/** attach an owner to a lot */
void gncOwnerAttachToLot (GncOwner *owner, GNCLot *lot);

/** Get the owner from the lot.  If an owner is found in the lot,
 * fill in "owner" and return TRUE.  Otherwise return FALSE.
 */
gboolean gncOwnerGetOwnerFromLot (GNCLot *lot, GncOwner *owner);

#define OWNER_TYPE        "type"
#define OWNER_TYPE_STRING "type-string"  /**< Allows the type to be handled externally. */
#define OWNER_CUSTOMER    "customer"
#define OWNER_JOB         "job"
#define OWNER_VENDOR      "vendor"
#define OWNER_EMPLOYEE    "employee"
#define OWNER_PARENT      "parent"
#define OWNER_PARENTG     "parent-guid"
#define OWNER_NAME        "name"

#define OWNER_FROM_LOT    "owner-from-lot"

/**
 * These two functions are mainly for the convenience of scheme code.
 * Normal C code has no need to ever use these two functions, and rather
 * can just use a GncOwner directly and just pass around a pointer to it.
 */
GncOwner * gncOwnerCreate (void);
void gncOwnerDestroy (GncOwner *owner);

#endif /* GNC_OWNER_H_ */
/** @} */
/** @} */
