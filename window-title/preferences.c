/***************************************************************************
 *            preferences.c
 *
 *  Mon May  4 01:23:08 2009
 *  Copyright  2009  Andrej Belcijan
 *  <{andrejx} at {gmail.com}>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include "preferences.h"

/* prototypes */
void cb_only_maximized (GtkButton *, WTApplet *);
void cb_click_effect (GtkButton *, WTApplet *);
void cb_hide_on_unmaximized (GtkButton *, WTApplet *);
const gchar *getCheckBoxGConfKey (gushort);
void updateTitle(WTApplet *);
void reloadWidgets(WTApplet *);
void toggleHidden(WTApplet *);
void toggleExpand(WTApplet *);
void setAlignment(WTApplet *, gfloat);
void properties_close (GtkButton *, WTApplet *);
void savePreferences(WTPreferences *, WTApplet *);
WTPreferences *loadPreferences(WTApplet *);
gchar* getCfgValue(FILE *, gchar *);

void savePreferences(WTPreferences *wtp, WTApplet *wtapplet) {
#if PLAINTEXT_CONFIG == 0
	panel_applet_gconf_set_bool (wtapplet->applet, CFG_ONLY_MAXIMIZED, wtp->only_maximized, NULL);
	panel_applet_gconf_set_bool (wtapplet->applet, CFG_HIDE_ON_UNMAXIMIZED, wtp->hide_on_unmaximized, NULL);
	panel_applet_gconf_set_bool (wtapplet->applet, CFG_HIDE_ICON, wtp->hide_icon, NULL);
	panel_applet_gconf_set_bool (wtapplet->applet, CFG_HIDE_TITLE, wtp->hide_title, NULL);
	panel_applet_gconf_set_bool (wtapplet->applet, CFG_SWAP_ORDER, wtp->swap_order, NULL);
	panel_applet_gconf_set_bool (wtapplet->applet, CFG_EXPAND_APPLET, wtp->expand_applet, NULL);
	panel_applet_gconf_set_bool (wtapplet->applet, CFG_CUSTOM_STYLE, wtp->custom_style, NULL);
	panel_applet_gconf_set_bool (wtapplet->applet, CFG_SHOW_WINDOW_MENU, wtp->show_window_menu, NULL);
	panel_applet_gconf_set_bool (wtapplet->applet, CFG_SHOW_TOOLTIPS, wtp->show_tooltips, NULL);
	panel_applet_gconf_set_float (wtapplet->applet, CFG_ALIGNMENT, wtp->alignment, NULL);
	panel_applet_gconf_set_int (wtapplet->applet, CFG_TITLE_SIZE, wtp->title_size, NULL);
	panel_applet_gconf_set_string (wtapplet->applet, CFG_TITLE_ACTIVE_FONT, wtp->title_active_font, NULL);
	panel_applet_gconf_set_string (wtapplet->applet, CFG_TITLE_ACTIVE_COLOR_FG, wtp->title_active_color, NULL);
	panel_applet_gconf_set_string (wtapplet->applet, CFG_TITLE_INACTIVE_FONT, wtp->title_inactive_font, NULL);
	panel_applet_gconf_set_string (wtapplet->applet, CFG_TITLE_INACTIVE_COLOR_FG, wtp->title_inactive_color, NULL);
#else
	FILE *cfg = g_fopen (g_strconcat(g_get_home_dir(),"/",FILE_CONFIGFILE, NULL), "w");

	fprintf(cfg, "%s %d\n", CFG_ONLY_MAXIMIZED, wtp->only_maximized);
	fprintf(cfg, "%s %d\n", CFG_HIDE_ON_UNMAXIMIZED, wtp->hide_on_unmaximized);
	fprintf(cfg, "%s %d\n", CFG_HIDE_ICON, wtp->hide_icon);
	fprintf(cfg, "%s %d\n", CFG_HIDE_TITLE, wtp->hide_title);
	fprintf(cfg, "%s %d\n", CFG_SWAP_ORDER, wtp->swap_order);
	fprintf(cfg, "%s %d\n", CFG_EXPAND_APPLET, wtp->expand_applet);
	fprintf(cfg, "%s %d\n", CFG_CUSTOM_STYLE, wtp->custom_style);
	fprintf(cfg, "%s %d\n", CFG_SHOW_WINDOW_MENU, wtp->show_window_menu);
	fprintf(cfg, "%s %d\n", CFG_SHOW_TOOLTIPS, wtp->show_tooltips);
	fprintf(cfg, "%s %f\n", CFG_ALIGNMENT, wtp->alignment);
	fprintf(cfg, "%s %d\n", CFG_TITLE_SIZE, wtp->title_size);
	fprintf(cfg, "%s %s\n", CFG_TITLE_ACTIVE_FONT, wtp->title_active_font);
	fprintf(cfg, "%s %s\n", CFG_TITLE_ACTIVE_COLOR_FG, wtp->title_active_color);
	fprintf(cfg, "%s %s\n", CFG_TITLE_INACTIVE_FONT, wtp->title_inactive_font);
	fprintf(cfg, "%s %s\n", CFG_TITLE_INACTIVE_COLOR_FG, wtp->title_inactive_color);

	fclose (cfg);
#endif
}

