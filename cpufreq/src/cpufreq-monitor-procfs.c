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

#include <gnome.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glib/gi18n.h>

#include <string.h>

#include "cpufreq-monitor-procfs.h"
#include "cpufreq-monitor-protected.h"

#define PARENT_TYPE TYPE_CPUFREQ_MONITOR

#define CPUFREQ_MONITOR_GET_PROTECTED(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PARENT_TYPE, CPUFreqMonitorProtected))

static void     cpufreq_monitor_procfs_class_init                (CPUFreqMonitorProcfsClass *klass);
static void     cpufreq_monitor_procfs_finalize                  (GObject *object);

static void     cpufreq_monitor_procfs_run                       (CPUFreqMonitor *monitor);
static GList   *cpufreq_monitor_procfs_get_available_frequencies (CPUFreqMonitor *monitor);

static gint     cpufreq_monitor_procfs_get_freq_from_userspace   (guint cpu);
static gboolean cpufreq_monitor_procfs_parse                     (CPUFreqMonitorProcfs *monitor, gint *cpu, gint *fmax,
													 gint *pmin, gint *pmax, gint *fmin, gchar *mode);
static gboolean cpufreq_monitor_procfs_get                       (gpointer gdata);


static CPUFreqMonitorClass *parent_class = NULL;

typedef struct _CPUFreqMonitorProtected CPUFreqMonitorProtected;

GType cpufreq_monitor_procfs_get_type ()
{
	   static GType type = 0;

	   if (!type) {
			 static const GTypeInfo info = {
				    sizeof (CPUFreqMonitorProcfsClass),
				    (GBaseInitFunc) NULL,
				    (GBaseFinalizeFunc) NULL,
				    (GClassInitFunc) cpufreq_monitor_procfs_class_init,
				    NULL,
				    NULL,
				    sizeof (CPUFreqMonitorProcfs),
				    0,
				    NULL
			 };

			 type = g_type_register_static (PARENT_TYPE, "CPUFreqMonitorProcfs",
									  &info, 0);
	   }

	   return type;
}

static void
cpufreq_monitor_procfs_class_init (CPUFreqMonitorProcfsClass *klass)
{
	   GObjectClass        *object_class = G_OBJECT_CLASS (klass);
	   CPUFreqMonitorClass *monitor_class = CPUFREQ_MONITOR_CLASS (klass);

	   parent_class = g_type_class_peek_parent (klass);

	   monitor_class->run = cpufreq_monitor_procfs_run;
	   monitor_class->get_available_frequencies = cpufreq_monitor_procfs_get_available_frequencies;
	   
	   object_class->finalize = cpufreq_monitor_procfs_finalize;
}

