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

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpufreq-sysfs.h"

#define PARENT_TYPE TYPE_CPUFREQ

#define CPUFREQ_SYSFS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TYPE_CPUFREQ_SYSFS, CPUFreqSysfsPrivate))

static void   cpufreq_sysfs_init          (CPUFreqSysfs *cfq);
static void   cpufreq_sysfs_class_init    (CPUFreqSysfsClass *klass);
static void   cpufreq_sysfs_finalize      (GObject *object);

static gint   cpufreq_sysfs_get_setting   (CPUFreqSysfs *cfq_sysfs, const gchar *setting);
static gchar *cpufreq_sysfs_get_governor  (CPUFreqSysfs *cfq_sysfs);
static gint   cpufreq_sysfs_get_index     (CPUFreqSysfs *cfq_sysfs, gint frequency);

static GList *cpufreq_sysfs_get_govs      (CPUFreqSysfs *cfq_sysfs);
static GList *cpufreq_sysfs_get_freqs     (CPUFreqSysfs *cfq_sysfs);

static void   cpufreq_sysfs_set_governor  (CPUFreq *cfq, const gchar *governor);
static void   cpufreq_sysfs_set_frequency (CPUFreq *cfq, gint frequency);

static GObjectClass *parent_class = NULL;

typedef struct _CPUFreqSysfsPrivate  CPUFreqSysfsPrivate;

struct _CPUFreqSysfsPrivate
{
	   gchar *base_path;
	   gint   cpu_max;
	   gint   cpu_min;
	   GList *available_freqs;
	   GList *available_govs;
};

GType cpufreq_sysfs_get_type (void)
{
	   static GType type = 0;

	   if (!type) {
			 static const GTypeInfo info = {
				    sizeof (CPUFreqSysfsClass),
				    (GBaseInitFunc) NULL,
				    (GBaseFinalizeFunc) NULL,
				    (GClassInitFunc) cpufreq_sysfs_class_init,
				    NULL,
				    NULL,
				    sizeof (CPUFreqSysfs),
				    0,
				    (GInstanceInitFunc) cpufreq_sysfs_init
			 };

			 type = g_type_register_static (PARENT_TYPE, "CPUFreqSysfs",
									  &info, 0);
	   }

	   return type;
}

static void
cpufreq_sysfs_init (CPUFreqSysfs *cfq)
{
	   CPUFreqSysfsPrivate *private;
	   gchar               *governor;
	   guint                cpu;
	   gint                 sc_max, sc_min;
	   
	   g_return_if_fail (IS_CPUFREQ_SYSFS (cfq));

	   g_object_get (G_OBJECT (cfq), "n_cpu", &cpu, NULL);
	   if (cpu < 0)
			 cpu = 0;

	   private = CPUFREQ_SYSFS_GET_PRIVATE (cfq);
	   private->base_path = g_strdup_printf ("/sys/devices/system/cpu/cpu%d/cpufreq/", cpu);
	   
	   private->cpu_max = cpufreq_sysfs_get_setting (cfq, "cpuinfo_max_freq");
	   private->cpu_min = cpufreq_sysfs_get_setting (cfq, "cpuinfo_min_freq");

	   private->available_freqs = cpufreq_sysfs_get_freqs (cfq);
	   private->available_govs  = cpufreq_sysfs_get_govs  (cfq);

	   sc_max = cpufreq_sysfs_get_setting (cfq, "scaling_max_freq");
	   sc_min = cpufreq_sysfs_get_setting (cfq, "scaling_min_freq");
	   
	   governor = cpufreq_sysfs_get_governor (cfq);
	   g_object_set (G_OBJECT (cfq), "sc_max", sc_max, "sc_min", sc_min,
				  "governor", governor, NULL);
	   g_free (governor);
}

static void
cpufreq_sysfs_class_init (CPUFreqSysfsClass *klass)
{
	   GObjectClass *object_class = G_OBJECT_CLASS (klass);
	   CPUFreqClass *cfq_class = CPUFREQ_CLASS (klass);

	   parent_class = g_type_class_peek_parent (klass);

	   g_type_class_add_private (klass, sizeof (CPUFreqSysfsPrivate));

	   cfq_class->set_governor = cpufreq_sysfs_set_governor;
	   cfq_class->set_frequency = cpufreq_sysfs_set_frequency;

	   object_class->finalize = cpufreq_sysfs_finalize;
}

static void
free_string (gpointer str, gpointer gdata)
{
	   if (str) g_free (str);
}

