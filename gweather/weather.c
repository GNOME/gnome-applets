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
#include <libxml/parser.h>
#include "weather.h"


#define DATA_SIZE 5000

/* Unit conversions and names */

#define TEMP_F_TO_C(f)  (((f) - 32.0) * 0.555556)
#define TEMP_C_TO_F(c)  (((c) * 1.8) + 32.0)
#define TEMP_UNIT_STR(units)  (((units) == UNITS_IMPERIAL) ? "\302\260 F" : "\302\260 C")

#define WINDSPEED_KNOTS_TO_KPH(knots)  ((knots) * 1.851965)
#define WINDSPEED_KPH_TO_KNOTS(kph)    ((kph) * 0.539967)
#define WINDSPEED_UNIT_STR(units) (((units) == UNITS_IMPERIAL) ? _("knots") : _("kph"))

#define PRESSURE_INCH_TO_HPA(inch)   ((inch) * 33.86)
#define PRESSURE_HPA_TO_INCH(mm)     ((mm) / 33.86)
#define PRESSURE_MBAR_TO_INCH(mbar) ((mbar) * 0.02963742)
#define PRESSURE_UNIT_STR(units) (((units) == UNITS_IMPERIAL) ? _("inHg") : _("hPa"))

#define VISIBILITY_SM_TO_KM(sm)  ((sm) * 1.609344)
#define VISIBILITY_KM_TO_SM(km)  ((km) * 0.621371)
#define VISIBILITY_UNIT_STR(units) (((units) == UNITS_IMPERIAL) ? _("miles") : _("kilometers"))



static const gchar *sky_str[] = {
    N_("Clear Sky"),
    N_("Broken clouds"),
    N_("Scattered clouds"),
    N_("Few clouds"),
    N_("Overcast")
};

static const gchar *conditions_str[] = {
	N_("Windy"),
	N_("Thunder Storms"),
	N_("Snow"),
	N_("Rain"),
	N_("Heavy Rain"),
	N_("Sleet"),
	N_("Light Flurries"),
	N_("Flurries"),
	N_("Snow and Wind"),
	N_("Dust"),
	N_("Fog"),
	N_("Haze"),
	N_("Smoke"),
	N_("Cold"),
	N_("Cloudy"),
	N_("Mostly Cloudy"),
	N_("Partly Cloudy"),
	N_("Sunny"),
	N_("Mostly Sunny"),
	N_("Hot"),
	N_("Nothing")
};

static const gchar *wind_direction_str[] = {
    N_("Variable"),
    N_("North"), N_("North - NorthEast"), N_("Northeast"), N_("East - NorthEast"),
    N_("East"), N_("East - Southeast"), N_("Southeast"), N_("South - Southeast"),
    N_("South"), N_("South - Southwest"), N_("Southwest"), N_("West - Southwest"),
    N_("West"), N_("West - Northwest"), N_("Northwest"), N_("North - Northwest"),
    N_("Calm")
};

static void
free_forecasts (GList *forecasts)
{
	GList *list = forecasts;
	while (list) {
		WeatherForecast *forecast = list->data;		
		if (forecast->day) g_free (forecast->day);
		g_free (forecast);
		list = g_list_next (list);
	}
	g_list_free (forecasts);
}

const gchar *
get_wind_direction (gchar *dir)
{
	if (!dir)
		return NULL;

	if (g_strcasecmp (dir, "N") == 0)
		return wind_direction_str[1];
	else if (g_strcasecmp (dir, "NNE") == 0)
		return wind_direction_str[2];
	else if (g_strcasecmp (dir, "NN") == 0)
		return wind_direction_str[3];
	else if (g_strcasecmp (dir, "ENE") == 0)
		return wind_direction_str[4];
	else if (g_strcasecmp (dir, "E") == 0)
		return wind_direction_str[5];
	else if (g_strcasecmp (dir, "ESE") == 0)
		return wind_direction_str[6];
	else if (g_strcasecmp (dir, "SE") == 0)
		return wind_direction_str[7];
	else if (g_strcasecmp (dir, "SSE") == 0)
		return wind_direction_str[8];
	else if (g_strcasecmp (dir, "S") == 0)
		return wind_direction_str[9];
	else if (g_strcasecmp (dir, "SSW") == 0)
		return wind_direction_str[10];
	else if (g_strcasecmp (dir, "SW") == 0)
		return wind_direction_str[11];
	else if (g_strcasecmp (dir, "WSW") == 0)
		return wind_direction_str[12];
	else if (g_strcasecmp (dir, "W") == 0)
		return wind_direction_str[13];
	else if (g_strcasecmp (dir, "WNW") == 0)
		return wind_direction_str[14];
	else if (g_strcasecmp (dir, "NW") == 0)
		return wind_direction_str[15];
	else if (g_strcasecmp (dir, "NNW") == 0)
		return wind_direction_str[16];
	else if (g_strcasecmp (dir, "CALM") == 0)
		return wind_direction_str[17];
	else
		return wind_direction_str[0];

}

