/* panel-menu-directory.c
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
#include <libgnomevfs/gnome-vfs-monitor.h>
#include <libgnomevfs/gnome-vfs-ops.h>

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-config.h"
#include "panel-menu-pixbuf.h"
#include "panel-menu-directory.h"

static const gchar *directory_menu_xml =
	"    <placeholder name=\"ChildItem\">\n"
	"        <menuitem name=\"Action\" verb=\"Action\" label=\"%s Menu Preferences...\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
	"%s"
	"        <menuitem name=\"Remove\" verb=\"Remove\" label=\"Remove %s Menu\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-close\"/>\n"
	"        <separator/>"
	"    </placeholder>";

static const gchar *additional_menu_xml =
	"        <menuitem name=\"Regenerate\" verb=\"Regenerate\" label=\"Regenerate Menus\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-refresh\"/>\n";

typedef struct _PanelMenuDocuments {
	gint id;
	GtkWidget *directory;
	GtkWidget *menu;
	gchar *name;
	gchar *path;
	gint level;
	GnomeVFSMonitorHandle *monitor;
} PanelMenuDirectory;

static void directory_changed_cb(GnomeVFSMonitorHandle *handle,
				 const gchar *monitor_uri,
				 const gchar *info_uri,
				 GnomeVFSMonitorEventType event_type,
				 gpointer user_data);
static void regenerate_menus_cb (GtkWidget *menuitem, PanelMenuEntry *entry,
				 const gchar *verb);
static void panel_menu_directory_load (const gchar *uri, GtkMenuShell *parent,
				       gint level);
static void directory_load_cb (GnomeVFSAsyncHandle *handle,
			       GnomeVFSResult result, GList *list,
			       guint entries_read, GtkMenuShell *parent);
static void change_directory_cb (GtkWidget *widget, PanelMenuEntry *entry,
				 const gchar *verb);
static GtkWidget *panel_menu_directory_edit_dialog_new (gchar *title,
							GtkWidget **nentry,
							GtkWidget **pentry,
							GtkWidget **spin);
static gboolean panel_menu_directory_warn_about_path (char *path);

static gint object_counter = 0;

PanelMenuEntry *
panel_menu_directory_new (PanelMenu *parent, gchar *name, gchar *path,
			  gint level)
{
	PanelMenuEntry *entry;
	gchar *item_key;

	item_key = g_strdup_printf ("directory%d/name", object_counter);
	panel_applet_gconf_set_string (parent->applet, item_key, name, NULL);
	g_free (item_key);
	item_key = g_strdup_printf ("directory%d/path", object_counter);
	panel_applet_gconf_set_string (parent->applet, item_key, path, NULL);
	g_free (item_key);
	item_key = g_strdup_printf ("directory%d/level", object_counter);
	panel_applet_gconf_set_int (parent->applet, item_key, level, NULL);
	g_free (item_key);
	entry = panel_menu_directory_new_with_id (parent, object_counter);
	return entry;
}

PanelMenuEntry *
panel_menu_directory_new_with_id (PanelMenu *parent, gint id)
{
	PanelMenuEntry *entry;
	PanelMenuDirectory *directory;
	GtkWidget *tearoff;
	GtkWidget *image;
	GConfClient *client;
	gchar *base_key;
	gchar *dir_key;
	gchar *name;
	gchar *path;
	gint level;

	if (id > object_counter)
		object_counter = id;

	entry = g_new0 (PanelMenuEntry, 1);
	entry->type = PANEL_MENU_TYPE_DIRECTORY;
	entry->parent = parent;
	directory = g_new0 (PanelMenuDirectory, 1);
	entry->data = (gpointer) directory;
	directory->id = id;
	directory->directory = gtk_menu_item_new_with_label ("");
	panel_menu_common_widget_dnd_init (entry);
	gtk_widget_show (directory->directory);
	directory->menu = gtk_menu_new ();

	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (directory->menu), tearoff);
	if (parent->menu_tearoffs) {
		gtk_widget_show (tearoff);
	}

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (directory->directory),
				   directory->menu);

	client = gconf_client_get_default ();
	base_key = g_strdup_printf ("directory%d", id);
	dir_key = panel_applet_gconf_get_full_key (parent->applet, base_key);
	if (gconf_client_dir_exists (client, dir_key, NULL)) {
		gchar *key;
		key = g_strdup_printf ("%s/name", base_key);
		name = panel_applet_gconf_get_string (parent->applet, key, NULL);
		g_free (key);
		key = g_strdup_printf ("%s/path", base_key);
		path = panel_applet_gconf_get_string (parent->applet, key, NULL);
		g_free (key);
		key = g_strdup_printf ("%s/level", base_key);
		level = panel_applet_gconf_get_int (parent->applet, key, NULL);
		g_free (key);
	} else {
		name = g_strdup ("My Documents");
		path = g_strdup_printf ("%s/Documents", g_get_home_dir ());
		level = 1;
	}
	g_object_unref (G_OBJECT (client));
	g_free (base_key);
	g_free (dir_key);
	directory->level = level;
	panel_menu_directory_set_name (entry, name);
	panel_menu_directory_set_path (entry, path);
	g_free (name);
	g_free (path);
	object_counter++;
	return entry;
}

void
panel_menu_directory_set_name (PanelMenuEntry *entry, gchar *name)
{
	PanelMenuDirectory *directory;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DIRECTORY);

	directory = (PanelMenuDirectory *) entry->data;
	if (directory->name)
		g_free (directory->name);
	directory->name = name ? g_strdup (name) : g_strdup ("");
	gtk_label_set_text (GTK_LABEL (GTK_BIN (directory->directory)->child),
			    name);
}

void
panel_menu_directory_set_path (PanelMenuEntry *entry, gchar *path)
{
	PanelMenuDirectory *directory;
	gchar *old_path = NULL;
	gchar *full_path = NULL;
	gchar *label;
	gchar *show_label;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DIRECTORY);

	directory = (PanelMenuDirectory *) entry->data;
	/* Save it here, we don't want to free till the end BUG #77003 */
	old_path = directory->path;
	if (directory->monitor) {
		gnome_vfs_monitor_cancel (directory->monitor);
		directory->monitor = NULL;
	}
	directory->path = panel_menu_common_build_full_path (path, "");;
	regenerate_menus_cb (NULL, entry, NULL);
	if (gnome_vfs_monitor_add (&directory->monitor,
			       directory->path,
			       GNOME_VFS_MONITOR_DIRECTORY,
			       directory_changed_cb,
			       entry) == GNOME_VFS_OK)
		g_print ("monitor successfully installed for %s\n",
			  directory->path);
	else {
		directory->monitor = NULL;
		g_print ("monitor installation failed for directory %s\n",
			  directory->path);
	}
	if (old_path) {
		g_free (directory->path);
		directory->path = NULL;
	}
}

