/* WARNING ____ IMMATURE API ____ liable to change */

/* property-entries.h - Property entries.

   Copyright (C) 1998 Martin Baulig

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation; either version 2, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.
*/

#ifndef __PROPERTY_ENTRIES_H__
#define __PROPERTY_ENTRIES_H__

#include <config.h>
#include <gnome.h>

BEGIN_GNOME_DECLS

/* Adjustment block. */
GtkWidget *
gnome_property_entry_adjustments (GnomePropertyObject *object,
				  const gchar *label, gint num_items,
				  gint columns, gint internal_columns,
				  gint *table_pos, const gchar *texts [],
				  glong *data_ptr, glong *values);

END_GNOME_DECLS

#endif