static gint
get_conditions_id (gint wid)
{
	switch (wid) {
	case 1:
	case 23:
	case 24:
		return WINDY;
		break;
	case 3:
	case 4:
	case 17:
	case 35:
	case 37:
	case 38:
		return TSTORMS;
		break;
	case 15:
	case 16:
	case 41:
	case 42:
	case 5:
		return SNOW;
		break;
	case 6:
	case 9:
	case 11:
		return RAIN;
		break;
	case 18:
	case 39:
	case 40:
	case 12:
		return HEAVYRAIN;
		break;
	case 7:
	case 8:
	case 10:
		return SLEET;
		break;
	case 13:
		return LIGHTFLURRIES;
		break;
	case 14:
		return FLURRIES;
		break;
	case 43:
		return SNOWWIND;
		break;
	case 19:
		return DUST;
		break;
	case 20:
		return FOG;
		break;
	case 21:
		return HAZE;
		break;
	case 22:
		return SMOKE;
		break;
	case 25:
		return COLD;
		break;
	case 26:
		return CLOUDY;
		break;
	case 27:
	case 28:
		return MOSTLYCLOUDY;
		break;
	case 29:
	case 30:
	case 44:
		return PARTLYCLOUDY;
		break;
	case 31:
	case 32:
		return SUNNY;
		break;
	case 33:
	case 34:
		return MOSTLYSUNNY;
		break;
	case 36:
		return HOT;
		break;
	default:
		return DEFAULT;
		break;
	}
}

const gchar *
get_conditions (gint wid)
{
	return conditions_str[get_conditions_id (wid)];
	
}

