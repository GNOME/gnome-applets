/*  panel-menu-config.c
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

#include <libbonobo.h>
#include <libbonoboui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-path.h"
#include "panel-menu-links.h"
#include "panel-menu-directory.h"
#include "panel-menu-documents.h"
#include "panel-menu-actions.h"
#include "panel-menu-windows.h"
#include "panel-menu-workspaces.h"

#include "panel-menu-config.h"

static gchar *panel_menu_config_get_layout_string (PanelMenu *panel_menu);

static void tearoff_visibility_changed (GConfClient *client, guint cnxn_id,
					GConfEntry *entry, PanelMenu *panel_menu);

void
panel_menu_config_load_prefs (PanelMenu *panel_menu)
{
	gchar *key;
	GConfClient *client;

	client = gconf_client_get_default ();

	key = panel_applet_get_preferences_key (panel_menu->applet);
	if (gconf_client_dir_exists (client, key, NULL)) {
		/* Leave this alone for now, im sure we will have stuff later */
	} else {
		/* Leave this alone for now, im sure we will have stuff later */
	}
	g_free (key);

	panel_menu->menu_tearoffs = gconf_client_get_bool (client, 
							   HAVE_TEAROFFS_KEY, NULL);
	panel_menu->client = client;
	gconf_client_add_dir (client, HAVE_TEAROFFS_KEY_PARENT,
			      GCONF_CLIENT_PRELOAD_NONE, NULL);
	panel_menu->tearoffs_id = gconf_client_notify_add (client, HAVE_TEAROFFS_KEY,
		(GConfClientNotifyFunc)	tearoff_visibility_changed, panel_menu,
		(GFreeFunc) NULL, NULL);
}

gboolean
panel_menu_config_load_layout (PanelMenu *panel_menu)
{
	gchar *key;
	GConfClient *client;
	gboolean retval = FALSE;
	gchar *layout = NULL;

	client = gconf_client_get_default ();
	key = panel_applet_get_preferences_key (panel_menu->applet);

	if (gconf_client_dir_exists (client, key, NULL)) {

		layout = panel_applet_gconf_get_string (panel_menu->applet,
							"layout", NULL);
		g_print ("(load-layout) layout is %s\n", layout);
		retval = TRUE;
	}
	if (!layout) {
		layout = g_strdup ("applications|actions|windows|workspaces");
		panel_applet_gconf_set_string (panel_menu->applet,
					      "layout", layout, NULL);
	}
	g_free (key);
	panel_menu_config_load (panel_menu, layout);
	g_free (layout);
	g_object_unref (G_OBJECT (client));
	return retval;
}

void
panel_menu_config_load (PanelMenu *panel_menu, const gchar *layout)
{
	gchar **items;
	gint index;

	items = g_strsplit (layout, "|", 50);
	if (!items)
		return;
	for (index = 0; items[index]; index++) {
		PanelMenuEntry *entry;
		entry = panel_menu_common_build_entry (panel_menu, items[index]);
		if (entry) {
			GtkWidget *item;
			item = panel_menu_common_get_entry_menuitem (entry);
			gtk_menu_shell_append (GTK_MENU_SHELL (panel_menu->menubar),
					       item);
			gtk_widget_show (item);
			panel_menu->entries = g_list_append (
				panel_menu->entries, entry);

		}
	}
}

void
panel_menu_config_save_prefs (PanelMenu *panel_menu)
{
	/*
	   How is it that we have no general preferences now ?
	   Must be that anti-prefs nazi seth 
	*/
}

void
panel_menu_config_save_layout (PanelMenu *panel_menu)
{
	gchar *layout;

	layout = panel_menu_config_get_layout_string (panel_menu);
	panel_applet_gconf_set_string (panel_menu->applet,
				      "layout", layout, NULL);
	g_print ("(save-layout) layout is %s\n", layout);
	g_free (layout);
}

void
panel_menu_config_save (PanelMenu *panel_menu)
{
	GList *cur;

	for (cur = panel_menu->entries; cur; cur = cur->next) {
		panel_menu_common_entry_save_config ((PanelMenuEntry *)cur->data);
	}
}

static gchar *
panel_menu_config_get_layout_string (PanelMenu *panel_menu)
{
	GString *layout;
	GList *cur;
	gchar *str;

	layout = g_string_new (NULL);
	for (cur = panel_menu->entries; cur; cur = cur->next) {
		gchar *temp;
		temp = panel_menu_common_call_entry_save_config ((PanelMenuEntry *)cur->data);
		if (temp) {
			if (cur != panel_menu->entries)
				g_string_append (layout, "|");
			g_string_append (layout, temp);
			g_free (temp);
		}
	}
	str = layout->str;
	g_string_free (layout, FALSE);
	return str;
}

