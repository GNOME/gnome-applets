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

static WeatherUnits weather_units = UNITS_IMPERIAL;
static WeatherForecastType weather_forecast = FORECAST_STATE;
static gboolean weather_radar = FALSE;

static gchar *weather_proxy_url = NULL;
static gchar *weather_proxy_user = NULL;
static gchar *weather_proxy_passwd = NULL;

#define DATA_SIZE 5000

/* Unit conversions and names */

#define TEMP_F_TO_C(f)  (((f) - 32.0) * 0.555556)
#define TEMP_C_TO_F(c)  (((c) * 1.8) + 32.0)
#define TEMP_UNIT_STR(units)  (((units) == UNITS_IMPERIAL) ? "\260F" : "\260C")

#define WINDSPEED_KNOTS_TO_KPH(knots)  ((knots) * 1.851965)
#define WINDSPEED_KPH_TO_KNOTS(kph)    ((kph) * 0.539967)
#define WINDSPEED_UNIT_STR(units) (((units) == UNITS_IMPERIAL) ? _("knots") : _("kph"))

#define PRESSURE_INCH_TO_MM(inch)   ((inch) * 25.4)
#define PRESSURE_MM_TO_INCH(mm)     ((mm) * 0.03937)
#define PRESSURE_MBAR_TO_INCH(mbar) ((mbar) * 0.02963742)
#define PRESSURE_UNIT_STR(units) (((units) == UNITS_IMPERIAL) ? _("inHg") : _("mmHg"))

#define VISIBILITY_SM_TO_KM(sm)  ((sm) * 1.609344)
#define VISIBILITY_KM_TO_SM(km)  ((km) * 0.621371)
#define VISIBILITY_UNIT_STR(units) (((units) == UNITS_IMPERIAL) ? _("miles") : _("kilometers"))


WeatherLocation *weather_location_new (const gchar *name, const gchar *code, const gchar *zone, const gchar *radar)
{
    WeatherLocation *location;

    location = g_new(WeatherLocation, 1);
    strncpy(location->name, name, WEATHER_LOCATION_NAME_MAX_LEN);
    location->name[WEATHER_LOCATION_NAME_MAX_LEN] = '\0';
    strncpy(location->code, code, WEATHER_LOCATION_CODE_LEN);
    location->code[WEATHER_LOCATION_CODE_LEN] = '\0';
    strncpy(location->zone, zone, WEATHER_LOCATION_ZONE_LEN);
    location->zone[WEATHER_LOCATION_ZONE_LEN] = '\0';
    strncpy(location->radar, radar, WEATHER_LOCATION_RADAR_LEN);
    location->radar[WEATHER_LOCATION_RADAR_LEN] = '\0';

    return location;
}

void weather_location_config_write (gchar *prefix, WeatherLocation *location)
{
    gchar **locdata;

    g_return_if_fail(prefix != NULL);

    locdata = g_new(gchar *, 4);
    locdata[0] = location->name;
    locdata[1] = location->code;
    locdata[2] = location->zone;
    locdata[3] = location->radar;
    gnome_config_set_vector(prefix, 4, (const char **)locdata);
    g_free(locdata);
}

WeatherLocation *weather_location_config_read (gchar *prefix)
{
    WeatherLocation *location;
/*
    gchar **locdata;
    gint nlocdata;
    gchar *prefix_with_default;
*/
/*    g_return_val_if_fail(prefix != NULL, NULL); */
/* FIXME: killing gnome-config code */
/*    prefix_with_default = g_strconcat(prefix, "=Pittsburgh KPIT PAZ021 pit", NULL); */

/*    gnome_config_get_vector(prefix_with_default, &nlocdata, &locdata); 

    if (nlocdata != 4) {
	    g_warning (_("Location vector needs to be 4 elements long"));
	    g_free(prefix_with_default);
	    g_strfreev(locdata);
	    return NULL;
    }
    location = weather_location_new(locdata[0], locdata[1], locdata[2], locdata[3]); */

	location = weather_location_new("Pittsburgh", "KPIT", "PAZ021", "pit");
/*    g_strfreev(locdata);

    g_free(prefix_with_default);
*/
    return location;
}

WeatherLocation *weather_location_clone (const WeatherLocation *location)
{
    WeatherLocation *clone;

    g_return_val_if_fail(location != NULL, NULL);
    clone = g_new(WeatherLocation, 1);
    g_memmove(clone, location, sizeof(WeatherLocation));
    return clone;
}

void weather_location_free (WeatherLocation *location)
{
    g_free(location);
}