static WeatherForecast *
parse_forecast (xmlDocPtr doc, xmlNodePtr cur)
{
	WeatherForecast *forecast;
	xmlNodePtr curtemp;

	forecast = g_new0 (WeatherForecast, 1);

	curtemp = cur->xmlChildrenNode;
	while (curtemp != NULL) {
		if ((!xmlStrcmp(curtemp->name, (const xmlChar *)"day"))) {
			gchar *tmp;
			tmp = xmlNodeListGetString(doc, 
								            curtemp->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				forecast->day = NULL;
			else
				forecast->day = g_strdup (tmp);
			xmlFree (tmp);
		}
		else if ((!xmlStrcmp(curtemp->name, (const xmlChar *)"high"))) {
			gchar *tmp;
			tmp =  xmlNodeListGetString(doc, 
								curtemp->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				forecast->high = 0;
			else
				forecast->high = atoi (tmp);
			xmlFree (tmp);
		}
		else if ((!xmlStrcmp(curtemp->name, (const xmlChar *)"low"))) {
			gchar *tmp;
			tmp =  xmlNodeListGetString(doc, 
								curtemp->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				forecast->low = 0;
			else
				forecast->low = atoi (tmp);
			xmlFree (tmp);
		}
		else if ((!xmlStrcmp(curtemp->name, (const xmlChar *)"wid"))) {
			gchar *tmp;
			tmp =  xmlNodeListGetString(doc, 
								curtemp->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				forecast->wid = -1;
			else
				forecast->wid = atoi (tmp);
			xmlFree (tmp);
		}
		else if ((!xmlStrcmp(curtemp->name, (const xmlChar *)"sky"))) {
		}	
		else if ((!xmlStrcmp(curtemp->name, (const xmlChar *)"precipitation"))) {
			gchar *tmp;
			tmp =  xmlNodeListGetString(doc, 
								curtemp->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				forecast->precip = 0;
			else
				forecast->precip = atoi (tmp);
			xmlFree (tmp);
		}
		curtemp = curtemp->next;
	}
	
	return forecast;
}

static void
parse_wind(WeatherInfo *info, xmlDocPtr doc, xmlNodePtr cur)
{
	xmlNodePtr curtemp;

	curtemp = cur->xmlChildrenNode;

	while (curtemp) {
		if ((!xmlStrcmp(curtemp->name, (const xmlChar *)"direction"))) {
			gchar *tmp;
			tmp = xmlNodeListGetString(doc, 
								            curtemp->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				info->winddir = NULL;
			else {
				if (info->winddir)
					g_free (info->winddir);
				info->winddir = g_strdup (tmp);
			}
			xmlFree (tmp);
		}
		else if ((!xmlStrcmp(curtemp->name, (const xmlChar *)"strength"))) {
			gchar *tmp;
			tmp = xmlNodeListGetString(doc, 
								            curtemp->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				info->windstrength = 0;
			else
				info->windstrength = atoi (tmp);
			xmlFree (tmp);	
		}
		curtemp = curtemp->next;
	}

}
static gboolean
parse_xml (WeatherInfo *info)
{
	gchar *xml = info->xml;
	xmlDocPtr doc;
	xmlNodePtr cur;

	doc = xmlParseMemory (xml, strlen(xml));
	if (!doc) {
		return FALSE;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		xmlFreeDoc(doc);
		return FALSE;
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "weather")) {
		xmlFreeDoc(doc);
		return FALSE;
	}
	
	if (info->forecasts)
		free_forecasts (info->forecasts);
	info->forecasts = NULL;
	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"city"))){
			xmlChar *tmp;
			tmp = xmlNodeListGetString(doc, 
									    cur->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				info->city = NULL;
			else {
				if (info->city) g_free (info->city);
				info->city = g_strdup (tmp);
			}
			xmlFree (tmp);	
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"forecast"))){
			WeatherForecast *forecast;
			forecast = parse_forecast (doc, cur);
			info->forecasts = g_list_append (info->forecasts, forecast);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"date"))){
			xmlChar *tmp;
			tmp = xmlNodeListGetString(doc, 
								cur->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				info->date = NULL;
			else {
				if (info->date) g_free (info->date);
				info->date = g_strdup (tmp);
			}
			xmlFree (tmp);			
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"temp"))){
			xmlChar *tmp = NULL;
			tmp = xmlNodeListGetString(doc, 
								cur->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				info->curtemp = 0;
			else
				info->curtemp = atoi(tmp);
			xmlFree (tmp);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"wind"))){
			parse_wind (info, doc, cur);	
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"barometer"))){
			xmlChar *tmp;
			tmp = xmlNodeListGetString(doc, 
								cur->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				info->barometer = 0.0;
			else
				info->barometer = atof (tmp);
			xmlFree (tmp);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"humidity"))){
			xmlChar *tmp;
			tmp = xmlNodeListGetString(doc, 
								cur->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				info->humidity = 0;
			else
				info->humidity = atoi (tmp);
			xmlFree (tmp);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"realtemp"))){
			xmlChar *tmp;
			tmp = xmlNodeListGetString(doc, 
								cur->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				info->realtemp = 0;
			else
				info->realtemp = atoi (tmp);
			xmlFree (tmp);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"visibility"))){
			xmlChar *tmp;
			tmp = xmlNodeListGetString(doc, 
								cur->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				info->visibility = 0.0;
			else
				info->visibility =atof (tmp);
			xmlFree (tmp);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"wid"))){
			xmlChar *tmp;
			tmp = xmlNodeListGetString(doc, 
								cur->xmlChildrenNode, 1);
			if (!tmp || (strlen(tmp)==0))
				info->wid = -1;
			else
				info->wid = atoi (tmp);
			xmlFree (tmp);
		}	 
		cur = cur->next;
	}

 	return TRUE;
}

