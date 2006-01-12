#ifndef __WEATHER_H_
#define __WEATHER_H_

/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Public header for weather server functions.
 *
 */

#include <gdk-pixbuf/gdk-pixbuf-loader.h>

G_BEGIN_DECLS

/*
 * Location
 */

#define WEATHER_LOCATION_CODE_LEN 4

struct _WeatherLocation {
    gchar *name;
    gchar *code;
    gchar *zone;
    gchar *radar;
    gboolean zone_valid;
    gchar *coordinates;
    gdouble  latitude;
    gdouble  longitude;
    gboolean latlon_valid;
};

typedef struct _WeatherLocation WeatherLocation;

WeatherLocation *	weather_location_new 	(const gchar *trans_name,
						 const gchar *code,
						 const gchar *zone,
						 const gchar *radar,
						 const gchar *coordinates);
WeatherLocation *	weather_location_clone	(const WeatherLocation *location);
void			weather_location_free	(WeatherLocation *location);
gboolean		weather_location_equal	(const WeatherLocation *location1,
						 const WeatherLocation *location2);

/*
 * Weather prefs
 */

typedef enum _WeatherForecastType {
    FORECAST_STATE,
    FORECAST_ZONE
} WeatherForecastType;

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

struct _WeatherPrefs {
    WeatherForecastType type;

    gboolean radar;
    const char *radar_custom_url;

    TempUnit temperature_unit;
    SpeedUnit speed_unit;
    PressureUnit pressure_unit;
    DistanceUnit distance_unit;
};

typedef struct _WeatherPrefs WeatherPrefs;

/*
 * Weather Info
 */

typedef struct _WeatherInfo WeatherInfo;

typedef void (*WeatherInfoFunc) (WeatherInfo *info, gpointer data);

WeatherInfo *	_weather_info_fill			(WeatherInfo *info,
							 WeatherLocation *location,
							 const WeatherPrefs *prefs,
							 WeatherInfoFunc cb,
							 gpointer data);
#define	weather_info_new(location, prefs, cb, data) _weather_info_fill(NULL, (location), (prefs), (cb), (data))
#define	weather_info_update(info, prefs, cb, data) _weather_info_fill((info), NULL, (prefs), (cb), (data));

void			weather_info_abort		(WeatherInfo *info);
WeatherInfo *		weather_info_clone		(const WeatherInfo *info);
void			weather_info_free		(WeatherInfo *info);

gboolean		weather_info_is_valid		(WeatherInfo *info);

void			weather_info_to_metric		(WeatherInfo *info);
void			weather_info_to_imperial	(WeatherInfo *info);

const WeatherLocation *	weather_info_get_location	(WeatherInfo *info);
const gchar *		weather_info_get_location_name	(WeatherInfo *info);
const gchar *		weather_info_get_update		(WeatherInfo *info);
const gchar *		weather_info_get_sky		(WeatherInfo *info);
const gchar *		weather_info_get_conditions	(WeatherInfo *info);
const gchar *		weather_info_get_temp		(WeatherInfo *info);
const gchar *		weather_info_get_dew		(WeatherInfo *info);
const gchar *		weather_info_get_humidity	(WeatherInfo *info);
const gchar *		weather_info_get_wind		(WeatherInfo *info);
const gchar *		weather_info_get_pressure	(WeatherInfo *info);
const gchar *		weather_info_get_visibility	(WeatherInfo *info);
const gchar *		weather_info_get_apparent	(WeatherInfo *info);
const gchar *		weather_info_get_sunrise	(WeatherInfo *info);
const gchar *		weather_info_get_sunset		(WeatherInfo *info);
const gchar *		weather_info_get_forecast	(WeatherInfo *info);
GdkPixbufAnimation *	weather_info_get_radar		(WeatherInfo *info);

const gchar *		weather_info_get_temp_summary	(WeatherInfo *info);
gchar *			weather_info_get_weather_summary(WeatherInfo *info);

void			_weather_info_get_pixbuf	(WeatherInfo *info,
							 gboolean mini,
							 GdkPixbuf **pixbuf);
#define weather_info_get_pixbuf_mini(info,pixbuf) _weather_info_get_pixbuf((info), TRUE, (pixbuf))
#define weather_info_get_pixbuf(info,pixbuf) _weather_info_get_pixbuf((info), FALSE, (pixbuf))

gint			weather_info_next_sun_event	(WeatherInfo *info);
G_END_DECLS

#endif /* __WEATHER_H_ */
