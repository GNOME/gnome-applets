/*  panel-menu-path.c
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

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-directory.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include "panel-menu-desktop-item.h"

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-options.h"
#include "panel-menu-path.h"

static const gchar *path_menu_xml =
	"    <placeholder name=\"ChildItem\">\n"
	"        <menuitem name=\"Action\" verb=\"Action\" label=\"Set Path...\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-convert\"/>\n"
	"        <menuitem name=\"Remove\" verb=\"Remove\" label=\"Remove %s\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-close\"/>\n"
	"        <separator/>"
	"    </placeholder>";

typedef struct _PanelMenuPath {
	GtkWidget *checkitem;
	GtkWidget *path;
	GtkWidget *menu;
	GtkWidget *regenitem;
	gchar *base_path;
	GList *items_list;
	GList *paths_list;
} PanelMenuPath;

static void regenerate_menus_cb (GtkWidget *menuitem, PanelMenuEntry *entry);
static void set_visibility (GtkCheckMenuItem *checkitem, GtkWidget *target);
static void panel_menu_path_load (const gchar *uri, GtkMenuShell *parent);
static void directory_load_cb (GnomeVFSAsyncHandle *handle,
			       GnomeVFSResult result, GList *list,
			       guint entries_read, GtkMenuShell *parent);
static gint panel_menu_path_remove_cb (GtkWidget *widget, GdkEventKey *event,
				       PanelMenuPath *path);
static void change_path_cb (GtkWidget *widget, PanelMenuEntry *entry, const char *verb);
static GtkWidget *panel_menu_path_edit_dialog_new (gchar *title, gchar *value,
						   GtkWidget ** entry);

PanelMenuEntry *
panel_menu_path_new (PanelMenu *parent, gchar *menu_path)
{
	static gint number;
	PanelMenuEntry *entry;
	PanelMenuPath *path;
	GtkWidget *tearoff;
	GtkWidget *image;
	GtkWidget *sep;

	entry = g_new0 (PanelMenuEntry, 1);
	entry->type = PANEL_MENU_TYPE_MENU_PATH;
	entry->parent = parent;
	path = g_new0 (PanelMenuPath, 1);
	entry->data = (gpointer) path;
	path->path = gtk_menu_item_new_with_label ("");
	panel_menu_common_widget_dnd_init (entry);
	gtk_widget_show (path->path);
	path->checkitem = gtk_check_menu_item_new_with_label ("");
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (path->checkitem),
					TRUE);
	g_signal_connect (G_OBJECT (path->checkitem), "toggled",
			  G_CALLBACK (panel_menu_common_set_visibility),
			  path->path);
	gtk_widget_show (path->checkitem);
	path->menu = gtk_menu_new ();
	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (path->menu), tearoff);
	gtk_widget_show (tearoff);
	path->regenitem =
		gtk_image_menu_item_new_with_label (_("Regenerate Menus"));
	image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (path->regenitem),
				       image);
	gtk_widget_show (image);
	gtk_menu_shell_append (GTK_MENU_SHELL (path->menu), path->regenitem);
	g_signal_connect (G_OBJECT (path->regenitem), "activate",
			  G_CALLBACK (regenerate_menus_cb), entry);
	gtk_widget_show (path->regenitem);
	sep = gtk_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (path->menu), sep);
	gtk_widget_show (sep);
	sep = gtk_menu_item_new ();
	g_object_set_data (G_OBJECT (sep), "append-before",
			   GINT_TO_POINTER (TRUE));
	gtk_menu_shell_append (GTK_MENU_SHELL (path->menu), sep);
	gtk_widget_show (sep);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (path->path), path->menu);
	GTK_MENU (path->menu)->parent_menu_item = path->path;
	g_signal_connect (G_OBJECT (path->menu), "key_press_event",
			  G_CALLBACK (panel_menu_path_remove_cb), path);
	panel_menu_path_set_uri (entry, menu_path);
	return entry;
}

void
panel_menu_path_set_uri (PanelMenuEntry *entry, gchar *uri)
{
	PanelMenuPath *path;
	PanelMenuDesktopItem *item;
	gchar *full_path = NULL;
	gchar *label = NULL;
	gchar *show_label = NULL;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_MENU_PATH);

	path = (PanelMenuPath *) entry->data;
	if (path->base_path)
		g_free (path->base_path);
	full_path = panel_menu_common_build_full_path (uri, ".directory");
	item = panel_menu_desktop_item_load_uri ((const char *) full_path,
						 "Directory", TRUE);
	g_free (full_path);
	if (item) {
		path->base_path = g_strdup (uri);
		label = g_strdup (item->name);
		panel_menu_desktop_item_destroy (item);
	} else {
		path->base_path = g_strdup ("applications:");
		label = g_strdup ("Programs");
	}
	gtk_label_set_text (GTK_LABEL (GTK_BIN (path->path)->child), label);
	show_label = g_strdup_printf ("Show %s", label);
	gtk_label_set_text (GTK_LABEL (GTK_BIN (path->checkitem)->child),
			    show_label);
	g_free (show_label);
	g_free (label);
	regenerate_menus_cb (NULL, entry);
}

static void
regenerate_menus_cb (GtkWidget *menuitem, PanelMenuEntry *entry)
{
	PanelMenuPath *path;
	GList *list;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_MENU_PATH);

	path = (PanelMenuPath *) entry->data;
	if (path->items_list) {
		g_list_free (path->items_list);
		path->items_list = NULL;
	}

	list = g_list_copy (GTK_MENU_SHELL (path->menu)->children);
	for (cur = list; cur; cur = cur->next) {
		if (g_object_get_data (G_OBJECT (cur->data), "uri-path"))
			gtk_widget_destroy (GTK_WIDGET (cur->data));
	}
	g_list_free (list);
	panel_menu_path_load ((const gchar *) path->base_path,
			      GTK_MENU_SHELL (path->menu));
	if (path->paths_list) {
		/* Here we copy the old list, and create a new list while iterating over the old list, and then killing it */
		list = g_list_copy (path->paths_list);
		g_list_free (path->paths_list);
		path->paths_list = NULL;
		for (cur = list; cur; cur = cur->next) {
			panel_menu_path_append_item (entry,
						     (gchar *) cur->data);
			g_free (cur->data);
		}
		g_list_free (list);
	}
}