/* Get our properties (the only properties getter that should be called) */
WTPreferences *loadPreferences(WTApplet *wtapplet) {
	WTPreferences *wtp = g_new0(WTPreferences, 1);

#if PLAINTEXT_CONFIG == 0
	wtp->only_maximized = panel_applet_gconf_get_bool(wtapplet->applet, CFG_ONLY_MAXIMIZED, NULL);
	wtp->hide_on_unmaximized = panel_applet_gconf_get_bool(wtapplet->applet, CFG_HIDE_ON_UNMAXIMIZED, NULL);
	wtp->hide_icon = panel_applet_gconf_get_bool(wtapplet->applet, CFG_HIDE_ICON, NULL);
	wtp->hide_title = panel_applet_gconf_get_bool(wtapplet->applet, CFG_HIDE_TITLE, NULL);
	wtp->alignment = panel_applet_gconf_get_float(wtapplet->applet, CFG_ALIGNMENT, NULL);
	wtp->swap_order = panel_applet_gconf_get_bool(wtapplet->applet, CFG_SWAP_ORDER, NULL);
	wtp->expand_applet = panel_applet_gconf_get_bool(wtapplet->applet, CFG_EXPAND_APPLET, NULL);
	wtp->custom_style = panel_applet_gconf_get_bool(wtapplet->applet, CFG_CUSTOM_STYLE, NULL);
	wtp->show_window_menu = panel_applet_gconf_get_bool(wtapplet->applet, CFG_SHOW_WINDOW_MENU, NULL);
	wtp->show_tooltips = panel_applet_gconf_get_bool(wtapplet->applet, CFG_SHOW_TOOLTIPS, NULL);
	wtp->title_size = panel_applet_gconf_get_int(wtapplet->applet, CFG_TITLE_SIZE, NULL);
	wtp->title_active_font = panel_applet_gconf_get_string(wtapplet->applet, CFG_TITLE_ACTIVE_FONT, NULL);
	wtp->title_active_color = panel_applet_gconf_get_string(wtapplet->applet, CFG_TITLE_ACTIVE_COLOR_FG, NULL);
	wtp->title_inactive_font = panel_applet_gconf_get_string(wtapplet->applet, CFG_TITLE_INACTIVE_FONT, NULL);;
	wtp->title_inactive_color = panel_applet_gconf_get_string(wtapplet->applet, CFG_TITLE_INACTIVE_COLOR_FG, NULL);
#else
	FILE *cfg = g_fopen (g_strconcat(g_get_home_dir(),"/",FILE_CONFIGFILE,NULL),"r");

	if (cfg) {
		wtp->only_maximized = g_ascii_strtod(getCfgValue(cfg,CFG_ONLY_MAXIMIZED),NULL);
		wtp->hide_on_unmaximized = g_ascii_strtod(getCfgValue(cfg,CFG_HIDE_ON_UNMAXIMIZED),NULL);
		wtp->hide_icon = g_ascii_strtod(getCfgValue(cfg,CFG_HIDE_ICON),NULL);
		wtp->hide_title = g_ascii_strtod(getCfgValue(cfg,CFG_HIDE_TITLE),NULL);
		wtp->alignment = g_ascii_strtod(getCfgValue(cfg,CFG_ALIGNMENT),NULL);
		wtp->swap_order = g_ascii_strtod(getCfgValue(cfg,CFG_SWAP_ORDER),NULL);
		wtp->expand_applet = g_ascii_strtod(getCfgValue(cfg,CFG_EXPAND_APPLET),NULL);
		wtp->custom_style = g_ascii_strtod(getCfgValue(cfg,CFG_CUSTOM_STYLE),NULL);
		wtp->show_window_menu = g_ascii_strtod(getCfgValue(cfg,CFG_SHOW_WINDOW_MENU),NULL);
		wtp->show_tooltips = g_ascii_strtod(getCfgValue(cfg,CFG_SHOW_TOOLTIPS),NULL);
		wtp->title_size = g_ascii_strtod(getCfgValue(cfg,CFG_TITLE_SIZE),NULL);
		wtp->title_active_font = getCfgValue(cfg,CFG_TITLE_ACTIVE_FONT);
		wtp->title_active_color = getCfgValue(cfg,CFG_TITLE_ACTIVE_COLOR_FG);
		wtp->title_inactive_font = getCfgValue(cfg,CFG_TITLE_INACTIVE_FONT);
		wtp->title_inactive_color = getCfgValue(cfg,CFG_TITLE_INACTIVE_COLOR_FG);
		
		fclose (cfg);		
	} else {
		// Defaults if the file doesn't exist
		
		wtp->only_maximized = TRUE;
		wtp->hide_on_unmaximized = FALSE;
		wtp->hide_icon = FALSE;
		wtp->hide_title = FALSE;
		wtp->alignment = 0.5;
		wtp->swap_order = FALSE;
		wtp->expand_applet = TRUE;
		wtp->custom_style = TRUE;
		wtp->show_window_menu = FALSE;
		wtp->show_tooltips = FALSE;
		wtp->title_size = 16;
		wtp->title_active_font = "Sans 10";
		wtp->title_active_color = "#FFFFFF";
		wtp->title_inactive_font = "Sans 10";
		wtp->title_inactive_color = "#808080";

		savePreferences(wtp,wtapplet);
	}
#endif

	return wtp;
}

