/*  panel-menu-common.c
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <libbonobo.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>

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

#include "panel-menu-desktop-item.h"
#include "panel-menu-applications.h"
#include "panel-menu-path.h"
#include "panel-menu-links.h"
#include "panel-menu-directory.h"
#include "panel-menu-documents.h"
#include "panel-menu-actions.h"
#include "panel-menu-windows.h"
#include "panel-menu-workspaces.h"
#include "panel-menu-pixbuf.h"
#include "panel-menu-config.h"
#include "panel-menu-common.h"

enum {
	TARGET_URI_LIST,
	TARGET_PANEL_MENU_ENTRY
};

static GtkTargetEntry drag_types[] = {
	{"text/uri-list", 0, TARGET_URI_LIST}
};
static gint n_drag_types = sizeof (drag_types) / sizeof (GtkTargetEntry);

static GtkTargetEntry widget_drag_types[] = {
	{"application/panel-menu-entry", GTK_TARGET_SAME_APP,
	 TARGET_PANEL_MENU_ENTRY}
};
static gint n_widget_drag_types =
	sizeof (widget_drag_types) / sizeof (GtkTargetEntry);

static GtkTooltips *menu_tooltips = NULL;

static void widget_dnd_drag_begin_cb (GtkWidget *widget,
				      GdkDragContext *context);
static void widget_dnd_set_data_cb (GtkWidget *widget,
				    GdkDragContext *context,
				    GtkSelectionData *selection_data,
				    guint info, guint time,
				    PanelMenuEntry *entry);

static void apps_menuitem_dnd_drag_begin_cb (GtkWidget *widget,
					     GdkDragContext *context,
					     gpointer data);
static void apps_menuitem_dnd_set_data_cb (GtkWidget *widget,
					   GdkDragContext *context,
					   GtkSelectionData *selection_data,
					   guint info, guint time,
					   gpointer data);

static gboolean check_dir_exists (gchar *path);

void
panel_menu_common_widget_dnd_init (PanelMenuEntry *entry)
{
	GtkWidget *menuitem;

	menuitem = panel_menu_common_get_entry_menuitem (entry);
	gtk_drag_source_set (menuitem, GDK_BUTTON2_MASK, widget_drag_types,
			     n_widget_drag_types,
			     GDK_ACTION_MOVE);
	g_signal_connect (G_OBJECT (menuitem), "drag_begin",
			  G_CALLBACK (widget_dnd_drag_begin_cb), NULL);
	g_signal_connect (G_OBJECT (menuitem), "drag_data_get",
			  G_CALLBACK (widget_dnd_set_data_cb), entry);
}

static void
widget_dnd_drag_begin_cb (GtkWidget *widget, GdkDragContext *context)
{
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *image;
	GdkPixbuf *pixbuf;

	if (GTK_IS_IMAGE_MENU_ITEM (widget)) {
		if ((image =
		     gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM
						   (widget)))
		    && (pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (image))))
			gtk_drag_set_icon_pixbuf (context, pixbuf, -5, -5);
	} else {
		window = gtk_window_new (GTK_WINDOW_POPUP);
		button = gtk_button_new_with_label (gtk_label_get_text
						    (GTK_LABEL
						    (GTK_BIN (widget)->
						      child)));
		gtk_container_add (GTK_CONTAINER (window), button);
		gtk_widget_show_all (button);
		gtk_drag_set_icon_widget (context, window, 0, 0);
	}
}

static void
widget_dnd_set_data_cb (GtkWidget *widget, GdkDragContext *context,
			GtkSelectionData *selection_data, guint info,
			guint time, PanelMenuEntry *entry)
{
	PanelMenu *panel_menu;
	GtkWidget *menuitem;
	gint position;

	if (info == TARGET_PANEL_MENU_ENTRY) {
		panel_menu = entry->parent;
		position = g_list_index (panel_menu->entries, entry);
		menuitem = panel_menu_common_get_entry_menuitem (entry);
		g_object_ref (G_OBJECT (menuitem));
		gtk_container_remove (GTK_CONTAINER (menuitem->parent),
				      menuitem);
		g_object_set_data (G_OBJECT (menuitem), "old-position",
				   GINT_TO_POINTER (position));
		panel_menu->entries =
			g_list_remove (panel_menu->entries, entry);
		gtk_selection_data_set (selection_data, selection_data->target,
					8, (gpointer) & entry,
					sizeof (gpointer));
	} else {
		gtk_selection_data_set (selection_data, selection_data->target,
					8, NULL, 0);
	}
}

void
panel_menu_common_apps_menuitem_dnd_init (GtkWidget *menuitem)
{
	gtk_drag_source_set (menuitem, GDK_BUTTON1_MASK, drag_types,
			     n_drag_types, GDK_ACTION_COPY | GDK_ACTION_LINK);
	g_signal_connect (G_OBJECT (menuitem), "drag_begin",
			  G_CALLBACK (apps_menuitem_dnd_drag_begin_cb), NULL);
	g_signal_connect (G_OBJECT (menuitem), "drag_data_get",
			  G_CALLBACK (apps_menuitem_dnd_set_data_cb), NULL);
}

static void
apps_menuitem_dnd_drag_begin_cb (GtkWidget *widget, GdkDragContext *context,
				 gpointer data)
{
	GtkWidget *image;
	GdkPixbuf *pixbuf;

	if (GTK_IS_IMAGE_MENU_ITEM (widget)) {
		if ((image =
		     gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM
						    (widget)))
		    && (pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (image))))
			gtk_drag_set_icon_pixbuf (context, pixbuf, -5, -5);
	}
}

static void
apps_menuitem_dnd_set_data_cb (GtkWidget *widget, GdkDragContext *context,
			       GtkSelectionData *selection_data, guint info,
			       guint time, gpointer data)
{
	gchar *text = NULL;

	if ((text = g_object_get_data (G_OBJECT (widget), "uri-path"))) {
		text = g_strconcat (text, "\r\n", NULL);
		gtk_selection_data_set (selection_data, selection_data->target,
					8, text, strlen (text));
		g_free (text);
	} else {
		gtk_selection_data_set (selection_data, selection_data->target,
					8, NULL, 0);
	}
}

void
panel_menu_common_activate_apps_menuitem (GtkWidget *menuitem, gpointer data)
{
	gchar *exec_string;
	gchar **argv;
	int argc;

	exec_string = g_object_get_data (G_OBJECT (menuitem), "exec-string");
	g_print ("executing %s\n", exec_string);
	argv = g_strsplit ((const gchar *) exec_string, " ", 20);
	for (argc = 0; argv[argc]; argc++);
	gnome_execute_async (g_get_home_dir (), argc, argv);
	g_strfreev (argv);
}

void
panel_menu_common_destroy_apps_menuitem (GtkWidget *menuitem, gpointer data)
{
	data = g_object_get_data (G_OBJECT (menuitem), "uri-path");
	if (data)
		g_free (data);
	data = g_object_get_data (G_OBJECT (menuitem), "exec-string");
	if (data)
		g_free (data);
}

gchar *
panel_menu_common_build_full_path (const gchar *path, const gchar *selection)
{
	GnomeVFSURI *uri = NULL;
	GnomeVFSURI *full_uri = NULL;
	gchar *text = NULL;
	gchar *temp = NULL;

	uri = gnome_vfs_uri_new (path);
	if (uri) {
		if (selection) {
			full_uri = gnome_vfs_uri_append_string (uri, selection);
			gnome_vfs_uri_unref (uri);
			uri = full_uri;
		}
		text = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
		gnome_vfs_uri_unref (uri);
		temp = gnome_vfs_unescape_string (text, "/");
		g_free (text);
		text = temp;
	}
	return (text);
}

static void
init_menu_tooltips (void)
{
	if (menu_tooltips == NULL)
		menu_tooltips = gtk_tooltips_new ();
}

GtkWidget *
panel_menu_common_menuitem_from_path (gchar *uri, GtkMenuShell *parent,
				      gboolean append)
{
	PanelMenuDesktopItem *item;
	GtkWidget *menuitem = NULL;
	gchar *icon = NULL;
	GList *cur = NULL;
	gint position = 0;

	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (parent != NULL, NULL);

	init_menu_tooltips ();
	if (!append) {
		for (cur = parent->children; cur; cur = cur->next, position++) {
			if (g_object_get_data
			    (G_OBJECT (cur->data), "append-before")) {
				break;
			} else if (g_object_get_data
				   (G_OBJECT (cur->data), "append-after")) {
				position++;
				break;
			}
		}
	}
	item = panel_menu_desktop_item_load_uri ((const char *) uri,
						 "Application", TRUE);
	if (item && item->name && item->exec) {
		menuitem = gtk_image_menu_item_new_with_label (item->name);
		panel_menu_common_apps_menuitem_dnd_init (menuitem);
		icon = panel_menu_desktop_item_find_icon (item->icon);
		panel_menu_pixbuf_set_icon (GTK_MENU_ITEM (menuitem),
					    icon ? icon : "unknown");
		g_free (icon);

		if (item->comment)
			gtk_tooltips_set_tip (menu_tooltips, menuitem,
					      item->comment, item->comment);
		if (append)
			gtk_menu_shell_append (parent, menuitem);
		else
			gtk_menu_shell_insert (parent, menuitem, position);
		if (item->exec) {
			g_object_set_data (G_OBJECT (menuitem), "exec-string",
					   g_strdup (item->exec));
			g_signal_connect (G_OBJECT (menuitem), "activate",
					  G_CALLBACK
					  (panel_menu_common_activate_apps_menuitem),
					  NULL);
		}
		g_object_set_data (G_OBJECT (menuitem), "uri-path",
				   g_strdup (uri));
		g_signal_connect (G_OBJECT (menuitem), "destroy",
				  G_CALLBACK
				  (panel_menu_common_destroy_apps_menuitem),
				  NULL);
		gtk_widget_show (menuitem);
	}
	if (item)
		panel_menu_desktop_item_destroy (item);
	return (menuitem);
}

GtkWidget *
panel_menu_common_menu_from_path (gchar *name, gchar *subpath,
				  GtkMenuShell *parent, gboolean append)
{
	GtkWidget *menuitem;
	GtkWidget *submenu;
	GtkWidget *tearoff;
	GList *cur = NULL;
	GConfClient *client;
	gint position = 0;

	if (!append) {
		for (cur = parent->children; cur; cur = cur->next, position++) {
			if (g_object_get_data
			    (G_OBJECT (cur->data), "append-before")) {
				break;
			} else if (g_object_get_data
				   (G_OBJECT (cur->data), "append-after")) {
				position++;
				break;
			}
		}
	}
	menuitem = gtk_image_menu_item_new_with_label (name);
	g_object_set_data (G_OBJECT (menuitem), "uri-path", g_strdup (subpath));
	g_signal_connect (G_OBJECT (menuitem), "destroy",
			  G_CALLBACK (panel_menu_common_destroy_apps_menuitem),
			  NULL);
	submenu = gtk_menu_new ();
	
	client = gconf_client_get_default();
        tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), tearoff);
	if (gconf_client_get_bool (client, 
				  HAVE_TEAROFFS_KEY, NULL)) {
		gtk_widget_show (tearoff);
	}
	g_object_unref (G_OBJECT (client));


	GTK_MENU (submenu)->parent_menu_item = menuitem;
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), submenu);
	if (append)
		gtk_menu_shell_append (parent, menuitem);
	else
		gtk_menu_shell_insert (parent, menuitem, position);
	gtk_widget_show (menuitem);
	return (submenu);
}

void
panel_menu_common_modify_menu_item (GtkWidget *menuitem, PanelMenuDesktopItem *item)
{
	gchar *icon = NULL;

	g_return_if_fail (menuitem != NULL);
	g_return_if_fail (GTK_IS_MENU_ITEM (menuitem));
	g_return_if_fail (item != NULL);

	/* Set the localized label of the parent menu item, and an icon if there is one */
	if (item->name)
		gtk_label_set_text (GTK_LABEL (GTK_BIN (menuitem)->child), item->name);
	if (item->comment)
		gtk_tooltips_set_tip (menu_tooltips, menuitem,
				      item->comment, item->comment);
	if (GTK_IS_IMAGE_MENU_ITEM (menuitem)) {
		icon = panel_menu_desktop_item_find_icon (item->icon);
		if (icon) {
			panel_menu_pixbuf_set_icon(GTK_MENU_ITEM (menuitem),
						   icon);
			g_free (icon);
		} else if (item->type && !strcmp (item->type, "Directory")) {
			panel_menu_pixbuf_set_icon (GTK_MENU_ITEM (menuitem),
				 		   "directory");
		} else {
			panel_menu_pixbuf_set_icon (GTK_MENU_ITEM (menuitem),
				 		   "unknown");
		}
	}
}