static void
cpufreq_monitor_procfs_finalize (GObject *object)
{
	   g_return_if_fail (IS_CPUFREQ_MONITOR_PROCFS (object));

	   if (G_OBJECT_CLASS (parent_class)->finalize)
			 (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

CPUFreqMonitor *
cpufreq_monitor_procfs_new (guint cpu)
{
	   CPUFreqMonitorProcfs *monitor;

	   monitor = g_object_new (TYPE_CPUFREQ_MONITOR_PROCFS, "cpu", cpu, NULL);

	   return CPUFREQ_MONITOR (monitor);
}

static gint
cpufreq_monitor_procfs_get_freq_from_userspace (guint cpu)
{
	   GnomeVFSHandle   *handle;
	   GnomeVFSFileSize  bytes_read;
	   GnomeVFSResult    result;
	   gchar            *uri, *path;
	   gchar             buffer[256];
	   gchar            *frequency;
	   gint              freq;

	   path = g_strdup_printf ("/proc/sys/cpu/%d/speed", cpu);
	   uri = gnome_vfs_get_uri_from_local_path (path);
	   g_free (path);

	   result = gnome_vfs_open (&handle, uri, GNOME_VFS_OPEN_READ);
	   if (result != GNOME_VFS_OK) {
			 if (uri) g_free (uri);

			 return -1;
	   }

	   g_free (uri);

	   result = gnome_vfs_read (handle, buffer, 256, &bytes_read);
	   frequency = g_strndup (buffer, bytes_read);
	   freq = atoi (frequency);
	   g_free (frequency);
	   
	   if (result != GNOME_VFS_OK) {
			 return -1;
	   }

	   result = gnome_vfs_close (handle);
	   if (result != GNOME_VFS_OK) {
			 return -1;
	   }

	   return freq;
}

static gboolean
cpufreq_monitor_procfs_parse (CPUFreqMonitorProcfs *monitor, gint *cpu, gint *fmax,
						gint *pmin, gint *pmax, gint *fmin, gchar *mode)
{
	   GnomeVFSHandle           *handle;
	   GnomeVFSFileSize          bytes_read;
	   GnomeVFSResult            result;
	   gchar                    *uri, *file;
	   gchar                   **lines;
	   gchar                     buffer[256];
	   gint                      i, count;
	   CPUFreqMonitorProtected  *private;

	   g_return_val_if_fail (IS_CPUFREQ_MONITOR (monitor), FALSE);

	   private = CPUFREQ_MONITOR_GET_PROTECTED (CPUFREQ_MONITOR (monitor));

	   uri = gnome_vfs_get_uri_from_local_path ("/proc/cpufreq");

	   result = gnome_vfs_open (&handle, uri, GNOME_VFS_OPEN_READ);
	   if (result != GNOME_VFS_OK) {
			 if (uri) g_free (uri);

			 return FALSE;
	   }

	   g_free (uri);

	   result = gnome_vfs_read (handle, buffer, 256, &bytes_read);
	   file = g_strndup (buffer, bytes_read);
	   if (result != GNOME_VFS_OK) {
			 g_free (file);

			 return FALSE;
	   }

	   result = gnome_vfs_close (handle);
	   if (result != GNOME_VFS_OK) {
			 g_free (file);

			 return FALSE;
	   }

	   lines = g_strsplit (file, "\n", -1);
	   for (i=0; lines[i]; i++) {
			 if (g_ascii_strncasecmp (lines[i], "CPU", 3) == 0) {
				    /* CPU  0       650000 kHz ( 81 %)  -     800000 kHz (100 %)  -  powersave */
				    count = sscanf (lines[i], "CPU %d %d kHz (%d %%) - %d kHz (%d %%) - %20s",
								cpu, fmin, pmin, fmax, pmax, mode);

				    if ((*cpu) == private->cpu)
						  break;
			 }
	   }

	   g_strfreev (lines);
	   g_free (file);

	   if (count != 6) {
			 /* TODO: manage error */
			 return FALSE;
	   }

	   return TRUE;
}	   

static gboolean
cpufreq_monitor_procfs_get (gpointer gdata)
{
	   gint                     fmax, fmin, cpu, ifreq;
	   gint                     pmin, pmax;
	   gchar                    mode[21];
	   gchar                   *freq, *perc, *unit, *governor;
	   gboolean                 changed;
	   CPUFreqMonitorProcfs    *monitor;
	   CPUFreqMonitorProtected *private;

	   monitor = (CPUFreqMonitorProcfs *) gdata;

	   if (!cpufreq_monitor_procfs_parse (monitor, &cpu, &fmax, &pmin, &pmax, &fmin, mode))
			 return FALSE;
	   
	   private = CPUFREQ_MONITOR_GET_PROTECTED (CPUFREQ_MONITOR (monitor));

	   governor = g_strdup (mode);
	   
	   if (g_ascii_strcasecmp (governor, "powersave") == 0) {
			 freq = parent_class->get_human_readable_freq (fmin);
			 perc = g_strdup_printf ("%d%%", pmin);
			 unit = parent_class->get_human_readable_unit (fmin);
	   } else if (g_ascii_strcasecmp (governor, "performance") == 0) {
			 freq = parent_class->get_human_readable_freq (fmax);
			 perc = g_strdup_printf ("%d%%", pmax);
			 unit = parent_class->get_human_readable_unit (fmax);
	   } else if (g_ascii_strcasecmp (governor, "userspace") == 0) {
			 ifreq = cpufreq_monitor_procfs_get_freq_from_userspace (private->cpu);
			 freq = parent_class->get_human_readable_freq (ifreq);
			 perc = parent_class->get_human_readable_perc (fmax, ifreq);
			 unit = parent_class->get_human_readable_unit (ifreq);
	   } else {
			 freq = g_strdup (_("Unknown"));
			 perc = g_strdup ("-");
			 unit = g_strdup ("-");
	   }

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
cpufreq_monitor_procfs_run (CPUFreqMonitor *monitor)
{
	   CPUFreqMonitorProtected *private;
	   
	   g_return_if_fail (IS_CPUFREQ_MONITOR_PROCFS (monitor));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (CPUFREQ_MONITOR (monitor));

	   if (private->timeout_handler > 0)
			 g_source_remove (private->timeout_handler);

	   private->timeout_handler = g_timeout_add (1000, cpufreq_monitor_procfs_get, (gpointer) monitor);
}

static void
free_string (gpointer str, gpointer gdata)
{
	   if (str) g_free (str);
}

static GList *
cpufreq_monitor_procfs_get_available_frequencies (CPUFreqMonitor *monitor)
{
	   gint                     fmax, fmin, cpu, freq;
	   gint                     pmin, pmax;
	   gchar                    mode[21];
	   GList                   *list = NULL;
	   CPUFreqMonitorProtected *private;

	   g_return_val_if_fail (IS_CPUFREQ_MONITOR_PROCFS (monitor), NULL);

	   private = CPUFREQ_MONITOR_GET_PROTECTED (CPUFREQ_MONITOR (monitor));

	   if (private->available_freqs) {
			 g_list_foreach (private->available_freqs,
						  free_string, NULL);
			 g_list_free (private->available_freqs);
			 private->available_freqs = NULL;
	   }

	   if (!cpufreq_monitor_procfs_parse (CPUFREQ_MONITOR_PROCFS (monitor), &cpu, &fmax, &pmin, &pmax, &fmin, mode))
			 return NULL;

	   if ((pmax > 0) && (pmax != 100)) {
			 freq = (fmax * 100) / pmax;
			 private->available_freqs = g_list_append (private->available_freqs, g_strdup_printf ("%d", freq));
	   }
	   
	   private->available_freqs = g_list_append (private->available_freqs, g_strdup_printf ("%d", fmax));
	   if (fmax != fmin)
			 private->available_freqs = g_list_append (private->available_freqs, g_strdup_printf ("%d", fmin));

	   return private->available_freqs;
}

