#ifndef __GWEATHER_H__
#define __GWEATHER_H__

/*
 *  todd kulesza <fflewddur@dropline.net>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 * main header file
 *
 */

#include <gnome.h>
#include <panel-applet.h>



/* Radar map on by default. */
#define RADARMAP

G_BEGIN_DECLS
 
typedef struct _GWeatherApplet GWeatherApplet;
typedef struct _GWeatherPrefs GWeatherPrefs;
typedef struct _WeatherInfo WeatherInfo;
typedef struct _WeatherLocation WeatherLocation;

struct _GWeatherPrefs {
    WeatherLocation *location;
    gint update_interval;  /* in seconds */
    gboolean update_enabled;
    gboolean use_metric;
    gboolean detailed;
    gboolean radar_enabled;
    gboolean use_custom_radar_url;
    gchar *radar;
};

struct _GWeatherApplet
{
	PanelApplet *applet;
	WeatherInfo *gweather_info;
	
	GtkWidget *box;	
	GtkWidget *label;
	GtkWidget *image;
	GtkTooltips *tooltips;
	
	PanelAppletOrient orient;
	gint size;
	gint timeout_tag;

	GtkWidget *about_dialog;
	
	/* preferences  */
	GWeatherPrefs gweather_pref;

	GtkWidget *pref;

	GtkWidget *pref_basic_metric_btn;
	GtkWidget *pref_basic_detailed_btn;
#ifdef RADARMAP
	GtkWidget *pref_basic_radar_btn;
	GtkWidget *pref_basic_radar_url_btn;
	GtkWidget *pref_basic_radar_url_hbox;
	GtkWidget *pref_basic_radar_url_entry;
#endif /* RADARMAP */
	GtkWidget *pref_basic_update_spin;
	GtkWidget *pref_basic_update_btn;
	GtkWidget *pref_tree;
	
	/* dialog stuff */
	GtkWidget *gweather_dialog;

	GtkWidget *cond_location;
	GtkWidget *cond_update;
	GtkWidget *cond_cond;
	GtkWidget *cond_sky;
	GtkWidget *cond_temp;
	GtkWidget *cond_dew;
	GtkWidget *cond_humidity;
	GtkWidget *cond_wind;
	GtkWidget *cond_pressure;
	GtkWidget *cond_vis;
	GtkWidget *cond_image;
	GtkWidget *forecast_text;
	GtkWidget *radar_image;

	GdkPixbuf *dialog_pixbuf;
	GdkBitmap *dialog_mask;
	GdkPixbuf *applet_pixbuf;
	GdkBitmap *applet_mask;
};

G_END_DECLS
 
#endif

