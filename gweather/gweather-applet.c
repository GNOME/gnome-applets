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
#include <applet-widget.h>

#include "weather.h"
#include "gweather-about.h"
#include "gweather-pref.h"
#include "gweather-dialog.h"
#include "gweather-applet.h"

WeatherInfo *gweather_info = NULL;

GtkWidget *gweather_applet = NULL;

static GtkWidget *frame = NULL;
static GtkWidget *pixmap = NULL;
static GtkWidget *label = NULL;
static GtkWidget *fixed = NULL;

static GdkPixmap *cond_pixmap = NULL;
static GdkBitmap *cond_mask = NULL;

static GtkTooltips *tooltips = NULL;

static PanelOrientType gweather_orient = ORIENT_DOWN;
#ifdef HAVE_PANEL_PIXEL_SIZE
static int gweather_size = PIXEL_SIZE_STANDARD;
#else /* HAVE_PANEL_PIXEL_SIZE */
static int gweather_size;
#endif /* HAVE_PANEL_PIXEL_SIZE */

/* FIX - This code is WAY too kludgy!... */
static void place_widgets (void)
{
    GtkRequisition pix_requisition;
    GtkRequisition lbl_requisition;

    gint size;

    g_return_if_fail(frame != NULL);
    g_return_if_fail(fixed != NULL);
    g_return_if_fail(pixmap != NULL);
    g_return_if_fail(label != NULL);

    gtk_widget_get_child_requisition(label, &lbl_requisition);
    gtk_widget_get_child_requisition(pixmap, &pix_requisition);

#ifdef HAVE_PANEL_PIXEL_SIZE
    size = gweather_size;
#else /* HAVE_PANEL_PIXEL_SIZE */
    size = 48;
#endif /* HAVE_PANEL_PIXEL_SIZE */

    if (((gweather_orient == ORIENT_LEFT) || (gweather_orient == ORIENT_RIGHT)) ^ (size < 25)) {
        gint sep = MAX(0, (size - pix_requisition.width - lbl_requisition.width) / 3);
        gint lbl_x = sep + pix_requisition.width + sep;
        if (lbl_x + lbl_requisition.width > size) {
            lbl_x = size + 2;
            sep = MAX(0, (size - pix_requisition.width) / 2);
        }

        gtk_widget_set_usize(frame, MAX (size, 48), 24);
        gtk_fixed_move(GTK_FIXED(fixed), pixmap, sep, 2);
        gtk_fixed_move(GTK_FIXED(fixed), label, lbl_x, 2);
    } else {
        gint panel_width = MAX(lbl_requisition.width, 22);
        gint sep = MAX(0, (size - pix_requisition.height - lbl_requisition.height) / 3);
        gint lbl_y = sep + pix_requisition.height + sep;
        if (lbl_y + lbl_requisition.height > size) {
            lbl_y = size + 2;
            sep = MAX(0, (size - pix_requisition.height) / 2);
        }

        gtk_widget_set_usize(frame, 24, size);
        gtk_fixed_move(GTK_FIXED(fixed), pixmap, (panel_width - pix_requisition.width) / 2 - 1, sep);
        gtk_fixed_move(GTK_FIXED(fixed), label, (panel_width - lbl_requisition.width) / 2, lbl_y);
    }
}

static void change_orient_cb (AppletWidget *w, PanelOrientType o)
{
    gweather_orient = o;
    place_widgets();
    return;
    w = NULL;
}

#ifdef HAVE_PANEL_PIXEL_SIZE
static void change_pixel_size_cb (AppletWidget *w, int s)
{
    gweather_size = s;
    place_widgets();
    return;
    w = NULL;  /* Hush warnings */
}
#endif /* HAVE_PANEL_PIXEL_SIZE */

#ifdef HAVE_SAVE_SESSION_SIGNAL
static int save_session_cb (AppletWidget *w, gchar *privcfgpath, gchar *globcfgpath)
{
    /* fprintf(stderr, "save_session_cb: %s\n", privcfgpath); */
    gweather_pref_save(privcfgpath);
    gweather_info_save(privcfgpath);
    return FALSE;
    w = NULL;
    globcfgpath = NULL;
}
#endif /* HAVE_SAVE_SESSION_SIGNAL */

