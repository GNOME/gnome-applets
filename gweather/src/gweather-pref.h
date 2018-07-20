/*
 * Copyright (C) Papadimitriou Spiros <spapadim+@cs.cmu.edu>
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

#ifndef __GWEATHER_PREF_H_
#define __GWEATHER_PREF_H_

#include "gweather-applet.h"

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
