/*  panel-menu.c
 * (c) 2001 Chris Phelps
 * Menubar based Panel Applet
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
#include <bonobo/bonobo-shlib-factory.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <libwnck/libwnck.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-init.h>
#include <libgnomevfs/gnome-vfs-directory.h>
#include <libgnomevfs/gnome-vfs-mime-utils.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-handle.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-result.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include "panel-menu-common.h"
#include "panel-menu-applications.h"
#include "panel-menu-path.h"
#include "panel-menu-links.h"
#include "panel-menu-directory.h"
#include "panel-menu-documents.h"
#include "panel-menu-actions.h"
#include "panel-menu-windows.h"
#include "panel-menu-workspaces.h"
#include "panel-menu-properties.h"
#include "panel-menu-add.h"
#include "panel-menu-config.h"
#include "panel-menu.h"

static gboolean applet_factory (PanelApplet *applet, const gchar *iid,
				gpointer data);
static gboolean construct_applet (PanelApplet *applet);

static void applet_change_background (PanelApplet *applet,
				      PanelAppletBackgroundType type,
				      GdkColor *color, GdkPixmap *pixmap,
				      PanelMenu *panel_menu);
static void panel_menu_background_restore (PanelMenu *panel_menu);

static gboolean panel_menu_id_from_key (gchar *key, gchar **profile,
					gchar **applet);

static gboolean panel_menu_drag_motion (GtkWidget *widget,
					GdkDragContext *context, gint x,
					gint y, guint time,
					PanelMenu *panel_menu);
static void panel_menu_drag_leave (GtkWidget *widget,
				   GdkDragContext *context, guint time,
				   PanelMenu *panel_menu);
static void panel_menu_drag_data_received (GtkWidget *widget,
					   GdkDragContext *context, gint x,
					   gint y,
					   GtkSelectionData *
					   selection_data, guint info,
					   guint time, PanelMenu *panel_menu);
static gboolean panel_menu_pass_drop (PanelMenu *panel_menu,
				      GnomeVFSURI *uri);
static gboolean panel_menu_accept_entry_drop (PanelMenu *panel_menu,
					      PanelMenuEntry *entry);

static gint applet_button_press_cb (GtkWidget *widget, GdkEventButton *event,
				    PanelMenu *panel_menu);
static gint applet_destroy_cb (GtkWidget *widget, PanelMenu *panel_menu);
static void applet_change_orientation_cb (PanelApplet *applet,
					  PanelAppletOrient orient,
					  PanelMenu *panel_menu);
static void applet_change_size_cb (PanelApplet *applet, gint size,
				   PanelMenu *panel_menu);
static void applet_help_cb (BonoboUIComponent *uic, PanelMenu *panel_menu,
			    const gchar *verbname);
static void applet_about_cb (BonoboUIComponent *uic,
			     PanelMenu *panel_menu, const gchar *verbname);

enum {
	TARGET_URI_LIST,
	TARGET_PANEL_MENU_ENTRY
};

static GtkTargetEntry drop_types[] = {
	{"text/uri-list", 0, TARGET_URI_LIST},
	{"application/panel-menu-entry", GTK_TARGET_SAME_APP,
	 TARGET_PANEL_MENU_ENTRY}
};
static gint n_drop_types = sizeof (drop_types) / sizeof (GtkTargetEntry);

static const BonoboUIVerb applet_menu_verbs[] = {
	BONOBO_UI_UNSAFE_VERB ("Add", applet_add_cb),
	BONOBO_UI_UNSAFE_VERB ("Properties", applet_properties_cb),
	BONOBO_UI_UNSAFE_VERB ("Help", applet_help_cb),
	BONOBO_UI_UNSAFE_VERB ("About", applet_about_cb),
	BONOBO_UI_VERB_END
};

static const char applet_menu_xml[] =
	"<popup name=\"button3\">\n"
	"   <placeholder name=\"ChildMerge\"/>\n"
	"   <menuitem name=\"Add\" verb=\"Add\" _label=\"Add...\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-add\"/>\n"
	"   <menuitem name=\"Properties Item\" verb=\"Properties\" _label=\"Properties ...\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
	"   <menuitem name=\"Help Item\" verb=\"Help\" _label=\"Help\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
	"   <menuitem name=\"About Item\" verb=\"About\" _label=\"About ...\"\n"
	"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
	"</popup>\n";


#if MENU_APPLET_IS_SHLIB
PANEL_APPLET_BONOBO_SHLIB_FACTORY ("OAFIID:GNOME_PanelMenuApplet_Factory",
				   "PanelMenu-Applet-Factory",
				   applet_factory, NULL);
#else
PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_PanelMenuApplet_Factory",
                             "PanelMenu-Applet-Factory",
                             "0",
                             applet_factory,
                             NULL)
#endif

static gboolean
applet_factory (PanelApplet *applet, const gchar *iid, gpointer data)
{
	gboolean retval = FALSE;

	if (!strcmp (iid, "OAFIID:GNOME_PanelMenuApplet"))
		retval = panel_menu_construct_applet (applet);

	return retval;
}

gboolean
panel_menu_construct_applet (PanelApplet *applet)
{
	PanelMenu *panel_menu;
	PanelMenuEntry *entry;
	gchar *key;

	panel_menu = g_new0 (PanelMenu, 1);

	panel_menu->applet = applet;
	panel_applet_set_expand_flags (panel_menu->applet, FALSE, FALSE);
	panel_menu->size = panel_applet_get_size (panel_menu->applet);

	key = panel_applet_get_preferences_key (applet);
	panel_menu_id_from_key (key, &panel_menu->profile_id,
				&panel_menu->applet_id);
	g_free (key);

	panel_applet_add_preferences (applet,
				      "/schemas/apps/panel-menu-applet/prefs",
				      NULL);
	panel_menu_config_load_prefs (panel_menu);

	gtk_rc_parse_string ("style \"no_shadow\"\n{\nGtkMenuBar::shadow_type = GTK_SHADOW_NONE\n}\nwidget \"*.PanelMenubarApplet\" style \"no_shadow\"\n");

	panel_menu->menubar = gtk_menu_bar_new ();
	gtk_widget_set_name (panel_menu->menubar, "PanelMenubarApplet");

	panel_menu_config_load_layout (panel_menu);

	gtk_drag_dest_set (panel_menu->menubar, GTK_DEST_DEFAULT_ALL,
			   drop_types, n_drop_types,
			   GDK_ACTION_COPY | GDK_ACTION_LINK | GDK_ACTION_MOVE);
	g_signal_connect (G_OBJECT (panel_menu->menubar), "drag_motion",
			  G_CALLBACK (panel_menu_drag_motion), panel_menu);
	g_signal_connect (G_OBJECT (panel_menu->menubar), "drag_leave",
			  G_CALLBACK (panel_menu_drag_leave), panel_menu);
	g_signal_connect (G_OBJECT (panel_menu->menubar), "drag_data_received",
			  G_CALLBACK (panel_menu_drag_data_received),
			  panel_menu);
	g_signal_connect (G_OBJECT (panel_menu->menubar), "button_press_event",
			  G_CALLBACK (applet_button_press_cb), panel_menu);
	g_signal_connect (G_OBJECT (panel_menu->applet), "destroy",
			  G_CALLBACK (applet_destroy_cb), panel_menu);
	g_signal_connect (G_OBJECT (panel_menu->applet), "change_background",
			  G_CALLBACK (applet_change_background), panel_menu);
	g_signal_connect (G_OBJECT (panel_menu->applet), "change_orient",
			  G_CALLBACK (applet_change_orientation_cb),
			  panel_menu);
	g_signal_connect (G_OBJECT (panel_menu->applet), "change_size",
			  G_CALLBACK (applet_change_size_cb), panel_menu);

	panel_applet_setup_menu (panel_menu->applet, applet_menu_xml,
				 applet_menu_verbs, panel_menu);

	gtk_container_add (GTK_CONTAINER (panel_menu->applet),
			   panel_menu->menubar);
	gtk_widget_show (panel_menu->menubar);
	gtk_widget_show (GTK_WIDGET (panel_menu->applet));
	return TRUE;
}

static void
applet_change_background (PanelApplet *applet,
			  PanelAppletBackgroundType type, GdkColor *color,
			  GdkPixmap *pixmap, PanelMenu *panel_menu)
{
	g_print ("(change-background) called.\n");
	panel_menu->bg_type = type;
	panel_menu_background_restore (panel_menu);
	switch (type) {
	case PANEL_NO_BACKGROUND:
		break;
	case PANEL_COLOR_BACKGROUND:
		if (color)
			gtk_widget_modify_bg (panel_menu->menubar, GTK_STATE_NORMAL,
					      color);
		break;
	case PANEL_PIXMAP_BACKGROUND:
		if (panel_menu->bg_pixmap) {
			g_object_unref (G_OBJECT (panel_menu->bg_pixmap));
			panel_menu->bg_pixmap = NULL;
		}
		if (pixmap) {
			GtkStyle *new_style;

			panel_menu->bg_pixmap = pixmap;
			g_object_ref (G_OBJECT (panel_menu->bg_pixmap));
			
			new_style = gtk_style_copy(panel_menu->menubar->style);
			if(new_style->bg_pixmap[GTK_STATE_NORMAL])
				g_object_unref(G_OBJECT(new_style->bg_pixmap[GTK_STATE_NORMAL]));                
			new_style->bg_pixmap[GTK_STATE_NORMAL] = panel_menu->bg_pixmap;
			g_object_ref(G_OBJECT(new_style->bg_pixmap[GTK_STATE_NORMAL]));
			gtk_widget_set_style(panel_menu->menubar, new_style);
		}
		break;
	default:
		break;
	}
}

static void
panel_menu_background_restore (PanelMenu *panel_menu)
{
	GtkRcStyle *rc_style;
	g_return_if_fail (panel_menu != NULL);

	if (panel_menu->bg_pixmap) {
		g_object_unref (G_OBJECT (panel_menu->bg_pixmap));
		panel_menu->bg_pixmap = NULL;
	}

	rc_style = gtk_rc_style_new ();
	gtk_widget_modify_style (panel_menu->menubar, rc_style);
}

static gboolean
panel_menu_id_from_key (gchar *key, gchar **profile, gchar **applet)
{
	gchar *str;
	gchar *ptr;
	gint len;
	gboolean retval = FALSE;

	g_return_val_if_fail (key != NULL, FALSE);
	g_return_val_if_fail (profile != NULL, FALSE);
	g_return_val_if_fail (applet != NULL, FALSE);

	key = g_strdup (key);
	if (str = strstr (key, "profiles/")) {
		str += strlen ("profiles/");
		ptr = str + 1;
		while (*ptr != '/' && *ptr != '\0')
			ptr++;
		len = (ptr - str);
		*profile = g_new0 (char, len + 1);

		strncpy (*profile, str, len);
		g_print ("profile is %s\n", *profile);
		retval = TRUE;
	}
	if (str = strstr (key, "applets/")) {
		str += strlen ("applets/");
		ptr = str + 1;
		while (*ptr != '/' && *ptr != '\0')
			ptr++;
		len = (ptr - str);
		*applet = g_new0 (char, len + 1);

		strncpy (*applet, str, len);
		g_print ("applet is %s\n", *applet);
		retval = TRUE;
	}
	g_free (key);
	return retval;
}

static gboolean
panel_menu_drag_motion (GtkWidget *widget, GdkDragContext *context,
			gint x, gint y, guint time, PanelMenu *panel_menu)
{
	GList *list;
	GtkWidget *target;
	gint old_position;

	old_position = panel_menu->position;
	panel_menu->on_item = FALSE;
	panel_menu->position = 0;

	for (list = GTK_MENU_SHELL (widget)->children; list; list = list->next) {
		target = GTK_WIDGET (list->data);
		if (!GTK_WIDGET_VISIBLE (target)) {
			panel_menu->position++;
		} else if (x >= target->allocation.x
		    && x <= (target->allocation.x + target->allocation.width)) {
			panel_menu->on_item = TRUE;
			gtk_draw_focus (widget->style, widget->window,
					target->allocation.x + 1,
					target->allocation.y + 1,
					target->allocation.width - 2,
					target->allocation.height - 2);
			break;
		} else if (x <= target->allocation.x) {
			break;
		} else {
			panel_menu->position++;
		}
	}
	if (panel_menu->position != old_position) {
		target = g_list_nth_data (GTK_MENU_SHELL (widget)->children,
					  old_position);
		if (target)
			gtk_widget_queue_draw_area (widget,
						    target->allocation.x,
						    target->allocation.y,
						    target->allocation.width,
						    target->allocation.height);
	}
	return FALSE;
}

static void
panel_menu_drag_leave (GtkWidget *widget, GdkDragContext *context,
		       guint time, PanelMenu *panel_menu)
{
	gtk_widget_queue_draw (widget);
}

static void
panel_menu_drag_data_received (GtkWidget *widget, GdkDragContext *context,
			       gint x, gint y,
			       GtkSelectionData *selection_data,
			       guint info, guint time, PanelMenu *panel_menu)
{
	GList *list;
	GList *uris;
	GnomeVFSURI *uri;
	PanelMenuEntry *entry;

	if (info == TARGET_URI_LIST) {
		uris = gnome_vfs_uri_list_parse ((gchar *) selection_data->
						 data);
		for (list = uris; list; list = list->next) {
			uri = (GnomeVFSURI *) list->data;
			if (panel_menu->on_item) {
				panel_menu_pass_drop (panel_menu, uri);
			} else {
				panel_menu_accept_drop (panel_menu, uri);
			}
		}
		gnome_vfs_uri_list_free (uris);
	} else if (info == TARGET_PANEL_MENU_ENTRY) {
		entry = *((PanelMenuEntry **) selection_data->data);
		if (entry) {
			panel_menu_accept_entry_drop (panel_menu, entry);
		}
	}
	panel_menu->position = 0;
	panel_menu->on_item = FALSE;
}

static gboolean
panel_menu_pass_drop (PanelMenu *panel_menu, GnomeVFSURI *uri)
{
	PanelMenuEntry *entry = NULL;
	gboolean retval = FALSE;

	entry = g_list_nth_data (panel_menu->entries, panel_menu->position);
	if (!entry)
		return FALSE;

	switch (entry->type) {
	case PANEL_MENU_TYPE_PATH:
		retval = panel_menu_path_accept_drop (entry, uri);
		break;
	case PANEL_MENU_TYPE_LINKS:
		retval = panel_menu_links_accept_drop (entry, uri);
		break;
	case PANEL_MENU_TYPE_DIRECTORY:
		retval = panel_menu_directory_accept_drop (entry, uri);
		break;
	case PANEL_MENU_TYPE_DOCUMENTS:
		retval = panel_menu_documents_accept_drop (entry, uri);
		break;
	default:
		break;
	}
	return retval;
}

gboolean
panel_menu_accept_drop (PanelMenu *panel_menu, GnomeVFSURI *uri)
{
	PanelMenuEntry *entry;
	gchar *fileuri;
	GnomeVFSURI *pathuri;
	gint insert;
	gboolean retval = FALSE;

	g_return_val_if_fail (panel_menu != NULL, FALSE);
	g_return_val_if_fail (uri != NULL, FALSE);

	fileuri = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);
	if (!strncmp (fileuri, "applications:", strlen ("applications:")) ||
	    !strncmp (fileuri, "file:", strlen ("file:"))) {
		if (strstr (fileuri, ".desktop")) {
			g_free (fileuri);
			pathuri = gnome_vfs_uri_get_parent (uri);
			fileuri =
				gnome_vfs_uri_to_string (pathuri,
							 GNOME_VFS_URI_HIDE_NONE);
			gnome_vfs_uri_unref (pathuri);
		}
		insert = panel_menu->position;
		entry = panel_menu_path_new (panel_menu, fileuri);
		panel_menu->entries =
			g_list_insert (panel_menu->entries, (gpointer) entry,
				       insert);
		gtk_menu_shell_insert (GTK_MENU_SHELL (panel_menu->menubar),
				       panel_menu_path_get_widget (entry),
				       insert);
		panel_menu_config_save_prefs (panel_menu);
		retval = TRUE;
	}
	g_free (fileuri);
	return retval;
}

static gboolean
panel_menu_accept_entry_drop (PanelMenu *panel_menu, PanelMenuEntry *entry)
{
	gboolean accept = FALSE;
	GtkWidget *menuitem;
	gint oldposition;

	if (entry->parent != panel_menu) {
		switch (entry->type) {
		case PANEL_MENU_TYPE_ACTIONS:
			if (!panel_menu->has_actions) {
				panel_menu->has_actions = TRUE;
				entry->parent->has_actions = FALSE;
				accept = TRUE;
			}
			break;
		case PANEL_MENU_TYPE_WINDOWS:
			if (!panel_menu->has_windows) {
				panel_menu->has_windows = TRUE;
				entry->parent->has_windows = FALSE;
				accept = TRUE;
			}
			break;
		case PANEL_MENU_TYPE_WORKSPACES:
			if (!panel_menu->has_workspaces) {
				panel_menu->has_workspaces = TRUE;
				entry->parent->has_workspaces = FALSE;
				accept = TRUE;
			}
			break;
		default:
			break;
		}
	} else {
		accept = TRUE;
	}
	if (!accept) {
		panel_menu = entry->parent;
		menuitem = panel_menu_common_get_entry_menuitem (entry);
		oldposition =
			(gint) g_object_get_data (G_OBJECT (menuitem),
						  "old-position");
		gtk_menu_shell_insert (GTK_MENU_SHELL (panel_menu->menubar),
				       menuitem, oldposition);
		g_object_unref (G_OBJECT (menuitem));
		panel_menu->entries =
			g_list_insert (panel_menu->entries, (gpointer) entry,
				       oldposition);
	} else {
		if (panel_menu != entry->parent && entry->parent) {
			panel_menu_common_call_entry_remove_config (entry);
			panel_menu_config_save_layout (entry->parent);
			entry->parent = panel_menu;
		}
		menuitem = panel_menu_common_get_entry_menuitem (entry);
		gtk_menu_shell_insert (GTK_MENU_SHELL (panel_menu->menubar), menuitem,
				       panel_menu->position);
		g_object_unref (G_OBJECT (menuitem));
		panel_menu->entries =
			g_list_insert (panel_menu->entries, (gpointer) entry,
				       panel_menu->position);
		/* And now, sync the prefs */
		panel_menu_config_save_layout (panel_menu);
		panel_menu_common_call_entry_save_config (entry);
	}
	return accept;
}

