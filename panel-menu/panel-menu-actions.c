/*  panel-menu-actions.c
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
	"        <menuitem name=\"Action\" verb=\"Action\" label=\"%s Properties...\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
	"        <menuitem name=\"Remove\" verb=\"Remove\" label=\"Remove %s\"\n"
	"                  pixtype=\"stock\" pixname=\"gtk-close\"/>\n"
	"        <separator/>"
	"    </placeholder>";

typedef struct _PanelMenuActions {
	GtkWidget *checkitem;
	GtkWidget *actions;
	GtkWidget *menu;
	gchar *name;
	GList *items_list;
	GList *actions_list;
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

static void set_visibility (GtkCheckMenuItem *checkitem, GtkWidget *target);
static gboolean check_valid_uri (gchar *uri);
static GtkWidget *construct_from_uri (gchar *uri);

static void construct_run_item (GtkWidget *widget);
static void construct_logout_item (GtkWidget *widget);
static void construct_lock_item (GtkWidget *widget);
static void construct_show_desktop_item (GtkWidget *widget);
static void show_desktop_cb (GtkWidget *widget, gpointer data);

static void actions_properties_cb (GtkWidget *widget, PanelMenuEntry *entry,
				   const gchar *verb);
static void handle_response (GtkWidget *dialog, gint response,
			     GtkWidget *name_entry);
static void button_clicked_append_item (GtkWidget *button,
					PanelMenuEntry *entry);
static void action_item_dnd_init (GtkWidget *button);
static void action_item_dnd_drag_begin_cb (GtkWidget *button,
					   GdkDragContext *context);
static void action_item_dnd_set_data_cb (GtkWidget *button,
					 GdkDragContext *context,
					 GtkSelectionData *selection_data,
					 guint info, guint time);

enum {
	TARGET_URI_LIST,
};

static GtkTargetEntry drag_types[] = {
	{"text/uri-list", 0, TARGET_URI_LIST}
};
static gint n_drag_types = sizeof (drag_types) / sizeof (GtkTargetEntry);

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
panel_menu_actions_new (PanelMenu *parent, gchar *name)
{
	PanelMenuEntry *entry;
	PanelMenuActions *actions;
	GtkWidget *tearoff;
	GtkWidget *image;

	entry = g_new0 (PanelMenuEntry, 1);
	entry->type = PANEL_MENU_TYPE_ACTIONS;
	entry->parent = parent;
	actions = g_new0 (PanelMenuActions, 1);
	entry->data = (gpointer) actions;
	actions->name = g_strdup (name);
	actions->actions = gtk_menu_item_new_with_label ("");
	panel_menu_common_widget_dnd_init (entry);
	gtk_widget_show (actions->actions);
	actions->checkitem = gtk_check_menu_item_new_with_label ("");
	panel_menu_actions_set_name (entry, (const gchar *)name);
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
					(actions->checkitem), TRUE);
	g_signal_connect (G_OBJECT (actions->checkitem), "toggled",
			  G_CALLBACK (panel_menu_common_set_visibility),
			  actions->actions);
	actions->menu = gtk_menu_new ();
	tearoff = gtk_tearoff_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (actions->menu), tearoff);
	gtk_widget_show (tearoff);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (actions->actions),
				   actions->menu);
	return entry;
}

void
panel_menu_actions_set_name (PanelMenuEntry *entry, const gchar *name)
{
	PanelMenuActions *actions;
	gchar *show_label;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS);

	actions = (PanelMenuActions *) entry->data;
	if (actions->name)
		g_free (actions->name);
	actions->name = name ? g_strdup (name) : g_strdup ("");
	gtk_label_set_text (GTK_LABEL (GTK_BIN (actions->actions)->child),
			    name);
	show_label = g_strconcat ("Show ", actions->name, NULL);
	gtk_label_set_text (GTK_LABEL (GTK_BIN (actions->checkitem)->child),
			    show_label);
	g_free (show_label);
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

	bonobo_ui_component_add_verb (component, "Action",
				     (BonoboUIVerbFn) actions_properties_cb, entry);
	bonobo_ui_component_add_verb (component, "Remove",
				     (BonoboUIVerbFn) panel_menu_common_remove_entry, entry);
	xml = g_strdup_printf (actions_menu_xml, actions->name, actions->name);
	bonobo_ui_component_set (component, "/popups/button3/ChildMerge/",
				 xml, NULL);
	g_free (xml);
}

void
panel_menu_actions_destroy (PanelMenuEntry *entry)
{
	PanelMenuActions *actions;
	GList *cur;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS);

	actions = (PanelMenuActions *) entry->data;
	if (actions->name)
		g_free (actions->name);
	for (cur = actions->actions_list; cur; cur = cur->next) {
		if (cur->data)
			g_free (cur->data);
	}
	gtk_widget_destroy (actions->checkitem);
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

GtkWidget *
panel_menu_actions_get_checkitem (PanelMenuEntry *entry)
{
	PanelMenuActions *actions;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS, NULL);

	actions = (PanelMenuActions *) entry->data;
	return actions->checkitem;
}

gchar *
panel_menu_actions_dump_xml (PanelMenuEntry *entry)
{
	PanelMenuActions *actions;
	GString *string;
	GList *cur;
	gchar *str;
	gboolean visible;

	g_return_val_if_fail (entry != NULL, NULL);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS, NULL);

	actions = (PanelMenuActions *) entry->data;
	visible = GTK_CHECK_MENU_ITEM (actions->checkitem)->active;
	string = g_string_new ("    <actions-item>\n");
	g_string_append (string, "        <name>");
	g_string_append (string, actions->name);
	g_string_append (string, "</name>\n");

	for (cur = actions->actions_list; cur; cur = cur->next) {
		g_string_append (string, "        <action>");
		g_string_append (string, (gchar *) cur->data);
		g_string_append (string, "</action>\n");
	}
	g_string_append (string, "        <visible>");
	g_string_append (string, visible ? "true" : "false");
	g_string_append (string, "</visible>\n");
	g_string_append (string, "    </actions-item>\n");
	str = string->str;
	g_string_free (string, FALSE);
	return str;
}

gboolean
panel_menu_actions_accept_drop (PanelMenuEntry *entry, gchar *uri)
{
	gboolean retval = FALSE;

	g_return_val_if_fail (entry != NULL, FALSE);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS, FALSE);

	retval = panel_menu_actions_append_item (entry, uri);
	if (retval)
		panel_menu_common_set_changed (entry->parent);
	return retval;
}

gboolean
panel_menu_actions_append_item (PanelMenuEntry *entry, gchar *uri)
{
	PanelMenuActions *actions;
	GtkWidget *menuitem;
	gboolean retval = FALSE;

	g_return_val_if_fail (entry != NULL, FALSE);
	g_return_val_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS, FALSE);

	g_print ("append_item called with uri %s\n", uri);
	actions = (PanelMenuActions *) entry->data;
	if (!strncmp (uri, "action:", strlen ("action:"))) {
		if ((menuitem = construct_from_uri (uri))) {
			gtk_menu_shell_append (GTK_MENU_SHELL (actions->menu),
					       menuitem);
			/* action_item_dnd_init(menuitem); */
			actions->items_list =
				g_list_append (actions->items_list, menuitem);
			actions->actions_list =
				g_list_append (actions->actions_list,
					       g_strdup (uri));
			retval = TRUE;
		}
	}
	return retval;
}

