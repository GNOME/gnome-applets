/* $Id$ */

#include <ctype.h>

#include <gnome.h>
#include "weather.h"

static char *met_reprocess(char *x, int len)
{
	char *p = x;
	char *o;
	int spacing = 0;
	static gchar *buf;
	static gint buflen = 0;
	gchar *lastspace = NULL;
	int count = 0;
	
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

void metoffice_start_open (WeatherInfo *info)
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

