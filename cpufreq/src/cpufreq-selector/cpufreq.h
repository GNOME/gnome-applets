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

#ifndef __CPUFREQ_H__
#define __CPUFREQ_H__

#include <glib-object.h>

#define TYPE_CPUFREQ            (cpufreq_get_type ())
#define CPUFREQ(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CPUFREQ, CPUFreq))
#define CPUFREQ_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_CPUFREQ, CPUFreqClass))
#define IS_CPUFREQ(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CPUFREQ))
#define IS_CPUFREQ_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CPUFREQ))
#define CPUFREQ_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CPUFREQ, CPUFreqClass))

typedef struct _CPUFreq      CPUFreq;
typedef struct _CPUFreqClass CPUFreqClass;

struct _CPUFreq {
	   GObject parent;
};

struct _CPUFreqClass {
	   GObjectClass parent_class;

	   void  (* set_governor)  (CPUFreq *cfq, const gchar *governor);
	   void  (* set_frequency) (CPUFreq *cfq, gint frequency);
};


GType  cpufreq_get_type      (void);

void   cpufreq_set_governor  (CPUFreq *cfq, const gchar *governor);
void   cpufreq_set_frequency (CPUFreq *cfq, gint frequency);

#endif /* __CPUFREQ_H__ */
