/*
 * gweather-gconf.c: GConf interaction methods for gweather.
 *
 * Copyright (C) 2005 Philip Langdale, Papadimitriou Spiros
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *     Philip Langdale <philipl@mail.utexas.edu>
 *     Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <glib/gi18n-lib.h>
#include <libgweather/gweather-gconf.h>

struct _GWeatherGConf
{
	GConfClient *gconf;
	char *prefix;
};


GWeatherGConf *
gweather_gconf_new(const char *prefix)
{
	GWeatherGConf *ctx = g_new0(GWeatherGConf, 1);
	ctx->gconf = gconf_client_get_default();
	ctx->prefix = g_strdup(prefix);

	return ctx;
}


void
gweather_gconf_free(GWeatherGConf *ctx)
{
	g_object_unref(ctx->gconf);
	g_free(ctx->prefix);
	g_free(ctx);
}


GConfClient *
gweather_gconf_get_client(GWeatherGConf *ctx)
{
	return ctx->gconf;
}


gchar *
gweather_gconf_get_full_key (GWeatherGConf *ctx,
			     const gchar     *key)
{
	return g_strdup_printf ("%s/%s", ctx->prefix, key);
}

void
gweather_gconf_set_bool (GWeatherGConf *ctx,
			 const gchar     *key,
			 gboolean         the_bool,
			 GError         **opt_error)
{
	gchar *full_key = gweather_gconf_get_full_key (ctx, key);
	gconf_client_set_bool (ctx->gconf, full_key, the_bool, opt_error);
	g_free (full_key);
}

void
gweather_gconf_set_int (GWeatherGConf *ctx,
			const gchar     *key,
			gint             the_int,
			GError         **opt_error)
{
	gchar *full_key = gweather_gconf_get_full_key (ctx, key);
	gconf_client_set_int (ctx->gconf, full_key, the_int, opt_error);
	g_free (full_key);
}

void
gweather_gconf_set_string (GWeatherGConf *ctx,
			   const gchar     *key,
			   const gchar     *the_string,
			   GError         **opt_error)
{
	gchar *full_key = gweather_gconf_get_full_key (ctx, key);
	gconf_client_set_string (ctx->gconf, full_key, the_string, opt_error);
	g_free (full_key);
}

gboolean
gweather_gconf_get_bool (GWeatherGConf *ctx,
			 const gchar     *key,
			 GError         **opt_error)
{
	gchar *full_key = gweather_gconf_get_full_key (ctx, key);
	gboolean ret = gconf_client_get_bool (ctx->gconf, full_key, opt_error);
	g_free (full_key);
	return ret;
}

gint
gweather_gconf_get_int (GWeatherGConf *ctx,
			const gchar     *key,
			GError         **opt_error)
{
	gchar *full_key = gweather_gconf_get_full_key (ctx, key);
	gint ret = gconf_client_get_int (ctx->gconf, full_key, opt_error);
	g_free (full_key);
	return ret;
}

gchar *
gweather_gconf_get_string (GWeatherGConf *ctx,
			   const gchar     *key,
			   GError         **opt_error)
{
	gchar *full_key = gweather_gconf_get_full_key (ctx, key);
	gchar *ret = gconf_client_get_string (ctx->gconf, full_key, opt_error);
	g_free (full_key);
	return ret;
}


WeatherLocation *
gweather_gconf_get_location(GWeatherGConf *ctx)
{
    WeatherLocation *location;
    gchar *name, *code, *zone, *radar, *coordinates;
    
    name = gweather_gconf_get_string (ctx, "location4", NULL);
    if (!name)
    {
        /* TRANSLATOR: Change this to the default location name (1st parameter) in the */
        /* gweather/Locations file */
        /* For example for New York (JFK) the entry is loc14=New\\ York-JFK\\ Arpt KJFK NYZ076 nyc */
        /* so this should be translated as "New York-JFK Arpt" */
        if (strcmp ("DEFAULT_LOCATION", _("DEFAULT_LOCATION")))
            name = g_strdup (_("DEFAULT_LOCATION"));
        else
            name = g_strdup ("Pittsburgh");
    }

    code = gweather_gconf_get_string (ctx, "location1", NULL);
    if (!code) 
    {
        /* TRANSLATOR: Change this to the default location code (2nd parameter) in the */
        /* gweather/Locations file */
        /* For example for New York (JFK) the entry is loc14=New\\ York-JFK\\ Arpt KJFK NYZ076 nyc */
        /* so this should be translated as "KJFK" */
        if (strcmp ("DEFAULT_CODE", _("DEFAULT_CODE")))
            code = g_strdup (_("DEFAULT_CODE"));
        else
    	    code = g_strdup ("KPIT");
    }

    zone = gweather_gconf_get_string (ctx, "location2", NULL);
    if (!zone)
    {
        /* TRANSLATOR: Change this to the default location zone (3rd parameter) in the */
        /* gweather/Locations file */
        /* For example for New York (JFK) the entry is loc14=New\\ York-JFK\\ Arpt KJFK NYZ076 nyc */
        /* so this should be translated as "NYZ076" */
        if (strcmp ("DEFAULT_ZONE", _("DEFAULT_ZONE")))
            zone = g_strdup (_("DEFAULT_ZONE" ));
        else
            zone = g_strdup ("PAZ021");

    }

    radar = gweather_gconf_get_string(ctx, "location3", NULL);
    if (!radar)
    {
        /* Translators: Change this to the default location radar (4th parameter) in the */
        /* gweather/Locations file */
        /* For example for New York (JFK) the entry is loc14=New\\ York-JFK\\ Arpt KJFK NYZ076 nyc */
        /* so this should be translated as "nyc" */
        if (strcmp ("DEFAULT_RADAR", _("DEFAULT_RADAR")))
            radar = g_strdup (_("DEFAULT_RADAR"));
        else
            radar = g_strdup ("pit");
    }
    coordinates = gweather_gconf_get_string (ctx, "coordinates", NULL);
    if (coordinates && !strcmp ("DEFAULT_COORDINATES", coordinates))
    {
        g_free (coordinates);
        coordinates = NULL;
    }
    
    location = weather_location_new (name, code, zone, radar, coordinates);
    
    g_free (name);
    g_free (code);
    g_free (zone);
    g_free (radar);
    g_free (coordinates);
    
    return location;
}