gboolean weather_location_equal (const WeatherLocation *location1, const WeatherLocation *location2)
{
    return (strcmp(location1->code, location2->code) == 0);
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
	if (wind < 0 ||
	    wind >= (sizeof (wind_direction_str) / sizeof (char *)))
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
 *
 * I tried to come up with logical names for most phenomena, but I'm no
 * meteorologist, so there will undoubtedly be some stupid mistakes.
 * However, combinations that did not seem plausible (eg. I cannot imagine
 * what a "light tornado" may be like ;-) were filled in with "??".  If this
 * ever comes up in the weather conditions field, let me know...
 */

/*
 * Note, magic numbers, when you change the size here, make sure to change
 * the below function so that new values are recognized
 */
static const gchar *conditions_str[24][13] = {
/*                   NONE                         VICINITY                             LIGHT                      MODERATE                      HEAVY                      SHALLOW                      PATCHES                         PARTIAL                      THUNDERSTORM                    BLOWING                      SHOWERS                         DRIFTING                      FREEZING                      */
/*               *******************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************/
/* NONE          */ {"",                          "",                                  "",                        "",                           "",                        "",                          "",                             "",                          "",                             "",                          "",                             "",                           "",                          },
/* DRIZZLE       */ {N_("Drizzle"),               N_("Drizzle in the vicinity"),       N_("Light drizzle"),       N_("Moderate drizzle"),       N_("Heavy drizzle"),       N_("Shallow drizzle"),       N_("Patches of drizzle"),       N_("Partial drizzle"),       N_("Thunderstorm"),             N_("Windy drizzle"),         N_("Showers"),                  N_("Drifting drizzle"),       N_("Freezing drizzle")       },
/* RAIN          */ {N_("Rain"),                  N_("Rain in the vicinity") ,         N_("Light rain"),          N_("Moderate rain"),          N_("Heavy rain"),          N_("Shallow rain"),          N_("Patches of rain"),          N_("Partial rainfall"),      N_("Thunderstorm"),             N_("Blowing rainfall"),      N_("Rain showers"),             N_("Drifting rain"),          N_("Freezing rain")          },
/* SNOW          */ {N_("Snow"),                  N_("Snow in the vicinity") ,         N_("Light snow"),          N_("Moderate snow"),          N_("Heavy snow"),          N_("Shallow snow"),          N_("Patches of snow"),          N_("Partial snowfall"),      N_("Snowstorm"),                N_("Blowing snowfall"),      N_("Snow showers"),             N_("Drifting snow"),          N_("Freezing snow")          },
/* SNOW_GRAINS   */ {N_("Snow grains"),           N_("Snow grains in the vicinity") ,  N_("Light snow grains"),   N_("Moderate snow grains"),   N_("Heavy snow grains"),   N_("Shallow snow grains"),   N_("Patches of snow grains"),   N_("Partial snow grains"),   N_("Snowstorm"),                N_("Blowing snow grains"),   N_("Snow grain showers"),       N_("Drifting snow grains"),   N_("Freezing snow grains")   },
/* ICE_CRYSTALS  */ {N_("Ice crystals"),          N_("Ice crystals in the vicinity") , N_("Few ice crystals"),    N_("Moderate ice crystals"),  N_("Heavy ice crystals"),  "??",                        N_("Patches of ice crystals"),  N_("Partial ice crystals"),  N_("Ice crystal storm"),        N_("Blowing ice crystals"),  N_("Showers of ice crystals"),  N_("Drifting ice crystals"),  N_("Freezing ice crystals")  },
/* ICE_PELLETS   */ {N_("Ice pellets"),           N_("Ice pellets in the vicinity") ,  N_("Few ice pellets"),     N_("Moderate ice pellets"),   N_("Heavy ice pellets"),   N_("Shallow ice pellets"),   N_("Patches of ice pellets"),   N_("Partial ice pellets"),   N_("Ice pellet storm"),         N_("Blowing ice pellets"),   N_("Showers of ice pellets"),   N_("Drifting ice pellets"),   N_("Freezing ice pellets")   },
/* HAIL          */ {N_("Hail"),                  N_("Hail in the vicinity") ,         N_("Light hail"),          N_("Moderate hail"),          N_("Heavy hail"),          N_("Shallow hail"),          N_("Patches of hail"),          N_("Partial hail"),          N_("Hailstorm"),                N_("Blowing hail"),          N_("Hail showers"),             N_("Drifting hail"),          N_("Freezing hail")          },
/* SMALL_HAIL    */ {N_("Small hail"),            N_("Small hail in the vicinity") ,   N_("Light hail"),          N_("Moderate small hail"),    N_("Heavy small hail"),    N_("Shallow small hail"),    N_("Patches of small hail"),    N_("Partial small hail"),    N_("Small hailstorm"),          N_("Blowing small hail"),    N_("Showers of small hail"),    N_("Drifting small hail"),    N_("Freezing small hail")    },
/* PRECIPITATION */ {N_("Unknown precipitation"), N_("Precipitation in the vicinity"), N_("Light precipitation"), N_("Moderate precipitation"), N_("Heavy precipitation"), N_("Shallow precipitation"), N_("Patches of precipitation"), N_("Partial precipitation"), N_("Unknown thunderstorm"),     N_("Blowing precipitation"), N_("Showers, type unknown"),    N_("Drifting precipitation"), N_("Freezing precipitation") },
/* MIST          */ {N_("Mist"),                  N_("Mist in the vicinity") ,         N_("Light mist"),          N_("Moderate mist"),          N_("Thick mist"),          N_("Shallow mist"),          N_("Patches of mist"),          N_("Partial mist"),          "??",                           N_("Mist with wind"),        "??",                           N_("Drifting mist"),          N_("Freezing mist")          },
/* FOG           */ {N_("Fog"),                   N_("Fog in the vicinity") ,          N_("Light fog"),           N_("Moderate fog"),           N_("Thick fog"),           N_("Shallow fog"),           N_("Patches of fog"),           N_("Partial fog"),           "??",                           N_("Fog with wind"),         "??",                           N_("Drifting fog"),           N_("Freezing fog")           },
/* SMOKE         */ {N_("Smoke"),                 N_("Smoke in the vicinity") ,        N_("Thin smoke"),          N_("Moderate smoke"),         N_("Thick smoke"),         N_("Shallow smoke"),         N_("Patches of smoke"),         N_("Partial smoke"),         N_("Smoke w/ thunders"),        N_("Smoke with wind"),       "??",                           N_("Drifting smoke"),         "??"                         },
/* VOLCANIC_ASH  */ {N_("Volcanic ash"),          N_("Volcanic ash in the vicinity") , "??",                      N_("Moderate volcanic ash"),  N_("Thick volcanic ash"),  N_("Shallow volcanic ash"),  N_("Patches of volcanic ash"),  N_("Partial volcanic ash"),  N_("Volcanic ash w/ thunders"), N_("Blowing volcanic ash"),  N_("Showers of volcanic ash "), N_("Drifting volcanic ash"),  N_("Freezing volcanic ash")  },
/* SAND          */ {N_("Sand"),                  N_("Sand in the vicinity") ,         N_("Light sand"),          N_("Moderate sand"),          N_("Heavy sand"),          "??",                        N_("Patches of sand"),          N_("Partial sand"),          "??",                           N_("Blowing sand"),          "",                             N_("Drifting sand"),          "??"                         },
/* HAZE          */ {N_("Haze"),                  N_("Haze in the vicinity") ,         N_("Light haze"),          N_("Moderate haze"),          N_("Thick haze"),          N_("Shallow haze"),          N_("Patches of haze"),          N_("Partial haze"),          "??",                           N_("Haze with wind"),        "??",                           N_("Drifting haze"),          N_("Freezing haze")          },
/* SPRAY         */ {N_("Sprays"),                N_("Sprays in the vicinity") ,       N_("Light sprays"),        N_("Moderate sprays"),        N_("Heavy sprays"),        N_("Shallow sprays"),        N_("Patches of sprays"),        N_("Partial sprays"),        "??",                           N_("Blowing sprays"),        "??",                           N_("Drifting sprays"),        N_("Freezing sprays")        },
/* DUST          */ {N_("Dust"),                  N_("Dust in the vicinity") ,         N_("Light dust"),          N_("Moderate dust"),          N_("Heavy dust"),          "??",                        N_("Patches of dust"),          N_("Partial dust"),          "??",                           N_("Blowing dust"),          "??",                           N_("Drifting dust"),          "??"                         },
/* SQUALL        */ {N_("Squall"),                N_("Squall in the vicinity") ,       N_("Light squall"),        N_("Moderate squall"),        N_("Heavy squall"),        "??",                        "??",                           N_("Partial squall"),        N_("Thunderous squall"),        N_("Blowing squall"),        "??",                           N_("Drifting squall"),        N_("Freezing squall")        },
/* SANDSTORM     */ {N_("Sandstorm"),             N_("Sandstorm in the vicinity") ,    N_("Light standstorm"),    N_("Moderate sandstorm"),     N_("Heavy sandstorm"),     N_("Shallow sandstorm"),     "??",                           N_("Partial sandstorm"),     N_("Thunderous sandstorm"),     N_("Blowing sandstorm"),     "??",                           N_("Drifting sandstorm"),     N_("Freezing sandstorm")     },
/* DUSTSTORM     */ {N_("Duststorm"),             N_("Duststorm in the vicinity") ,    N_("Light duststorm"),     N_("Moderate duststorm"),     N_("Heavy duststorm"),     N_("Shallow duststorm"),     "??",                           N_("Partial duststorm"),     N_("Thunderous duststorm"),     N_("Blowing duststorm"),     "??",                           N_("Drifting duststorm"),     N_("Freezing duststorm")     },
/* FUNNEL_CLOUD  */ {N_("Funnel cloud"),          N_("Funnel cloud in the vicinity") , N_("Light funnel cloud"),  N_("Moderate funnel cloud"),  N_("Thick funnel cloud"),  N_("Shallow funnel cloud"),  N_("Patches of funnel clouds"), N_("Partial funnel clouds"), "??",                           N_("Funnel cloud w/ wind"),  "??",                           N_("Drifting funnel cloud"),  "??"                         },
/* TORNADO       */ {N_("Tornado"),               N_("Tornado in the vicinity") ,      "??",                      N_("Moderate tornado"),       N_("Raging tornado"),      "??",                        "??",                           N_("Partial tornado"),       N_("Thunderous tornado"),       N_("Tornado"),               "??",                           N_("Drifting tornado"),       N_("Freezing tornado")       },
/* DUST_WHIRLS   */ {N_("Dust whirls"),           N_("Dust whirls in the vicinity") ,  N_("Light dust whirls"),   N_("Moderate dust whirls"),   N_("Heavy dust whirls"),   N_("Shallow dust whirls"),   N_("Patches of dust whirls"),   N_("Partial dust whirls"),   "??",                           N_("Blowing dust whirls"),   "??",                           N_("Drifting dust whirls"),   "??"                         }
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

static WeatherInfoFunc request_cb = NULL;
static WeatherInfo *request_info = NULL;
/*
static ghttp_request *metar_request = NULL,
                     *iwin_request = NULL,
                     *wx_request = NULL,
                     *met_request = NULL;
*/
static GnomeVFSAsyncHandle *metar_handle = NULL,
										*iwin_handle = NULL,
										*wx_handle = NULL,
										*met_handle = NULL;
										
static gboolean requests_pending = FALSE;

static __inline gboolean requests_init (WeatherInfoFunc cb, WeatherInfo *info)
{
    if (requests_pending)
        return FALSE;

    g_assert(!metar_handle && !iwin_handle && !wx_handle && !met_handle);
    g_assert(!request_info && !request_cb);

    requests_pending = TRUE;
    request_cb = cb;
    request_info = info;
	
    return TRUE;
}

static inline void request_done (GnomeVFSAsyncHandle *handle)
{
    g_return_if_fail(handle != NULL);

    gnome_vfs_async_close(handle, close_cb, NULL);
    
    return;
}

static inline void requests_done_check (void)
{
    g_return_if_fail(requests_pending);

    if (!metar_handle && !iwin_handle && !wx_handle && !met_handle) {
        requests_pending = FALSE;
        /* Next two lines temporarily here */
        if (weather_units == UNITS_METRIC)
            weather_info_to_metric(request_info);
        (*request_cb)(request_info);
        request_cb = NULL;
        request_info = NULL;
    }
}

void
close_cb (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
	if (result != GNOME_VFS_OK)
		g_warning("Error closing GnomeVFSAsyncHandle.\n");
	
	if (handle == metar_handle)
		metar_handle = NULL;
	if (handle == iwin_handle)
		iwin_handle = NULL;
	if (handle == wx_handle)
		wx_handle = NULL;
	if (handle == met_handle)
		met_handle = NULL;
	
	requests_done_check();
		
	return;
}

#define TIME_RE_STR  "^([0-9]{6})Z$"
#define WIND_RE_STR  "^(([0-9]{3})|VRB)([0-9]?[0-9]{2})(G[0-9]?[0-9]{2})?KT$"
#define VIS_RE_STR   "^(([0-9]?[0-9])|(M?1/[0-9]?[0-9]))SM$"
#define CLOUD_RE_STR "^(CLR|BKN|SCT|FEW|OVC)([0-9]{3})?$"
#define TEMP_RE_STR  "^(M?[0-9][0-9])/(M?[0-9][0-9])$"
#define PRES_RE_STR  "^(A|Q)([0-9]{4})$"
#define COND_RE_STR  "^(-|\\+)?(VC|MI|BC|PR|TS|BL|SH|DR|FZ)?(DZ|RA|SN|SG|IC|PE|GR|GS|UP|BR|FG|FU|VA|SA|HZ|PY|DU|SQ|SS|DS|PO|\\+?FC)$"

#define TIME_RE   0
#define WIND_RE   1
#define VIS_RE    2
#define CLOUD_RE  3
#define TEMP_RE   4
#define PRES_RE   5
#define COND_RE   6

#define RE_NUM   7

static regex_t metar_re[RE_NUM];

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
}

static inline gint days_in_month (gint month, gint year)
{
    if (month == 1)
        return ((year % 4) == 0) ? 29 : 28;
    else if (((month <= 6) && (month % 2 == 0)) || ((month >=7) && (month % 2 != 0)))
        return 31;
    else
        return 30;
}

/* FIX - there *must* be a simpler, less stupid way to do this!... */
time_t make_time (gint date, gint hour, gint min)
{
    struct tm *tm;
    struct tm tms;
    time_t now;
    gint loc_mday, loc_hour, gm_mday, gm_hour;
    gint hour_diff;  /* local time = UTC - hour_diff */
    gint is_dst;

    now = time(NULL);

    tm = gmtime(&now);
    gm_mday = tm->tm_mday;
    gm_hour = tm->tm_hour;
    memcpy(&tms, tm, sizeof(struct tm));

    tm = localtime(&now);
    loc_mday = tm->tm_mday;
    loc_hour = tm->tm_hour;
    is_dst = tm->tm_isdst;

    /* Estimate timezone */
    if (gm_mday == loc_mday)
        hour_diff = gm_hour - loc_hour;
    else
        if ((gm_mday == loc_mday + 1) || ((gm_mday == 1) && (loc_mday >= 27)))
            hour_diff = gm_hour + (24 - loc_hour);
        else
            hour_diff = -((24 - gm_hour) + loc_hour);

    /* Make time */
    tms.tm_min  = min;
    tms.tm_sec  = 0;
    tms.tm_hour = hour - hour_diff;
    tms.tm_mday = date;
    tms.tm_isdst = is_dst;
    if (tms.tm_hour < 0) {
        tms.tm_hour += 24;
        --tms.tm_mday;
        if (tms.tm_mday < 1) {
            --tms.tm_mon;
            if (tms.tm_mon < 0) {
                tms.tm_mon = 11;
                --tms.tm_year;
            }
            tms.tm_mday = days_in_month(tms.tm_mon, tms.tm_year + 1900);
        }
    } else if (tms.tm_hour > 23) {
        tms.tm_hour -= 24;
        ++tms.tm_mday;
        if (tms.tm_mday > days_in_month(tms.tm_mon, tms.tm_year + 1900)) {
            ++tms.tm_mon;
            if (tms.tm_mon > 11) {
                tms.tm_mon = 0;
                ++tms.tm_year;
            }
            tms.tm_mday = 1;
        }
    }

    return mktime(&tms);
}

static gboolean metar_tok_time (gchar *tokp, WeatherInfo *info)
{
    gchar sday[3], shr[3], smin[3];
    gint day, hr, min;

    if (regexec(&metar_re[TIME_RE], tokp, 0, NULL, 0) == REG_NOMATCH)
        return FALSE;

    strncpy(sday, tokp, 2);
    sday[2] = 0;
    day = atoi(sday);

    strncpy(shr, tokp+2, 2);
    shr[2] = 0;
    hr = atoi(shr);

    strncpy(smin, tokp+4, 2);
    smin[2] = 0;
    min = atoi(smin);

    info->update = make_time(day, hr, min);

    return TRUE;
}

#define CONST_DIGITS "0123456789"

static gboolean metar_tok_wind (gchar *tokp, WeatherInfo *info)
{
    gchar sdir[4], sspd[4], sgust[4];
    gint dir, spd, gust = -1;
    gchar *gustp;

    if (regexec(&metar_re[WIND_RE], tokp, 0, NULL, 0) == REG_NOMATCH)
        return FALSE;

    strncpy(sdir, tokp, 3);
    sdir[3] = 0;
    dir = (!strcmp(sdir, "VRB")) ? -1 : atoi(sdir);

    memset(sspd, 0, sizeof(sspd));
    strncpy(sspd, tokp+3, strspn(tokp+3, CONST_DIGITS));
    spd = atoi(sspd);

    gustp = strchr(tokp, 'G');
    if (gustp) {
        memset(sgust, 0, sizeof(sgust));
        strncpy(sgust, gustp+1, strspn(gustp+1, CONST_DIGITS));
        gust = atoi(sgust);
    }

    if ((349 <= dir) && (dir <= 11))
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
    else if ((247 <= dir) && (dir <= 258))
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

    return TRUE;
}

static gboolean metar_tok_vis (gchar *tokp, WeatherInfo *info)
{
    gchar *pfrac, *pend;
    gchar sval[4];
    gint val;

    if (regexec(&metar_re[VIS_RE], tokp, 0, NULL, 0) == REG_NOMATCH)
        return FALSE;

    pfrac = strchr(tokp, '/');
    pend = strstr(tokp, "SM");
    memset(sval, 0, sizeof(sval));

    if (pfrac) {
        strncpy(sval, pfrac + 1, pend - pfrac - 1);
        val = atoi(sval);
        info->visibility = (*tokp == 'M') ? 0.001 : (1.0 / ((WeatherVisibility)val));
    } else {
        strncpy(sval, tokp, pend - tokp);
        val = atoi(sval);
        info->visibility = (WeatherVisibility)val;
    }

    return TRUE;
}

static gboolean metar_tok_cloud (gchar *tokp, WeatherInfo *info)
{
    gchar stype[4], salt[4];
    gint alt = -1;

    if (regexec(&metar_re[CLOUD_RE], tokp, 0, NULL, 0) == REG_NOMATCH)
        return FALSE;

    strncpy(stype, tokp, 3);
    stype[3] = 0;
    if (strlen(tokp) == 6) {
        strncpy(salt, tokp+3, 3);
        salt[3] = 0;
        alt = atoi(salt);  /* Altitude - currently unused */
    }

    if (!strcmp(stype, "CLR")) {
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

    return TRUE;
}

static gboolean metar_tok_pres (gchar *tokp, WeatherInfo *info)
{
    if (regexec(&metar_re[PRES_RE], tokp, 0, NULL, 0) == REG_NOMATCH)
        return FALSE;

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

    return TRUE;
}

/* Relative humidity computation - thanks to <Olof.Oberg@modopaper.modogroup.com> */


static inline gint calc_humidity(gdouble temp, gdouble dewp)
{
    gdouble esat, esurf;

    temp = TEMP_F_TO_C(temp);
    dewp = TEMP_F_TO_C(dewp);

    esat = 6.11 * pow(10.0, (7.5 * temp) / (237.7 + temp));
    esurf = 6.11 * pow(10.0, (7.5 * dewp) / (237.7 + dewp));

    return (gint)((esurf/esat) * 100.0);
}

static gboolean metar_tok_temp (gchar *tokp, WeatherInfo *info)
{
    gchar *ptemp, *pdew, *psep;

    if (regexec(&metar_re[TEMP_RE], tokp, 0, NULL, 0) == REG_NOMATCH)
        return FALSE;

    psep = strchr(tokp, '/');
    *psep = 0;
    ptemp = tokp;
    pdew = psep + 1;

    info->temp = (*ptemp == 'M') ? TEMP_C_TO_F(-atoi(ptemp+1)) :
                                   TEMP_C_TO_F(atoi(ptemp));
    info->dew = (*pdew == 'M') ? TEMP_C_TO_F(-atoi(pdew+1)) :
                                 TEMP_C_TO_F(atoi(pdew));
    info->humidity = calc_humidity(info->temp, info->dew);
    return TRUE;
}

static gboolean metar_tok_cond (gchar *tokp, WeatherInfo *info)
{
    gchar squal[3], sphen[4];
    gchar *pphen;

    if (regexec(&metar_re[COND_RE], tokp, 0, NULL, 0) == REG_NOMATCH)
        return FALSE;

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
        g_return_val_if_fail(FALSE, FALSE);
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
        g_return_val_if_fail(FALSE, FALSE);
    }

    if ((info->cond.qualifier != QUALIFIER_NONE) || (info->cond.phenomenon != PHENOMENON_NONE))
        info->cond.significant = TRUE;

    return TRUE;
}

static void metar_parse_token (gchar *tokp, gboolean in_remark, WeatherInfo *info)
{
    if (!in_remark) {
        if (metar_tok_time(tokp, info))
            return;
        else if (metar_tok_wind(tokp, info))
            return;
        else if (metar_tok_vis(tokp, info))
            return;
        else if (metar_tok_cloud(tokp, info))
            return;
        else if (metar_tok_temp(tokp, info))
            return;
        else if (metar_tok_pres(tokp, info))
            return;
        else if (metar_tok_cond(tokp, info))
            return;
    }
}

static gboolean metar_parse (gchar *metar, WeatherInfo *info)
{
    gchar **toks;
    gint ntoks;
    gint i;
    gboolean in_remark = FALSE;

    g_return_val_if_fail(info != NULL, FALSE);
    g_return_val_if_fail(metar != NULL, FALSE);

    metar_init_re();

    toks = g_strsplit(metar, " ", 0);

    for (ntoks = 0;  toks[ntoks];  ntoks++)
        if (!strcmp(toks[ntoks], "RMK"))
            in_remark = TRUE;

    for (i = ntoks-1; i >= 0;  i--)
        if (strlen(toks[i]) > 0) {
            if (!strcmp(toks[i], "RMK"))
                in_remark = FALSE;
            else
                metar_parse_token(toks[i], in_remark, info);
        }

    g_strfreev(toks);

    return TRUE;
}

static void metar_finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer buffer,
										GnomeVFSFileSize requested, GnomeVFSFileSize body_len, gpointer data)
{
	WeatherInfo *info = (WeatherInfo *)data;
    WeatherLocation *loc;
    gchar *metar, *eoln, *body, *temp;
    gboolean success = FALSE;
    gchar searchkey[WEATHER_LOCATION_CODE_LEN + 2];

	info->forecast = NULL;
    loc = info->location;
	body = (gchar *)buffer;
	body[body_len] = '\0';

	if (info->read_buffer == NULL)
		info->read_buffer = g_strdup(body);
	else
	{
		temp = g_strdup(info->read_buffer);
		g_free(info->read_buffer);
		info->read_buffer = g_strdup_printf("%s%s", temp, body);
		g_free(temp);
	}
		
	if (result == GNOME_VFS_ERROR_EOF)
	{
		g_snprintf (searchkey, sizeof (searchkey), "\n%s", loc->code);
        metar = strstr(body, searchkey);
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
		gnome_vfs_async_read(handle, body, DATA_SIZE - 1, metar_finish_read, info);

		return;      
    }
    
    request_done(metar_handle);
    
	return;
}

