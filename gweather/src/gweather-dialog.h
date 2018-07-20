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

#ifndef __GWEATHER_DIALOG_H_
#define __GWEATHER_DIALOG_H_

#include "gweather-applet.h"

G_BEGIN_DECLS

#define GWEATHER_TYPE_DIALOG		(gweather_dialog_get_type ())
#define GWEATHER_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GWEATHER_TYPE_DIALOG, GWeatherDialog))
#define GWEATHER_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), GWEATHER_TYPE_DIALOG, GWeatherDialogClass))
#define GWEATHER_IS_DIALOG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GWEATHER_TYPE_DIALOG))
#define GWEATHER_IS_DIALOG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GWEATHER_TYPE_DIALOG))
#define GWEATHER_DIALOG_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GWEATHER_TYPE_DIALOG, GWeatherDialogClass))

typedef struct _GWeatherDialog GWeatherDialog;
typedef struct _GWeatherDialogPrivate GWeatherDialogPrivate;
typedef struct _GWeatherDialogClass GWeatherDialogClass;

struct _GWeatherDialog
{
	GtkDialog parent;

	/* private */
	GWeatherDialogPrivate *priv;
};


struct _GWeatherDialogClass
{
	GtkDialogClass parent_class;
};

GType		 gweather_dialog_get_type	(void);
GtkWidget	*gweather_dialog_new		(GWeatherApplet *applet);
void		 gweather_dialog_update		(GWeatherDialog *dialog);

G_END_DECLS

#endif /* __GWEATHER_DIALOG_H_ */