static gboolean
check_valid_uri (gchar *uri)
{
	gboolean retval = FALSE;
	int counter;

	for (counter = 0; counter < n_actions; counter++) {
		if (!strcmp (uri, action_items[counter].path)) {
			retval = TRUE;
			break;
		}
	}
	return retval;
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
			panel_menu_common_set_icon_scaled_from_file
				(GTK_MENU_ITEM (widget),
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
construct_run_item (GtkWidget *widget)
{

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

void
panel_menu_actions_new_with_dialog (PanelMenu *panel_menu)
{
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *name_entry;
	gint response;
	gchar *name;
	PanelMenuEntry *entry;

	dialog = panel_menu_common_single_entry_dialog_new (_("Create actions item..."),
							    _("Name:"),
							    _("Actions"),
							    &name_entry);
	gtk_widget_show (dialog);
	gtk_widget_grab_focus (name_entry);

	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_OK) {
		name = (gchar *) gtk_entry_get_text (GTK_ENTRY (name_entry));
		entry = panel_menu_actions_new (panel_menu, name);
		panel_menu->entries =
			g_list_append (panel_menu->entries, (gpointer) entry);
		panel_menu_options_append_option (panel_menu_common_find_options
						  (panel_menu),
						  panel_menu_actions_get_checkitem
						  (entry));
		gtk_menu_shell_append (GTK_MENU_SHELL (panel_menu->menubar),
				       panel_menu_actions_get_widget (entry));
		panel_menu_common_set_changed (panel_menu);
	}
	gtk_widget_destroy (dialog);
}

static void
actions_properties_cb (GtkWidget *widget, PanelMenuEntry *entry, const gchar *verb)
{
	PanelMenuActions *actions;
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *name_entry;
	GtkWidget *label;
	GtkWidget *bbox;
	GtkSizeGroup *group;
	GtkWidget *button;
	GtkWidget *hbox;
	GdkPixbuf *pixbuf;
	GtkWidget *image;
	gint counter;
	gint response;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->type == PANEL_MENU_TYPE_ACTIONS);

	actions = (PanelMenuActions *)entry->data;
	/* FIXME strdup_printf the title */
	dialog = gtk_dialog_new_with_buttons (_("Actions item Properties"),
					      NULL, GTK_DIALOG_MODAL,
					      GTK_STOCK_OK, GTK_RESPONSE_OK,
					      GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					      NULL);
	box = GTK_DIALOG (dialog)->vbox;
	gtk_container_set_border_width (GTK_CONTAINER (box), 5);

	hbox = gtk_hbox_new (FALSE, 5);
	gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
	gtk_box_pack_start (GTK_BOX (box), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);

	label = gtk_label_new (_("Name:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 5);
	gtk_widget_show (label);

	name_entry = gtk_entry_new_with_max_length (64);
	gtk_box_pack_start (GTK_BOX (hbox), name_entry, TRUE, TRUE, 5);
	gtk_entry_set_text (GTK_ENTRY (name_entry), actions->name);
	g_object_set_data (G_OBJECT (name_entry), "panel-menu-entry", entry);
	gtk_widget_show (name_entry);

	label = gtk_label_new (_("To add an item to your actions menu,\n"
			         "either drag the item with a middle-click,\n"
			         "or left-click on an item to append\n"
			         "it to your actions menu."));
	gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 5);
	gtk_widget_show (label);

	bbox = gtk_hbox_new (FALSE, 5);
	gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
	gtk_box_pack_start (GTK_BOX (box), bbox, TRUE, TRUE, 0);
	gtk_widget_show (bbox);

	group = gtk_size_group_new (GTK_SIZE_GROUP_BOTH);

	for (counter = 0; counter < n_actions; counter++) {
		button = gtk_button_new ();
		gtk_size_group_add_widget (group, button);
		hbox = gtk_hbox_new (FALSE, 0);
		gtk_container_add (GTK_CONTAINER (button), hbox);
		gtk_widget_show (hbox);
		pixbuf = gdk_pixbuf_new_from_file (action_items[counter].image,
						   NULL);
		if (pixbuf) {
			image = gtk_image_new_from_pixbuf (pixbuf);
			gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE,
					    0);
			gtk_widget_show (image);
		}
		label = gtk_label_new (action_items[counter].name);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
		gtk_widget_show (label);
		g_object_set_data (G_OBJECT (button), "uri-path",
				   action_items[counter].path);
		g_signal_connect (G_OBJECT (button), "clicked",
				  G_CALLBACK (button_clicked_append_item),
				  entry);
		action_item_dnd_init (button);
		gtk_box_pack_start (GTK_BOX (bbox), button, FALSE, FALSE, 0);
		gtk_widget_show (button);
	}
	gtk_widget_show (dialog);
	gtk_widget_grab_focus (name_entry);
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (handle_response), name_entry);
}