static void metar_finish_open (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
    WeatherInfo *info = (WeatherInfo *)data;
    WeatherLocation *loc;
    gchar *body;
    int body_len;
    gchar *metar, *eoln;
    gboolean success = FALSE;
    gchar searchkey[WEATHER_LOCATION_CODE_LEN + 2];

    g_return_if_fail(handle == metar_handle);

    g_return_if_fail(info != NULL);
    
    body = g_malloc0(DATA_SIZE);
    
    info->read_buffer = NULL;
    info->forecast = NULL;
    loc = info->location;
    if (loc == NULL) {
	    g_warning (_("WeatherInfo missing location"));
	    request_done(metar_handle);
	    requests_done_check();
	    return;
    }

    if (result != GNOME_VFS_OK) {
        g_warning(_("Failed to get METAR data.\n"));
    } else {
        gnome_vfs_async_read(handle, body, DATA_SIZE - 1, metar_finish_read, info);
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
    gnome_vfs_async_open(&metar_handle, url, GNOME_VFS_OPEN_READ, 0, metar_finish_open, info);
    g_free(url);
/*
    ghttp_set_proxy(metar_request, weather_proxy_url);
    ghttp_set_proxy_authinfo(metar_request, weather_proxy_user, weather_proxy_passwd);
    ghttp_set_header(metar_request, http_hdr_Connection, "close");

    http_process_bg(metar_request, metar_get_finish, info);
*/
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
    if (loc->name[0] == '-')
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
        gchar *end = strstr(p, "\n</PRE>");
        if ((regexec(&iwin_re, p, 1, match, 0) != REG_NOMATCH) &&
            (end - p > match[0].rm_so))
            end = p + match[0].rm_so - 1;
        *end = 0;
        return g_strdup(rangep);
    } else {
        return NULL;
    }

}

