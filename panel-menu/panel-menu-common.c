/*  panel-menu-common.c
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
#include "panel-menu-options.h"
#include "panel-menu-path.h"
#include "panel-menu-links.h"
#include "panel-menu-directory.h"
#include "panel-menu-documents.h"
#include "panel-menu-actions.h"
#include "panel-menu-windows.h"
#include "panel-menu-workspaces.h"
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

static void widget_dnd_drag_begin_cb (GtkWidget * widget,
				      GdkDragContext * context);
static void widget_dnd_set_data_cb (GtkWidget * widget,
				    GdkDragContext * context,
				    GtkSelectionData * selection_data,
				    guint info, guint time,
				    PanelMenuEntry * entry);

static void apps_menuitem_dnd_drag_begin_cb (GtkWidget * widget,
					     GdkDragContext * context,
					     gpointer data);
static void apps_menuitem_dnd_set_data_cb (GtkWidget * widget,
					   GdkDragContext * context,
					   GtkSelectionData * selection_data,
					   guint info, guint time,
					   gpointer data);

static gboolean check_dir_exists (gchar * path);

void
panel_menu_common_initialize ()
{
	gchar *path = NULL;

	if (!gnome_vfs_initialized ())
		gnome_vfs_init ();
	path = g_strdup_printf ("%s/.gnome2/panel-menu", g_get_home_dir ());
	if (!check_dir_exists (path)) {
		gnome_vfs_make_directory (path,
					  GNOME_VFS_PERM_USER_ALL |
					  GNOME_VFS_PERM_GROUP_ALL |
					  GNOME_VFS_PERM_OTHER_READ |
					  GNOME_VFS_PERM_OTHER_EXEC);
	}
	g_free (path);
}

void
panel_menu_common_set_changed (PanelMenu * panel_menu)
{
	panel_menu->changed = TRUE;
	if (panel_menu->auto_save_config) {
		panel_menu_config_save_xml (panel_menu);
	}
}

void
panel_menu_common_widget_dnd_init (PanelMenuEntry * entry)
{
	GtkWidget *menuitem;

	menuitem = panel_menu_common_get_entry_menuitem (entry);
	gtk_drag_source_set (menuitem, GDK_BUTTON2_MASK, widget_drag_types,
			     n_widget_drag_types,
			     GDK_ACTION_COPY | GDK_ACTION_LINK |
			     GDK_ACTION_ASK);
	g_signal_connect (G_OBJECT (menuitem), "drag_begin",
			  G_CALLBACK (widget_dnd_drag_begin_cb), NULL);
	g_signal_connect (G_OBJECT (menuitem), "drag_data_get",
			  G_CALLBACK (widget_dnd_set_data_cb), entry);
}

static void
widget_dnd_drag_begin_cb (GtkWidget * widget, GdkDragContext * context)
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
widget_dnd_set_data_cb (GtkWidget * widget, GdkDragContext * context,
			GtkSelectionData * selection_data, guint info,
			guint time, PanelMenuEntry * entry)
{
	PanelMenu *panel_menu;
	GtkWidget *menuitem;
	GtkWidget *checkitem;
	gint position;

	if (info == TARGET_PANEL_MENU_ENTRY) {
		panel_menu = entry->parent;
		position = g_list_index (panel_menu->entries, entry);
		menuitem = panel_menu_common_get_entry_menuitem (entry);
		g_object_ref (G_OBJECT (menuitem));
		gtk_container_remove (GTK_CONTAINER (menuitem->parent),
				      menuitem);
		if (entry->type != PANEL_MENU_TYPE_OPTIONS) {
			checkitem =
				panel_menu_common_get_entry_checkitem (entry);
			g_object_ref (G_OBJECT (checkitem));
			gtk_container_remove (GTK_CONTAINER (checkitem->parent),
					      checkitem);
		} else {
			g_object_set_data (G_OBJECT (menuitem), "old-position",
					   GINT_TO_POINTER (position));
		}
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
panel_menu_common_apps_menuitem_dnd_init (GtkWidget * menuitem)
{
	gtk_drag_source_set (menuitem, GDK_BUTTON1_MASK, drag_types,
			     n_drag_types,
			     GDK_ACTION_COPY | GDK_ACTION_LINK |
			     GDK_ACTION_ASK);
	g_signal_connect (G_OBJECT (menuitem), "drag_begin",
			  G_CALLBACK (apps_menuitem_dnd_drag_begin_cb), NULL);
	g_signal_connect (G_OBJECT (menuitem), "drag_data_get",
			  G_CALLBACK (apps_menuitem_dnd_set_data_cb), NULL);
}

static void
apps_menuitem_dnd_drag_begin_cb (GtkWidget * widget, GdkDragContext * context,
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
apps_menuitem_dnd_set_data_cb (GtkWidget * widget, GdkDragContext * context,
			       GtkSelectionData * selection_data, guint info,
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
panel_menu_common_activate_apps_menuitem (GtkWidget * menuitem, gpointer data)
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
panel_menu_common_destroy_apps_menuitem (GtkWidget * menuitem, gpointer data)
{
	data = g_object_get_data (G_OBJECT (menuitem), "uri-path");
	if (data)
		g_free (data);
	data = g_object_get_data (G_OBJECT (menuitem), "exec-string");
	if (data)
		g_free (data);
}

void
panel_menu_common_set_icon_scaled_from_file (GtkMenuItem * menuitem,
					     gchar * file)
{
	GdkPixbuf *pixbuf;
	GtkWidget *image;
	double pix_x, pix_y;
	double greatest;

	if (!GTK_IS_IMAGE_MENU_ITEM (menuitem))
		return;

	pixbuf = gdk_pixbuf_new_from_file (file, NULL);
	if (pixbuf) {
		pix_x = gdk_pixbuf_get_width (pixbuf);
		pix_y = gdk_pixbuf_get_height (pixbuf);
		if (pix_x > ICON_SIZE || pix_y > ICON_SIZE) {
			greatest = pix_x > pix_y ? pix_x : pix_y;
			pixbuf = gdk_pixbuf_scale_simple (pixbuf,
							  (ICON_SIZE /
							   greatest) * pix_x,
							  (ICON_SIZE /
							   greatest) * pix_y,
							  GDK_INTERP_BILINEAR);
		}
		image = gtk_image_new_from_pixbuf (pixbuf);
		gtk_widget_show (image);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem),
					       image);
	}
}

void
panel_menu_common_set_visibility (GtkCheckMenuItem * checkitem,
				  GtkWidget * target)
{
	if (gtk_check_menu_item_get_active (checkitem))
		gtk_widget_show (target);
	else
		gtk_widget_hide (target);
}

gchar *
panel_menu_common_build_full_path (const gchar * path, const gchar * selection)
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

GtkWidget *
panel_menu_common_menuitem_from_path (gchar * uri, GtkMenuShell * parent,
				      gboolean append)
{
	PanelMenuDesktopItem *item;
	GtkWidget *menuitem = NULL;
	gchar *icon = NULL;
	GList *cur = NULL;
	gint position = 0;

	g_return_val_if_fail (uri != NULL, NULL);
	g_return_val_if_fail (parent != NULL, NULL);

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
		if (icon) {
			panel_menu_common_set_icon_scaled_from_file
				(GTK_MENU_ITEM (menuitem), icon);
			g_free (icon);
		}
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
panel_menu_common_menu_from_path (gchar * name, gchar * subpath,
				  GtkMenuShell * parent, gboolean append)
{
	GtkWidget *menuitem;
	GtkWidget *submenu;
	GtkWidget *tearoff;
	GList *cur = NULL;
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
	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (submenu), tearoff);
	gtk_widget_show (tearoff);
	GTK_MENU (submenu)->parent_menu_item = menuitem;
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), submenu);
	if (append)
		gtk_menu_shell_append (parent, menuitem);
	else
		gtk_menu_shell_insert (parent, menuitem, position);
	gtk_widget_show (menuitem);
	return (submenu);
}

PanelMenuEntry *
panel_menu_common_find_options (PanelMenu * panel_menu)
{
	PanelMenuEntry *entry = NULL;
	GList *cur = NULL;

	for (cur = panel_menu->entries; cur; cur = cur->next) {
		entry = (PanelMenuEntry *) cur->data;
		if (entry->type == PANEL_MENU_TYPE_OPTIONS)
			break;
	}
	return (entry);
}

GtkWidget *
panel_menu_common_get_entry_menuitem (PanelMenuEntry * entry)
{
	GtkWidget *menuitem = NULL;

	switch (entry->type) {
	case PANEL_MENU_TYPE_OPTIONS:
		menuitem = panel_menu_options_get_widget (entry);
		break;
	case PANEL_MENU_TYPE_MENU_PATH:
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

GtkWidget *
panel_menu_common_get_entry_checkitem (PanelMenuEntry * entry)
{
	GtkWidget *checkitem = NULL;

	switch (entry->type) {
	case PANEL_MENU_TYPE_MENU_PATH:
		checkitem = panel_menu_path_get_checkitem (entry);
		break;
	case PANEL_MENU_TYPE_LINKS:
		checkitem = panel_menu_links_get_checkitem (entry);
		break;
	case PANEL_MENU_TYPE_DIRECTORY:
		checkitem = panel_menu_directory_get_checkitem (entry);
		break;
	case PANEL_MENU_TYPE_DOCUMENTS:
		checkitem = panel_menu_documents_get_checkitem (entry);
		break;
	case PANEL_MENU_TYPE_ACTIONS:
		checkitem = panel_menu_actions_get_checkitem (entry);
		break;
	case PANEL_MENU_TYPE_WINDOWS:
		checkitem = panel_menu_windows_get_checkitem (entry);
		break;
	case PANEL_MENU_TYPE_WORKSPACES:
		checkitem = panel_menu_workspaces_get_checkitem (entry);
		break;
	default:
		break;
	}
	return (checkitem);
}

void
panel_menu_common_call_entry_destroy (PanelMenuEntry * entry)
{
	switch (entry->type) {
	case PANEL_MENU_TYPE_OPTIONS:
		panel_menu_options_destroy (entry);
		break;
	case PANEL_MENU_TYPE_MENU_PATH:
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

static gboolean
check_dir_exists (gchar * path)
{
	GnomeVFSResult result;
	GnomeVFSFileInfo *finfo;
	gboolean exists = FALSE;

	if (!path)
		return (FALSE);
	finfo = gnome_vfs_file_info_new ();
	result = gnome_vfs_get_file_info (path, finfo,
					  GNOME_VFS_FILE_INFO_DEFAULT);
	if (result == GNOME_VFS_OK) {
		gnome_vfs_file_info_ref (finfo);
		if (finfo->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
			exists = TRUE;
		}
		gnome_vfs_file_info_unref (finfo);
	}
	return (exists);
}

void
panel_menu_common_merge_entry_ui (PanelMenuEntry * entry)
{
	switch (entry->type) {
	case PANEL_MENU_TYPE_MENU_PATH:
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

void
panel_menu_common_remove_entry (GtkWidget *widget, PanelMenuEntry *entry, const char *verb)
{
	PanelMenu *panel_menu;

	panel_menu = entry->parent;
	g_return_if_fail (panel_menu != NULL);

	panel_menu->entries = g_list_remove (panel_menu->entries, entry);
	panel_menu_common_call_entry_destroy (entry);
}

void
panel_menu_common_demerge_ui (PanelApplet *applet)
{
	BonoboUIComponent  *component;

	component = panel_applet_get_popup_component (applet);
	if (bonobo_ui_component_path_exists (component,
		"/popups/button3/ChildMerge/ChildItem", NULL)) {
		bonobo_ui_component_remove_verb (component, "Remove");
		bonobo_ui_component_remove_verb (component, "Action");
		bonobo_ui_component_rm (component, "/popups/button3/ChildMerge/ChildItem", NULL);
	} else {
		g_print ("Unable to remove verbs and xml, they do not exist.\n");
	}
}

GtkWidget *
panel_menu_common_single_entry_dialog_new (gchar * title, gchar * label,
					   gchar * value, GtkWidget ** entry)
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
