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

/* Sticky Notes Icons */
static const StickyNotesStockIcon stickynotes_icons[] =
{
	{ STICKYNOTES_STOCK_LOCKED, STICKYNOTES_ICONDIR "/locked.png" },
	{ STICKYNOTES_STOCK_UNLOCKED, STICKYNOTES_ICONDIR "/unlocked.png" },
	{ STICKYNOTES_STOCK_CLOSE, STICKYNOTES_ICONDIR "/close.png" },
	{ STICKYNOTES_STOCK_RESIZE_SE, STICKYNOTES_ICONDIR "/resize_se.png" },
	{ STICKYNOTES_STOCK_RESIZE_SW, STICKYNOTES_ICONDIR "/resize_sw.png" }
};

/* Sticky Notes applet factory */
static gboolean stickynotes_applet_factory(PanelApplet *panel_applet, const gchar *iid, gpointer data) 
{
	if (!strcmp(iid, "OAFIID:GNOME_StickyNotesApplet")) {
		if (!stickynotes)
			stickynotes_applet_init (panel_applet);
			
		panel_applet_set_flags (panel_applet, PANEL_APPLET_EXPAND_MINOR);

		/* Add applet to linked list of all applets */
		stickynotes->applets = g_list_append (stickynotes->applets, stickynotes_applet_new(panel_applet));

		stickynotes_applet_update_menus ();
		stickynotes_applet_update_tooltips ();

		return TRUE;
	}

	return FALSE;
}

/* Sticky Notes applet factory */
PANEL_APPLET_BONOBO_FACTORY("OAFIID:GNOME_StickyNotesApplet_Factory", PANEL_TYPE_APPLET, "stickynotes_applet", VERSION,
			    stickynotes_applet_factory, NULL);

/* colorshift a pixbuf */
static void
stickynotes_make_prelight_icon (GdkPixbuf *dest, GdkPixbuf *src, int shift)
{
	gint i, j;
	gint width, height, has_alpha, srcrowstride, destrowstride;
	guchar *target_pixels;
	guchar *original_pixels;
	guchar *pixsrc;
	guchar *pixdest;
	int val;
	guchar r,g,b;

	has_alpha = gdk_pixbuf_get_has_alpha (src);
	width = gdk_pixbuf_get_width (src);
	height = gdk_pixbuf_get_height (src);
	srcrowstride = gdk_pixbuf_get_rowstride (src);
	destrowstride = gdk_pixbuf_get_rowstride (dest);
	target_pixels = gdk_pixbuf_get_pixels (dest);
	original_pixels = gdk_pixbuf_get_pixels (src);

	for (i = 0; i < height; i++) {
		pixdest = target_pixels + i*destrowstride;
		pixsrc = original_pixels + i*srcrowstride;
		for (j = 0; j < width; j++) {
			r = *(pixsrc++);
			g = *(pixsrc++);
			b = *(pixsrc++);
			val = r + shift;
			*(pixdest++) = CLAMP(val, 0, 255);
			val = g + shift;
			*(pixdest++) = CLAMP(val, 0, 255);
			val = b + shift;
			*(pixdest++) = CLAMP(val, 0, 255);
			if (has_alpha)
				*(pixdest++) = *(pixsrc++);
		}
	}
}


