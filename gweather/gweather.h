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
#include <libgnomevfs/gnome-vfs.h>

G_BEGIN_DECLS
 
typedef struct _GWeatherApplet GWeatherApplet;
typedef struct _GWeatherPrefs GWeatherPrefs;
typedef struct _WeatherForecast WeatherForecast;
typedef struct _WeatherInfo WeatherInfo;
typedef struct _WeatherLocation WeatherLocation;

#define MAX_FORECASTS 5

struct _GWeatherPrefs {
    WeatherLocation *location;
    gint update_interval;  /* in seconds */
    gboolean update_enabled;
    gboolean use_metric;
    gboolean detailed;
    gchar *city;
    gchar *url;
    gboolean show_labels;
};

struct _GWeatherApplet
{
	PanelApplet *applet;
	WeatherInfo *gweather_info;
	
	GtkWidget *box;	
	GtkWidget *label;
	GtkWidget *image;
	GtkWidget *images[MAX_FORECASTS+1];
    	GtkWidget *boxes[MAX_FORECASTS+1];
	GtkWidget *labels[MAX_FORECASTS+1];
	GtkWidget *events[MAX_FORECASTS+1];
	GtkTooltips *tooltips;
	
	PanelAppletOrient orient;
	gint size;
	gint timeout_tag;
	gint animation_tag;
	gint animation_loc;

	/* Locations stuff */
	GnomeVFSAsyncHandle *locations_handle;
	gchar *locations_xml;
	GtkTreeStore *country_model;
	GtkWidget *country_tree;
	GtkTreeStore *city_model;
	GtkWidget *city_tree;
	GtkWidget *druid;
	GtkTreeIter country_iter;
	gint fetchid; /*-1 not fetching, 0 county, 1 cities */
	
	/* preferences  */
	GWeatherPrefs gweather_pref;

	GtkWidget *pref;

	GtkWidget *pref_basic_metric_btn;
	GtkWidget *pref_basic_detailed_btn;
	GtkWidget *pref_basic_update_spin;
	GtkWidget *pref_basic_update_btn;
	GtkWidget *pref_location_city_label;
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
	GtkWidget *cond_feeltemp;
	GtkWidget *forecast_text;
	GtkWidget *radar_image;
	GtkWidget *forecast_tree;
	GtkTreeModel *forecast_model;

};

void update_display (GWeatherApplet *applet);

G_END_DECLS
 
#endif

