/*  panel-menu-links.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libbonobo.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-directory.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include "panel-menu.h"
#include "panel-menu-desktop-item.h"
#include "panel-menu-common.h"
#include "panel-menu-config.h"
#include "panel-menu-path.h"
#include "panel-menu-links.h"

static const gchar *links_menu_xml =
	"    <placeholder name=\"ChildItem\">\n"
	"        <menuitem name=\"Action\" verb=\"Action\" label=\"Rename %s _Menu...\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-convert\"/>\n"
	"        <menuitem name=\"Regenerate\" verb=\"Regenerate\" label=\"_Regenerate %s\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-refresh\"/>\n"
	"        <menuitem name=\"Remove\" verb=\"Remove\" label=\"R_emove %s\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-close\"/>\n"
	"        <separator/>"
	"    </placeholder>";

typedef struct _PanelMenuLinks {
	gint id;
	GtkWidget *links;
	GtkWidget *menu;
	gchar *name;
	GList *items_list;
	GList *links_list;
} PanelMenuLinks;

static void panel_menu_links_set_list (PanelMenuEntry *entry, GList *list);

static void regenerate_menus_cb (GtkWidget *menuitem, PanelMenuEntry *entry,
				 const gchar *verb);
static gint panel_menu_links_remove_cb (GtkWidget *widget, GdkEventKey *event,
					PanelMenuLinks *links);
static void rename_links_cb (GtkWidget *widget, PanelMenuEntry *entry);

static gint object_counter = 0;

PanelMenuEntry *
panel_menu_links_new (PanelMenu *parent, gchar *name)
{
	PanelMenuEntry *entry;
	gchar *item_key;

	item_key = g_strdup_printf ("links%d/name", object_counter);
	panel_applet_gconf_set_string (parent->applet, item_key, name, NULL);
	g_free (item_key);
	entry = panel_menu_links_new_with_id (parent, object_counter);
	return entry;
}

PanelMenuEntry *
panel_menu_links_new_with_id (PanelMenu *parent, gint id)
{
	PanelMenuEntry *entry;
	PanelMenuLinks *links;
	GtkWidget *tearoff;
	GtkWidget *image;
	gchar *base_key;
	gchar *dir_key;
	GConfClient *client;
	gchar *name = NULL;

	if (id > object_counter)
		object_counter = id;

	entry = g_new0 (PanelMenuEntry, 1);
	entry->type = PANEL_MENU_TYPE_LINKS;
	entry->parent = parent;
	links = g_new0 (PanelMenuLinks, 1);
	entry->data = (gpointer) links;
	links->id = id;
	links->links = gtk_menu_item_new_with_label ("");
	panel_menu_common_widget_dnd_init (entry);
	gtk_widget_show (links->links);
	panel_menu_links_set_name (entry, name);
	links->menu = gtk_menu_new ();

	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (links->menu), tearoff);
	if (parent->menu_tearoffs) {
		gtk_widget_show (tearoff);
	}

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (links->links), links->menu);
	g_signal_connect (G_OBJECT (links->menu), "key_press_event",
			  G_CALLBACK (panel_menu_links_remove_cb), links);
	base_key = g_strdup_printf ("links%d", id);
	dir_key = panel_applet_gconf_get_full_key (parent->applet, base_key);
	client = gconf_client_get_default ();
	if (gconf_client_dir_exists (client, dir_key, NULL)) {
		gchar *key;
		key = g_strdup_printf ("%s/name", base_key);
		name = panel_applet_gconf_get_string (parent->applet, key, NULL);
		g_free (key);
		key = g_strdup_printf ("%s/links-list", base_key);
		panel_menu_links_set_list (entry,
			panel_applet_gconf_get_string_list (parent->applet, key));
		g_free (key);
	} else {
		name = g_strdup (_("Shortcuts"));
	}
	g_object_unref (G_OBJECT (client));
	g_free (base_key);
	g_free (dir_key);
	panel_menu_links_set_name (entry, name);
	g_free (name);
	object_counter++;
	return entry;
}

static void
panel_menu_links_set_list (PanelMenuEntry *entry, GList *list)
{
	PanelMenuLinks *links;
	GList *iter;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_LINKS);
	if (!list) return;

	links = (PanelMenuLinks *)entry->data;
	for (iter = list; iter; iter = iter->next) {
		panel_menu_links_append_item (entry, iter->data);
		g_free (iter->data);
	}
	g_list_free (list);
}

void
panel_menu_links_set_name (PanelMenuEntry *entry, gchar *name)
{
	PanelMenuLinks *links;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_LINKS);

	links = (PanelMenuLinks *) entry->data;
	if (links->name)
		g_free (links->name);
	links->name = name ? g_strdup (name) : g_strdup ("");
	gtk_label_set_text (GTK_LABEL (GTK_BIN (links->links)->child), name);
}

static void
regenerate_menus_cb (GtkWidget *menuitem, PanelMenuEntry *entry,
		     const gchar *verb)
{
	PanelMenuLinks *links;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_PATH);

	links = (PanelMenuLinks *) entry->data;
	if (links->items_list) {
		for (cur = links->items_list; cur; cur = cur->next) {
			gtk_widget_destroy (GTK_WIDGET (cur->data));
			cur->data = NULL;
		}
		g_list_free (links->items_list);
		links->items_list = NULL;
	}

	if (links->links_list) {
		GList *list;
		/* Here we copy the old list, and create a new list while iterating
		  over the old list, and then killing it */
		list = links->links_list;
		links->links_list = NULL;
		for (cur = list; cur; cur = cur->next) {
			panel_menu_links_append_item (entry,
						     (gchar *) cur->data);
			g_free (cur->data);
		}
		g_list_free (list);
	}
}

