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


#include <libgnomevfs/gnome-vfs.h>
#include <gnome.h>
#include <string.h>

#include "cpufreq-applet.h"
#include "cpufreq.h"

static gboolean cpufreq_parse_procfs   (CPUFreqApplet *applet, gint *cpu, gint *fmax,
								gint *pmin, gint *pmax, gint *fmin, gchar *mode);
static void     cpufreq_applet_prepare (CPUFreqApplet *applet);

gchar *
cpufreq_get_human_readble_freq (gint freq)
{
	   gint divisor;

	   if (freq > 999999) /* freq (kHz) */
			 divisor = (1000 * 1000);
	   else
			 divisor = 1000;

	   if (((freq % divisor) == 0) || divisor == 1000) /* integer */
			 return g_strdup_printf ("%d", freq / divisor);
	   else /* float */
			 return g_strdup_printf ("%3.2f", ((gfloat)freq / divisor));
}

gchar *
cpufreq_get_human_readble_unit (gint freq)
{
	   if (freq > 999999) /* freq (kHz) */
			 return g_strdup ("GHz");
	   else
			 return g_strdup ("MHz");
}

gchar *
cpufreq_get_human_readble_perc (gint fmax, gint fmin)
{
	   return g_strdup_printf ("%d%%", (fmin * 100) / fmax);
}

static gint
cpufreq_get_freq_from_userspace_procfs (guint cpu)
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
cpufreq_parse_procfs (CPUFreqApplet *applet, gint *cpu, gint *fmax,
				  gint *pmin, gint *pmax, gint *fmin, gchar *mode)
{
	   GnomeVFSHandle   *handle;
	   GnomeVFSFileSize  bytes_read;
	   GnomeVFSResult    result;
	   gchar            *uri, *file;
	   gchar            **lines;
	   gchar             buffer[256];
	   gint              i, count;

	   g_return_val_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)), FALSE);

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

				    if ((*cpu) == applet->cpu)
						  break;
			 }
	   }

	   g_strfreev (lines);
	   g_free (file);

	   if (count != 6) {
			 /* /proc/cpufreq contains only the header */
			 applet->iface = IFACE_CPUINFO;
			 cpufreq_applet_run (applet);

			 return FALSE;
	   }

	   return TRUE;
}	   

static void
cpufreq_applet_prepare (CPUFreqApplet *applet)
{
	   g_return_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)));
	   
	   if (applet->freq) {
			 g_free (applet->freq);
			 applet->freq = NULL;
	   }

	   if (applet->perc) {
			 g_free (applet->perc);
			 applet->perc = NULL;
	   }

	   if (applet->unit) {
			 g_free (applet->unit);
			 applet->unit = NULL;
	   }

	   if (applet->governor) {
			 g_free (applet->governor);
			 applet->governor = NULL;
	   }
}

gboolean
cpufreq_get_from_procfs (gpointer gdata)
{
	   gint           fmax, fmin, cpu, freq;
	   gint           pmin, pmax;
	   gchar          mode[21];
	   CPUFreqApplet *applet;

	   applet = (CPUFreqApplet *) gdata;

	   g_return_val_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)), FALSE);

	   if (!cpufreq_parse_procfs (applet, &cpu, &fmax, &pmin, &pmax, &fmin, mode))
			 return FALSE;
	   
	   cpufreq_applet_prepare (applet);
	   
	   applet->governor = g_strdup (mode);
	   
	   if (g_ascii_strcasecmp (applet->governor, "powersave") == 0) {
			 applet->freq = cpufreq_get_human_readble_freq (fmin);
			 applet->perc = g_strdup_printf ("%d%%", pmin);
			 applet->unit = cpufreq_get_human_readble_unit (fmin);
	   } else if (g_ascii_strcasecmp (applet->governor, "performance") == 0) {
			 applet->freq = cpufreq_get_human_readble_freq (fmax);
			 applet->perc = g_strdup_printf ("%d%%", pmax);
			 applet->unit = cpufreq_get_human_readble_unit (fmax);
	   } else if (g_ascii_strcasecmp (applet->governor, "userspace") == 0) {
			 freq = cpufreq_get_freq_from_userspace_procfs (applet->cpu);
			 applet->freq = cpufreq_get_human_readble_freq (freq);
			 applet->perc = cpufreq_get_human_readble_perc (fmax, freq);
			 applet->unit = cpufreq_get_human_readble_unit (freq);
	   } else {
			 applet->freq = g_strdup (_("Unknown"));
			 applet->perc = g_strdup ("-");
			 applet->unit = g_strdup ("-");
	   }
			 
	   if (applet->freq == NULL)
			 return FALSE;
	   if (applet->perc == NULL)
			 return FALSE;
	   if (applet->unit == NULL)
			 return FALSE;

	   cpufreq_applet_update (applet);

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

GList *
cpufreq_get_frequencies_from_procfs (CPUFreqApplet *applet)
{
	   gint   fmax, fmin, cpu, freq;
	   gint   pmin, pmax;
	   gchar  mode[21];
	   GList *list = NULL;

	   g_return_val_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)), FALSE);

	   if (!cpufreq_parse_procfs (applet, &cpu, &fmax, &pmin, &pmax, &fmin, mode))
			 return FALSE;

	   if (pmax != 100) {
			 freq = (fmax * 100) / pmax;
			 list = g_list_append (list, g_strdup_printf ("%d", freq));
	   }
	   
	   list = g_list_append (list, g_strdup_printf ("%d", fmax));
	   if (fmax != fmin)
			 list = g_list_append (list, g_strdup_printf ("%d", fmin));

	   return list;
}