PanelMenuEntry *
panel_menu_common_build_entry (PanelMenu *panel_menu, const gchar *item)
{
	PanelMenuEntry *entry = NULL;
	gint id;

	if (!strcmp (item, "applications")) {
		entry = panel_menu_applications_new (panel_menu);
		panel_menu->has_applications = TRUE;
	} else if (!strncmp (item, "path", 4)) {
		if (sscanf (item, "path%d", &id))
			entry = panel_menu_path_new_with_id (panel_menu, id);
	} else if (!strncmp (item, "links", 5)) {
		if (sscanf (item, "links%d", &id))
			entry = panel_menu_links_new_with_id (panel_menu, id);
	} else if (!strncmp (item, "directory", 9)) {
		if (sscanf (item, "directory%d", &id))
			entry = panel_menu_directory_new_with_id (panel_menu, id);
	} else if (!strncmp (item, "documents", 9)) {
		if (sscanf (item, "documents%d", &id))
			entry = panel_menu_documents_new_with_id (panel_menu, id);
	} else if (!strcmp (item, "actions")) {
		entry = panel_menu_actions_new (panel_menu);
		panel_menu->has_actions = TRUE;
	} else if (!strcmp (item, "windows")) {
		entry = panel_menu_windows_new (panel_menu);
		panel_menu->has_windows = TRUE;
	} else if (!strcmp (item, "workspaces")) {
		entry = panel_menu_workspaces_new (panel_menu);
		panel_menu->has_workspaces = TRUE;
	}
	return entry;
}

