/*  panel-menu-properties.c
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
#include <libbonoboui.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include "panel-menu.h"
#include "panel-menu-common.h"
#include "panel-menu-applications.h"
#include "panel-menu-preferences.h"
#include "panel-menu-actions.h"
#include "panel-menu-windows.h"
#include "panel-menu-workspaces.h"
#include "panel-menu-properties.h"

static void set_widget_sensitivity (GtkWidget *checkitem, GtkWidget *target);
static void handle_has_applications (GtkWidget *widget, PanelMenu *panel_menu);
static void handle_has_preferences (GtkWidget *widget, PanelMenu *panel_menu);
static void handle_applications_icon (GtkWidget *widget, PanelMenu *panel_menu);
static void handle_has_actions (GtkWidget *widget, PanelMenu *panel_menu);
static void handle_has_windows (GtkWidget *widget, PanelMenu *panel_menu);
static void handle_has_workspaces (GtkWidget *widget, PanelMenu *panel_menu);
static void handle_response_cb (GtkDialog *dialog, gint response,
				gpointer data);

void
applet_properties_cb (BonoboUIComponent *uic, PanelMenu *panel_menu,
		      const gchar *verbname)
{
	GtkWidget *dialog;
	GtkWidget *box;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *fbox;
	GtkWidget *label;
	GtkWidget *menu;
	GtkWidget *item;

	GtkWidget *has_applications;
	GtkWidget *has_preferences;
	GtkWidget *applications_icon;
	GtkWidget *has_actions;
	GtkWidget *has_windows;
	GtkWidget *has_workspaces;
	gchar *icon;

	dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (dialog), _("Menu Bar Preferences"));
	gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE);
	box = GTK_DIALOG (dialog)->vbox;

	frame = gtk_frame_new (_("Layout"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);
	gtk_widget_show (frame);

	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	gtk_widget_show (vbox);

	hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

	has_applications =
		gtk_check_button_new_with_mnemonic (_("_Applications Menu"));
	gtk_box_pack_start (GTK_BOX (hbox), has_applications, FALSE,
			    FALSE, 0);
	gtk_widget_show (has_applications);

	fbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (hbox), fbox, TRUE, TRUE, 0);
	gtk_widget_set_sensitive (fbox, panel_menu->has_applications);
	gtk_widget_show (fbox);

	g_signal_connect (G_OBJECT (has_applications), "toggled",
			  G_CALLBACK (set_widget_sensitivity), fbox);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				     (has_applications),
				      panel_menu->has_applications);
	g_signal_connect (G_OBJECT (has_applications), "toggled",
			  G_CALLBACK (handle_has_applications), panel_menu);

	label = gtk_label_new_with_mnemonic (_("_Icon :"));
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (fbox), label, FALSE, FALSE, 0);
	
	applications_icon =
		gnome_icon_entry_new ("panel-menu-applet-id",
				    _("Select an icon for the Applications menu"));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), applications_icon);
	gtk_box_pack_start (GTK_BOX (fbox), applications_icon, FALSE, FALSE, 0);

	icon = panel_applet_gconf_get_string (panel_menu->applet,
					     "applications-image",
					      NULL);
	if (!icon)
		icon = g_strdup (DATADIR "/pixmaps/gnome-logo-icon-transparent.png");
	gnome_icon_entry_set_filename (GNOME_ICON_ENTRY(applications_icon),
				       icon);
	g_free (icon);
	g_signal_connect (G_OBJECT (applications_icon), "changed",
			  G_CALLBACK (handle_applications_icon), panel_menu);
	gtk_widget_show (applications_icon);

	has_preferences =
		gtk_check_button_new_with_mnemonic (_("_Preferences Menu"));
	gtk_box_pack_start (GTK_BOX (vbox), has_preferences, FALSE,
			    FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				     (has_preferences),
				      panel_menu->has_preferences);
	g_signal_connect (G_OBJECT (has_preferences), "toggled",
			  G_CALLBACK (handle_has_preferences), panel_menu);
	gtk_widget_show (has_preferences);

	has_actions =
		gtk_check_button_new_with_mnemonic (_("Ac_tions Menu"));
	gtk_box_pack_start (GTK_BOX (vbox), has_actions, FALSE,
			    FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				     (has_actions),
				      panel_menu->has_actions);
	g_signal_connect (G_OBJECT (has_actions), "toggled",
			  G_CALLBACK (handle_has_actions), panel_menu);
	gtk_widget_show (has_actions);

	has_windows =
		gtk_check_button_new_with_mnemonic (_("_Windows Menu"));
	gtk_box_pack_start (GTK_BOX (vbox), has_windows, FALSE,
			    FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				     (has_windows),
				      panel_menu->has_windows);
	g_signal_connect (G_OBJECT (has_windows), "toggled",
			  G_CALLBACK (handle_has_windows), panel_menu);
	gtk_widget_show (has_windows);

	has_workspaces =
		gtk_check_button_new_with_mnemonic(_("W_orkspaces Menu"));
	gtk_box_pack_start (GTK_BOX (vbox), has_workspaces, FALSE,
			    FALSE, 0);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				     (has_workspaces),
				      panel_menu->has_workspaces);
	g_signal_connect (G_OBJECT (has_workspaces), "toggled",
			  G_CALLBACK (handle_has_workspaces), panel_menu);
	gtk_widget_show (has_workspaces);

	/* Behavior */