#if PLAINTEXT_CONFIG != 0
/* Returns a string value of the specified configuration parameter (key) */
// TODO: It wouldn't be too bad if we had this function in a common file instead of duplicating it for both applets
gchar* getCfgValue(FILE *f, gchar *key) {
    gchar tmp[256] = {0x0};
	long int pos = ftell(f);
	
    while (f!=NULL && fgets(tmp,sizeof(tmp),f)!=NULL) {
		if (g_strrstr(tmp, key))
		    break;
    }

	gchar *r = g_strndup(tmp+strlen(key)+1,strlen(tmp)-strlen(key)+1);
	g_strstrip(r);

	fseek(f,pos,SEEK_SET);
    return r;
}
#endif

void cb_only_maximized(GtkButton *button, WTApplet *wtapplet) {
	wtapplet->prefs->only_maximized = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
}

void cb_hide_on_unmaximized(GtkButton *button, WTApplet *wtapplet) {
	wtapplet->prefs->hide_on_unmaximized = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	updateTitle(wtapplet);
}

void cb_hide_icon(GtkButton *button, WTApplet *wtapplet) {
	wtapplet->prefs->hide_icon = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	toggleHidden (wtapplet);
}

void cb_hide_title(GtkButton *button, WTApplet *wtapplet) {
	wtapplet->prefs->hide_title = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	toggleHidden(wtapplet);
}

void cb_swap_order(GtkButton *button, WTApplet *wtapplet) {
	wtapplet->prefs->swap_order = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	reloadWidgets(wtapplet);
}

void cb_expand_applet(GtkButton *button, WTApplet *wtapplet) {
	wtapplet->prefs->expand_applet = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	toggleExpand(wtapplet);
}

void cb_show_window_menu(GtkButton *button, WTApplet *wtapplet) {
	wtapplet->prefs->show_window_menu = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
}

void cb_show_tooltips(GtkButton *button, WTApplet *wtapplet) {
	wtapplet->prefs->show_tooltips = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	gtk_widget_set_has_tooltip (GTK_WIDGET(wtapplet->icon), wtapplet->prefs->show_tooltips);
	gtk_widget_set_has_tooltip (GTK_WIDGET(wtapplet->title), wtapplet->prefs->show_tooltips);
	savePreferences(wtapplet->prefs, wtapplet);
}

void cb_custom_style(GtkButton *button, WTApplet *wtapplet) {
	GtkTable *parent = GTK_TABLE(gtk_builder_get_object(wtapplet->prefbuilder, "table_custom_style"));
	gtk_widget_set_sensitive(GTK_WIDGET(parent), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button)));
	wtapplet->prefs->custom_style = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button));
	savePreferences(wtapplet->prefs, wtapplet);
	updateTitle(wtapplet);
}

