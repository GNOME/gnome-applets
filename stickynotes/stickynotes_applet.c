/* Sticky Notes 
 * Copyright (C) 2002-2003 Loban A Rahman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>
#include <stickynotes_applet.h>
#include <stickynotes_applet_callbacks.h>
#include <stickynotes.h>

StickyNotes *stickynotes = NULL;

/* Popup menu on the applet */
static const BonoboUIVerb stickynotes_applet_menu_verbs[] = 
{
        BONOBO_UI_UNSAFE_VERB ("create", menu_create_cb),
        BONOBO_UI_UNSAFE_VERB ("destroy_all", menu_destroy_all_cb),
        BONOBO_UI_UNSAFE_VERB ("preferences", menu_preferences_cb),
        BONOBO_UI_UNSAFE_VERB ("help", menu_help_cb),
        BONOBO_UI_UNSAFE_VERB ("about", menu_about_cb),
        BONOBO_UI_VERB_END
};

/* Sticky Notes applet factory */
static gboolean stickynotes_applet_factory(PanelApplet *panel_applet, const gchar *iid, gpointer data) 
{
	if (!strcmp(iid, "OAFIID:GNOME_StickyNotesApplet")) {
		if (!stickynotes)
			stickynotes_applet_init();

		/* Add applet to linked list of all applets */
		stickynotes->applets = g_list_append(stickynotes->applets, stickynotes_applet_new(panel_applet));

		stickynotes_applet_update_menus();
		stickynotes_applet_update_tooltips();

		return TRUE;
	}

	return FALSE;
}

/* Sticky Notes applet factory */
PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_StickyNotesApplet_Factory", PANEL_TYPE_APPLET, PACKAGE, VERSION, stickynotes_applet_factory, NULL);

/* Create and initalize global sticky notes instance */
void stickynotes_applet_init()
{
	stickynotes = g_new(StickyNotes, 1);

	stickynotes->notes = NULL;
	stickynotes->applets = NULL;
	stickynotes->icon_normal = gdk_pixbuf_new_from_file(STICKYNOTES_ICONDIR "/stickynotes.png", NULL);
	stickynotes->icon_prelight = gdk_pixbuf_new_from_file(STICKYNOTES_ICONDIR "/stickynotes_prelight.png", NULL);
	stickynotes->gconf = gconf_client_get_default();
	stickynotes->tooltips = gtk_tooltips_new();

	stickynotes_applet_init_about();
	stickynotes_applet_init_prefs();

	/* Set default icon for all sticky note windows */
	gnome_window_icon_set_default_from_file(STICKYNOTES_ICONDIR "/stickynotes.png");

	/* Watch GConf values */
	gconf_client_add_dir(stickynotes->gconf, GCONF_PATH, GCONF_CLIENT_PRELOAD_NONE, NULL);
	gconf_client_notify_add(stickynotes->gconf, GCONF_PATH "/defaults", (GConfClientNotifyFunc) preferences_apply_cb, NULL, NULL, NULL);
	gconf_client_notify_add(stickynotes->gconf, GCONF_PATH "/settings", (GConfClientNotifyFunc) preferences_apply_cb, NULL, NULL, NULL);
	
	/* Load sticky notes */
	stickynotes_load();
	
	/* Auto-save every so minutes (default 5) */
	g_timeout_add(1000 * 60 * gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/settings/autosave_time", NULL),
		      (GSourceFunc) applet_save_cb, NULL);
}

void stickynotes_applet_init_about()
{
	stickynotes->about = glade_xml_new(GLADE_PATH, "about_dialog", NULL);

	stickynotes->w_about = glade_xml_get_widget(stickynotes->about, "about_dialog");

	/* FIXME : Hack because libglade does not properly set these */
	g_object_set(G_OBJECT(stickynotes->w_about), "name", _("Sticky Notes"));
	g_object_set(G_OBJECT(stickynotes->w_about), "version", VERSION);
	
	{
		GdkPixbuf *logo = gdk_pixbuf_new_from_file(STICKYNOTES_ICONDIR "/stickynotes.png", NULL);
		g_object_set(G_OBJECT(stickynotes->w_about), "logo", logo);
		g_object_unref(logo);
	}
	if (strcmp(_("translator_credits"), "translator_credits") == 0)
		g_object_set(G_OBJECT(stickynotes->w_about), "translator_credits", NULL);
	
	g_signal_connect(G_OBJECT(stickynotes->w_about), "response", G_CALLBACK(about_response_cb), NULL);
}

