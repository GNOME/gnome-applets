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

#ifndef __GWEATHER_PREF_H_
#define __GWEATHER_PREF_H_

#include <gtk/gtk.h>

#define GWEATHER_I_KNOW_THIS_IS_UNSTABLE

#include "gweather.h"

G_BEGIN_DECLS

#define GWEATHER_TYPE_PREF		(gweather_pref_get_type ())
#define GWEATHER_PREF(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GWEATHER_TYPE_PREF, GWeatherPref))
#define GWEATHER_PREF_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GWEATHER_TYPE_PREF, GWeatherPrefClass))
#define GWEATHER_IS_PREF(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GWEATHER_TYPE_PREF))
#define GWEATHER_IS_PREF_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GWEATHER_TYPE_PREF))
#define GWEATHER_PREF_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GWEATHER_TYPE_PREF, GWeatherPrefClass))

typedef struct _GWeatherPref GWeatherPref;
typedef struct _GWeatherPrefPrivate GWeatherPrefPrivate;
typedef struct _GWeatherPrefClass GWeatherPrefClass;

struct _GWeatherPref
{
	GtkDialog parent;

	/* private */
	GWeatherPrefPrivate *priv;
};


struct _GWeatherPrefClass
{
	GtkDialogClass parent_class;
};

GType		 gweather_pref_get_type	(void);
GtkWidget	*gweather_pref_new	(GWeatherApplet *applet);


void set_access_namedesc (GtkWidget *widget, const gchar *name, const gchar *desc);


G_END_DECLS

#endif /* __GWEATHER_PREF_H */

