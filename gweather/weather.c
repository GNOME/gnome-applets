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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <values.h>

#ifdef __FreeBSD__
#include <sys/types.h>
#endif
#include <regex.h>
#include <time.h>
#include <unistd.h>

#include <gnome.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <libgnomevfs/gnome-vfs.h>

#include "weather.h"

void
close_cb (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data);
static gchar* formatWeatherMsg (gchar* msg);
static gboolean calc_sun (WeatherInfo *info);

/* FIXME: these global variables will cause conflicts when multiple
** instances of the applets update at the same time
*/
static WeatherForecastType weather_forecast = FORECAST_STATE;
static gboolean weather_radar = FALSE;

#define DATA_SIZE 5000

/* Unit conversions and names */

#define TEMP_F_TO_C(f)  (((f) - 32.0) * 0.555556)
#define TEMP_F_TO_K(f)  (TEMP_F_TO_C(f) + 273.15)
#define TEMP_C_TO_F(c)  (((c) * 1.8) + 32.0)

#define WINDSPEED_KNOTS_TO_KPH(knots)  ((knots) * 1.851965)
#define WINDSPEED_KNOTS_TO_MPH(knots)  ((knots) * 1.150779)
#define WINDSPEED_KNOTS_TO_MS(knots)   ((knots) * 0.514444)

#define PRESSURE_INCH_TO_KPA(inch)   ((inch) * 3.386)
#define PRESSURE_INCH_TO_HPA(inch)   ((inch) * 33.86)
#define PRESSURE_INCH_TO_MM(inch)    ((inch) * 25.40005)
#define PRESSURE_INCH_TO_MB(inch)    (PRESSURE_INCH_TO_HPA(inch))
#define PRESSURE_MBAR_TO_INCH(mbar) ((mbar) * 0.02963742)

#define VISIBILITY_SM_TO_KM(sm)  ((sm) * 1.609344)
#define VISIBILITY_SM_TO_M(sm)   (VISIBILITY_SM_TO_KM(sm) * 1000)

#define DEGREES_TO_RADIANS(deg)   ((fmod(deg,360.) / 180.) * M_PI)
#define RADIANS_TO_DEGREES(rad)   ((rad) * 180. / M_PI)
#define RADIANS_TO_HOURS(rad)     ((rad) * 12. / M_PI)

/*
 * Convert string of the form "DD-MM-SSH" to radians
 * DD:degrees (to 3 digits), MM:minutes, SS:seconds H:hemisphere (NESW)
 * Return value is positive for N,E; negative for S,W.
 */
gdouble dmsh2rad (const gchar *latlon)
{
    char *p1, *p2;
    int deg, min, sec, dir;
    gdouble value;
    
    if (latlon == NULL)
	return DBL_MAX;
    p1 = strchr(latlon, '-');
    p2 = strrchr(latlon, '-');
    if (p1 == NULL || p1 == latlon) {
        return DBL_MAX;
    } else if (p1 == p2) {
	sscanf (latlon, "%u-%u", &deg, &min);
	sec = 0;
    } else if (p2 == 1 + p1) {
	return DBL_MAX;
    } else {
	sscanf (latlon, "%u-%u-%u", &deg, &min, &sec);
    }
    if (deg > 180 || min >= 60 || sec >= 60)
	return DBL_MAX;
    value = (gdouble)((deg * 60 + min) * 60 + sec) * M_PI / 648000.;

    dir = toupper(latlon[strlen(latlon) - 1]);
    if (dir == 'W' || dir == 'S')
	value = -value;
    else if (dir != 'E' && dir != 'N' && (value != 0.0 || dir != '0'))
	value = DBL_MAX;
    return value;
}

WeatherLocation *weather_location_new (const gchar *untrans_name, const gchar *trans_name, const gchar *code,
				       const gchar *zone, const gchar *radar, const gchar *coordinates)
{
    WeatherLocation *location;

    location = g_new(WeatherLocation, 1);
  
    /* untransalted name and metar code must be set */
    location->untrans_name = g_strdup(untrans_name);
    location->code = g_strdup(code);

    /* if there is no translated name, then use the untranslated version */
    if (trans_name) {
        location->trans_name = g_strdup(trans_name);    
    } else {
        location->trans_name = g_strdup(untrans_name);
    }
    
    if (zone) {    
        location->zone = g_strdup(zone);
    } else {
        location->zone = g_strdup("------");
    }
    
    if (radar) {
        location->radar = g_strdup(radar);
    } else {
        location->radar = g_strdup("---");
    }

    if (location->zone[0] == '-') {
        location->zone_valid = FALSE;
    } else {
        location->zone_valid = TRUE;
    }

    char lat[12], lon[12];
    if (coordinates && sscanf(coordinates, "%s %s", lat, lon) == 2) {
        location->coordinates = g_strdup(coordinates);
        location->latitude = dmsh2rad (lat);
	location->longitude = dmsh2rad (lon);
    } else {
        location->coordinates = g_strdup("---");
        location->latitude = DBL_MAX;
        location->longitude = DBL_MAX;
    }
    location->latlon_valid = (location->latitude < DBL_MAX && location->longitude < DBL_MAX);
    
    return location;
}

WeatherLocation *weather_location_config_read (PanelApplet *applet)
{
    WeatherLocation *location;
    gchar *untrans_name, *trans_name, *code, *zone, *radar, *coordinates;
    
    untrans_name = panel_applet_gconf_get_string(applet, "location0", NULL);
    if (!untrans_name) {
        if ( g_strstr_len ("DEFAULT_LOCATION", 16, _("DEFAULT_LOCATION")) == NULL ) {
            /* TRANSLATOR: Change this to the default location name (1st parameter) in the */
            /* gweather/Locations file */
            /* For example for New York (JFK) the entry is loc14=New\\ York-JFK\\ Arpt KJFK NYZ076 nyc */
            /* so this should be translated as "New York-JFK Arpt" */
            untrans_name = g_strdup ( _("DEFAULT_LOCATION") );
            trans_name = g_strdup ( _( _("DEFAULT_LOCATION") ) );
		} else {
    	    untrans_name = g_strdup ("Pittsburgh");
        }
    } else if ( g_strstr_len ("DEFAULT_LOCATION", 16, untrans_name) ) {
        g_free ( untrans_name );
		untrans_name = g_strdup ("Pittsburgh");
        trans_name = g_strdup ( _("Pittsburgh") );
    } else {
        /* Use the stored value */
        trans_name = panel_applet_gconf_get_string (applet, "location4", NULL);
    }

    code = panel_applet_gconf_get_string(applet, "location1", NULL);
    if (!code) { 
        if (g_strstr_len ("DEFAULT_CODE", 12, _("DEFAULT_CODE")) == NULL) {
            /* TRANSLATOR: Change this to the default location code (2nd parameter) in the */
            /* gweather/Locations file */
            /* For example for New York (JFK) the entry is loc14=New\\ York-JFK\\ Arpt KJFK NYZ076 nyc */
            /* so this should be translated as "KJFK" */
            code = g_strdup ( _("DEFAULT_CODE") );
        } else {
    	    code = g_strdup ("KPIT");
        }
    } else if ( g_strstr_len ("DEFAULT_CODE", 12, code) ) {
        g_free (code);
        code = g_strdup ("KPIT");
    }
	
    zone = panel_applet_gconf_get_string(applet, "location2", NULL);
    if (!zone) {
        if (g_strstr_len("DEFAULT_ZONE", 12, _("DEFAULT_ZONE")) == NULL) {
            /* TRANSLATOR: Change this to the default location zone (3rd parameter) in the */
            /* gweather/Locations file */
            /* For example for New York (JFK) the entry is loc14=New\\ York-JFK\\ Arpt KJFK NYZ076 nyc */
            /* so this should be translated as "NYZ076" */
            zone = g_strdup ( _("DEFAULT_ZONE" ) );
        } else {
            zone = g_strdup ("PAZ021");
        }
    } else if ( g_strstr_len ("DEFAULT_ZONE", 12, code) ) {
        g_free (zone);
		zone = g_strdup ("PAZ021");
    }

    radar = panel_applet_gconf_get_string(applet, "location3", NULL);
    if (!radar) {
        if (g_strstr_len("DEFAULT_RADAR", 13, N_("DEFAULT_RADAR")) == NULL) {
            /* Translators: Change this to the default location radar (4th parameter) in the */
            /* gweather/Locations file */
            /* For example for New York (JFK) the entry is loc14=New\\ York-JFK\\ Arpt KJFK NYZ076 nyc */
            /* so this should be translated as "nyc" */
			radar = g_strdup ( _("DEFAULT_RADAR") );
        } else {
            radar = g_strdup ("pit");
        }
    } else if ( g_strstr_len ("DEFAULT_RADAR", 13, radar) ) {
        g_free (radar);
        radar = g_strdup ("pit");
    }

    coordinates = panel_applet_gconf_get_string(applet, "coordinates", NULL);
    if (coordinates && g_strstr_len ("DEFAULT_COORDINATES", 19, coordinates) ) {
        g_free (coordinates);
	coordinates = NULL;
    }
    
    
    location = weather_location_new(untrans_name, trans_name, code, zone, radar, coordinates);
    
    g_free (untrans_name);
    g_free (trans_name);
    g_free (code);
    g_free (zone);
    g_free (radar);
    g_free (coordinates);
    
    return location;
}

WeatherLocation *weather_location_clone (const WeatherLocation *location)
{
    WeatherLocation *clone;

    clone = weather_location_new (location->untrans_name, location->trans_name, 
				  location->code, location->zone,
				  location->radar, location->coordinates);
    clone->latitude = location->latitude;
    clone->longitude = location->longitude;
    clone->latlon_valid = location->latlon_valid;
    return clone;
}

void weather_location_free (WeatherLocation *location)
{
    if (location) {
        g_free (location->untrans_name);
        g_free (location->trans_name);
        g_free (location->code);
        g_free (location->zone);
        g_free (location->radar);
    
        g_free (location);
    }
}

gboolean weather_location_equal (const WeatherLocation *location1, const WeatherLocation *location2)
{
    if (!location1->code || !location2->code)
        return 1;
    return ( (strcmp(location1->code, location2->code) == 0) &&
             (strcmp(location1->untrans_name, location2->untrans_name) == 0) );    
}

static const gchar *wind_direction_str[] = {
    N_("Variable"),
    N_("North"), N_("North - NorthEast"), N_("Northeast"), N_("East - NorthEast"),
    N_("East"), N_("East - Southeast"), N_("Southeast"), N_("South - Southeast"),
    N_("South"), N_("South - Southwest"), N_("Southwest"), N_("West - Southwest"),
    N_("West"), N_("West - Northwest"), N_("Northwest"), N_("North - Northwest")
};

const gchar *weather_wind_direction_string (WeatherWindDirection wind)
{
        if (wind < 0)
	        return _("Unknown");
	if (wind >= (sizeof (wind_direction_str) / sizeof (char *)))
		return _("Invalid");

	return _(wind_direction_str[(int)wind]);
}

static const gchar *sky_str[] = {
    N_("Clear Sky"),
    N_("Broken clouds"),
    N_("Scattered clouds"),
    N_("Few clouds"),
    N_("Overcast")
};

const gchar *weather_sky_string (WeatherSky sky)
{
	if (sky < 0 ||
	    sky >= (sizeof (sky_str) / sizeof (char *)))
		return _("Invalid");

	return _(sky_str[(int)sky]);
}


/*
 * Even though tedious, I switched to a 2D array for weather condition
 * strings, in order to facilitate internationalization, esp. for languages
 * with genders.
 */

/*
 * Almost all reportable combinations listed in
 * http://www.crh.noaa.gov/arx/wx.tbl.html are entered below, except those
 * having 2 qualifiers mixed together [such as "Blowing snow in vicinity"
 * (VCBLSN), "Thunderstorm in vicinity" (VCTS), etc].
 * Combinations that are not possible are filled in with "??".
 * Some other exceptions not handled yet, such as "SN BLSN" which has
 * special meaning.
 */

