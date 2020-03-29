/*
 * Copyright (C) 2004 Carlos Garcia Campos
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

#ifndef CPUFREQ_SELECTOR_H
#define CPUFREQ_SELECTOR_H

#include <glib-object.h>

G_BEGIN_DECLS

#define CPUFREQ_TYPE_SELECTOR cpufreq_selector_get_type ()
G_DECLARE_FINAL_TYPE (CPUFreqSelector, cpufreq_selector,
                      CPUFREQ, SELECTOR, GObject)

CPUFreqSelector *cpufreq_selector_new            (guint             cpu);

gboolean         cpufreq_selector_set_frequency  (CPUFreqSelector  *selector,
                                                  guint             frequency,
                                                  GError          **error);

gboolean         cpufreq_selector_set_governor   (CPUFreqSelector  *selector,
                                                  const gchar      *governor,
                                                  GError          **error);

G_END_DECLS

#endif
