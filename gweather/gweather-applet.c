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

static void place_widgets (void)
{

    g_return_if_fail(frame != NULL);
    g_return_if_fail(fixed != NULL);
    g_return_if_fail(pixmap != NULL);
    g_return_if_fail(label != NULL);

    if ((gweather_orient == ORIENT_LEFT) || (gweather_orient == ORIENT_RIGHT)) {
        GtkRequisition requisition;
        gint ofs;
        gtk_widget_get_child_requisition(label, &requisition);
        ofs = MAX(0, (31-requisition.width)/2);
        gtk_widget_set_usize(frame, 48, 26);
        gtk_fixed_move(GTK_FIXED(fixed), pixmap, 1, 5);
        gtk_fixed_move(GTK_FIXED(fixed), label, 17 + ofs, 4);
    } else {
        GtkRequisition requisition;
        gint width;
        gtk_widget_get_child_requisition(label, &requisition);
        width = MAX(requisition.width, 24) + 2;
        gtk_widget_set_usize(frame, width, 48);
        gtk_fixed_move(GTK_FIXED(fixed), pixmap, (width-18)/2, 4);
        gtk_fixed_move(GTK_FIXED(fixed), label, (width-requisition.width)/2, 26);
    }
}

static void change_orient_cb (GtkWidget *w, PanelOrientType o, gpointer data)
{
    gweather_orient = o;
    place_widgets();
}

static void clicked_cb (GtkWidget *widget, GdkEventButton *ev, gpointer data)
{
    if ((ev == NULL) || (ev->button != 1) || (ev->type != GDK_2BUTTON_PRESS))
        return;

    gweather_dialog_show_toggle();
}

static void about_cb (AppletWidget *widget, gpointer data)
{
    gweather_about_run();
}

static void pref_cb (AppletWidget *widget, gpointer data)
{
    gweather_pref_run();
}

static void update_cb (AppletWidget *widget, gpointer data)
{
    gweather_update();
}

void gweather_applet_create (int argc, char *argv[])
{
    g_return_if_fail(gweather_applet == NULL);

    applet_widget_init("gweather", VERSION, argc, argv,
                       NULL, 0, NULL);

    if ((gweather_applet = applet_widget_new(PACKAGE)) == NULL)
        g_error(_("Cannot create applet!\n"));

    gtk_widget_set_events(gweather_applet, gtk_widget_get_events(gweather_applet) | \
                          GDK_BUTTON_PRESS_MASK);
    applet_widget_register_stock_callback (APPLET_WIDGET(gweather_applet), "about",
                                           GNOME_STOCK_MENU_ABOUT, _("About..."),
                                           about_cb, NULL);
    applet_widget_register_stock_callback (APPLET_WIDGET(gweather_applet), "preferences",
                                           GNOME_STOCK_MENU_PREF, _("Properties..."),
                                           pref_cb, NULL);
    applet_widget_register_callback (APPLET_WIDGET(gweather_applet),
                                     "update", _("Update"),
                                     update_cb, NULL);
    gtk_signal_connect (GTK_OBJECT(gweather_applet), "change_orient",
                       GTK_SIGNAL_FUNC(change_orient_cb), NULL);
    gtk_signal_connect (GTK_OBJECT(gweather_applet), "button_press_event",
                       GTK_SIGNAL_FUNC(clicked_cb), NULL);
    gtk_signal_connect (GTK_OBJECT(gweather_applet), "destroy",
                        GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

    tooltips = gtk_tooltips_new();

    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
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

    gtk_widget_show(gweather_applet);
}


void gweather_info_save (void)
{
    gchar *prefix;

    g_return_if_fail(gweather_info != NULL);

    prefix = g_strconcat ("/", gnome_app_id, "/WeatherInfo/", NULL);

    gnome_config_push_prefix(prefix);
    weather_info_config_write(gweather_info);
    gnome_config_sync();
    gnome_config_pop_prefix();
    g_free(prefix);
}

void gweather_info_load (void)
{
    gchar *prefix = g_strconcat ("/", gnome_app_id, "/WeatherInfo/", NULL);

    gnome_config_push_prefix(prefix);
    gweather_info = weather_info_config_read();
    gnome_config_pop_prefix();
    g_free(prefix);
}

static guint timeout_tag = -1;

static gint timeout_cb (gpointer data)
{
    gweather_update();
    return 0;  /* Do not repeat timeout (will be re-set by gweather_update) */
}

void gweather_update (void)
{
    /* Set preferred units */
    weather_units_set(gweather_pref.use_metric ? UNITS_METRIC : UNITS_IMPERIAL);

    /* Set preferred forecast type */
    weather_forecast_set(gweather_pref.detailed ? FORECAST_ZONE : FORECAST_STATE);

    /* Set proxy */
    if (gweather_pref.use_proxy)
        weather_proxy_set(gweather_pref.proxy_url, gweather_pref.proxy_user, gweather_pref.proxy_passwd);

    /* Update current conditions */
    if (gweather_pref.update_enabled) {
        if (gweather_info && weather_location_equal(gweather_info->location, gweather_pref.location)) {
            weather_info_update(gweather_info);
        } else {
            weather_info_free(gweather_info);
            gweather_info = weather_info_new(gweather_pref.location);
        }
    } else {
        if (gweather_info)
            if (gweather_pref.use_metric)
                weather_info_to_metric(gweather_info);
            else
                weather_info_to_imperial(gweather_info);
    }

    /* Store current conditions */
    gweather_info_save();

    /* Update applet pixmap */
    weather_info_get_pixmap_mini(gweather_info, &cond_pixmap, &cond_mask);
    gtk_pixmap_set(GTK_PIXMAP(pixmap), cond_pixmap, cond_mask);

    /* Update temperature text */
    gtk_label_set_text(GTK_LABEL(label), weather_info_get_temp_summary(gweather_info));

    /* Resize as necessary */
    place_widgets();

    /* Update tooltip */
    gtk_tooltips_set_tip(tooltips, gweather_applet, weather_info_get_weather_summary(gweather_info), NULL);

    /* Update timer */
    if (timeout_tag > 0)
        gtk_timeout_remove(timeout_tag);
    if (gweather_pref.update_enabled)
        timeout_tag =  gtk_timeout_add (gweather_pref.update_interval * 1000,
                                        timeout_cb, NULL);

    /* Update dialog */
    gweather_dialog_update();
}

