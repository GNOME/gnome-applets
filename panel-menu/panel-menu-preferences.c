/*  panel-menu-preferences.c
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
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-directory.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-monitor.h>

#include "panel-menu-desktop-item.h"

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-path.h"
#include "panel-menu-preferences.h"

static const gchar *preferences_menu_xml =
	"    <placeholder name=\"ChildItem\">\n"
	"%s"
	"        <menuitem name=\"Remove\" verb=\"Remove\" label=\"Remove Preferences\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-close\"/>\n"
	"        <separator/>"
	"    </placeholder>";

static const gchar *additional_xml =
	"        <menuitem name=\"Regenerate\" verb=\"Regenerate\" label=\"Regenerate Menus\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-refresh\"/>\n";

typedef struct _PanelMenuPreferences {
	GtkWidget *preferences;
	GtkWidget *menu;
} PanelMenuPreferences;

static void regenerate_menus_cb (GtkWidget *menuitem, PanelMenuEntry *entry,
				 const gchar *verb);

PanelMenuEntry *
panel_menu_preferences_new (PanelMenu *parent)
{
	PanelMenuEntry *entry;
	PanelMenuPreferences *preferences;
	GtkWidget *tearoff;

	entry = g_new0 (PanelMenuEntry, 1);
	entry->type = PANEL_MENU_TYPE_PREFERENCES;
	entry->parent = parent;
	preferences = g_new0 (PanelMenuPreferences, 1);
	entry->data = (gpointer) preferences;
	preferences->preferences = gtk_menu_item_new_with_label (_("Preferences"));
	panel_menu_common_widget_dnd_init (entry);
	gtk_widget_show (preferences->preferences);
	preferences->menu = gtk_menu_new ();

	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (preferences->menu), tearoff);
	if (parent->menu_tearoffs) {
		gtk_widget_show (tearoff);
	}

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (preferences->preferences),
				   preferences->menu);
	g_object_set_data (G_OBJECT (preferences->menu), "immortal",
			   GINT_TO_POINTER(TRUE));
	GTK_MENU (preferences->menu)->parent_menu_item = preferences->preferences;
	panel_menu_path_load ("preferences:///", GTK_MENU_SHELL(preferences->menu));
	return entry;
}

static void
regenerate_menus_cb (GtkWidget *menuitem, PanelMenuEntry *entry,
		     const gchar *verb)
{
	PanelMenuPreferences *preferences;
	GList *list;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_PREFERENCES);

	preferences = (PanelMenuPreferences *) entry->data;

	list = g_list_copy (GTK_MENU_SHELL (preferences->menu)->children);
	for (cur = list; cur; cur = cur->next) {
		if (g_object_get_data (G_OBJECT (cur->data), "uri-path"))
			gtk_widget_destroy (GTK_WIDGET (cur->data));
	}
	g_list_free (list);

	panel_menu_path_load ("preferences:///",
			      GTK_MENU_SHELL (preferences->menu));
}

void
panel_menu_preferences_merge_ui (PanelMenuEntry *entry)
{
	PanelMenuPreferences *preferences;
	BonoboUIComponent  *component;
	GnomeVFSMonitorHandle *monitor;
	gchar *xml;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_PREFERENCES);

	preferences = (PanelMenuPreferences *) entry->data;
	component = panel_applet_get_popup_component (entry->parent->applet);
	monitor = g_object_get_data (G_OBJECT (preferences->preferences), "vfs-monitor");
	if (!monitor) {
		bonobo_ui_component_add_verb (component, "Regenerate",
					     (BonoboUIVerbFn)regenerate_menus_cb, entry);
	}
	bonobo_ui_component_add_verb (component, "Remove",
				     (BonoboUIVerbFn)panel_menu_common_remove_entry, entry);
	xml = g_strdup_printf (preferences_menu_xml, monitor ? "" : additional_xml);
	bonobo_ui_component_set (component, "/popups/button3/ChildMerge/",
				 xml, NULL);
	g_free (xml);
}

void
panel_menu_preferences_destroy (PanelMenuEntry *entry)
{
	PanelMenuPreferences *preferences;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_PREFERENCES);

	preferences = (PanelMenuPreferences *) entry->data;
	gtk_widget_destroy (preferences->preferences);
	g_free (preferences);
}

GtkWidget *
panel_menu_preferences_get_widget (PanelMenuEntry *entry)
{
	PanelMenuPreferences *preferences;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_PREFERENCES, NULL);

	preferences = (PanelMenuPreferences *) entry->data;
	return preferences->preferences;
}

gchar *
panel_menu_preferences_save_config (PanelMenuEntry *entry)
{
	PanelMenuPreferences *preferences;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_PREFERENCES);

	preferences = (PanelMenuPreferences *)entry->data;

	return g_strdup ("preferences");
}
