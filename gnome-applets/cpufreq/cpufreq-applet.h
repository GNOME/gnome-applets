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

#ifndef CPUFREQ_APPLET_H
#define CPUFREQ_APPLET_H

#include <libgnome-panel/gp-applet.h>

G_BEGIN_DECLS

#define CPUFREQ_TYPE_APPLET (cpufreq_applet_get_type ())
G_DECLARE_FINAL_TYPE (CPUFreqApplet, cpufreq_applet, CPUFREQ, APPLET, GpApplet)

#define CPUFREQ_TYPE_SHOW_MODE      (cpufreq_applet_show_mode_get_type ())
#define CPUFREQ_TYPE_SHOW_TEXT_MODE (cpufreq_applet_show_text_mode_get_type ())

typedef enum {
        CPUFREQ_MODE_GRAPHIC,
        CPUFREQ_MODE_TEXT,
        CPUFREQ_MODE_BOTH
} CPUFreqShowMode;

typedef enum {
        CPUFREQ_MODE_TEXT_FREQUENCY,
        CPUFREQ_MODE_TEXT_FREQUENCY_UNIT,
        CPUFREQ_MODE_TEXT_PERCENTAGE
} CPUFreqShowTextMode;

GType    cpufreq_applet_show_mode_get_type      (void) G_GNUC_CONST;
GType    cpufreq_applet_show_text_mode_get_type (void) G_GNUC_CONST;

void     cpufreq_applet_setup_about             (GtkAboutDialog *dialog);

G_END_DECLS

#endif /* CPUFREQ_APPLET_H */
