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

/* Applet Callback : Mouse button press on the applet. */
gboolean applet_button_cb(GtkWidget *widget, GdkEventButton *event, StickyNotesApplet *stickynotes)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
		stickynotes->pressed = TRUE;
	
	else if (event->type == GDK_BUTTON_RELEASE && event->button == 1) {
		stickynotes_applet_do_default_action(stickynotes);	
		stickynotes->pressed = FALSE;
	}
	
	else
		return FALSE;

	stickynotes_applet_update_icon(stickynotes, TRUE);
	
	/* Let other handlers receive this event. */
	return FALSE;
}

/* Applet Callback : Keypress on the applet. */
gboolean applet_key_cb(GtkWidget *widget, GdkEventKey *event, StickyNotesApplet *stickynotes)
{
	if (event->type == GDK_KEY_PRESS && event->keyval == GDK_Return)
		stickynotes->pressed = TRUE;
	
	else if (event->type == GDK_KEY_RELEASE && event->keyval == GDK_Return) {
		stickynotes_applet_do_default_action(stickynotes);	
		stickynotes->pressed = FALSE;
	}
	
	else
		return FALSE;
	
	stickynotes_applet_update_icon(stickynotes, TRUE);
	
	/* Let other handlers receive this event. */
	return FALSE;
}

/* Applet Callback : Cross (enter or leave) the applet. */
gboolean applet_cross_cb(GtkWidget *widget, GdkEventCrossing *event, StickyNotesApplet *stickynotes)
{
	stickynotes_applet_update_icon(stickynotes, event->type == GDK_ENTER_NOTIFY || GTK_WIDGET_HAS_FOCUS(widget));
	
	/* Let other handlers receive this event. */
	return FALSE;
}

/* Applet Callback : On focus (in or out) of the applet. */
gboolean applet_focus_cb(GtkWidget *widget, GdkEventFocus *event, StickyNotesApplet *stickynotes)
{
	stickynotes_applet_update_icon(stickynotes, event->in);

	/* Let other handlers receive this event. */
	return FALSE;
}

/* Applet Callback : Save all sticky notes. */
gboolean applet_save_cb(StickyNotesApplet *stickynotes)
{
	stickynotes_save(stickynotes);

	return TRUE;
}

/* Applet Callback : Resize the applet. */
gboolean applet_change_size_cb(GtkWidget *widget, gint size, StickyNotesApplet *stickynotes)
{
	stickynotes_applet_update_icon(stickynotes, FALSE);

	/* Let other handlers receive this event. */
	return FALSE;
}

/* Applet Callback : Change the applet background. */
gboolean applet_change_bg_cb(PanelApplet *applet, PanelAppletBackgroundType type, GdkColor *color, GdkPixmap *pixmap, StickyNotesApplet *stickynotes)
{
	if (type == PANEL_NO_BACKGROUND) {
		GtkRcStyle *rc_style = gtk_rc_style_new();
		gtk_widget_modify_style(GTK_WIDGET(applet), rc_style);
		gtk_rc_style_unref(rc_style);
	}

	else if (type == PANEL_COLOR_BACKGROUND) {
		gtk_widget_modify_bg(GTK_WIDGET(applet), GTK_STATE_NORMAL, color);
	}

	else /* if (type == PANEL_PIXMAP_BACKGROUND) */ {
		gdk_window_set_back_pixmap(GTK_WIDGET(applet)->window, pixmap, FALSE); /* FIXME : Does not work */
	}
	
	/* Let other handlers receive this event. */
	return FALSE;
}

/* Menu Callback : Create a new sticky note */
void menu_create_cb(BonoboUIComponent *uic, StickyNotesApplet *stickynotes, const gchar *verbname)
{
	stickynotes_add(stickynotes);
}

/* Menu Callback : Destroy all sticky notes */
void menu_destroy_all_cb(BonoboUIComponent *uic, StickyNotesApplet *stickynotes, const gchar *verbname)
{
	GladeXML *glade = glade_xml_new(GLADE_PATH, "delete_all_dialog", NULL);
	GtkWidget *dialog = glade_xml_get_widget(glade, "delete_all_dialog");

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		while (g_list_length(stickynotes->notes) > 0) {
			StickyNote *note = g_list_nth_data(stickynotes->notes, 0);
			stickynote_free(note);
			stickynotes->notes = g_list_remove(stickynotes->notes, note);
		}
	}
		
	stickynotes_applet_update_tooltips(stickynotes);
	stickynotes_save(stickynotes);

	gtk_widget_destroy(dialog);
	g_object_unref(glade);
}

/* Menu Callback : Hide all Sticky notes */
void menu_hide_cb(BonoboUIComponent *uic, StickyNotesApplet *stickynotes, const gchar *verbname)
{
	gconf_client_set_bool(stickynotes->gconf_client, GCONF_PATH "/settings/visible", FALSE, NULL);
}