void stickynotes_applet_init_prefs()
{
	stickynotes->prefs = glade_xml_new(GLADE_PATH, "preferences_dialog", NULL);

	stickynotes->w_prefs = glade_xml_get_widget(stickynotes->prefs, "preferences_dialog");
	stickynotes->w_prefs_width = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(stickynotes->prefs, "width_spin")));
	stickynotes->w_prefs_height= gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(stickynotes->prefs, "height_spin")));
	stickynotes->w_prefs_color = glade_xml_get_widget(stickynotes->prefs, "note_color");
	stickynotes->w_prefs_system = GTK_WIDGET(&GTK_CHECK_BUTTON(glade_xml_get_widget(stickynotes->prefs, "use_system_check"))->toggle_button);
	stickynotes->w_prefs_click = glade_xml_get_widget(stickynotes->prefs, "click_behavior_menu");
	stickynotes->w_prefs_sticky = GTK_WIDGET(&GTK_CHECK_BUTTON(glade_xml_get_widget(stickynotes->prefs, "sticky_check"))->toggle_button);
	stickynotes->w_prefs_force = GTK_WIDGET(&GTK_CHECK_BUTTON(glade_xml_get_widget(stickynotes->prefs, "force_default_check"))->toggle_button);

	g_signal_connect(G_OBJECT(stickynotes->w_prefs), "response", G_CALLBACK(preferences_response_cb), NULL);
	g_signal_connect_swapped(G_OBJECT(stickynotes->w_prefs_width), "value-changed", G_CALLBACK(preferences_save_cb), NULL);
	g_signal_connect_swapped(G_OBJECT(stickynotes->w_prefs_height), "value-changed", G_CALLBACK(preferences_save_cb), NULL);
	g_signal_connect_swapped(G_OBJECT(stickynotes->w_prefs_system), "toggled", G_CALLBACK(preferences_save_cb), NULL);
	g_signal_connect(G_OBJECT(stickynotes->w_prefs_color), "color_set", G_CALLBACK(preferences_color_cb), NULL);
	g_signal_connect_swapped(G_OBJECT(stickynotes->w_prefs_click), "changed", G_CALLBACK(preferences_save_cb), NULL);
	g_signal_connect_swapped(G_OBJECT(stickynotes->w_prefs_sticky), "toggled", G_CALLBACK(preferences_save_cb), NULL);
	g_signal_connect_swapped(G_OBJECT(stickynotes->w_prefs_force), "toggled", G_CALLBACK(preferences_save_cb), NULL);
	
	{
		GtkSizeGroup *group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

		gtk_size_group_add_widget(group, glade_xml_get_widget(stickynotes->prefs, "width_label"));
		gtk_size_group_add_widget(group, glade_xml_get_widget(stickynotes->prefs, "height_label"));
		gtk_size_group_add_widget(group, glade_xml_get_widget(stickynotes->prefs, "color_label"));
		gtk_size_group_add_widget(group, glade_xml_get_widget(stickynotes->prefs, "click_label"));

		g_object_unref(group);
	}
			
	stickynotes_applet_update_prefs();
}

