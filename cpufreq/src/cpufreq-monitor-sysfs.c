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
#include <glib/gi18n.h>
#include <libgnomevfs/gnome-vfs.h>

#include <string.h>

#include "cpufreq-monitor-sysfs.h"
#include "cpufreq-monitor-protected.h"

#define PARENT_TYPE TYPE_CPUFREQ_MONITOR

#define CPUFREQ_MONITOR_GET_PROTECTED(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PARENT_TYPE, CPUFreqMonitorProtected))

static void     cpufreq_monitor_sysfs_class_init                (CPUFreqMonitorSysfsClass *klass);
static void     cpufreq_monitor_sysfs_finalize                  (GObject *object);

static void     cpufreq_monitor_sysfs_run                       (CPUFreqMonitor *monitor);
static GList   *cpufreq_monitor_sysfs_get_available_frequencies (CPUFreqMonitor *monitor);

static gboolean cpufreq_monitor_sysfs_get                       (gpointer gdata);


static CPUFreqMonitorClass *parent_class = NULL;

typedef struct _CPUFreqMonitorProtected CPUFreqMonitorProtected;

GType cpufreq_monitor_sysfs_get_type ()
{
	   static GType type = 0;

	   if (!type) {
			 static const GTypeInfo info = {
				    sizeof (CPUFreqMonitorSysfsClass),
				    (GBaseInitFunc) NULL,
				    (GBaseFinalizeFunc) NULL,
				    (GClassInitFunc) cpufreq_monitor_sysfs_class_init,
				    NULL,
				    NULL,
				    sizeof (CPUFreqMonitorSysfs),
				    0,
				    NULL
			 };

			 type = g_type_register_static (PARENT_TYPE, "CPUFreqMonitorSysfs",
									  &info, 0);
	   }

	   return type;
}

static void
cpufreq_monitor_sysfs_class_init (CPUFreqMonitorSysfsClass *klass)
{
	   GObjectClass        *object_class = G_OBJECT_CLASS (klass);
	   CPUFreqMonitorClass *monitor_class = CPUFREQ_MONITOR_CLASS (klass);

	   parent_class = g_type_class_peek_parent (klass);

	   monitor_class->run = cpufreq_monitor_sysfs_run;
	   monitor_class->get_available_frequencies = cpufreq_monitor_sysfs_get_available_frequencies;
	   
	   object_class->finalize = cpufreq_monitor_sysfs_finalize;
}