static void
directory_changed_cb(GnomeVFSMonitorHandle *handle,
		     const gchar *monitor_uri,
		     const gchar *info_uri,
		     GnomeVFSMonitorEventType event_type,
		     gpointer user_data)
{
	PanelMenuEntry *entry;
	entry = (PanelMenuEntry *) user_data;

	regenerate_menus_cb (NULL, entry, NULL);
}

static void
regenerate_menus_cb (GtkWidget *menuitem, PanelMenuEntry *entry,
		     const gchar *verb)
{
	PanelMenuDirectory *directory;
	GList *list;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DIRECTORY);

	directory = (PanelMenuDirectory *) entry->data;
	list = g_list_copy (GTK_MENU_SHELL (directory->menu)->children);
	for (cur = list; cur; cur = cur->next) {
		if (g_object_get_data (G_OBJECT (cur->data), "uri-path"))
			gtk_widget_destroy (GTK_WIDGET (cur->data));
	}
	g_list_free (list);
	panel_menu_directory_load (directory->path, GTK_MENU_SHELL(directory->menu),
				   directory->level);
}

void
panel_menu_directory_merge_ui (PanelMenuEntry *entry)
{
	PanelMenuDirectory *directory;
	BonoboUIComponent  *component;
	gchar *xml;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DIRECTORY);

	directory = (PanelMenuDirectory *) entry->data;
	component = panel_applet_get_popup_component (entry->parent->applet);

	bonobo_ui_component_add_verb (component, "Action",
				     (BonoboUIVerbFn)change_directory_cb, entry);
	bonobo_ui_component_add_verb (component, "Regenerate",
				     (BonoboUIVerbFn)regenerate_menus_cb, entry);
	bonobo_ui_component_add_verb (component, "Remove",
				     (BonoboUIVerbFn)panel_menu_common_remove_entry, entry);
	xml = g_strdup_printf (directory_menu_xml,
			       directory->name,
			       directory->monitor ?
			       "" : additional_menu_xml,
			       directory->name);
	bonobo_ui_component_set (component, "/popups/button3/ChildMerge/",
				 xml, NULL);
	g_free (xml);
}

