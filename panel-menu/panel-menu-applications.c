/*  panel-menu-applications.c
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

#include "panel-menu-desktop-item.h"

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-path.h"
#include "panel-menu-applications.h"

static const gchar *applications_menu_xml =
	"    <placeholder name=\"ChildItem\">\n"
	"        <menuitem name=\"Regenerate\" verb=\"Regenerate\" label=\"Regenerate Applications\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-refresh\"/>\n"
	"        <menuitem name=\"Remove\" verb=\"Remove\" label=\"Remove Applications\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-close\"/>\n"
	"        <separator/>"
	"    </placeholder>";

typedef struct _PanelMenuApplications {
	GtkWidget *applications;
	GtkWidget *menu;
	gchar *icon;
} PanelMenuApplications;

static void regenerate_menus_cb (GtkWidget *menuitem, PanelMenuEntry *entry,
				 const gchar *verb);

PanelMenuEntry *
panel_menu_applications_new (PanelMenu *parent)
{
	PanelMenuEntry *entry;
	PanelMenuApplications *applications;
	GtkWidget *tearoff;
	gchar *icon;

	entry = g_new0 (PanelMenuEntry, 1);
	entry->type = PANEL_MENU_TYPE_APPLICATIONS;
	entry->parent = parent;
	applications = g_new0 (PanelMenuApplications, 1);
	entry->data = (gpointer) applications;
	applications->applications = gtk_image_menu_item_new_with_label (_("Applications"));
	panel_menu_common_widget_dnd_init (entry);
	gtk_widget_show (applications->applications);
	applications->menu = gtk_menu_new ();

	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (applications->menu), tearoff);
	if (parent->menu_tearoffs) {
		gtk_widget_show (tearoff);
	}

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (applications->applications),
				   applications->menu);
	g_object_set_data (G_OBJECT (applications->menu), "immortal",
			   GINT_TO_POINTER(TRUE));
	GTK_MENU (applications->menu)->parent_menu_item = applications->applications;
	icon = panel_applet_gconf_get_string (parent->applet,
					     "applications-image",
					      NULL);
	if (!icon)
		icon = g_strdup (DATADIR "/pixmaps/gnome-logo-icon-transparent.png");
	panel_menu_applications_set_icon (entry, icon);
	g_free (icon);
	panel_menu_path_load ("applications:///", GTK_MENU_SHELL(applications->menu));
	return entry;
}

void
panel_menu_applications_set_icon (PanelMenuEntry *entry, const gchar *icon)
{
	PanelMenuApplications *applications;
	GdkPixbuf *pixbuf;
	GdkPixbuf *scaled;
	GtkWidget *image;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_APPLICATIONS);

	applications = (PanelMenuApplications *) entry->data;
	g_free (applications->icon);
	if (icon) {
		applications->icon = g_strdup (icon);
		panel_menu_applications_rescale_icon (entry);
	} else {
		applications->icon = NULL;
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM
			(applications->applications), NULL);
	}
}

const gchar *
panel_menu_applications_get_icon (PanelMenuEntry *entry)
{
	PanelMenuApplications *applications;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_APPLICATIONS);

	applications = (PanelMenuApplications *) entry->data;
	return applications->icon;
}

static void
regenerate_menus_cb (GtkWidget *menuitem, PanelMenuEntry *entry,
		     const gchar *verb)
{
	PanelMenuApplications *applications;
	GList *list;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_APPLICATIONS);

	applications = (PanelMenuApplications *) entry->data;

	list = g_list_copy (GTK_MENU_SHELL (applications->menu)->children);
	for (cur = list; cur; cur = cur->next) {
		if (g_object_get_data (G_OBJECT (cur->data), "uri-path"))
			gtk_widget_destroy (GTK_WIDGET (cur->data));
	}
	g_list_free (list);

	panel_menu_path_load ("applications:///",
			      GTK_MENU_SHELL (applications->menu));
}

void
panel_menu_applications_merge_ui (PanelMenuEntry *entry)
{
	PanelMenuApplications *applications;
	BonoboUIComponent  *component;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_APPLICATIONS);

	applications = (PanelMenuApplications *) entry->data;
	component = panel_applet_get_popup_component (entry->parent->applet);

	bonobo_ui_component_add_verb (component, "Regenerate",
				     (BonoboUIVerbFn)regenerate_menus_cb, entry);
	bonobo_ui_component_add_verb (component, "Remove",
				     (BonoboUIVerbFn)panel_menu_common_remove_entry, entry);
	bonobo_ui_component_set (component, "/popups/button3/ChildMerge/",
				 applications_menu_xml, NULL);
}

void
panel_menu_applications_destroy (PanelMenuEntry *entry)
{
	PanelMenuApplications *applications;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_APPLICATIONS);

	applications = (PanelMenuApplications *) entry->data;
	gtk_widget_destroy (applications->applications);
	g_free (applications);
}

GtkWidget *
panel_menu_applications_get_widget (PanelMenuEntry *entry)
{
	PanelMenuApplications *applications;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_APPLICATIONS, NULL);

	applications = (PanelMenuApplications *) entry->data;
	return applications->applications;
}

gchar *
panel_menu_applications_save_config (PanelMenuEntry *entry)
{
	PanelMenuApplications *applications;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_APPLICATIONS);

	applications = (PanelMenuApplications *)entry->data;

	panel_applet_gconf_set_string (entry->parent->applet,
				      "applications-image",
				       applications->icon ?
				       applications->icon :
				       "none", NULL);
	return g_strdup ("applications");
}

void
panel_menu_applications_rescale_icon (PanelMenuEntry *entry)
{
	PanelMenuApplications *applications;
	GdkPixbuf *pixbuf;
	GdkPixbuf *scaled;
	GtkWidget *image;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_APPLICATIONS);

	applications = (PanelMenuApplications *) entry->data;
	pixbuf = gdk_pixbuf_new_from_file (applications->icon,
					   NULL);
	if (pixbuf) {
		scaled = gdk_pixbuf_scale_simple (pixbuf, entry->parent->size - 8,
						  entry->parent->size - 8,
						  GDK_INTERP_BILINEAR);
		image = gtk_image_new_from_pixbuf (scaled);
		g_object_unref (G_OBJECT (pixbuf));
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM
			(applications->applications), image);
		gtk_widget_show (image);
	}
}
