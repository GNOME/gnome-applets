/*  panel-menu-pixbuf.h
 *
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


#ifndef _PANEL_MENU_PIXBUF_H_
#define _PANEL_MENU_PIXBUF_H_

#include <glib.h>
#include <gtk/gtk.h>
#include <panel-applet.h>

#include "panel-menu.h"

G_BEGIN_DECLS

/* Don't need to call this, we check later when a pixmap is asked for */
void panel_menu_pixbuf_init (void);
/* Please call this, it free's memory and stuff */
void panel_menu_pixbuf_exit (void);

gint panel_menu_pixbuf_get_icon_size (void);
void panel_menu_pixbuf_set_icon_size (gint size);

void panel_menu_pixbuf_set_icon (GtkMenuItem *menuitem,
				 const gchar *icon_filename);

#define ICON_SIZE_KEY "/apps/panel-menu-applet/default/icon-size"
#define ICON_SIZE_KEY_PARENT "/apps/panel-menu-applet"

G_END_DECLS

#endif
