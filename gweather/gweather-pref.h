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

struct _GWeatherPrefs {
    WeatherLocation *location;
    gint update_interval;  /* in seconds */
    gboolean update_enabled;
    gboolean use_metric;
    gboolean detailed;
    gchar *proxy_url;
    gchar *proxy_user;
    gchar *proxy_passwd;
    gboolean use_proxy;
};

typedef struct _GWeatherPrefs GWeatherPrefs;

extern GWeatherPrefs gweather_pref;

extern void gweather_pref_run (void);

extern void gweather_pref_load (void);
extern void gweather_pref_save (void);

#endif /* __GWEATHER_PREF_H_ */