PanelMenuEntry *
panel_menu_common_find_applications (PanelMenu *panel_menu)
{
	PanelMenuEntry *entry = NULL;
	GList *cur = NULL;
	gboolean found = FALSE;

	for (cur = panel_menu->entries; cur; cur = cur->next) {
		entry = (PanelMenuEntry *) cur->data;
		if (entry->type == PANEL_MENU_TYPE_APPLICATIONS) {
			found = TRUE;
			break;
		}
	}
	if (!found) entry = NULL;
	return entry;
}

PanelMenuEntry *
panel_menu_common_find_actions (PanelMenu *panel_menu)
{
	PanelMenuEntry *entry = NULL;
	GList *cur = NULL;
	gboolean found = FALSE;

	for (cur = panel_menu->entries; cur; cur = cur->next) {
		entry = (PanelMenuEntry *) cur->data;
		if (entry->type == PANEL_MENU_TYPE_ACTIONS) {
			found = TRUE;
			break;
		}
	}
	if (!found) entry = NULL;
	return entry;
}

PanelMenuEntry *
panel_menu_common_find_windows (PanelMenu *panel_menu)
{
	PanelMenuEntry *entry = NULL;
	GList *cur = NULL;
	gboolean found = FALSE;

	for (cur = panel_menu->entries; cur; cur = cur->next) {
		entry = (PanelMenuEntry *) cur->data;
		if (entry->type == PANEL_MENU_TYPE_WINDOWS) {
			found = TRUE;
			break;
		}
	}
	if (!found) entry = NULL;
	return entry;
}

