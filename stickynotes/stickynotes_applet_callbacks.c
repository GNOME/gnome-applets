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
gboolean applet_button_cb(GtkWidget *widget, GdkEventButton *event, StickyNotesApplet *applet)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
		applet->pressed = TRUE;
	
	else if (event->type == GDK_BUTTON_RELEASE && event->button == 1) {
		stickynotes_applet_do_default_action();	
		applet->pressed = FALSE;
	}
	
	else
		return FALSE;

	stickynotes_applet_update_icon(applet);
	
	return TRUE;
}

/* Applet Callback : Keypress on the applet. */
gboolean applet_key_cb(GtkWidget *widget, GdkEventKey *event, StickyNotesApplet *applet)
{
	if (event->type == GDK_KEY_PRESS && event->keyval == GDK_Return)
		stickynotes_applet_do_default_action();	
	else
		return FALSE;
	
	return TRUE;
}

/* Applet Callback : Cross (enter or leave) the applet. */
gboolean applet_cross_cb(GtkWidget *widget, GdkEventCrossing *event, StickyNotesApplet *applet)
{
	applet->prelighted = event->type == GDK_ENTER_NOTIFY || GTK_WIDGET_HAS_FOCUS(widget);

	stickynotes_applet_update_icon(applet);
	
	return FALSE;
}

/* Applet Callback : On focus (in or out) of the applet. */
gboolean applet_focus_cb(GtkWidget *widget, GdkEventFocus *event, StickyNotesApplet *applet)
{
	applet->prelighted = event->in;

	stickynotes_applet_update_icon(applet);

	return FALSE;
}

/* Applet Callback : Save all sticky notes. */
gboolean applet_save_cb(StickyNotesApplet *applet)
{
	stickynotes_save();

	return TRUE;
}

/* Applet Callback : Resize the applet. */
gboolean applet_change_size_cb(GtkWidget *widget, gint size, StickyNotesApplet *applet)
{
	stickynotes_applet_update_icon(applet);

	return FALSE;
}

/* Applet Callback : Change the applet background. */
gboolean applet_change_bg_cb(PanelApplet *panel_applet, PanelAppletBackgroundType type, GdkColor *color, GdkPixmap *pixmap,
			     StickyNotesApplet *applet)
{
	if (type == PANEL_NO_BACKGROUND) {
		GtkRcStyle *rc_style = gtk_rc_style_new();
		gtk_widget_modify_style(applet->w_applet, rc_style);
		gtk_rc_style_unref(rc_style);
	}

	else if (type == PANEL_COLOR_BACKGROUND) {
		gtk_widget_modify_bg(applet->w_applet, GTK_STATE_NORMAL, color);
	}

	else /* if (type == PANEL_PIXMAP_BACKGROUND) */ {
		gdk_window_set_back_pixmap(applet->w_applet->window, pixmap, FALSE); /* FIXME : Does not work */
	}
	
	return FALSE;
}

/* Menu Callback : Create a new sticky note */
void menu_create_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
{
	stickynotes_add();
}

/* Menu Callback : Destroy all sticky notes */
void menu_destroy_all_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
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
		
	stickynotes_applet_update_tooltips();
	stickynotes_save();

	gtk_widget_destroy(dialog);
	g_object_unref(glade);
}

/* Menu Callback : Show/Hide sticky notes */
void menu_toggle_show_cb(BonoboUIComponent *uic, const gchar *path, Bonobo_UIComponent_EventType type, const gchar *state, StickyNotesApplet *applet)
{
	if (gconf_client_key_is_writable (stickynotes->gconf, GCONF_PATH "/settings/visible", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/visible", strcmp(state, "0") != 0, NULL);
}

/* Menu Callback: Lock/Unlock sticky notes */
void menu_toggle_lock_cb(BonoboUIComponent *uic, const gchar *path, Bonobo_UIComponent_EventType type, const gchar *state, StickyNotesApplet *applet)
{
	if (gconf_client_key_is_writable (stickynotes->gconf, GCONF_PATH "/settings/locked", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/locked", strcmp(state, "0") != 0, NULL);
}

/* Menu Callback : Configure preferences */
void menu_preferences_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
{
	stickynotes_applet_update_prefs();
	gtk_window_present(GTK_WINDOW(stickynotes->w_prefs));
}

/* Menu Callback : Show help */
void menu_help_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
{
    	gnome_help_display("stickynotes_applet", "stickynotes-introduction", NULL);
}

/* Menu Callback : Display About window */
void menu_about_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
{
	gtk_window_present(GTK_WINDOW(stickynotes->w_about));
}

/* Preferences Callback : Save. */
void preferences_save_cb(gpointer data)
{
	gint width = gtk_adjustment_get_value(stickynotes->w_prefs_width);
	gint height = gtk_adjustment_get_value(stickynotes->w_prefs_height);
	gboolean sys_color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_sys_color));
	gboolean sys_font = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_sys_font));
	gint click_behavior = gtk_option_menu_get_history(GTK_OPTION_MENU(stickynotes->w_prefs_click));
	gboolean sticky = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_sticky));
	gboolean force_default = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_force));

	/* FIXME: This is so ugly, we are setting everything whenever we change anything,
	   this is why key-writability is even a bigger mess here */


	if (gconf_client_key_is_writable (stickynotes->gconf,
					  GCONF_PATH "/defaults/width", NULL))
		gconf_client_set_int(stickynotes->gconf, GCONF_PATH "/defaults/width", width, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
					  GCONF_PATH "/defaults/height", NULL))
		gconf_client_set_int(stickynotes->gconf, GCONF_PATH "/defaults/height", height, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
					  GCONF_PATH "/settings/use_system_color", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/use_system_color", sys_color, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
					  GCONF_PATH "/settings/use_system_font", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/use_system_font", sys_font, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
					  GCONF_PATH "/settings/click_behavior", NULL))
		gconf_client_set_int(stickynotes->gconf, GCONF_PATH "/settings/click_behavior", click_behavior, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
					  GCONF_PATH "/settings/sticky", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/sticky", sticky, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
					  GCONF_PATH "/settings/force_default", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/force_default", force_default, NULL);
}