void
panel_menu_links_merge_ui (PanelMenuEntry *entry)
{
	PanelMenuLinks *links;
	BonoboUIComponent *component;
	gchar *xml;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_LINKS);

	links = (PanelMenuLinks *) entry->data;
	component = panel_applet_get_popup_component (entry->parent->applet);

	bonobo_ui_component_add_verb (component, "Action",
				     (BonoboUIVerbFn) rename_links_cb, entry);
	bonobo_ui_component_add_verb (component, "Regenerate",
				     (BonoboUIVerbFn) regenerate_menus_cb, entry);
	bonobo_ui_component_add_verb (component, "Remove",
				     (BonoboUIVerbFn) panel_menu_common_remove_entry, entry);
	xml = g_strdup_printf (links_menu_xml, links->name, links->name, links->name);
	bonobo_ui_component_set (component, "/popups/button3/ChildMerge/",
				 xml, NULL);
	g_free (xml);
}

void
panel_menu_links_destroy (PanelMenuEntry *entry)
{
	PanelMenuLinks *links;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_LINKS);

	links = (PanelMenuLinks *) entry->data;
	if (links->name)
		g_free (links->name);
	if (links->items_list)
		g_list_free (links->items_list);
	for (cur = links->links_list; cur; cur = cur->next) {
		if (cur->data)
			g_free (cur->data);
	}
	gtk_widget_destroy (links->links);
	g_free (links);
}

GtkWidget *
panel_menu_links_get_widget (PanelMenuEntry *entry)
{
	PanelMenuLinks *links;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_LINKS, NULL);

	links = (PanelMenuLinks *) entry->data;
	return (links->links);
}

gboolean
panel_menu_links_accept_drop (PanelMenuEntry *entry, GnomeVFSURI *uri)
{
	gchar *fileuri;
	gboolean retval = FALSE;

	g_return_val_if_fail (entry != NULL, FALSE);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_LINKS, FALSE);

	fileuri = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	g_print ("(links) uri is %s\n", fileuri);
	retval = panel_menu_links_append_item (entry, fileuri);
	if (retval)
		panel_menu_links_save_config (entry);
	g_free (fileuri);
	return (retval);
}

gboolean
panel_menu_links_append_item (PanelMenuEntry *entry, gchar *uri)
{
	PanelMenuLinks *links;
	PanelMenuDesktopItem *item;
	GnomeVFSFileInfo finfo;
	gchar *icon;
	GtkWidget *menuitem;
	GtkWidget *submenu;
	gboolean retval = FALSE;

	g_return_val_if_fail (entry != NULL, FALSE);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_LINKS, FALSE);

	links = (PanelMenuLinks *) entry->data;

	if ((!strncmp (uri, "applications:", strlen ("applications:")) || 
	     !strncmp (uri, "file:", strlen ("file:")))) {
		if (strstr (uri, ".desktop")) {
			if ((menuitem = panel_menu_common_menuitem_from_path (
			     uri, GTK_MENU_SHELL (links->menu), FALSE)))
				retval = TRUE;
		} else {
			gchar *name;
			name = g_path_get_basename (uri);
			if (name && !strcmp (name, "applnk")) {
				g_free (name);
				name = g_strdup (_("KDE Menus"));
			} else {
				g_free (name);
				name = g_strdup (_("Programs"));
			}
			submenu =
				panel_menu_common_menu_from_path (name,
								  uri,
								  GTK_MENU_SHELL
								  (links->menu),
								  TRUE);
			g_free (name);
			/* Dont kill this item if it doesnt have any children */
			g_object_set_data (G_OBJECT (submenu), "immortal",
					   GINT_TO_POINTER(TRUE));
			menuitem = GTK_MENU (submenu)->parent_menu_item;
			panel_menu_path_load ((const gchar *) uri,
					      GTK_MENU_SHELL (submenu));
			panel_menu_path_monitor ((const gchar *) uri,
						 GTK_MENU_ITEM (menuitem));
			retval = TRUE;
		}
	}
	if (retval) {
		links->items_list = g_list_append (links->items_list, menuitem);
		links->links_list =
			g_list_append (links->links_list, g_strdup (uri));
	}

	return retval;
}