static void
handle_response (GtkWidget *dialog, gint response, GtkWidget *name_entry)
{
	PanelMenuEntry *entry;
	PanelMenuActions *actions;
	const gchar *name;

	entry = (PanelMenuEntry *)g_object_get_data (
		G_OBJECT (name_entry), "panel-menu-entry");
	actions = (PanelMenuActions *)entry->data;
	if (response == GTK_RESPONSE_OK) {
		name = gtk_entry_get_text (GTK_ENTRY (name_entry));
		if (strcmp (name, actions->name)) {
			panel_menu_actions_set_name (entry, name);
			panel_menu_common_set_changed (entry->parent);
		}
	}
	gtk_widget_destroy (dialog);
}

static void
button_clicked_append_item (GtkWidget *button, PanelMenuEntry *entry)
{
	gchar *uri;

	uri = g_object_get_data (G_OBJECT (button), "uri-path");
	panel_menu_actions_append_item (entry, uri);
}

static void
action_item_dnd_init (GtkWidget *button)
{
	gtk_drag_source_set (button, GDK_BUTTON2_MASK, drag_types, n_drag_types,
			     GDK_ACTION_COPY | GDK_ACTION_LINK |
			     GDK_ACTION_ASK);
	g_signal_connect (G_OBJECT (button), "drag_begin",
			  G_CALLBACK (action_item_dnd_drag_begin_cb), NULL);
	g_signal_connect (G_OBJECT (button), "drag_data_get",
			  G_CALLBACK (action_item_dnd_set_data_cb), NULL);
}