static void
cpufreq_sysfs_finalize (GObject *object)
{
	   CPUFreqSysfsPrivate *private;

	   g_return_if_fail (IS_CPUFREQ_SYSFS (object));

	   private = CPUFREQ_SYSFS_GET_PRIVATE (object);

	   if (private->base_path) {
			 g_free (private->base_path);
			 private->base_path = NULL;
	   }
			 
	   if (private->available_freqs) {
			 g_list_foreach (private->available_freqs,
						  free_string, NULL);
			 g_list_free (private->available_freqs);
			 private->available_freqs = NULL;
	   }
	   
	   if (private->available_govs) {
			 g_list_foreach (private->available_govs,
						  free_string, NULL);
			 g_list_free (private->available_govs);
			 private->available_govs = NULL;
	   }
	   
	   if (G_OBJECT_CLASS (parent_class)->finalize)
			 (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

CPUFreqSysfs *
cpufreq_sysfs_new (void)
{
	   CPUFreqSysfs *cfq;

	   cfq = g_object_new (TYPE_CPUFREQ_SYSFS, NULL);

	   return cfq;
}

static gint
cpufreq_sysfs_get_setting (CPUFreqSysfs *cfq_sysfs, const gchar *setting)
{
	   FILE                *fd;
	   gchar                buf[50];
	   gchar               *path = NULL;
	   gchar               *str = NULL;
	   CPUFreqSysfsPrivate *private;

	   private = CPUFREQ_SYSFS_GET_PRIVATE (cfq_sysfs);
	   
	   path = g_build_filename (private->base_path, setting, NULL);

	   if ((fd = fopen (path, "r")) != NULL) {
			 if (fgets (buf, 50, fd) != NULL) {
				    str = g_strchomp (buf);
			 }

			 fclose (fd);
	   }

	   g_free (path);

	   if (str)
			 return atoi (str);
	   else
			 return 0;
}

static gchar *
cpufreq_sysfs_get_governor (CPUFreqSysfs *cfq_sysfs)
{
	   FILE                *fd;
	   gchar                buf[50];
	   gchar               *str = NULL;
	   gchar               *path;
	   CPUFreqSysfsPrivate *private;

	   private = CPUFREQ_SYSFS_GET_PRIVATE (cfq_sysfs);

	   path = g_build_filename (private->base_path, "scaling_governor", NULL);

	   if ((fd = fopen (path, "r")) != NULL) {
			 if (fgets (buf, 50, fd) != NULL) {
				    str = g_strchomp (buf);
			 }

			 fclose (fd);
	   }

	   g_free (path);

	   return g_strdup (str);
}

static GList *
cpufreq_sysfs_get_govs (CPUFreqSysfs *cfq_sysfs)
{
	   FILE                *fd;
	   gchar                buf[50];
	   gchar               *str;
	   GList               *list = NULL;
	   gchar               **governors = NULL;
	   gint                 i;
	   gchar               *path;
	   CPUFreqSysfsPrivate *private;

	   private = CPUFREQ_SYSFS_GET_PRIVATE (cfq_sysfs);

	   path = g_build_filename (private->base_path, "scaling_available_governors", NULL);

	   if ((fd = fopen (path, "r")) != NULL) {
			 if (fgets (buf, 50, fd) != NULL) {
				    str = g_strchomp (buf);
				    governors = g_strsplit (str, " ", 4);
			 }

			 fclose (fd);
	   }

	   g_free (path);

	   i = 0;
	   while (governors[i] != NULL) {
			 list = g_list_append (list, g_strdup (governors[i]));
			 i++;
	   }

	   g_strfreev (governors);

	   return list;
}

static gint
compare_int (gconstpointer a, gconstpointer b)
{
	   gint aa, bb;

	   aa = atoi ((gchar *) a);
	   bb = atoi ((gchar *) b);

	   if (aa == bb)
			 return 0;
	   else if (aa > bb)
			 return -1;
	   else
			 return 1;
}

static GList *
cpufreq_sysfs_get_freqs (CPUFreqSysfs *cfq_sysfs)
{
	   FILE                *fd;
	   gchar                buf[256];
	   gchar               *str;
	   GList               *list = NULL;
	   gchar               **frequencies = NULL;
	   gint                 i;
	   gchar               *path;
	   CPUFreqSysfsPrivate *private;

	   private = CPUFREQ_SYSFS_GET_PRIVATE (cfq_sysfs);

	   path = g_build_filename (private->base_path, "scaling_available_frequencies", NULL);

	   if ((fd = fopen (path, "r")) != NULL) {
			 if (fgets (buf, 256, fd) != NULL) {
				    str = g_strchomp (buf);
				    frequencies = g_strsplit (str, " ", 0); 
			 }

			 fclose (fd);
	   }

	   g_free (path);

	   i = 0;
	   while (frequencies[i] != NULL) {
			 if (!g_list_find_custom (list, frequencies[i], compare_int))
				    list = g_list_prepend (list, g_strdup (frequencies[i]));
			 i++;
	   }

	   g_strfreev (frequencies);

	   list = g_list_sort (list, compare_int);
	   
	   return list;
}

static gint
compare_string (gconstpointer a, gconstpointer b)
{
	   return (g_ascii_strcasecmp (a, b));
}

static void
cpufreq_sysfs_set_governor  (CPUFreq *cfq, const gchar *governor)
{
	   FILE                *fd;
	   gchar               *path;
	   CPUFreqSysfsPrivate *private;

	   g_return_if_fail (IS_CPUFREQ_SYSFS (cfq));
	   
	   private = CPUFREQ_SYSFS_GET_PRIVATE (cfq);

	   path = g_build_filename (private->base_path, "scaling_governor", NULL);

	   if ((fd = fopen (path, "w")) != NULL) {
			 if (!g_list_find_custom (private->available_govs,
								 governor, compare_string)) {
				    g_print ("Invalid governor %s\n", governor);

				    g_free (path);
				    fclose (fd);
				    return;
			 }
			 
			 if (fputs (governor, fd) < 0) {
				    g_print ("Failed to set the governor\n");
			 }

			 fclose (fd);
	   }

	   g_free (path);
}

static gint
cpufreq_sysfs_get_index (CPUFreqSysfs *cfq_sysfs, gint frequency)
{
	   GList               *list = NULL;
	   gint                 i, index = 0;
	   gint                 dist = G_MAXINT;
	   gint                 freq;
	   CPUFreqSysfsPrivate *private;

	   private = CPUFREQ_SYSFS_GET_PRIVATE (cfq_sysfs);

	   if (private->available_freqs) {
			 list = g_list_first (private->available_freqs);

			 index = 0;
			 i = 0;
			 while (list) {
				    freq = atoi (list->data);
				    if (freq == frequency) {
						  return i;
				    }
				    
				    if (abs (frequency - freq) < dist) {
						  dist = abs (frequency - freq);
						  index = i;
				    }
				    list = g_list_next (list);
				    i++;
			 }
	   }

	   return index;
}

static void
cpufreq_sysfs_set_frequency (CPUFreq *cfq, gint frequency)
{
	   FILE                *fd;
	   gint                 index, len;
	   gchar               *governor, *sc_max, *sc_min;
	   gchar               *setspeed;
	   gchar               *path;
	   CPUFreqSysfs        *cfq_sysfs;
	   CPUFreqSysfsPrivate *private;

	   g_return_if_fail (IS_CPUFREQ_SYSFS (cfq));
	   
	   private = CPUFREQ_SYSFS_GET_PRIVATE (cfq);
	   
	   cfq_sysfs = CPUFREQ_SYSFS (cfq);

	   g_object_get (G_OBJECT (cfq), "governor", &governor, NULL);
	   
	   if ((g_ascii_strcasecmp (governor, "performance") != 0) &&
		  (g_ascii_strcasecmp (governor, "powersave") != 0)) {
			 if (g_ascii_strcasecmp (governor, "userspace") == 0)
				    path = g_build_filename (private->base_path, "scaling_setspeed", NULL);
			 else
				    path = g_build_filename (private->base_path, "scaling_cur_freq", NULL);

			 if (g_file_test (path, G_FILE_TEST_EXISTS)) {
				    index = cpufreq_sysfs_get_index (cfq_sysfs, frequency);
				    setspeed = g_strdup (g_list_nth_data (private->available_freqs,
												  index));
				    if ((fd = fopen (path, "w")) != NULL) {
						  if (fputs (setspeed, fd) < 0) {
								g_print ("Failed to set the frequency\n");
						  }

						  fclose (fd);
				    }

				    g_free (path);
				    g_free (setspeed);
				    g_free (governor);

				    return;
			 }

			 g_free (path);
	   }

	   g_free (governor);
	   governor = NULL;

	   if (frequency >= private->cpu_max) {
			 sc_max = g_strdup_printf ("%d", private->cpu_max);
			 sc_min = g_strdup_printf ("%d", private->cpu_min);
			 governor = g_strdup ("performance");
	   } else if (frequency <= private->cpu_min) {
			 sc_max = g_strdup_printf ("%d", private->cpu_max);
			 sc_min = g_strdup_printf ("%d", private->cpu_min);
			 governor = g_strdup ("powersave");
	   } else {
			 index = cpufreq_sysfs_get_index (cfq_sysfs, frequency);
			 len = g_list_length (private->available_freqs);

			 if (index < (len / 2)) {
				    sc_max = g_strdup (g_list_nth_data (private->available_freqs,
												index));
				    sc_min = g_strdup_printf ("%d", private->cpu_min);
				    governor = g_strdup ("performance");
			 } else {
				    sc_max = g_strdup_printf ("%d", private->cpu_max);
				    sc_min = g_strdup (g_list_nth_data (private->available_freqs,
												index));
				    governor = g_strdup ("powersave");
			 }
	   }

	   path = g_build_filename (private->base_path, "scaling_max_freq", NULL);

	   if ((fd = fopen (path, "w")) != NULL) {
			 if (fputs (sc_max, fd) < 0) {
				    g_print ("Failed to set the frequency\n");
			 }
			 
			 fclose (fd);
	   }

	   g_free (path);
	   path = g_build_filename (private->base_path, "scaling_min_freq", NULL);
	   
	   if ((fd = fopen (path, "w")) != NULL) {
			 if (fputs (sc_min, fd) < 0) {
				    g_print ("Failed to set the frequency\n");
			 }
			 
			 fclose (fd);
	   }

	   g_free (path);

	   cpufreq_sysfs_set_governor (cfq, governor);
	   
	   g_free (sc_max);
	   g_free (sc_min);
	   g_free (governor);
}