void
panel_menu_directory_destroy (PanelMenuEntry *entry)
{
	PanelMenuDirectory *directory;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DIRECTORY);

	directory = (PanelMenuDirectory *) entry->data;
	if (directory->monitor)
		gnome_vfs_monitor_cancel (directory->monitor);
	if (directory->name)
		g_free (directory->name);
	if (directory->path)
		g_free (directory->path);
	gtk_widget_destroy (directory->directory);
	g_free (directory);
	g_print ("directory item completely removed\n");
}

GtkWidget *
panel_menu_directory_get_widget (PanelMenuEntry *entry)
{
	PanelMenuDirectory *directory;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_DIRECTORY, NULL);

	directory = (PanelMenuDirectory *) entry->data;
	return directory->directory;
}

gboolean
panel_menu_directory_accept_drop (PanelMenuEntry *entry, GnomeVFSURI *uri)
{
	gchar *fileuri;
	gchar *icon;
	GnomeVFSFileInfo finfo;
	GtkWidget *menuitem;
	GtkWidget *image;
	gboolean retval = FALSE;

	g_return_val_if_fail (entry != NULL, FALSE);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_DIRECTORY, FALSE);
	/* FIXME: possibly set a new directory */
	/* or merge multiple directories */
	return retval;
}

static void
panel_menu_directory_load (const gchar *uri, GtkMenuShell *parent, gint level)
{
	GnomeVFSResult result;
	GnomeVFSAsyncHandle *load_handle;
	gchar *path;

	if (uri && parent) {
		path = panel_menu_common_build_full_path (uri, "");
		g_object_set_data (G_OBJECT (parent), "uri-path", path);
		g_object_set_data (G_OBJECT (parent), "level",
				   GINT_TO_POINTER (level));
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
	GtkWidget *image = NULL;
	gchar *icon = NULL;
	gchar *path = NULL;
	gchar *subpath = NULL;
	gint level = 0;
	gint count = 0;

	path = (gchar *) g_object_get_data (G_OBJECT (parent), "uri-path");
	level = (gint) g_object_get_data (G_OBJECT (parent), "level") - 1;

	for (iter = list, count = 0; iter && count < entries_read;
	     iter = iter->next, count++) {
		finfo = (GnomeVFSFileInfo *) iter->data;
		subpath = panel_menu_common_build_full_path (path, finfo->name);
		if (!subpath)
			continue;
		if (finfo->type == GNOME_VFS_FILE_TYPE_DIRECTORY) {
			if (level > 0) {
				menuitem =
					gtk_image_menu_item_new_with_label
					(finfo->name);
				g_object_set_data (G_OBJECT (menuitem),
						   "uri-path",
						   g_strdup (subpath));
				g_signal_connect (G_OBJECT (menuitem),
						  "destroy",
						  G_CALLBACK
						  (panel_menu_common_destroy_apps_menuitem),
						  NULL);
				panel_menu_pixbuf_set_icon (GTK_MENU_ITEM (menuitem),
							   "directory");
				gtk_menu_shell_append (parent, menuitem);
				gtk_widget_show (menuitem);
				submenu = gtk_menu_new ();
				GTK_MENU (submenu)->parent_menu_item = menuitem;
				gtk_menu_item_set_submenu (GTK_MENU_ITEM
							   (menuitem), submenu);
				panel_menu_directory_load ((const gchar *)
							   subpath,
							   GTK_MENU_SHELL
							   (submenu), level);
			}
		} else {
			menuitem =
				gtk_image_menu_item_new_with_label (finfo->
								    name);
			panel_menu_common_apps_menuitem_dnd_init (menuitem);
			icon = (gchar *) gnome_vfs_mime_get_value ((gchar *)
								   gnome_vfs_mime_type_from_name
								   (finfo->
								    name),
								   "icon-filename");
			if (icon) {
				panel_menu_pixbuf_set_icon (GTK_MENU_ITEM (menuitem), icon);
			} else {
				image = gtk_image_new_from_stock (GTK_STOCK_NEW,
								  GTK_ICON_SIZE_MENU);
				gtk_image_menu_item_set_image
					(GTK_IMAGE_MENU_ITEM (menuitem), image);
				gtk_widget_show (image);
			}
			gtk_menu_shell_append (parent, menuitem);
			g_object_set_data (G_OBJECT (menuitem), "uri-path",
					   g_strdup (subpath));
			g_signal_connect (G_OBJECT (menuitem), "destroy",
					  G_CALLBACK
					  (panel_menu_common_destroy_apps_menuitem),
					  NULL);
			gtk_widget_show (menuitem);
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

static void
handle_create_dir_response (GtkDialog *dialog, gint response, PanelMenu *panel_menu)
{
	if (response == GTK_RESPONSE_OK) {

		GtkWidget *name_entry;
		GtkWidget *path_entry;
		GtkWidget *level_spin;
		PanelMenuEntry *entry;
		gchar *name;
		gchar *path;
		gint level;

		name_entry = GTK_WIDGET(g_object_get_data (G_OBJECT (dialog), "uri-name"));
		name = (gchar *) gtk_entry_get_text (GTK_ENTRY (name_entry));

		path_entry = GTK_WIDGET(g_object_get_data (G_OBJECT (dialog), "uri-path"));
		path = (gchar *) gtk_entry_get_text (GTK_ENTRY (path_entry));
		
		if (!panel_menu_directory_warn_about_path(path))
			return;

		level_spin = GTK_WIDGET(g_object_get_data (G_OBJECT (dialog), "depth-level"));
		level = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
							  (level_spin));
		entry = panel_menu_directory_new (panel_menu, name, path,
						  level);
		panel_menu->entries =
			g_list_append (panel_menu->entries, (gpointer) entry);
		gtk_menu_shell_append (GTK_MENU_SHELL (panel_menu->menubar),
				       panel_menu_directory_get_widget (entry));
		panel_menu_config_save_layout (panel_menu);
		panel_menu_directory_save_config (entry);
	}

	gtk_widget_destroy (GTK_WIDGET(dialog));
}

void
panel_menu_directory_new_with_dialog (PanelMenu *panel_menu)
{
	GtkWidget *dialog;
	GtkWidget *name_entry;
	GtkWidget *path_entry;
	GtkWidget *level_spin;

	dialog = panel_menu_directory_edit_dialog_new (_
						       ("Create directory item..."),
						       &name_entry, &path_entry,
						       &level_spin);

        g_signal_connect (G_OBJECT (dialog), "response",
                          G_CALLBACK (handle_create_dir_response),
                          panel_menu);

	gtk_entry_set_text (GTK_ENTRY (name_entry), _("Home"));
	gtk_entry_set_text (GTK_ENTRY (path_entry), g_get_home_dir ());
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (level_spin), 1.0);
	gtk_widget_show (dialog);
	gtk_widget_grab_focus (path_entry);
}

static void
handle_edit_dir_response (GtkDialog *dialog, gint response, PanelMenuEntry *entry)
{
	if (response == GTK_RESPONSE_OK) {

		PanelMenuDirectory *directory;
		GtkWidget *name_entry;
		GtkWidget *path_entry;
		GtkWidget *level_spin;

		const gchar *new_name;
		const gchar *new_path;
		gint new_level;

		gboolean changed = FALSE;

        	directory = (PanelMenuDirectory *) entry->data;

		name_entry = GTK_WIDGET(g_object_get_data (G_OBJECT (dialog), "uri-name"));
		new_name = gtk_entry_get_text (GTK_ENTRY (name_entry));

		path_entry = GTK_WIDGET(g_object_get_data (G_OBJECT (dialog), "uri-path"));
		new_path = gtk_entry_get_text (GTK_ENTRY (path_entry));

		level_spin = GTK_WIDGET(g_object_get_data (G_OBJECT (dialog), "depth-level"));
		new_level =
			gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON
							  (level_spin));
		if (strcmp (directory->name, new_name)) {
			panel_menu_directory_set_name (entry, (gchar *)new_name);
			changed = TRUE;
		}
		if (strcmp (directory->path, new_path)) {
			directory->level = new_level;
			panel_menu_directory_warn_about_path((gchar *)new_path);
			panel_menu_directory_set_path (entry, (gchar *)new_path);
			changed = TRUE;
		} else if (directory->level != new_level) {
			directory->level = new_level;
			panel_menu_directory_set_path (entry, directory->path);
			changed = TRUE;
		}
		if (changed) {
			panel_menu_directory_save_config (entry);
		}
	}

	gtk_widget_destroy (GTK_WIDGET(dialog));
}

static void
change_directory_cb (GtkWidget *widget, PanelMenuEntry *entry, const gchar *verb)
{
	PanelMenuDirectory *directory;
	GtkWidget *dialog;
	GtkWidget *name_entry;
	GtkWidget *path_entry;
	GtkWidget *level_spin;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DIRECTORY);

	directory = (PanelMenuDirectory *) entry->data;
	dialog = panel_menu_directory_edit_dialog_new (_
						       ("Edit directory item..."),
						       &name_entry, &path_entry,
						       &level_spin);
        g_signal_connect (G_OBJECT (dialog), "response",
                          G_CALLBACK (handle_edit_dir_response),
                          entry);

	gtk_entry_set_text (GTK_ENTRY (name_entry), directory->name);
	gtk_entry_set_text (GTK_ENTRY (path_entry), directory->path);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (level_spin),
				   (gfloat) directory->level);

	gtk_widget_show (dialog);
	gtk_widget_grab_focus (name_entry);
}


