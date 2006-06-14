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

#include <string.h>
#include <stdlib.h>

#include "cpufreq-monitor-sysfs.h"

enum {
        SCALING_MAX,
        SCALING_MIN,
        GOVERNOR,
        CPUINFO_MAX,
        SCALING_SETSPEED,
        SCALING_CUR_FREQ,
        N_FILES
};

static void     cpufreq_monitor_sysfs_class_init                (CPUFreqMonitorSysfsClass *klass);

static gboolean cpufreq_monitor_sysfs_run                       (CPUFreqMonitor *monitor);
static GList   *cpufreq_monitor_sysfs_get_available_frequencies (CPUFreqMonitor *monitor);
static GList   *cpufreq_monitor_sysfs_get_available_governors   (CPUFreqMonitor *monitor);

/* /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_max_freq
 * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_min_freq
 * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_governor
 * /sys/devices/system/cpu/cpu[0]/cpufreq/cpuinfo_max_freq
 * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_setspeed (userspace)
 * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_cur_freq (new governors)
 */
const gchar *monitor_sysfs_files[] = {
        "scaling_max_freq",
        "scaling_min_freq",
        "scaling_governor",
        "cpuinfo_max_freq",
        "scaling_setspeed",
        "scaling_cur_freq",
        NULL
};

#define CPUFREQ_SYSFS_BASE_PATH "/sys/devices/system/cpu/cpu%u/cpufreq/%s"

G_DEFINE_TYPE (CPUFreqMonitorSysfs, cpufreq_monitor_sysfs, CPUFREQ_TYPE_MONITOR)

static void
cpufreq_monitor_sysfs_init (CPUFreqMonitorSysfs *monitor)
{
}

static void
cpufreq_monitor_sysfs_class_init (CPUFreqMonitorSysfsClass *klass)
{
        CPUFreqMonitorClass *monitor_class = CPUFREQ_MONITOR_CLASS (klass);

        monitor_class->run = cpufreq_monitor_sysfs_run;
        monitor_class->get_available_frequencies = cpufreq_monitor_sysfs_get_available_frequencies;
        monitor_class->get_available_governors = cpufreq_monitor_sysfs_get_available_governors;
}

CPUFreqMonitor *
cpufreq_monitor_sysfs_new (guint cpu)
{
        CPUFreqMonitorSysfs *monitor;

        monitor = g_object_new (CPUFREQ_TYPE_MONITOR_SYSFS,
                                "cpu", cpu, NULL);

        return CPUFREQ_MONITOR (monitor);
}

static gboolean
cpufreq_monitor_sysfs_run (CPUFreqMonitor *monitor)
{
        gint    i;
        gchar **data;
        guint   cpu;
        gint    cur_freq, max_freq;
        gchar *governor;

        g_object_get (G_OBJECT (monitor), "cpu", &cpu, NULL);

        data = g_new0 (gchar *, N_FILES);
           
        for (i = 0; i < N_FILES; i++) {
                gchar *path, *p;
                gint   len;
                gchar *buffer = NULL;
                GError *error = NULL;
                
                path = g_strdup_printf (CPUFREQ_SYSFS_BASE_PATH,
                                        cpu, monitor_sysfs_files[i]);

                if (!g_file_get_contents (path, &buffer, NULL, &error)) {
                        int j;

                        g_free (path);
                        
                        if (error->code == G_FILE_ERROR_NOENT) {
                                g_error_free (error);
                                continue;
                        }

                        g_warning (error->message);
                        g_error_free (error);

                        for (j = 0; j < N_FILES; j++) {
                                g_free (data[j]);
                                data[j] = NULL;
                        }
                        g_free (data);
                        
                        return FALSE;
                }

                g_free (path);

                /* Try to remove the '\n' */
                p = g_strrstr (buffer, "\n");
                len = strlen (buffer);
                if (p)
                        len -= strlen (p);
                
                data[i] = g_strndup (buffer, len);

                g_free (buffer);
        }

        governor = data[GOVERNOR];
           
        if (g_ascii_strcasecmp (governor, "userspace") == 0) {
                cur_freq = atoi (data[SCALING_SETSPEED]);
                max_freq = atoi (data[CPUINFO_MAX]);
        } else if (g_ascii_strcasecmp (governor, "powersave") == 0) {
                cur_freq = atoi (data[SCALING_MIN]);
                max_freq = atoi (data[CPUINFO_MAX]);
        } else if (g_ascii_strcasecmp (governor, "performance") == 0) {
                cur_freq = atoi (data[SCALING_MAX]);
                max_freq = atoi (data[CPUINFO_MAX]);
        } else { /* Ondemand, Conservative, ... */
                cur_freq = atoi (data[SCALING_CUR_FREQ]);
                max_freq = atoi (data[CPUINFO_MAX]);
        }

        g_object_set (G_OBJECT (monitor),
                      "governor", governor,
                      "frequency", cur_freq,
                      "max-frequency", max_freq,
                      NULL);

        for (i = 0; i < N_FILES; i++) {
                g_free (data[i]);
                data[i] = NULL;
        }
        g_free (data);
        
        return TRUE;
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
        gchar  *path;
        GList  *list = NULL;
        gchar **frequencies = NULL;
        gint    i;
        guint   cpu;
        gchar  *buffer = NULL;
        GError *error = NULL;

        g_object_get (G_OBJECT (monitor),
                      "cpu", &cpu, NULL);

        path = g_strdup_printf (CPUFREQ_SYSFS_BASE_PATH, cpu, 
                                "scaling_available_frequencies");

        if (!g_file_get_contents (path, &buffer, NULL, &error)) {
                g_warning (error->message);
                g_error_free (error);

                g_free (path);

                return NULL;
        }

        g_free (path);
        
        buffer = g_strchomp (buffer);
        frequencies = g_strsplit (buffer, " ", -1);

        i = 0;
        while (frequencies[i]) {
                if (!g_list_find_custom (list, frequencies[i], compare))
                        list = g_list_prepend (list, g_strdup (frequencies[i]));
                i++;
        }
           
        g_strfreev (frequencies);
        g_free (buffer);

        return g_list_sort (list, compare);
}

static GList *
cpufreq_monitor_sysfs_get_available_governors (CPUFreqMonitor *monitor)
{
        gchar   *path;
        GList   *list = NULL;
        gchar  **governors = NULL;
        gint     i;
        guint    cpu;
        gchar   *buffer = NULL;
        GError  *error = NULL;

        g_object_get (G_OBJECT (monitor),
                      "cpu", &cpu, NULL);
        
        path = g_strdup_printf (CPUFREQ_SYSFS_BASE_PATH, cpu,
                                "scaling_available_governors");

        if (!g_file_get_contents (path, &buffer, NULL, &error)) {
                g_warning (error->message);
                g_error_free (error);

                g_free (path);

                return NULL;
        }

        g_free (path);
        
        buffer = g_strchomp (buffer);

        governors = g_strsplit (buffer, " ", -1);

        i = 0;
        while (governors[i] != NULL) {
                list = g_list_prepend (list, g_strdup (governors[i]));
                i++;
        }
           
        g_strfreev (governors);
        g_free (buffer);

        return list;
}
