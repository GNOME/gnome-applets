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

#include <libgnome/gnome-help.h>

/* Applet Callback : Mouse button press on the applet. */
gboolean applet_button_cb(GtkWidget *widget, GdkEventButton *event, StickyNotesApplet *applet)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
		applet->pressed = TRUE;
	
	else if (event->type == GDK_BUTTON_RELEASE && event->button == 1) {
		stickynotes_applet_do_default_action(gtk_widget_get_screen(applet->w_applet));	
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
		stickynotes_applet_do_default_action(gtk_widget_get_screen(applet->w_applet));	
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

/* Applet Callback : Change the panel orientation. */
void applet_change_orient_cb(PanelApplet *panel_applet, PanelAppletOrient orient, StickyNotesApplet *applet)
{
	applet->panel_orient = orient;

	return;
}

/* Applet Callback : Resize the applet. */
void applet_size_allocate_cb(GtkWidget *widget, GtkAllocation *allocation, StickyNotesApplet *applet)
{
	if ((applet->panel_orient == PANEL_APPLET_ORIENT_UP) || (applet->panel_orient == PANEL_APPLET_ORIENT_DOWN)) {
	  if (applet->panel_size == allocation->height)
	    return;
	  applet->panel_size = allocation->height;
	} else {
	  if (applet->panel_size == allocation->width)
	    return;
	  applet->panel_size = allocation->width;
	}

	stickynotes_applet_update_icon(applet);

	return;
}

/* Applet Callback : Change the applet background. */
void
applet_change_bg_cb (PanelApplet *panel_applet,
		     PanelAppletBackgroundType type,
		     GdkColor *color,
		     GdkPixmap *pixmap,
		     StickyNotesApplet *applet)
{
	/* Taken from TrashApplet */
	GtkRcStyle *rc_style;
	GtkStyle *style;

	if (!applet) g_print ("arrg, no applet!\n");

	/* reset style */
	gtk_widget_set_style (GTK_WIDGET (applet->w_applet), NULL);
	rc_style = gtk_rc_style_new ();
	gtk_widget_modify_style (GTK_WIDGET (applet->w_applet), rc_style);
	g_object_unref (rc_style);

	switch (type)
	{
		case PANEL_NO_BACKGROUND:
			break;
		case PANEL_COLOR_BACKGROUND:
			gtk_widget_modify_bg (GTK_WIDGET (applet->w_applet),
					GTK_STATE_NORMAL, color);
			break;
		case PANEL_PIXMAP_BACKGROUND:
			style = gtk_style_copy (
					GTK_WIDGET (applet->w_applet)->style);
			if (style->bg_pixmap[GTK_STATE_NORMAL])
				g_object_unref (
					style->bg_pixmap[GTK_STATE_NORMAL]);
			style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref (
					pixmap);
			gtk_widget_set_style (
					GTK_WIDGET (applet->w_applet), style);
			break;
	}
}

/* Applet Callback : Deletes the applet. */
void applet_destroy_cb (PanelApplet *panel_applet, StickyNotesApplet *applet)
{
	GList *notes;
	
	if (applet->destroy_all_dialog != NULL)
		gtk_widget_destroy (applet->destroy_all_dialog);

	if (applet->about_dialog != NULL)
		gtk_widget_destroy (applet->about_dialog);
	
	if (stickynotes->applets != NULL)
		stickynotes->applets = g_list_remove (stickynotes->applets, applet);
		
	if (stickynotes->applets == NULL) {
               notes = stickynotes->notes;
               while (notes) {
                       StickyNote *note = notes->data;
                       stickynote_free (note);
                       notes = g_list_next (notes);
               }
	}
	
	
}		

/* Menu Callback : Create a new sticky note */
void menu_create_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
{
	stickynotes_add(gtk_widget_get_screen(applet->w_applet));
}

/* Destroy all response Callback: Callback for the destroy all dialog */
static void
destroy_all_response_cb (GtkDialog *dialog, gint id, StickyNotesApplet *applet)
{
	if (id == GTK_RESPONSE_OK) {
		while (g_list_length(stickynotes->notes) > 0) {
			StickyNote *note = g_list_nth_data(stickynotes->notes, 0);
			stickynote_free(note);
			stickynotes->notes = g_list_remove(stickynotes->notes, note);
		}							       
	}

	stickynotes_applet_update_tooltips();
	stickynotes_save();

	gtk_widget_destroy (GTK_WIDGET (dialog));
	applet->destroy_all_dialog = NULL;
}

/* Menu Callback : Destroy all sticky notes */
void menu_destroy_all_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
{
	GladeXML *glade = glade_xml_new(GLADE_PATH, "delete_all_dialog", NULL);

	if (applet->destroy_all_dialog != NULL) {
		gtk_window_set_screen (GTK_WINDOW (applet->destroy_all_dialog),
				       gtk_widget_get_screen (GTK_WIDGET (applet->w_applet)));

		gtk_window_present (GTK_WINDOW (applet->destroy_all_dialog));
		return;
	}
	
	applet->destroy_all_dialog = glade_xml_get_widget(glade, "delete_all_dialog");

	g_object_unref (glade);

	g_signal_connect (applet->destroy_all_dialog, "response",
			  G_CALLBACK (destroy_all_response_cb),
			  applet);

	gtk_window_set_screen (GTK_WINDOW (applet->destroy_all_dialog),
			gtk_widget_get_screen (applet->w_applet));

	gtk_widget_show_all (applet->destroy_all_dialog);
}

/* Menu Callback : Show/Hide sticky notes */
void menu_toggle_show_cb(BonoboUIComponent *uic, const gchar *path, Bonobo_UIComponent_EventType type, const gchar *state, StickyNotesApplet *applet)
{
	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/visible", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/visible", strcmp(state, "0") != 0, NULL);
}

