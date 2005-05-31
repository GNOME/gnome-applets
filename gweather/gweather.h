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

typedef enum {
    TEMP_UNIT_INVALID = 0,
    TEMP_UNIT_DEFAULT,
    TEMP_UNIT_KELVIN,
    TEMP_UNIT_CENTIGRADE,
    TEMP_UNIT_FAHRENHEIT
} TempUnit;

typedef enum {
    SPEED_UNIT_INVALID = 0,
    SPEED_UNIT_DEFAULT,
    SPEED_UNIT_MS,    /* metres per second */
    SPEED_UNIT_KPH,   /* kilometres per hour */
    SPEED_UNIT_MPH,   /* miles per hour */
    SPEED_UNIT_KNOTS, /* Knots */
    SPEED_UNIT_BFT    /* Beaufort scale */
} SpeedUnit;

typedef enum {
    PRESSURE_UNIT_INVALID = 0,
    PRESSURE_UNIT_DEFAULT,
	PRESSURE_UNIT_KPA,    /* kiloPascal */
	PRESSURE_UNIT_HPA,    /* hectoPascal */
	PRESSURE_UNIT_MB,     /* 1 millibars = 1 hectoPascal */
    PRESSURE_UNIT_MM_HG,  /* millimeters of mecury */
	PRESSURE_UNIT_INCH_HG /* inchecs of mercury */
} PressureUnit;

typedef enum {
    DISTANCE_UNIT_INVALID = 0,
    DISTANCE_UNIT_DEFAULT,
    DISTANCE_UNIT_METERS,
    DISTANCE_UNIT_KM,
    DISTANCE_UNIT_MILES
} DistanceUnit;

struct _GWeatherPrefs {
    WeatherLocation *location;
    gint update_interval;  /* in seconds */
    gboolean update_enabled;
    gboolean detailed;
    gboolean radar_enabled;
    gboolean use_custom_radar_url;
    gchar *radar;
	
	TempUnit     temperature_unit;
	gboolean     use_temperature_default;
	SpeedUnit    speed_unit;
	gboolean     use_speed_default;
	PressureUnit pressure_unit;
	gboolean     use_pressure_default;
	DistanceUnit distance_unit;
	gboolean     use_distance_default;

};

struct _GWeatherApplet
{
	PanelApplet *applet;
	WeatherInfo *gweather_info;

	GtkWidget *container;
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

	GtkWidget *pref_basic_detailed_btn;
	GtkWidget *pref_basic_temp_combo;
    GtkWidget *pref_basic_speed_combo;
    GtkWidget *pref_basic_dist_combo;
    GtkWidget *pref_basic_pres_combo;
	
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
	GtkWidget *cond_apparent;
	GtkWidget *cond_sunrise;
	GtkWidget *cond_sunset;
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
