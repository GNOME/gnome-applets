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
#include <stickynotes_applet_callbacks.h>
#include <stickynotes.h>

/* Applet Callback : Click the applet (turn depress on/off). */
gboolean applet_click_cb(GtkWidget *widget, GdkEventButton *event, PanelApplet *applet)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
		stickynotes->pressed = TRUE;
	
	else if (event->type == GDK_BUTTON_RELEASE && event->button == 1) {
		gint click_behavior = gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/settings/click_behavior", NULL);

                switch (click_behavior) {
			case 0:
				stickynote_new();
				stickynotes_save_all();
				break;
			case 1:
				if (stickynotes->hidden)
					stickynotes_show_all();
				else
					stickynotes_hide_all();
				break;
                }
		
		stickynotes->pressed = FALSE;
	}
	
	else
		return FALSE;

	stickynotes_applet_highlight(applet, TRUE);
	
	/* Let other handlers receive this event. */
	return FALSE;
}

/* Applet Callback : Resize the applet. */
gboolean applet_resize_cb(GtkWidget *widget, gint size, PanelApplet *applet)
{
	stickynotes->size = size;

	stickynotes_applet_highlight(applet, FALSE);

	return TRUE;
}

/* Applet Callback : Cross (enter or leave) the applet. */
gboolean applet_cross_cb(GtkWidget *widget, GdkEventCrossing *event, PanelApplet *applet)
{
	if (event->type == GDK_ENTER_NOTIFY || GTK_WIDGET_HAS_FOCUS(applet))
		stickynotes_applet_highlight(applet, TRUE);
	else /* (event->type == GDK_LEAVE_NOTIFY) */
		stickynotes_applet_highlight(applet, FALSE);
	
	return TRUE;
}

/* Applet Callback : On focus (in or out) of the applet. */
gboolean applet_focus_cb(GtkWidget *widget, GdkEventFocus *event, PanelApplet *applet)
{
	if (event->in)
		stickynotes_applet_highlight(applet, TRUE);
	else
		stickynotes_applet_highlight(applet, FALSE);

	return TRUE;
}

/* Applet Callback : Save all sticky notes. */
gboolean applet_save_cb(PanelApplet *applet)
{
	stickynotes_save_all();

	return TRUE;
}

/* Menu Callback : Create a new sticky note */
void menu_create_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname)
{
	stickynote_new();
	stickynotes_save_all();
}

/* Menu Callback : Destroy all sticky notes */
void menu_destroy_all_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname)
{
	GladeXML *glade = glade_xml_new(GLADE_PATH, "delete_all_dialog", NULL);
	GtkWidget *dialog = glade_xml_get_widget(glade, "delete_all_dialog");

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(sticky->applet));

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		while (g_list_length(stickynotes->notes) > 0)
		    stickynote_free(g_list_nth_data(stickynotes->notes, 0));
		stickynotes_save_all();
	}

	gtk_widget_destroy(dialog);
	g_object_unref(glade);
}

/* Menu Callback : Hide all Sticky notes */
void menu_hide_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname)
{
	stickynotes_hide_all();
}

/* Menu Callback : Reveal all Sticky notes */
void menu_show_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname)
{
	stickynotes_show_all();
}

/* Menu Callback : Configure preferences */
void menu_preferences_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname)
{
	GladeXML *glade = glade_xml_new(GLADE_PATH, "preferences_dialog", NULL);
	GtkWidget *dialog = glade_xml_get_widget(glade, "preferences_dialog");
	GtkAdjustment *width_adjust = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(glade, "width_spin")));
	GtkAdjustment *height_adjust = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(glade, "height_spin")));
	GtkWidget *sticky_check = glade_xml_get_widget(glade, "sticky_check");
	GtkWidget *note_color = glade_xml_get_widget(glade, "note_color");
	GtkWidget *click_behavior_menu = glade_xml_get_widget(glade, "click_behavior_menu");
	    
	g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(preferences_response_cb), glade);

	g_signal_connect_swapped(G_OBJECT(width_adjust), "value-changed", G_CALLBACK(preferences_save_cb), glade);
	g_signal_connect_swapped(G_OBJECT(height_adjust), "value-changed", G_CALLBACK(preferences_save_cb), glade);
	g_signal_connect_swapped(G_OBJECT(sticky_check), "toggled", G_CALLBACK(preferences_save_cb), glade);
	g_signal_connect_swapped(G_OBJECT(click_behavior_menu), "changed", G_CALLBACK(preferences_save_cb), glade);
	
	g_signal_connect(G_OBJECT(note_color), "color_set", G_CALLBACK(preferences_color_cb), glade);

	{
		gint width = gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/defaults/width", NULL);
		gint height = gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/defaults/height", NULL);
		gboolean stickyness = gconf_client_get_bool(stickynotes->gconf_client, GCONF_PATH "/settings/sticky", NULL);
		gint click_behavior = gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/settings/click_behavior", NULL);

		GdkColor color;
		{
			gchar *color_str = gconf_client_get_string(stickynotes->gconf_client, GCONF_PATH "/settings/body_color_prelight", NULL);
			gdk_color_parse(color_str, &color);
			g_free(color_str);
		}

		gtk_adjustment_set_value(width_adjust, width);
		gtk_adjustment_set_value(height_adjust, height);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sticky_check), stickyness);
		gtk_option_menu_set_history(GTK_OPTION_MENU(click_behavior_menu), click_behavior);

		gnome_color_picker_set_i16(GNOME_COLOR_PICKER(note_color), color.red, color.green, color.blue, 65535);
	}
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(sticky->applet));
	gtk_widget_show(dialog);
}

