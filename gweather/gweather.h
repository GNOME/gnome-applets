#ifndef __GWEATHER_H__
#define __GWEATHER_H__

/*
 *  todd kulesza <fflewddur@dropline.net>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 * main header file
 *
 */
#include <glib/gi18n.h>

#include <libgweather/gweather-weather.h>

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
