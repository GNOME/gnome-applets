/*  panel-menu-pixbuf.c
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <glib.h>
#include <string.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include "panel-menu.h"
#include "panel-menu-pixbuf.h"

typedef struct _PixbufEntry {
	gchar *icon_filename;
	GdkPixbuf *pixbuf;
	time_t mtime;
	GList *attachments;
}PixbufEntry;

static void register_gconf_notifications (void);
static gboolean pixbuf_cache_remove (gpointer key, gpointer value,
				     gpointer user_data);

static PixbufEntry *panel_menu_pixbuf_entry_add (const gchar *icon_filename,
						 GdkPixbuf *pixbuf);
static void panel_menu_pixbuf_entry_set (PixbufEntry *entry,
					 GdkPixbuf *pixbuf,
					 time_t mtime);
static void panel_menu_pixbuf_entry_remove (PixbufEntry *entry);
static void panel_menu_pixbuf_entry_validate (PixbufEntry *entry);
static void panel_menu_pixbuf_entry_propogate_change (PixbufEntry *entry);
static void panel_menu_pixbuf_add_reference_by_name (GtkMenuItem *menuitem,
						     const gchar *icon_filename);
static void panel_menu_pixbuf_add_reference_by_entry (GtkMenuItem *menuitem,
						      PixbufEntry *entry);

static void panel_menu_pixbuf_remove_reference_by_entry (GtkMenuItem *menuitem,
							 PixbufEntry *entry);
static void panel_menu_pixbuf_remove_reference_by_name (GtkMenuItem *menuitem,
							const gchar *icon_filename);

static GdkPixbuf *create_pixbuf_scaled (const gchar *icon_filename);

static void nautilus_theme_changed (GConfClient *client, guint cnxn_id,
				    GConfEntry *entry, gpointer data);
static void icon_size_changed (GConfClient *client, guint cnxn_id,
			       GConfEntry *entry, gpointer data);
static void pixbuf_cache_foreach_resize (gpointer key, gpointer value,
					 gpointer user_data);
static void panel_menu_pixbuf_setup_nautilus_icons (void);
static void setup_icons_from_nautilus_theme (const gchar *theme);
static void setup_default_icons (void);

static time_t get_file_mtime (const gchar *uri);

static GHashTable *pixbuf_cache = NULL;
static GConfClient *gconf_client;
static guint nautilus_connection = 0;
static guint icon_connection = 0;
static gint icon_size = 0;

static void
panel_menu_pixbuf_init (void)
{
	if (pixbuf_cache == NULL) {
		pixbuf_cache = g_hash_table_new (g_str_hash,
						 g_str_equal);

		gconf_client = gconf_client_get_default ();
		if (!gconf_client_dir_exists (gconf_client,
			ICON_SIZE_KEY_PARENT, NULL)) {
		   	icon_size = 20;
		   	gconf_client_set_int (gconf_client,
		   			      ICON_SIZE_KEY,
		   			      icon_size, NULL);
		} else {
			icon_size = gconf_client_get_int (gconf_client,
							  ICON_SIZE_KEY,
							  NULL);
		}
		register_gconf_notifications ();
		setup_default_icons ();
		panel_menu_pixbuf_setup_nautilus_icons ();
	}
}

void
panel_menu_pixbuf_exit (void)
{
	if (pixbuf_cache) {
		/*
		   Don't bother killing the pixmaps
		   They should die on their own as menu items
		   unreference them, or it wont matter because
		   we've been killed explicitly.
		*/
		g_hash_table_destroy (pixbuf_cache);
		pixbuf_cache = NULL;
	}
	if (nautilus_connection) {
		gconf_client_notify_remove (gconf_client,
					    nautilus_connection);
	}
	if (icon_connection) {
		gconf_client_notify_remove (gconf_client,
					    icon_connection);
	}
	if (gconf_client) {
		g_object_unref (G_OBJECT (gconf_client));
		gconf_client = NULL;
	}
}

