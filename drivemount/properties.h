/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <glib.h>
#include <libgnome/libgnome.h>
#include <panel-applet.h>

#ifndef _DRIVEMOUNT_PROPERTIES_H_
#define _DRIVEMOUNT_PROPERTIES_H_

G_BEGIN_DECLS

void property_load(const gchar *path, DriveData *dd);
void property_save(const gchar *path, DriveData *dd);
void property_show(PanelApplet *applet, gpointer data);

G_END_DECLS
#endif