static void
handle_browse_response (GtkDialog *dialog, gint response, GtkFileSelection *fsel)
{
	GtkEntry *entry;
	const gchar *new_path;

	if (response == GTK_RESPONSE_OK) {
		new_path = gtk_file_selection_get_filename (fsel);
		if (new_path) {
			entry = GTK_ENTRY(g_object_get_data (G_OBJECT (dialog), "target"));
			gtk_entry_set_text (entry, new_path);
		}
	}
	gtk_widget_destroy (GTK_WIDGET(dialog));
}

static void
browse_callback (GtkButton *button, GtkEntry *entry)
{
	GtkWidget *fsel;
	const gchar *old_path;

	fsel = gtk_file_selection_new ("Choose a directory...");
	gtk_window_set_transient_for (GTK_WINDOW (fsel), GTK_WINDOW (
				      gtk_widget_get_toplevel (GTK_WIDGET (entry))));
	gtk_window_set_modal (GTK_WINDOW (fsel), TRUE);
	old_path = gtk_entry_get_text (entry);
	gtk_file_selection_set_filename (GTK_FILE_SELECTION (fsel), old_path);
	gtk_file_selection_set_select_multiple (GTK_FILE_SELECTION (fsel), FALSE);
	g_object_set_data (G_OBJECT (fsel), "target", entry);
	g_signal_connect (G_OBJECT (fsel), "response",
			  G_CALLBACK (handle_browse_response), fsel);
	gtk_widget_show (fsel);
}