static void clicked_cb (GtkWidget *widget, GdkEventButton *ev, gpointer data)
{
    if ((ev == NULL) || (ev->button != 1) || (ev->type != GDK_2BUTTON_PRESS))
        return;

    gweather_dialog_display_toggle();
    return;
    widget = NULL;
    data = NULL;
}

static void about_cb (AppletWidget *widget, gpointer data)
{
    gweather_about_run();
    return;
    widget = NULL;
    data = NULL;
}

static void help_cb (AppletWidget *applet, gpointer data)
{
    static GnomeHelpMenuEntry help_entry = { NULL, "index.html"};

    help_entry.name = gnome_app_id;
    gnome_help_display(NULL, &help_entry);
}

static void pref_cb (AppletWidget *widget, gpointer data)
{
    gweather_pref_run();
    return;
    widget = NULL;
    data = NULL;
}

static void update_cb (AppletWidget *widget, gpointer data)
{
    gweather_update();
    return;
    widget = NULL;
    data = NULL;
}

void gweather_applet_create (int argc, char *argv[])
{
    g_return_if_fail(gweather_applet == NULL);

    applet_widget_init("gweather", VERSION, argc, argv,
                       NULL, 0, NULL);

    if ((gweather_applet = applet_widget_new("gweather")) == NULL)
        g_error(_("Cannot create applet!\n"));

    gtk_widget_set_events(gweather_applet, gtk_widget_get_events(gweather_applet) | \
                          GDK_BUTTON_PRESS_MASK);

    applet_widget_register_stock_callback (APPLET_WIDGET(gweather_applet),
					   "about",
                                           GNOME_STOCK_MENU_ABOUT,
					   _("About..."),
                                           about_cb, NULL);
    applet_widget_register_stock_callback (APPLET_WIDGET(gweather_applet), 
					   "help",
					   GNOME_STOCK_PIXMAP_HELP,
					   _("Help"),
					   help_cb, NULL);
    applet_widget_register_stock_callback (APPLET_WIDGET(gweather_applet),
					   "update",
					   GNOME_STOCK_MENU_REFRESH,
					   _("Update"),
					   update_cb, NULL);
    applet_widget_register_stock_callback (APPLET_WIDGET(gweather_applet),
					   "preferences",
                                           GNOME_STOCK_MENU_PREF,
					   _("Properties..."),
                                           pref_cb, NULL);


#ifdef HAVE_SAVE_SESSION_SIGNAL
    gtk_signal_connect (GTK_OBJECT(gweather_applet), "save_session",
                       GTK_SIGNAL_FUNC(save_session_cb), NULL);
#endif /* HAVE_SAVE_SESSION_SIGNAL */
    gtk_signal_connect (GTK_OBJECT(gweather_applet), "change_orient",
                       GTK_SIGNAL_FUNC(change_orient_cb), NULL);
#ifdef HAVE_PANEL_PIXEL_SIZE
    gtk_signal_connect (GTK_OBJECT(gweather_applet), "change_pixel_size",
                       GTK_SIGNAL_FUNC(change_pixel_size_cb), NULL);
#endif /* HAVE_PANEL_PIXEL_SIZE */
    gtk_signal_connect (GTK_OBJECT(gweather_applet), "button_press_event",
                       GTK_SIGNAL_FUNC(clicked_cb), NULL);
    gtk_signal_connect (GTK_OBJECT(gweather_applet), "destroy",
                        GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

    tooltips = gtk_tooltips_new();

    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_widget_set_usize(frame, 26, 48);
    gtk_widget_show(frame);

    weather_info_get_pixmap_mini(NULL, &cond_pixmap, &cond_mask);
    pixmap = gtk_pixmap_new(cond_pixmap, cond_mask);
    gtk_widget_show(pixmap);

    label = gtk_label_new("95\260");
    gtk_widget_show(label);

    fixed = gtk_fixed_new();
    gtk_widget_show(fixed);
    gtk_fixed_put(GTK_FIXED(fixed), pixmap, 4, 4);
    gtk_fixed_put(GTK_FIXED(fixed), label, 2, 26);

    gtk_container_add(GTK_CONTAINER(frame), fixed);

    applet_widget_add(APPLET_WIDGET(gweather_applet), frame);

    gtk_tooltips_set_tip(tooltips, gweather_applet, "GNOME Weather", NULL);

#ifdef HAVE_PANEL_PIXEL_SIZE
    gweather_size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (gweather_applet));
#else /* HAVE_PANEL_PIXEL_SIZE */
    gweather_size = 48;
#endif /* HAVE_PANEL_PIXEL_SIZE */

    gweather_orient = applet_widget_get_panel_orient (APPLET_WIDGET (gweather_applet));

    place_widgets();
    
    gtk_widget_show(gweather_applet);
}


