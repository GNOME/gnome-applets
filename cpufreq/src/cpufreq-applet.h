/*
 * Copyright (C) 2001, 2002 Free Software Foundation
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


#include <gnome.h>
#include <panel-applet.h>

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

typedef enum {
	   IFACE_SYSFS,
	   IFACE_PROCFS,
	   IFACE_CPUINFO
} CPUFreqIface;

typedef struct {
	   PanelApplet base;
	   guint cpu;
	   guint mcpu; /* Max cpu number (0 in a single cpu system) */

	   CPUFreqShowMode     show_mode;
	   CPUFreqShowTextMode show_text_mode;
	   CPUFreqIface        iface;

	   PanelAppletOrient orient;
	   gint              size;

	   gchar *freq;
	   gchar *perc;
	   gchar *unit;
	   gchar *governor;
	   GList *available_freqs;

	   guint timeout_handler;

	   GtkWidget   *label;
	   GtkWidget   *unit_label;
	   GtkWidget   *pixmap;
	   GtkWidget   *box;
	   GtkWidget   *container;
	   GdkPixbuf   *pixbufs[5];
	   GtkTooltips *tips;

	   GtkWidget *prefs;
	   GtkWidget *about_dialog;
	   GtkWidget *popup;
} CPUFreqApplet;

void cpufreq_applet_update        (CPUFreqApplet *applet);
void cpufreq_applet_display_error (const gchar *error_message);
void cpufreq_applet_run           (CPUFreqApplet *applet);