PanelMenuEntry *
panel_menu_common_find_workspaces (PanelMenu *panel_menu)
{
	PanelMenuEntry *entry = NULL;
	GList *cur = NULL;
	gboolean found = FALSE;

	for (cur = panel_menu->entries; cur; cur = cur->next) {
		entry = (PanelMenuEntry *) cur->data;
		if (entry->type == PANEL_MENU_TYPE_WORKSPACES) {
			found = TRUE;
			break;
		}
	}
	if (!found) entry = NULL;
	return entry;
}

GtkWidget *
panel_menu_common_get_entry_menuitem (PanelMenuEntry *entry)
{
	GtkWidget *menuitem = NULL;

	switch (entry->type) {
	case PANEL_MENU_TYPE_APPLICATIONS:
		menuitem = panel_menu_applications_get_widget (entry);
		break;
	case PANEL_MENU_TYPE_PATH:
		menuitem = panel_menu_path_get_widget (entry);
		break;
	case PANEL_MENU_TYPE_LINKS:
		menuitem = panel_menu_links_get_widget (entry);
		break;
	case PANEL_MENU_TYPE_DIRECTORY:
		menuitem = panel_menu_directory_get_widget (entry);
		break;
	case PANEL_MENU_TYPE_DOCUMENTS:
		menuitem = panel_menu_documents_get_widget (entry);
		break;
	case PANEL_MENU_TYPE_ACTIONS:
		menuitem = panel_menu_actions_get_widget (entry);
		break;
	case PANEL_MENU_TYPE_WINDOWS:
		menuitem = panel_menu_windows_get_widget (entry);
		break;
	case PANEL_MENU_TYPE_WORKSPACES:
		menuitem = panel_menu_workspaces_get_widget (entry);
		break;
	default:
		break;
	}
	return (menuitem);
}

