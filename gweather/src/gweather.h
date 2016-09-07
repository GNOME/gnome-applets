/*
 * Copyright (C) todd kulesza <fflewddur@dropline.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GWEATHER_H
#define GWEATHER_H

#include <glib/gi18n.h>

#include <libgweather/gweather.h>

#include <panel-applet.h>

/* Radar map on by default. */
#define RADARMAP

G_BEGIN_DECLS
 
typedef struct _GWeatherApplet GWeatherApplet;

struct _GWeatherApplet
{
	PanelApplet *applet;
	GWeatherInfo *gweather_info;

	GSettings *lib_settings;
	GSettings *applet_settings;

	GtkWidget *container;
	GtkWidget *box;	
	GtkWidget *label;
	GtkWidget *image;
	
	PanelAppletOrient orient;
	gint size;
	gint timeout_tag;
	gint suncalc_timeout_tag;

	GtkWidget *pref_dialog;

	/* dialog stuff */
	GtkWidget *details_dialog;
};

G_END_DECLS
 
#endif
