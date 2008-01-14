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

#define GWEATHER_I_KNOW_THIS_IS_UNSTABLE

#include "gweather.h"

G_BEGIN_DECLS

extern void gweather_applet_create(GWeatherApplet *gw_applet);
extern gint timeout_cb (gpointer data);
extern gint suncalc_timeout_cb (gpointer data);
extern void gweather_update (GWeatherApplet *applet);

G_END_DECLS

#endif /* __GWEATHER_APPLET_H_ */

