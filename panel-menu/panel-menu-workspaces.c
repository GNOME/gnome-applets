/*  panel-menu-workspaces.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libbonobo.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>
#include <libwnck/libwnck.h>

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-workspaces.h"

static const gchar *workspaces_menu_xml =
	"    <placeholder name=\"ChildItem\">\n"
	"        <menuitem name=\"Remove\" verb=\"Remove\" label=\"Remove Workspaces Menu\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-close\"/>\n"
	"        <separator/>"
	"    </placeholder>";

typedef struct _PanelMenuWorkspaces {
	GtkWidget *workspaces;
	GtkWidget *menu;
	/* The screen we're attached to */
	WnckScreen *screen;
	/* Signals we need to keep track of */
	gulong active_changed;
	gulong workspace_created;
	gulong workspace_destroyed;
} PanelMenuWorkspaces;

/* get/remove the workspace items from the menu */
static void fill_workspace_menu (GtkMenuShell *menu);
static void kill_workspace_menu (GtkMenuShell *menu);

/* Set the current workspace from a menu-item click */
static void handle_workspace_change (GtkWidget *menuitem, int number);

/* libwnck action */
static void setup_workspace_signals (PanelMenuWorkspaces *workspaces);
static void wnck_active_workspace_changed (WnckScreen *screen,
					   GtkMenuShell *menu);
static void wnck_workspace_added (WnckScreen *screen,
				  WnckWorkspace *workspace,
				  GtkMenuShell *menu);
static void wnck_workspace_removed (WnckScreen *screen,
				    WnckWorkspace *workspace,
				    GtkMenuShell *menu);

PanelMenuEntry *
panel_menu_workspaces_new (PanelMenu *parent)
{
	PanelMenuEntry *entry;
	PanelMenuWorkspaces *workspaces;
	GtkWidget *tearoff;

	entry = g_new0 (PanelMenuEntry, 1);
	entry->type = PANEL_MENU_TYPE_WORKSPACES;
	entry->parent = parent;
	workspaces = g_new0 (PanelMenuWorkspaces, 1);
	entry->data = (gpointer) workspaces;
	workspaces->workspaces = gtk_menu_item_new_with_label (_("Workspaces"));
	panel_menu_common_widget_dnd_init (entry);
	gtk_widget_show (workspaces->workspaces);
	workspaces->menu = gtk_menu_new ();

	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (workspaces->menu), tearoff);
	if (parent->menu_tearoffs) {
		gtk_widget_show (tearoff);
	}

	gtk_menu_item_set_submenu (GTK_MENU_ITEM (workspaces->workspaces),
				   workspaces->menu);
	fill_workspace_menu (GTK_MENU_SHELL (workspaces->menu));
	setup_workspace_signals (workspaces);
	return (entry);
}

void
panel_menu_workspaces_merge_ui (PanelMenuEntry *entry)
{
	BonoboUIComponent  *component;
	gchar *xml;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_WORKSPACES);

	component = panel_applet_get_popup_component (entry->parent->applet);

	bonobo_ui_component_add_verb (component, "Remove",
				     (BonoboUIVerbFn)panel_menu_common_remove_entry, entry);
	bonobo_ui_component_set (component, "/popups/button3/ChildMerge/",
				 workspaces_menu_xml, NULL);
}

void
panel_menu_workspaces_destroy (PanelMenuEntry *entry)
{
	PanelMenuWorkspaces *workspaces;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_WORKSPACES);

	workspaces = (PanelMenuWorkspaces *) entry->data;
	g_signal_handler_disconnect (workspaces->screen,
				     workspaces->active_changed);
	g_signal_handler_disconnect (workspaces->screen,
				     workspaces->workspace_created);
	g_signal_handler_disconnect (workspaces->screen,
				     workspaces->workspace_destroyed);
	gtk_widget_destroy (workspaces->workspaces);
	g_free (workspaces);
}

GtkWidget *
panel_menu_workspaces_get_widget (PanelMenuEntry *entry)
{
	PanelMenuWorkspaces *workspaces;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_WORKSPACES, NULL);

	workspaces = (PanelMenuWorkspaces *) entry->data;
	return (workspaces->workspaces);
}

