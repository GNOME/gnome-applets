#ifndef __GWEATHER_PREF_H_
#define __GWEATHER_PREF_H_

/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Preferences dialog
 *
 */

#include "weather.h"
#include "gweather.h"

G_BEGIN_DECLS

extern void gweather_pref_run (GWeatherApplet *gw_applet);

extern void gweather_pref_load (GWeatherApplet *gw_applet);
extern void gweather_pref_save (const gchar *path, GWeatherApplet *gw_applet);

void add_atk_relation (GtkWidget *widget1, GtkWidget *widget2, AtkRelationType type);
void set_access_namedesc (GtkWidget *widget, const gchar *name, const gchar *desc);



G_END_DECLS

#endif /* __GWEATHER_PREF_H_ */

