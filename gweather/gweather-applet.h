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

extern GtkWidget *gweather_applet;
extern WeatherInfo *gweather_info;

extern void gweather_applet_create(int argc, char *argv[]);

extern void gweather_update (void);
extern void gweather_info_load (const gchar *path);
extern void gweather_info_save (const gchar *path);


#endif /* __GWEATHER_APPLET_H_ */