static void
fill_workspace_menu (GtkMenuShell *menu)
{
	WnckScreen *screen;
	WnckWorkspace *workspace;
	WnckWorkspace *active;
	int spaces;
	int count;
	gchar *string;
	const gchar *name;
	GtkWidget *menuitem;
	GSList *group = NULL;

	screen = wnck_screen_get (0);
	spaces = wnck_screen_get_workspace_count (screen);
	active = wnck_screen_get_active_workspace (screen);
	for (count = 0; count < spaces; count++) {
		workspace = wnck_workspace_get (count);
		/* string = g_strdup_printf("Workspace %d", count+1); */
		name = wnck_workspace_get_name (workspace);
		menuitem = gtk_radio_menu_item_new_with_label (group, name);
		if (active == workspace)
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
							(menuitem), TRUE);
		gtk_widget_show (menuitem);
		/* g_free(string); */
		gtk_menu_shell_append (menu, menuitem);
		g_signal_connect (G_OBJECT (menuitem), "activate",
				  G_CALLBACK (handle_workspace_change),
				  GINT_TO_POINTER (count));
		group = GTK_RADIO_MENU_ITEM (menuitem)->group;
	}
}

static void
kill_workspace_menu (GtkMenuShell *menu)
{
	GList *list;
	GList *cur;

	if (menu->children) {
		list = g_list_copy (menu->children);
		for (cur = list->next; cur; cur = cur->next) {
			gtk_container_remove (GTK_CONTAINER (menu),
					      GTK_WIDGET (cur->data));
		}
		g_list_free (list);
	}
}

static void
handle_workspace_change (GtkWidget *menuitem, int number)
{
	WnckScreen *screen;
	WnckWorkspace *active;
	WnckWorkspace *workspace;

	screen = wnck_screen_get (0);
	active = wnck_screen_get_active_workspace (screen);
	workspace = wnck_workspace_get (number);
	if (workspace && workspace != active
	    && GTK_CHECK_MENU_ITEM (menuitem)->active) {
		wnck_workspace_activate (workspace);
	}
}

static void
setup_workspace_signals (PanelMenuWorkspaces *workspaces)
{
	WnckScreen *screen;

	screen = wnck_screen_get (0);
	workspaces->screen = screen;
	workspaces->active_changed =
		g_signal_connect (G_OBJECT (screen), "active_workspace_changed",
				  G_CALLBACK (wnck_active_workspace_changed),
				  workspaces->menu);
	workspaces->workspace_created =
		g_signal_connect (G_OBJECT (screen), "workspace_created",
				  G_CALLBACK (wnck_workspace_added),
				  workspaces->menu);
	workspaces->workspace_destroyed =
		g_signal_connect (G_OBJECT (screen), "workspace_destroyed",
				  G_CALLBACK (wnck_workspace_removed),
				  workspaces->menu);
}

static void
wnck_active_workspace_changed (WnckScreen *screen, GtkMenuShell *menu)
{
	WnckWorkspace *active;
	GList *cur;
	int active_num;
	int count;

	active = wnck_screen_get_active_workspace (screen);
	active_num = wnck_workspace_get_number (active);
	/* Add one to skip the tearoff item */
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
					(g_list_nth_data
					 (menu->children, active_num + 1)),
					TRUE);
}

static void
wnck_workspace_added (WnckScreen *screen, WnckWorkspace *workspace,
		      GtkMenuShell *menu)
{
	WnckWorkspace *active = NULL;
	GtkWidget *menuitem = NULL;
	GSList *group = NULL;
	gchar *string = NULL;
	const gchar *name;
	int count = 0;

	if (menu->children && (g_list_length (menu->children) > 1)
	    && (menuitem =
		g_list_nth_data (menu->children,
				 g_list_length (menu->children) - 1))) {
		group = GTK_RADIO_MENU_ITEM (menuitem)->group;
	}
	active = wnck_screen_get_active_workspace (screen);
	count = wnck_workspace_get_number (workspace);
	/* string = g_strdup_printf("Workspace %d", count+1); */
	name = wnck_workspace_get_name (workspace);
	menuitem = gtk_radio_menu_item_new_with_label (group, name);
	if (active == workspace)
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem),
						TRUE);
	gtk_widget_show (menuitem);
	/* g_free(string); */
	gtk_menu_shell_append (menu, menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate",
			  G_CALLBACK (handle_workspace_change),
			  GINT_TO_POINTER (count));
}

static void
wnck_workspace_removed (WnckScreen *screen, WnckWorkspace *workspace,
			GtkMenuShell *menu)
{
	kill_workspace_menu (menu);
	fill_workspace_menu (menu);
}

gchar *
panel_menu_workspaces_save_config (PanelMenuEntry *entry)
{
	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_WORKSPACES);

	return g_strdup ("workspaces");
}
