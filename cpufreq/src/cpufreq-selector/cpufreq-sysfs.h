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

#ifndef __CPUFREQ_SYSFS_H__
#define __CPUFREQ_SYSFS_H__

#include <glib-object.h>

#include "cpufreq.h"

#define TYPE_CPUFREQ_SYSFS            (cpufreq_sysfs_get_type ())
#define CPUFREQ_SYSFS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CPUFREQ_SYSFS, CPUFreqSysfs))
#define CPUFREQ_SYSFS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_CPUFREQ_SYSFS, CPUFreqSysfsClass))
#define IS_CPUFREQ_SYSFS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CPUFREQ_SYSFS))
#define IS_CPUFREQ_SYSFS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CPUFREQ_SYSFS))
#define CPUFREQ_SYSFS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CPUFREQ_SYSFS, CPUFreqSysfsClass))

typedef struct _CPUFreqSysfs      CPUFreqSysfs;
typedef struct _CPUFreqSysfsClass CPUFreqSysfsClass;

struct _CPUFreqSysfs {
	   CPUFreq parent;
};

struct _CPUFreqSysfsClass {
	   CPUFreqClass parent_class;
};


GType         cpufreq_sysfs_get_type ();
CPUFreqSysfs *cpufreq_sysfs_new      ();

#endif /* __CPUFREQ_SYSFS_H__ */
