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

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpufreq-procfs.h"

#define PARENT_TYPE TYPE_CPUFREQ

#define CPUFREQ_PROCFS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TYPE_CPUFREQ_PROCFS, CPUFreqProcfsPrivate))

static void cpufreq_procfs_init           (CPUFreqProcfs *cfq);
static void cpufreq_procfs_class_init     (CPUFreqProcfsClass *klass);
static void cpufreq_procfs_finalize       (GObject *object);

static void  cpufreq_procfs_set_governor  (CPUFreq *cfq, const gchar *governor);
static void  cpufreq_procfs_set_frequency (CPUFreq *cfq, gint frequency);

static void  cpufreq_procfs_setup         (CPUFreqProcfs *cfq);

static GObjectClass *parent_class = NULL;

typedef struct _CPUFreqProcfsPrivate CPUFreqProcfsPrivate;

struct _CPUFreqProcfsPrivate
{
	   gint pmax;
};

GType cpufreq_procfs_get_type ()
{
	   static GType type = 0;

	   if (!type) {
			 static const GTypeInfo info = {
				    sizeof (CPUFreqProcfsClass),
				    (GBaseInitFunc) NULL,
				    (GBaseFinalizeFunc) NULL,
				    (GClassInitFunc) cpufreq_procfs_class_init,
				    NULL,
				    NULL,
				    sizeof (CPUFreqProcfs),
				    0,
				    (GInstanceInitFunc) cpufreq_procfs_init
			 };

			 type = g_type_register_static (PARENT_TYPE, "CPUFreqProcfs",
									  &info, 0);
	   }

	   return type;
}

static void
cpufreq_procfs_init (CPUFreqProcfs *cfq)
{
	   CPUFreqProcfsPrivate *private;
	   
	   g_return_if_fail (IS_CPUFREQ_PROCFS (cfq));

	   private = CPUFREQ_PROCFS_GET_PRIVATE (cfq);
}

static void
cpufreq_procfs_class_init (CPUFreqProcfsClass *klass)
{
	   GObjectClass *object_class = G_OBJECT_CLASS (klass);
	   CPUFreqClass *cfq_class = CPUFREQ_CLASS (klass);

	   parent_class = g_type_class_peek_parent (klass);

	   g_type_class_add_private (klass, sizeof (CPUFreqProcfsPrivate));

	   cfq_class->set_governor = cpufreq_procfs_set_governor;
	   cfq_class->set_frequency = cpufreq_procfs_set_frequency;

	   object_class->finalize = cpufreq_procfs_finalize;
}

static void
cpufreq_procfs_finalize (GObject *object)
{
	   if (G_OBJECT_CLASS (parent_class)->finalize)
			 (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

CPUFreqProcfs *
cpufreq_procfs_new ()
{
	   CPUFreqProcfs *cfq;

	   cfq = g_object_new (TYPE_CPUFREQ_PROCFS, NULL);

	   cpufreq_procfs_setup (cfq);

	   return cfq;
}

static void
cpufreq_procfs_set_governor (CPUFreq *cfq, const gchar *governor)
{
	   FILE  *fd;
	   gchar *str;
	   guint  cpu;
	   gint   sc_max, sc_min;

	   g_return_if_fail (IS_CPUFREQ_PROCFS (cfq));

	   g_object_get (G_OBJECT (cfq), "n_cpu", &cpu,
				  "sc_max", &sc_max, "sc_min", &sc_min, NULL);

	   str = g_strdup_printf ("%d:%d:%d:%s", cpu, sc_min, sc_max, governor);
	   
	   if ((fd = fopen ("/proc/cpufreq", "w")) != NULL) {
			 if (fputs (str, fd) < 0) {
				    g_print ("Failed to set the governor\n");
			 }

			 fclose (fd);
	   }

	   g_free (str);
}

static void
cpufreq_procfs_set_frequency (CPUFreq *cfq, gint frequency)
{
	   FILE                 *fd;
	   gchar                *str, *path;
	   guint                 cpu;
	   gint                  sc_max, sc_min;
	   gint                  cpu_max;
	   gchar                *governor = NULL;
	   CPUFreqProcfsPrivate *private;

	   g_return_if_fail (IS_CPUFREQ_PROCFS (cfq));

	   g_object_get (G_OBJECT (cfq), "n_cpu", &cpu,
				  "sc_max", &sc_max, "sc_min", &sc_min,
				  "governor", &governor, NULL);

	   if (governor && (g_ascii_strcasecmp (governor, "userspace") == 0)) {
			 path = g_strdup_printf ("/proc/sys/cpu/%d/speed", cpu);
			 if ((fd = fopen (path, "w")) != NULL) {
				    str = g_strdup_printf ("%d", frequency);
				    if (fputs (str, fd) < 0) {
						  g_print ("Failed to set the governor\n");
				    }
				    g_free (str);

				    fclose (fd);
			 }
			 g_free (path);
			 g_free (governor);

			 return;
			 
	   }

	   private = CPUFREQ_PROCFS_GET_PRIVATE (cfq);

	   if (private->pmax == 100)
			 cpu_max = sc_max;
	   else
			 cpu_max = (sc_max * 100) / private->pmax;
	   
	   if (frequency >= cpu_max)
			 str = g_strdup_printf ("%d:%d:%d:performance", cpu, sc_min, cpu_max);
	   else if (frequency <= sc_min)
			 str = g_strdup_printf ("%d:%d:%d:powersave", cpu, frequency, sc_max);
	   else {
			 if (abs (cpu_max - frequency) < abs (sc_min - frequency))
				    str = g_strdup_printf ("%d:%d:%d:performance", cpu, sc_min, frequency);
			 else
				    str = g_strdup_printf ("%d:%d:%d:powersave", cpu, frequency, cpu_max);
	   }

	   if ((fd = fopen ("/proc/cpufreq", "w")) != NULL) {
			 if (fputs (str, fd) < 0) {
				    g_print ("Failed to set the governor\n");
			 }

			 fclose (fd);
	   }

	   g_free (str);
}
	

static void
cpufreq_procfs_setup (CPUFreqProcfs *cfq)
{
	   FILE                 *fd;
	   gchar                 buf[256];
	   guint                 cpu;
	   gint                  fmax, fmin;
	   gint                  pmin, pmax;
	   gchar                 mode[21];
	   CPUFreqProcfsPrivate *private;

	   g_return_if_fail (IS_CPUFREQ_PROCFS (cfq));

	   private = CPUFREQ_PROCFS_GET_PRIVATE (cfq);

	   if ((fd = fopen ("/proc/cpufreq", "r")) != NULL) {
			 while (fgets (buf, 256, fd) != NULL) {
				    if (g_ascii_strncasecmp (buf, "CPU  0", 6) == 0) {
						  sscanf (buf, "CPU %d %d kHz (%d %%) - %d kHz (%d %%) - %20s",
								&cpu, &fmin, &pmin, &fmax, &pmax, mode);

						  private->pmax = pmax;
						  g_object_set (G_OBJECT (cfq), "n_cpu", cpu, 
									 "sc_max", fmax, "sc_min", fmin,
									 "governor", mode, NULL);
						  
				    }
			 }

			 fclose (fd);
	   }
}