void
panel_menu_path_merge_ui (PanelMenuEntry *entry)
{
	PanelMenuPath *path;
	BonoboUIComponent  *component;
	gchar *xml;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_MENU_PATH);

	path = (PanelMenuPath *) entry->data;
	component = panel_applet_get_popup_component (entry->parent->applet);

	bonobo_ui_component_add_verb (component, "Action",
				     (BonoboUIVerbFn)change_path_cb, entry);
	bonobo_ui_component_add_verb (component, "Remove",
				     (BonoboUIVerbFn)panel_menu_common_remove_entry, entry);
	xml = g_strdup_printf (path_menu_xml, gtk_label_get_text (
		GTK_LABEL (GTK_BIN (path->path)->child)));
	bonobo_ui_component_set (component, "/popups/button3/ChildMerge/",
				 xml, NULL);
	g_free (xml);
}

void
panel_menu_path_destroy (PanelMenuEntry *entry)
{
	PanelMenuPath *path;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_MENU_PATH);

	path = (PanelMenuPath *) entry->data;
	gtk_widget_destroy (path->checkitem);
	gtk_widget_destroy (path->path);
	if (path->base_path)
		g_free (path->base_path);
	if (path->items_list)
		g_list_free (path->items_list);
	for (cur = path->paths_list; cur; cur = cur->next) {
		g_free (cur->data);
	}
	g_list_free (path->paths_list);
	g_free (path);
}

GtkWidget *
panel_menu_path_get_widget (PanelMenuEntry *entry)
{
	PanelMenuPath *path;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_MENU_PATH, NULL);

	path = (PanelMenuPath *) entry->data;
	return path->path;
}

GtkWidget *
panel_menu_path_get_checkitem (PanelMenuEntry *entry)
{
	PanelMenuPath *path;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_MENU_PATH, NULL);

	path = (PanelMenuPath *) entry->data;
	return path->checkitem;
}

gchar *
panel_menu_path_dump_xml (PanelMenuEntry *entry)
{
	PanelMenuPath *path;
	GString *string;
	GList *cur;
	gchar *str;
	gboolean visible;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_MENU_PATH, NULL);

	path = (PanelMenuPath *) entry->data;
	visible = GTK_CHECK_MENU_ITEM (path->checkitem)->active;

	string = g_string_new ("    <path-item>\n" "        <base-path>");
	g_string_append (string, path->base_path);
	g_string_append (string, "</base-path>\n");
	for (cur = path->paths_list; cur; cur = cur->next) {
		g_string_append (string, "        <path>");
		g_string_append (string, (gchar *) cur->data);
		g_string_append (string, "</path>\n");
	}
	g_string_append (string, "        <visible>");
	g_string_append (string, visible ? "true" : "false");
	g_string_append (string, "</visible>\n");
	g_string_append (string, "    </path-item>\n");
	str = string->str;
	g_string_free (string, FALSE);
	return str;
}