static void
cpufreq_monitor_sysfs_finalize (GObject *object)
{
	   g_return_if_fail (IS_CPUFREQ_MONITOR_SYSFS (object));

	   if (G_OBJECT_CLASS (parent_class)->finalize)
			 (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

CPUFreqMonitor *
cpufreq_monitor_sysfs_new (guint cpu)
{
	   CPUFreqMonitorSysfs *monitor;

	   monitor = g_object_new (TYPE_CPUFREQ_MONITOR_SYSFS, "cpu", cpu, NULL);

	   return CPUFREQ_MONITOR (monitor);
}

static gboolean
cpufreq_monitor_sysfs_get (gpointer gdata)
{
	   GnomeVFSHandle          *handle;
	   GnomeVFSFileSize         bytes_read;
	   GnomeVFSResult           result;
	   gchar                   *uri;
	   gchar                    buffer[20];
	   gint                     i;
	   gchar                  **cpufreq_data;
	   gchar                   *path;
	   gchar                   *freq, *perc, *unit, *governor;
	   gboolean                 changed;
	   CPUFreqMonitorSysfs     *monitor;
	   CPUFreqMonitorProtected *private;
	   gchar                   *files[] = {
			 "scaling_max_freq",
			 "scaling_min_freq",
			 "scaling_governor",
			 "cpuinfo_max_freq",
			 "scaling_setspeed",
			 "scaling_cur_freq",
			 NULL };

	   enum {
			 SCALING_MAX,
			 SCALING_MIN,
			 GOVERNOR,
			 CPUINFO_MAX,
			 SCALING_SETSPEED,
			 SCALING_CUR_FREQ,
			 LAST
	   };

	   monitor = (CPUFreqMonitorSysfs *) gdata;

	   private = CPUFREQ_MONITOR_GET_PROTECTED (CPUFREQ_MONITOR (monitor));

	   /* /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_max_freq
	    * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_min_freq
	    * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_governor
	    * /sys/devices/system/cpu/cpu[0]/cpufreq/cpuinfo_max_freq
	    * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_setspeed (userspace)
	    * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_cur_freq (new governors)
	    */

	   cpufreq_data = g_new (gchar *, LAST + 1);
	   
	   for (i = 0; i < LAST; i++) {
			 cpufreq_data[i] = NULL;

			 path = g_strdup_printf ("/sys/devices/system/cpu/cpu%d/cpufreq/%s",
								private->cpu, files[i]);
			 
			 if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
				    cpufreq_data[i] = g_strdup ("");
				    buffer[0] = '\0';
				    g_free (path);
				    
				    continue;
			 }

			 uri = gnome_vfs_get_uri_from_local_path (path);
			 g_free (path);
			 
			 result = gnome_vfs_open (&handle, uri, GNOME_VFS_OPEN_READ);
			 if (result != GNOME_VFS_OK) {
				    if (uri) g_free (uri);
				    g_strfreev (cpufreq_data);
				    
				    return FALSE;
			 }

			 g_free (uri);

			 result = gnome_vfs_read (handle, buffer, sizeof (buffer), &bytes_read);
			 
			 /* bytes_read - 1 in order to remove the \n character */
			 cpufreq_data[i] = g_strndup (buffer, bytes_read - 1);

			 if (result != GNOME_VFS_OK) {
				    g_strfreev (cpufreq_data);
				    gnome_vfs_close (handle);
				    
				    return FALSE;
			 }

			 result = gnome_vfs_close (handle);
			 if (result != GNOME_VFS_OK) {
				    g_strfreev (cpufreq_data);
				    
				    return FALSE;
			 }
			 buffer[0] = '\0';
	   }
	   cpufreq_data[LAST] = NULL;

	   governor = g_strdup (cpufreq_data[GOVERNOR]);
	   
	   if (g_ascii_strcasecmp (governor, "userspace") == 0) {
			 freq = parent_class->get_human_readable_freq (atoi (cpufreq_data[SCALING_SETSPEED]));
			 perc = parent_class->get_human_readable_perc (atoi (cpufreq_data[CPUINFO_MAX]),
											atoi (cpufreq_data[SCALING_SETSPEED]));
			 unit = parent_class->get_human_readable_unit (atoi (cpufreq_data[SCALING_SETSPEED]));
	   } else if (g_ascii_strcasecmp (governor, "powersave") == 0) {
			 freq = parent_class->get_human_readable_freq (atoi (cpufreq_data[SCALING_MIN]));
			 perc = parent_class->get_human_readable_perc (atoi (cpufreq_data[CPUINFO_MAX]),
											atoi (cpufreq_data[SCALING_MIN]));
			 unit = parent_class->get_human_readable_unit (atoi (cpufreq_data[SCALING_MIN]));
	   } else if (g_ascii_strcasecmp (governor, "performance") == 0) {
			 freq = parent_class->get_human_readable_freq (atoi (cpufreq_data[SCALING_MAX]));
			 perc = parent_class->get_human_readable_perc (atoi (cpufreq_data[CPUINFO_MAX]),
											atoi (cpufreq_data[SCALING_MAX]));
			 unit = parent_class->get_human_readable_unit (atoi (cpufreq_data[SCALING_MAX]));
	   } else {
			 freq = parent_class->get_human_readable_freq (atoi (cpufreq_data[SCALING_CUR_FREQ]));
			 perc = parent_class->get_human_readable_perc (atoi (cpufreq_data[CPUINFO_MAX]),
											atoi (cpufreq_data[SCALING_CUR_FREQ]));
			 unit = parent_class->get_human_readable_unit (atoi (cpufreq_data[SCALING_CUR_FREQ]));
	   }
	   
	   g_strfreev (cpufreq_data);
	   
	   changed = FALSE;
	   
	   if (!private->governor || (g_ascii_strcasecmp (governor, private->governor) != 0)) {
			 changed = TRUE;
	   }

	   if (!private->freq || (g_ascii_strcasecmp (freq, private->freq) != 0)) {
			 
			 changed = TRUE;
	   }

	   if (!private->perc || (g_ascii_strcasecmp (perc, private->perc) != 0)) {
			 changed = TRUE;
	   }

	   if (!private->unit || (g_ascii_strcasecmp (unit, private->unit) != 0)) {
			 changed = TRUE;
	   }

	   parent_class->free_data (CPUFREQ_MONITOR (monitor));

	   private->governor = governor;
	   private->freq = freq;
	   private->perc = perc;
	   private->unit = unit;

	   if (private->governor == NULL)
			 return FALSE;
	   if (private->freq == NULL)
			 return FALSE;
	   if (private->perc == NULL)
			 return FALSE;
	   if (private->unit == NULL)
			 return FALSE;

	   if (changed)
			 g_signal_emit (CPUFREQ_MONITOR (monitor), parent_class->signals[CHANGED], 0);

	   return TRUE;
}

static void
cpufreq_monitor_sysfs_run (CPUFreqMonitor *monitor)
{
	   CPUFreqMonitorProtected *private;
	   
	   g_return_if_fail (IS_CPUFREQ_MONITOR_SYSFS (monitor));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (CPUFREQ_MONITOR (monitor));

	   if (private->timeout_handler > 0)
			 g_source_remove (private->timeout_handler);

	   private->timeout_handler = g_timeout_add (1000, cpufreq_monitor_sysfs_get, (gpointer) monitor);
}

static gint
compare (gconstpointer a, gconstpointer b)
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
cpufreq_monitor_sysfs_get_available_frequencies (CPUFreqMonitor *monitor)
{
	   GnomeVFSHandle          *handle;
	   GnomeVFSFileSize         bytes_read;
	   GnomeVFSResult           result;
	   gchar                   *uri;
	   gchar                    buffer[256];
	   gchar                   *str;
	   gchar                   *path;
	   GList                   *list = NULL;
	   gchar                  **frequencies = NULL;
	   gint                     i;
	   CPUFreqMonitorProtected *private;

	   g_return_val_if_fail (IS_CPUFREQ_MONITOR_SYSFS (monitor), NULL);

	   private = CPUFREQ_MONITOR_GET_PROTECTED (CPUFREQ_MONITOR (monitor));

	   if (private->available_freqs) {
			 return private->available_freqs;
	   }

	   path = g_strdup_printf ("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies",
						  private->cpu);

	   uri = gnome_vfs_get_uri_from_local_path (path);
	   g_free (path);

	   result = gnome_vfs_open (&handle, uri, GNOME_VFS_OPEN_READ);
	   if (result != GNOME_VFS_OK) {
			 if (uri) g_free (uri);

			 return FALSE;
	   }

	   g_free (uri);

	   result = gnome_vfs_read (handle, buffer, sizeof (buffer), &bytes_read);

	   str = g_strndup (buffer, bytes_read);
	   str = g_strchomp (str);

	   frequencies = g_strsplit (str, " ", 0);

	   if (result != GNOME_VFS_OK) {
			 g_strfreev (frequencies);
			 gnome_vfs_close (handle);
			 
			 return FALSE;
	   }
	   
	   result = gnome_vfs_close (handle);
	   if (result != GNOME_VFS_OK) {
			 g_strfreev (frequencies);
			 
			 return FALSE;
	   }
	   
	   i = 0;
	   while (frequencies[i] != NULL) {
			 if (!g_list_find_custom (list, frequencies[i], compare))
				    list = g_list_append (list, g_strdup (frequencies[i]));
			 i++;
	   }
	   
	   g_strfreev (frequencies);
	   g_free (str);

	   private->available_freqs = g_list_sort (list, compare);

	   return private->available_freqs;
}

