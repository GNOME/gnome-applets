/*  panel-menu-path.h
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

#ifndef _PANEL_MENU_PATH_H_
#define _PANEL_MENU_PATH_H_

#include <glib.h>
#include <gtk/gtk.h>
#include <panel-applet.h>
#include "panel-menu.h"

G_BEGIN_DECLS

PanelMenuEntry *panel_menu_path_new (PanelMenu *parent, gchar *menu_path);
PanelMenuEntry *panel_menu_path_new_with_id (PanelMenu *parent, gint id);
void panel_menu_path_set_uri (PanelMenuEntry *entry, gchar *uri);
void panel_menu_path_merge_ui (PanelMenuEntry *entry);
void panel_menu_path_destroy (PanelMenuEntry *entry);
GtkWidget *panel_menu_path_get_widget (PanelMenuEntry *entry);
gboolean panel_menu_path_accept_drop (PanelMenuEntry *entry, GnomeVFSURI *uri);
gboolean panel_menu_path_append_item (PanelMenuEntry *entry, gchar *uri);
void panel_menu_path_new_with_dialog (PanelMenu *panel_menu);
gchar *panel_menu_path_save_config (PanelMenuEntry *entry);
void panel_menu_path_remove_config (PanelMenuEntry *entry);

G_END_DECLS

#endif