static void
register_gconf_notifications (void)
{
	g_return_if_fail (gconf_client != NULL);

	gconf_client_add_dir (gconf_client,
			     "/apps/nautilus/preferences",
			      GCONF_CLIENT_PRELOAD_NONE, NULL);
	nautilus_connection = gconf_client_notify_add (gconf_client,
		"/apps/nautilus/preferences/theme",
		(GConfClientNotifyFunc) nautilus_theme_changed, NULL,
		(GFreeFunc) NULL, NULL);
	gconf_client_add_dir (gconf_client,
			      ICON_SIZE_KEY_PARENT,
			      GCONF_CLIENT_PRELOAD_NONE, NULL);
	icon_connection = gconf_client_notify_add (gconf_client,
		ICON_SIZE_KEY,
		(GConfClientNotifyFunc) icon_size_changed, NULL,
		(GFreeFunc) NULL, NULL);
}

gint
panel_menu_pixbuf_get_icon_size (void)
{
	panel_menu_pixbuf_init ();

	return icon_size;
}

void
panel_menu_pixbuf_set_icon_size (gint size)
{
	if (size != icon_size) {
		icon_size = size;
		gconf_client_set_int (gconf_client, ICON_SIZE_KEY,
				      icon_size, NULL);
	}
}

static PixbufEntry *
panel_menu_pixbuf_entry_add (const gchar *icon_filename, GdkPixbuf *pixbuf)
{
	PixbufEntry *entry;

	g_return_val_if_fail ((g_hash_table_lookup (pixbuf_cache, icon_filename) == NULL), NULL);
				      

	entry = g_new0(PixbufEntry, 1);
	entry->icon_filename = g_strdup (icon_filename);
	entry->pixbuf = pixbuf;
	g_object_ref (G_OBJECT (entry->pixbuf));
	entry->mtime = get_file_mtime (icon_filename);
	g_hash_table_insert (pixbuf_cache,
			     entry->icon_filename,
			     entry);
	return entry;
}

static void
panel_menu_pixbuf_entry_set (PixbufEntry *entry, GdkPixbuf *pixbuf, time_t mtime)
{
	GdkPixbuf *old;

	g_return_if_fail (entry != NULL);

	old = entry->pixbuf;
	entry->pixbuf = pixbuf;
	g_object_ref (G_OBJECT (entry->pixbuf));
	panel_menu_pixbuf_entry_propogate_change (entry);
	g_object_unref (G_OBJECT (old));
}

static void
panel_menu_pixbuf_entry_remove (PixbufEntry *entry)
{
	g_return_if_fail (entry != NULL);
	g_return_if_fail (g_list_length (entry->attachments) == 0);

	if (entry->mtime) {
		g_hash_table_remove (pixbuf_cache, entry->icon_filename);
		g_free (entry->icon_filename);
		g_object_unref (G_OBJECT (entry->pixbuf));
		g_free (entry);
	}
}

static void
panel_menu_pixbuf_entry_validate (PixbufEntry *entry)
{
	time_t new_time;

	g_return_if_fail (entry != NULL);

	new_time = get_file_mtime (entry->icon_filename);
	if (new_time > entry->mtime) {
		GdkPixbuf *pixbuf;
		g_print ("(PixbufEntry::validate) invalidated, "
			 "re-allocating pixbuf for %s\n", entry->icon_filename);
		pixbuf = create_pixbuf_scaled (entry->icon_filename);
		if (pixbuf) {
			panel_menu_pixbuf_entry_set (entry, pixbuf,
						     new_time);
		}
	}
}

static void
panel_menu_pixbuf_entry_propogate_change (PixbufEntry *entry)
{
	GList *cur;

	g_return_if_fail (entry != NULL);

	for (cur = entry->attachments; cur; cur = cur->next) {
		GtkWidget *image;
		image = gtk_image_menu_item_get_image (
			GTK_IMAGE_MENU_ITEM (cur->data));
		if (image) {
			gtk_image_set_from_pixbuf (GTK_IMAGE(image),
					      entry->pixbuf);
		}
	}
}

static void
panel_menu_pixbuf_add_reference_by_entry (GtkMenuItem *menuitem, PixbufEntry *entry)
{
	g_return_if_fail (entry != NULL);

	entry->attachments = g_list_prepend (entry->attachments,
					     menuitem);
	g_object_set_data (G_OBJECT (menuitem), "icon-filename",
			   entry->icon_filename);
	g_signal_connect (G_OBJECT (menuitem), "destroy",
			  G_CALLBACK (panel_menu_pixbuf_remove_reference_by_entry),
			  entry);
}