/* Preferences Callback : Change color. */
void preferences_color_cb(GnomeColorPicker *cp, guint r, guint g, guint b, guint a, gpointer data)
{
	/* Reduce RGB from 16-bit to 8-bit values and calculate HTML-style hex specification for the color */
	gchar *color_str = g_strdup_printf("#%.2x%.2x%.2x", r / 256, g / 256, b / 256);
	gconf_client_set_string(stickynotes->gconf, GCONF_PATH "/defaults/color", color_str, NULL);
	g_free(color_str);
}

/* Preferences Callback : Change font. */
void preferences_font_cb(GnomeFontPicker *fp, gchar *font_str, gpointer data)
{
	gconf_client_set_string(stickynotes->gconf, GCONF_PATH "/defaults/font", font_str, NULL);
}

/* Preferences Callback : Apply to existing notes. */
void preferences_apply_cb(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer data)
{
	gint i;

	if (strcmp(entry->key, GCONF_PATH "/settings/sticky") == 0) {
		if (gconf_value_get_bool(entry->value))
			for (i = 0; i < g_list_length(stickynotes->notes); i++) {
				StickyNote *note = g_list_nth_data(stickynotes->notes, i);
				gtk_window_stick(GTK_WINDOW(note->w_window));
			}
		else
			for (i = 0; i < g_list_length(stickynotes->notes); i++) {
				StickyNote *note = g_list_nth_data(stickynotes->notes, i);
				gtk_window_unstick(GTK_WINDOW(note->w_window));
			}
	}

	else if (strcmp(entry->key, GCONF_PATH "/settings/locked") == 0) {
		for (i = 0; i < g_list_length(stickynotes->notes); i++) {
			StickyNote *note = g_list_nth_data(stickynotes->notes, i);
			stickynote_set_locked(note, gconf_value_get_bool(entry->value));
		}
		stickynotes_save();
	}

	else if (strcmp(entry->key, GCONF_PATH "/settings/visible") == 0) {
		for (i = 0; i < g_list_length(stickynotes->notes); i++) {
			StickyNote *note = g_list_nth_data(stickynotes->notes, i);
			stickynote_set_visible(note, gconf_value_get_bool(entry->value));
		}
		stickynotes_save();
	}

	else if (strcmp(entry->key, GCONF_PATH "/settings/use_system_color") == 0 ||
	    strcmp(entry->key, GCONF_PATH "/defaults/color") == 0) {
		for (i = 0; i < g_list_length(stickynotes->notes); i++) {
			StickyNote *note = g_list_nth_data(stickynotes->notes, i);
			stickynote_set_color(note, note->color, FALSE);
		}
	}

	else if (strcmp(entry->key, GCONF_PATH "/settings/use_system_font") == 0 ||
	    strcmp(entry->key, GCONF_PATH "/defaults/font") == 0) {
		for (i = 0; i < g_list_length(stickynotes->notes); i++) {
			StickyNote *note = g_list_nth_data(stickynotes->notes, i);
			stickynote_set_font(note, note->font, FALSE);
		}
	}

	else if (strcmp(entry->key, GCONF_PATH "/settings/force_default") == 0) {
		for (i = 0; i < g_list_length(stickynotes->notes); i++) {
			StickyNote *note = g_list_nth_data(stickynotes->notes, i);
			stickynote_set_color(note, note->color, FALSE);
			stickynote_set_font(note, note->font, FALSE);
		}
	}

	stickynotes_applet_update_prefs();
	stickynotes_applet_update_menus();
}

/* Preferences Callback : Response. */
void preferences_response_cb(GtkDialog *dialog, gint response, gpointer data)
{
	if (response == GTK_RESPONSE_HELP)
		gnome_help_display("stickynotes_applet", "stickynotes-introduction", NULL);
	
	else /* if (response == GTK_RESPONSE_CLOSE  || response == GTK_RESPONSE_DELETE_EVENT || response == GTK_RESPONSE_NONE)*/
		gtk_widget_hide(GTK_WIDGET(dialog));
}

/* About Callback : Response. */
void about_response_cb(GtkDialog *dialog, gint response, gpointer data)
{
	if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_DELETE_EVENT || response == GTK_RESPONSE_NONE)
		gtk_widget_hide(GTK_WIDGET(dialog));
}
