#ifndef __WEATHER_H_
#define __WEATHER_H_

/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Weather server functions (http://weather.interceptvector.com)
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

struct _WeatherForecast{
	gchar *day;
	gint high;
	gint low;
	gint wid;
	gint precip;
};

enum _WeatherCondition {
	WINDY,
	TSTORMS,
	SNOW,
	RAIN,
	HEAVYRAIN,
	SLEET,
	LIGHTFLURRIES,
	FLURRIES,
	SNOWWIND,
	DUST,
	FOG,
	HAZE,
	SMOKE,
	COLD,
	CLOUDY,
	MOSTLYCLOUDY,
	PARTLYCLOUDY,
	SUNNY,
	MOSTLYSUNNY,
	HOT,
	DEFAULT,
	NUM_COND
};

typedef enum _WeatherCondition WeatherCondition;

struct _WeatherInfo {
	GnomeVFSAsyncHandle *main_handle;
	gboolean success;
	gchar *xml;
  	GList *forecasts;
	gint numforecasts;
	gchar *city;
	gchar *state;
	gchar *date;
	gint curtemp;
	gchar *winddir;
	gint windstrength;
	gfloat barometer;
	gint humidity;
	gint realtemp;
	gfloat visibility;
	gint wid;
};

const gchar * get_conditions (gint wid);
gboolean update_weather (GWeatherApplet *applet);
GdkPixbuf * get_conditions_pixbuf (gint wid);
const gchar * get_wind_direction (gchar *dir);
void start_animation (GWeatherApplet *applet);
void end_animation (GWeatherApplet *applet);

G_END_DECLS

#endif /* __WEATHER_H_ */

