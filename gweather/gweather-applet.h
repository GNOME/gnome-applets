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

extern void gweather_applet_create(GWeatherApplet *gw_applet);
extern gint timeout_cb (gpointer data);
void update_display (GWeatherApplet *applet);
void place_widgets (GWeatherApplet *gw_applet);
extern void gweather_update (GWeatherApplet *applet);

G_END_DECLS

#endif /* __GWEATHER_APPLET_H_ */

