/* $Id$ */

#include <gnome.h>
#include "weather.h"

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

void bom_start_open (WeatherInfo *info)
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