void
panel_menu_common_call_entry_destroy (PanelMenuEntry *entry)
{
	switch (entry->type) {
	case PANEL_MENU_TYPE_APPLICATIONS:
		panel_menu_applications_destroy (entry);
		break;
	case PANEL_MENU_TYPE_PATH:
		panel_menu_path_destroy (entry);
		break;
	case PANEL_MENU_TYPE_LINKS:
		panel_menu_links_destroy (entry);
		break;
	case PANEL_MENU_TYPE_DIRECTORY:
		panel_menu_directory_destroy (entry);
		break;
	case PANEL_MENU_TYPE_DOCUMENTS:
		panel_menu_documents_destroy (entry);
		break;
	case PANEL_MENU_TYPE_ACTIONS:
		panel_menu_actions_destroy (entry);
		break;
	case PANEL_MENU_TYPE_WINDOWS:
		panel_menu_windows_destroy (entry);
		break;
	case PANEL_MENU_TYPE_WORKSPACES:
		panel_menu_workspaces_destroy (entry);
		break;
	default:
		break;
	}
}

gchar *
panel_menu_common_call_entry_save_config (PanelMenuEntry *entry)
{
	gchar *retval = NULL;
	switch (entry->type) {
	case PANEL_MENU_TYPE_APPLICATIONS:
		retval = panel_menu_applications_save_config (entry);
		break;
	case PANEL_MENU_TYPE_PATH:
		retval = panel_menu_path_save_config (entry);
		break;
	case PANEL_MENU_TYPE_LINKS:
		retval = panel_menu_links_save_config (entry);
		break;
	case PANEL_MENU_TYPE_DIRECTORY:
		retval = panel_menu_directory_save_config (entry);
		break;
	case PANEL_MENU_TYPE_DOCUMENTS:
		retval = panel_menu_documents_save_config (entry);
		break;
	case PANEL_MENU_TYPE_ACTIONS:
		retval = panel_menu_actions_save_config (entry);
		break;
	case PANEL_MENU_TYPE_WINDOWS:
		retval = panel_menu_windows_save_config (entry);
		break;
	case PANEL_MENU_TYPE_WORKSPACES:
		retval = panel_menu_workspaces_save_config (entry);
		break;
	default:
		break;
	}
	return retval;
}

void
panel_menu_common_call_entry_remove_config (PanelMenuEntry *entry)
{
	switch (entry->type) {
	case PANEL_MENU_TYPE_PATH:
		panel_menu_path_remove_config (entry);
		break;
	case PANEL_MENU_TYPE_LINKS:
		panel_menu_links_remove_config (entry);
		break;
	case PANEL_MENU_TYPE_DIRECTORY:
		panel_menu_directory_remove_config (entry);
		break;
	case PANEL_MENU_TYPE_DOCUMENTS:
		panel_menu_documents_remove_config (entry);
		break;
	default:
		break;
	}
}

