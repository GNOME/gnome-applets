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
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Authors : Carlos García Campos <carlosgc@gnome.org>
 */

#ifndef __CPUFREQ_PROCFS_H__
#define __CPUFREQ_PROCFS_H__

#include <glib-object.h>

#include "cpufreq.h"

#define TYPE_CPUFREQ_PROCFS            (cpufreq_procfs_get_type ())
#define CPUFREQ_PROCFS(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CPUFREQ_PROCFS, CPUFreqProcfs))
#define CPUFREQ_PROCFS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_CPUFREQ_PROCFS, CPUFreqProcfsClass))
#define IS_CPUFREQ_PROCFS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CPUFREQ_PROCFS))
#define IS_CPUFREQ_PROCFS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CPUFREQ_PROCFS))
#define CPUFREQ_PROCFS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CPUFREQ_PROCFS, CPUFreqProcfsClass))

typedef struct _CPUFreqProcfs      CPUFreqProcfs;
typedef struct _CPUFreqProcfsClass CPUFreqProcfsClass;
typedef struct _CPUFreqProcfsPriv  CPUFreqProcfsPriv;

struct _CPUFreqProcfs {
        CPUFreq parent;
};

struct _CPUFreqProcfsClass {
        CPUFreqClass parent_class;
};


GType          cpufreq_procfs_get_type (void) G_GNUC_CONST;
CPUFreqProcfs *cpufreq_procfs_new      (void);

#endif /* __CPUFREQ_PROCFS_H__ */
