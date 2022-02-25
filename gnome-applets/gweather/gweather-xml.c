/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* gweather-xml.c - Locations.xml parsing code
 *
 * Copyright (C) 2005 Ryan Lortie, 2004 Gareth Owen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <math.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <libxml/xmlreader.h>

#include <libgweather/gweather.h>

#include "gweather-xml.h"


/**
 * SECTION:gweather-xml
 * @Title: gweather-xml
 */


static gboolean
gweather_xml_parse_node (GWeatherLocation *gloc,
			 GtkTreeStore *store, GtkTreeIter *parent)
{
    GtkTreeIter iter, *self = &iter;
    GWeatherLocation *first_child;
    GWeatherLocation *second_child;
    GWeatherLocation *parent_loc;
    GWeatherLocationLevel level;
    const char *name, *code;
    double latitude, longitude;
    gboolean has_coords;
    GWeatherLocation *child;

    name = gweather_location_get_name (gloc);
    first_child = gweather_location_next_child (gloc, NULL);
    level = gweather_location_get_level (gloc);

    if (first_child == NULL && level < GWEATHER_LOCATION_WEATHER_STATION) {
	return TRUE;
    }

    switch (gweather_location_get_level (gloc)) {
    case GWEATHER_LOCATION_WORLD:
	self = parent;
	break;

    case GWEATHER_LOCATION_REGION:
    case GWEATHER_LOCATION_COUNTRY:
    case GWEATHER_LOCATION_ADM1:
	/* Create a row with a name but no WeatherLocation */
	gtk_tree_store_append (store, &iter, parent);
	gtk_tree_store_set (store, &iter,
			    GWEATHER_XML_COL_LOCATION_NAME, name,
			    -1);
	break;

    case GWEATHER_LOCATION_CITY:
	/* If multiple children, treat this like a
	 * region/country/adm1. If a single child, merge with that
	 * location.
	 */
	gtk_tree_store_append (store, &iter, parent);
	gtk_tree_store_set (store, &iter,
			    GWEATHER_XML_COL_LOCATION_NAME, name,
			    -1);

	if (first_child != NULL) {
	    g_object_ref (first_child);
	    second_child = gweather_location_next_child (gloc, first_child);

	    if (second_child == NULL) {
		code = gweather_location_get_code (first_child);
		has_coords = gweather_location_has_coords (first_child);
		latitude = longitude = 0;
		if (has_coords) {
		    gweather_location_get_coords (first_child, &latitude, &longitude);
		}

		gtk_tree_store_set (store, &iter,
				    GWEATHER_XML_COL_METAR_CODE, code,
				    GWEATHER_XML_COL_LATLON_VALID, has_coords,
				    GWEATHER_XML_COL_LATITUDE, latitude,
				    GWEATHER_XML_COL_LONGITUDE, longitude,
				    -1);
	    }

	    g_clear_object (&second_child);
	}
	break;

    case GWEATHER_LOCATION_WEATHER_STATION:
	gtk_tree_store_append (store, &iter, parent);
	gtk_tree_store_set (store, &iter,
			    GWEATHER_XML_COL_LOCATION_NAME, name,
			    -1);

	parent_loc = gweather_location_get_parent (gloc);
	if (parent_loc != NULL) {
		if (gweather_location_get_level (parent_loc) == GWEATHER_LOCATION_CITY)
			name = gweather_location_get_name (parent_loc);

		g_object_unref (parent_loc);
	}

	code = gweather_location_get_code (gloc);
	has_coords = gweather_location_has_coords (gloc);
	latitude = longitude = 0;
	if (has_coords) {
		gweather_location_get_coords (gloc, &latitude, &longitude);
	}
	
	gtk_tree_store_set (store, &iter,
			    GWEATHER_XML_COL_METAR_CODE, code,
			    GWEATHER_XML_COL_LATLON_VALID, has_coords,
			    GWEATHER_XML_COL_LATITUDE, latitude,
			    GWEATHER_XML_COL_LONGITUDE, longitude,
			    -1);
	break;

    case GWEATHER_LOCATION_NAMED_TIMEZONE:
	break;

    case GWEATHER_LOCATION_DETACHED:
    default:
	g_assert_not_reached ();

	break;
    }

    g_clear_object (&first_child);

    child = NULL;
    while ((child = gweather_location_next_child (gloc, child)) != NULL) {
	if (!gweather_xml_parse_node (child, store, self)) {
	    g_object_unref (child);
	    return FALSE;
	}
    }

    return TRUE;
}

GtkTreeModel *
gweather_xml_load_locations (void)
{
    GWeatherLocation *world;
    GtkTreeStore *store;

    world = gweather_location_get_world ();
    if (!world)
	return NULL;

    store = gtk_tree_store_new (5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_DOUBLE, G_TYPE_DOUBLE);

    if (!gweather_xml_parse_node (world, store, NULL)) {
	g_object_unref (store);
	store = NULL;
    }

    g_object_unref (world);

    return (GtkTreeModel *)store;
}
