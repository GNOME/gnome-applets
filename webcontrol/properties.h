/* Webcontrol properties dialog and actions */

#ifndef __PROPERTIES_H_
#define __PROPERTIES_H_

#include <gnome.h>
#include <applet-widget.h>

#include "webcontrol.h"

typedef struct _webcontrol_properties 
{
	gint newwindow;			/* do we launch a new window */
	gint show_url;			/* do we show the url label in front */
	gint show_check;		/* do we show "launch new window" */
        gint width;                     /* horizontal size of applet */
	gint show_clear;                /* show the "Clear" button */
	gint hist_len;                  /* length of URL history */
} webcontrol_properties;

extern void
properties_box (AppletWidget *widget, gpointer data);

extern void 
check_box_toggled (GtkWidget *check, int *data);

#endif






