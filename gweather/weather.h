#ifndef __WEATHER_H_
#define __WEATHER_H_

/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Weather server functions (METAR and IWIN)
 *
 */

#include <time.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <libgnomevfs/gnome-vfs.h>
#include "gweather.h"

/*
 * Location
 */

G_BEGIN_DECLS

#define WEATHER_LOCATION_CODE_LEN 4

struct _WeatherLocation {
    gchar *name;
    gchar *code;
    gchar *zone;
    gchar *radar;
};



extern WeatherLocation *weather_location_new (const gchar *name, const gchar *code, const gchar *zone, const gchar *radar);
extern WeatherLocation *weather_location_clone (const WeatherLocation *location);
extern void weather_location_free (WeatherLocation *location);
extern gboolean weather_location_equal (const WeatherLocation *location1, const WeatherLocation *location2);

extern void weather_location_config_write (gchar *prefix, WeatherLocation *location);
extern WeatherLocation *weather_location_config_read (PanelApplet *applet);


/*
 * Weather information
 */

enum _WeatherWindDirection {
    WIND_VARIABLE,
    WIND_N, WIND_NNE, WIND_NE, WIND_ENE,
    WIND_E, WIND_ESE, WIND_SE, WIND_SSE,
    WIND_S, WIND_SSW, WIND_SW, WIND_WSW,
    WIND_W, WIND_WNW, WIND_NW, WIND_NNW
};

typedef enum _WeatherWindDirection WeatherWindDirection;

extern const gchar *weather_wind_direction_string (WeatherWindDirection wind);

enum _WeatherSky {
    SKY_CLEAR,
    SKY_BROKEN,
    SKY_SCATTERED,
    SKY_FEW,
    SKY_OVERCAST
};

typedef enum _WeatherSky WeatherSky;

extern const gchar *weather_sky_string (WeatherSky sky);

enum _WeatherConditionPhenomenon {
   PHENOMENON_NONE,

   PHENOMENON_DRIZZLE,
   PHENOMENON_RAIN,
   PHENOMENON_SNOW,
   PHENOMENON_SNOW_GRAINS,
   PHENOMENON_ICE_CRYSTALS,
   PHENOMENON_ICE_PELLETS,
   PHENOMENON_HAIL,
   PHENOMENON_SMALL_HAIL,
   PHENOMENON_UNKNOWN_PRECIPITATION,

   PHENOMENON_MIST,
   PHENOMENON_FOG,
   PHENOMENON_SMOKE,
   PHENOMENON_VOLCANIC_ASH,
   PHENOMENON_SAND,
   PHENOMENON_HAZE,
   PHENOMENON_SPRAY,
   PHENOMENON_DUST,

   PHENOMENON_SQUALL,
   PHENOMENON_SANDSTORM,
   PHENOMENON_DUSTSTORM,
   PHENOMENON_FUNNEL_CLOUD,
   PHENOMENON_TORNADO,
   PHENOMENON_DUST_WHIRLS
};

typedef enum _WeatherConditionPhenomenon WeatherConditionPhenomenon;

enum _WeatherConditionQualifier {
   QUALIFIER_NONE,

   QUALIFIER_VICINITY,

   QUALIFIER_LIGHT,
   QUALIFIER_MODERATE,
   QUALIFIER_HEAVY,
   QUALIFIER_SHALLOW,
   QUALIFIER_PATCHES,
   QUALIFIER_PARTIAL,
   QUALIFIER_THUNDERSTORM,
   QUALIFIER_BLOWING,
   QUALIFIER_SHOWERS,
   QUALIFIER_DRIFTING,
   QUALIFIER_FREEZING
};

typedef enum _WeatherConditionQualifier WeatherConditionQualifier;

struct _WeatherConditions {
    gboolean significant;
    WeatherConditionPhenomenon phenomenon;
    WeatherConditionQualifier qualifier;
};

typedef struct _WeatherConditions WeatherConditions;

extern const gchar *weather_conditions_string (WeatherConditions cond);

enum _WeatherUnits {
    UNITS_IMPERIAL,
    UNITS_METRIC
};

typedef enum _WeatherUnits WeatherUnits;