static GtkWidget *
panel_menu_directory_edit_dialog_new (gchar *title, GtkWidget **nentry,
				      GtkWidget **pentry, GtkWidget **spin)
{
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *browse;

	dialog = gtk_dialog_new_with_buttons (title,
					      NULL, 0,
					      GTK_STOCK_CLOSE,
					      GTK_RESPONSE_CLOSE, GTK_STOCK_OK,
					      GTK_RESPONSE_OK, NULL);

	box = GTK_DIALOG (dialog)->vbox;
	gtk_container_set_border_width (GTK_CONTAINER (box), 5);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 5);
	gtk_widget_show (hbox);
	label = gtk_label_new_with_mnemonic (_("_Name:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);
	gtk_widget_show (label);
	*nentry = gtk_entry_new_with_max_length (50);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), *nentry);
	set_relation(*nentry, GTK_LABEL(label));
	gtk_box_pack_start (GTK_BOX (hbox), *nentry, TRUE, TRUE, 5);
	gtk_widget_show (*nentry);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 5);
	gtk_widget_show (hbox);
	label = gtk_label_new_with_mnemonic (_("_Path:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);
	gtk_widget_show (label);
	*pentry = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox), *pentry, TRUE, TRUE, 5);
	gtk_widget_show (*pentry);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), *pentry);
	set_relation(*pentry, GTK_LABEL(label));
	browse = gtk_button_new_with_label (_("..."));
	add_tooltip(browse, "Browse to select a directory");
	gtk_box_pack_start (GTK_BOX (hbox), browse, FALSE, FALSE, 5);
	g_signal_connect (G_OBJECT (browse), "clicked",
			  G_CALLBACK(browse_callback), *pentry);
	gtk_widget_show (browse);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box), hbox, FALSE, FALSE, 5);
	gtk_widget_show (hbox);
	label = gtk_label_new_with_mnemonic (_("_Depth level:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);
	gtk_widget_show (label);
	*spin = gtk_spin_button_new (GTK_ADJUSTMENT
				     (gtk_adjustment_new (1, 1.0, 10, 1, 1, 1)),
				     1, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), *spin);
	set_relation(*spin, GTK_LABEL(label));
	gtk_box_pack_end (GTK_BOX (hbox), *spin, TRUE, TRUE, 5);
	gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (*spin),
					   GTK_UPDATE_ALWAYS);
	gtk_widget_show (*spin);

        g_object_set_data (G_OBJECT (dialog), "uri-name", *nentry);
        g_object_set_data (G_OBJECT (dialog), "uri-path", *pentry);
        g_object_set_data (G_OBJECT (dialog), "depth-level", *spin);

	return dialog;
}