static void
panel_menu_pixbuf_add_reference_by_name (GtkMenuItem *menuitem, const gchar *icon_filename)
{
	PixbufEntry *entry;

	entry = g_hash_table_lookup (pixbuf_cache, icon_filename);
	g_return_if_fail (entry != NULL);

	panel_menu_pixbuf_add_reference_by_entry (menuitem, entry);
}

static void
panel_menu_pixbuf_remove_reference_by_entry (GtkMenuItem *menuitem, PixbufEntry *entry)
{
	g_return_if_fail (menuitem != NULL);
	g_return_if_fail (entry != NULL);
	entry->attachments = g_list_remove (entry->attachments,
					    menuitem);
	if (g_list_length (entry->attachments) < 1)
		panel_menu_pixbuf_entry_remove (entry);
	g_object_set_data (G_OBJECT (menuitem), "icon-filename", NULL);
}

static void
panel_menu_pixbuf_remove_reference_by_name (GtkMenuItem *menuitem, const gchar *icon_filename)
{
	PixbufEntry *entry;
	
	g_return_if_fail (menuitem != NULL);
	g_return_if_fail (icon_filename != NULL);
	g_return_if_fail (entry = g_hash_table_lookup (pixbuf_cache, icon_filename));

	panel_menu_pixbuf_remove_reference_by_entry (menuitem, entry);
}

void
panel_menu_pixbuf_set_icon (GtkMenuItem *menuitem,
			    const gchar *icon_filename)
{
	gchar *prev;
	PixbufEntry *entry;
	GdkPixbuf *pixbuf;
	GtkWidget *image;

	if (!GTK_IS_IMAGE_MENU_ITEM (menuitem))
		return;

	panel_menu_pixbuf_init ();

	prev = g_object_get_data (G_OBJECT (menuitem), "icon-filename");
	if (prev) {
		panel_menu_pixbuf_remove_reference_by_name (
			menuitem, icon_filename);
	}

	entry = g_hash_table_lookup (pixbuf_cache, icon_filename);
	if (entry) {
		/* Update the pixbuf if the file has changed */
		panel_menu_pixbuf_entry_validate (entry);
		/* Set icon-filename and add to attachments list */
		panel_menu_pixbuf_add_reference_by_entry (menuitem, entry);
		pixbuf = entry->pixbuf;
	} else {
		pixbuf = create_pixbuf_scaled (icon_filename);
		if (pixbuf) {
			entry = panel_menu_pixbuf_entry_add (icon_filename, pixbuf);
			panel_menu_pixbuf_add_reference_by_entry (menuitem, entry);
		}
	}
	if (pixbuf) {
		image = gtk_image_new_from_pixbuf (pixbuf);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
		gtk_widget_show (image);
	}
}

static GdkPixbuf *
create_pixbuf_scaled (const gchar *icon_filename)
{
	GdkPixbuf *pixbuf = NULL;

	pixbuf = gdk_pixbuf_new_from_file (icon_filename, NULL);
	if (pixbuf) {
		double pix_x, pix_y;
		gint icon_size;
		pix_x = gdk_pixbuf_get_width (pixbuf);
		pix_y = gdk_pixbuf_get_height (pixbuf);
		icon_size = panel_menu_pixbuf_get_icon_size ();
		if (pix_x > icon_size || pix_y > icon_size) {
			GdkPixbuf *scaled;
			double greatest;
			greatest = pix_x > pix_y ? pix_x : pix_y;
			scaled = gdk_pixbuf_scale_simple (pixbuf,
							 (icon_size /
							  greatest) * pix_x,
							 (icon_size /
							  greatest) * pix_y,
							  GDK_INTERP_BILINEAR);
			g_object_unref (G_OBJECT (pixbuf));
			pixbuf = scaled;
		}
	}
	return pixbuf;
}

static void
nautilus_theme_changed (GConfClient *client,
			guint cnxn_id,
			GConfEntry *entry,
			gpointer data)
{
	const gchar *key;
	GConfValue  *value;
	const gchar *keyvalue;

	g_return_if_fail (client != NULL);
	g_return_if_fail (entry != NULL);

	key = gconf_entry_get_key (entry);
	value = gconf_entry_get_value (entry);
	keyvalue = gconf_value_get_string (value);
	g_print ("(nautilus-theme-changed) key - value pair is %s - %s\n",
		  key, keyvalue);
	if (keyvalue && strlen (keyvalue) > 1) {
		g_print ("Current nautilus theme is %s\n", keyvalue);
		setup_icons_from_nautilus_theme (keyvalue);
	}
}

