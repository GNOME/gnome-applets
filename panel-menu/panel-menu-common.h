/*  panel-menu-common.h
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

#ifndef _PANEL_MENU_COMMON_H_
#define _PANEL_MENU_COMMON_H_

#include <glib.h>
#include <gtk/gtk.h>
#include <panel-applet.h>
#include "panel-menu.h"
#include "panel-menu-desktop-item.h"

G_BEGIN_DECLS

void panel_menu_common_widget_dnd_init (PanelMenuEntry *entry);

void panel_menu_common_apps_menuitem_dnd_init (GtkWidget *menuitem);
void panel_menu_common_activate_apps_menuitem (GtkWidget *menuitem, gpointer data);
void panel_menu_common_destroy_apps_menuitem (GtkWidget *menuitem, gpointer data);

gchar *panel_menu_common_build_full_path (const gchar *path, const gchar *selection);

PanelMenuEntry *panel_menu_common_build_entry (PanelMenu *panel_menu,
					       const gchar *item);

PanelMenuEntry *panel_menu_common_find_applications (PanelMenu *panel_menu);
PanelMenuEntry *panel_menu_common_find_preferences (PanelMenu *panel_menu);
PanelMenuEntry *panel_menu_common_find_actions (PanelMenu *panel_menu);
PanelMenuEntry *panel_menu_common_find_windows (PanelMenu *panel_menu);
PanelMenuEntry *panel_menu_common_find_workspaces (PanelMenu *panel_menu);

GtkWidget *panel_menu_common_menuitem_from_path(gchar *uri, GtkMenuShell *parent,
						gboolean append);
GtkWidget *panel_menu_common_menu_from_path(gchar *name, gchar *subpath,
					    GtkMenuShell *parent, gboolean append);
void panel_menu_common_modify_menu_item (GtkWidget *menuitem,
					 PanelMenuDesktopItem *item);

GtkWidget *panel_menu_common_get_entry_menuitem (PanelMenuEntry *entry);

gchar *panel_menu_common_call_entry_save_config (PanelMenuEntry *entry);
void panel_menu_common_call_entry_remove_config (PanelMenuEntry *entry);
void panel_menu_common_call_entry_destroy (PanelMenuEntry *entry);

void panel_menu_common_merge_entry_ui (PanelMenuEntry *entry);

void panel_menu_common_remove_entry (GtkWidget *widget,
				     PanelMenuEntry *entry,
				     const char *verb);

void panel_menu_common_demerge_ui (PanelApplet *applet);

GtkWidget *panel_menu_common_construct_entry_popup (PanelMenuEntry *entry);

GtkWidget *panel_menu_common_single_entry_dialog_new (PanelMenu  *panel_menu,
						      char       *title,
						      char       *label,
						      char       *value,
						      GtkWidget **entry);

G_END_DECLS

#endif