gchar *
panel_menu_directory_save_config (PanelMenuEntry *entry)
{
	PanelMenuDirectory *directory;
	PanelMenu *panel_menu;
	PanelApplet *applet;
	gchar *id;
	gchar *key;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_DIRECTORY, NULL);

	panel_menu = entry->parent;
	applet = panel_menu->applet;
	directory = (PanelMenuDirectory *) entry->data;

	id = g_strdup_printf ("directory%d", directory->id);
	g_print ("Saving config for %s as %s - %s - %d.\n", id, directory->name, directory->path, directory->level);
	key = g_strdup_printf ("%s/name", id);
	panel_applet_gconf_set_string (applet, key, directory->name, NULL);
	g_free (key);
	key = g_strdup_printf ("%s/path", id);
	panel_applet_gconf_set_string (applet, key, directory->path, NULL);
	g_free (key);
	key = g_strdup_printf ("%s/level", id);
	panel_applet_gconf_set_int (applet, key, directory->level, NULL);
	g_free (key);
	return id;
}

void
panel_menu_directory_remove_config (PanelMenuEntry *entry)
{
	PanelMenuDirectory *directory;
	PanelMenu *panel_menu;
	PanelApplet *applet;
	GConfClient *client;
	gchar *base_key;
	gchar *key;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DIRECTORY);

	panel_menu = entry->parent;
	applet = panel_menu->applet;
	directory = (PanelMenuDirectory *) entry->data;

	client = gconf_client_get_default ();
	g_return_if_fail (client != NULL);
	base_key = panel_applet_get_preferences_key (applet);
	key = g_strdup_printf ("%s/directory%d", base_key, directory->id);
	_gconf_client_clean_dir (client, key);
	g_free (base_key);
	g_free (key);
	g_object_unref (G_OBJECT (client));
}

static void
message_dlg_response (GtkDialog *dialog, gint response, gpointer data)
{
	gtk_widget_destroy (GTK_WIDGET(dialog));
}

static gboolean
panel_menu_directory_warn_about_path (char *path)
{
	struct stat sbuf;
	GtkWidget *message_dlg;

	stat(path, &sbuf);

	if (S_ISDIR (sbuf.st_mode) == FALSE ){

		message_dlg = gtk_message_dialog_new (NULL,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING,
				GTK_BUTTONS_OK,
				_("Path specified is currently not a valid directory."));
		gtk_dialog_set_default_response (GTK_DIALOG (message_dlg), GTK_RESPONSE_OK);
		gtk_window_set_resizable (GTK_WINDOW (message_dlg), FALSE);

        	g_signal_connect (G_OBJECT (message_dlg), "response",
                          G_CALLBACK (message_dlg_response),
                          NULL);
		gtk_widget_show_all (message_dlg);
		
		return FALSE;
	}
	
	return TRUE;
}
