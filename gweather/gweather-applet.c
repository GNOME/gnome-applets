/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main applet widget
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>
#include <panel-applet.h>
#include <libgnomeui/gnome-window-icon.h>

#include "weather.h"
#include "gweather.h"
#include "gweather-about.h"
#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"

/* this may wreak havoc with the bonobo control+gconf stuff, but it's the only way i can see to do it */
static GWeatherApplet *evil_global_applet;

/* FIX - This code is WAY too kludgy!... */
static void place_widgets (GWeatherApplet *gw_applet)
{
    GtkRequisition pix_requisition;
    GtkRequisition lbl_requisition;

    gint size;
/*
    g_return_if_fail(frame != NULL);
    g_return_if_fail(fixed != NULL);
    g_return_if_fail(pixmap != NULL);
    g_return_if_fail(label != NULL);
*/

	size = gw_applet->size;

    gtk_widget_get_child_requisition(gw_applet->label, &lbl_requisition);
    gtk_widget_get_child_requisition(gw_applet->pixmap, &pix_requisition);

    if (((gw_applet->orient == PANEL_APPLET_ORIENT_LEFT) || (gw_applet->orient == PANEL_APPLET_ORIENT_RIGHT)) ^ (size < 25)) {
        gint sep = MAX(0, (size - pix_requisition.width - lbl_requisition.width) / 3);
        gint lbl_x = sep + pix_requisition.width + sep;
        if (lbl_x + lbl_requisition.width > size) {
            lbl_x = size + 2;
            sep = MAX(0, (size - pix_requisition.width) / 2);
        }

        gtk_widget_set_size_request(gw_applet->frame, MAX (size, 48), 24);
        gtk_fixed_move(GTK_FIXED(gw_applet->fixed), gw_applet->pixmap, sep, 2);
        gtk_fixed_move(GTK_FIXED(gw_applet->fixed), gw_applet->label, lbl_x, 2);
    } else {
        gint panel_width = MAX(lbl_requisition.width, 22);
        gint sep = MAX(0, (size - pix_requisition.height - lbl_requisition.height) / 3);
        gint lbl_y = sep + pix_requisition.height + sep;
        if (lbl_y + lbl_requisition.height > size) {
            lbl_y = size + 2;
            sep = MAX(0, (size - pix_requisition.height) / 2);
        }

        gtk_widget_set_size_request(gw_applet->frame, 24, size);
        gtk_fixed_move(GTK_FIXED(gw_applet->fixed), gw_applet->pixmap, (panel_width - pix_requisition.width) / 2 - 1, sep);
        gtk_fixed_move(GTK_FIXED(gw_applet->fixed), gw_applet->label, (panel_width - lbl_requisition.width) / 2, lbl_y);
    }

	return;
}

static void change_orient_cb (PanelApplet *w, PanelAppletOrient o, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gw_applet->orient = o;
    place_widgets(gw_applet);
    return;
}

static void change_size_cb(PanelApplet *w, gint s, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gw_applet->size = s;
    place_widgets(gw_applet);
    return;
}

