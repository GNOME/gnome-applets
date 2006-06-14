/*
 * GNOME CPUFreq Applet
 * Copyright (C) 2006 Carlos Garcia Campos <carlosgc@gnome.org>
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

#include <gtk/gtkmessagedialog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cpufreq-utils.h"

guint
cpufreq_utils_get_n_cpus (void)
{
	gint   mcpu = -1;
	gchar *file = NULL;
	static guint n_cpus = 0;

	if (n_cpus > 0)
		return n_cpus;
	
	do {
		if (file)
			g_free (file);
		mcpu ++;
		file = g_strdup_printf ("/sys/devices/system/cpu/cpu%d", mcpu);
	} while (g_file_test (file, G_FILE_TEST_EXISTS));
	g_free (file);

	if (mcpu >= 0) {
		n_cpus = (guint)mcpu;
		return mcpu;
	}

	mcpu = -1;
	file = NULL;
	do {
		if (file)
			g_free (file);
		mcpu ++;
		file = g_strdup_printf ("/proc/sys/cpu/%d", mcpu);
	} while (g_file_test (file, G_FILE_TEST_EXISTS));
	g_free (file);

	if (mcpu >= 0) {
		n_cpus = (guint)mcpu;
		return mcpu;
	}

	n_cpus = 1;
	
	return 1;
}

void
cpufreq_utils_display_error (const gchar *message,
			     const gchar *secondary)
{
	GtkWidget *dialog;

	g_return_if_fail (message != NULL);

	dialog = gtk_message_dialog_new (NULL,
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_OK,
					 message);
	if (secondary) {
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
							  secondary);
	}
	
	gtk_window_set_title (GTK_WINDOW (dialog), ""); /* as per HIG */
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
	g_signal_connect (G_OBJECT (dialog),
			  "response",
			  G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_widget_show (dialog);
}

gboolean
cpufreq_utils_selector_is_available (void)
{
	struct stat *info;
	gchar       *path = NULL;

	path = g_find_program_in_path ("cpufreq-selector");
	if (!path)
		return FALSE;

	if (geteuid () == 0)
		return TRUE;

	info = (struct stat *) g_malloc (sizeof (struct stat));

	if ((lstat (path, info)) != -1) {
		if ((info->st_mode & S_ISUID) && (info->st_uid == 0)) {
			g_free (info);
			g_free (path);

			return TRUE;
		}
	}

	g_free (info);
	g_free (path);

	return FALSE;
}

gchar *
cpufreq_utils_get_frequency_label (guint freq)
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
cpufreq_utils_get_frequency_unit (guint freq)
{
	if (freq > 999999) /* freq (kHz) */
		return g_strdup ("GHz");
	else
		return g_strdup ("MHz");
}

gboolean
cpufreq_utils_governor_is_automatic (const gchar *governor)
{
	g_return_val_if_fail (governor != NULL, FALSE);
	
	if (g_ascii_strcasecmp (governor, "userspace") == 0)
		return FALSE;

	return TRUE;
}
