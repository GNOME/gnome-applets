/*  panel-menu-options.c
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

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-options.h"

typedef struct _PanelMenuOptions {
	GtkWidget *options;
	GtkWidget *menu;
} PanelMenuOptions;

PanelMenuEntry *
panel_menu_options_new (PanelMenu *parent, const gchar *icon)
{
	PanelMenuEntry *entry;
	PanelMenuOptions *options;
	GdkPixbuf *pixbuf;
	GdkPixbuf *scaled;
	GtkWidget *image;
	GtkWidget *tearoff;

	entry = g_new0 (PanelMenuEntry, 1);
	entry->type = PANEL_MENU_TYPE_OPTIONS;
	entry->parent = parent;
	options = g_new0 (PanelMenuOptions, 1);
	entry->data = (gpointer) options;
	options->options = gtk_image_menu_item_new_with_label ("");
	panel_menu_common_widget_dnd_init (entry);
	gtk_widget_show (options->options);
	panel_menu_options_set_icon (entry, icon, parent->size - 8);
	options->menu = gtk_menu_new ();
	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (options->menu), tearoff);
	gtk_widget_show (tearoff);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (options->options),
				   options->menu);
	return (entry);
}

void
panel_menu_options_set_icon (PanelMenuEntry *entry, const gchar *icon,
			     gint size)
{
	PanelMenuOptions *options;
	GdkPixbuf *pixbuf;
	GdkPixbuf *scaled;
	GtkWidget *image;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_OPTIONS);

	options = (PanelMenuOptions *) entry->data;
	if (icon) {
		if (entry->parent->icon_handle_image != icon) {
			if (entry->parent->icon_handle_image)
				g_free (entry->parent->icon_handle_image);
			entry->parent->icon_handle_image = g_strdup (icon);
		}
		pixbuf = gdk_pixbuf_new_from_file (icon, NULL);
		if (pixbuf) {
			scaled = gdk_pixbuf_scale_simple (pixbuf, size, size,
							  GDK_INTERP_BILINEAR);
			if (scaled) {
				image = gtk_image_new_from_pixbuf (scaled);
				gtk_image_menu_item_set_image
					(GTK_IMAGE_MENU_ITEM (options->options),
					 image);
				gtk_widget_show (image);
			}
			g_object_unref (G_OBJECT (pixbuf));
		}
	} else {
		if (entry->parent->icon_handle_image) {
			g_free (entry->parent->icon_handle_image);
			entry->parent->icon_handle_image = NULL;
		}
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM
					       (options->options), NULL);
	}
}

void
panel_menu_options_destroy (PanelMenuEntry *entry)
{
	PanelMenuOptions *options;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_OPTIONS);

	options = (PanelMenuOptions *) entry->data;
	gtk_widget_destroy (options->options);
	g_free (options);
}

GtkWidget *
panel_menu_options_get_widget (PanelMenuEntry *entry)
{
	PanelMenuOptions *options;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_OPTIONS, NULL);

	options = (PanelMenuOptions *) entry->data;
	return (options->options);
}

void
panel_menu_options_append_option (PanelMenuEntry *entry, GtkWidget *option)
{
	PanelMenuOptions *options;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_OPTIONS);

	options = (PanelMenuOptions *) entry->data;
	gtk_menu_shell_append (GTK_MENU_SHELL (options->menu), option);
	gtk_widget_show (option);
}

void
panel_menu_options_insert_option (PanelMenuEntry *entry, GtkWidget *option,
				  gint position)
{
	PanelMenuOptions *options;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_OPTIONS);

	options = (PanelMenuOptions *) entry->data;
	gtk_menu_shell_insert (GTK_MENU_SHELL (options->menu), option,
			       position);
	gtk_widget_show (option);
}

gchar *
panel_menu_options_dump_xml (PanelMenuEntry *entry)
{
	PanelMenuOptions *options;
	GString *string;
	gchar *str;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_OPTIONS, NULL);

	options = (PanelMenuOptions *) entry->data;

	string = g_string_new ("    <options-item>\n");
	g_string_append (string, "    </options-item>\n");
	str = string->str;
	g_string_free (string, FALSE);
	return (str);
}

void
panel_menu_options_rescale (PanelMenuEntry *entry)
{
	PanelMenuOptions *options;
	GdkPixbuf *pixbuf;
	GdkPixbuf *scaled;
	GtkWidget *image;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_OPTIONS);

	options = (PanelMenuOptions *) entry->data;
	pixbuf = gdk_pixbuf_new_from_file (entry->parent->icon_handle_image,
					   NULL);
	scaled = gdk_pixbuf_scale_simple (pixbuf, entry->parent->size - 8,
					  entry->parent->size - 8,
					  GDK_INTERP_BILINEAR);
	image = gtk_image_new_from_pixbuf (scaled);
	g_object_unref (G_OBJECT (pixbuf));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (options->options),
				       image);
	gtk_widget_show (image);
}