#ifdef FIXME
static void clicked_cb (GtkWidget *widget, GdkEventButton *ev, gpointer data)
{
    if ((ev == NULL) || (ev->button != 1))
	    return;

    gweather_pref_run ();

    return;
}
#endif
static void about_cb (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{

    gweather_about_run();
    return;

}

static void help_cb (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
#ifdef FIXME
    static GnomeHelpMenuEntry help_entry = { "gweather_applet", "index.html"};

    gnome_help_display(NULL, &help_entry);
#endif
}

static void pref_cb (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
    GWeatherApplet *gw_applet = data;
    gweather_pref_run(gw_applet);
    return;
}

static void forecast_cb (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
    GWeatherApplet *gw_applet = data;

    gweather_dialog_display_toggle(gw_applet);

    return;
}

static void update_cb (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
    GWeatherApplet *gw_applet = data;

    gweather_update(gw_applet);

    return;
}


static const BonoboUIVerb weather_applet_menu_verbs [] = {
	BONOBO_UI_VERB ("Forecast", forecast_cb),
	BONOBO_UI_VERB ("Update", update_cb),
        BONOBO_UI_VERB ("Props", pref_cb),
        BONOBO_UI_VERB ("Help", help_cb),
        BONOBO_UI_VERB ("About", about_cb),

        BONOBO_UI_VERB_END
};

static const char weather_applet_menu_xml [] =
	"<popup name=\"button3\">\n"
	"   <menuitem name=\"Item 1\" verb=\"Forecast\" _label=\"Forecast\"/>\n"
	"   <menuitem name=\"Item 2\" verb=\"Update\" _label=\"Update\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-refresh\"/>\n"
	"   <menuitem name=\"Item 3\" verb=\"Props\" _label=\"Properties\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
	"   <menuitem name=\"Item 4\" verb=\"Help\" _label=\"Help\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
	"   <menuitem name=\"Item 5\" verb=\"About\" _label=\"About\"\n"
	"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
	"</popup>\n";
	
void gweather_applet_create (GWeatherApplet *gw_applet)
{
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *fixed;
	GtkWidget *pixmap;
	GtkTooltips *tooltips;

 
   gw_applet->gweather_pref.location = NULL;
   gw_applet->gweather_pref.update_interval = 1800;
   gw_applet->gweather_pref.update_enabled = TRUE;
   gw_applet->gweather_pref.use_metric = FALSE;
   gw_applet->gweather_pref.detailed = FALSE;
   gw_applet->gweather_pref.radar_enabled = TRUE;
   gw_applet->gweather_pref.proxy_url = NULL;
   gw_applet->gweather_pref.proxy_user = NULL;
   gw_applet->gweather_pref.proxy_passwd = NULL;
   gw_applet->gweather_pref.use_proxy = FALSE;
   
    gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gweather/tstorm.xpm");

    /* PUSH */
    gtk_widget_push_visual (gdk_rgb_get_visual ());
    gtk_widget_push_colormap (gdk_rgb_get_cmap ());

    /*gtk_widget_realize(GTK_WIDGET(gw_applet->applet));

    gtk_widget_set_events(GTK_WIDGET(gw_applet->applet), gtk_widget_get_events(GTK_WIDGET(gw_applet->applet)) | \
                          GDK_BUTTON_PRESS_MASK);*/


    g_signal_connect (G_OBJECT(gw_applet->applet), "change_orient",
                       G_CALLBACK(change_orient_cb), gw_applet);
    g_signal_connect (G_OBJECT(gw_applet->applet), "change_size",
                       G_CALLBACK(change_size_cb), gw_applet);
/*
    gtk_signal_connect (GTK_OBJECT(gweather_applet), "button_press_event",
                       GTK_SIGNAL_FUNC(clicked_cb), NULL);
*/
    g_signal_connect (G_OBJECT(gw_applet->applet), "destroy",
                        G_CALLBACK(gtk_main_quit), NULL);
                        
    tooltips = gtk_tooltips_new();

    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_NONE);
    gtk_widget_set_size_request(frame, 26, 48);
    gtk_widget_show(frame);

    weather_info_get_pixmap_mini(NULL, &(gw_applet->applet_pixmap), &(gw_applet->applet_mask));
    pixmap = gtk_pixmap_new(gw_applet->applet_pixmap, gw_applet->applet_mask);
    gtk_widget_show(pixmap);

    label = gtk_label_new("95\260");
    gtk_widget_show(label);

    fixed = gtk_fixed_new();
    gtk_widget_show(fixed);
    gtk_fixed_put(GTK_FIXED(fixed), pixmap, 4, 4);
    gtk_fixed_put(GTK_FIXED(fixed), label, 2, 26);

    gtk_container_add(GTK_CONTAINER(frame), fixed);

    gtk_container_add(GTK_CONTAINER(gw_applet->applet), frame);

    gtk_tooltips_set_tip(tooltips, GTK_WIDGET(gw_applet->applet), _("GNOME Weather"), NULL);

    gw_applet->size = panel_applet_get_size (gw_applet->applet);

    gw_applet->orient = panel_applet_get_orient (gw_applet->applet);
    
    panel_applet_setup_menu (gw_applet->applet,
			     weather_applet_menu_xml,
			     weather_applet_menu_verbs,
			     gw_applet);

	gw_applet->frame = frame;
	gw_applet->label = label;
	gw_applet->fixed = fixed;
	gw_applet->pixmap = pixmap;
	gw_applet->tooltips = tooltips;
	
	gtk_widget_show_all(GTK_WIDGET(gw_applet->applet));

    place_widgets(gw_applet);
    /* POP */
    gtk_widget_pop_colormap ();
    gtk_widget_pop_visual ();
    
	return;
}


void gweather_info_save (const gchar *path, GWeatherApplet *gw_applet)
{
    gchar *prefix;

    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(path != NULL);

    prefix = g_strconcat (path, "WeatherInfo/", NULL);
    gnome_config_push_prefix(prefix);
    /* fprintf(stderr, "gweather_info_save: %s\n", prefix); */
    g_free(prefix);

    weather_info_config_write(gw_applet->gweather_info);

    gnome_config_pop_prefix();
    gnome_config_sync();
    gnome_config_drop_all();
}