static void 
finish_read(GnomeVFSAsyncHandle *handle, GnomeVFSResult result, 
		      gpointer buffer, GnomeVFSFileSize requested, 
		      GnomeVFSFileSize body_len, gpointer data)
{
	GWeatherApplet *applet = data;
	WeatherInfo *info = applet->gweather_info;
	gchar *body, *temp;

	body = (gchar *)buffer;
	body[body_len] = '\0';
	if (info->xml == NULL)
        	info->xml = g_strdup(body);
    	else
    	{
       		temp = g_strdup(info->xml);
		g_free(info->xml);
		info->xml = g_strdup_printf("%s%s", temp, body);
		g_free(temp);
    	}

	if (result == GNOME_VFS_ERROR_EOF)
    	{
		parse_xml (info);
		g_free (info->xml);
		info->xml = NULL;
		info->success = TRUE;
		end_animation (applet);
		update_display (applet);
		gweather_dialog_update (applet);
		info->main_handle = NULL;
    	}
    	else if (result != GNOME_VFS_OK) {
		g_print("%s", gnome_vfs_result_to_string(result));
		info->main_handle = NULL;
		gtk_tooltips_set_tip(applet->tooltips, 
						    GTK_WIDGET(applet->applet), 
    			                 	    _("Could not download forecast"), NULL);
		info->success = FALSE;
		end_animation (applet);
    	} else {
		gnome_vfs_async_read(handle, body, DATA_SIZE - 1, 
							    finish_read, applet);
		return;
    	}

	 g_free (buffer);
}

static void 
url_opened (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer data)
{
	GWeatherApplet *applet = data;
	WeatherInfo *info = applet->gweather_info;
	gchar *body;
	gint body_len;
	
	if (result != GNOME_VFS_OK) {
        	g_print("%s", gnome_vfs_result_to_string(result));
        	info->main_handle = NULL;
		gtk_tooltips_set_tip(applet->tooltips, 
						    GTK_WIDGET(applet->applet), 
    			                 	    _("Could not download forecast"), NULL);
		end_animation (applet);
		info->success = FALSE;
    	} else {
		body = g_malloc0(DATA_SIZE);
        	gnome_vfs_async_read(handle, body, DATA_SIZE -1, 
							   finish_read, applet); 
    	}
}

static 
void get_xml (GWeatherApplet *applet)
{
	WeatherInfo *info = applet->gweather_info;
	gchar *url;
	
	if (info->xml) {
		g_free (info->xml);
		info->xml = NULL;
	}

	if (applet->gweather_pref.use_metric)
		url = g_strdup_printf ("%s&celsius=true", applet->gweather_pref.url);
	else
		url = g_strdup (applet->gweather_pref.url);
	g_print ("%s \n", url);
	gnome_vfs_async_open(&info->main_handle, url,
						    GNOME_VFS_OPEN_READ, 
    						     0, url_opened, applet);
	g_free (url);
}

static void
cancel_update (GWeatherApplet *applet)
{
	if (!applet->gweather_info->main_handle)
		return;

	gnome_vfs_async_cancel (applet->gweather_info->main_handle);
	applet->gweather_info->main_handle = NULL;

}

gboolean 
update_weather (GWeatherApplet *applet)
{
	/* If another update is in progress cancel it. Perhaps we
	** don't always want to do this */
	if (applet->gweather_info->main_handle != NULL)
		cancel_update (applet);
	start_animation (applet);
	get_xml (applet);
	return TRUE;
}

static GdkPixbuf **weather_pixbufs_mini = NULL;

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

