/*
 * Copyright (C) 2020 Alberts Muktupāvels
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
 */

#ifndef CPUFREQ_SELECTOR_SERVICE_H
#define CPUFREQ_SELECTOR_SERVICE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define CPUFREQ_TYPE_SELECTOR_SERVICE (cpufreq_selector_service_get_type ())
G_DECLARE_FINAL_TYPE (CPUFreqSelectorService, cpufreq_selector_service,
                      CPUFREQ, SELECTOR_SERVICE, GObject)

CPUFreqSelectorService *cpufreq_selector_service_new (void);

G_END_DECLS

#endif