/*
 * Note, magic numbers, when you change the size here, make sure to change
 * the below function so that new values are recognized
 */
/*                   NONE                         VICINITY                             LIGHT                      MODERATE                      HEAVY                      SHALLOW                      PATCHES                         PARTIAL                      THUNDERSTORM                    BLOWING                      SHOWERS                         DRIFTING                      FREEZING                      */
/*               *******************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************/
static const gchar *conditions_str[24][13] = {
/* TRANSLATOR: If you want to know what "blowing" "shallow" "partial"
 * etc means, you can go to http://www.weather.com/glossary/ and
 * http://www.crh.noaa.gov/arx/wx.tbl.html */
/* NONE          */ {"??",                        "??",                                "??",                      "??",                         "??",                      "??",                        "??",                           "??",                        N_("Thunderstorm"),             "??",                        "??",                           "??",                         "??"                         },
/* DRIZZLE       */ {N_("Drizzle"),               "??",                                N_("Light drizzle"),       N_("Moderate drizzle"),       N_("Heavy drizzle"),       "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         N_("Freezing drizzle")       },
/* RAIN          */ {N_("Rain"),                  "??",                                N_("Light rain"),          N_("Moderate rain"),          N_("Heavy rain"),          "??",                        "??",                           "??",                        N_("Thunderstorm"),             "??",                        N_("Rain showers"),             "??",                         N_("Freezing rain")          },
/* SNOW          */ {N_("Snow"),                  "??",                                N_("Light snow"),          N_("Moderate snow"),          N_("Heavy snow"),          "??",                        "??",                           "??",                        N_("Snowstorm"),                N_("Blowing snowfall"),      N_("Snow showers"),             N_("Drifting snow"),          "??"                         },
/* SNOW_GRAINS   */ {N_("Snow grains"),           "??",                                N_("Light snow grains"),   N_("Moderate snow grains"),   N_("Heavy snow grains"),   "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* ICE_CRYSTALS  */ {N_("Ice crystals"),          "??",                                "??",                      N_("Ice crystals"),           "??",                      "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* ICE_PELLETS   */ {N_("Ice pellets"),           "??",                                N_("Few ice pellets"),     N_("Moderate ice pellets"),   N_("Heavy ice pellets"),   "??",                        "??",                           "??",                        N_("Ice pellet storm"),         "??",                        N_("Showers of ice pellets"),   "??",                         "??"                         },
/* HAIL          */ {N_("Hail"),                  "??",                                "??",                      N_("Hail"),                   "??",                      "??",                        "??",                           "??",                        N_("Hailstorm"),                "??",                        N_("Hail showers"),             "??",                         "??",                        },
/* SMALL_HAIL    */ {N_("Small hail"),            "??",                                "??",                      N_("Small hail"),             "??",                      "??",                        "??",                           "??",                        N_("Small hailstorm"),          "??",                        N_("Showers of small hail"),    "??",                         "??"                         },
/* PRECIPITATION */ {N_("Unknown precipitation"), "??",                                "??",                      "??",                         "??",                      "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* MIST          */ {N_("Mist"),                  "??",                                "??",                      N_("Mist"),                   "??",                      "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* FOG           */ {N_("Fog"),                   N_("Fog in the vicinity") ,          "??",                      N_("Fog"),                    "??",                      N_("Shallow fog"),           N_("Patches of fog"),           N_("Partial fog"),           "??",                           "??",                        "??",                           "??",                         N_("Freezing fog")           },
/* SMOKE         */ {N_("Smoke"),                 "??",                                "??",                      N_("Smoke"),                  "??",                      "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* VOLCANIC_ASH  */ {N_("Volcanic ash"),          "??",                                "??",                      N_("Volcanic ash"),           "??",                      "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* SAND          */ {N_("Sand"),                  "??",                                "??",                      N_("Sand"),                   "??",                      "??",                        "??",                           "??",                        "??",                           N_("Blowing sand"),          "",                             N_("Drifting sand"),          "??"                         },
/* HAZE          */ {N_("Haze"),                  "??",                                "??",                      N_("Haze"),                   "??",                      "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* SPRAY         */ {"??",                        "??",                                "??",                      "??",                         "??",                      "??",                        "??",                           "??",                        "??",                           N_("Blowing sprays"),        "??",                           "??",                         "??"                         },
/* DUST          */ {N_("Dust"),                  "??",                                "??",                      N_("Dust"),                   "??",                      "??",                        "??",                           "??",                        "??",                           N_("Blowing dust"),          "??",                           N_("Drifting dust"),          "??"                         },
/* SQUALL        */ {N_("Squall"),                "??",                                "??",                      N_("Squall"),                 "??",                      "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* SANDSTORM     */ {N_("Sandstorm"),             N_("Sandstorm in the vicinity") ,    "??",                      N_("Sandstorm"),              N_("Heavy sandstorm"),     "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* DUSTSTORM     */ {N_("Duststorm"),             N_("Duststorm in the vicinity") ,    "??",                      N_("Duststorm"),              N_("Heavy duststorm"),     "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* FUNNEL_CLOUD  */ {N_("Funnel cloud"),          "??",                                "??",                      "??",                         "??",                      "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* TORNADO       */ {N_("Tornado"),               "??",                                "??",                      "??",                         "??",                      "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         },
/* DUST_WHIRLS   */ {N_("Dust whirls"),           N_("Dust whirls in the vicinity") ,  "??",                      N_("Dust whirls"),            "??",                      "??",                        "??",                           "??",                        "??",                           "??",                        "??",                           "??",                         "??"                         }
};

const gchar *weather_conditions_string (WeatherConditions cond)
{
    const gchar *str;

    if (!cond.significant) {
	    return "-";
    } else {
	    if (cond.phenomenon >= 0 &&
		cond.phenomenon < 24 &&
		cond.qualifier >= 0 &&
		cond.qualifier < 13)
		    str = _(conditions_str[(int)cond.phenomenon][(int)cond.qualifier]);
	    else
		    str = _("Invalid");
	    return (strlen(str) > 0) ? str : "-";
    }
}

/* Locals turned global to facilitate asynchronous HTTP requests */


static inline gboolean requests_init (WeatherInfoFunc cb, WeatherInfo *info)
{
    if (info->requests_pending)
        return FALSE;

    /*g_assert(!metar_handle && !iwin_handle && !wx_handle && !met_handle);*/

    info->requests_pending = TRUE;
    	
    return TRUE;
}

static inline void request_done (GnomeVFSAsyncHandle *handle, WeatherInfo *info)
{
    if (!handle)
    	return;

    gnome_vfs_async_close(handle, close_cb, info->applet);

    info->sunValid = info->valid && calc_sun(info);
    return;
}

static inline void requests_done_check (WeatherInfo *info)
{
    g_return_if_fail(info->requests_pending);

    if (!info->metar_handle && !info->iwin_handle && 
        !info->wx_handle && !info->met_handle &&
	!info->bom_handle) {
        info->requests_pending = FALSE;
        update_finish(info);
    }
}

void
close_cb (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
	GWeatherApplet *gw_applet = (GWeatherApplet *)data;
	WeatherInfo *info;

	g_return_if_fail (gw_applet != NULL);
	g_return_if_fail (gw_applet->gweather_info != NULL);

	info = gw_applet->gweather_info;

	if (result != GNOME_VFS_OK)
		g_warning("Error closing GnomeVFSAsyncHandle.\n");
	
	if (handle == info->metar_handle)
		info->metar_handle = NULL;
	if (handle == info->iwin_handle)
		info->iwin_handle = NULL;
	if (handle == info->wx_handle)
		info->wx_handle = NULL;
	if (handle == info->met_handle)
		info->met_handle = NULL;
	if (handle == info->bom_handle)
	        info->bom_handle = NULL;
 	
	requests_done_check(info);
		
	return;
}

/* Return time of weather report as secs since epoch UTC */
static time_t make_time (gint utcDate, gint utcHour, gint utcMin)
{
    const time_t now = time(NULL);
    struct tm tm;

    gmtime_r (&now, &tm);

    /* If last reading took place just before midnight UTC on the first, adjust the
     * date downward.  This ASSUMES that the reading won't be more than 24 hrs old! */
    if (utcDate > tm.tm_mday) {
        tm.tm_mday--;
    } else {
        tm.tm_mday = utcDate;
    }
    tm.tm_hour = utcHour;
    tm.tm_min  = utcMin;
    tm.tm_sec  = 0;

    /* mktime() assumes value is local, not UTC.  Use tm_gmtoff to compensate */
   return tm.tm_gmtoff + mktime(&tm);
}

static void metar_tok_time (gchar *tokp, WeatherInfo *info)
{
    gint day, hr, min;

    sscanf(tokp, "%2u%2u%2u", &day, &hr, &min);
    info->update = make_time(day, hr, min);
}

#define CONST_DIGITS "0123456789"

static void metar_tok_wind (gchar *tokp, WeatherInfo *info)
{
    gchar sdir[4], sspd[4], sgust[4];
    gint dir, spd, gust = -1;
    gchar *gustp;
    size_t glen;
    
    strncpy(sdir, tokp, 3);
    sdir[3] = 0;
    dir = (!strcmp(sdir, "VRB")) ? -1 : atoi(sdir);

    memset(sspd, 0, sizeof(sspd));
    glen = strspn(tokp+3, CONST_DIGITS);
    strncpy(sspd, tokp+3, glen);
    spd = atoi(sspd);
    tokp += glen + 3;

    gustp = strchr(tokp, 'G');
    if (gustp) {
        memset(sgust, 0, sizeof(sgust));
	glen = strspn(gustp+1, CONST_DIGITS);
        strncpy(sgust, gustp+1, glen);
        gust = atoi(sgust);
    }

    if ((349 <= dir) || (dir <= 11))
        info->wind = WIND_N;
    else if ((12 <= dir) && (dir <= 33))
        info->wind = WIND_NNE;
    else if ((34 <= dir) && (dir <= 56))
        info->wind = WIND_NE;
    else if ((57 <= dir) && (dir <= 78))
        info->wind = WIND_ENE;
    else if ((79 <= dir) && (dir <= 101))
        info->wind = WIND_E;
    else if ((102 <= dir) && (dir <= 123))
        info->wind = WIND_ESE;
    else if ((124 <= dir) && (dir <= 146))
        info->wind = WIND_SE;
    else if ((147 <= dir) && (dir <= 168))
        info->wind = WIND_SSE;
    else if ((169 <= dir) && (dir <= 191))
        info->wind = WIND_S;
    else if ((192 <= dir) && (dir <= 213))
        info->wind = WIND_SSW;
    else if ((214 <= dir) && (dir <= 236))
        info->wind = WIND_SW;
    else if ((237 <= dir) && (dir <= 258))
        info->wind = WIND_WSW;
    else if ((259 <= dir) && (dir <= 281))
        info->wind = WIND_W;
    else if ((282 <= dir) && (dir <= 303))
        info->wind = WIND_WNW;
    else if ((304 <= dir) && (dir <= 326))
        info->wind = WIND_NW;
    else if ((327 <= dir) && (dir <= 348))
        info->wind = WIND_NNW;
    
    info->windspeed = (WeatherWindSpeed)spd;
}

static void metar_tok_vis (gchar *tokp, WeatherInfo *info)
{
    gchar *pfrac, *pend, *psp;
    gchar sval[6];
    gint num, den, val;

    pend = strstr(tokp, "SM");
    memset(sval, 0, sizeof(sval));

    if (pend) {
        // US observation: field ends with "SM"
        pfrac = strchr(tokp, '/');
        if (pfrac) {
	    if (*tokp == 'M') {
	        info->visibility = 0.001;
	    } else {
	        num = (*(pfrac-1) - '0');
		strncpy(sval, pfrac + 1, pend - pfrac - 1);
		den = atoi(sval);
		info->visibility =
		    ((WeatherVisibility)num / ((WeatherVisibility)den));

		psp = strchr(tokp, ' ');
		if (psp) {
		    *psp = '\0';
		    val = atoi(tokp);
		    info->visibility += (WeatherVisibility)val;
		}
	    }
	} else {
	    strncpy(sval, tokp, pend - tokp);
            val = atoi(sval);
            info->visibility = (WeatherVisibility)val;
	}
    } else {
        // International observation: NNNN(DD NNNNDD)?
        // For now: use only the minimum visibility and ignore its direction
        strncpy (sval, tokp, strspn(tokp, CONST_DIGITS));
	val = atoi(sval);
	info->visibility = (WeatherVisibility)val / VISIBILITY_SM_TO_M(1.);
    }
}

static void metar_tok_cloud (gchar *tokp, WeatherInfo *info)
{
    gchar stype[4], salt[4];
    gint alt = -1;

    if (!strcmp(tokp,"CAVOK")) {
        info->sky=SKY_CLEAR;
	return;
    }

    strncpy(stype, tokp, 3);
    stype[3] = 0;
    if (strlen(tokp) == 6) {
        strncpy(salt, tokp+3, 3);
        salt[3] = 0;
        alt = atoi(salt);  /* Altitude - currently unused */
    }

    if (!strcmp(stype, "CLR")) {
        info->sky = SKY_CLEAR;
    } else if (!strcmp(stype, "SKC")) {
        info->sky = SKY_CLEAR;
    } else if (!strcmp(stype, "NSC")) {
        info->sky = SKY_CLEAR;
    } else if (!strcmp(stype, "BKN")) {
        info->sky = SKY_BROKEN;
    } else if (!strcmp(stype, "SCT")) {
        info->sky = SKY_SCATTERED;
    } else if (!strcmp(stype, "FEW")) {
        info->sky = SKY_FEW;
    } else if (!strcmp(stype, "OVC")) {
        info->sky = SKY_OVERCAST;
    }
}

static void metar_tok_pres (gchar *tokp, WeatherInfo *info)
{
    if (*tokp == 'A') {
        gchar sintg[3], sfract[3];
        gint intg, fract;

        strncpy(sintg, tokp+1, 2);
        sintg[2] = 0;
        intg = atoi(sintg);

        strncpy(sfract, tokp+3, 2);
        sfract[2] = 0;
        fract = atoi(sfract);

        info->pressure = (WeatherPressure)intg + (((WeatherPressure)fract)/100.0);
    } else {  /* *tokp == 'Q' */
        gchar spres[5];
        gint pres;

        strncpy(spres, tokp+1, 4);
        spres[4] = 0;
        pres = atoi(spres);

        info->pressure = PRESSURE_MBAR_TO_INCH((WeatherPressure)pres);
    }
}

/* Relative humidity computation - thanks to <Olof.Oberg@modopaper.modogroup.com> */


static inline gdouble calc_humidity(gdouble temp, gdouble dewp)
{
    gdouble esat, esurf;

    temp = TEMP_F_TO_C(temp);
    dewp = TEMP_F_TO_C(dewp);

    esat = 6.11 * pow(10.0, (7.5 * temp) / (237.7 + temp));
    esurf = 6.11 * pow(10.0, (7.5 * dewp) / (237.7 + dewp));

    return ((esurf/esat) * 100.0);
}

static inline gdouble calc_apparent (WeatherInfo *info)
{
    gdouble temp = info->temp;
    gdouble wind = WINDSPEED_KNOTS_TO_MPH(info->wind);
    gdouble apparent;


    /*
     * Wind chill calculations as of 01-Nov-2001
     * http://www.nws.noaa.gov/om/windchill/index.shtml
     * Some pages suggest that the formula will soon be adjusted
     * to account for solar radiation (bright sun vs cloudy sky)
     */
    if (temp <= 50.0 && wind > 3.0) {
        gdouble v = pow(wind, 0.16);
	apparent = 35.74 + 0.6215 * temp - 35.75 * v + 0.4275 * temp * v;
    }
    /*
     * Heat index calculations:
     * http://www.srh.noaa.gov/fwd/heatindex/heat5.html
     */
    else if (temp >= 80.0) {
        gdouble humidity = calc_humidity(info->temp, info->dew);
        gdouble t2 = temp * temp;
	gdouble h2 = humidity * humidity;

#if 1
	/*
	 * A really precise formula.  Note that overall precision is
	 * constrained by the accuracy of the instruments and that the
	 * we receive the temperature and dewpoints as integers.
	 */
	gdouble t3 = t2 * temp;
	gdouble h3 = h2 * temp;
	
	apparent = 16.923
	          + 0.185212 * temp
	          + 5.37941 * humidity
	          - 0.100254 * temp * humidity
	          + 9.41695e-3 * t2
	          + 7.28898e-3 * h2
	          + 3.45372e-4 * t2 * humidity
	          - 8.14971e-4 * temp * h2
	          + 1.02102e-5 * t2 * h2
	          - 3.8646e-5 * t3
	          + 2.91583e-5 * h3
	          + 1.42721e-6 * t3 * humidity
	          + 1.97483e-7 * temp * h3
	          - 2.18429e-8 * t3 * h2
	          + 8.43296e-10 * t2 * h3
	          - 4.81975e-11 * t3 * h3;
#else
	/*
	 * An often cited alternative: values are within 5 degrees for
	 * most ranges between 10% and 70% humidity and to 110 degrees.
	 */
	apparent = - 42.379
	           +  2.04901523 * temp
	           + 10.14333127 * humidity
	           -  0.22475541 * temp * humidity
	           -  6.83783e-3 * t2
	           -  5.481717e-2 * h2
	           +  1.22874e-3 * t2 * humidity
	           +  8.5282e-4 * temp * h2
	           -  1.99e-6 * t2 * h2;
#endif
    }
    else {
        apparent = temp;
    }

    return apparent;
}


/*
 * Formulas from:
 * "Practical Astronomy With Your Calculator" (3e), Peter Duffett-Smith
 * Cambridge University Press 1988
 *
 * Planetary Mean Orbit parameters from http://ssd.jpl.nasa.gov/elem_planets.html,
 * converting longitudes from heliocentric to geocentric coordinates
 * epoch J2000 (2000 Jan 1 12:00:00 UTC)
 */

#define EPOCH_TO_J2000(t)       (t-946728000)
#define MEAN_ECLIPTIC_LONGITUDE 280.46435
#define PERIGEE_LONGITUDE       282.94719
#define ECCENTRICITY            0.01671002
#define MEAN_ECLIPTIC_OBLIQUITY 
#define SOL_PROGRESSION         (360./365.242191)

/*
 * Convert ecliptic longitude (radians) to equitorial coordinates,
 * expressed as right ascension (hours) and declination (radians)
 * Assume ecliptic latitude is 0.0
 */
static void ecl2equ (gdouble time, gdouble eclipLon,
		     gdouble *ra, gdouble *decl)
{
    gdouble jc = EPOCH_TO_J2000(time) / (36525. * 86400.);
    gdouble mEclipObliq = DEGREES_TO_RADIANS(23.439291667
					     - .013004166 * jc
					     - 1.666667e-7 * jc * jc
					     + 5.027777e-7 * jc * jc * jc);
    *ra = RADIANS_TO_HOURS(atan2 ( sin(eclipLon) * cos(mEclipObliq), cos(eclipLon)));
    if (*ra < 0.)
        *ra += 24.;
    *decl = asin ( sin(mEclipObliq) * sin(eclipLon) );
}

/*
 * Calculate rising and setting times of an object
 * based on it equitorial coordinates
 */
static void gstObsv (gdouble ra, gdouble decl,
		     gdouble obsLat, gdouble obsLon,
		     gdouble *rise, gdouble *set)
{
    double a = acos(-tan(obsLat) * tan(decl));
    double b;

    if (isnan(a)) {
	*set = *rise = a;
	return;
    }
    a = RADIANS_TO_HOURS(a);
    b = 24. - a + ra;
    a += ra;
    a -= RADIANS_TO_HOURS(obsLon);
    b -= RADIANS_TO_HOURS(obsLon);
    if ((a = fmod(a, 24.)) < 0)
      a += 24.;
    if ((b = fmod(b, 24.)) < 0)
      b += 24.;

    *set = a;
    *rise = b;
}


static gdouble t0(time_t date)
{
    register gdouble t = ((gdouble)(EPOCH_TO_J2000(date) / 86400)) / 36525.0;
    gdouble t0 = fmod( 6.697374558 + 2400.051366 * t + 2.5862e-5 * t * t, 24.);
    if (t0 < 0.)
        t0 += 24.;
    return t0;
}



gboolean sun (time_t t, gdouble obsLat, gdouble obsLon,
	  time_t *rise, time_t *set)
{
    time_t gm_midn;
    time_t lcl_midn;
    gdouble gm_hoff;
    
    /* Approximate preceding local midnight at observer's longitude */
    gm_midn = t - (t % 86400);
    gm_hoff = floor((RADIANS_TO_DEGREES(obsLon) + 7.5) / 15.);
    lcl_midn = gm_midn - 3600. * gm_hoff;
    if (t - lcl_midn >= 86400)
        lcl_midn += 86400;
    else if (lcl_midn > t)
        lcl_midn -= 86400;
    
    /* 
     * Ecliptic longitude of the sun at midnight local time
     * Start with an estimate based on a fixed daily rate
     */
    gdouble ndays = EPOCH_TO_J2000(lcl_midn) / 86400.;
    gdouble meanAnom = DEGREES_TO_RADIANS(ndays * SOL_PROGRESSION
					  + MEAN_ECLIPTIC_LONGITUDE - PERIGEE_LONGITUDE);
    
    /*
     * Approximate solution of Kepler's equation:
     * Find E which satisfies  E - e sin(E) = M (mean anomaly)
     */
    gdouble eccenAnom = meanAnom;
    gdouble delta;
    gdouble lambda;
    
    while (1e-12 < fabs( delta = eccenAnom - ECCENTRICITY * sin(eccenAnom) - meanAnom))
    {
	eccenAnom -= delta / (1.- ECCENTRICITY * cos(eccenAnom));
    }

    /* Sun's longitude on the ecliptic */
    lambda = fmod( DEGREES_TO_RADIANS ( PERIGEE_LONGITUDE )
		   + 2. * atan( sqrt((1.+ECCENTRICITY)/(1.-ECCENTRICITY))
				* tan ( eccenAnom / 2. ) ),
		   2. * M_PI);
    
    /* Calculate equitorial coordinates of sun at previous and next local midnights */

    gdouble ra1, ra2, decl1, decl2;
    
    ecl2equ (lcl_midn, lambda, &ra1, &decl1);
    ecl2equ (lcl_midn + 86400., lambda + DEGREES_TO_RADIANS(SOL_PROGRESSION), &ra2, &decl2);
    
    /* Convert to rise and set times assuming the earth were to stay in its position
     * in its orbit around the sun.  This will soon be followed by interpolating
     * between the two
     */

    gdouble rise1, rise2, set1, set2;
    gstObsv (ra1, decl1, obsLat, obsLon - (gm_hoff * M_PI / 12.), &rise1, &set1);
    gstObsv (ra2, decl2, obsLat, obsLon - (gm_hoff * M_PI / 12.), &rise2, &set2);
    
    /* TODO: include calculations for regions near the poles. */
    if (isnan(rise1) || isnan(rise2))
        return FALSE;

    gdouble tt = t0(lcl_midn);
    gdouble t00 = tt - (gm_hoff + RADIANS_TO_HOURS(obsLon)) * 1.002737909;

    if (t00 < 0.)
	t00 += 24.;

    if (rise1 < t00) {
        rise1 += 24.;
        rise2 += 24.;
    }
    if (set1 < t00) {
        set1  += 24.;
        set2  += 24.;
    }
    
    /*
     * Interpolate between the two
     */
    rise1 = (24.07 * rise1 - t00 * (rise2 - rise1)) / (24.07 + rise1 - rise2);
    set1 = (24.07 * set1 - t00 * (set2 - set1)) / (24.07 + set1 - set2);

    decl2 = (decl1 + decl2) / 2.;
    gdouble x = DEGREES_TO_RADIANS(0.830725);
    gdouble u = acos ( sin(obsLat) / cos(decl2) );
    gdouble dt =  RADIANS_TO_HOURS ( asin ( sin(x) / sin(u) )
				     / cos(decl2) );
    
    rise1 = (rise1 - dt - tt) * 0.9972695661;
    set1  = (set1 + dt - tt) * 0.9972695661;
    if (rise1 < 0.)
      rise1 += 24;
    else if (rise1 >= 24.)
      rise1 -= 24.;
    if (set1 < 0.)
      set1 += 24;
    else if (set1 >= 24.)
      set1 -= 24.;
    
    *rise = (rise1 * 3600.) + lcl_midn;
    *set = (set1 * 3600.) + lcl_midn;
    return TRUE;
}


static gboolean calc_sun (WeatherInfo *info)
{
  return info->location->latlon_valid
    && sun(info->update,
	   info->location->latitude, info->location->longitude,
	   &info->sunrise, &info->sunset);
}

static void metar_tok_temp (gchar *tokp, WeatherInfo *info)
{
    gchar *ptemp, *pdew, *psep;

    psep = strchr(tokp, '/');
    *psep = 0;
    ptemp = tokp;
    pdew = psep + 1;

    info->temp = (*ptemp == 'M') ? TEMP_C_TO_F(-atoi(ptemp+1))
                                 : TEMP_C_TO_F(atoi(ptemp));
    info->dew = (*pdew == 'M') ? TEMP_C_TO_F(-atoi(pdew+1))
                               : TEMP_C_TO_F(atoi(pdew));
}

static void metar_tok_cond (gchar *tokp, WeatherInfo *info)
{
    gchar squal[3], sphen[4];
    gchar *pphen;

    if ((strlen(tokp) > 3) && ((*tokp == '+') || (*tokp == '-')))
        ++tokp;   /* FIX */

    if ((*tokp == '+') || (*tokp == '-'))
        pphen = tokp + 1;
    else if (strlen(tokp) < 4)
        pphen = tokp;
    else
        pphen = tokp + 2;

    memset(squal, 0, sizeof(squal));
    strncpy(squal, tokp, pphen - tokp);
    squal[pphen - tokp] = 0;

    memset(sphen, 0, sizeof(sphen));
    strncpy(sphen, pphen, sizeof(sphen));
    sphen[sizeof(sphen)-1] = '\0';

    /* Defaults */
    info->cond.qualifier = QUALIFIER_NONE;
    info->cond.phenomenon = PHENOMENON_NONE;
    info->cond.significant = FALSE;

    if (!strcmp(squal, "")) {
        info->cond.qualifier = QUALIFIER_MODERATE;
    } else if (!strcmp(squal, "-")) {
        info->cond.qualifier = QUALIFIER_LIGHT;
    } else if (!strcmp(squal, "+")) {
        info->cond.qualifier = QUALIFIER_HEAVY;
    } else if (!strcmp(squal, "VC")) {
        info->cond.qualifier = QUALIFIER_VICINITY;
    } else if (!strcmp(squal, "MI")) {
        info->cond.qualifier = QUALIFIER_SHALLOW;
    } else if (!strcmp(squal, "BC")) {
        info->cond.qualifier = QUALIFIER_PATCHES;
    } else if (!strcmp(squal, "PR")) {
        info->cond.qualifier = QUALIFIER_PARTIAL;
    } else if (!strcmp(squal, "TS")) {
        info->cond.qualifier = QUALIFIER_THUNDERSTORM;
    } else if (!strcmp(squal, "BL")) {
        info->cond.qualifier = QUALIFIER_BLOWING;
    } else if (!strcmp(squal, "SH")) {
        info->cond.qualifier = QUALIFIER_SHOWERS;
    } else if (!strcmp(squal, "DR")) {
        info->cond.qualifier = QUALIFIER_DRIFTING;
    } else if (!strcmp(squal, "FZ")) {
        info->cond.qualifier = QUALIFIER_FREEZING;
    } else {
        g_return_if_fail(FALSE);
    }

    if (!strcmp(sphen, "DZ")) {
        info->cond.phenomenon = PHENOMENON_DRIZZLE;
    } else if (!strcmp(sphen, "RA")) {
        info->cond.phenomenon = PHENOMENON_RAIN;
    } else if (!strcmp(sphen, "SN")) {
        info->cond.phenomenon = PHENOMENON_SNOW;
    } else if (!strcmp(sphen, "SG")) {
        info->cond.phenomenon = PHENOMENON_SNOW_GRAINS;
    } else if (!strcmp(sphen, "IC")) {
        info->cond.phenomenon = PHENOMENON_ICE_CRYSTALS;
    } else if (!strcmp(sphen, "PE")) {
        info->cond.phenomenon = PHENOMENON_ICE_PELLETS;
    } else if (!strcmp(sphen, "GR")) {
        info->cond.phenomenon = PHENOMENON_HAIL;
    } else if (!strcmp(sphen, "GS")) {
        info->cond.phenomenon = PHENOMENON_SMALL_HAIL;
    } else if (!strcmp(sphen, "UP")) {
        info->cond.phenomenon = PHENOMENON_UNKNOWN_PRECIPITATION;
    } else if (!strcmp(sphen, "BR")) {
        info->cond.phenomenon = PHENOMENON_MIST;
    } else if (!strcmp(sphen, "FG")) {
        info->cond.phenomenon = PHENOMENON_FOG;
    } else if (!strcmp(sphen, "FU")) {
        info->cond.phenomenon = PHENOMENON_SMOKE;
    } else if (!strcmp(sphen, "VA")) {
        info->cond.phenomenon = PHENOMENON_VOLCANIC_ASH;
    } else if (!strcmp(sphen, "SA")) {
        info->cond.phenomenon = PHENOMENON_SAND;
    } else if (!strcmp(sphen, "HZ")) {
        info->cond.phenomenon = PHENOMENON_HAZE;
    } else if (!strcmp(sphen, "PY")) {
        info->cond.phenomenon = PHENOMENON_SPRAY;
    } else if (!strcmp(sphen, "DU")) {
        info->cond.phenomenon = PHENOMENON_DUST;
    } else if (!strcmp(sphen, "SQ")) {
        info->cond.phenomenon = PHENOMENON_SQUALL;
    } else if (!strcmp(sphen, "SS")) {
        info->cond.phenomenon = PHENOMENON_SANDSTORM;
    } else if (!strcmp(sphen, "DS")) {
        info->cond.phenomenon = PHENOMENON_DUSTSTORM;
    } else if (!strcmp(sphen, "PO")) {
        info->cond.phenomenon = PHENOMENON_DUST_WHIRLS;
    } else if (!strcmp(sphen, "+FC")) {
        info->cond.phenomenon = PHENOMENON_TORNADO;
    } else if (!strcmp(sphen, "FC")) {
        info->cond.phenomenon = PHENOMENON_FUNNEL_CLOUD;
    } else {
        g_return_if_fail(FALSE);
    }

    if ((info->cond.qualifier != QUALIFIER_NONE) || (info->cond.phenomenon != PHENOMENON_NONE))
        info->cond.significant = TRUE;
}

#define TIME_RE_STR  "^([0-9]{6})Z"
#define WIND_RE_STR  "^(([0-9]{3})|VRB)([0-9]?[0-9]{2})(G[0-9]?[0-9]{2})?KT"
#define VIS_RE_STR   "^((([0-9]?[0-9])|(M?([12] )?([1357]/1?[0-9])))SM)|" \
                     "([0-9]{4}(N|NE|E|SE|S|SW|W|NW( [0-9]{4}(N|NE|E|SE|S|SW|W|NW))?)?)"
#define CLOUD_RE_STR "^(((CLR|BKN|SCT|FEW|OVC|SKC|NSC)([0-9]{3})?(CB|TCU)? ?)+|CAVOK)"
#define TEMP_RE_STR  "^(M?[0-9][0-9])/(M?(//|[0-9][0-9]))"
#define PRES_RE_STR  "^(A|Q)([0-9]{4})"
#define COND_RE_STR  "^(-|\\+)?(VC|MI|BC|PR|TS|BL|SH|DR|FZ)?(DZ|RA|SN|SG|IC|PE|GR|GS|UP|BR|FG|FU|VA|SA|HZ|PY|DU|SQ|SS|DS|PO|\\+?FC)"

#define TIME_RE   0
#define WIND_RE   1
#define VIS_RE    2
#define CLOUD_RE  3
#define TEMP_RE   4
#define PRES_RE   5
#define COND_RE   6

#define RE_NUM   7

static regex_t metar_re[RE_NUM];
static void (*metar_f[RE_NUM])(gchar *tokp, WeatherInfo *info);

static void metar_init_re (void)
{
    static gboolean initialized = FALSE;
    if (initialized)
        return;
    initialized = TRUE;

    regcomp(&metar_re[TIME_RE], TIME_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[WIND_RE], WIND_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[VIS_RE], VIS_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[CLOUD_RE], CLOUD_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[TEMP_RE], TEMP_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[PRES_RE], PRES_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[COND_RE], COND_RE_STR, REG_EXTENDED);

    metar_f[TIME_RE] = metar_tok_time;
    metar_f[WIND_RE] = metar_tok_wind;
    metar_f[VIS_RE] = metar_tok_vis;
    metar_f[CLOUD_RE] = metar_tok_cloud;
    metar_f[TEMP_RE] = metar_tok_temp;
    metar_f[PRES_RE] = metar_tok_pres;
    metar_f[COND_RE] = metar_tok_cond;
}

/*static*/ gboolean metar_parse (gchar *metar, WeatherInfo *info)
{
    gchar *p;
    gint ntoks;
    gint i;
    regmatch_t rm;
    gchar *tokp;

    g_return_val_if_fail(info != NULL, FALSE);
    g_return_val_if_fail(metar != NULL, FALSE);

    metar_init_re();
    p = metar;
    
    for (i = 0; i < RE_NUM; i++) {
        if (*p == '\0' || !strncmp(p, "RMK", 3))
	    break;

        if (regexec(&metar_re[i], p, 1, &rm, 0) == 0) {
	    tokp = g_strndup(p+rm.rm_so, rm.rm_eo);
	    metar_f[i](tokp, info);
	    g_free (tokp);
	    p += rm.rm_so + rm.rm_eo;
	    p += strspn(p, " ");
	}
    }
    return TRUE;
}

static void metar_finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, 
			      gpointer buffer, GnomeVFSFileSize requested, 
			      GnomeVFSFileSize body_len, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
    WeatherInfo *info;
    WeatherLocation *loc;
    gchar *metar, *eoln, *body, *temp;
    gboolean success = FALSE;
    gchar *searchkey;

    g_return_if_fail(gw_applet != NULL);
    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(handle == gw_applet->gweather_info->metar_handle);
	
    info = gw_applet->gweather_info;
    loc = info->location;
    body = (gchar *)buffer;

    body[body_len] = '\0';

    if (info->metar_buffer == NULL)
    	info->metar_buffer = g_strdup(body);
    else
    {
	temp = g_strdup(info->metar_buffer);
	g_free(info->metar_buffer);
	info->metar_buffer = g_strdup_printf("%s%s", temp, body);
	g_free(temp);
    }
		
    if (result == GNOME_VFS_ERROR_EOF)
    {

        searchkey = g_strdup_printf("\n%s", loc->code);

        metar = strstr(info->metar_buffer, searchkey);
        g_free (searchkey);
        if (metar == NULL) {
            success = FALSE;
        } else {
            metar += WEATHER_LOCATION_CODE_LEN + 2;
            eoln = strchr(metar, '\n');
	    if (eoln != NULL)
		    *eoln = 0;
            success = metar_parse(metar, info);
	    if (eoln != NULL)
		    *eoln = '\n';
        }

        info->valid = success;
    }
    else if (result != GNOME_VFS_OK) {
	g_print("%s", gnome_vfs_result_to_string(result));
        g_warning(_("Failed to get METAR data.\n"));
    } else {
	 gnome_vfs_async_read(handle, body, DATA_SIZE - 1, metar_finish_read, gw_applet);
	 return;      
    }
    
    request_done(info->metar_handle, info);
    g_free (buffer);
    return;
}

static void metar_finish_open (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
    WeatherInfo *info;
    WeatherLocation *loc;
    gchar *body;
    int body_len;
    gchar *metar, *eoln;
    gboolean success = FALSE;

    g_return_if_fail(gw_applet != NULL);
    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(handle == gw_applet->gweather_info->metar_handle);

    info = gw_applet->gweather_info;
   
    body = g_malloc0(DATA_SIZE);
    
    if (info->metar_buffer)
    	g_free (info->metar_buffer);
    info->metar_buffer = NULL;
    loc = info->location;
    if (loc == NULL) {
	    g_warning (_("WeatherInfo missing location"));
	    request_done(info->metar_handle, info);
	    requests_done_check(info);
	    g_free (body);
	    return;
    }

    if (result != GNOME_VFS_OK) {
        g_warning(_("Failed to get METAR data.\n"));
        info->metar_handle = NULL;
	requests_done_check(info); 
	g_free (body);
    } else {
        gnome_vfs_async_read(handle, body, DATA_SIZE - 1, metar_finish_read, gw_applet);
    }
    return;
}

/* Read current conditions and fill in info structure */
static void metar_start_open (WeatherInfo *info)
{
    gchar *url;
    WeatherLocation *loc;

    g_return_if_fail(info != NULL);
    info->valid = FALSE;
    loc = info->location;
    if (loc == NULL) {
	    g_warning (_("WeatherInfo missing location"));
	    return;
    }

    url = g_strdup_printf("http://weather.noaa.gov/cgi-bin/mgetmetar.pl?cccc=%s", loc->code);
    gnome_vfs_async_open(&info->metar_handle, url, GNOME_VFS_OPEN_READ, 
    		         0, metar_finish_open, info->applet);
    g_free(url);

}

#define IWIN_RE_STR "([A-Z][A-Z]Z(([0-9]{3}>[0-9]{3}-)|([0-9]{3}-))+)+([0-9]{6}-)?"

static regex_t iwin_re;

static void iwin_init_re (void)
{
    static gboolean initialized = FALSE;
    if (initialized)
        return;
    initialized = TRUE;

    regcomp(&iwin_re, IWIN_RE_STR, REG_EXTENDED);
}

#define CONST_ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

static gboolean iwin_range_match (gchar *range, WeatherLocation *loc)
{
    gchar *zonep;
    gchar *endp;
    gchar zone_state[4], zone_num_str[4];
    gint zone_num;

    endp = range + strcspn(range, " \t\r\n") - 1;
    if (strspn(endp - 6, CONST_DIGITS) == 6) {
        --endp;
        while (*endp != '-')
            --endp;
    }
    g_assert(range <= endp);

    strncpy(zone_state, loc->zone, 3);
    zone_state[3] = 0;
    strncpy(zone_num_str, loc->zone + 3, 3);
    zone_num_str[3] = 0;
    zone_num = atoi(zone_num_str);

    zonep = range;
    while (zonep < endp) {
        gchar from_str[4], to_str[4];
        gint from, to;

        if (strncmp(zonep, zone_state, 3) != 0) {
            zonep += 3;
            zonep += strcspn(zonep, CONST_ALPHABET "\n");
            continue;
        }

        zonep += 3;

        do {
            strncpy(from_str, zonep, 3);
            from_str[3] = 0;
            from = atoi(from_str);

            zonep += 3;

            if (*zonep == '-') {
                ++zonep;
                to = from;
            } else if (*zonep == '>') {
                ++zonep;
                strncpy(to_str, zonep, 3);
                to_str[3] = 0;
                to = atoi(to_str);
                zonep += 4;
            } else {
                g_assert_not_reached();
                to = 0;  /* Hush compiler warning */
            }

            if ((from <= zone_num) && (zone_num <= to))
                return TRUE;

        } while (!isupper(*zonep) && (zonep < endp));
    }

    return FALSE;
}

static gchar *iwin_parse (gchar *iwin, WeatherLocation *loc)
{
    gchar *p, *rangep = NULL;
    regmatch_t match[1];
    gint ret;

    g_return_val_if_fail(iwin != NULL, NULL);
    g_return_val_if_fail(loc != NULL, NULL);
    if (loc->untrans_name[0] == '-')
        return NULL;
	
    iwin_init_re();

    p = iwin;

    /* Strip CR from CRLF input */
    while ((p = strchr(p, '\r')) != NULL)
        *p = ' ';

    p = iwin;

    while ((ret = regexec(&iwin_re, p, 1, match, 0)) != REG_NOMATCH) {
        rangep = p + match[0].rm_so;
        p = strchr(rangep, '\n');
        if (iwin_range_match(rangep, loc)) 
        {
            break;
	}
    }

    if (ret != REG_NOMATCH) {
        /*gchar *end = strstr(p, "\n</PRE>");
        if ((regexec(&iwin_re, p, 1, match, 0) != REG_NOMATCH) &&
            (end - p > match[0].rm_so))
            end = p + match[0].rm_so - 1;
        *end = 0;*/
        return g_strdup(rangep);
    } else {
        return NULL;
    }

}

static void iwin_finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, 
			     gpointer buffer, GnomeVFSFileSize requested, 
			     GnomeVFSFileSize body_len, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
    WeatherInfo *info;
    WeatherLocation *loc;
    gchar *body, *temp;
    
    g_return_if_fail(gw_applet != NULL);
    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(handle == gw_applet->gweather_info->iwin_handle);

    info = gw_applet->gweather_info;
	
    info->forecast = NULL;
    loc = info->location;
    body = (gchar *)buffer;
    body[body_len] = '\0';

    if (info->iwin_buffer == NULL)
	info->iwin_buffer = g_strdup(body);
    else
    {
	temp = g_strdup(info->iwin_buffer);
	g_free(info->iwin_buffer);
	info->iwin_buffer = g_strdup_printf("%s%s", temp, body);
	g_free(temp);
    }
	
    if (result == GNOME_VFS_ERROR_EOF)
    {
        info->forecast = formatWeatherMsg(g_strdup (info->iwin_buffer));
    }
    else if (result != GNOME_VFS_OK) {
	g_print("%s", gnome_vfs_result_to_string(result));
        g_warning("Failed to get IWIN data.\n");
    } else {
        gnome_vfs_async_read(handle, body, DATA_SIZE - 1, iwin_finish_read, gw_applet);
        return;
    }
    
    request_done(info->iwin_handle, info);
    g_free (buffer);
    return;
}


/**
 *  Human's don't deal well with .MONDAY...SUNNY AND BLAH BLAH.TUESDAY...THEN THIS AND THAT.WEDNESDAY...RAINY BLAH BLAH.
 *  This function makes it easier to read.
 */
static gchar* formatWeatherMsg (gchar* forecast) {

    gchar* ptr = forecast;
    gchar* startLine = NULL;

    while (0 != *ptr) {
        if (ptr[0] == '\n' && ptr[1] == '.') {
            if (NULL == startLine) {
                memmove(forecast, ptr, strlen(ptr) + 1);
                ptr[0] = ' ';
                ptr = forecast;
            }
            ptr[1] = '\n';
            ptr += 2;
            startLine = ptr;

        } else if (ptr[0] == '.' && ptr[1] == '.' && ptr[2] == '.' && NULL != startLine) {
            memmove(startLine + 2, startLine, (ptr - startLine) * sizeof(gchar));
            startLine[0] = ' ';
            startLine[1] = '\n';
            ptr[2] = '\n';

            ptr += 3;

        } else if (ptr[0] == '$' && ptr[1] == '$') {
            ptr[0] = ptr[1] = ' ';

        } else {
            ptr++;
        }
    }

    return forecast;
}


static void iwin_finish_open (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
    WeatherInfo *info;
    WeatherLocation *loc;
    gchar *body;
    gint body_len;
    gchar *forecast;

    g_return_if_fail(gw_applet != NULL);
    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(handle == gw_applet->gweather_info->iwin_handle);

    info = gw_applet->gweather_info;
	
    body = g_malloc0(DATA_SIZE);

    if (info->iwin_buffer)
    	g_free (info->iwin_buffer);
    info->iwin_buffer = NULL;	
    if (info->forecast)
    g_free (info->forecast);
    info->forecast = NULL;
    loc = info->location;
    if (loc == NULL) {
	    g_warning (_("WeatherInfo missing location"));
	    request_done(info->iwin_handle, info);
	    info->iwin_handle = NULL;
	    requests_done_check(info);
	    g_free (body);
	    return;
    }

    if (result != GNOME_VFS_OK) {
        /* forecast data is not really interesting anyway ;) */
	  g_warning("Failed to get IWIN forecast data.\n"); 
        info->iwin_handle = NULL;
        requests_done_check (info);
        g_free (body);
    } else {
        gnome_vfs_async_read(handle, body, DATA_SIZE - 1, iwin_finish_read, gw_applet);
    }
    return;
}

static char *met_reprocess(char *x, int len)
{
	char *p = x;
	char *o;
	int spacing = 0;
	static gchar *buf;
	static gint buflen = 0;
	gchar *lastspace = NULL;
	int count = 0;
	int lastcount = 0;
	
	if(buflen < len)
	{
		if(buf)
			g_free(buf);
		buf=g_malloc(len + 1);
		buflen=len;
	}
		
	o=buf;
	x += len;       /* End mark */

	while(*p && p < x)
	{
		if(isspace(*p))
		{
			if(!spacing)
			{
				spacing = 1;
				lastspace = o;
				count++;
				*o++ = ' ';
			}
			p++;
			continue;
		}
		spacing = 0;
		if(count > 75 && lastspace)
		{
			count = o - lastspace - 1;
			*lastspace = '\n';
			lastspace = NULL;
		}

		if(*p=='&')
		{
			if(strncasecmp(p, "&amp;", 5)==0)
			{
				*o++='&';
				count++;
				p+=5;
				continue;
			}
			if(strncasecmp(p, "&lt;", 4)==0)
			{
				*o++='<';
				count++;
				p+=4;
				continue;
			}
			if(strncasecmp(p, "&gt;", 4)==0)
			{
				*o++='>';
				count++;
				p+=4;
				continue;
			}
		}
		if(*p=='<')
		{
			if(strncasecmp(p, "<BR>", 4)==0)
			{
				*o++='\n';
				count = 0;
			}
			if(strncasecmp(p, "<B>", 3)==0)
			{
				*o++='\n';
				*o++='\n';
				count = 0;
			}
			p++;
			while(*p && *p != '>')
				p++;
			if(*p)
				p++;
			continue;
		}
		*o++=*p++;
		count++;
	}
	*o=0;
	return buf;
}


/*
 * Parse the metoffice forecast info.
 * For gnome 3.0 we want to just embed an HTML bonobo component and 
 * be done with this ;) 
 */

static gchar *met_parse (gchar *meto, WeatherLocation *loc)
{ 
    gchar *p;
    gchar *rp;
    gchar *r = g_strdup("Met Office Forecast\n");
    gchar *t;    
    gint i=0;

    p = strstr(meto, "Summary: </b>");
    if(p == NULL)
	    return r;
    p += 13;
    rp = strstr(p, "Text issued at:");
    if(rp == NULL)
	    return r;

    /* p to rp is the text block we want but in HTML malformat */
    t = g_strconcat(r, met_reprocess(p, rp-p), NULL);
    free(r);
    return t;
}

static void met_finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, 
			    gpointer buffer, GnomeVFSFileSize requested, 
			    GnomeVFSFileSize body_len, gpointer data)
{
    GWeatherApplet* gw_applet = (GWeatherApplet *)data;
    WeatherInfo *info;
    WeatherLocation *loc;
    gchar *body, *forecast, *temp;

    g_return_if_fail(gw_applet != NULL);
	g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(handle == gw_applet->gweather_info->met_handle);

    info = gw_applet->gweather_info;
	
    info->forecast = NULL;
    loc = info->location;
    body = (gchar *)buffer;
    body[body_len] = '\0';

    if (info->met_buffer == NULL)
        info->met_buffer = g_strdup(body);
    else
    {
        temp = g_strdup(info->met_buffer);
	g_free(info->met_buffer);
	info->met_buffer = g_strdup_printf("%s%s", temp, body);
	g_free(temp);
    }
	
    if (result == GNOME_VFS_ERROR_EOF)
    {
	forecast = met_parse(info->met_buffer, loc);
        info->forecast = forecast;
    }
    else if (result != GNOME_VFS_OK) {
	g_print("%s", gnome_vfs_result_to_string(result));
	info->met_handle = NULL;
	requests_done_check (info);
        g_warning("Failed to get Met Office data.\n");
    } else {
        gnome_vfs_async_read(handle, body, DATA_SIZE - 1, met_finish_read, gw_applet);
        return;
    }
    
    request_done(info->met_handle, info);
    g_free (buffer);
    return;
}

static void met_finish_open (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
    GWeatherApplet* gw_applet = (GWeatherApplet *) data;
    WeatherInfo *info;
    WeatherLocation *loc;
    gchar *body;
    gint body_len;
    gchar *forecast;

    g_return_if_fail(gw_applet != NULL);
    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(handle == gw_applet->gweather_info->met_handle);

	info = gw_applet->gweather_info;

    body = g_malloc0(DATA_SIZE);

    info->met_buffer = NULL;
    if (info->forecast)
    	g_free (info->forecast);	
    info->forecast = NULL;
    loc = info->location;
    g_return_if_fail(loc != NULL);

    if (result != GNOME_VFS_OK) {
        g_warning("Failed to get Met Office forecast data.\n");
        info->met_handle = NULL;
        requests_done_check (info);
        g_free (body);
    } else {
    	gnome_vfs_async_read(handle, body, DATA_SIZE - 1, met_finish_read, gw_applet);
    }
    return;
}

static void metoffice_start_open (WeatherInfo *info)
{
    gchar *url;
    WeatherLocation *loc;
    loc = info->location;
  
    url = g_strdup_printf("http://www.metoffice.gov.uk/weather/europe/uk/%s.html", loc->zone+1);

    gnome_vfs_async_open(&info->met_handle, url, GNOME_VFS_OPEN_READ, 
    			 0, met_finish_open, info->applet);
    g_free(url);

    return;
}	

static gchar *bom_parse (gchar *meto)
{ 
    gchar *p, *rp;
    
    p = strstr(meto, "<pre>");
    p += 5; /* skip the <pre> */
    rp = strstr(p, "</pre>");

    return g_strndup(p, rp-p);
}

static void bom_finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, 
			    gpointer buffer, GnomeVFSFileSize requested, 
			    GnomeVFSFileSize body_len, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
    WeatherInfo *info;
    WeatherLocation *loc;
    gchar *body, *forecast, *temp;

    g_return_if_fail(gw_applet != NULL);
    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(handle == gw_applet->gweather_info->bom_handle);

    info = gw_applet->gweather_info;
	
    info->forecast = NULL;
    loc = info->location;
    body = (gchar *)buffer;
    body[body_len] = '\0';
	
    if (info->bom_buffer == NULL)
        info->bom_buffer = g_strdup(body);
    else
    {
        temp = g_strdup(info->bom_buffer);
	g_free(info->bom_buffer);
	info->bom_buffer = g_strdup_printf("%s%s", temp, body);
	g_free(temp);
    }
	
    if (result == GNOME_VFS_ERROR_EOF)
    {
	forecast = bom_parse(info->bom_buffer);
        info->forecast = forecast;
    }
    else if (result != GNOME_VFS_OK) {
	info->bom_handle = NULL;
	requests_done_check (info);
        g_warning("Failed to get BOM data.\n");
    } else {
	gnome_vfs_async_read(handle, body, DATA_SIZE - 1, bom_finish_read, gw_applet);

	return;
    }
    
    request_done(info->bom_handle, info);
    g_free (buffer);
    return;
}

static void bom_finish_open (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
    WeatherInfo *info;
    WeatherLocation *loc;
    gchar *body;
    gint body_len;
    gchar *forecast;

    g_return_if_fail(gw_applet != NULL);
    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(handle == gw_applet->gweather_info->bom_handle);

    info = gw_applet->gweather_info;

    body = g_malloc0(DATA_SIZE);

    info->bom_buffer = NULL;
    if (info->forecast)
    	g_free (info->forecast);	
    info->forecast = NULL;
    loc = info->location;
    g_return_if_fail(loc != NULL);

    if (result != GNOME_VFS_OK) {
        g_warning("Failed to get BOM forecast data.\n");
        info->bom_handle = NULL;
        requests_done_check (info);
        g_free (body);
    } else {
    	gnome_vfs_async_read(handle, body, DATA_SIZE - 1, bom_finish_read, gw_applet);
    }
    return;
}

static void bom_start_open (WeatherInfo *info)
{
    gchar *url;
    WeatherLocation *loc;
    loc = info->location;
  
    url = g_strdup_printf("http://www.bom.gov.au/cgi-bin/wrap_fwo.pl?%s.txt",
			  loc->zone+1);

    gnome_vfs_async_open(&info->bom_handle, url, GNOME_VFS_OPEN_READ, 
    			 0, bom_finish_open, info->applet);
    g_free(url);

    return;
}	

/* Get forecast into newly alloc'ed string */
static void iwin_start_open (WeatherInfo *info)
{
    gchar *url, *state, *zone;
    WeatherLocation *loc;

    g_return_if_fail(info != NULL);
    loc = info->location;
    g_return_if_fail(loc != NULL);

    if (loc->zone[0] == '-')
        return;
        
    if (loc->zone[0] == ':')	/* Met Office Region Names */
    {
    	metoffice_start_open (info);
    	return;
    }
    if (loc->zone[0] == '@')    /* Australian BOM forecasts */
    {
    	bom_start_open (info);
    	return;
    }
    
#if 0
    if (weather_forecast == FORECAST_ZONE)
        url = g_strdup_printf("http://iwin.nws.noaa.gov/iwin/%s/zone.html",
			loc->zone);
    else
        url = g_strdup_printf("http://iwin.nws.noaa.gov/iwin/%s/state.html",
			loc->zone);
#endif
    
    /* The zone for Pittsburgh (for example) is given as PAZ021 in the locations
    ** file (the PA stands for the state pennsylvania). The url used wants the state
    ** as pa, and the zone as lower case paz021.
    */
    zone = g_ascii_strdown (loc->zone, -1);
    state = g_strndup (zone, 2);
    
    url = g_strdup_printf ("http://weather.noaa.gov/pub/data/forecasts/zone/%s/%s.txt",
        		   state, zone); 
    g_free (zone);   
    g_free (state);

    gnome_vfs_async_open(&info->iwin_handle, url, GNOME_VFS_OPEN_READ, 
    			 0, iwin_finish_open, info->applet);
    g_free(url);

}

static void wx_finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, 
			   gpointer buffer, GnomeVFSFileSize requested, 
			   GnomeVFSFileSize body_len, gpointer data)
{
    GWeatherApplet * gw_applet = (GWeatherApplet *)data;
    WeatherInfo *info;
    WeatherLocation *loc;
    GdkPixmap *pixmap = NULL;

    g_return_if_fail(gw_applet != NULL);
    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(handle == gw_applet->gweather_info->wx_handle);

    info = gw_applet->gweather_info;
	
    info->radar = NULL;
    loc = info->location;

    if (result == GNOME_VFS_OK && body_len != 0) {
        GError *error = NULL;
    	gdk_pixbuf_loader_write (info->radar_loader, buffer, body_len, &error);
        if (error) {
            g_print ("%s \n", error->message);
            g_error_free (error);
        }
        gnome_vfs_async_read(handle, buffer, DATA_SIZE - 1, wx_finish_read, gw_applet);
        return;
    }
    else if (result == GNOME_VFS_ERROR_EOF)
    {
        GdkPixbuf *pixbuf;
    	
        gdk_pixbuf_loader_close (info->radar_loader, NULL);
        pixbuf = gdk_pixbuf_loader_get_pixbuf (info->radar_loader);
	if (pixbuf != NULL) {
            if (info->radar)
                g_object_unref (info->radar);
	    gdk_pixbuf_render_pixmap_and_mask (pixbuf, &info->radar, &info->radar_mask, 127);
        }
	g_object_unref (G_OBJECT (info->radar_loader));
        
    }
    else
    {
        g_print("%s", gnome_vfs_result_to_string(result));
        g_warning(_("Failed to get METAR data.\n"));
        info->wx_handle = NULL;
        requests_done_check (info);
	if(info->radar_loader)  g_object_unref (G_OBJECT (info->radar_loader));
    }
    
    request_done(info->wx_handle, info);
    g_free (buffer);    
    return;
}

static void wx_finish_open (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
    GWeatherApplet *gw_applet = (GWeatherApplet *)data;
    WeatherInfo *info;
    WeatherLocation *loc;
    gchar *body;
    gint body_len;
    GdkPixmap *pixmap = NULL;

    g_return_if_fail(gw_applet != NULL);
    g_return_if_fail(gw_applet->gweather_info != NULL);
    g_return_if_fail(handle == gw_applet->gweather_info->wx_handle);

    info = gw_applet->gweather_info;
	
    body = g_malloc0(DATA_SIZE);

    info->radar_buffer = NULL;
    info->radar = NULL;
    loc = info->location;
    g_return_if_fail(loc != NULL);

    if (result != GNOME_VFS_OK) {
        g_warning("Failed to get radar map image.\n");
        info->wx_handle = NULL;
        requests_done_check (info);
        g_free (body);
    } else {
        gnome_vfs_async_read(handle, body, DATA_SIZE -1, wx_finish_read, gw_applet); 
    	
    }
     return;
}

/* Get radar map and into newly allocated pixmap */
static void wx_start_open (WeatherInfo *info)
{
    gchar *url;
    WeatherLocation *loc;
    
    g_return_if_fail(info != NULL);
    info->radar = NULL;
    info->radar_loader = gdk_pixbuf_loader_new ();
    loc = info->location;
    g_return_if_fail(loc != NULL);

    
    if (info->radar_url)
    	url = g_strdup (info->radar_url);
    else {
    	 if (loc->radar[0] == '-') return;
	 url = g_strdup_printf ("http://image.weather.com/web/radar/us_%s_closeradar_medium_usen.jpg", loc->radar);
    }
 
    gnome_vfs_async_open(&info->wx_handle, url, GNOME_VFS_OPEN_READ, 
    			 0, wx_finish_open, info->applet);
    				 
    g_free(url);

    return;
}

gboolean _weather_info_fill (GWeatherApplet *applet, WeatherInfo *info, WeatherLocation *location, WeatherInfoFunc cb)
{
    g_return_val_if_fail(((info == NULL) && (location != NULL)) || \
                         ((info != NULL) && (location == NULL)), FALSE);

    /* FIXME: i'm not sure this works as intended anymore */
    if (!info) {
    	info = g_new0(WeatherInfo, 1);
        info->metar_handle = NULL;
    	info->iwin_handle = NULL;
    	info->wx_handle = NULL;
    	info->met_handle = NULL;
        info->bom_handle = NULL;
    	info->requests_pending = FALSE;
    	info->metar_buffer = NULL;
        info->iwin_buffer = NULL;
		info->met_buffer = NULL;
		info->bom_buffer = NULL;
    	info->location = weather_location_clone(location);
    } else {
        location = info->location;
	if (info->forecast)
	    g_free (info->forecast);

	info->forecast = NULL;
	if (info->radar != NULL) {
	    gdk_pixmap_unref (info->radar);
	    info->radar = NULL;
	}
    }

    /* Update in progress */
    if (!requests_init(cb, info)) {
        return FALSE;
    }

    /* Defaults (just in case...) */
    /* Well, no just in case anymore.  We may actually fail to fetch some
     * fields. */
    info->update = 0;
    info->sky = -1;
    info->cond.significant = FALSE;
    info->cond.phenomenon = PHENOMENON_NONE;
    info->cond.qualifier = QUALIFIER_NONE;
    info->temp = -1000.0;
    info->dew = -1000.0;
    info->wind = -1;
    info->windspeed = -1;
    info->pressure = -1.0;
    info->visibility = -1.0;
    info->sunValid = FALSE;
    info->sunrise = 0;
    info->sunset = 0;
    info->forecast = NULL;
    info->radar = NULL;
    info->radar_url = NULL;
    if (applet->gweather_pref.use_custom_radar_url && applet->gweather_pref.radar) {
    	info->radar_url = g_strdup (applet->gweather_pref.radar); 
    }   
    info->metar_handle = NULL;
    info->iwin_handle = NULL;
    info->wx_handle = NULL;
    info->met_handle = NULL;
    info->bom_handle = NULL;
    info->requests_pending = TRUE;
    info->applet = applet;
    applet->gweather_info = info;

    metar_start_open(info);
    iwin_start_open(info);

    if (weather_radar)
        wx_start_open(info);

    return TRUE;
}

void weather_info_config_write (WeatherInfo *info)
{
    g_return_if_fail(info != NULL);

    gnome_config_set_bool("valid", info->valid);
    gnome_config_set_int("update", info->update);
    gnome_config_set_int("sky", (gint)info->sky);
    gnome_config_set_bool("cond_significant", info->cond.significant);
    gnome_config_set_int("cond_phenomenon", (gint)info->cond.phenomenon);
    gnome_config_set_int("cond_qualifier", (gint)info->cond.qualifier);
    gnome_config_set_float("temp", info->temp);
    gnome_config_set_float("dew", info->dew);
    gnome_config_set_int("wind", (gint)info->wind);
    gnome_config_set_int("windspeed", info->windspeed);
    gnome_config_set_float("pressure", info->pressure);
    gnome_config_set_float("visibility", info->visibility);
    if (info->forecast)
        gnome_config_set_string("forecast", info->forecast);
    else
        gnome_config_set_string("forecast", "");
    /* info->radar = NULL; */
}

WeatherInfo *weather_info_config_read (PanelApplet *applet)
{
    WeatherInfo *info = g_new(WeatherInfo, 1);
#if 0
    info->valid = gnome_config_get_bool("valid=FALSE");
    info->location = weather_location_config_read("location");
    info->update = (WeatherUpdate)gnome_config_get_int("update=0");
    info->sky = (WeatherSky)gnome_config_get_int("sky=0");
    info->cond.significant = gnome_config_get_bool("cond_significant=FALSE");
    info->cond.phenomenon = (WeatherConditionPhenomenon)gnome_config_get_int("cond_phenomenon=0");
    info->cond.qualifier = (WeatherConditionQualifier)gnome_config_get_int("cond_qualifier=0");
    info->temp = gnome_config_get_float("temp=0");
    info->dew = gnome_config_get_float("dew=0");
    info->wind = (WeatherWindDirection)gnome_config_get_int("wind=0");
    info->windspeed = gnome_config_get_int("windspeed=0");
    info->pressure = gnome_config_get_float("pressure=0");
    info->visibility = gnome_config_get_float("visibility=0");
    info->forecast = gnome_config_get_string("forecast=None");
    info->radar = NULL;
#endif
    info->valid = FALSE;
    info->location = weather_location_config_read(applet);
    info->update = (WeatherUpdate)0;
    info->sky = -1;
    info->cond.significant = FALSE;
    info->cond.phenomenon = (WeatherConditionPhenomenon)0;
    info->cond.qualifier = (WeatherConditionQualifier)0;
    info->temp = -1000.0;
    info->dew = -1000.0;
    info->wind = -1;
    info->windspeed = -1;
    info->pressure = -1.0;
    info->visibility = -1.0;
    info->sunValid = FALSE;
    info->sunrise = 0;
    info->sunset = 0;
    info->forecast = g_strdup ("None");
    info->radar = NULL;  /* FIX */
    info->requests_pending = FALSE;
    info->metar_buffer = NULL;
    info->iwin_buffer = NULL;

    info->met_buffer = NULL;
    info->bom_buffer = NULL;

    info->met_handle = NULL;
    info->iwin_handle = NULL;
    info->met_handle = NULL;
    info->bom_handle = NULL;

    return info;
}

WeatherInfo *weather_info_clone (const WeatherInfo *info)
{
    WeatherInfo *clone;
    
    g_return_val_if_fail(info != NULL, NULL);

    clone = g_new(WeatherInfo, 1);


    /* move everything */
    g_memmove(clone, info, sizeof(WeatherInfo));


    /* special moves */
    clone->location = weather_location_clone(info->location);
    /* This handles null correctly */
    clone->forecast = g_strdup(info->forecast);
    clone->radar_url = g_strdup (info->radar_url); 

    clone->radar = info->radar;
    if (clone->radar != NULL)
	    gdk_pixmap_ref (clone->radar);

    return clone;
}

void weather_info_free (WeatherInfo *info)
{
    if (!info)
        return;

    weather_location_free(info->location);
    info->location = NULL;

    g_free(info->forecast);
    info->forecast = NULL;

    if (info->radar != NULL) {
        gdk_pixmap_unref (info->radar);
        info->radar = NULL;
    }
	
	if (info->iwin_buffer)
	    g_free (info->iwin_buffer);

	if (info->metar_buffer)	
	    g_free (info->metar_buffer);

    if (info->met_buffer)
        g_free (info->met_buffer);

    if (info->bom_buffer)
        g_free (info->bom_buffer);

    if (info->metar_handle)
        gnome_vfs_async_cancel (info->metar_handle);

    if (info->iwin_handle)
        gnome_vfs_async_cancel (info->iwin_handle);

    if (info->wx_handle)
        gnome_vfs_async_cancel (info->wx_handle);

    if (info->met_handle)
        gnome_vfs_async_cancel (info->met_handle);

    if (info->bom_handle)
        gnome_vfs_async_cancel (info->bom_handle);
	
    g_free(info);
}

void weather_forecast_set (WeatherForecastType forecast)
{
    weather_forecast = forecast;
}

WeatherForecastType weather_forecast_get (void)
{
    return weather_forecast;
}

void weather_radar_set (gboolean enable)
{
    weather_radar = enable;
}

gboolean weather_radar_get (void)
{
    return weather_radar;
}


const gchar *weather_info_get_location (WeatherInfo *info)
{
    g_return_val_if_fail(info != NULL, NULL);
    g_return_val_if_fail(info->location != NULL, NULL);
    return info->location->trans_name;
}

const gchar *weather_info_get_update (WeatherInfo *info)
{
    static gchar buf[200];

    g_return_val_if_fail(info != NULL, NULL);

    if (!info->valid)
        return "-";

    if (info->update != 0) {
        struct tm tm;
        localtime_r(&info->update, &tm);
        if (strftime(buf, sizeof(buf), _("%a, %b %d / %H:%M"), &tm) <= 0)
		strcpy (buf, "???");
	buf[sizeof(buf)-1] = '\0';
    } else {
        strncpy(buf, _("Unknown observation time"), sizeof (buf));
	buf[sizeof(buf)-1] = '\0';
    }

    return buf;
}

const gchar *weather_info_get_sky (WeatherInfo *info)
{
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    if (info->sky < 0)
	return "Unknown";
    return weather_sky_string(info->sky);
}

const gchar *weather_info_get_conditions (WeatherInfo *info)
{
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    return weather_conditions_string(info->cond);
}

static const gchar *temperature_string (gfloat far, TempUnit to_unit, gboolean round)
{
	static gchar buf[100];

    switch (to_unit) {
        case TEMP_UNIT_FAHRENHEIT:
			if (!round) {
                if ( strcmp (_("%.1f F"), "%.1f F") != 0 ) {
                    /* TRANSLATOR: This is the temperature in degrees fahrenheit, use the degree */
                    /* symbol Unicode 00B0 if possible */
                    g_snprintf(buf, sizeof (buf), _("%.1f F"), far);
                } else {
                    g_snprintf(buf, sizeof (buf), "%.1f \302\260F", far);
                }
            } else {
                if ( strcmp (_("%dF"), "%dF") != 0 ) {
                    /* TRANSLATOR: This is the temperature in degrees fahrenheit, use the degree */
                    /* symbol Unicode 00B0 if possible */
                    g_snprintf(buf, sizeof (buf), _("%dF"), (int)floor(far + 0.5));
                } else {
                    g_snprintf(buf, sizeof (buf), "%d\302\260F", (int)floor(far + 0.5));
                }
            }
            break;
        case TEMP_UNIT_CENTIGRADE:
            if (!round) {
                if ( strcmp (_("%.1f C"), "%.1f C") != 0 ) {
                    /* TRANSLATOR: This is the temperature in degrees centigrade , use the degree */
                    /* symbol Unicode 00B0 if possible */
                    g_snprintf (buf, sizeof (buf), _("%.1f C"), TEMP_F_TO_C(far));
                } else { 
                    g_snprintf (buf, sizeof (buf), "%.1f \302\260C", TEMP_F_TO_C(far));
                }
            } else {
                if ( strcmp (_("%dC"), "%dC") != 0 ) {
                    /* TRANSLATOR: This is the temperature in degrees centigrade , use the degree */
                    /* symbol Unicode 00B0 if possible */
                    g_snprintf (buf, sizeof (buf), _("%dC"), (int)floor(TEMP_F_TO_C(far) + 0.5));
                } else { 
                    g_snprintf (buf, sizeof (buf), "%d\302\260C", (int)floor(TEMP_F_TO_C(far) + 0.5));
				}
            }
            break;
        case TEMP_UNIT_KELVIN:
            if (!round) {
                /* TRANSLATOR: This is the temperature in Kelvin */
                g_snprintf (buf, sizeof (buf), _("%.1f K"), TEMP_F_TO_K(far));
            } else {
                /* TRANSLATOR: This is the temperature in Kelvin */
                g_snprintf (buf, sizeof (buf), _("%dK"), (int)floor(TEMP_F_TO_K(far)));
            } 
            break;

        case TEMP_UNIT_INVALID:
        case TEMP_UNIT_DEFAULT:
        default:
            g_warning("Conversion to illegal temperature unit: %d", to_unit);
            return (_("Unknown"));
            break;
    }

    return buf;
}

const gchar *weather_info_get_temp (WeatherInfo *info)
{
    g_return_val_if_fail(info != NULL, NULL);

    if (!info->valid)
        return "-";
    if (info->temp < -500.0)
        return _("Unknown");
    
    return temperature_string (info->temp, info->applet->gweather_pref.temperature_unit, FALSE);
}

const gchar *weather_info_get_dew (WeatherInfo *info)
{
    g_return_val_if_fail(info != NULL, NULL);

    if (!info->valid)
        return "-";
    if (info->dew < -500.0)
        return _("Unknown");

    return temperature_string (info->dew, info->applet->gweather_pref.temperature_unit, FALSE);
}

const gchar *weather_info_get_humidity (WeatherInfo *info)
{
    static gchar buf[20];
    gdouble humidity;
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";

    humidity = calc_humidity(info->temp, info->dew);
    if (humidity < 0.0)
        return _("Unknown");

    /* TRANSLATOR: This is the humidity in percent */
    g_snprintf(buf, sizeof (buf), _("%.f%%"), humidity);
    return buf;
}

const gchar *weather_info_get_apparent (WeatherInfo *info)
{
    gdouble apparent;
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";

    apparent = calc_apparent(info);
    if (apparent < -500.0)
        return _("Unknown");
    
    return temperature_string (apparent, info->applet->gweather_pref.temperature_unit, FALSE);
}

static const gchar *windspeed_string (gfloat knots, SpeedUnit to_unit)
{
    static gchar buf[100];

    switch (to_unit) {
        case SPEED_UNIT_KNOTS:
            /* TRANSLATOR: This is the wind speed in knots */
            g_snprintf(buf, sizeof (buf), _("%0.1f knots"), knots);
            break;
        case SPEED_UNIT_MPH:
            /* TRANSLATOR: This is the wind speed in miles per hour */
            g_snprintf(buf, sizeof (buf), _("%.1f mph"), WINDSPEED_KNOTS_TO_MPH(knots));
            break;
        case SPEED_UNIT_KPH:
            /* TRANSLATOR: This is the wind speed in kilometers per hour */
            g_snprintf(buf, sizeof (buf), _("%.1f km/h"), WINDSPEED_KNOTS_TO_KPH(knots));
            break;
        case SPEED_UNIT_MS:
            /* TRANSLATOR: This is the wind speed in meters per second */
            g_snprintf(buf, sizeof (buf), _("%.1f m/s"), WINDSPEED_KNOTS_TO_MS(knots));
            break;

        case SPEED_UNIT_INVALID:
        case SPEED_UNIT_DEFAULT:
        default:
            g_warning("Conversion to illegal speed unit: %d", to_unit);
            return _("Unknown");
    }

    return buf;
}
const gchar *weather_info_get_wind (WeatherInfo *info)
{
    static gchar buf[200];
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    if (info->windspeed < 0.0 || info->wind < 0)
        return _("Unknown");
    if (info->windspeed == 0.00) {
        strncpy(buf, _("Calm"), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
    } else
        /* TRANSLATOR: This is 'wind direction' / 'wind speed' */
        g_snprintf(buf, sizeof(buf), _("%s / %s"),
		   weather_wind_direction_string(info->wind),
		   windspeed_string(info->windspeed, info->applet->gweather_pref.speed_unit));
    return buf;
}

const gchar *weather_info_get_pressure (WeatherInfo *info)
{
    static gchar buf[100];
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    if (info->pressure < 0.0)
        return _("Unknown");

    switch (info->applet->gweather_pref.pressure_unit) {
        case PRESSURE_UNIT_INCH_HG:
            /* TRANSLATOR: This is pressure in inches of mercury */
            g_snprintf (buf, sizeof (buf), _("%.2f inHg"), info->pressure);
            break;
        case PRESSURE_UNIT_MM_HG:
            /* TRANSLATOR: This is pressure in millimeters of mercury */
            g_snprintf (buf, sizeof (buf), _("%.1f mmHg"), PRESSURE_INCH_TO_MM(info->pressure));
            break;
        case PRESSURE_UNIT_KPA:
            /* TRANSLATOR: This is pressure in kiloPascals */
            g_snprintf (buf, sizeof (buf), _("%.2f kPa"), PRESSURE_INCH_TO_KPA(info->pressure));
            break;
        case PRESSURE_UNIT_HPA:
            /* TRANSLATOR: This is pressure in hectoPascals */
            g_snprintf (buf, sizeof (buf), _("%.2f hPa"), PRESSURE_INCH_TO_HPA(info->pressure));
            break;
        case PRESSURE_UNIT_MB:
            /* TRANSLATOR: This is pressure in millibars */
            g_snprintf (buf, sizeof (buf), _("%.2f mb"), PRESSURE_INCH_TO_MB(info->pressure));
            break;

        case PRESSURE_UNIT_INVALID:
        case PRESSURE_UNIT_DEFAULT:
        default:
            g_warning("Conversion to illegal pressure unit: %d", info->applet->gweather_pref.pressure_unit);
            return _("Unknown");
    }

    return buf;
}

const gchar *weather_info_get_visibility (WeatherInfo *info)
{
    static gchar buf[100];
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    if (info->visibility < 0.0)
        return _("Unknown");

    switch (info->applet->gweather_pref.distance_unit) {
        case DISTANCE_UNIT_MILES:
            /* TRANSLATOR: This is the visibility in miles */
            g_snprintf(buf, sizeof (buf), _("%.1f miles"), info->visibility);
            break;
        case DISTANCE_UNIT_KM:
            /* TRANSLATOR: This is the visibility in kilometers */
            g_snprintf(buf, sizeof (buf), _("%.1f km"), VISIBILITY_SM_TO_KM(info->visibility));
            break;
        case DISTANCE_UNIT_METERS:
            /* TRANSLATOR: This is the visibility in meters */
            g_snprintf(buf, sizeof (buf), _("%.0fm"), VISIBILITY_SM_TO_M(info->visibility));
            break;

        case DISTANCE_UNIT_INVALID:
        case DISTANCE_UNIT_DEFAULT:
        default:
            g_warning("Conversion to illegal visibility unit: %d", info->applet->gweather_pref.pressure_unit);
            return _("Unknown");
    }

    return buf;
}

const gchar *weather_info_get_sunrise (WeatherInfo *info)
{
    static gchar buf[200];
    struct tm tm;
    
    g_return_val_if_fail(info && info->location, NULL);
    
    if (!info->location->latlon_valid)
        return "-";
    if (!info->valid)
        return "-";
    if (!calc_sun (info))
        return "-";

    localtime_r(&info->sunrise, &tm);
    if (strftime(buf, sizeof(buf), _("%H:%M"), &tm) <= 0)
        return "-";
    return buf;
}

const gchar *weather_info_get_sunset (WeatherInfo *info)
{
    static gchar buf[200];
    struct tm tm;
    
    g_return_val_if_fail(info && info->location, NULL);
    
    if (!info->location->latlon_valid)
        return "-";
    if (!info->valid)
        return "-";
    if (!calc_sun (info))
        return "-";

    localtime_r(&info->sunset, &tm);
    if (strftime(buf, sizeof(buf), _("%H:%M"), &tm) <= 0)
        return "-";
    return buf;
}

const gchar *weather_info_get_forecast (WeatherInfo *info)
{
    g_return_val_if_fail(info != NULL, NULL);
    return info->forecast;
}

GdkPixmap *weather_info_get_radar (WeatherInfo *info)
{
    g_return_val_if_fail(info != NULL, NULL);
    return info->radar;
}

const gchar *weather_info_get_temp_summary (WeatherInfo *info)
{
    if (!info)
        return NULL;
    if (!info->valid || info->temp < -500.0)
        return "--";
          
    return temperature_string (info->temp, info->applet->gweather_pref.temperature_unit, TRUE);
    
}

gchar *weather_info_get_weather_summary (WeatherInfo *info)
{
    const gchar *buf;
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
      return g_strdup (_("Retrieval failed"));
    buf = weather_info_get_conditions(info);
    if (!strcmp(buf, "-"))
        buf = weather_info_get_sky(info);
    return g_strdup_printf ("%s: %s", weather_info_get_location (info), buf);
}


static GdkPixbuf **weather_pixbufs_mini = NULL;
static GdkPixbuf **weather_pixbufs = NULL;

enum
{
	PIX_UNKNOWN,
	PIX_SUN,
	PIX_SUNCLOUD,
	PIX_CLOUD,
	PIX_RAIN,
	PIX_TSTORM,
	PIX_SNOW,
	PIX_FOG,
	PIX_MOON,
	PIX_MOONCLOUD,

	NUM_PIX
};

#if 0
static void
xpm_to_pixmap (char **xpm_data, GdkPixmap **pixmap, GdkBitmap **mask)
{
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **)xpm_data);

	*pixmap = NULL;
	*mask = NULL;

	if (pixbuf != NULL) {
		gdk_pixbuf_render_pixmap_and_mask (pixbuf, pixmap, mask, 127);

		gdk_pixbuf_unref (pixbuf);
	}
}
#endif

static void init_pixbufs (void)
{
    static gboolean initialized = FALSE;
    GtkIconTheme *icon_theme;
    static char *icons[NUM_PIX] = {
      "stock_unknown",
      "stock_weather-sunny",
      "stock_weather-few-clouds",
      "stock_weather-cloudy",
      "stock_weather-showers",
      "stock_weather-storm",
      "stock_weahter-snow",
      "stock_weather-fog",
      "stock_weather-night-clear",
      "stock_weather-night-few-clouds"
    };
    int idx;
    
    if (initialized)
       return;
    initialized = TRUE;

    icon_theme = gtk_icon_theme_get_default ();

    weather_pixbufs_mini = g_new(GdkPixbuf *, NUM_PIX);
    weather_pixbufs = g_new(GdkPixbuf *, NUM_PIX);

    for (idx = 0; idx < NUM_PIX; idx++) {
        weather_pixbufs_mini[idx] = gtk_icon_theme_load_icon (icon_theme, icons[idx], 16, 0, NULL);
        weather_pixbufs[idx] = gtk_icon_theme_load_icon (icon_theme, icons[idx], 48, 0, NULL);
    }
}

void _weather_info_get_pixbuf (WeatherInfo *info, gboolean mini, GdkPixbuf **pixbuf)
{
    GdkPixbuf **pixbufs;
    gint idx = -1;

    g_return_if_fail(pixbuf != NULL);

    init_pixbufs();
    pixbufs = mini ? weather_pixbufs_mini : weather_pixbufs;

    if (!info || !info->valid) {
        idx = PIX_UNKNOWN;
    } else {
        WeatherConditions cond = info->cond;
        WeatherSky sky = info->sky;

        if (cond.significant && (cond.phenomenon != PHENOMENON_NONE)) {
            switch (cond.qualifier) {
            case QUALIFIER_NONE:
                break;
	    case QUALIFIER_VICINITY:
		break;
            case QUALIFIER_THUNDERSTORM:
                idx = PIX_TSTORM;
                break;
            case QUALIFIER_SHALLOW:
            case QUALIFIER_PATCHES:
            case QUALIFIER_PARTIAL:
            case QUALIFIER_BLOWING:
            case QUALIFIER_SHOWERS:
            case QUALIFIER_DRIFTING:
            case QUALIFIER_FREEZING:
                break;
            case QUALIFIER_LIGHT:
            case QUALIFIER_MODERATE:
            case QUALIFIER_HEAVY:
                break;
            default:
                g_assert_not_reached();
            }

            if (idx < 0)
                switch (cond.phenomenon) {
                case PHENOMENON_DRIZZLE:
                case PHENOMENON_RAIN:
                case PHENOMENON_UNKNOWN_PRECIPITATION:
                    if ((cond.qualifier == QUALIFIER_SHALLOW) ||
                        (cond.qualifier == QUALIFIER_PATCHES) ||
                        (cond.qualifier == QUALIFIER_PARTIAL))
                        idx = PIX_RAIN;
                    else
                        idx = PIX_RAIN;
                    break;
                case PHENOMENON_SNOW:
		case PHENOMENON_SNOW_GRAINS:
                case PHENOMENON_ICE_PELLETS:
		case PHENOMENON_ICE_CRYSTALS:
                    idx = PIX_SNOW;
                    break;
                case PHENOMENON_HAIL:
                case PHENOMENON_SMALL_HAIL:
                    idx = PIX_RAIN;
                    break;
                case PHENOMENON_TORNADO:
                case PHENOMENON_SQUALL:
                    idx = PIX_TSTORM;
                    break;
                case PHENOMENON_MIST:
                case PHENOMENON_FOG:
                case PHENOMENON_SMOKE:
                case PHENOMENON_VOLCANIC_ASH:
                case PHENOMENON_SAND:
                case PHENOMENON_HAZE:
                case PHENOMENON_SPRAY:
                case PHENOMENON_DUST:
                case PHENOMENON_SANDSTORM:
                case PHENOMENON_DUSTSTORM:
                case PHENOMENON_FUNNEL_CLOUD:
                case PHENOMENON_DUST_WHIRLS:
                    idx = PIX_FOG;
                    break;
                default:
	            idx = PIX_UNKNOWN;
                }
        } else {
	    time_t current_time;
	    current_time = time (NULL);
	    gboolean useSun = (! info->sunValid )
	                      || (current_time >= info->sunrise &&
				  current_time < info->sunset);
            switch (sky) {
            case SKY_CLEAR:
	        idx = useSun ? PIX_SUN : PIX_MOON;
                break;
            case SKY_BROKEN:
            case SKY_SCATTERED:
            case SKY_FEW:
	        idx = useSun ? PIX_SUNCLOUD : PIX_MOONCLOUD;
                break;
            case SKY_OVERCAST:
                idx = PIX_CLOUD;
                break;
            default:
	        idx = PIX_UNKNOWN;
            }
        }
    }

    *pixbuf = pixbufs[idx];
}