/* Menu Callback : Reveal all Sticky notes */
void menu_show_cb(BonoboUIComponent *uic, StickyNotesApplet *stickynotes, const gchar *verbname)
{
	gconf_client_set_bool(stickynotes->gconf_client, GCONF_PATH "/settings/visible", TRUE, NULL);
}

/* Menu Callback : Lock all Sticky notes from editing */
void menu_lock_cb(BonoboUIComponent *uic, StickyNotesApplet *stickynotes, const gchar *verbname)
{
	gconf_client_set_bool(stickynotes->gconf_client, GCONF_PATH "/settings/locked", TRUE, NULL);
}

/* Menu Callback : Unlock all Sticky notes for editiing */
void menu_unlock_cb(BonoboUIComponent *uic, StickyNotesApplet *stickynotes, const gchar *verbname)
{
	gconf_client_set_bool(stickynotes->gconf_client, GCONF_PATH "/settings/locked", FALSE, NULL);
}

/* Menu Callback : Configure preferences */
void menu_preferences_cb(BonoboUIComponent *uic, StickyNotesApplet *stickynotes, const gchar *verbname)
{
	if (stickynotes->prefs == NULL)
		stickynotes_applet_create_preferences(stickynotes);
	else
		gdk_window_raise(glade_xml_get_widget(stickynotes->prefs, "preferences_dialog")->window);
	
}

/* Menu Callback : Show help */
void menu_help_cb(BonoboUIComponent *uic, StickyNotesApplet *stickynotes, const gchar *verbname)
{
    	gnome_help_display("stickynotes_applet", "stickynotes-introduction", NULL);
}

/* Menu Callback : Display About window */
void menu_about_cb(BonoboUIComponent *uic, StickyNotesApplet *stickynotes, const gchar *verbname)
{
	if (stickynotes->about == NULL)
		stickynotes_applet_create_about(stickynotes);
	else
		gdk_window_raise(glade_xml_get_widget(stickynotes->about, "about_dialog")->window);
}

/* Preferences Callback : Save. */
void preferences_save_cb(StickyNotesApplet *stickynotes)
{
	GtkAdjustment *width_adjust = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(stickynotes->prefs, "width_spin")));
	GtkAdjustment *height_adjust = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(stickynotes->prefs, "height_spin")));
	GtkWidget *sticky_check = glade_xml_get_widget(stickynotes->prefs, "sticky_check");
	GtkWidget *click_behavior_menu = glade_xml_get_widget(stickynotes->prefs, "click_behavior_menu");

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
void preferences_color_cb(GnomeColorPicker *cp, guint r, guint g, guint b, guint a, StickyNotesApplet *stickynotes)
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

/* Preferences Callback : Apply to existing notes. */
void preferences_apply_cb(GConfClient *client, guint cnxn_id, GConfEntry *entry, StickyNotesApplet *stickynotes)
{
	gint i;

	if (strcmp(entry->key, GCONF_PATH "/settings/sticky") == 0) {
		if (gconf_value_get_bool(entry->value))
			for (i = 0; i < g_list_length(stickynotes->notes); i++) {
				StickyNote *note = g_list_nth_data(stickynotes->notes, i);
				gtk_window_stick(GTK_WINDOW(note->window));
			}
		else
			for (i = 0; i < g_list_length(stickynotes->notes); i++) {
				StickyNote *note = g_list_nth_data(stickynotes->notes, i);
				gtk_window_unstick(GTK_WINDOW(note->window));
			}
	}

	else if (strcmp(entry->key, GCONF_PATH "/settings/visible") == 0) {
		stickynotes_set_visible(stickynotes, gconf_value_get_bool(entry->value));
	}

	else if (strcmp(entry->key, GCONF_PATH "/settings/locked") == 0) {
		stickynotes_set_locked(stickynotes, gconf_value_get_bool(entry->value));
		stickynotes_applet_update_tooltips(stickynotes);
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

/* Preferences Callback : Response. */
void preferences_response_cb(GtkDialog *dialog, gint response, StickyNotesApplet *stickynotes)
{
	if (response == GTK_RESPONSE_HELP)
		gnome_help_display("stickynotes_applet", "stickynotes-introduction", NULL);
	
	else /* if (response == GTK_RESPONSE_CLOSE || response == GTK_RESPONSE_NONE) || GTK_RESPONSE_DELETE_EVENT */ {
		gtk_widget_destroy(GTK_WIDGET(dialog));
		g_object_unref(stickynotes->prefs);
		stickynotes->prefs = NULL;
	}
}

/* About Callback : Response. */
void about_response_cb(GtkDialog *dialog, gint response, StickyNotesApplet *stickynotes)
{
	if (response == GTK_RESPONSE_DELETE_EVENT || response == GTK_RESPONSE_OK || response == GTK_RESPONSE_CLOSE) {
		gtk_widget_destroy(GTK_WIDGET(dialog));
		g_object_unref(stickynotes->about);
		stickynotes->about = NULL;
	}
}
