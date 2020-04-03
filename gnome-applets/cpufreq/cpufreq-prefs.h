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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors : Carlos García Campos <carlosgc@gnome.org>
 */

#ifndef CPUFREQ_PREFS_H
#define CPUFREQ_PREFS_H

#include <gdk/gdk.h>
#include <glib-object.h>
#include "cpufreq-applet.h"

G_BEGIN_DECLS

#define CPUFREQ_TYPE_PREFS (cpufreq_prefs_get_type ())
G_DECLARE_FINAL_TYPE (CPUFreqPrefs, cpufreq_prefs, CPUFREQ, PREFS, GObject)

CPUFreqPrefs       *cpufreq_prefs_new                (CPUFreqApplet *applet,
                                                      GSettings     *settings);

guint               cpufreq_prefs_get_cpu            (CPUFreqPrefs *prefs);
CPUFreqShowMode     cpufreq_prefs_get_show_mode      (CPUFreqPrefs *prefs);
CPUFreqShowTextMode cpufreq_prefs_get_show_text_mode (CPUFreqPrefs *prefs);

/* Properties dialog */
void                cpufreq_preferences_dialog_run   (CPUFreqPrefs *prefs,
						      GdkScreen    *screen);

G_END_DECLS

#endif /* CPUFREQ_PREFS_H */