/*
	frame = gtk_frame_new (_("Behavior"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);
	gtk_widget_show (frame);

	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	gtk_widget_show (vbox);
*/

	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (handle_response_cb), NULL);
	gtk_widget_show_all (dialog);
}

static void
set_widget_sensitivity (GtkWidget *checkitem, GtkWidget *target)
{
	gtk_widget_set_sensitive (target,
				  GTK_TOGGLE_BUTTON (checkitem)->active);
}

static void
handle_has_applications (GtkWidget *widget, PanelMenu *panel_menu)
{
	PanelMenuEntry *entry;
	gint insert = 0;

	entry = panel_menu_common_find_applications (panel_menu);
	panel_menu->has_applications = GTK_TOGGLE_BUTTON (widget)->active;
	if (panel_menu->has_applications && entry == NULL) {
		entry = panel_menu_applications_new (panel_menu);
		widget = panel_menu_common_get_entry_menuitem (entry);
		gtk_menu_shell_prepend (GTK_MENU_SHELL (panel_menu->menubar),
				        widget);
		panel_menu->entries = g_list_prepend (panel_menu->entries,
						      entry);
		gtk_widget_show (widget);
	} else if (entry) {
		panel_menu->entries = g_list_remove (panel_menu->entries,
						     entry);
		panel_menu_common_call_entry_destroy (entry);
	}
	panel_menu_config_save_layout (panel_menu);
}

static void
handle_applications_icon (GtkWidget *widget, PanelMenu *panel_menu)
{
	PanelMenuEntry *entry;
	const gchar *new;
	const gchar *icon;

	new = gnome_icon_entry_get_filename(GNOME_ICON_ENTRY (widget));
	entry = panel_menu_common_find_applications (panel_menu);
	if ((icon = panel_menu_applications_get_icon (entry))) {
		if (!icon || strcmp (new, icon)) {
			panel_menu_applications_set_icon (entry, new);
			panel_menu_applications_save_config (entry);
		}
	}
}

