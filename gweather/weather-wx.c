/* $Id$ */

#include <gnome.h>
#include "weather.h"

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
void wx_start_open (WeatherInfo *info)
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

