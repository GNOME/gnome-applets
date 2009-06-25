#ifndef __GWEATHER_DIALOG_H_
#define __GWEATHER_DIALOG_H_

/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Main status dialog
 *
 */

#include <gtk/gtk.h>

#define GWEATHER_I_KNOW_THIS_IS_UNSTABLE

#include "gweather.h"

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