static void
action_item_dnd_drag_begin_cb (GtkWidget *button, GdkDragContext *context)
{
	GtkWidget *image;
	GdkPixbuf *pixbuf;
	GtkWidget *window;
	GtkWidget *frame;
	GtkWidget *box;
	GtkWidget *widget;
	GtkWidget *temp;
	GList *children;

	if (GTK_IS_IMAGE_MENU_ITEM (button)) {
		if ((image =
		     gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM
						    (button)))
		    && (pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (image))))
			gtk_drag_set_icon_pixbuf (context, pixbuf, -5, -5);
	} else if (GTK_IS_BUTTON (button)) {
		window = gtk_window_new (GTK_WINDOW_POPUP);
		frame = gtk_button_new ();
		gtk_container_add (GTK_CONTAINER (window), frame);
		box = gtk_hbox_new (FALSE, 0);
		gtk_container_add (GTK_CONTAINER (frame), box);
		gtk_widget_show (box);
		for (children = GTK_BOX (GTK_BIN (button)->child)->children;
		     children; children = children->next) {
			temp = NULL;
			widget = ((GtkBoxChild *) (children->data))->widget;
			if (GTK_IS_LABEL (widget))
				temp = gtk_label_new (gtk_label_get_text
						      (GTK_LABEL (widget)));
			else if (GTK_IS_IMAGE (widget))
				temp = gtk_image_new_from_pixbuf
					(gtk_image_get_pixbuf
					 (GTK_IMAGE (widget)));
			if (temp)
				gtk_box_pack_start (GTK_BOX (box), temp, FALSE,
						    FALSE, 0);
		}
		gtk_widget_show_all (frame);
		gtk_drag_set_icon_widget (context, window, 0, 0);
	}
}

static void
action_item_dnd_set_data_cb (GtkWidget *button, GdkDragContext *context,
			     GtkSelectionData *selection_data, guint info,
			     guint time)
{
	gchar *uri;

	if ((uri = g_object_get_data (G_OBJECT (button), "uri-path"))) {
		gtk_selection_data_set (selection_data, selection_data->target,
					8, uri, strlen (uri));
	} else {
		gtk_selection_data_set (selection_data, selection_data->target,
					8, NULL, 0);
	}
}
