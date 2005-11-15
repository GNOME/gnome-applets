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
#include <string.h>
#include "stickynotes_applet_callbacks.h"
#include "stickynotes.h"
#include <gdk/gdkkeysyms.h>
#include <libgnomeui/gnome-help.h>
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

static gboolean get_desktop_window (Window *window)
{
	Window *desktop_window;
	GdkWindow *root_window;
	GdkAtom type_returned;
	int format_returned;
	int length_returned;

	root_window = gdk_screen_get_root_window (
				gdk_screen_get_default ());

	if (gdk_property_get (root_window,
			      gdk_atom_intern ("NAUTILUS_DESKTOP_WINDOW_ID", FALSE),
			      gdk_x11_xatom_to_atom (XA_WINDOW),
			      0, 4, FALSE,
			      &type_returned,
			      &format_returned,
			      &length_returned,
			      (guchar**) &desktop_window)) {
		*window = *desktop_window;
		g_free (desktop_window);
		return TRUE;
	}
	else {
		*window = 0;
		return FALSE;
	}
}

static void
popup_add_note (StickyNotesApplet *applet, GtkWidget *item)
{
	GList *l;

	stickynotes_add (gtk_widget_get_screen (applet->w_applet));
	for (l = stickynotes->applets; l; l = l->next)
        {
                applet = l->data;

                gtk_check_menu_item_set_active (
                                GTK_CHECK_MENU_ITEM (applet->menu_show),
                                TRUE);
        }
}

static void
stickynote_show_notes (gboolean visible)
{
	StickyNote *note;
	GList *l;

	for (l = stickynotes->notes; l; l = l->next)
	{
		note = l->data;
		stickynote_set_visible (note, visible);
	}
}

/* Applet Callback : Mouse button press on the applet. */
gboolean
applet_button_cb (GtkWidget         *widget,
		  GdkEventButton    *event,
		  StickyNotesApplet *applet)
{
	if (event->type == GDK_2BUTTON_PRESS)
	{
		popup_add_note (applet, NULL);
		return TRUE;
	}
	else if (event->button == 1)
	{
		stickynote_show_notes (TRUE);
		return TRUE;
	}

	return FALSE;
}

/* Applet Callback : Keypress on the applet. */
gboolean
applet_key_cb (GtkWidget         *widget,
	       GdkEventKey       *event,
	       StickyNotesApplet *applet)
{
	switch (event->keyval)
	{
		case GDK_KP_Space:
		case GDK_space:
		case GDK_KP_Enter:
		case GDK_Return:
			stickynote_show_notes (TRUE);
			return TRUE;
	}
 	return FALSE;
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

/* Applet Callback : Click on the desktop background */
gboolean applet_check_click_on_desktop_cb(gpointer data)
{
	static gboolean first_time = TRUE;
	static Display *dpy;
	Window desktop_window;
	char *name;

	if (first_time) {
		dpy = XOpenDisplay (NULL);
		first_time = FALSE;
	}

	if (!get_desktop_window (&desktop_window)) {
		return TRUE;
	}
	/* X does not let us monitor mouse clicks in two applications
	 * so we look at the PropertyChange event which is also fired
	 * at every click on the desktop */
	XSelectInput(dpy, desktop_window, PropertyChangeMask);

	XEvent event;

	if (XCheckWindowEvent(dpy, desktop_window, PropertyChangeMask, &event) == True) {
		XPropertyEvent *property_event;
		property_event = (XPropertyEvent*) &event;
		name = XGetAtomName (dpy, property_event->atom);

		if ((property_event->state == PropertyNewValue) && (strcmp (name, "_NET_WM_USER_TIME") == 0)) { 
			stickynote_show_notes (FALSE);
		} 

		if (name) {
			XFree (name);
		}
	}
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
	gtk_rc_style_unref (rc_style);

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
			g_object_unref (style);
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

/* Menu Callback : New Note */
void menu_new_note_cb(BonoboUIComponent *uic, StickyNotesApplet *applet, const gchar *verbname)
{
	popup_add_note (applet, NULL);
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
void
menu_about_cb (BonoboUIComponent *uic,
	       StickyNotesApplet *applet,
	       const gchar *verbname)
{
	static const gchar *authors[] = {
		"Loban A Rahman <loban@earthling.net>",
		"Davyd Madeley <davyd@madeley.id.au>",
		NULL
	};

	static const gchar *documenters[] = {
		"Loban A Rahman <loban@earthling.net>",
		"Sun GNOME Documentation Team <gdocteam@sun.com>",
		NULL
	};

	gtk_show_about_dialog (NULL,
		"name",		_("Sticky Notes"),
		"version",	VERSION,
		"copyright",	"\xC2\xA9 2002-2003 Loban A Rahman, "
				"\xC2\xA9 2005 Davyd Madeley",
		"comments",	_("Sticky Notes for the "
				  "GNOME Desktop Environment"),
		"authors",	authors,
		"documenters",	documenters,
		"translator-credits",	_("translator-credits"),
		"logo-icon-name",	"stock_notes",
		NULL);
}

/* Preferences Callback : Save. */
void
preferences_save_cb (gpointer data)
{
	gint width = gtk_adjustment_get_value (stickynotes->w_prefs_width);
	gint height = gtk_adjustment_get_value (stickynotes->w_prefs_height);
	gboolean sys_color = gtk_toggle_button_get_active (
			GTK_TOGGLE_BUTTON (stickynotes->w_prefs_sys_color));
	gboolean sys_font = gtk_toggle_button_get_active (
			GTK_TOGGLE_BUTTON (stickynotes->w_prefs_sys_font));
	gboolean sticky = gtk_toggle_button_get_active (
			GTK_TOGGLE_BUTTON (stickynotes->w_prefs_sticky));
	gboolean force_default = gtk_toggle_button_get_active (
			GTK_TOGGLE_BUTTON (stickynotes->w_prefs_force));

	if (gconf_client_key_is_writable (stickynotes->gconf,
				GCONF_PATH "/defaults/width", NULL))
		gconf_client_set_int (stickynotes->gconf,
				GCONF_PATH "/defaults/width", width, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
				GCONF_PATH "/defaults/height", NULL))
		gconf_client_set_int (stickynotes->gconf,
				GCONF_PATH "/defaults/height", height, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
				GCONF_PATH "/settings/use_system_color", NULL))
		gconf_client_set_bool (stickynotes->gconf,
				GCONF_PATH "/settings/use_system_color",
				sys_color, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
				GCONF_PATH "/settings/use_system_font", NULL))
		gconf_client_set_bool (stickynotes->gconf,
				GCONF_PATH "/settings/use_system_font",
				sys_font, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
				GCONF_PATH "/settings/sticky", NULL))
		gconf_client_set_bool (stickynotes->gconf,
				GCONF_PATH "/settings/sticky", sticky, NULL);
	if (gconf_client_key_is_writable (stickynotes->gconf,
				GCONF_PATH "/settings/force_default", NULL))
		gconf_client_set_bool (stickynotes->gconf,
				GCONF_PATH "/settings/force_default",
				force_default, NULL);
}

