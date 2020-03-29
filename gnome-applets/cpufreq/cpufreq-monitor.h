/*
 * Copyright (C) 2004 Carlos García Campos
 * Copyright (C) 2016 Alberts Muktupāvels
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Alberts Muktupāvels <alberts.muktupavels@gmail.com>
 *     Carlos García Campos <carlosgc@gnome.org>
 */

#ifndef CPUFREQ_MONITOR_H
#define CPUFREQ_MONITOR_H

#include <glib-object.h>

G_BEGIN_DECLS

#define CPUFREQ_TYPE_MONITOR cpufreq_monitor_get_type ()
G_DECLARE_FINAL_TYPE (CPUFreqMonitor, cpufreq_monitor, CPUFREQ, MONITOR, GObject)

CPUFreqMonitor *cpufreq_monitor_new                       (guint           cpu);

void            cpufreq_monitor_run                       (CPUFreqMonitor *monitor);

GList          *cpufreq_monitor_get_available_frequencies (CPUFreqMonitor *monitor);

GList          *cpufreq_monitor_get_available_governors   (CPUFreqMonitor *monitor);

guint           cpufreq_monitor_get_cpu                   (CPUFreqMonitor *monitor);

void            cpufreq_monitor_set_cpu                   (CPUFreqMonitor *monitor,
                                                           guint           cpu);

const gchar    *cpufreq_monitor_get_governor              (CPUFreqMonitor *monitor);

gint            cpufreq_monitor_get_frequency             (CPUFreqMonitor *monitor);

gint            cpufreq_monitor_get_percentage            (CPUFreqMonitor *monitor);

gboolean        cpufreq_monitor_get_hardware_limits       (CPUFreqMonitor *monitor,
                                                           gulong         *min,
                                                           gulong         *max);

G_END_DECLS

#endif