/* Menu Callback: Lock/Unlock sticky notes */
void menu_toggle_lock_cb(BonoboUIComponent *uic, const gchar *path, Bonobo_UIComponent_EventType type, const gchar *state, StickyNotesApplet *applet)
{
	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/locked", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/locked", strcmp(state, "0") != 0, NULL);
}

/* Menu Callback : Configure preferences */
void menu_preferences_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
{
	stickynotes_applet_update_prefs();
	gtk_window_set_screen(GTK_WINDOW(stickynotes->w_prefs), gtk_widget_get_screen(applet->w_applet));
	gtk_window_present(GTK_WINDOW(stickynotes->w_prefs));
}

/* Menu Callback : Show help */
void menu_help_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
{
	GError *error = NULL;
	gnome_help_display_on_screen("stickynotes_applet", NULL, gtk_widget_get_screen(applet->w_applet), &error);
	if (error) {
		GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
							   _("There was an error displaying help: %s"), error->message);
		g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(gtk_widget_destroy), NULL);
		gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW(dialog), gtk_widget_get_screen(applet->w_applet));
		gtk_widget_show(dialog);
		g_error_free(error);
	}
}

/* Menu Callback : Display About window */
void menu_about_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
{
	static const gchar *authors[] = {
		"Loban A Rahman <loban@earthling.net>",
		NULL
	};

	static const gchar *documenters[] = {
		"Loban A Rahman <loban@earthling.net>",
		"Sun GNOME Documentation Team <gdocteam@sun.com>",
		NULL
	};

	const gchar *translator_credits = _("translator_credits");

	if (applet->about_dialog) {
		gtk_window_set_screen (GTK_WINDOW (applet->about_dialog),
				       gtk_widget_get_screen (GTK_WIDGET (applet->w_applet)));

		gtk_window_present (GTK_WINDOW (applet->about_dialog));
		return;
	}
	
	applet->about_dialog = gnome_about_new(_("Sticky Notes"),
					       VERSION,
					       _("(c) 2002-2003 Loban A Rahman"),
					       _("Sticky Notes for the Gnome Desktop Environment"),
					       authors,
					       documenters,
					       strcmp(translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
					       stickynotes->icon_normal);

	gtk_window_set_screen (GTK_WINDOW (applet->about_dialog),
			       gtk_widget_get_screen (GTK_WIDGET (applet->w_applet)));

	g_signal_connect (applet->about_dialog, "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &applet->about_dialog);
	
	gtk_widget_show(applet->about_dialog);
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

	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/defaults/width", NULL))
		gconf_client_set_int(stickynotes->gconf, GCONF_PATH "/defaults/width", width, NULL);
	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/defaults/height", NULL))
		gconf_client_set_int(stickynotes->gconf, GCONF_PATH "/defaults/height", height, NULL);
	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/use_system_color", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/use_system_color", sys_color, NULL);
	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/use_system_font", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/use_system_font", sys_font, NULL);
	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/click_behavior", NULL))
		gconf_client_set_int(stickynotes->gconf, GCONF_PATH "/settings/click_behavior", click_behavior, NULL);
	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/sticky", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/sticky", sticky, NULL);
	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/force_default", NULL))
		gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/force_default", force_default, NULL);
}

/* Preferences Callback : Change color. */
void
preferences_color_cb (GtkWidget *button, gpointer data)
{
	GdkColor color;

	gtk_color_button_get_color (GTK_COLOR_BUTTON (button), &color);
	
	gchar *color_str = g_strdup_printf("#%.2x%.2x%.2x",
			color.red / 256,
			color.green / 256,
			color.blue / 256);
	gconf_client_set_string(stickynotes->gconf,
			GCONF_PATH "/defaults/color", color_str, NULL);
}

/* Preferences Callback : Change font. */
void preferences_font_cb (GtkWidget *button, gpointer data)
{
	char *font_str;

	font_str = gtk_font_button_get_font_name (GTK_FONT_BUTTON (button));
	gconf_client_set_string(stickynotes->gconf,
			GCONF_PATH "/defaults/font", font_str, NULL);
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
	if (response == GTK_RESPONSE_HELP) {
		GError *error = NULL;
		gnome_help_display_on_screen("stickynotes_applet", "stickynotes-advanced-settings", gtk_widget_get_screen(GTK_WIDGET(dialog)), &error);
		if (error) {
			GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
								   _("There was an error displaying help: %s"), error->message);
			g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(gtk_widget_destroy), NULL);
			gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
			gtk_window_set_screen (GTK_WINDOW(dialog), gtk_widget_get_screen(GTK_WIDGET(dialog)));
			gtk_widget_show(dialog);
			g_error_free(error);
		}
	}

	else
		gtk_widget_hide(GTK_WIDGET(dialog));
}

/* Preferences Callback : Delete */
gboolean preferences_delete_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_widget_hide(widget);

	return TRUE;
}
