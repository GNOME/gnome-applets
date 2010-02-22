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

#include <panel-applet.h>

#include <libgweather/gweather-gconf.h>
#include <libgweather/gweather-prefs.h>


/* Radar map on by default. */
#define RADARMAP

G_BEGIN_DECLS
 
typedef struct _GWeatherApplet GWeatherApplet;

struct _GWeatherApplet
{
	PanelApplet *applet;
	WeatherInfo *gweather_info;

	GWeatherGConf *gconf;

	GtkWidget *container;
	GtkWidget *box;	
	GtkWidget *label;
	GtkWidget *image;
	
	PanelAppletOrient orient;
	gint size;
	gint timeout_tag;
	gint suncalc_timeout_tag;

	/* preferences  */
	GWeatherPrefs gweather_pref;

	GtkWidget *pref_dialog;

	/* dialog stuff */
	GtkWidget *details_dialog;
};

G_END_DECLS
 
#endif
