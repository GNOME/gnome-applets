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

#ifndef __CPUFREQ_MONITOR_H__
#define __CPUFREQ_MONITOR_H__

#include <glib-object.h>

#define TYPE_CPUFREQ_MONITOR            (cpufreq_monitor_get_type ())
#define CPUFREQ_MONITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CPUFREQ_MONITOR, CPUFreqMonitor))
#define CPUFREQ_MONITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_CPUFREQ_MONITOR, CPUFreqMonitorClass))
#define IS_CPUFREQ_MONITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CPUFREQ_MONITOR))
#define IS_CPUFREQ_MONITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CPUFREQ_MONITOR))
#define CPUFREQ_MONITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CPUFREQ_MONITOR, CPUFreqMonitorClass))

enum {
	   CHANGED,
	   LAST
};

typedef struct _CPUFreqMonitor      CPUFreqMonitor;
typedef struct _CPUFreqMonitorClass CPUFreqMonitorClass;

struct _CPUFreqMonitor {
	   GObject parent;
};

struct _CPUFreqMonitorClass {
	   GObjectClass parent_class;

	   /*< public >*/
	   void   (* run)                       (CPUFreqMonitor *monitor);
	   GList *(* get_available_frequencies) (CPUFreqMonitor *monitor);

	   /*< protected >*/
	   gchar *(* get_human_readable_freq)   (gint freq);
	   gchar *(* get_human_readable_unit)   (gint freq);
	   gchar *(* get_human_readable_perc)   (gint fmax, gint fmin);
	   void   (* free_data)                 (CPUFreqMonitor *monitor);

	   /*< signals >*/
	   guint  signals[LAST];
	   void   (* changed)                   (CPUFreqMonitor *monitor);
};

GType  cpufreq_monitor_get_type ();

void   cpufreq_monitor_run                       (CPUFreqMonitor *monitor);
GList *cpufreq_monitor_get_available_frequencies (CPUFreqMonitor *monitor);

guint  cpufreq_monitor_get_cpu                   (CPUFreqMonitor *monitor);
gchar *cpufreq_monitor_get_governor              (CPUFreqMonitor *monitor);
gchar *cpufreq_monitor_get_frequency             (CPUFreqMonitor *monitor);
gchar *cpufreq_monitor_get_percentage            (CPUFreqMonitor *monitor);
gchar *cpufreq_monitor_get_unit                  (CPUFreqMonitor *monitor);
void   cpufreq_monitor_set_cpu                   (CPUFreqMonitor *monitor,
										guint cpu);

#endif /* __CPUFREQ_MONITOR_H__ */
