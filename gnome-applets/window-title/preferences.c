/*
 * Copyright (C) 2009 Andrej Belcijan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Andrej Belcijan <{andrejx} at {gmail.com}>
 */

#include "config.h"
#include "preferences.h"

static void properties_close (GtkButton *, WTApplet *);

static void
savePreferences (WTPreferences *wtp,
                 WTApplet      *wtapplet)
{
	g_settings_set_boolean (wtapplet->settings, CFG_ONLY_MAXIMIZED, wtp->only_maximized);
	g_settings_set_boolean (wtapplet->settings, CFG_HIDE_ON_UNMAXIMIZED, wtp->hide_on_unmaximized);
	g_settings_set_boolean (wtapplet->settings, CFG_HIDE_ICON, wtp->hide_icon);
	g_settings_set_boolean (wtapplet->settings, CFG_HIDE_TITLE, wtp->hide_title);
	g_settings_set_boolean (wtapplet->settings, CFG_SWAP_ORDER, wtp->swap_order);
	g_settings_set_boolean (wtapplet->settings, CFG_EXPAND_APPLET, wtp->expand_applet);
	g_settings_set_boolean (wtapplet->settings, CFG_CUSTOM_STYLE, wtp->custom_style);
	g_settings_set_boolean (wtapplet->settings, CFG_SHOW_WINDOW_MENU, wtp->show_window_menu);
	g_settings_set_boolean (wtapplet->settings, CFG_SHOW_TOOLTIPS, wtp->show_tooltips);
	g_settings_set_double (wtapplet->settings, CFG_ALIGNMENT, wtp->alignment);
	g_settings_set_int (wtapplet->settings, CFG_TITLE_SIZE, wtp->title_size);
	g_settings_set_string (wtapplet->settings, CFG_TITLE_ACTIVE_FONT, wtp->title_active_font);
	g_settings_set_string (wtapplet->settings, CFG_TITLE_ACTIVE_COLOR_FG, wtp->title_active_color);
	g_settings_set_string (wtapplet->settings, CFG_TITLE_INACTIVE_FONT, wtp->title_inactive_font);
	g_settings_set_string (wtapplet->settings, CFG_TITLE_INACTIVE_COLOR_FG, wtp->title_inactive_color);
}

/* Get our properties (the only properties getter that should be called) */
WTPreferences *
wt_applet_load_preferences (WTApplet *wtapplet)
{
	WTPreferences *wtp = g_new0(WTPreferences, 1);

	wtp->only_maximized = g_settings_get_boolean(wtapplet->settings, CFG_ONLY_MAXIMIZED);
	wtp->hide_on_unmaximized = g_settings_get_boolean(wtapplet->settings, CFG_HIDE_ON_UNMAXIMIZED);
	wtp->hide_icon = g_settings_get_boolean(wtapplet->settings, CFG_HIDE_ICON);
	wtp->hide_title = g_settings_get_boolean(wtapplet->settings, CFG_HIDE_TITLE);
	wtp->alignment = g_settings_get_double(wtapplet->settings, CFG_ALIGNMENT);
	wtp->swap_order = g_settings_get_boolean(wtapplet->settings, CFG_SWAP_ORDER);
	wtp->expand_applet = g_settings_get_boolean(wtapplet->settings, CFG_EXPAND_APPLET);
	wtp->custom_style = g_settings_get_boolean(wtapplet->settings, CFG_CUSTOM_STYLE);
	wtp->show_window_menu = g_settings_get_boolean(wtapplet->settings, CFG_SHOW_WINDOW_MENU);
	wtp->show_tooltips = g_settings_get_boolean(wtapplet->settings, CFG_SHOW_TOOLTIPS);
	wtp->title_size = g_settings_get_int(wtapplet->settings, CFG_TITLE_SIZE);
	wtp->title_active_font = g_settings_get_string(wtapplet->settings, CFG_TITLE_ACTIVE_FONT);
	wtp->title_active_color = g_settings_get_string(wtapplet->settings, CFG_TITLE_ACTIVE_COLOR_FG);
	wtp->title_inactive_font = g_settings_get_string(wtapplet->settings, CFG_TITLE_INACTIVE_FONT);;
	wtp->title_inactive_color = g_settings_get_string(wtapplet->settings, CFG_TITLE_INACTIVE_COLOR_FG);

	return wtp;
}

static void
cb_only_maximized (GtkButton *button,
                   WTApplet  *wtapplet)
{
	wtapplet->prefs->only_maximized = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
}

static void
cb_hide_on_unmaximized (GtkButton *button,
                        WTApplet  *wtapplet)
{
	wtapplet->prefs->hide_on_unmaximized = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_update_title(wtapplet);
}

static void
cb_hide_icon (GtkButton *button,
              WTApplet  *wtapplet)
{
	wtapplet->prefs->hide_icon = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_toggle_hidden (wtapplet);
}

static void
cb_hide_title (GtkButton *button,
               WTApplet  *wtapplet)
{
	wtapplet->prefs->hide_title = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_toggle_hidden (wtapplet);
}

