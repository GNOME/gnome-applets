/*  panel-menu-links.h
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _PANEL_MENU_LINKS_H_
#define _PANEL_MENU_LINKS_H_

#include <glib.h>
#include <gtk/gtk.h>
#include <panel-applet.h>
#include "panel-menu.h"

G_BEGIN_DECLS

PanelMenuEntry *panel_menu_links_new (PanelMenu *parent, gchar *name);
void panel_menu_links_set_name (PanelMenuEntry *entry, gchar *name);
void panel_menu_links_destroy (PanelMenuEntry *entry);
void panel_menu_links_merge_ui (PanelMenuEntry *entry);
GtkWidget *panel_menu_links_get_widget (PanelMenuEntry *entry);
GtkWidget *panel_menu_links_get_checkitem (PanelMenuEntry *entry);
gchar *panel_menu_links_dump_xml (PanelMenuEntry *entry);
gboolean panel_menu_links_accept_drop (PanelMenuEntry *entry, GnomeVFSURI *uri);
gboolean panel_menu_links_append_item (PanelMenuEntry *entry, gchar *uri);
void panel_menu_links_new_with_dialog (PanelMenu *panel_menu);

G_END_DECLS

#endif
