/* 
 * GNOME web browser control module
 * (C) 2000 The Free Software Foundation
 * 
 * based on:
 * GNOME fish module.
 * code snippets from APPLET_WRITING document
 *
 * Authors: Garrett Smith
 *          Rusty Geldmacher
 *
 */

#ifndef __WEBCONTROL_H_
#define __WEBCONTROL_H_

#include <config.h>
#include <gnome.h>
#include <applet-widget.h>
#include <libgnomeui/gnome-window-icon.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "properties.h"

typedef struct _WebControl WebControl;

struct _WebControl 
{
	GtkWidget *applet;
	GtkWidget *input;
	GtkWidget *label;
	GtkWidget *check;
	GtkWidget *clear;
	GtkWidget *go;
};

void draw_applet (void);

void clear_url_history (GtkButton *button, gpointer combo);

#endif