static void
cb_swap_order (GtkButton *button,
               WTApplet  *wtapplet)
{
	wtapplet->prefs->swap_order = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_reload_widgets(wtapplet);
}

static void
cb_expand_applet (GtkButton *button,
                  WTApplet  *wtapplet)
{
	wtapplet->prefs->expand_applet = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_toggle_expand(wtapplet);
}

static void
cb_show_window_menu (GtkButton *button,
                     WTApplet  *wtapplet)
{
	wtapplet->prefs->show_window_menu = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
}

static void
cb_show_tooltips (GtkButton *button,
                  WTApplet  *wtapplet)
{
	wtapplet->prefs->show_tooltips = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	gtk_widget_set_has_tooltip (GTK_WIDGET(wtapplet->icon), wtapplet->prefs->show_tooltips);
	gtk_widget_set_has_tooltip (GTK_WIDGET(wtapplet->title), wtapplet->prefs->show_tooltips);
	savePreferences(wtapplet->prefs, wtapplet);
}

static void
cb_custom_style (GtkButton *button,
                 WTApplet  *wtapplet)
{
	GtkGrid *parent = GTK_GRID (gtk_builder_get_object (wtapplet->prefbuilder, "grid_custom_style"));
	gtk_widget_set_sensitive(GTK_WIDGET(parent), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button)));
	wtapplet->prefs->custom_style = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_update_title(wtapplet);
}

static void cb_alignment_changed(GtkRange *range, WTApplet *wtapplet)
{
	wtapplet->prefs->alignment = gtk_range_get_value(range);
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_set_alignment(wtapplet, (gdouble)wtapplet->prefs->alignment);
}

static void cb_font_active_set(GtkFontButton *widget, WTApplet *wtapplet)
{
	//we need to copy the new font string, because we lose the pointer after prefs close
	wtapplet->prefs->title_active_font = g_strdup(gtk_font_button_get_font_name(widget));
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_update_title(wtapplet);
}

static void cb_font_inactive_set(GtkFontButton *widget, WTApplet *wtapplet)
{
	//we need to copy the new font string, because we lose the pointer after prefs close
	wtapplet->prefs->title_inactive_font = g_strdup(gtk_font_button_get_font_name(widget));
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_update_title(wtapplet);
}

static void cb_color_active_fg_set(GtkColorButton *widget, WTApplet *wtapplet)
{
	GdkColor color;

	gtk_color_button_get_color(widget, &color);
	wtapplet->prefs->title_active_color = gdk_color_to_string(&color);
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_update_title(wtapplet);
}

static void cb_color_inactive_fg_set(GtkColorButton *widget, gpointer user_data)
{
	WTApplet *wtapplet = WT_APPLET(user_data);
	GdkColor color;

	gtk_color_button_get_color(widget, &color);
	wtapplet->prefs->title_inactive_color = gdk_color_to_string(&color);
	savePreferences(wtapplet->prefs, wtapplet);
	wt_applet_update_title(wtapplet);
}