static gint
applet_button_press_cb (GtkWidget *widget, GdkEventButton *event,
			PanelMenu *panel_menu)
{
	PanelApplet *applet;

	applet = panel_menu->applet;

	if (event->button == 1) {
		return FALSE;
	} else if (event->button == 3) {
		GList *list;
		GtkWidget *target;
		gint position = 0;
		gboolean on_item = FALSE;
		gint x, y, window_x, window_y;
		gdk_window_get_position (event->window, &window_x, &window_y);
		x = window_x + event->x;
		y = window_y + event->y;
		for (list = GTK_MENU_SHELL (panel_menu->menubar)->children; list; list = list->next)
		{
			target = GTK_WIDGET (list->data);
			if (x < target->allocation.x) {
				break;
			} else if (x >= target->allocation.x && x <= target->allocation.x + target->allocation.width) {
				on_item = TRUE;
				break;
			} else {
				position++;
			}
		}
		panel_menu_common_demerge_ui (panel_menu->applet);
		if (on_item) {
			/* merge the items from the child item here */
			PanelMenuEntry *entry;
			entry = g_list_nth_data (panel_menu->entries, position);
			panel_menu_common_merge_entry_ui (entry);
		}
		GTK_WIDGET_CLASS (PANEL_APPLET_GET_CLASS (applet))->
			button_press_event (GTK_WIDGET(applet), event);
	}
	return TRUE;
}