enum _WeatherForecastType {
    FORECAST_STATE,
    FORECAST_ZONE
};

typedef enum _WeatherForecastType WeatherForecastType;

typedef gdouble WeatherTemperature;
typedef gint WeatherHumidity;
typedef gint WeatherWindSpeed;
typedef gdouble WeatherPressure;
typedef gdouble WeatherVisibility;

extern void weather_units_set (WeatherUnits units);
extern WeatherUnits weather_units_get (void);

extern void weather_forecast_set (WeatherForecastType forecast);
extern WeatherForecastType weather_forecast_get (void);

extern void weather_radar_set (gboolean enable);
extern gboolean weather_radar_get (void);

extern void weather_proxy_set (const gchar *url, const gchar *user, const gchar *passwd);

typedef time_t WeatherUpdate;

struct _WeatherInfo {
    gboolean valid;
    WeatherLocation *location;
    WeatherUnits units;
    WeatherUpdate update;
    WeatherSky sky;
    WeatherConditions cond;
    WeatherTemperature temp;
    WeatherTemperature dew;
    WeatherHumidity humidity;
    WeatherWindDirection wind;
    WeatherWindSpeed windspeed;
    WeatherPressure pressure;
    WeatherVisibility visibility;
    gchar *forecast;
    gchar *metar_buffer;
    gchar *iwin_buffer;
    gchar *met_buffer;
    gchar *radar_buffer;
    gchar *radar_url;
    GdkPixbufLoader *radar_loader;
    GdkPixmap *radar;
    GnomeVFSAsyncHandle *metar_handle;
    GnomeVFSAsyncHandle *iwin_handle;
    GnomeVFSAsyncHandle *wx_handle;
    GnomeVFSAsyncHandle *met_handle;
    gboolean requests_pending;
    GWeatherApplet *applet;
};



typedef void (*WeatherInfoFunc) (WeatherInfo *info);

extern gboolean _weather_info_fill (GWeatherApplet *applet, WeatherInfo *info, WeatherLocation *location, WeatherInfoFunc cb);
#define weather_info_new(applet, location,cb) _weather_info_fill((applet), NULL, (location), (cb))
#define weather_info_update(applet, info,cb) _weather_info_fill((applet), (info), NULL, (cb));
extern WeatherInfo *weather_info_clone (const WeatherInfo *info);
extern void weather_info_free (WeatherInfo *info);

extern void weather_info_config_write (WeatherInfo *info);
extern WeatherInfo *weather_info_config_read (PanelApplet *applet);

extern void weather_info_to_metric (WeatherInfo *info);
extern void weather_info_to_imperial (WeatherInfo *info);

extern const gchar *weather_info_get_location (WeatherInfo *info);
extern const gchar *weather_info_get_update (WeatherInfo *info);
extern const gchar *weather_info_get_sky (WeatherInfo *info);
extern const gchar *weather_info_get_conditions (WeatherInfo *info);
extern const gchar *weather_info_get_temp (WeatherInfo *info);
extern const gchar *weather_info_get_dew (WeatherInfo *info);
extern const gchar *weather_info_get_humidity (WeatherInfo *info);
extern const gchar *weather_info_get_wind (WeatherInfo *info);
extern const gchar *weather_info_get_pressure (WeatherInfo *info);
extern const gchar *weather_info_get_visibility (WeatherInfo *info);
extern const gchar *weather_info_get_forecast (WeatherInfo *info);
extern GdkPixmap *weather_info_get_radar (WeatherInfo *info);

extern const gchar *weather_info_get_temp_summary (WeatherInfo *info);
extern gchar *weather_info_get_weather_summary (WeatherInfo *info);

extern void update_finish (WeatherInfo *info);

extern time_t make_time (gint date, gint hour, gint min);

extern void _weather_info_get_pixbuf (WeatherInfo *info, gboolean mini, GdkPixbuf **pixbuf);
#define weather_info_get_pixbuf_mini(info,pixbuf) _weather_info_get_pixbuf((info), TRUE, (pixbuf))
#define weather_info_get_pixbuf(info,pixbuf) _weather_info_get_pixbuf((info), FALSE, (pixbuf))

G_END_DECLS

#endif /* __WEATHER_H_ */