static void cb_alignment_changed(GtkRange *range, WTApplet *wtapplet)
{
	wtapplet->prefs->alignment = gtk_range_get_value(range);
	savePreferences(wtapplet->prefs, wtapplet);
	setAlignment(wtapplet, (gfloat)wtapplet->prefs->alignment);
}

static void cb_font_active_set(GtkFontButton *widget, WTApplet *wtapplet)
{
	//we need to copy the new font string, because we lose the pointer after prefs close
	wtapplet->prefs->title_active_font = g_strdup(gtk_font_button_get_font_name(widget));		
	savePreferences(wtapplet->prefs, wtapplet);
	updateTitle(wtapplet);
}

static void cb_font_inactive_set(GtkFontButton *widget, WTApplet *wtapplet)
{
	//we need to copy the new font string, because we lose the pointer after prefs close
	wtapplet->prefs->title_inactive_font = g_strdup(gtk_font_button_get_font_name(widget));		
	savePreferences(wtapplet->prefs, wtapplet);
	updateTitle(wtapplet);
}

static void cb_color_active_fg_set(GtkColorButton *widget, WTApplet *wtapplet)
{
	GdkColor color;

	gtk_color_button_get_color(widget, &color);
	wtapplet->prefs->title_active_color = gdk_color_to_string(&color);	
	savePreferences(wtapplet->prefs, wtapplet);
	updateTitle(wtapplet);
}

static void cb_color_inactive_fg_set(GtkColorButton *widget, gpointer user_data)
{
	WTApplet *wtapplet = WT_APPLET(user_data);
	GdkColor color;

	gtk_color_button_get_color(widget, &color);
	wtapplet->prefs->title_inactive_color = gdk_color_to_string(&color);	
	savePreferences(wtapplet->prefs, wtapplet);
	updateTitle(wtapplet);
}


/* The Preferences Dialog */
void properties_cb (GtkAction *action, WTApplet *wtapplet) {
	GdkColor btn_color_color;
	
	// Create the Properties dialog from the GtkBuilder file
	if(wtapplet->window_prefs) {
		gtk_window_present(GTK_WINDOW(wtapplet->window_prefs));
	} else {
		gtk_builder_add_from_file (wtapplet->prefbuilder, PATH_UI_PREFS, NULL);
		wtapplet->window_prefs = GTK_WIDGET (gtk_builder_get_object (wtapplet->prefbuilder, "properties"));
	}
	//gtk_builder_connect_signals (wtapplet->prefbuilder, NULL); // no need for now

	GtkToggleButton 
		*chkb_only_maximized = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_ONLY_MAXIMIZED)),
		*chkb_hide_on_unmaximized = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_HIDE_ON_UNMAXIMIZED)),
		*chkb_hide_icon = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_HIDE_ICON)),
		*chkb_hide_title = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_HIDE_TITLE)),
		*chkb_swap_order = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_SWAP_ORDER)),
		*chkb_expand_applet = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_EXPAND_APPLET)),
		*chkb_custom_style = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_CUSTOM_STYLE)),
		*chkb_show_window_menu = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_SHOW_WINDOW_MENU)),
		*chkb_show_tooltips = GTK_TOGGLE_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, CFG_SHOW_TOOLTIPS));
	GtkHScale *scale_alignment = GTK_HSCALE (gtk_builder_get_object(wtapplet->prefbuilder, "scale_alignment"));
	GtkColorButton *btn_color_active = GTK_COLOR_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, "btn_color_active"));
	GtkFontButton *btn_font_active = GTK_FONT_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, "btn_font_active"));
	GtkColorButton *btn_color_inactive = GTK_COLOR_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, "btn_color_inactive"));
	GtkFontButton *btn_font_inactive = GTK_FONT_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, "btn_font_inactive"));
	GtkButton *btn_close = GTK_BUTTON (gtk_builder_get_object(wtapplet->prefbuilder, "btn_close"));
	GtkTable *table_custom_style = GTK_TABLE(gtk_builder_get_object(wtapplet->prefbuilder, "table_custom_style"));
	
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
	gtk_widget_set_sensitive(GTK_WIDGET(table_custom_style), gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(chkb_custom_style)));
	
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
void properties_close (GtkButton *object, WTApplet *wtapplet) {
	gtk_widget_destroy(wtapplet->window_prefs);
	wtapplet->window_prefs = NULL;
}