void
panel_menu_common_merge_entry_ui (PanelMenuEntry *entry)
{
	switch (entry->type) {
	case PANEL_MENU_TYPE_APPLICATIONS:
		panel_menu_applications_merge_ui (entry);
		break;
	case PANEL_MENU_TYPE_PATH:
		panel_menu_path_merge_ui (entry);
		break;
	case PANEL_MENU_TYPE_LINKS:
		panel_menu_links_merge_ui (entry);
		break;
	case PANEL_MENU_TYPE_DIRECTORY:
		panel_menu_directory_merge_ui (entry);
		break;
	case PANEL_MENU_TYPE_DOCUMENTS:
		panel_menu_documents_merge_ui (entry);
		break;
	case PANEL_MENU_TYPE_ACTIONS:
		panel_menu_actions_merge_ui (entry);
		break;
	case PANEL_MENU_TYPE_WINDOWS:
		panel_menu_windows_merge_ui (entry);
		break;
	case PANEL_MENU_TYPE_WORKSPACES:
		panel_menu_workspaces_merge_ui (entry);
		break;
	default:
		break;
	}
}

/* Called from right-click context menu */
void
panel_menu_common_remove_entry (GtkWidget *widget, PanelMenuEntry *entry, const char *verb)
{
	PanelMenu *panel_menu;

	panel_menu = entry->parent;
	g_return_if_fail (panel_menu != NULL);

	switch (entry->type) {
	case PANEL_MENU_TYPE_APPLICATIONS:
		panel_menu->has_applications = FALSE;
		break;
	case PANEL_MENU_TYPE_ACTIONS:
		panel_menu->has_actions = FALSE;
		break;
	case PANEL_MENU_TYPE_WINDOWS:
		panel_menu->has_windows = FALSE;
		break;
	case PANEL_MENU_TYPE_WORKSPACES:
		panel_menu->has_workspaces = FALSE;
		break;
	default:
		break;
	}
	panel_menu_common_call_entry_remove_config (entry);
	panel_menu->entries = g_list_remove (panel_menu->entries, entry);
	panel_menu_common_call_entry_destroy (entry);
	panel_menu_config_save_prefs (panel_menu);
	panel_menu_config_save_layout (panel_menu);
}

void
panel_menu_common_demerge_ui (PanelApplet *applet)
{
	BonoboUIComponent  *component;

	component = panel_applet_get_popup_component (applet);
	if (bonobo_ui_component_path_exists (component,
		"/popups/button3/ChildMerge/ChildItem", NULL)) {
		bonobo_ui_component_remove_verb (component, "Action");
		bonobo_ui_component_remove_verb (component, "Regenerate");
		bonobo_ui_component_remove_verb (component, "Remove");
		bonobo_ui_component_rm (component, "/popups/button3/ChildMerge/ChildItem", NULL);
	}
}

GtkWidget *
panel_menu_common_single_entry_dialog_new (gchar *title, gchar *label,
					   gchar *value, GtkWidget ** entry)
{
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *hbox;
	GtkWidget *l;

	dialog = gtk_dialog_new_with_buttons (title,
					      NULL, GTK_DIALOG_MODAL,
					      GTK_STOCK_CLOSE,
					      GTK_RESPONSE_CLOSE, GTK_STOCK_OK,
					      GTK_RESPONSE_OK, NULL);

	box = GTK_DIALOG (dialog)->vbox;

	hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 5);
	gtk_widget_show (hbox);

	l = gtk_label_new (label);
	gtk_box_pack_start (GTK_BOX (hbox), l, FALSE, FALSE, 5);
	gtk_widget_show (l);

	*entry = gtk_entry_new_with_max_length (50);
	gtk_box_pack_start (GTK_BOX (hbox), *entry, TRUE, TRUE, 5);
	gtk_widget_show (*entry);
	gtk_entry_set_text (GTK_ENTRY (*entry), value);
	return (dialog);
}