/* Create and initalize global sticky notes instance */
void stickynotes_applet_init(PanelApplet *panel_applet)
{	
	GnomeIconTheme *icon_theme;
	gchar *sticky_icon;
	GnomeClient *client;

	stickynotes = g_new(StickyNotes, 1);

	stickynotes->notes = NULL;
	stickynotes->applets = NULL;

	icon_theme = gnome_icon_theme_new ();
	sticky_icon = gnome_icon_theme_lookup_icon (icon_theme, "stock_notes", 48, NULL, NULL);
	/* Register size for icons */
	gtk_icon_size_register ("stickynotes_icon_size", 8,8);
	

	if (sticky_icon) {
		gnome_window_icon_set_default_from_file(sticky_icon);
		stickynotes->icon_normal = gdk_pixbuf_new_from_file(sticky_icon, NULL);
	}
	g_free (sticky_icon);
	g_object_unref (icon_theme);

	stickynotes->icon_prelight = gdk_pixbuf_new(gdk_pixbuf_get_colorspace (stickynotes->icon_normal),
						    gdk_pixbuf_get_has_alpha (stickynotes->icon_normal),
						    gdk_pixbuf_get_bits_per_sample (stickynotes->icon_normal),
						    gdk_pixbuf_get_width (stickynotes->icon_normal),
						    gdk_pixbuf_get_height (stickynotes->icon_normal));
	stickynotes_make_prelight_icon (stickynotes->icon_prelight, stickynotes->icon_normal, 30);
	stickynotes->gconf = gconf_client_get_default();
	stickynotes->tooltips = gtk_tooltips_new();

	stickynotes_applet_init_icons();
	stickynotes_applet_init_prefs();

	/* Watch GConf values */
	gconf_client_add_dir(stickynotes->gconf, GCONF_PATH, GCONF_CLIENT_PRELOAD_NONE, NULL);
	gconf_client_notify_add(stickynotes->gconf, GCONF_PATH "/defaults", (GConfClientNotifyFunc) preferences_apply_cb, NULL, NULL, NULL);
	gconf_client_notify_add(stickynotes->gconf, GCONF_PATH "/settings", (GConfClientNotifyFunc) preferences_apply_cb, NULL, NULL, NULL);

	client = gnome_master_client ();
	gnome_client_set_restart_style (client, GNOME_RESTART_NEVER);

	/* The help suggests this is automatic, but that seems to not be the case here. */
	gnome_client_connect (client);
	gnome_client_flush (client);
	
	/* Load sticky notes */
	stickynotes_load(gtk_widget_get_screen(GTK_WIDGET(panel_applet)));
	
	/* Auto-save every so minutes (default 5) */
	g_timeout_add(1000 * 60 * gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/settings/autosave_time", NULL),
		      (GSourceFunc) applet_save_cb, NULL);
}

/* Initialize Sticky Notes Icons */
void stickynotes_applet_init_icons()
{
	GtkIconFactory *icon_factory = gtk_icon_factory_new();

	gint i;
	for (i = 0; i < G_N_ELEMENTS(stickynotes_icons); i++) {
		StickyNotesStockIcon icon = stickynotes_icons[i];
		GtkIconSource *icon_source = gtk_icon_source_new();
		GtkIconSet *icon_set = gtk_icon_set_new();

		gtk_icon_source_set_filename(icon_source, icon.filename);
		gtk_icon_set_add_source(icon_set, icon_source);
		gtk_icon_factory_add(icon_factory, icon.stock_id, icon_set);

		gtk_icon_source_free(icon_source);
		gtk_icon_set_unref(icon_set);
	}

	gtk_icon_factory_add_default(icon_factory);

	g_object_unref(G_OBJECT(icon_factory));
}

