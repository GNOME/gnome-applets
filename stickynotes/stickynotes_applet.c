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

/* Settings for the popup menu on the applet */
static const BonoboUIVerb stickynotes_applet_menu_verbs[] = 
{
        BONOBO_UI_UNSAFE_VERB ("create", menu_create_cb),
        BONOBO_UI_UNSAFE_VERB ("destroy_all", menu_destroy_all_cb),
        BONOBO_UI_UNSAFE_VERB ("hide", menu_hide_cb),
        BONOBO_UI_UNSAFE_VERB ("show", menu_show_cb),
        BONOBO_UI_UNSAFE_VERB ("lock", menu_lock_cb),
        BONOBO_UI_UNSAFE_VERB ("unlock", menu_unlock_cb),
        BONOBO_UI_UNSAFE_VERB ("preferences", menu_preferences_cb),
        BONOBO_UI_UNSAFE_VERB ("help", menu_help_cb),
        BONOBO_UI_UNSAFE_VERB ("about", menu_about_cb),
        BONOBO_UI_VERB_END
};

/* Fill the Sticky Notes applet */
static gboolean stickynotes_applet_fill(PanelApplet *applet)
{
	/* Sticky Notes Applet settings instance */
	StickyNotesApplet *stickynotes;

	/* Create and initialize Sticky Note Applet Settings Instance */
	stickynotes = g_new(StickyNotesApplet, 1);
	stickynotes->applet = GTK_WIDGET(applet);
	stickynotes->about = NULL;
	stickynotes->prefs= NULL;
	stickynotes->pressed = FALSE;
	stickynotes->notes = NULL;
	stickynotes->pixbuf_normal = gdk_pixbuf_new_from_file(STICKYNOTES_ICONDIR "/stickynotes.png", NULL);
	stickynotes->pixbuf_prelight = gdk_pixbuf_new_from_file(STICKYNOTES_ICONDIR "/stickynotes_prelight.png", NULL);
	stickynotes->image = gtk_image_new();
	stickynotes->gconf_client = gconf_client_get_default();
	stickynotes->tooltips = gtk_tooltips_new();

	/* Create the applet */
	gtk_container_add(GTK_CONTAINER(applet), stickynotes->image);
	panel_applet_setup_menu_from_file(applet, NULL, "GNOME_StickyNotesApplet.xml", NULL, stickynotes_applet_menu_verbs, stickynotes);
	stickynotes_applet_update_icon(stickynotes, FALSE);

	gtk_widget_add_events(GTK_WIDGET(applet), GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

	/* Connect all signals for applet management */
	g_signal_connect(G_OBJECT(applet), "button-press-event", G_CALLBACK(applet_button_cb), stickynotes);
	g_signal_connect(G_OBJECT(applet), "button-release-event", G_CALLBACK(applet_button_cb), stickynotes);
	g_signal_connect(G_OBJECT(applet), "key-press-event", G_CALLBACK(applet_key_cb), stickynotes);
	g_signal_connect(G_OBJECT(applet), "key-release-event", G_CALLBACK(applet_key_cb), stickynotes);
	g_signal_connect(G_OBJECT(applet), "focus-in-event", G_CALLBACK(applet_focus_cb), stickynotes);
	g_signal_connect(G_OBJECT(applet), "focus-out-event", G_CALLBACK(applet_focus_cb), stickynotes);
	g_signal_connect(G_OBJECT(applet), "enter-notify-event", G_CALLBACK(applet_cross_cb), stickynotes);
	g_signal_connect(G_OBJECT(applet), "leave-notify-event", G_CALLBACK(applet_cross_cb), stickynotes);
	g_signal_connect(G_OBJECT(applet), "change-size", G_CALLBACK(applet_change_size_cb), stickynotes);
	g_signal_connect(G_OBJECT(applet), "change-background", G_CALLBACK(applet_change_bg_cb), stickynotes);

	/* Show the applet */
	gtk_widget_show_all(GTK_WIDGET(applet));

	/* Watch GConf values */
	gconf_client_add_dir(stickynotes->gconf_client, GCONF_PATH, GCONF_CLIENT_PRELOAD_NONE, NULL);
	gconf_client_notify_add(stickynotes->gconf_client, GCONF_PATH "/defaults", (GConfClientNotifyFunc) preferences_apply_cb,
				stickynotes, NULL, NULL);
	gconf_client_notify_add(stickynotes->gconf_client, GCONF_PATH "/settings", (GConfClientNotifyFunc) preferences_apply_cb,
				stickynotes, NULL, NULL);
	
	/* Set default icon for all sticky note windows */
	gnome_window_icon_set_default_from_file(STICKYNOTES_ICONDIR "/stickynotes.png");

	/* Load sticky notes */
	stickynotes_load(stickynotes);
	
	/* Hide sticky notes if necessary */
	stickynotes_set_visible(stickynotes, gconf_client_get_bool(stickynotes->gconf_client, GCONF_PATH "/settings/visible", NULL));
	/* Lock sticky notes if necessary */
	stickynotes_set_locked(stickynotes, gconf_client_get_bool(stickynotes->gconf_client, GCONF_PATH "/settings/locked", NULL));

	/* Auto-save every so minutes (default 5) */
	g_timeout_add(1000 * 60 * gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/settings/autosave_time", NULL),
		      (GSourceFunc) applet_save_cb, stickynotes);
	
	return TRUE;
}

/* Create the Sticky Notes applet */
static gboolean stickynotes_applet_factory(PanelApplet *applet, const gchar *iid, gpointer data) 
{
	if (!strcmp(iid, "OAFIID:GNOME_StickyNotesApplet"))
		return stickynotes_applet_fill(applet);

	return FALSE;
}

/* Initialize the applet */
PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_StickyNotesApplet_Factory", PANEL_TYPE_APPLET, PACKAGE, VERSION, stickynotes_applet_factory, NULL);

/* Highlight the Sticky Notes Applet */
void stickynotes_applet_update_icon(StickyNotesApplet *stickynotes, gboolean highlighted)
{
	GdkPixbuf *pixbuf1, *pixbuf2;

	gint size = panel_applet_get_size(PANEL_APPLET(stickynotes->applet));

	/* Choose appropriate icon and size it */
	if (highlighted)
	    	pixbuf1 = gdk_pixbuf_scale_simple(stickynotes->pixbuf_prelight, size, size, GDK_INTERP_BILINEAR);
	else
	    	pixbuf1 = gdk_pixbuf_scale_simple(stickynotes->pixbuf_normal, size, size, GDK_INTERP_BILINEAR);
	
	/* Shift the icon if pressed */
	pixbuf2 = gdk_pixbuf_copy(pixbuf1);
	if (stickynotes->pressed)
		gdk_pixbuf_scale(pixbuf1, pixbuf2, 0, 0, size, size, 1, 1, 1, 1, GDK_INTERP_BILINEAR);

	/* Apply the finished pixbuf to the applet image */
	gtk_image_set_from_pixbuf(GTK_IMAGE(stickynotes->image), pixbuf2);

	g_object_unref(pixbuf1);
	g_object_unref(pixbuf2);
}

/* Update the Sticky Notes Applet tooltip */
void stickynotes_applet_update_tooltips(StickyNotesApplet *stickynotes)
{
	gint num = g_list_length(stickynotes->notes);
	
	gchar *tooltip = g_strdup_printf(ngettext("%s\n%d note", "%s\n%d notes", num), _("Sticky Notes"), num);
	gtk_tooltips_set_tip(stickynotes->tooltips, GTK_WIDGET(stickynotes->applet), tooltip, NULL);
	g_free(tooltip);
}

void stickynotes_applet_do_default_action(StickyNotesApplet *stickynotes)
{
	gboolean visible = gconf_client_get_bool(stickynotes->gconf_client, GCONF_PATH "/settings/visible", NULL);
	gboolean locked = gconf_client_get_bool(stickynotes->gconf_client, GCONF_PATH "/settings/locked", NULL);

	StickyNotesDefaultAction click_behavior = gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/settings/click_behavior", NULL);

	switch (click_behavior) {
		case STICKYNOTES_NEW:
			stickynotes_add(stickynotes);
			break;

		case STICKYNOTES_SET_VISIBLE:
			gconf_client_set_bool(stickynotes->gconf_client, GCONF_PATH "/settings/visible", !visible, NULL);
			break;

		case STICKYNOTES_SET_LOCKED:
			gconf_client_set_bool(stickynotes->gconf_client, GCONF_PATH "/settings/locked", !locked, NULL);
			break;
	}
}

void stickynotes_applet_create_preferences(StickyNotesApplet *stickynotes)
{
	GtkWidget *preferences_dialog;
	
	GtkAdjustment *width_adjust;
	GtkAdjustment *height_adjust;
	GtkWidget *sticky_check;
	GtkWidget *note_color;
	GtkWidget *click_behavior_menu;

	stickynotes->prefs = glade_xml_new(GLADE_PATH, "preferences_dialog", NULL);
	preferences_dialog = glade_xml_get_widget(stickynotes->prefs, "preferences_dialog");
	
	{
		GtkSizeGroup *size= gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
		gtk_size_group_add_widget(size, glade_xml_get_widget(stickynotes->prefs, "width_label"));
		gtk_size_group_add_widget(size, glade_xml_get_widget(stickynotes->prefs, "height_label"));
		gtk_size_group_add_widget(size, glade_xml_get_widget(stickynotes->prefs, "color_label"));
		g_object_unref(size);
	}
	    
	width_adjust = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(stickynotes->prefs, "width_spin")));
	height_adjust = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(stickynotes->prefs, "height_spin")));
	sticky_check = glade_xml_get_widget(stickynotes->prefs, "sticky_check");
	note_color = glade_xml_get_widget(stickynotes->prefs, "note_color");
	click_behavior_menu = glade_xml_get_widget(stickynotes->prefs, "click_behavior_menu");

	g_signal_connect(G_OBJECT(preferences_dialog), "response", G_CALLBACK(preferences_response_cb), stickynotes);

	g_signal_connect_swapped(G_OBJECT(width_adjust), "value-changed", G_CALLBACK(preferences_save_cb), stickynotes);
	g_signal_connect_swapped(G_OBJECT(height_adjust), "value-changed", G_CALLBACK(preferences_save_cb), stickynotes);
	g_signal_connect_swapped(G_OBJECT(sticky_check), "toggled", G_CALLBACK(preferences_save_cb), stickynotes);
	g_signal_connect_swapped(G_OBJECT(click_behavior_menu), "changed", G_CALLBACK(preferences_save_cb), stickynotes);
	
	g_signal_connect(G_OBJECT(note_color), "color_set", G_CALLBACK(preferences_color_cb), stickynotes);

	{
		gint width = gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/defaults/width", NULL);
		gint height = gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/defaults/height", NULL);
		gboolean stickyness = gconf_client_get_bool(stickynotes->gconf_client, GCONF_PATH "/settings/sticky", NULL);
		gint click_behavior = gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/settings/click_behavior", NULL);

		gtk_adjustment_set_value(width_adjust, width);
		gtk_adjustment_set_value(height_adjust, height);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sticky_check), stickyness);
		gtk_option_menu_set_history(GTK_OPTION_MENU(click_behavior_menu), click_behavior);
	}

	{
		GdkColor color;
		
		gchar *color_str = gconf_client_get_string(stickynotes->gconf_client, GCONF_PATH "/defaults/color", NULL);
		gdk_color_parse(color_str, &color);
		g_free(color_str);

		gnome_color_picker_set_i16(GNOME_COLOR_PICKER(note_color), color.red, color.green, color.blue, 65535);
	}
	
	gtk_widget_show(preferences_dialog);
}

void stickynotes_applet_create_about(StickyNotesApplet *stickynotes)
{
	GtkWidget *about_dialog;
	
	stickynotes->about = glade_xml_new(GLADE_PATH, "about_dialog", NULL);
	about_dialog = glade_xml_get_widget(stickynotes->about, "about_dialog");

	g_signal_connect(G_OBJECT(about_dialog), "response", G_CALLBACK(about_response_cb), stickynotes);

	/* FIXME : Hack because libglade does not properly set these */
	g_object_set(G_OBJECT(about_dialog), "name", _("Sticky Notes"), "version", VERSION);
	{
		GdkPixbuf *logo = gdk_pixbuf_new_from_file(STICKYNOTES_ICONDIR "/stickynotes.png", NULL);
		g_object_set(G_OBJECT(about_dialog), "logo", logo);
		g_object_unref(logo);
	}
	if (strcmp(_("translator_credits"), "translator_credits") == 0)
		g_object_set(G_OBJECT(about_dialog), "translator_credits", NULL);
	
	gtk_widget_show(about_dialog);
}
