/*
 * GNOME CPUFreq Applet
 * Copyright (C) 2004 Carlos Garcia Campos <carlosgc@gnome.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors : Carlos García Campos <carlosgc@gnome.org>
 */

#ifndef __CPUFREQ_APPLET_H__
#define __CPUFREQ_APPLET_H__

#include <gnome.h>
#include <panel-applet.h>

#include "cpufreq-monitor.h"

#define TYPE_CPUFREQ_APPLET            (cpufreq_applet_get_type ())
#define CPUFREQ_APPLET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CPUFREQ_APPLET, CPUFreqApplet))
#define CPUFREQ_APPLET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_CPUFREQ_APPLET, CPUFreqAppletClass))
#define IS_CPUFREQ_APPLET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CPUFREQ_APPLET))
#define IS_CPUFREQ_APPLET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CPUFREQ_APPLET))
#define CPUFREQ_APPLET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CPUFREQ_APPLET, CPUFreqAppletClass))

typedef struct _CPUFreqApplet      CPUFreqApplet;
typedef struct _CPUFreqAppletClass CPUFreqAppletClass;

typedef enum {
	   MODE_GRAPHIC,
	   MODE_TEXT,
	   MODE_BOTH
} CPUFreqShowMode;

typedef enum {
	   MODE_TEXT_FREQUENCY,
	   MODE_TEXT_FREQUENCY_UNIT,
	   MODE_TEXT_PERCENTAGE
} CPUFreqShowTextMode;

struct _CPUFreqApplet {
	   PanelApplet base;
	   
	   guint mcpu; /* Max cpu number (0 in a single cpu system) */

	   CPUFreqShowMode     show_mode;
	   CPUFreqShowTextMode show_text_mode;

	   CPUFreqMonitor *monitor;

	   PanelAppletOrient orient;
	   gint              size;

	   GtkWidget   *label;
	   GtkWidget   *unit_label;
	   GtkWidget   *pixmap;
	   GtkWidget   *box;
	   GtkWidget   *container;
	   GdkPixbuf   *pixbufs[5];
	   GtkTooltips *tips;

	   GtkWidget *prefs;
	   GtkWidget *popup;
};

struct _CPUFreqAppletClass {
	   PanelAppletClass parent_class;
};

GType cpufreq_applet_get_type ();

void cpufreq_applet_display_error (const gchar *message, 
                                   const gchar *secondary);

#endif /* __CPUFREQ_APPLET_H__ */