void stickynotes_applet_init_prefs()
{
	stickynotes->prefs = glade_xml_new(GLADE_PATH, "preferences_dialog", NULL);

	stickynotes->w_prefs = glade_xml_get_widget(stickynotes->prefs, "preferences_dialog");
	stickynotes->w_prefs_width = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(stickynotes->prefs, "width_spin")));
	stickynotes->w_prefs_height= gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(glade_xml_get_widget(stickynotes->prefs, "height_spin")));
	stickynotes->w_prefs_color = glade_xml_get_widget(stickynotes->prefs, "default_color");
	stickynotes->w_prefs_sys_color = GTK_WIDGET(&GTK_CHECK_BUTTON(glade_xml_get_widget(stickynotes->prefs, "sys_color_check"))->toggle_button);
	stickynotes->w_prefs_font = glade_xml_get_widget(stickynotes->prefs, "default_font");
	stickynotes->w_prefs_sys_font = GTK_WIDGET(&GTK_CHECK_BUTTON(glade_xml_get_widget(stickynotes->prefs, "sys_font_check"))->toggle_button);
	stickynotes->w_prefs_click = glade_xml_get_widget(stickynotes->prefs, "click_behavior_menu");
	stickynotes->w_prefs_sticky = GTK_WIDGET(&GTK_CHECK_BUTTON(glade_xml_get_widget(stickynotes->prefs, "sticky_check"))->toggle_button);
	stickynotes->w_prefs_force = GTK_WIDGET(&GTK_CHECK_BUTTON(glade_xml_get_widget(stickynotes->prefs, "force_default_check"))->toggle_button);

	g_signal_connect(G_OBJECT(stickynotes->w_prefs), "response", G_CALLBACK(preferences_response_cb), NULL);
	g_signal_connect(G_OBJECT(stickynotes->w_prefs), "delete-event", G_CALLBACK(preferences_delete_cb), NULL);
	g_signal_connect_swapped(G_OBJECT(stickynotes->w_prefs_width), "value-changed", G_CALLBACK(preferences_save_cb), NULL);
	g_signal_connect_swapped(G_OBJECT(stickynotes->w_prefs_height), "value-changed", G_CALLBACK(preferences_save_cb), NULL);
	g_signal_connect_swapped(G_OBJECT(stickynotes->w_prefs_sys_color), "toggled", G_CALLBACK(preferences_save_cb), NULL);
	g_signal_connect_swapped(G_OBJECT(stickynotes->w_prefs_sys_font), "toggled", G_CALLBACK(preferences_save_cb), NULL);
	g_signal_connect(G_OBJECT(stickynotes->w_prefs_color), "color_set", G_CALLBACK(preferences_color_cb), NULL);
	g_signal_connect(G_OBJECT(stickynotes->w_prefs_font), "font_set", G_CALLBACK(preferences_font_cb), NULL);
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
			
	if (!gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/defaults/width", NULL)) {
		gtk_widget_set_sensitive(glade_xml_get_widget(stickynotes->prefs, "width_label"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget(stickynotes->prefs, "width_spin"), FALSE);
	}
	if (!gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/defaults/height", NULL)) {
		gtk_widget_set_sensitive(glade_xml_get_widget(stickynotes->prefs, "height_label"), FALSE);
		gtk_widget_set_sensitive(glade_xml_get_widget(stickynotes->prefs, "height_spin"), FALSE);
	}
	if (!gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/defaults/color", NULL)) {
		gtk_widget_set_sensitive(glade_xml_get_widget(stickynotes->prefs, "color_label"), FALSE);
		gtk_widget_set_sensitive(stickynotes->w_prefs_color, FALSE);
	}
	if (!gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/use_system_color", NULL))
		gtk_widget_set_sensitive(stickynotes->w_prefs_sys_color, FALSE);
	if (!gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/defaults/font", NULL)) {
		gtk_widget_set_sensitive(glade_xml_get_widget(stickynotes->prefs, "font_label"), FALSE);
		gtk_widget_set_sensitive(stickynotes->w_prefs_font, FALSE);
	}
	if (!gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/use_system_font", NULL))
		gtk_widget_set_sensitive(stickynotes->w_prefs_sys_font, FALSE);
	if (!gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/click_behavior", NULL)) {
		gtk_widget_set_sensitive(glade_xml_get_widget(stickynotes->prefs, "click_label"), FALSE);
		gtk_widget_set_sensitive(stickynotes->w_prefs_click, FALSE);
	}
	if (!gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/sticky", NULL))
		gtk_widget_set_sensitive(stickynotes->w_prefs_sticky, FALSE);
	if (!gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/force_default", NULL))
		gtk_widget_set_sensitive(stickynotes->w_prefs_force, FALSE);

	stickynotes_applet_update_prefs();
}

/* Create a Sticky Notes applet */
StickyNotesApplet * stickynotes_applet_new(PanelApplet *panel_applet)
{
	AtkObject *atk_obj;

	/* Create Sticky Notes Applet */
	StickyNotesApplet *applet = g_new(StickyNotesApplet, 1);

	/* Initialize Sticky Notes Applet */
	applet->w_applet = GTK_WIDGET(panel_applet);
	applet->w_image = gtk_image_new();
	applet->about_dialog = NULL;
	applet->destroy_all_dialog = NULL;
	applet->prelighted = FALSE;
	applet->pressed = FALSE;

	/* Expand the applet for Fitts' law complience. */
	panel_applet_set_flags(panel_applet, PANEL_APPLET_EXPAND_MINOR);

	/* Add the applet icon */
	gtk_container_add(GTK_CONTAINER(panel_applet), applet->w_image);
	stickynotes_applet_update_icon(applet);

	/* Add the popup menu */
	panel_applet_setup_menu_from_file(panel_applet, NULL, "GNOME_StickyNotesApplet.xml", NULL, stickynotes_applet_menu_verbs, applet);

	if (panel_applet_get_locked_down (panel_applet)) {
		BonoboUIComponent *popup_component;

		popup_component = panel_applet_get_popup_component (panel_applet);

		bonobo_ui_component_set_prop (popup_component,
					      "/commands/preferences",
					      "hidden", "1",
					      NULL);
	}

	bonobo_ui_component_add_listener(panel_applet_get_popup_component(panel_applet), "show", (BonoboUIListenerFn) menu_toggle_show_cb, applet);
	bonobo_ui_component_add_listener(panel_applet_get_popup_component(panel_applet), "lock", (BonoboUIListenerFn) menu_toggle_lock_cb, applet);

	/* Connect all signals for applet management */
	g_signal_connect(G_OBJECT(applet->w_applet), "button-press-event", G_CALLBACK(applet_button_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "button-release-event", G_CALLBACK(applet_button_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "key-press-event", G_CALLBACK(applet_key_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "focus-in-event", G_CALLBACK(applet_focus_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "focus-out-event", G_CALLBACK(applet_focus_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "enter-notify-event", G_CALLBACK(applet_cross_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "leave-notify-event", G_CALLBACK(applet_cross_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "change-size", G_CALLBACK(applet_change_size_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "change-background", G_CALLBACK(applet_change_bg_cb), applet);
	g_signal_connect(G_OBJECT(applet->w_applet), "destroy", G_CALLBACK(applet_destroy_cb), applet);

	atk_obj = gtk_widget_get_accessible (applet->w_applet);
	atk_object_set_name (atk_obj, _("Sticky Notes"));

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
	    	pixbuf1 = gdk_pixbuf_scale_simple(stickynotes->icon_prelight, size - 3, size - 3, GDK_INTERP_BILINEAR);
	else
	    	pixbuf1 = gdk_pixbuf_scale_simple(stickynotes->icon_normal, size - 3, size - 3, GDK_INTERP_BILINEAR);
	
	/* Shift the icon if pressed */
	pixbuf2 = gdk_pixbuf_copy(pixbuf1);
	if (applet->pressed)
		gdk_pixbuf_scale(pixbuf1, pixbuf2, 0, 0, size - 3, size - 3, 1, 1, 1, 1, GDK_INTERP_BILINEAR);

	/* Apply the finished pixbuf to the applet image */
	gtk_image_set_from_pixbuf(GTK_IMAGE(applet->w_image), pixbuf2);

	g_object_unref(pixbuf1);
	g_object_unref(pixbuf2);
}

void stickynotes_applet_update_prefs()
{
	gint height, click_behavior;
	gboolean sys_color, sys_font, sticky, force_default;
	gchar *font_str;
	GdkColor color;
	gint width = gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/defaults/width", NULL);

	width = MAX (width, 1);
	height = gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/defaults/height", NULL);
	height = MAX (height, 1);

	sys_color = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/use_system_color", NULL);
	sys_font = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/use_system_font", NULL);
	click_behavior = gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/settings/click_behavior", NULL);
	sticky = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/sticky", NULL);
	force_default = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/force_default", NULL);
	font_str = gconf_client_get_string(stickynotes->gconf, GCONF_PATH "/defaults/font", NULL);

	if (!font_str)
		font_str = g_strdup ("Sans 10");
	
	{
		gchar *color_str = gconf_client_get_string(stickynotes->gconf, GCONF_PATH "/defaults/color", NULL);
		if (!color_str)
			color_str = g_strdup ("#FFFF00");
		gdk_color_parse(color_str, &color);
		g_free(color_str);
	}

	gtk_adjustment_set_value(stickynotes->w_prefs_width, width);
	gtk_adjustment_set_value(stickynotes->w_prefs_height, height);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_sys_color), sys_color);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_sys_font), sys_font);
	gtk_option_menu_set_history(GTK_OPTION_MENU(stickynotes->w_prefs_click), click_behavior);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_sticky), sticky);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stickynotes->w_prefs_force), force_default);

	gnome_color_picker_set_i16(GNOME_COLOR_PICKER(stickynotes->w_prefs_color), color.red, color.green, color.blue, 65535);
	gnome_font_picker_set_font_name(GNOME_FONT_PICKER(stickynotes->w_prefs_font), font_str);

	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/defaults/color", NULL)) {
		gtk_widget_set_sensitive(glade_xml_get_widget(stickynotes->prefs, "color_label"), !sys_color);
		gtk_widget_set_sensitive(stickynotes->w_prefs_color, !sys_color);
	}
	if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/defaults/font", NULL)) {
		gtk_widget_set_sensitive(glade_xml_get_widget(stickynotes->prefs, "font_label"), !sys_font);
		gtk_widget_set_sensitive(stickynotes->w_prefs_font, !sys_font);
	}
}

void stickynotes_applet_update_menus()
{
	gboolean visible = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/visible", NULL);
	gboolean locked = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/locked", NULL);
	
	gboolean visible_writable = gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/visible", NULL);
	gboolean locked_writable = gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/locked", NULL);
	
	gboolean inconsistent = FALSE;
	gint i;

	for (i = 0; i < g_list_length(stickynotes->notes) && !inconsistent; i++) {
		StickyNote *note = g_list_nth_data(stickynotes->notes, i);

		if (note->locked != locked)
			inconsistent = TRUE;
	}

	for (i = 0; i < g_list_length(stickynotes->applets); i++) {
		StickyNotesApplet *applet = g_list_nth_data(stickynotes->applets, i);
		BonoboUIComponent *popup = panel_applet_get_popup_component(PANEL_APPLET(applet->w_applet));
		
		bonobo_ui_component_set_prop(popup, "/commands/show", "state", visible ? "1" : "0", NULL);
		bonobo_ui_component_set_prop(popup, "/commands/lock", "state", locked ? "1" : "0", NULL);

		bonobo_ui_component_set_prop(popup, "/commands/lock", "inconsistent", inconsistent ? "1" : "0", NULL); /* FIXME : Doesn't work */

		bonobo_ui_component_set_prop(popup, "/commands/show", "sensitive", visible_writable ? "1" : "0", NULL);
		bonobo_ui_component_set_prop(popup, "/commands/lock", "sensitive", locked_writable ? "1" : "0", NULL);
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

void stickynotes_applet_do_default_action(GdkScreen *screen)
{
	StickyNotesDefaultAction click_behavior = gconf_client_get_int(stickynotes->gconf, GCONF_PATH "/settings/click_behavior", NULL);

	gboolean visible = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/visible", NULL);
	gboolean locked = gconf_client_get_bool(stickynotes->gconf, GCONF_PATH "/settings/locked", NULL);

	switch (click_behavior) {
		case STICKYNOTES_NEW:
			stickynotes_add(screen);
			break;

		case STICKYNOTES_SET_VISIBLE:
			if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/visible", NULL))
				gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/visible", !visible, NULL);
			break;

		case STICKYNOTES_SET_LOCKED:
			if (gconf_client_key_is_writable(stickynotes->gconf, GCONF_PATH "/settings/locked", NULL))
				gconf_client_set_bool(stickynotes->gconf, GCONF_PATH "/settings/locked", !locked, NULL);
			break;
	}
}
