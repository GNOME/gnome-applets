/* panel-menu-documents.c
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
#include "panel-menu-common.h"
#include "panel-menu-config.h"
#include "panel-menu-documents.h"

static const gchar *documents_menu_xml =
	"    <placeholder name=\"ChildItem\">\n"
	"        <menuitem name=\"Action\" verb=\"Action\" label=\"Rename %s...\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-convert\"/>\n"
	"        <menuitem name=\"Remove\" verb=\"Remove\" label=\"Remove %s\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-close\"/>\n"
	"        <separator/>" "    </placeholder>";

typedef struct _PanelMenuDocuments {
	gint id;
	GtkWidget *documents;
	GtkWidget *menu;
	gchar *name;
	GList *items_list;
	GList *docs_list;
} PanelMenuDocuments;

static void panel_menu_documents_set_list (PanelMenuEntry *entry, GList *list);
static gint panel_menu_documents_remove_cb (GtkWidget *widget,
					    GdkEventKey *event,
					    PanelMenuDocuments *documents);
static void rename_documents_cb (GtkWidget *widget, PanelMenuEntry *entry,
				 const gchar *verb);

static gint object_counter = 0;

PanelMenuEntry *
panel_menu_documents_new (PanelMenu *parent, gchar *name)
{
	PanelMenuEntry *entry;
	gchar *item_key;

	item_key = g_strdup_printf ("documents%d/name", object_counter);
	panel_applet_gconf_set_string (parent->applet, item_key, name, NULL);
	g_free (item_key);
	entry = panel_menu_documents_new_with_id (parent, object_counter);
	return entry;
}

PanelMenuEntry *
panel_menu_documents_new_with_id (PanelMenu *parent, gint id)
{
	PanelMenuEntry *entry;
	PanelMenuDocuments *documents;
	GtkWidget *tearoff;
	GtkWidget *image;
	gchar *base_key;
	gchar *dir_key;
	GConfClient *client;
	gchar *name;

	if (id > object_counter)
		object_counter = id;

	entry = g_new0 (PanelMenuEntry, 1);
	entry->type = PANEL_MENU_TYPE_DOCUMENTS;
	entry->parent = parent;
	documents = g_new0 (PanelMenuDocuments, 1);
	entry->data = (gpointer) documents;
	documents->id = id;
	documents->documents = gtk_menu_item_new_with_label ("");
	panel_menu_common_widget_dnd_init (entry);
	gtk_widget_show (documents->documents);
	documents->menu = gtk_menu_new ();
	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (documents->menu), tearoff);
	gtk_widget_show (tearoff);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (documents->documents),
				   documents->menu);
	g_signal_connect (G_OBJECT (documents->menu), "key_press_event",
			  G_CALLBACK (panel_menu_documents_remove_cb),
			  documents);
	base_key = g_strdup_printf ("documents%d", id);
	dir_key = panel_applet_gconf_get_full_key (parent->applet, base_key);
	client = gconf_client_get_default ();
	if (gconf_client_dir_exists (client, dir_key, NULL)) {
		gchar *key;
		key = g_strdup_printf ("%s/name", base_key);
		name = panel_applet_gconf_get_string (parent->applet, key, NULL);
		g_free (key);
		key = g_strdup_printf ("%s/docs-list", base_key);
		panel_menu_documents_set_list (entry,
			panel_applet_gconf_get_string_list (parent->applet, key));
		g_free (key);
	} else {
		name = g_strdup (_("Documents"));
	}
	g_object_unref (G_OBJECT (client));
	g_free (base_key);
	g_free (dir_key);
	panel_menu_documents_set_name (entry, name);
	g_free (name);
	object_counter++;
	return entry;
}

static void
panel_menu_documents_set_list (PanelMenuEntry *entry, GList *list)
{
	PanelMenuDocuments *documents;
	GList *iter;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DOCUMENTS);
	if (!list) return;

	documents = (PanelMenuDocuments *)entry->data;
	for (iter = list; iter; iter = iter->next) {
		panel_menu_documents_append_item (entry, iter->data);
		g_free (iter->data);
	}
	g_list_free (list);
}

void
panel_menu_documents_set_name (PanelMenuEntry *entry, gchar *name)
{
	PanelMenuDocuments *documents;

	g_return_if_fail (name != NULL);
	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DOCUMENTS);

	documents = (PanelMenuDocuments *) entry->data;
	if (documents->name)
		g_free (documents->name);
	documents->name = name ? g_strdup (name) : g_strdup ("");
	gtk_label_set_text (GTK_LABEL (GTK_BIN (documents->documents)->child),
			    name);
}

void
panel_menu_documents_merge_ui (PanelMenuEntry *entry)
{
	PanelMenuDocuments *documents;
	BonoboUIComponent *component;
	gchar *xml;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DOCUMENTS);

	documents = (PanelMenuDocuments *) entry->data;
	component = panel_applet_get_popup_component (entry->parent->applet);

	bonobo_ui_component_add_verb (component, "Action",
				      (BonoboUIVerbFn) rename_documents_cb,
				      entry);
	bonobo_ui_component_add_verb (component, "Remove",
				      (BonoboUIVerbFn)
				      panel_menu_common_remove_entry, entry);
	xml = g_strdup_printf (documents_menu_xml, documents->name,
			       documents->name);
	bonobo_ui_component_set (component, "/popups/button3/ChildMerge/", xml,
				 NULL);
	g_free (xml);
}

void
panel_menu_documents_destroy (PanelMenuEntry *entry)
{
	PanelMenuDocuments *documents;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DOCUMENTS);

	documents = (PanelMenuDocuments *) entry->data;
	if (documents->name)
		g_free (documents->name);
	if (documents->items_list)
		g_list_free (documents->items_list);
	for (cur = documents->docs_list; cur; cur = cur->next) {
		if (cur->data)
			g_free (cur->data);
	}
	gtk_widget_destroy (documents->documents);
	g_free (documents);
}

GtkWidget *
panel_menu_documents_get_widget (PanelMenuEntry *entry)
{
	PanelMenuDocuments *documents;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DOCUMENTS);

	documents = (PanelMenuDocuments *) entry->data;
	return (documents->documents);
}

gboolean
panel_menu_documents_accept_drop (PanelMenuEntry *entry, GnomeVFSURI *uri)
{
	gchar *fileuri;
	gboolean retval = FALSE;

	g_return_val_if_fail (entry != NULL, FALSE);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_DOCUMENTS, FALSE);

	fileuri = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	g_print ("(documents) uri is %s\n", fileuri);
	if ((retval = panel_menu_documents_append_item (entry, fileuri)))
		panel_menu_documents_save_config (entry);
	g_free (fileuri);
	return (retval);
}

gboolean
panel_menu_documents_append_item (PanelMenuEntry *entry, gchar *uri)
{
	PanelMenuDocuments *documents;
	GnomeVFSFileInfo finfo;
	GtkWidget *menuitem;
	GtkWidget *image;
	gchar *icon;
	gboolean retval = FALSE;

	g_return_val_if_fail (entry != NULL, FALSE);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_DOCUMENTS, FALSE);

	documents = (PanelMenuDocuments *) entry->data;
	if (!strncmp (uri, "file:", strlen ("file:"))) {
		if (gnome_vfs_get_file_info
		    (uri, &finfo,
		     GNOME_VFS_FILE_INFO_FOLLOW_LINKS) == GNOME_VFS_OK) {
			if (finfo.type == GNOME_VFS_FILE_TYPE_REGULAR) {
				menuitem =
					gtk_image_menu_item_new_with_label
					(finfo.name);
				panel_menu_common_apps_menuitem_dnd_init
					(menuitem);
				icon = (gchar *)
					gnome_vfs_mime_get_value ((gchar *)
								  gnome_vfs_mime_type_from_name
								  (finfo.name),
								  "icon-filename");
				if (icon) {
					panel_menu_common_set_icon_scaled_from_file
						(GTK_MENU_ITEM (menuitem),
						 icon);
				} else {
					image = gtk_image_new_from_stock
						(GTK_STOCK_NEW,
						 GTK_ICON_SIZE_MENU);
					gtk_image_menu_item_set_image
						(GTK_IMAGE_MENU_ITEM (menuitem),
						 image);
					gtk_widget_show (image);
				}
				gtk_menu_shell_append (GTK_MENU_SHELL
						       (documents->menu),
						       menuitem);
				g_object_set_data (G_OBJECT (menuitem),
						   "uri-path", g_strdup (uri));
				g_signal_connect (G_OBJECT (menuitem),
						  "destroy",
						  G_CALLBACK
						  (panel_menu_common_destroy_apps_menuitem),
						  NULL);
				gtk_widget_show (menuitem);
				documents->items_list =
					g_list_append (documents->items_list,
						       menuitem);
				documents->docs_list =
					g_list_append (documents->docs_list,
						       g_strdup (uri));
				retval = TRUE;
			}
		}
	}
	return (retval);
}

static gint
panel_menu_documents_remove_cb (GtkWidget *widget, GdkEventKey *event,
				PanelMenuDocuments *documents)
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
		for (cur = documents->items_list; cur;
		     cur = cur->next, counter++) {
			if (item == GTK_WIDGET (cur->data)) {
				documents->items_list =
					g_list_remove (documents->items_list,
						       cur->data);
				temp = (gchar *) g_list_nth_data (documents->
								  docs_list,
								  counter);
				g_print ("removing %s from the documents list\n", temp);
				documents->docs_list =
					g_list_remove (documents->docs_list,
						       temp);
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
panel_menu_documents_new_with_dialog (PanelMenu *panel_menu)
{
	GtkWidget *dialog;
	GtkWidget *name_entry;
	gint response;
	gchar *name;
	PanelMenuEntry *entry;

	dialog = panel_menu_common_single_entry_dialog_new (_
							    ("Create documents item..."),
							    _("Name:"),
							    _("Documents"),
							    &name_entry);
	gtk_widget_show (dialog);
	gtk_widget_grab_focus (name_entry);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_OK) {
		name = (gchar *) gtk_entry_get_text (GTK_ENTRY (name_entry));
		entry = panel_menu_documents_new (panel_menu, name);
		panel_menu->entries =
			g_list_append (panel_menu->entries, (gpointer) entry);
		gtk_menu_shell_append (GTK_MENU_SHELL (panel_menu->menubar),
				       panel_menu_documents_get_widget (entry));
		panel_menu_config_save_layout (panel_menu);
		panel_menu_documents_save_config (entry);
	}
	gtk_widget_destroy (dialog);
}

static void
rename_documents_cb (GtkWidget *widget, PanelMenuEntry *entry,
		     const gchar *verb)
{
	PanelMenuDocuments *documents;
	GtkWidget *dialog;
	GtkWidget *name_entry;
	gint response;
	gchar *name;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DOCUMENTS);

	documents = (PanelMenuDocuments *) entry->data;

	dialog = panel_menu_common_single_entry_dialog_new (_("Rename documents item..."),
							    _("Name:"),
							    documents->name,
							    &name_entry);
	gtk_widget_show (dialog);
	gtk_widget_grab_focus (name_entry);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_OK) {
		name = (gchar *) gtk_entry_get_text (GTK_ENTRY (name_entry));
		if (strcmp (name, documents->name)) {
			panel_menu_documents_set_name (entry, name);
			panel_menu_documents_save_config (entry);
		}
	}
	gtk_widget_destroy (dialog);
}

gchar *
panel_menu_documents_save_config (PanelMenuEntry *entry)
{
	PanelMenuDocuments *documents;
	PanelMenu *panel_menu;
	PanelApplet *applet;
	gchar *id;
	gchar *key;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DOCUMENTS);

	panel_menu = entry->parent;
	applet = panel_menu->applet;
	documents = (PanelMenuDocuments *) entry->data;

	id = g_strdup_printf ("documents%d", documents->id);
	key = g_strdup_printf ("%s/name", id);
	panel_applet_gconf_set_string (applet, key, documents->name, NULL);
	g_free (key);
	key = g_strdup_printf ("%s/docs-list", id);
	panel_applet_gconf_set_string_list (applet, key, documents->docs_list);
	g_free (key);
	return id;
}

void
panel_menu_documents_remove_config (PanelMenuEntry *entry)
{
	PanelMenuDocuments *documents;
	PanelMenu *panel_menu;
	PanelApplet *applet;
	GConfClient *client;
	gchar *base_key;
	gchar *key;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_DOCUMENTS);

	panel_menu = entry->parent;
	applet = panel_menu->applet;
	documents = (PanelMenuDocuments *) entry->data;

	client = gconf_client_get_default ();
	g_return_if_fail (client != NULL);
	base_key = panel_applet_get_preferences_key (applet);
	key = g_strdup_printf ("%s/documents%d", base_key, documents->id);
	_gconf_client_clean_dir (client, key);
	g_free (base_key);
	g_free (key);
	g_object_unref (G_OBJECT (client));
}
