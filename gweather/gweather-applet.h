#ifndef __GWEATHER_APPLET_H_
#define __GWEATHER_APPLET_H_

/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main applet widget
 *
 */

#include "weather.h"
#include "gweather.h"

G_BEGIN_DECLS
/*
extern GtkWidget *gweather_applet;
extern WeatherInfo *gweather_info;
*/

extern void gweather_applet_create(GWeatherApplet *gw_applet);

extern void gweather_update (GWeatherApplet *applet);
extern void gweather_info_load (const gchar *path, GWeatherApplet *applet);
extern void gweather_info_save (const gchar *path, GWeatherApplet *applet);

G_END_DECLS

#endif /* __GWEATHER_APPLET_H_ */