gboolean
panel_menu_path_accept_drop (PanelMenuEntry *entry, GnomeVFSURI *uri)
{
	gchar *fileuri;
	gboolean retval = FALSE;

	g_return_val_if_fail (entry != NULL, FALSE);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_MENU_PATH, FALSE);

	fileuri = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	g_print ("(panel-menu-path) uri is %s\n", fileuri);
	retval = panel_menu_path_append_item (entry, fileuri);
	if (retval)
		panel_menu_common_set_changed (entry->parent);
	g_free (fileuri);
	return retval;
}

static void
panel_menu_path_load (const gchar *uri, GtkMenuShell *parent)
{
	GnomeVFSResult result;
	GnomeVFSAsyncHandle *load_handle;
	gchar *path;

	if (uri && parent) {
		path = panel_menu_common_build_full_path (uri, "");
		g_object_set_data (G_OBJECT (parent), "uri-path", path);
		gnome_vfs_async_load_directory (&load_handle, path,
						GNOME_VFS_FILE_INFO_FOLLOW_LINKS,
						5, 1,
						(GnomeVFSAsyncDirectoryLoadCallback)
						directory_load_cb,
						(gpointer) parent);
	}
}

static void
directory_load_cb (GnomeVFSAsyncHandle *handle, GnomeVFSResult result,
		   GList *list, guint entries_read, GtkMenuShell *parent)
{
	GList *iter = NULL;
	GnomeVFSFileInfo *finfo = NULL;
	GtkWidget *menuitem = NULL;
	GtkWidget *submenu = NULL;
	PanelMenuDesktopItem *item;
	gchar *icon = NULL;
	gchar *path = NULL;
	gchar *subpath = NULL;
	gint count = 0;

	path = (gchar *) g_object_get_data (G_OBJECT (parent), "uri-path");
	for (iter = list, count = 0; iter && count < entries_read;
	     iter = iter->next, count++) {
		finfo = (GnomeVFSFileInfo *) iter->data;
		subpath = panel_menu_common_build_full_path (path, finfo->name);
		if (!subpath)
			continue;
		if (finfo->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
			submenu =
				panel_menu_common_menu_from_path (finfo->name,
								  subpath,
								  parent,
								  FALSE);
			panel_menu_path_load ((const gchar *) subpath,
					      GTK_MENU_SHELL (submenu));
		} else {
			if (!strcmp (".directory", finfo->name)) {
				item = panel_menu_desktop_item_load_uri ((const
									  char
									  *)
									 subpath,
									 "Directory",
									 TRUE);
				if (item) {
					/* Set the localized label of the parent menu item, and an icon if there is one */
					/* Dont bother trying to set an icon if the parent wont take one anyway */
					gtk_label_set_text (GTK_LABEL
							    (GTK_BIN
							     (GTK_MENU
							      (parent)->
							      parent_menu_item)->
							     child),
							    item->name);
					if (GTK_IS_IMAGE_MENU_ITEM
					    (GTK_MENU (parent)->
					     parent_menu_item)) {
						icon = panel_menu_desktop_item_find_icon (item->icon);
						if (icon) {
							panel_menu_common_set_icon_scaled_from_file
								(GTK_MENU_ITEM
								 (GTK_MENU
								  (parent)->
								  parent_menu_item),
								 icon);
							g_free (icon);
						} else {	/* Try and fall back to a default here if someone has jacked their GNOME Core install */

							icon = panel_menu_desktop_item_find_icon ("gnome-directory.png");
							if (icon) {
								panel_menu_common_set_icon_scaled_from_file
									(GTK_MENU_ITEM
									 (GTK_MENU
									  (parent)->
									  parent_menu_item),
									 icon);
								g_free (icon);
							}
						}
					}
					panel_menu_desktop_item_destroy (item);
				}
			} else if (strcmp (".order", finfo->name)) {	/* We want to skip these items */
				menuitem =
					panel_menu_common_menuitem_from_path
					(subpath, parent, FALSE);
			}
		}
		g_free (subpath);
	}
	if (result != GNOME_VFS_OK) {
		if (parent) {
			subpath =
				(gchar *) g_object_get_data (G_OBJECT (parent),
							     "uri-path");
			g_free (subpath);
			g_object_set_data (G_OBJECT (parent), "uri-path", NULL);
			if (GTK_MENU (parent)->parent_menu_item
			    && g_list_length (parent->children) < 2) {
				gtk_widget_destroy (GTK_MENU (parent)->
						    parent_menu_item);
			}
		}
	}
}