static gint
applet_destroy_cb (GtkWidget *widget, PanelMenu *panel_menu)
{
	PanelMenuEntry *entry;
	GList *cur;

	g_free (panel_menu->profile_id);
	g_free (panel_menu->applet_id);

	if (panel_menu->tearoffs_id) {
		gconf_client_notify_remove (panel_menu->client,
					    panel_menu->tearoffs_id);
	}
	g_object_unref (G_OBJECT (panel_menu->client));

	for (cur = panel_menu->entries; cur; cur = cur->next) {
		entry = (PanelMenuEntry *) cur->data;
		panel_menu_common_call_entry_destroy (entry);
	}
	if (panel_menu->entries)
		g_list_free (panel_menu->entries);
	g_free (panel_menu);
}

static void
applet_change_orientation_cb (PanelApplet *applet,
			      PanelAppletOrient orient, PanelMenu *panel_menu)
{
	GtkOrientation new_orient;

	switch (orient) {
	case PANEL_APPLET_ORIENT_LEFT:
	case PANEL_APPLET_ORIENT_RIGHT:
		new_orient = GTK_ORIENTATION_VERTICAL;
		break;
	case PANEL_APPLET_ORIENT_UP:
	case PANEL_APPLET_ORIENT_DOWN:
	default:
		new_orient = GTK_ORIENTATION_HORIZONTAL;
		break;
	}

	if (new_orient != panel_menu->orientation) {
		panel_menu->orientation = new_orient;
	}
	/* FIXME: Re-pack the menubar? */
}