static void
icon_size_changed (GConfClient *client,
		   guint cnxn_id,
		   GConfEntry *entry,
		   gpointer data)
{
	const gchar *key;
	GConfValue  *value;
	gint size;

	g_return_if_fail (client != NULL);
	g_return_if_fail (entry != NULL);

	key = gconf_entry_get_key (entry);
	value = gconf_entry_get_value (entry);
	size = gconf_value_get_int (value);
	g_print ("(icon-size-changed) - %d\n", size);
	if (size && size != icon_size) {
		icon_size = size;
		g_hash_table_foreach (pixbuf_cache,
				      pixbuf_cache_foreach_resize,
				      NULL);
		setup_default_icons ();
		panel_menu_pixbuf_setup_nautilus_icons ();
	}
}

static void
pixbuf_cache_foreach_resize (gpointer key, gpointer value, gpointer user_data)
{
	PixbufEntry *entry;
	GdkPixbuf *pixbuf;

	g_return_if_fail (value != NULL);

	entry = (PixbufEntry *)value;
	pixbuf = create_pixbuf_scaled (entry->icon_filename);
	if (pixbuf) {
		panel_menu_pixbuf_entry_set (entry, pixbuf,
			get_file_mtime (entry->icon_filename));
	}
}

static void
panel_menu_pixbuf_setup_nautilus_icons (void)
{
	gchar *keyvalue;

	keyvalue = gconf_client_get_string (gconf_client,
		"/apps/nautilus/preferences/theme", NULL);

	if (keyvalue && strlen (keyvalue) > 1) {
		g_print ("Current nautilus theme is %s\n", keyvalue);
		setup_icons_from_nautilus_theme (keyvalue);
	}
}

static void
setup_icons_from_nautilus_theme (const gchar *theme)
{
	gchar *path;
	gchar *file;
	GdkPixbuf *pixbuf;
	PixbufEntry *entry;

	entry = g_hash_table_lookup (pixbuf_cache, "directory");

	if (!strcmp (theme, "default"))
		path = g_strdup (DATADIR "/pixmaps/nautilus/");
	else
		path = g_strdup_printf (DATADIR "/pixmaps/nautilus/%s/", theme);
	if (icon_size > 48)
		file = g_strconcat (path, "i-directory-96-aa.png", NULL);
	else if (icon_size > 36)
		file = g_strconcat (path, "i-directory-48-aa.png", NULL);
	else
		file = g_strconcat (path, "i-directory-20-aa.png", NULL);
	pixbuf = create_pixbuf_scaled (file);
	if (!pixbuf) {
		g_free (file);
		file = g_strconcat (path, "i-directory-aa.png", NULL);
		pixbuf = create_pixbuf_scaled (file);
	}

	if (pixbuf && entry) {
		panel_menu_pixbuf_entry_set (entry, pixbuf, 0);
	} else if (pixbuf) {
		panel_menu_pixbuf_entry_add ("directory", pixbuf);
	}
	g_free (file);
	g_free (path);
}

static void
setup_default_icons (void)
{
	PixbufEntry *entry;
	GdkPixbuf *pixbuf;

	entry = g_hash_table_lookup (pixbuf_cache, "directory");
	pixbuf = create_pixbuf_scaled (DATADIR "/pixmaps/gnome-folder.png");
	if (pixbuf && entry) {
		panel_menu_pixbuf_entry_set (entry, pixbuf, 0);
	} else if (pixbuf) {
		panel_menu_pixbuf_entry_add ("directory", pixbuf);
	}
	entry = g_hash_table_lookup (pixbuf_cache, "unknown");
	pixbuf = create_pixbuf_scaled (DATADIR "/pixmaps/gnome-unknown.png");
	if (pixbuf && entry) {
		panel_menu_pixbuf_entry_set (entry, pixbuf, 0);
	} else if (pixbuf) {
		panel_menu_pixbuf_entry_add ("unknown", pixbuf);
	}
}

static time_t
get_file_mtime (const gchar *uri)
{
	struct stat s;
	time_t mtime = 0;

	if ((stat (uri, &s) == 0)) {
		mtime = s.st_mtime;
	}
	return mtime;
}
