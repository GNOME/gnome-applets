/* panel-menu-actions.c
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
#include <libwnck/libwnck.h>

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-actions.h"

static const gchar *actions_menu_xml =
	"    <placeholder name=\"ChildItem\">\n"
	"        <menuitem name=\"Remove\" verb=\"Remove\" label=\"Remove Actions\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-close\"/>\n"
	"        <separator/>"
	"    </placeholder>";

typedef struct _PanelMenuActions {
	GtkWidget *actions;
	GtkWidget *menu;
} PanelMenuActions;

typedef enum _ActionItemType {
	ACTION_ITEM_TYPE_RUN,
	ACTION_ITEM_TYPE_LOGOUT,
	ACTION_ITEM_TYPE_LOCK,
	ACTION_ITEM_TYPE_SHOW_DESKTOP
} ActionItemType;

typedef void (*ActionItemConstructor) (GtkWidget *widget);

typedef struct _ActionItem {
	ActionItemType type;
	gchar *name;
	gchar *path;
	gchar *image;
	ActionItemConstructor constructor;
} ActionItem;

static GtkWidget *construct_from_uri (gchar *uri);

static void construct_run_item (GtkWidget *widget);
static void construct_logout_item (GtkWidget *widget);
static void construct_lock_item (GtkWidget *widget);
static void construct_show_desktop_item (GtkWidget *widget);
static void show_desktop_cb (GtkWidget *widget, gpointer data);

static ActionItem action_items[] = {
	{ACTION_ITEM_TYPE_RUN, "Run...", "action:run",
	 DATADIR "/pixmaps/gnome-run.png", construct_run_item},
	{ACTION_ITEM_TYPE_LOGOUT, "Logout", "action:logout",
	 DATADIR "/pixmaps/gnome-term-night.png", construct_logout_item},
	{ACTION_ITEM_TYPE_LOCK, "Lock Screen", "action:lock",
	 DATADIR "/pixmaps/gnome-lockscreen.png", construct_lock_item},
	{ACTION_ITEM_TYPE_SHOW_DESKTOP, "Show Desktop", "action:show-desktop",
	 DATADIR "/pixmaps/gnome-ccdesktop.png", construct_show_desktop_item}
};

static gint n_actions = sizeof (action_items) / sizeof (ActionItem);

PanelMenuEntry *
panel_menu_actions_new (PanelMenu *parent)
{
	PanelMenuEntry *entry;
	PanelMenuActions *actions;
	GtkWidget *tearoff;
	GtkWidget *image;
	gint counter;

	entry = g_new0 (PanelMenuEntry, 1);
	entry->type = PANEL_MENU_TYPE_ACTIONS;
	entry->parent = parent;
	actions = g_new0 (PanelMenuActions, 1);
	entry->data = (gpointer) actions;
	actions->actions = gtk_menu_item_new_with_label (_("Actions"));
	panel_menu_common_widget_dnd_init (entry);
	gtk_widget_show (actions->actions);
	actions->menu = gtk_menu_new ();

	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (actions->menu), tearoff);
	if (parent->menu_tearoffs) {
		gtk_widget_show (tearoff);
	}

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (actions->actions),
				   actions->menu);
	for (counter = 0; counter < n_actions; counter++) {
		GtkWidget *widget;
		widget = construct_from_uri (action_items[counter].path);
		gtk_menu_shell_append (GTK_MENU_SHELL (actions->menu), widget);
		gtk_widget_show (widget);
	}
	return entry;
}

void
panel_menu_actions_merge_ui (PanelMenuEntry *entry)
{
	PanelMenuActions *actions;
	BonoboUIComponent *component;
	gchar *xml;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS);

	actions = (PanelMenuActions *) entry->data;
	component = panel_applet_get_popup_component (entry->parent->applet);

	bonobo_ui_component_add_verb (component, "Remove",
				     (BonoboUIVerbFn) panel_menu_common_remove_entry, entry);
	bonobo_ui_component_set (component, "/popups/button3/ChildMerge/",
				 actions_menu_xml, NULL);
}

void
panel_menu_actions_destroy (PanelMenuEntry *entry)
{
	PanelMenuActions *actions;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS);

	actions = (PanelMenuActions *) entry->data;
	gtk_widget_destroy (actions->actions);
	g_free (actions);
}

GtkWidget *
panel_menu_actions_get_widget (PanelMenuEntry *entry)
{
	PanelMenuActions *actions;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS);

	actions = (PanelMenuActions *) entry->data;
	return actions->actions;
}

static GtkWidget *
construct_from_uri (gchar *uri)
{
	GtkWidget *widget = NULL;
	int counter;

	for (counter = 0; counter < n_actions; counter++) {
		if (!strcmp (uri, action_items[counter].path)) {
			widget = gtk_image_menu_item_new_with_label
				(action_items[counter].name);
			panel_menu_pixbuf_set_icon (GTK_MENU_ITEM (widget),
						    action_items[counter].image);
			g_object_set_data (G_OBJECT (widget), "uri-path",
					   action_items[counter].path);
			gtk_widget_show (widget);
			action_items[counter].constructor (widget);
			break;
		}
	}
	return widget;
}

static void
show_run_dialog_cb (GtkWidget *widget, gpointer data)
{
/*
	Bonobo_Unknown *panel;
	bonobo_get_object
*/
}

static void
construct_run_item (GtkWidget *widget)
{
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (show_run_dialog_cb), NULL);
}

static void
construct_logout_item (GtkWidget *widget)
{

}

static void
construct_lock_item (GtkWidget *widget)
{

}

static void
construct_show_desktop_item (GtkWidget *widget)
{
	g_signal_connect (G_OBJECT (widget), "activate",
			  G_CALLBACK (show_desktop_cb), NULL);
}

static void
show_desktop_cb (GtkWidget *widget, gpointer data)
{
	WnckWorkspace *active;
	GList *windows;
	WnckWindow *win;

	active = wnck_screen_get_active_workspace (wnck_screen_get (0));
	for (windows = wnck_screen_get_windows (wnck_screen_get (0)); windows;
	     windows = windows->next) {
		win = (WnckWindow *) windows->data;
		if ((active == wnck_window_get_workspace (win))
		    && !wnck_window_is_minimized (win)) {
			wnck_window_minimize (win);
		}
	}
}

gchar *
panel_menu_actions_save_config (PanelMenuEntry *entry)
{
	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS);

	return g_strdup ("actions");
}
