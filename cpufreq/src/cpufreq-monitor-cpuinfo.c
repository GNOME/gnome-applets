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

#include "cpufreq-monitor-cpuinfo.h"
#include "cpufreq-monitor-protected.h"

#define PARENT_TYPE TYPE_CPUFREQ_MONITOR

#define CPUFREQ_MONITOR_GET_PROTECTED(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PARENT_TYPE, CPUFreqMonitorProtected))

static void     cpufreq_monitor_cpuinfo_class_init (CPUFreqMonitorCPUInfoClass *klass);
static void     cpufreq_monitor_cpuinfo_finalize   (GObject *object);

static void     cpufreq_monitor_cpuinfo_run        (CPUFreqMonitor *monitor);
static gboolean cpufreq_monitor_cpuinfo_get        (gpointer gdata);


static CPUFreqMonitorClass *parent_class = NULL;

typedef struct _CPUFreqMonitorProtected CPUFreqMonitorProtected;

GType cpufreq_monitor_cpuinfo_get_type ()
{
	   static GType type = 0;

	   if (!type) {
			 static const GTypeInfo info = {
				    sizeof (CPUFreqMonitorCPUInfoClass),
				    (GBaseInitFunc) NULL,
				    (GBaseFinalizeFunc) NULL,
				    (GClassInitFunc) cpufreq_monitor_cpuinfo_class_init,
				    NULL,
				    NULL,
				    sizeof (CPUFreqMonitorCPUInfo),
				    0,
				    NULL
			 };

			 type = g_type_register_static (PARENT_TYPE, "CPUFreqMonitorCPUInfo",
									  &info, 0);
	   }

	   return type;
}

static void
cpufreq_monitor_cpuinfo_class_init (CPUFreqMonitorCPUInfoClass *klass)
{
	   GObjectClass        *object_class = G_OBJECT_CLASS (klass);
	   CPUFreqMonitorClass *monitor_class = CPUFREQ_MONITOR_CLASS (klass);

	   parent_class = g_type_class_peek_parent (klass);

	   monitor_class->run = cpufreq_monitor_cpuinfo_run;
	   monitor_class->get_available_frequencies = NULL;
	   
	   object_class->finalize = cpufreq_monitor_cpuinfo_finalize;
}

static void
cpufreq_monitor_cpuinfo_finalize (GObject *object)
{
	   g_return_if_fail (IS_CPUFREQ_MONITOR_CPUINFO (object));

	   if (G_OBJECT_CLASS (parent_class)->finalize)
			 (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

CPUFreqMonitor *
cpufreq_monitor_cpuinfo_new (guint cpu)
{
	   CPUFreqMonitorCPUInfo *monitor;

	   monitor = g_object_new (TYPE_CPUFREQ_MONITOR_CPUINFO, "cpu", cpu, NULL);

	   return CPUFREQ_MONITOR (monitor);
}

static gboolean
cpufreq_monitor_cpuinfo_get (gpointer gdata)
{
	   GnomeVFSHandle           *handle;
	   GnomeVFSFileSize          bytes_read;
	   GnomeVFSResult            result;
	   gchar                    *uri, *file;
	   gchar                   **lines;
	   gchar                     buffer[256];
	   gchar                    *p;
	   gchar                    *freq, *perc, *unit, *governor;
	   gint                      cpu, i;
	   CPUFreqMonitorCPUInfo    *monitor;
	   CPUFreqMonitorProtected  *private;

	   monitor = (CPUFreqMonitorCPUInfo *) gdata;

	   private = CPUFREQ_MONITOR_GET_PROTECTED (CPUFREQ_MONITOR (monitor));
	   
	   uri = gnome_vfs_get_uri_from_local_path ("/proc/cpuinfo");

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
			 gnome_vfs_close (handle);
			 
			 return FALSE;
	   }

	   result = gnome_vfs_close (handle);
	   if (result != GNOME_VFS_OK) {
			 g_free (file);
			 
			 return FALSE;
	   }

	   /* TODO: SMP support */
	   lines = g_strsplit (file, "\n", -1);
	   for (i=0; lines[i]; i++) {
			 if (g_ascii_strncasecmp ("cpu MHz", lines[i], strlen ("cpu MHz")) == 0) {
				    p = g_strrstr (lines[i], ":");

				    if (p == NULL) {
						  g_strfreev (lines);
						  g_free (file);
						  
						  return FALSE;
				    }

				    if (strlen (lines[i]) < (p - lines[i])) {
						  g_strfreev (lines);
						  g_free (file);
						  
						  return FALSE;
				    }

				    if ((sscanf(p + 1, "%d.", &cpu)) != 1) {
						  g_strfreev (lines);
						  g_free (file);
						  
						  return FALSE;
				    }

				    break;
			 }
	   }

	   g_strfreev (lines);
	   g_free (file);
	   
	   governor = g_strdup (_("Frequency Scaling Unsupported"));
	   freq = parent_class->get_human_readable_freq (cpu * 1000); /* kHz are expected*/
	   unit = parent_class->get_human_readable_unit (cpu * 1000); /* kHz are expected*/
	   perc = g_strdup ("100%");

	   parent_class->free_data (CPUFREQ_MONITOR (monitor));

	   private->governor = governor;
	   private->freq = freq;
	   private->perc = perc;
	   private->unit = unit;

	   if (private->freq == NULL)
			 return FALSE;
	   if (private->unit == NULL)
			 return FALSE;

	   g_signal_emit (CPUFREQ_MONITOR (monitor), parent_class->signals[CHANGED], 0);

	   return TRUE;
}

static void
cpufreq_monitor_cpuinfo_run (CPUFreqMonitor *monitor)
{
	   CPUFreqMonitorProtected *private;
	   
	   g_return_if_fail (IS_CPUFREQ_MONITOR_CPUINFO (monitor));

	   cpufreq_monitor_cpuinfo_get (monitor);
}