static void iwin_finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer buffer,
										GnomeVFSFileSize requested, GnomeVFSFileSize body_len, gpointer data)
{
	WeatherInfo *info = (WeatherInfo *)data;
    WeatherLocation *loc;
	gchar *body, *temp;
	gchar *forecast;

	info->forecast = NULL;
    loc = info->location;
	body = (gchar *)buffer;
	body[body_len] = '\0';

	if (info->read_buffer == NULL)
		info->read_buffer = g_strdup(body);
	else
	{
		temp = g_strdup(info->read_buffer);
		g_free(info->read_buffer);
		info->read_buffer = g_strdup_printf("%s%s", temp, body);
		g_free(temp);
	}
	
	if (result == GNOME_VFS_ERROR_EOF)
	{
		forecast = iwin_parse(info->read_buffer, loc);
		info->forecast = forecast;
	}
	else if (result != GNOME_VFS_OK) {
		g_print("%s", gnome_vfs_result_to_string(result));
        g_warning(_("Failed to get IWIN data.\n"));
    } else {
		gnome_vfs_async_read(handle, body, DATA_SIZE - 1, iwin_finish_read, info);

		return;
    }
    
    request_done(iwin_handle);
    
	return;
}

static void iwin_finish_open (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
    WeatherInfo *info = (WeatherInfo *)data;
    WeatherLocation *loc;
    gchar *body;
    gint body_len;
    gchar *forecast;

    g_return_if_fail(handle == iwin_handle);
	
    g_return_if_fail(info != NULL);

	body = g_malloc0(DATA_SIZE);

	info->read_buffer = NULL;	
    g_free (info->forecast);
    info->forecast = NULL;
    loc = info->location;
    if (loc == NULL) {
	    g_warning (_("WeatherInfo missing location"));
	    request_done(iwin_handle);
	    requests_done_check();
	    return;
    }

    if (result != GNOME_VFS_OK) {
        g_warning(_("Failed to get IWIN forecast data.\n"));
    } else {
        gnome_vfs_async_read(handle, body, DATA_SIZE - 1, iwin_finish_read, info);
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
		buf=g_malloc(len);
		buflen=len;
	}
	memcpy(buf, x, len);
		
	o=buf;

	while(*p)
	{
		if(isspace(*p))
		{
			spacing=1;
			p++;
			continue;
		}
		if(spacing)
		{
			if(count>75)
			{
				if(lastspace)
					*lastspace = '\n';
				count -= lastcount;
			}
			lastspace = o;
			lastcount = count;
			*o++=' ';
			spacing=0;
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
				count=0;
				lastspace = NULL;
				p+=4;
				continue;
			}
			break;
		}
		*o++=*p++;
		count++;
	}
	*o=0;
	return buf;
}