static gint
panel_menu_links_remove_cb (GtkWidget *widget, GdkEventKey *event,
			    PanelMenuLinks *links)
{
	GtkWidget *item;
	GList *cur;
	gchar *string;
	gchar *temp;
	gint counter = 0;
	gboolean retval = FALSE;

	if (event->keyval == GDK_Delete) {
		item = GTK_MENU_SHELL (widget)->active_menu_item;
		string = g_object_get_data (G_OBJECT (item), "uri-path");
		if (!string)
			return (FALSE);
		for (cur = links->items_list; cur; cur = cur->next, counter++) {
			if (item == GTK_WIDGET (cur->data)) {
				links->items_list =
					g_list_remove (links->items_list,
						       cur->data);
				temp = (gchar *) g_list_nth_data (links->
								  links_list,
								  counter);
				g_print ("removing %s from the links list\n",
					 temp);
				links->links_list =
					g_list_remove (links->links_list, temp);
				gtk_widget_destroy (item);
				g_free (temp);
				retval = TRUE;
				break;
			}
		}
	}
	return (retval);
}

void
panel_menu_links_new_with_dialog (PanelMenu *panel_menu)
{
	GtkWidget *dialog;
	GtkWidget *name_entry;
	gint response;
	gchar *name;
	PanelMenuEntry *entry;

	dialog = panel_menu_common_single_entry_dialog_new (panel_menu,
							    _("Create links item..."),
							    _("_Name:"),
							    _("Shortcuts"),
							    &name_entry);
	gtk_widget_show (dialog);
	gtk_widget_grab_focus (name_entry);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_OK) {
		name = (gchar *) gtk_entry_get_text (GTK_ENTRY (name_entry));
		entry = panel_menu_links_new (panel_menu, name);
		panel_menu->entries =
			g_list_append (panel_menu->entries, (gpointer) entry);
		gtk_menu_shell_append (GTK_MENU_SHELL (panel_menu->menubar),
				       panel_menu_links_get_widget (entry));
		panel_menu_config_save_layout (panel_menu);
		panel_menu_links_save_config (entry);
	}
	gtk_widget_destroy (dialog);
}

static void
rename_links_cb (GtkWidget *widget, PanelMenuEntry *entry)
{
	PanelMenuLinks *links;
	GtkWidget *dialog;
	GtkWidget *name_entry;
	gint response;
	gchar *name;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_LINKS);

	links = (PanelMenuLinks *) entry->data;

	dialog = panel_menu_common_single_entry_dialog_new (entry->parent,
							    _("Rename links item..."),
							    _("_Name:"),
							    links->name,
							    &name_entry);
	gtk_widget_show (dialog);
	gtk_widget_grab_focus (name_entry);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_OK) {
		name = (gchar *) gtk_entry_get_text (GTK_ENTRY (name_entry));
		if (strcmp (name, links->name)) {
			panel_menu_links_set_name (entry, name);
			panel_menu_links_save_config (entry);
		}
	}
	gtk_widget_destroy (dialog);
}

gchar *
panel_menu_links_save_config (PanelMenuEntry *entry)
{
	PanelMenuLinks *links;
	PanelMenu *panel_menu;
	PanelApplet *applet;
	gchar *id;
	gchar *key;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_LINKS, NULL);

	panel_menu = entry->parent;
	applet = panel_menu->applet;
	links = (PanelMenuLinks *) entry->data;

	id = g_strdup_printf ("links%d", links->id);
	key = g_strdup_printf ("%s/name", id);
	panel_applet_gconf_set_string (applet, key, links->name, NULL);
	g_free (key);
	key = g_strdup_printf ("%s/links-list", id);
	panel_applet_gconf_set_string_list (applet, key, links->links_list);
	g_free (key);
	return id;
}

void
panel_menu_links_remove_config (PanelMenuEntry *entry)
{
	PanelMenuLinks *links;
	PanelMenu *panel_menu;
	PanelApplet *applet;
	GConfClient *client;
	gchar *base_key;
	gchar *key;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_LINKS);

	panel_menu = entry->parent;
	applet = panel_menu->applet;
	links = (PanelMenuLinks *) entry->data;

	client = gconf_client_get_default ();
	g_return_if_fail (client != NULL);
	base_key = panel_applet_get_preferences_key (applet);
	key = g_strdup_printf ("%s/links%d", base_key, links->id);
	_gconf_client_clean_dir (client, key);
	g_free (base_key);
	g_free (key);
	g_object_unref (G_OBJECT (client));
}