/* Preferences Callback : Change color. */
void
preferences_color_cb (GtkWidget *button, gpointer data)
{
	GdkColor color, font_color;
	char *color_str, *font_color_str;

	gtk_color_button_get_color (
			GTK_COLOR_BUTTON (stickynotes->w_prefs_color), &color);
	gtk_color_button_get_color (
			GTK_COLOR_BUTTON (stickynotes->w_prefs_font_color),
			&font_color);
	
	color_str = g_strdup_printf ("#%.2x%.2x%.2x",
			color.red / 256,
			color.green / 256,
			color.blue / 256);
	font_color_str = g_strdup_printf ("#%.2x%.2x%.2x",
			font_color.red / 256,
			font_color.green / 256,
			font_color.blue / 256);
	
	gconf_client_set_string (stickynotes->gconf,
			GCONF_PATH "/defaults/color", color_str, NULL);
	gconf_client_set_string (stickynotes->gconf,
			GCONF_PATH "/defaults/font_color", font_color_str,
			NULL);

	g_free (color_str);
	g_free (font_color_str);
}

/* Preferences Callback : Change font. */
void preferences_font_cb (GtkWidget *button, gpointer data)
{
	const char *font_str;

	font_str = gtk_font_button_get_font_name (GTK_FONT_BUTTON (button));
	gconf_client_set_string(stickynotes->gconf,
			GCONF_PATH "/defaults/font", font_str, NULL);
}

/* Preferences Callback : Apply to existing notes. */
void preferences_apply_cb(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer data)
{
	GList *l;
	StickyNote *note;

	if (!strcmp (entry->key, GCONF_PATH "/settings/sticky"))
	{
		if (gconf_value_get_bool(entry->value))
			for (l = stickynotes->notes; l; l = l->next)
			{
				note = l->data;
				gtk_window_stick (GTK_WINDOW (note->w_window));
			}
		else
			for (l= stickynotes->notes; l; l = l->next)
			{
				note = l->data;
				gtk_window_unstick (GTK_WINDOW (
							note->w_window));
			}
	}

	else if (!strcmp (entry->key, GCONF_PATH "/settings/locked"))
	{
		for (l = stickynotes->notes; l; l = l->next)
		{
			note = l->data;
			stickynote_set_locked (note,
					gconf_value_get_bool (entry->value));
		}
		stickynotes_save();
	}

	else if (!strcmp (entry->key,
				GCONF_PATH "/settings/use_system_color") ||
		 !strcmp (entry->key, GCONF_PATH "/defaults/color"))
	{
		for (l = stickynotes->notes; l; l = l->next)
		{
			note = l->data;
			stickynote_set_color (note,
					note->color, note->font_color,
					FALSE);
		}
	}

	else if (!strcmp (entry->key, GCONF_PATH "/settings/use_system_font") ||
		 !strcmp (entry->key, GCONF_PATH "/defaults/font"))
	{
		for (l = stickynotes->notes; l; l = l->next)
		{
			note = l->data;
			stickynote_set_font (note, note->font, FALSE);
		}
	}

	else if (!strcmp (entry->key, GCONF_PATH "/settings/force_default"))
	{
		for (l = stickynotes->notes; l; l = l->next)
		{
			note = l->data;
			stickynote_set_color(note,
					note->color, note->font_color,
					FALSE);
			stickynote_set_font(note, note->font, FALSE);
		}
	}

	stickynotes_applet_update_prefs();
	stickynotes_applet_update_menus();
}

/* Preferences Callback : Response. */
void preferences_response_cb(GtkWidget *dialog, gint response, gpointer data)
{
	if (response == GTK_RESPONSE_HELP) {
		GError *error = NULL;
		gnome_help_display_on_screen("stickynotes_applet", "stickynotes-advanced-settings", gtk_widget_get_screen(GTK_WIDGET(dialog)), &error);
		if (error) {
			dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
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