/*
 * Parse the metoffice forecast info.
 * For gnome 2.0 we want to just embed an HTML bonobo component and 
 * be done with this ;) 
 */

static gchar *met_parse (gchar *meto, WeatherLocation *loc)
{ 
    gchar *p;
    gchar *rp;
    gchar *r = g_strdup("Met Office Forecast\n");
    gchar *t;    
    gint i=0;

    static char *key[]=
    {
    	"<!-- <!TODAY_START> -->",
    	"<!-- <!TONIGHT_START> -->",
    	"<!-- <!TOMORROW_START> -->",
    	"<!-- <!OUTLOOK_START> -->",
    	NULL
    };
    static char *keyend[]=
    {
    	"<!-- <!TODAY_END> -->",
    	"<!-- <!TONIGHT_END> -->",
    	"<!-- <!TOMORROW_END> -->",
    	"<!-- <!OUTLOOK_END> -->",
    	NULL
    };
    static char *name[4]=
    {
    	"Today:",
    	"Tonight:",
    	"Tomorrow:",
    	"Outlook:"
    };
    
    
    while(key[i]!=NULL)
    {
    	p=strstr(meto, key[i]);
    	if(p==NULL)
    	{
    		printf("No %s\n", key[i]);
    		i++;
    		continue;
    	}
    	p+=strlen(key[i]);

    	rp = strstr(p, keyend[i]);
	if(rp==NULL)
	{
    		printf("No %s\n", keyend[i]);
		i++;
		continue;
	}
	
	/* p to rp is the text block we want but in HTML malformat */    	

    	t = g_strconcat(r, name[i], "\n", met_reprocess(p, rp-p), "\n", NULL);
    	
    	g_free(r);
    	r = t;
    	i++;
    }
    return r;
}

static void met_finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer buffer,
										GnomeVFSFileSize requested, GnomeVFSFileSize body_len, gpointer data)
{
	WeatherInfo *info = (WeatherInfo *)data;
    WeatherLocation *loc;
    gchar *body, *forecast, *temp;

	info->forecast = NULL;
    loc = info->location;
	body = (gchar *)buffer;
	body[body_len] = '\0';
	
	if (info->read_buffer == NULL)
		info->read_buffer = g_strdup(body);
	else
	{
		temp = g_strdup(info->read_buffer);
		g_free(info->read_buffer);
		info->read_buffer = g_strdup_printf("%s%s", temp, body);
		g_free(temp);
	}
	
	if (result == GNOME_VFS_ERROR_EOF)
	{
		forecast = met_parse(body, loc);
        info->forecast = forecast;
    }
	else if (result != GNOME_VFS_OK) {
		g_print("%s", gnome_vfs_result_to_string(result));
        g_warning(_("Failed to get Met Office data.\n"));
    } else {
		gnome_vfs_async_read(handle, body, DATA_SIZE - 1, met_finish_read, info);

		return;
    }
    
    request_done(met_handle);
    
	return;
}