static void
applet_change_size_cb (PanelApplet *applet, gint size, PanelMenu *panel_menu)
{
  printf ("RECEIVED SIZE SIGNAL!!!\n\nsize specified was %d\n\n\n", size);
	if (panel_menu->size != size) {
		panel_menu->size = size;
		if (panel_menu->has_applications) {
			panel_menu_applications_rescale_icon (
				panel_menu_common_find_applications (panel_menu));
		}
	}
}

static void
applet_help_cb (BonoboUIComponent *uic, PanelMenu *panel_menu,
		const gchar *verbname)
{
	/* FIXME: Implement this */
}

static void
applet_about_cb (BonoboUIComponent *uic, PanelMenu *panel_menu,
		 const gchar *verbname)
{
	static GtkWidget *about = NULL;
	GdkPixbuf *pixbuf = NULL;
	gchar *file = NULL;

	static const gchar *authors[] = {
		"Chris Phelps <chicane@reninet.com>",
		NULL
	};

	if (about != NULL) {
		gtk_widget_show (about);
		gtk_window_present (GTK_WINDOW (about));
		return;
	}

	pixbuf = NULL;

	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
					  "gnome-logo-icon-transparent.png", TRUE, NULL);
	if (!file) {
		g_warning (G_STRLOC ": icon was not found cannot be found");
	} else {
		pixbuf = gdk_pixbuf_new_from_file (file, NULL);
	}

	about = gnome_about_new (_("PanelMenu Applet"), "0.0.1", _("(c) 2001 Chris Phelps"), _("The Panel Menu Applet allows you to display customized menubars on your panels."), authors, NULL,	/* documenters */
				 NULL,	/* translator_credits */
				 pixbuf);

	gtk_window_set_wmclass (GTK_WINDOW (about), "panel-menu", "PanelMenu");
	if (pixbuf) {
		gtk_window_set_icon (GTK_WINDOW (about), pixbuf);
	}
	g_signal_connect (G_OBJECT (about), "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &about);
	gtk_widget_show (about);
}