void gweather_info_load (const gchar *path, GWeatherApplet *gw_applet)
{
    gchar *prefix;

    g_return_if_fail(path != NULL);

    prefix = g_strconcat (path, "WeatherInfo/", NULL);
    gnome_config_push_prefix(prefix);
    /* fprintf(stderr, "gweather_info_save: %s\n", prefix); */
    g_free(prefix);

    weather_info_free (gw_applet->gweather_info);
    gw_applet->gweather_info = weather_info_config_read();

    gnome_config_pop_prefix();
}

static gint timeout_cb (gpointer data)
{
	GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	
    gweather_update(gw_applet);
    return 0;  /* Do not repeat timeout (will be re-set by gweather_update) */
}

static void update_finish (WeatherInfo *info)
{
    char *s;
    GWeatherApplet *gw_applet =evil_global_applet;
    
/*
    if (info != gweather_info) {
	    if (gweather_info != NULL) {
		    weather_info_free (gweather_info);
		    gweather_info = NULL;
	    }

*/	    /* Save weather info */ /*
	    gweather_info = weather_info_clone (info);
    }
*/
    /* Store current conditions */
    /* gweather_info_save(APPLET_WIDGET(gweather_applet)->privcfgpath); */
    
    gw_applet->gweather_info = info;

    /* Update applet pixmap */
    weather_info_get_pixmap_mini(gw_applet->gweather_info, &(gw_applet->applet_pixmap), &(gw_applet->applet_mask));
    gtk_pixmap_set(GTK_PIXMAP(gw_applet->pixmap), gw_applet->applet_pixmap, gw_applet->applet_mask);

    /* Update temperature text */
   
    gtk_label_set_text(GTK_LABEL(gw_applet->label), weather_info_get_temp_summary(gw_applet->gweather_info));
 
    /* Resize as necessary */
    place_widgets(gw_applet);

    /* Update tooltip */

    s = weather_info_get_weather_summary(gw_applet->gweather_info);
    gtk_tooltips_set_tip(gw_applet->tooltips, GTK_WIDGET(gw_applet->applet), s, NULL);
    g_free (s);


    /* Update timer */
    if (gw_applet->timeout_tag > 0)
        gtk_timeout_remove(gw_applet->timeout_tag);
    if (gw_applet->gweather_pref.update_enabled)
        gw_applet->timeout_tag =  gtk_timeout_add (gw_applet->gweather_pref.update_interval * 1000,
                                        timeout_cb, NULL);

    /* Update dialog -- if one is present */
    gweather_dialog_update(gw_applet);
}

void gweather_update (GWeatherApplet *gw_applet)
{
    gboolean update_success;

    /* Let user know we are updating */

    weather_info_get_pixmap_mini(gw_applet->gweather_info, &(gw_applet->applet_pixmap), &(gw_applet->applet_mask));

    gtk_pixmap_set(GTK_PIXMAP(gw_applet->pixmap), gw_applet->applet_pixmap, gw_applet->applet_mask);
    gtk_tooltips_set_tip(gw_applet->tooltips, GTK_WIDGET(gw_applet->applet), _("Updating..."), NULL);

    /* Set preferred units */
    weather_units_set(gw_applet->gweather_pref.use_metric ? UNITS_METRIC : UNITS_IMPERIAL);

    /* Set preferred forecast type */
    weather_forecast_set(gw_applet->gweather_pref.detailed ? FORECAST_ZONE : FORECAST_STATE);

    /* Set radar map retrieval option */
    weather_radar_set(gw_applet->gweather_pref.radar_enabled);

    /* Set proxy */
    if (gw_applet->gweather_pref.use_proxy)
        weather_proxy_set(gw_applet->gweather_pref.proxy_url, gw_applet->gweather_pref.proxy_user, gw_applet->gweather_pref.proxy_passwd);

    /* Update current conditions */
    if (gw_applet->gweather_info && weather_location_equal(gw_applet->gweather_info->location, gw_applet->gweather_pref.location)) {
    	evil_global_applet = gw_applet;
        update_success = weather_info_update(gw_applet->gweather_info, update_finish);
    } else {
        weather_info_free(gw_applet->gweather_info);
        gw_applet->gweather_info = NULL;
        evil_global_applet = gw_applet;
        update_success = weather_info_new(gw_applet->gweather_pref.location, update_finish);
    }
    if (!update_success)
        gnome_error_dialog(_("Update failed! Maybe another update was already in progress?"));  /* FIX */

    return;
}