/* Create a Sticky Notes applet */
StickyNotesApplet * stickynotes_applet_new(PanelApplet *panel_applet)
{
	/* Create Sticky Notes Applet */
	StickyNotesApplet *applet = g_new(StickyNotesApplet, 1);

	/* Initialize Sticky Notes Applet */
	applet->w_applet = GTK_WIDGET(panel_applet);
	applet->w_image = gtk_image_new();
	applet->prelighted = FALSE;
	applet->pressed = FALSE;

	/* Create the applet */
	gtk_container_add(GTK_CONTAINER(panel_applet), applet->w_image);
	stickynotes_applet_update_icon(applet);

	/* Add the popup menu */
	panel_applet_setup_menu_from_file(panel_applet, NULL, "GNOME_StickyNotesApplet.xml", NULL, stickynotes_applet_menu_verbs, applet);
	g_signal_connect(G_OBJECT(panel_applet_get_popup_component(panel_applet)), "ui-event", G_CALLBACK(menu_event_cb), applet);

	gtk_widget_add_events(GTK_WIDGET(applet->w_applet), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

	/* Connect all signals for applet management */
	g_signal_connect(G_OBJECT(applet->w_applet), "button-press-event", G_CALLBACK(applet_button_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "button-release-event", G_CALLBACK(applet_button_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "key-press-event", G_CALLBACK(applet_key_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "key-release-event", G_CALLBACK(applet_key_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "focus-in-event", G_CALLBACK(applet_focus_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "focus-out-event", G_CALLBACK(applet_focus_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "enter-notify-event", G_CALLBACK(applet_cross_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "leave-notify-event", G_CALLBACK(applet_cross_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "change-size", G_CALLBACK(applet_change_size_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "change-background", G_CALLBACK(applet_change_bg_cb), applet);

	/* Show the applet */
	gtk_widget_show_all(GTK_WIDGET(applet->w_applet));

	return applet;
}

void stickynotes_applet_update_icon(StickyNotesApplet *applet)
{
	GdkPixbuf *pixbuf1, *pixbuf2;

	gint size = panel_applet_get_size(PANEL_APPLET(applet->w_applet));

	/* Choose appropriate icon and size it */
	if (applet->prelighted)
	    	pixbuf1 = gdk_pixbuf_scale_simple(stickynotes->icon_prelight, size, size, GDK_INTERP_BILINEAR);
	else
	    	pixbuf1 = gdk_pixbuf_scale_simple(stickynotes->icon_normal, size, size, GDK_INTERP_BILINEAR);
	
	/* Shift the icon if pressed */
	pixbuf2 = gdk_pixbuf_copy(pixbuf1);
	if (applet->pressed)
		gdk_pixbuf_scale(pixbuf1, pixbuf2, 0, 0, size, size, 1, 1, 1, 1, GDK_INTERP_BILINEAR);

	/* Apply the finished pixbuf to the applet image */
	gtk_image_set_from_pixbuf(GTK_IMAGE(applet->w_image), pixbuf2);

	g_object_unref(pixbuf1);
	g_object_unref(pixbuf2);
}

void stickynotes_applet_update_prefs()
{
	gint width = gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/defaults/width", NULL);
	gint height = gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/defaults/height", NULL);
	gboolean use_system = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/use_system_color", NULL);
	gint click_behavior = gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/settings/click_behavior", NULL);
	gboolean sticky = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/sticky", NULL);
	gboolean force_default = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/force_default_color", NULL);

	GdkColor color;
	{
		gchar *color_str = gconf_client_get_string(stickynotes->gconf, GCONF_PATH "/defaults/color", NULL);
		gdk_color_parse(color_str, &color);
		g_free(color_str);
	}

	gtk_adjustment_set_value(stickynotes->w_prefs_width, width);
	gtk_adjustment_set_value(stickynotes->w_prefs_height, height);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_system), use_system);
	gtk_option_menu_set_history(GTK_OPTION_MENU(stickynotes->w_prefs_click), click_behavior);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_sticky), sticky);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_force), force_default);

	gnome_color_picker_set_i16(GNOME_COLOR_PICKER(stickynotes->w_prefs_color), color.red, color.green, color.blue, 65535);
	
	gtk_widget_set_sensitive(glade_xml_get_widget(stickynotes->prefs, "color_label"), !use_system);
	gtk_widget_set_sensitive(stickynotes->w_prefs_color, !use_system);
}

void stickynotes_applet_update_menus()
{
	gboolean visible = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/visible", NULL);
	gboolean locked = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/locked", NULL);

	gint i;
	for (i = 0; i < g_list_length(stickynotes->applets); i++) {
		StickyNotesApplet *applet = g_list_nth_data(stickynotes->applets, i);
		BonoboUIComponent *popup = panel_applet_get_popup_component(PANEL_APPLET(applet->w_applet));
		
		bonobo_ui_component_set_prop(popup, "/commands/show", "state", visible ? "1" : "0", NULL);
		bonobo_ui_component_set_prop(popup, "/commands/lock", "state", locked ? "1" : "0", NULL);
	}
}

void stickynotes_applet_update_tooltips()
{
	gint num = g_list_length(stickynotes->notes);
	gchar *tooltip = g_strdup_printf(ngettext("%s\n%d note", "%s\n%d notes", num), _("Sticky Notes"), num);

	gint i;	
	for (i = 0; i < g_list_length(stickynotes->applets); i++) {
		StickyNotesApplet *applet = g_list_nth_data(stickynotes->applets, i);
		gtk_tooltips_set_tip(stickynotes->tooltips, applet->w_applet, tooltip, NULL);
	}

	g_free(tooltip);
}

void stickynotes_applet_do_default_action()
{
	StickyNotesDefaultAction click_behavior = gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/settings/click_behavior", NULL);

	gboolean visible = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/visible", NULL);
	gboolean locked = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/locked", NULL);

	switch (click_behavior) {
		case STICKYNOTES_NEW:
			stickynotes_add();
			break;

		case STICKYNOTES_SET_VISIBLE:
			gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/visible", !visible, NULL);
			break;

		case STICKYNOTES_SET_LOCKED:
			gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/locked", !locked, NULL);
			break;
	}
}