static void init_pixbufs (void)
{
    static gboolean initialized = FALSE;

    if (initialized)
       return;
    initialized = TRUE;

    weather_pixbufs_mini = g_new(GdkPixbuf *, NUM_COND);
    weather_pixbufs_mini[DEFAULT] = gdk_pixbuf_new_from_xpm_data ((const char **)unknown_mini_xpm);	
    weather_pixbufs_mini[SUNNY] = gdk_pixbuf_new_from_xpm_data ((const char **)sun_mini_xpm);	
    weather_pixbufs_mini[PARTLYCLOUDY] = 	
		gdk_pixbuf_new_from_xpm_data ((const char **)suncloud_mini_xpm);	   
    weather_pixbufs_mini[CLOUDY] = gdk_pixbuf_new_from_xpm_data ((const char **)cloud_mini_xpm);	
    weather_pixbufs_mini[RAIN] = gdk_pixbuf_new_from_xpm_data ((const char **)rain_mini_xpm);	
    weather_pixbufs_mini[TSTORMS] = gdk_pixbuf_new_from_xpm_data ((const char **)tstorm_mini_xpm);	
    weather_pixbufs_mini[SNOW] = gdk_pixbuf_new_from_xpm_data ((const char **)snow_mini_xpm);	
    weather_pixbufs_mini[FOG] = gdk_pixbuf_new_from_xpm_data ((const char **)fog_mini_xpm);	

    /* FIXME: we need new icons for these conditions */
    weather_pixbufs_mini[WINDY] =  weather_pixbufs_mini[PARTLYCLOUDY];
    weather_pixbufs_mini[HEAVYRAIN] = weather_pixbufs_mini[RAIN];
    weather_pixbufs_mini[SLEET] = weather_pixbufs_mini[SNOW];
    weather_pixbufs_mini[LIGHTFLURRIES] = weather_pixbufs_mini[SNOW];
    weather_pixbufs_mini[FLURRIES] = weather_pixbufs_mini[SNOW];
    weather_pixbufs_mini[SNOWWIND] = weather_pixbufs_mini[SNOW];
    weather_pixbufs_mini[DUST] = weather_pixbufs_mini[FOG];
    weather_pixbufs_mini[HAZE] = weather_pixbufs_mini[FOG];
    weather_pixbufs_mini[SMOKE] = weather_pixbufs_mini[FOG];
    weather_pixbufs_mini[COLD] = weather_pixbufs_mini[CLOUDY];
    weather_pixbufs_mini[MOSTLYCLOUDY] = weather_pixbufs_mini[CLOUDY];
    weather_pixbufs_mini[MOSTLYSUNNY] = weather_pixbufs_mini[PARTLYCLOUDY];
    weather_pixbufs_mini[HOT] = weather_pixbufs_mini[SUNNY];

#if 0
    weather_pixbufs = g_new(GdkPixbuf *, NUM_PIX);
    weather_pixbufs[PIX_UNKNOWN] = gdk_pixbuf_new_from_xpm_data ((const char **)unknown_xpm);
    weather_pixbufs[PIX_SUN] = gdk_pixbuf_new_from_xpm_data ((const char **)sun_xpm);
    weather_pixbufs[PIX_SUNCLOUD] = gdk_pixbuf_new_from_xpm_data ((const char **)suncloud_xpm);
    weather_pixbufs[PIX_CLOUD] = gdk_pixbuf_new_from_xpm_data ((const char **)cloud_xpm);
    weather_pixbufs[PIX_RAIN] = gdk_pixbuf_new_from_xpm_data ((const char **)rain_xpm);
    weather_pixbufs[PIX_TSTORM] = gdk_pixbuf_new_from_xpm_data ((const char **)tstorm_xpm);
    weather_pixbufs[PIX_SNOW] = gdk_pixbuf_new_from_xpm_data ((const char **)snow_xpm);
    weather_pixbufs[PIX_FOG] = gdk_pixbuf_new_from_xpm_data ((const char **)fog_xpm);
#endif

}

GdkPixbuf * 
get_conditions_pixbuf (gint wid) 
{
	init_pixbufs ();
	return weather_pixbufs_mini[get_conditions_id (wid)];

}

static gboolean
do_update (gpointer data)
{
	GWeatherApplet *applet = data;
	GdkPixbuf *tmp, *tmp2;
	GList *list = applet->gweather_info->forecasts;
	WeatherForecast *forecast = NULL;
	gint oldnum;
	
	if (applet->animation_loc > applet->gweather_info->numforecasts-1)
		applet->animation_loc = 0;

	tmp = get_conditions_pixbuf (-1);
	gtk_image_set_from_pixbuf (GTK_IMAGE (applet->images[applet->animation_loc]), tmp);

	if (applet->animation_loc == 1) {
		gtk_image_set_from_pixbuf (GTK_IMAGE (applet->images[0]), 
							  get_conditions_pixbuf (applet->gweather_info->wid));
	}
	else {
		if (applet->animation_loc == 0)
			oldnum = applet->gweather_info->numforecasts-1;
		else
			oldnum = applet->animation_loc - 1;
		forecast = g_list_nth_data (list, oldnum-1);
		if (forecast)
			gtk_image_set_from_pixbuf (GTK_IMAGE (applet->images[oldnum]), 
						  	  	  get_conditions_pixbuf (forecast->wid));
		else
			gtk_image_set_from_pixbuf (GTK_IMAGE (applet->images[oldnum]), 
						  	  	  get_conditions_pixbuf (31));
	}
	applet->animation_loc ++;

	return TRUE;
}

void
start_animation (GWeatherApplet *applet)
{
	if (applet->animation_tag > 0)
		return;
	applet->animation_loc = 0;
	applet->animation_tag = gtk_timeout_add (100, do_update, applet);

}

void 
end_animation (GWeatherApplet *applet)
{
	if (applet->animation_tag <= 0)
		return;
	
	gtk_timeout_remove (applet->animation_tag);
	applet->animation_tag = -1;

}




