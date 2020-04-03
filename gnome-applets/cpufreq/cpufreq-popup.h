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

#ifndef CPUFREQ_POPUP_H
#define CPUFREQ_POPUP_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include "cpufreq-monitor.h"

G_BEGIN_DECLS

#define CPUFREQ_TYPE_POPUP (cpufreq_popup_get_type ())
G_DECLARE_FINAL_TYPE (CPUFreqPopup, cpufreq_popup, CPUFREQ, POPUP, GObject)

CPUFreqPopup *cpufreq_popup_new             (void);

void          cpufreq_popup_set_monitor     (CPUFreqPopup   *popup,
					     CPUFreqMonitor *monitor);
GtkWidget    *cpufreq_popup_get_menu        (CPUFreqPopup   *popup);

G_END_DECLS

#endif /* CPUFREQ_POPUP_H */