void
panel_applet_gconf_set_string_list (PanelApplet *applet,
				    const char *key,
				    GList *strings)
{
	GConfClient *client;
	char *full = NULL;
	GSList *list = NULL;
	GList *iter = NULL;

	g_return_if_fail (key != NULL);

	client = gconf_client_get_default ();
	g_return_if_fail (client != NULL);
	for (iter = strings; iter; iter = iter->next) {
		list = g_slist_append (list, iter->data);
	}

	full = panel_applet_gconf_get_full_key (applet, key);
	gconf_client_set_list (client, full, GCONF_VALUE_STRING, list, NULL);
	g_free (full);
	g_slist_free (list);
	g_object_unref (G_OBJECT (client));
}

GList *
panel_applet_gconf_get_string_list (PanelApplet *applet,
				    const char *key)
{
	GConfClient *client;
	char *full = NULL;
	GList *result = NULL;
	GSList *list = NULL;
	GSList *iter = NULL;

	g_return_val_if_fail (key != NULL, NULL);

	client = gconf_client_get_default ();
	g_return_val_if_fail (client != NULL, NULL);

	full = panel_applet_gconf_get_full_key (applet, key);
	list = gconf_client_get_list (client, full, GCONF_VALUE_STRING, NULL);
	g_free (full);

	for (iter = list; iter; iter = iter->next) {
		if (iter->data && strlen ((char *)iter->data) > 1) {
			result = g_list_append (result, iter->data);
		}
	}
	g_slist_free (list);
	g_object_unref (G_OBJECT (client));
	return result;
}

void
_gconf_client_clean_dir (GConfClient *client, const gchar *dir)
{
	GSList *subdirs;
	GSList *entries;
	GSList *tmp;

	subdirs = gconf_client_all_dirs (client, dir, NULL);

	for (tmp = subdirs; tmp; tmp = tmp->next) {
		gchar *s = tmp->data;
		_gconf_client_clean_dir (client, s);
		g_free (s);
  	}
	g_slist_free (subdirs);
 
  	entries = gconf_client_all_entries (client, dir, NULL);

	for (tmp = entries; tmp; tmp = tmp->next) {
		GConfEntry *entry = tmp->data;
		gconf_client_unset (client, gconf_entry_get_key (entry), NULL);
		gconf_entry_free (entry);
  	}
	g_slist_free (entries);
	gconf_client_unset (client, dir, NULL);
}

static void
menuitem_propogate_tearoff_visibility (GtkMenuItem *menuitem, gboolean setting)
{
	GList *children;

	g_return_if_fail (menuitem != NULL);

	if (GTK_IS_TEAROFF_MENU_ITEM (menuitem)) {
		if (setting)
			gtk_widget_show (GTK_WIDGET (menuitem));
		else
			gtk_widget_hide (GTK_WIDGET (menuitem));
	} else if (menuitem->submenu) {
		GtkMenuShell *menu;
		menu = GTK_MENU_SHELL (menuitem->submenu);
		for (children = menu->children; children; children = children->next) {
			g_print ("child found, propogating to next level\n");
			menuitem_propogate_tearoff_visibility (GTK_MENU_ITEM (children->data),
							       setting);
		}
	}
}

static void
tearoff_visibility_changed (GConfClient *client,
			    guint cnxn_id,
			    GConfEntry *entry,
			    PanelMenu *panel_menu)
{
	const gchar *key;
	GConfValue  *value;
	gint setting;
	GList *cur;

	g_return_if_fail (client != NULL);
	g_return_if_fail (entry != NULL);

	key = gconf_entry_get_key (entry);
	value = gconf_entry_get_value (entry);
	setting = gconf_value_get_bool (value);
	g_print ("(tearoff-visiblity-changed) value %d\n", setting);
	panel_menu->menu_tearoffs = setting;
	for (cur = panel_menu->entries; cur; cur = cur->next) {
		PanelMenuEntry *entry;
		GtkMenuItem *menuitem;
		entry = (PanelMenuEntry *)cur->data;
		menuitem = GTK_MENU_ITEM(panel_menu_common_get_entry_menuitem (entry));
		menuitem_propogate_tearoff_visibility (menuitem,
						       panel_menu->menu_tearoffs);
	}
}
