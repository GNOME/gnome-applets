/* $Id$ */

/*
 * Code for parsing METAR weather observations
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <regex.h>
#include <gnome.h>
#include "weather.h"



#define TIME_RE   0
#define WIND_RE   1
#define VIS_RE    2
#define COND_RE   3
#define CLOUD_RE  4
#define TEMP_RE   5
#define PRES_RE   6

#define RE_NUM   7


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

static gint metar_tok_time (gchar *tokp, WeatherInfo *info)
{
    gint day, hr, min;

    sscanf(tokp, "%2u%2u%2u", &day, &hr, &min);
    info->update = make_time(day, hr, min);
    return WIND_RE;
}

static gint metar_tok_wind (gchar *tokp, WeatherInfo *info)
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
    return VIS_RE;
}

static gint metar_tok_vis (gchar *tokp, WeatherInfo *info)
{
    gchar *pfrac, *pend, *psp;
    gchar sval[6];
    gint num, den, val;
    gint rtnval = COND_RE;
    
    memset(sval, 0, sizeof(sval));

    if (!strcmp(tokp,"CAVOK")) {
        // "Ceiling And Visibility OK": visibility >= 10 KM
        info->visibility=10000. / VISIBILITY_SM_TO_M(1.);
	info->sky = SKY_CLEAR;
	rtnval = CLOUD_RE;
    }
    else if (0 != (pend = strstr(tokp, "SM"))) {
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
    return rtnval;
}

static gint metar_tok_cloud (gchar *tokp, WeatherInfo *info)
{
    gchar stype[4], salt[4];
    gint alt = -1;

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
    return CLOUD_RE;
}

static gint metar_tok_pres (gchar *tokp, WeatherInfo *info)
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
    return RE_NUM;
}

static gint metar_tok_temp (gchar *tokp, WeatherInfo *info)
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
    return PRES_RE;
}

static gint metar_tok_cond (gchar *tokp, WeatherInfo *info)
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
        return CLOUD_RE;
    }

    if ((info->cond.qualifier != QUALIFIER_NONE) || (info->cond.phenomenon != PHENOMENON_NONE))
        info->cond.significant = TRUE;
    return CLOUD_RE;
}

#define TIME_RE_STR  "([0-9]{6})Z"
#define WIND_RE_STR  "(([0-9]{3})|VRB)([0-9]?[0-9]{2})(G[0-9]?[0-9]{2})?KT"
#define VIS_RE_STR   "((([0-9]?[0-9])|(M?([12] )?([1357]/1?[0-9])))SM)|" \
                     "([0-9]{4}(N|NE|E|SE|S|SW|W|NW( [0-9]{4}(N|NE|E|SE|S|SW|W|NW))?)?)|" \
                     "CAVOK"
#define COND_RE_STR  "(-|\\+)?(VC|MI|BC|PR|TS|BL|SH|DR|FZ)?(DZ|RA|SN|SG|IC|PE|GR|GS|UP|BR|FG|FU|VA|SA|HZ|PY|DU|SQ|SS|DS|PO|\\+?FC)"
#define CLOUD_RE_STR "((CLR|BKN|SCT|FEW|OVC|SKC|NSC)([0-9]{3})?(CB|TCU)?)"
#define TEMP_RE_STR  "(M?[0-9][0-9])/(M?(//|[0-9][0-9]))"
#define PRES_RE_STR  "(A|Q)([0-9]{4})"

static regex_t metar_re[RE_NUM];
static gint (*metar_f[RE_NUM])(gchar *tokp, WeatherInfo *info);

static void metar_init_re (void)
{
    static gboolean initialized = FALSE;
    if (initialized)
        return;
    initialized = TRUE;

    regcomp(&metar_re[TIME_RE], TIME_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[WIND_RE], WIND_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[VIS_RE], VIS_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[COND_RE], COND_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[CLOUD_RE], CLOUD_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[TEMP_RE], TEMP_RE_STR, REG_EXTENDED);
    regcomp(&metar_re[PRES_RE], PRES_RE_STR, REG_EXTENDED);

    metar_f[TIME_RE] = metar_tok_time;
    metar_f[WIND_RE] = metar_tok_wind;
    metar_f[VIS_RE] = metar_tok_vis;
    metar_f[COND_RE] = metar_tok_cond;
    metar_f[CLOUD_RE] = metar_tok_cloud;
    metar_f[TEMP_RE] = metar_tok_temp;
    metar_f[PRES_RE] = metar_tok_pres;
}

/*static*/ gboolean metar_parse (gchar *metar, WeatherInfo *info)
{
    gchar *p;
    //gchar *rmk;
    gint ntoks;
    gint i;
    gboolean isSet;
    regmatch_t rm;
    gchar *tokp;

    g_return_val_if_fail(info != NULL, FALSE);
    g_return_val_if_fail(metar != NULL, FALSE);

    metar_init_re();

    /*
     * Force parsing to end at "RMK" field.  This prevents a subtle
     * problem when info within the remark happens to match an earlier state
     * and, as a result, throws off all the remaining expression
     */
    if (0 != (p = strstr(metar, "RMK"))) {
        *p = '\0';
	//rmk = p+4;   // uncomment this if RMK data becomes useful
    }
    
    p = metar;
    
    i = TIME_RE;
    isSet = FALSE;
    while (i < RE_NUM && *p) {
        if (0 == regexec(&metar_re[i], p, 1, &rm, 0)
	    && (rm.rm_so == 0 || p[rm.rm_so - 1] == ' ')) {
	    if (!isSet) {
	        tokp = g_strndup(p+rm.rm_so, rm.rm_eo-rm.rm_so);
		gint next = metar_f[i](tokp, info);
		isSet = (next == i);
		g_free (tokp);
		i = next;
	    }
	    p += rm.rm_eo;
	    p += strspn(p, " ");
	} else {
	  i++;
	  isSet = FALSE;
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
void metar_start_open (WeatherInfo *info)
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