static void met_finish_open (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
    WeatherInfo *info = (WeatherInfo *)data;
    WeatherLocation *loc;
    gchar *body;
    gint body_len;
    gchar *forecast;

    g_return_if_fail(handle == met_handle);

    g_return_if_fail(info != NULL);

	body = g_malloc0(DATA_SIZE);

	info->read_buffer = NULL;	
    info->forecast = NULL;
    loc = info->location;
    g_return_if_fail(loc != NULL);

    if (result != GNOME_VFS_OK) {
        g_warning(_("Failed to get Met Office forecast data.\n"));
    } else {
		gnome_vfs_async_read(handle, body, DATA_SIZE - 1, met_finish_read, info);
	}

	return;
}

static void metoffice_start_open (WeatherInfo *info)
{
    gchar *url;
    WeatherLocation *loc;
    loc = info->location;
    
    url = g_strdup_printf("http://www.metoffice.gov.uk/datafiles/%s.html", loc->zone+1);
    gnome_vfs_async_open(&met_handle, url, GNOME_VFS_OPEN_READ, 0, met_finish_open, info);
    g_free(url);
/*
    ghttp_set_proxy(met_request, weather_proxy_url);
    ghttp_set_proxy_authinfo(met_request, weather_proxy_user, weather_proxy_passwd);
    ghttp_set_header(met_request, http_hdr_Connection, "close");

    http_process_bg(met_request, met_get_finish, info);
*/
	return;
}	

/* Get forecast into newly alloc'ed string */
static void iwin_start_open (WeatherInfo *info)
{
    gchar state[WEATHER_LOCATION_ZONE_LEN];
    gchar *url;
    WeatherLocation *loc;

    g_return_if_fail(info != NULL);
    info->forecast = NULL;
    loc = info->location;
    g_return_if_fail(loc != NULL);

    if (loc->zone[0] == '-')
        return;
        
    if (loc->zone[0] == ':')	/* Met Office Region Names */
    {
    	metoffice_start_open (info);
    	return;
    }

    strncpy(state, loc->zone, 2);
    state[2] = 0;
    if (weather_forecast == FORECAST_ZONE)
        url = g_strdup_printf("http://iwin.nws.noaa.gov/iwin/%s/zone.html", state);
    else
        url = g_strdup_printf("http://iwin.nws.noaa.gov/iwin/%s/state.html", state);
    gnome_vfs_async_open(&iwin_handle, url, GNOME_VFS_OPEN_READ, 0, iwin_finish_open, info);
    g_free(url);
/*
    ghttp_set_proxy(iwin_request, weather_proxy_url);
    ghttp_set_proxy_authinfo(iwin_request, weather_proxy_user, weather_proxy_passwd);
    ghttp_set_header(iwin_request, http_hdr_Connection, "close");

    http_process_bg(iwin_request, iwin_get_finish, info);
*/
}

static GdkPixmap *wx_construct (gpointer data, gint data_len)
{
    GdkPixmap *pixmap;
    GdkPixbuf *pixbuf;
    GdkPixbufLoader *pixbuf_loader;

    pixbuf_loader = gdk_pixbuf_loader_new ();

    gdk_pixbuf_loader_write (pixbuf_loader, data, data_len, NULL);
    gdk_pixbuf_loader_close (pixbuf_loader, NULL);

    pixbuf = gdk_pixbuf_loader_get_pixbuf (pixbuf_loader);

    pixmap = NULL;

    if (pixbuf != NULL) {
	    gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, NULL, 127);
    }

    g_object_unref (G_OBJECT (pixbuf_loader));

    return pixmap;
}

static void wx_finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer buffer,
										GnomeVFSFileSize requested, GnomeVFSFileSize body_len, gpointer data)
{
	WeatherInfo *info = (WeatherInfo *)data;
    WeatherLocation *loc;
    gchar *body, *temp;
    GdkPixmap *pixmap = NULL;
	
	info->forecast = NULL;
	info->radar = NULL;
    loc = info->location;
	body = (gchar *)buffer;
	body[body_len] = '\0';
	
	if (info->read_buffer == NULL)
		info->read_buffer = g_strdup(body);
	else
	{
		temp = g_strdup(info->read_buffer);
		g_free(info->read_buffer);
		info->read_buffer = g_strdup_printf("%s%s", temp, body);
		g_free(temp);
	}
	
	if (result == GNOME_VFS_ERROR_EOF)
	{
		pixmap = wx_construct(body, body_len);
        info->radar = pixmap;
	}
	else if (result != GNOME_VFS_OK) {
		g_print("%s", gnome_vfs_result_to_string(result));
        g_warning(_("Failed to get METAR data.\n"));
    } else {
		gnome_vfs_async_read(handle, body, DATA_SIZE - 1, wx_finish_read, info);

		return;
    }

	request_done(wx_handle);
	    
	return;
}