gboolean
cpufreq_get_from_sysfs (gpointer gdata)
{
	   GnomeVFSHandle   *handle;
	   GnomeVFSFileSize  bytes_read;
	   GnomeVFSResult    result;
	   gchar            *uri;
	   gchar             buffer[20];
	   gint              i;
	   CPUFreqApplet    *applet;
	   gchar            **cpufreq_data;
	   gchar            *path;
	   gchar            *files[] = {
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

	   applet = (CPUFreqApplet *) gdata;

	   g_return_val_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)), FALSE);

	   /* /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_max_freq
	    * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_min_freq
	    * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_governor
	    * /sys/devices/system/cpu/cpu[0]/cpufreq/cpuinfo_max_freq
	    * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_setspeed (userspace)
	    * /sys/devices/system/cpu/cpu[0]/cpufreq/scaling_cur_freq (new governors)
	    */

	   cpufreq_data = g_new (gchar *, LAST + 1);
	   
	   for (i=0; i<LAST; i++) {
			 cpufreq_data[i] = NULL;

			 path = g_strdup_printf ("/sys/devices/system/cpu/cpu%d/cpufreq/%s",
								applet->cpu, files[i]);
			 
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

	   cpufreq_applet_prepare (applet);

	   applet->governor = g_strdup (cpufreq_data[GOVERNOR]);
	   
	   if (g_ascii_strcasecmp (applet->governor, "userspace") == 0) {
			 applet->freq = cpufreq_get_human_readble_freq (atoi (cpufreq_data[SCALING_SETSPEED]));
			 applet->perc = cpufreq_get_human_readble_perc (atoi (cpufreq_data[CPUINFO_MAX]),
												   atoi (cpufreq_data[SCALING_SETSPEED]));
			 applet->unit = cpufreq_get_human_readble_unit (atoi (cpufreq_data[SCALING_SETSPEED]));
	   } else if (g_ascii_strcasecmp (applet->governor, "powersave") == 0) {
			 applet->freq = cpufreq_get_human_readble_freq (atoi (cpufreq_data[SCALING_MIN]));
			 applet->perc = cpufreq_get_human_readble_perc (atoi (cpufreq_data[CPUINFO_MAX]),
												   atoi (cpufreq_data[SCALING_MIN]));
			 applet->unit = cpufreq_get_human_readble_unit (atoi (cpufreq_data[SCALING_MIN]));
	   } else if (g_ascii_strcasecmp (applet->governor, "performance") == 0) {
			 applet->freq = cpufreq_get_human_readble_freq (atoi (cpufreq_data[SCALING_MAX]));
			 applet->perc = cpufreq_get_human_readble_perc (atoi (cpufreq_data[CPUINFO_MAX]),
												   atoi (cpufreq_data[SCALING_MAX]));
			 applet->unit = cpufreq_get_human_readble_unit (atoi (cpufreq_data[SCALING_MAX]));
	   } else {
			 applet->freq = cpufreq_get_human_readble_freq (atoi (cpufreq_data[SCALING_CUR_FREQ]));
			 applet->perc = cpufreq_get_human_readble_perc (atoi (cpufreq_data[CPUINFO_MAX]),
												   atoi (cpufreq_data[SCALING_CUR_FREQ]));
			 applet->unit = cpufreq_get_human_readble_unit (atoi (cpufreq_data[SCALING_CUR_FREQ]));
	   }

	   if (applet->freq == NULL) {
			 g_strfreev (cpufreq_data);
			 
			 return FALSE;
	   }

	   if (applet->perc == NULL) {
			 g_strfreev (cpufreq_data);
			 
			 return FALSE;
	   }

	   if (applet->unit == NULL) {
			 g_strfreev (cpufreq_data);

			 return FALSE;
	   }

	   g_strfreev (cpufreq_data);

	   cpufreq_applet_update (applet);

	   return TRUE;
}

GList *
cpufreq_get_frequencies_from_sysfs (CPUFreqApplet *applet)
{
	   GnomeVFSHandle   *handle;
	   GnomeVFSFileSize  bytes_read;
	   GnomeVFSResult    result;
	   gchar            *uri;
	   gchar             buffer[256];
	   gchar            *str;
	   gchar            *path;
	   GList            *list = NULL;
	   gchar            **frequencies = NULL;
	   gint              i;

	   path = g_strdup_printf ("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies",
						  applet->cpu);

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

	   list = g_list_sort (list, compare);
	   
	   return list;
}

gboolean
cpufreq_get_from_procfs_cpuinfo (CPUFreqApplet *applet)
{
	   GnomeVFSHandle   *handle;
	   GnomeVFSFileSize  bytes_read;
	   GnomeVFSResult    result;
	   gchar            *uri, *file;
	   gchar            **lines;
	   gchar             buffer[256];
	   gchar            *p;
	   gint              cpu, i;

	   g_return_val_if_fail (PANEL_IS_APPLET (PANEL_APPLET (applet)), FALSE);

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

	   cpufreq_applet_prepare (applet);

	   applet->governor = g_strdup ("performance");
	   applet->freq = cpufreq_get_human_readble_freq (cpu * 1000); /* kHz are expected*/
	   applet->unit = cpufreq_get_human_readble_unit (cpu * 1000); /* kHz are expected*/
	   applet->perc = g_strdup ("100%");

	   if (applet->freq == NULL)
			 return FALSE;
	   if (applet->perc == NULL)
			 return FALSE;
	   if (applet->unit == NULL)
			 return FALSE;

	   cpufreq_applet_update (applet);

	   return TRUE;
}
