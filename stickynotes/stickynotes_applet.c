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

/* Sticky Notes Applet settings instance */
StickyNotesApplet *stickynotes;

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
	/* Create and initialize Sticky Note Applet Settings Instance */
	stickynotes = g_new(StickyNotesApplet, 1);
	stickynotes->applet = GTK_WIDGET(applet);
	stickynotes->about = 0;
	stickynotes->preferences = 0;
	stickynotes->size = panel_applet_get_size(applet);
	stickynotes->pressed = FALSE;
	stickynotes->notes = NULL;
	stickynotes->hidden = FALSE;
	stickynotes->pixbuf_normal = gdk_pixbuf_new_from_file(STICKYNOTES_ICONDIR "/stickynotes.png", NULL);
	stickynotes->pixbuf_prelight = gdk_pixbuf_new_from_file(STICKYNOTES_ICONDIR "/stickynotes_prelight.png", NULL);
	stickynotes->image = NULL;
	stickynotes->gconf_client = gconf_client_get_default();
	stickynotes->tooltips = gtk_tooltips_new();

	/* Create the applet */
	{
		GdkPixbuf *pixbuf = gdk_pixbuf_scale_simple(stickynotes->pixbuf_normal, stickynotes->size, stickynotes->size, GDK_INTERP_BILINEAR);
		stickynotes->image = gtk_image_new_from_pixbuf(pixbuf);
		g_object_unref(pixbuf);
	}
	gtk_container_add(GTK_CONTAINER(applet), stickynotes->image);
	panel_applet_setup_menu_from_file(applet, NULL, "GNOME_StickyNotesApplet.xml", NULL, stickynotes_applet_menu_verbs, stickynotes);
	gtk_tooltips_set_tip(stickynotes->tooltips, GTK_WIDGET(applet), _("Sticky Notes"), NULL);

	/* Connect all signals for applet management */
	g_signal_connect(G_OBJECT(applet), "button-press-event", G_CALLBACK(applet_click_cb), applet);
	g_signal_connect(G_OBJECT(applet), "button-release-event", G_CALLBACK(applet_click_cb), applet);
	g_signal_connect(G_OBJECT(applet), "change-size", G_CALLBACK(applet_resize_cb), applet);
	g_signal_connect(G_OBJECT(applet), "focus-in-event", G_CALLBACK(applet_focus_cb), applet);
	g_signal_connect(G_OBJECT(applet), "focus-out-event", G_CALLBACK(applet_focus_cb), applet);
	g_signal_connect(G_OBJECT(applet), "enter-notify-event", G_CALLBACK(applet_cross_cb), applet);
	g_signal_connect(G_OBJECT(applet), "leave-notify-event", G_CALLBACK(applet_cross_cb), applet);

	/* Show the applet */
	gtk_widget_show_all(GTK_WIDGET(applet));

	/* Watch GConf values */
	gconf_client_add_dir(stickynotes->gconf_client, GCONF_PATH, GCONF_CLIENT_PRELOAD_NONE, NULL);
	gconf_client_notify_add(stickynotes->gconf_client, GCONF_PATH "/settings", (GConfClientNotifyFunc) preferences_apply_cb,
		stickynotes, NULL, NULL);
	
	/* Set default icon for all sticky note windows */
	gnome_window_icon_set_default_from_file(STICKYNOTES_ICONDIR "/stickynotes.png");

	/* Load sticky notes */
	stickynotes_load_all();
	
	/* Lock sticky notes if necessary */
	if (gconf_client_get_bool(stickynotes->gconf_client, GCONF_PATH "/settings/locked", NULL))
		stickynotes_lock_all();

	/* Auto-save every so minutes (default 5) */
	g_timeout_add(1000 * 60 * gconf_client_get_int(stickynotes->gconf_client, GCONF_PATH "/settings/autosave_time", NULL),
		(GSourceFunc) applet_save_cb, applet);
	
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
void stickynotes_applet_set_highlighted(gboolean highlight)
{
	GdkPixbuf *pixbuf1, *pixbuf2;
	
	if (highlight)
	    	pixbuf1 = gdk_pixbuf_scale_simple(stickynotes->pixbuf_prelight, stickynotes->size, stickynotes->size, GDK_INTERP_BILINEAR);
	else
	    	pixbuf1 = gdk_pixbuf_scale_simple(stickynotes->pixbuf_normal, stickynotes->size, stickynotes->size, GDK_INTERP_BILINEAR);
	
	pixbuf2 = gdk_pixbuf_copy(pixbuf1);
	if (stickynotes->pressed)
		gdk_pixbuf_scale(pixbuf1, pixbuf2, 0, 0, stickynotes->size, stickynotes->size, 1, 1, 1, 1, GDK_INTERP_BILINEAR);

	/* Apply the finished pixbuf to the applet image */
	gtk_image_set_from_pixbuf(GTK_IMAGE(stickynotes->image), pixbuf2);

	g_object_unref(pixbuf1);
	g_object_unref(pixbuf2);
}

/* Update the Sticky Notes Applet tooltip */
void stickynotes_applet_update_tooltips()
{
	gchar *tooltip;
	
	gint num = g_list_length(stickynotes->notes);
	
	if (gconf_client_get_bool(stickynotes->gconf_client, GCONF_PATH "/settings/locked", NULL))
		tooltip = g_strdup_printf(ngettext("%s\n%d locked note", "%s\n%d locked notes", num), _("Sticky Notes"), num);
	else
		tooltip = g_strdup_printf(ngettext("%s\n%d note", "%s\n%d notes", num), _("Sticky Notes"), num);

	gtk_tooltips_set_tip(stickynotes->tooltips, GTK_WIDGET(stickynotes->applet), tooltip, NULL);

	g_free(tooltip);
}
