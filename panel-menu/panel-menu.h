/*  panel-menu.h
 *  (c) 2001 Chris Phelps
 *  Menubar based Panel Applet
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

#ifndef _PANEL_MENU_H_
#define _PANEL_MENU_H_

#include <glib.h>
#include <gtk/gtk.h>
#include <panel-applet.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

G_BEGIN_DECLS

#define MENU_APPLET_IS_SHLIB 1

typedef enum
{
	PANEL_MENU_TYPE_APPLICATIONS,
	PANEL_MENU_TYPE_PATH,
	PANEL_MENU_TYPE_LINKS,
	PANEL_MENU_TYPE_DIRECTORY,
	PANEL_MENU_TYPE_DOCUMENTS,
	PANEL_MENU_TYPE_ACTIONS,
	PANEL_MENU_TYPE_WINDOWS,
	PANEL_MENU_TYPE_WORKSPACES,
	PANEL_MENU_TYPE_PREFERENCES
}PanelMenuEntryType;

typedef struct _PanelMenu
{
	PanelApplet *applet;
	GtkWidget *menubar;
	/* Information that we keep around */
	GtkOrientation orientation;
	gint size;
        gboolean menu_tearoffs;
	/* Background info */
	PanelAppletBackgroundType bg_type;
	GdkPixmap *bg_pixmap;
	/* For session loading/saving */
	gchar *profile_id;
	gchar *applet_id;
	/* Layout */
	gboolean has_applications;
        gboolean has_preferences;
	gboolean has_actions;
	gboolean has_windows;
	gboolean has_workspaces;
	/* GConf client that listens to some keys for us */
	GConfClient *client;
	guint tearoffs_id;
	/* DnD data */
	gint position;
	gboolean on_item;
	/* List of our children */
	GList *entries;
}PanelMenu;

typedef struct _PanelMenuEntry
{
	PanelMenuEntryType type;
	PanelMenu *parent;
	gpointer data;
}PanelMenuEntry;

gboolean panel_menu_construct_applet(PanelApplet *applet);

gboolean panel_menu_accept_drop(PanelMenu *panel_menu, GnomeVFSURI *uri);

void add_tooltip (GtkWidget *widget, gchar *description);
void destroy_tooltip (GtkWidget *object);
void set_relation (GtkWidget *widget, GtkLabel *label);
G_END_DECLS

#endif