gboolean
panel_menu_path_append_item (PanelMenuEntry *entry, gchar *uri)
{
	PanelMenuPath *path;
	GtkWidget *menuitem;
	GtkWidget *submenu;
	gboolean retval = FALSE;

	g_return_val_if_fail (entry != NULL, FALSE);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_MENU_PATH, FALSE);

	path = (PanelMenuPath *) entry->data;

	if (!strncmp (uri, "applications:", strlen ("applications:"))
	    || (!strncmp (uri, "file:", strlen ("file:"))
		&& strstr (uri, ".desktop"))) {
		if (strstr (uri, ".desktop")) {
			if ((menuitem =
			     panel_menu_common_menuitem_from_path (uri,
								   GTK_MENU_SHELL
								   (path->menu),
								   TRUE)))
				retval = TRUE;
		} else {
			submenu =
				panel_menu_common_menu_from_path (_("Programs"),
								  uri,
								  GTK_MENU_SHELL
								  (path->menu),
								  TRUE);
			menuitem = GTK_MENU (submenu)->parent_menu_item;
			panel_menu_path_load ((const gchar *) uri,
					      GTK_MENU_SHELL (submenu));
			retval = TRUE;
		}
		if (retval) {
			path->items_list =
				g_list_append (path->items_list, menuitem);
			path->paths_list =
				g_list_append (path->paths_list,
					       g_strdup (uri));
		}
	}
	return retval;
}

static gint
panel_menu_path_remove_cb (GtkWidget *widget, GdkEventKey *event,
			   PanelMenuPath *path)
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
			return FALSE;
		for (cur = path->items_list; cur; cur = cur->next, counter++) {
			if (item == GTK_WIDGET (cur->data)) {
				path->items_list =
					g_list_remove (path->items_list,
						       cur->data);
				temp = (gchar *) g_list_nth_data (path->
								  paths_list,
								  counter);
				g_print ("removing %s from the paths list\n",
					 temp);
				path->paths_list =
					g_list_remove (path->paths_list, temp);
				gtk_widget_destroy (item);
				g_free (temp);
				retval = TRUE;
				break;
			}
		}
	}
	return retval;
}

void
panel_menu_path_new_with_dialog (PanelMenu *panel_menu)
{
	GtkWidget *dialog;
	GtkWidget *path_entry;
	gint response;
	gchar *path;
	PanelMenuEntry *entry;

	dialog = panel_menu_path_edit_dialog_new (_("Create menu path item..."),
						  "applications:", &path_entry);
	gtk_widget_show (dialog);
	gtk_widget_grab_focus (path_entry);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_OK) {
		path = (gchar *) gtk_entry_get_text (GTK_ENTRY (path_entry));
		entry = panel_menu_path_new (panel_menu, path);
		panel_menu->entries =
			g_list_append (panel_menu->entries, (gpointer) entry);
		panel_menu_options_append_option (panel_menu_common_find_options
						  (panel_menu),
						  panel_menu_path_get_checkitem
						  (entry));
		gtk_menu_shell_append (GTK_MENU_SHELL (panel_menu->menubar),
				       panel_menu_path_get_widget (entry));
		panel_menu_common_set_changed (panel_menu);
	}
	gtk_widget_destroy (dialog);
}

static void
change_path_cb (GtkWidget *widget, PanelMenuEntry *entry, const char *verb)
{
	PanelMenuPath *path;
	GtkWidget *dialog;
	GtkWidget *path_entry;
	gint response;
	gchar *new_path;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_MENU_PATH);

	path = (PanelMenuPath *) entry->data;
	dialog = panel_menu_path_edit_dialog_new (_("Edit path item..."),
						  path->base_path, &path_entry);
	gtk_widget_show (dialog);
	gtk_widget_grab_focus (path_entry);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_OK) {
		new_path =
			(gchar *) gtk_entry_get_text (GTK_ENTRY (path_entry));
		if (strcmp (path->base_path, new_path)) {
			panel_menu_path_set_uri (entry, new_path);
			panel_menu_common_set_changed (entry->parent);
		}
	}
	gtk_widget_destroy (dialog);
}

static GtkWidget *
panel_menu_path_edit_dialog_new (gchar *title, gchar *value,
				 GtkWidget ** entry)
{
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *browse;

	dialog = gtk_dialog_new_with_buttons (title,
					      NULL, GTK_DIALOG_MODAL,
					      GTK_STOCK_CLOSE,
					      GTK_RESPONSE_CLOSE, GTK_STOCK_OK,
					      GTK_RESPONSE_OK, NULL);

	box = GTK_DIALOG (dialog)->vbox;

	hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 5);
	gtk_widget_show (hbox);

	label = gtk_label_new (_("Path:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);
	gtk_widget_show (label);

	*entry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox), *entry, TRUE, TRUE, 5);
	gtk_widget_show (*entry);
	gtk_entry_set_text (GTK_ENTRY (*entry), value);

	browse = gtk_button_new_with_label (_("Browse..."));
	gtk_box_pack_start (GTK_BOX (hbox), browse, FALSE, FALSE, 5);
	gtk_widget_show (browse);
	return dialog;
}