static void wx_finish_open (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
    WeatherInfo *info = (WeatherInfo *)data;
    WeatherLocation *loc;
    gchar *body;
    gint body_len;
    GdkPixmap *pixmap = NULL;

    g_return_if_fail(handle == wx_handle);

    g_return_if_fail(info != NULL);

	body = g_malloc0(DATA_SIZE);

	info->read_buffer = NULL;	
	info->radar = NULL;
    loc = info->location;
    g_return_if_fail(loc != NULL);

    if (result != GNOME_VFS_OK) {
        g_warning(_("Failed to get radar map image.\n"));
    } else {
    	gnome_vfs_async_read(handle, body, DATA_SIZE - 1, wx_finish_read, info);
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
    loc = info->location;
    g_return_if_fail(loc != NULL);

    if (loc->radar[0] == '-')
        return;

    url = g_strdup_printf("http://image.weather.com/images/radar/single_site/%sloc_450x284.jpg", loc->radar);
    gnome_vfs_async_open(&wx_handle, url, GNOME_VFS_OPEN_READ, 0, wx_finish_open, info);
    g_free(url);
/*
    ghttp_set_proxy(wx_request, weather_proxy_url);
    ghttp_set_proxy_authinfo(wx_request, weather_proxy_user, weather_proxy_passwd);
    ghttp_set_header(wx_request, http_hdr_Connection, "close");

    http_process_bg(wx_request, wx_get_finish, info);
*/
	return;
}

gboolean _weather_info_fill (WeatherInfo *info, WeatherLocation *location, WeatherInfoFunc cb)
{
    g_return_val_if_fail(((info == NULL) && (location != NULL)) || \
                         ((info != NULL) && (location == NULL)), FALSE);

    /* FIXME: i'm not sure this works as intended anymore */
    if (!info) {
        info = g_new(WeatherInfo, 1);
        info->location = weather_location_clone(location);
    } else {
        location = info->location;
        /* g_free(info->forecast); */
	    info->forecast = NULL;
		if (info->radar != NULL) {
			gdk_pixmap_unref (info->radar);
			info->radar = NULL;
		}
    }

    /* FIX */
    
    if (!requests_init(cb, info)) {
        g_warning(_("Another update already in progress!\n"));
        return FALSE;
    }

    /* Defaults (just in case...) */
    info->units = UNITS_IMPERIAL;
    info->update = 0;
    info->sky = SKY_CLEAR;
    info->cond.significant = FALSE;
    info->cond.phenomenon = PHENOMENON_NONE;
    info->cond.qualifier = QUALIFIER_NONE;
    info->temp = 0;
    info->dew = 0;
    info->humidity = 0;
    info->wind = WIND_VARIABLE;
    info->windspeed = 0;
    info->pressure = 0.0;
    info->visibility = -1.0;
    info->forecast = NULL;
    info->radar = NULL;

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
    weather_location_config_write("location", info->location);
    gnome_config_set_int("units", (gint)info->units);
    gnome_config_set_int("update", info->update);
    gnome_config_set_int("sky", (gint)info->sky);
    gnome_config_set_bool("cond_significant", info->cond.significant);
    gnome_config_set_int("cond_phenomenon", (gint)info->cond.phenomenon);
    gnome_config_set_int("cond_qualifier", (gint)info->cond.qualifier);
    gnome_config_set_float("temp", info->temp);
    gnome_config_set_float("dew", info->dew);
    gnome_config_set_int("humidity", info->humidity);
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

WeatherInfo *weather_info_config_read (void)
{
    WeatherInfo *info = g_new(WeatherInfo, 1);
/*
    info->valid = gnome_config_get_bool("valid=FALSE");
    info->location = weather_location_config_read("location");
    info->units = (WeatherUnits)gnome_config_get_int("units=0");
    info->update = (WeatherUpdate)gnome_config_get_int("update=0");
    info->sky = (WeatherSky)gnome_config_get_int("sky=0");
    info->cond.significant = gnome_config_get_bool("cond_significant=FALSE");
    info->cond.phenomenon = (WeatherConditionPhenomenon)gnome_config_get_int("cond_phenomenon=0");
    info->cond.qualifier = (WeatherConditionQualifier)gnome_config_get_int("cond_qualifier=0");
    info->temp = gnome_config_get_float("temp=0");
    info->dew = gnome_config_get_float("dew=0");
    info->humidity = gnome_config_get_int("humidity=0");
    info->wind = (WeatherWindDirection)gnome_config_get_int("wind=0");
    info->windspeed = gnome_config_get_int("windspeed=0");
    info->pressure = gnome_config_get_float("pressure=0");
    info->visibility = gnome_config_get_float("visibility=0");
    info->forecast = gnome_config_get_string("forecast=None");
    info->radar = NULL;
*/
	info->valid = FALSE;
    info->location = weather_location_config_read("location");
    info->units = (WeatherUnits)0;
    info->update = (WeatherUpdate)0;
    info->sky = (WeatherSky)0;
    info->cond.significant = FALSE;
    info->cond.phenomenon = (WeatherConditionPhenomenon)0;
    info->cond.qualifier = (WeatherConditionQualifier)0;
    info->temp = 0;
    info->dew = 0;
    info->humidity = 0;
    info->wind = (WeatherWindDirection)0;
    info->windspeed = 0;
    info->pressure = 0;
    info->visibility = 0;
    info->forecast = "None";
    info->radar = NULL;  /* FIX */

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

    clone->radar = info->radar;
    if (clone->radar != NULL)
	    gdk_pixmap_ref (clone->radar);

    return clone;
}

void weather_info_free (WeatherInfo *info)
{
    if (info) {
        g_free(info->location);
        info->location = NULL;
        g_free(info->forecast);
        info->forecast = NULL;
	if (info->radar != NULL) {
		gdk_pixmap_unref (info->radar);
		info->radar = NULL;
	}
    }
    g_free(info);
}


void weather_units_set (WeatherUnits units)
{
    weather_units = units;
}

WeatherUnits weather_units_get (void)
{
    return weather_units;
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


void weather_proxy_set (const gchar *url, const gchar *user, const gchar *passwd)
{
    g_free(weather_proxy_url);    weather_proxy_url = NULL;
    g_free(weather_proxy_user);   weather_proxy_user = NULL;
    g_free(weather_proxy_passwd); weather_proxy_passwd = NULL;

    if (url && (strlen(url) > 0))
        weather_proxy_url = g_strdup(url);
    if (user && (strlen(user) > 0))
        weather_proxy_user = g_strdup(user);
    if (passwd && (strlen(passwd) > 0))
        weather_proxy_passwd = g_strdup(passwd);
}

void weather_info_to_metric (WeatherInfo *info)
{
    g_return_if_fail(info != NULL);

    if (info->units == UNITS_METRIC)
        return;

    /* Do conversion */
    info->temp = TEMP_F_TO_C(info->temp);
    info->dew = TEMP_F_TO_C(info->dew);
    info->windspeed = WINDSPEED_KNOTS_TO_KPH(info->windspeed);
    info->pressure = PRESSURE_INCH_TO_MM(info->pressure);
    info->visibility = VISIBILITY_SM_TO_KM(info->visibility);

    /* Change units flag */
    info->units = UNITS_METRIC;
}

void weather_info_to_imperial (WeatherInfo *info)
{
    g_return_if_fail(info != NULL);

    if (info->units == UNITS_IMPERIAL)
        return;

    /* Do conversion */
    info->temp = TEMP_C_TO_F(info->temp);
    info->dew = TEMP_C_TO_F(info->dew);
    info->windspeed = WINDSPEED_KPH_TO_KNOTS(info->windspeed);
    info->pressure = PRESSURE_MM_TO_INCH(info->pressure);
    info->visibility = VISIBILITY_KM_TO_SM(info->visibility);

    /* Change units flag */
    info->units = UNITS_IMPERIAL;
}


const gchar *weather_info_get_location (WeatherInfo *info)
{
    g_return_val_if_fail(info != NULL, NULL);
    g_return_val_if_fail(info->location != NULL, NULL);
    return info->location->name;
}

const gchar *weather_info_get_update (WeatherInfo *info)
{
    static gchar buf[200];

    g_return_val_if_fail(info != NULL, NULL);

    if (!info->valid)
        return "-";

    if (info->update != 0) {
        struct tm *tm;
        tm = localtime(&info->update);
        if (strftime(buf, sizeof(buf), _("%a, %b %d / %H:%M"), tm) <= 0)
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
    return weather_sky_string(info->sky);
}

const gchar *weather_info_get_conditions (WeatherInfo *info)
{
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    return weather_conditions_string(info->cond);
}

const gchar *weather_info_get_temp (WeatherInfo *info)
{
    static gchar buf[10];
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    g_snprintf(buf, sizeof (buf), "%.1f%s", info->temp, TEMP_UNIT_STR(info->units));
    return buf;
}

const gchar *weather_info_get_dew (WeatherInfo *info)
{
    static gchar buf[10];
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    g_snprintf(buf, sizeof (buf), "%.1f%s", info->dew, TEMP_UNIT_STR(info->units));
    return buf;
}

const gchar *weather_info_get_humidity (WeatherInfo *info)
{
    static gchar buf[10];
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    g_snprintf(buf, sizeof (buf), "%d%%", info->humidity);
    return buf;
}

const gchar *weather_info_get_wind (WeatherInfo *info)
{
    static gchar buf[100];
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    if (info->windspeed == 0.00) {
        strncpy(buf, _("Calm"), sizeof(buf));
	buf[sizeof(buf)-1] = '\0';
    } else
        g_snprintf(buf, sizeof(buf), "%s / %d %s",
		   weather_wind_direction_string(info->wind),
		   info->windspeed, WINDSPEED_UNIT_STR(info->units));
    return buf;
}

const gchar *weather_info_get_pressure (WeatherInfo *info)
{
    static gchar buf[30];
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    g_snprintf(buf, sizeof (buf), "%.2f %s", info->pressure, PRESSURE_UNIT_STR(info->units));
    return buf;
}

const gchar *weather_info_get_visibility (WeatherInfo *info)
{
    static gchar buf[40];
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "-";
    if (info->visibility < 0.0)
        return _("Unknown");
    g_snprintf(buf, sizeof (buf), "%.1f %s", info->visibility, VISIBILITY_UNIT_STR(info->units));
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
    static gchar buf[10];
    g_return_val_if_fail(info != NULL, NULL);
    if (!info->valid)
        return "--";
    g_snprintf(buf, sizeof (buf), "%d\260", (int)(info->temp + 0.5));
    return buf;
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


static GdkPixmap **weather_pixmaps_mini = NULL;
static GdkBitmap **weather_pixmaps_mini_mask = NULL;
static GdkPixmap **weather_pixmaps = NULL;
static GdkBitmap **weather_pixmaps_mask = NULL;

#include "pixmaps/unknown-mini.xpm"
#include "pixmaps/sun-mini.xpm"
#include "pixmaps/suncloud-mini.xpm"
#include "pixmaps/cloud-mini.xpm"
#include "pixmaps/rain-mini.xpm"
#include "pixmaps/tstorm-mini.xpm"
#include "pixmaps/snow-mini.xpm"
#include "pixmaps/fog-mini.xpm"

#include "pixmaps/unknown.xpm"
#include "pixmaps/sun.xpm"
#include "pixmaps/suncloud.xpm"
#include "pixmaps/cloud.xpm"
#include "pixmaps/rain.xpm"
#include "pixmaps/tstorm.xpm"
#include "pixmaps/snow.xpm"
#include "pixmaps/fog.xpm"

#define PIX_UNKNOWN   0
#define PIX_SUN       1
#define PIX_SUNCLOUD  2
#define PIX_CLOUD     3
#define PIX_RAIN      4
#define PIX_TSTORM    5
#define PIX_SNOW      6
#define PIX_FOG       7

#define NUM_PIX       8

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

static void init_pixmaps (void)
{
    static gboolean initialized = FALSE;

    if (initialized)
       return;
    initialized = TRUE;

    weather_pixmaps_mini = g_new(GdkPixmap *, NUM_PIX);
    weather_pixmaps_mini_mask = g_new(GdkBitmap *, NUM_PIX);
    xpm_to_pixmap (unknown_mini_xpm, &weather_pixmaps_mini[PIX_UNKNOWN], &weather_pixmaps_mini_mask[PIX_UNKNOWN]);
    xpm_to_pixmap (sun_mini_xpm, &weather_pixmaps_mini[PIX_SUN], &weather_pixmaps_mini_mask[PIX_SUN]);
    xpm_to_pixmap (suncloud_mini_xpm, &weather_pixmaps_mini[PIX_SUNCLOUD], &weather_pixmaps_mini_mask[PIX_SUNCLOUD]);
    xpm_to_pixmap (cloud_mini_xpm, &weather_pixmaps_mini[PIX_CLOUD], &weather_pixmaps_mini_mask[PIX_CLOUD]);
    xpm_to_pixmap (rain_mini_xpm, &weather_pixmaps_mini[PIX_RAIN], &weather_pixmaps_mini_mask[PIX_RAIN]);
    xpm_to_pixmap (tstorm_mini_xpm, &weather_pixmaps_mini[PIX_TSTORM], &weather_pixmaps_mini_mask[PIX_TSTORM]);
    xpm_to_pixmap (snow_mini_xpm, &weather_pixmaps_mini[PIX_SNOW], &weather_pixmaps_mini_mask[PIX_SNOW]);
    xpm_to_pixmap (fog_mini_xpm, &weather_pixmaps_mini[PIX_FOG], &weather_pixmaps_mini_mask[PIX_FOG]);

    weather_pixmaps = g_new(GdkPixmap *, NUM_PIX);
    weather_pixmaps_mask = g_new(GdkBitmap *, NUM_PIX);
    xpm_to_pixmap (unknown_xpm, &weather_pixmaps[PIX_UNKNOWN], &weather_pixmaps_mask[PIX_UNKNOWN]);
    xpm_to_pixmap (sun_xpm, &weather_pixmaps[PIX_SUN], &weather_pixmaps_mask[PIX_SUN]);
    xpm_to_pixmap (suncloud_xpm, &weather_pixmaps[PIX_SUNCLOUD], &weather_pixmaps_mask[PIX_SUNCLOUD]);
    xpm_to_pixmap (cloud_xpm, &weather_pixmaps[PIX_CLOUD], &weather_pixmaps_mask[PIX_CLOUD]);
    xpm_to_pixmap (rain_xpm, &weather_pixmaps[PIX_RAIN], &weather_pixmaps_mask[PIX_RAIN]);
    xpm_to_pixmap (tstorm_xpm, &weather_pixmaps[PIX_TSTORM], &weather_pixmaps_mask[PIX_TSTORM]);
    xpm_to_pixmap (snow_xpm, &weather_pixmaps[PIX_SNOW], &weather_pixmaps_mask[PIX_SNOW]);
    xpm_to_pixmap (fog_xpm, &weather_pixmaps[PIX_FOG], &weather_pixmaps_mask[PIX_FOG]);
}

void _weather_info_get_pixmap (WeatherInfo *info, gboolean mini, GdkPixmap **pixmap, GdkBitmap **mask)
{
    GdkPixmap **pixmaps;
    GdkBitmap **masks;
    gint idx = -1;

    g_return_if_fail(pixmap != NULL);

    init_pixmaps();
    pixmaps = mini ? weather_pixmaps_mini : weather_pixmaps;
    masks = mini ? weather_pixmaps_mini_mask : weather_pixmaps_mask;

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
                    g_assert_not_reached();
                }
        } else {
            switch (sky) {
            case SKY_CLEAR:
                idx = PIX_SUN;
                break;
            case SKY_BROKEN:
            case SKY_SCATTERED:
            case SKY_FEW:
                idx = PIX_SUNCLOUD;
                break;
            case SKY_OVERCAST:
                idx = PIX_CLOUD;
                break;
            default:
                g_assert_not_reached();
            }
        }
    }

    *pixmap = pixmaps[idx];
    if (mask)
        *mask = masks[idx];
}