/* The Preferences Dialog */
void
wt_applet_properties_cb (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
	WTApplet *wtapplet;
	GdkColor btn_color_color;
	GtkToggleButton *chkb_only_maximized;
	GtkToggleButton *chkb_hide_on_unmaximized;
	GtkToggleButton *chkb_hide_icon;
	GtkToggleButton *chkb_hide_title;
	GtkToggleButton *chkb_swap_order;
	GtkToggleButton *chkb_expand_applet;
	GtkToggleButton *chkb_custom_style;
	GtkToggleButton *chkb_show_window_menu;
	GtkToggleButton *chkb_show_tooltips;
	GtkHScale *scale_alignment;
	GtkColorButton *btn_color_active;
	GtkFontButton *btn_font_active;
	GtkColorButton *btn_color_inactive;
	GtkFontButton *btn_font_inactive;
	GtkButton *btn_close;
	GtkGrid *grid_custom_style;

	wtapplet = (WTApplet *) user_data;

	// Create the Properties dialog from the GtkBuilder file
	if(wtapplet->window_prefs) {
		gtk_window_present(GTK_WINDOW(wtapplet->window_prefs));
	} else {
		gtk_builder_set_translation_domain (wtapplet->prefbuilder, GETTEXT_PACKAGE);
		gtk_builder_add_from_resource (wtapplet->prefbuilder, GRESOURCE_PREFIX "/ui/window-title.ui", NULL);
		wtapplet->window_prefs = GTK_WIDGET (gtk_builder_get_object (wtapplet->prefbuilder, "properties"));
	}
	//gtk_builder_connect_signals (wtapplet->prefbuilder, NULL); // no need for now

	chkb_only_maximized = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_ONLY_MAXIMIZED));
	chkb_hide_on_unmaximized = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_HIDE_ON_UNMAXIMIZED));
	chkb_hide_icon = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_HIDE_ICON));
	chkb_hide_title = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_HIDE_TITLE));
	chkb_swap_order = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_SWAP_ORDER));
	chkb_expand_applet = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_EXPAND_APPLET));
	chkb_custom_style = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_CUSTOM_STYLE));
	chkb_show_window_menu = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_SHOW_WINDOW_MENU));
	chkb_show_tooltips = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_SHOW_TOOLTIPS));
	scale_alignment = GTK_HSCALE (gtk_builder_get_object(wtapplet->prefbuilder, "scale_alignment"));
	btn_color_active = GTK_COLOR_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, "btn_color_active"));
	btn_font_active = GTK_FONT_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, "btn_font_active"));
	btn_color_inactive = GTK_COLOR_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, "btn_color_inactive"));
	btn_font_inactive = GTK_FONT_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, "btn_font_inactive"));
	btn_close = GTK_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, "btn_close"));
	grid_custom_style = GTK_GRID (gtk_builder_get_object (wtapplet->prefbuilder, "grid_custom_style"));

	// set widgets according to preferences
	gtk_toggle_button_set_active (chkb_only_maximized, wtapplet->prefs->only_maximized);
	gtk_toggle_button_set_active (chkb_hide_on_unmaximized, wtapplet->prefs->hide_on_unmaximized);
	gtk_toggle_button_set_active (chkb_hide_icon, wtapplet->prefs->hide_icon);
	gtk_toggle_button_set_active (chkb_hide_title, wtapplet->prefs->hide_title);
	gtk_toggle_button_set_active (chkb_swap_order, wtapplet->prefs->swap_order);
	gtk_toggle_button_set_active (chkb_expand_applet, wtapplet->prefs->expand_applet);
	gtk_toggle_button_set_active (chkb_custom_style, wtapplet->prefs->custom_style);
	gtk_toggle_button_set_active (chkb_show_window_menu, wtapplet->prefs->show_window_menu);
	gtk_toggle_button_set_active (chkb_show_tooltips, wtapplet->prefs->show_tooltips);
	gtk_range_set_adjustment (GTK_RANGE(scale_alignment), GTK_ADJUSTMENT(gtk_adjustment_new(wtapplet->prefs->alignment, 0, 1.0, 0.1, 0, 0)));
	gdk_color_parse(wtapplet->prefs->title_active_color, &btn_color_color);
	gtk_color_button_set_color(btn_color_active, &btn_color_color);
	gdk_color_parse(wtapplet->prefs->title_inactive_color, &btn_color_color);
	gtk_color_button_set_color(btn_color_inactive, &btn_color_color);
	gtk_font_button_set_font_name(btn_font_active, wtapplet->prefs->title_active_font);
	gtk_font_button_set_font_name(btn_font_inactive, wtapplet->prefs->title_inactive_font);
	gtk_widget_set_sensitive (GTK_WIDGET (grid_custom_style), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chkb_custom_style)));

	g_signal_connect(G_OBJECT(chkb_only_maximized), "clicked", G_CALLBACK (cb_only_maximized), wtapplet);
	g_signal_connect(G_OBJECT(chkb_hide_on_unmaximized), "clicked", G_CALLBACK (cb_hide_on_unmaximized), wtapplet);
	g_signal_connect(G_OBJECT(chkb_hide_icon), "clicked", G_CALLBACK (cb_hide_icon), wtapplet);
	g_signal_connect(G_OBJECT(chkb_hide_title), "clicked", G_CALLBACK (cb_hide_title), wtapplet);
	g_signal_connect(G_OBJECT(chkb_swap_order), "clicked", G_CALLBACK(cb_swap_order), wtapplet);
	g_signal_connect(G_OBJECT(chkb_expand_applet), "clicked", G_CALLBACK(cb_expand_applet), wtapplet);
	g_signal_connect(G_OBJECT(chkb_custom_style), "clicked", G_CALLBACK(cb_custom_style), wtapplet);
	g_signal_connect(G_OBJECT(chkb_show_window_menu), "clicked", G_CALLBACK(cb_show_window_menu), wtapplet);
	g_signal_connect(G_OBJECT(chkb_show_tooltips), "clicked", G_CALLBACK(cb_show_tooltips), wtapplet);
	g_signal_connect(G_OBJECT(scale_alignment), "value-changed", G_CALLBACK(cb_alignment_changed), wtapplet);
	g_signal_connect(G_OBJECT(btn_color_active), "color-set", G_CALLBACK(cb_color_active_fg_set), wtapplet);
	g_signal_connect(G_OBJECT(btn_font_active), "font-set", G_CALLBACK(cb_font_active_set), wtapplet);
	g_signal_connect(G_OBJECT(btn_color_inactive), "color-set", G_CALLBACK(cb_color_inactive_fg_set), wtapplet);
	g_signal_connect(G_OBJECT(btn_font_inactive), "font-set", G_CALLBACK(cb_font_inactive_set), wtapplet);
	g_signal_connect(G_OBJECT(btn_close), "clicked", G_CALLBACK (properties_close), wtapplet);
	g_signal_connect(G_OBJECT(wtapplet->window_prefs), "destroy", G_CALLBACK(properties_close), wtapplet);

	gtk_widget_show_all (wtapplet->window_prefs);
}

/* Close the Properties dialog - we're not saving anything (it's already saved) */
static void
properties_close (GtkButton *object,
                  WTApplet  *wtapplet)
{
	gtk_widget_destroy(wtapplet->window_prefs);
	wtapplet->window_prefs = NULL;
}