/* Menu Callback : Show help */
void menu_help_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname)
{
    	gnome_help_display("stickynotes_applet", "stickynotes-introduction", NULL);
}

/* Menu Callback : Display About window */
void menu_about_cb(BonoboUIComponent *uic, StickyNotesApplet *sticky, const gchar *verbname)
{
	GladeXML *glade = glade_xml_new(GLADE_PATH, "about_dialog", NULL);
	GtkWidget *dialog = glade_xml_get_widget(glade, "about_dialog");

	/* FIXME : Hack because libglade does not properly set these */
	g_object_set(G_OBJECT(dialog), "name", _("Sticky Notes"), "version", VERSION);
	
	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(sticky->applet));
	gtk_widget_show(dialog);
	
	g_object_unref(glade);
}

/* Preferences Callback : Save. */
void preferences_save_cb(GladeXML *glade)
{
	GtkAdjustment *width_adjust = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(glade, "width_spin")));
	GtkAdjustment *height_adjust = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(glade, "height_spin")));
	GtkWidget *sticky_check = glade_xml_get_widget(glade, "sticky_check");
	GtkWidget *click_behavior_menu = glade_xml_get_widget(glade, "click_behavior_menu");

	gint width = gtk_adjustment_get_value(width_adjust);
	gint height = gtk_adjustment_get_value(height_adjust);
	gint stickyness = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sticky_check));
	gint click_behavior = gtk_option_menu_get_history(GTK_OPTION_MENU(click_behavior_menu));

	gconf_client_set_int(stickynotes->gconf_client, GCONF_PATH "/defaults/width", width, NULL);
	gconf_client_set_int(stickynotes->gconf_client, GCONF_PATH "/defaults/height", height, NULL);
	gconf_client_set_bool(stickynotes->gconf_client, GCONF_PATH "/settings/sticky", stickyness, NULL);
	gconf_client_set_int(stickynotes->gconf_client, GCONF_PATH "/settings/click_behavior", click_behavior, NULL);
}

/* Preferences Callback : Change color. */
void preferences_color_cb(GnomeColorPicker *cp, guint r, guint g, guint b, guint a, GladeXML *glade)
{
	gchar *title_color, *title_color_prelight, *body_color, *body_color_prelight;

	/* Reduce RGB from 16-bit to 8-bit values */
	r /= 256;
	g /= 256;
	b /= 256;

	/* Calculate HTML-style hex specification for the 4 required colors */
	title_color = g_strdup_printf("#%.2x%.2x%.2x", (r*8)/10, (g*8)/10, (b*8)/10);
	title_color_prelight = g_strdup_printf("#%.2x%.2x%.2x", (r*9)/10, (g*9)/10, (b*9)/10);
	body_color = g_strdup_printf("#%.2x%.2x%.2x", (r*9)/10, (g*9)/10, (b*9)/10);
	body_color_prelight = g_strdup_printf("#%.2x%.2x%.2x", (r*10)/10, (g*10)/10, (b*10)/10);

	/* Set the GConf values */
	gconf_client_set_string(stickynotes->gconf_client, GCONF_PATH "/settings/title_color", title_color, NULL);
	gconf_client_set_string(stickynotes->gconf_client, GCONF_PATH "/settings/title_color_prelight", title_color_prelight, NULL);
	gconf_client_set_string(stickynotes->gconf_client, GCONF_PATH "/settings/body_color", body_color, NULL);
	gconf_client_set_string(stickynotes->gconf_client, GCONF_PATH "/settings/body_color_prelight", body_color_prelight, NULL);

	g_free(title_color);
	g_free(title_color_prelight);
	g_free(body_color);
	g_free(body_color_prelight);
}

/* Preferences Callback : Response. */
void preferences_response_cb(GtkDialog *dialog, gint response, GladeXML *glade)
{
	if (response == GTK_RESPONSE_HELP) {
		gnome_help_display("stickynotes_applet", "stickynotes-introduction", NULL);
	}
	
	else { /* if (response == GTK_RESPONSE_CLOSE || response == GTK_RESPONSE_NONE) */
		gtk_widget_destroy(GTK_WIDGET(dialog));
		g_object_unref(glade);
	}
}

/* Preferences Callback : Apply to existing notes. */
void preferences_apply_cb(GConfClient *client, guint cnxn_id, GConfEntry *entry, StickyNotesApplet *sticky)
{
	gint i;

	if (strcmp(entry->key, GCONF_PATH "/settings/sticky") == 0) {
		if (gconf_value_get_bool(entry->value)) {
			for (i = 0; i < g_list_length(stickynotes->notes); i++) {
				StickyNote *note = g_list_nth_data(stickynotes->notes, i);
				gtk_window_stick(GTK_WINDOW(note->window));
			}
		}
		else {
			for (i = 0; i < g_list_length(stickynotes->notes); i++) {
				StickyNote *note = g_list_nth_data(stickynotes->notes, i);
				gtk_window_unstick(GTK_WINDOW(note->window));
			}
		}
	}

	else if (strcmp(entry->key, GCONF_PATH "/settings/title_color") == 0
		    || strcmp(entry->key, GCONF_PATH "/settings/title_color_prelight") == 0
		    || strcmp(entry->key, GCONF_PATH "/settings/body_color") == 0
		    || strcmp(entry->key, GCONF_PATH "/settings/body_color_prelight") == 0) {
		for (i = 0; i < g_list_length(stickynotes->notes); i++) {
			StickyNote *note = g_list_nth_data(stickynotes->notes, i);
			stickynote_set_highlighted(note, FALSE);
		}
	}
}