void gweather_info_save (const gchar *path)
{
    gchar *prefix;

    g_return_if_fail(gweather_info != NULL);
    g_return_if_fail(path != NULL);

    prefix = g_strconcat (path, "WeatherInfo/", NULL);
    gnome_config_push_prefix(prefix);
    /* fprintf(stderr, "gweather_info_save: %s\n", prefix); */
    g_free(prefix);

    weather_info_config_write(gweather_info);

    gnome_config_pop_prefix();
    gnome_config_sync();
    gnome_config_drop_all();
}

void gweather_info_load (const gchar *path)
{
    gchar *prefix;

    g_return_if_fail(path != NULL);

    prefix = g_strconcat (path, "WeatherInfo/", NULL);
    gnome_config_push_prefix(prefix);
    /* fprintf(stderr, "gweather_info_save: %s\n", prefix); */
    g_free(prefix);

    gweather_info = weather_info_config_read();

    gnome_config_pop_prefix();
}

static guint timeout_tag = -1;

static gint timeout_cb (gpointer data)
{
    gweather_update();
    return 0;  /* Do not repeat timeout (will be re-set by gweather_update) */
    data = NULL;
}

static void update_finish (WeatherInfo *info)
{
    char *s;
    /* Save weather info */
    gweather_info = info;

    /* Store current conditions */
    /* gweather_info_save(APPLET_WIDGET(gweather_applet)->privcfgpath); */

    /* Update applet pixmap */
    weather_info_get_pixmap_mini(gweather_info, &cond_pixmap, &cond_mask);
    gtk_pixmap_set(GTK_PIXMAP(pixmap), cond_pixmap, cond_mask);

    /* Update temperature text */
    gtk_label_set_text(GTK_LABEL(label), weather_info_get_temp_summary(gweather_info));

    /* Resize as necessary */
    place_widgets();

    /* Update tooltip */
    s = weather_info_get_weather_summary(gweather_info);
    gtk_tooltips_set_tip(tooltips, gweather_applet, s, NULL);
    g_free (s);

    /* Update timer */
    if (timeout_tag > 0)
        gtk_timeout_remove(timeout_tag);
    if (gweather_pref.update_enabled)
        timeout_tag =  gtk_timeout_add (gweather_pref.update_interval * 1000,
                                        timeout_cb, NULL);

    /* Update dialog -- if one is present */
    gweather_dialog_update();
}

void gweather_update (void)
{
    gboolean update_success;

    /* Let user know we are updating */
    weather_info_get_pixmap_mini(gweather_info, &cond_pixmap, &cond_mask);
    gtk_pixmap_set(GTK_PIXMAP(pixmap), cond_pixmap, cond_mask);
    gtk_tooltips_set_tip(tooltips, gweather_applet, "Updating...", NULL);

    /* Set preferred units */
    weather_units_set(gweather_pref.use_metric ? UNITS_METRIC : UNITS_IMPERIAL);

    /* Set preferred forecast type */
    weather_forecast_set(gweather_pref.detailed ? FORECAST_ZONE : FORECAST_STATE);

    /* Set radar map retrieval option */
    weather_radar_set(gweather_pref.radar_enabled);

    /* Set proxy */
    if (gweather_pref.use_proxy)
        weather_proxy_set(gweather_pref.proxy_url, gweather_pref.proxy_user, gweather_pref.proxy_passwd);

    /* Update current conditions */
    if (gweather_pref.update_enabled) {
        if (gweather_info && weather_location_equal(gweather_info->location, gweather_pref.location)) {
            update_success = weather_info_update(gweather_info, update_finish);
        } else {
            weather_info_free(gweather_info);
            update_success = weather_info_new(gweather_pref.location, update_finish);
        }
        if (!update_success)
            gnome_error_dialog("Update failed! Maybe another already in progress?");  /* FIX */
    } else {
        if (gweather_info) {
            if (gweather_pref.use_metric)
                weather_info_to_metric(gweather_info);
            else
		  weather_info_to_imperial(gweather_info);
	}
        update_finish(gweather_info);
    }
}
