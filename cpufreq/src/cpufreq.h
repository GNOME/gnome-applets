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

gchar   *cpufreq_get_human_readble_freq      (gint freq);
gchar   *cpufreq_get_human_readble_unit      (gint freq);
gchar   *cpufreq_get_human_readble_perc      (gint fmax, gint fmin);

gboolean cpufreq_get_from_procfs             (gpointer gdata);
gboolean cpufreq_get_from_sysfs              (gpointer gdata);
GList   *cpufreq_get_frequencies_from_procfs (CPUFreqApplet *applet);
GList   *cpufreq_get_frequencies_from_sysfs  (CPUFreqApplet *applet);
gboolean cpufreq_get_from_procfs_cpuinfo     (CPUFreqApplet *applet);