static void
handle_has_preferences (GtkWidget *widget, PanelMenu *panel_menu)
{
	PanelMenuEntry *entry;
	gint insert = 0;

	entry = panel_menu_common_find_preferences (panel_menu);
	panel_menu->has_preferences = GTK_TOGGLE_BUTTON (widget)->active;
	if (panel_menu->has_preferences && entry == NULL) {
		entry = panel_menu_preferences_new (panel_menu);
		widget = panel_menu_common_get_entry_menuitem (entry);
		insert = g_list_length (panel_menu->entries);
		if (panel_menu->has_workspaces)
			insert--;
		if (panel_menu->has_windows)
			insert--;
		if (panel_menu->has_actions)
			insert--;
		gtk_menu_shell_insert (GTK_MENU_SHELL (panel_menu->menubar),
				       widget, insert);
		panel_menu->entries = g_list_insert (panel_menu->entries,
						     entry, insert);
		gtk_widget_show (widget);
	} else if (entry) {
		panel_menu->entries = g_list_remove (panel_menu->entries,
						     entry);
		panel_menu_common_call_entry_destroy (entry);
	}
	panel_menu_config_save_layout (panel_menu);
}

static void
handle_has_actions (GtkWidget *widget, PanelMenu *panel_menu)
{
	PanelMenuEntry *entry;
	gint insert = 0;

	entry = panel_menu_common_find_actions (panel_menu);
	panel_menu->has_actions = GTK_TOGGLE_BUTTON (widget)->active;
	if (panel_menu->has_actions && entry == NULL) {
		entry = panel_menu_actions_new (panel_menu);
		widget = panel_menu_common_get_entry_menuitem (entry);
		insert = g_list_length (panel_menu->entries);
		if (panel_menu->has_workspaces)
			insert--;
		if (panel_menu->has_windows)
			insert--;
		gtk_menu_shell_insert (GTK_MENU_SHELL (panel_menu->menubar),
				       widget, insert);
		panel_menu->entries = g_list_insert (panel_menu->entries,
						     entry, insert);
		gtk_widget_show (widget);
	} else if (entry) {
		panel_menu->entries = g_list_remove (panel_menu->entries,
						     entry);
		panel_menu_common_call_entry_destroy (entry);
	}
	panel_menu_config_save_layout (panel_menu);
}

static void
handle_has_windows (GtkWidget *widget, PanelMenu *panel_menu)
{
	PanelMenuEntry *entry;
	gint insert = 0;

	entry = panel_menu_common_find_windows (panel_menu);
	panel_menu->has_windows = GTK_TOGGLE_BUTTON (widget)->active;
	if (panel_menu->has_windows && entry == NULL) {
		entry = panel_menu_windows_new (panel_menu);
		widget = panel_menu_common_get_entry_menuitem (entry);
		insert = g_list_length (panel_menu->entries);
		if (panel_menu->has_workspaces)
			insert--;
		gtk_menu_shell_insert (GTK_MENU_SHELL (panel_menu->menubar),
				       widget, insert);
		panel_menu->entries = g_list_insert (panel_menu->entries,
						     entry, insert);
		gtk_widget_show (widget);
	} else if (entry) {
		panel_menu->entries = g_list_remove (panel_menu->entries,
						     entry);
		panel_menu_common_call_entry_destroy (entry);
	}
	panel_menu_config_save_layout (panel_menu);
}

static void
handle_has_workspaces (GtkWidget *widget, PanelMenu *panel_menu)
{
	PanelMenuEntry *entry;

	entry = panel_menu_common_find_workspaces (panel_menu);
	panel_menu->has_workspaces = GTK_TOGGLE_BUTTON (widget)->active;
	if (panel_menu->has_workspaces && entry == NULL) {
		entry = panel_menu_workspaces_new (panel_menu);
		widget = panel_menu_common_get_entry_menuitem (entry);
		gtk_menu_shell_append (GTK_MENU_SHELL (panel_menu->menubar),
				       widget);
		panel_menu->entries = g_list_append (panel_menu->entries,
						     entry);
		gtk_widget_show (widget);
	} else if (entry) {
		panel_menu->entries = g_list_remove (panel_menu->entries,
						     entry);
		panel_menu_common_call_entry_destroy (entry);
	}
	panel_menu_config_save_layout (panel_menu);
}

static void
handle_response_cb (GtkDialog *dialog, gint response, gpointer data)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));
}
