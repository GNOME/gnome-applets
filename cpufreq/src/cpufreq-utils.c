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

#include <config.h>

#ifdef HAVE_POLKIT_GNOME
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <polkit/polkit.h>
#include <polkit-dbus/polkit-dbus.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

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
					 "%s", message);
	if (secondary) {
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
							  "%s", secondary);
	}
	
	gtk_window_set_title (GTK_WINDOW (dialog), ""); /* as per HIG */
	gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
	g_signal_connect (G_OBJECT (dialog),
			  "response",
			  G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_widget_show (dialog);
}

#ifdef HAVE_POLKIT_GNOME
#define CACHE_VALIDITY_SEC 2

static gboolean
pk_io_watch_have_data (GIOChannel    *channel,
		       GIOCondition   condition,
		       PolKitContext *pk_context)
{
	polkit_context_io_func (pk_context,
				g_io_channel_unix_get_fd (channel));
	return TRUE;
}

static int
pk_add_io_watch (PolKitContext *pk_context,
		 int            watch_id)
{
	GIOChannel *channel;
	guint       id = 0;

	channel = g_io_channel_unix_new (watch_id);
	if (!channel)
		return 0;

	id = g_io_add_watch (channel, G_IO_IN,
			     (GIOFunc)pk_io_watch_have_data,
			     pk_context);
	g_io_channel_unref (channel);

	return id;
}

static void
pk_remove_io_watch (PolKitContext *pk_context,
		    int            watch_id)
{
	g_source_remove (watch_id);
}

static gboolean
selector_is_available (const gchar *action)
{
	static DBusGConnection *system_bus = NULL;
	static PolKitContext   *pk_context = NULL;
	PolKitCaller           *pk_caller;
	PolKitAction           *pk_action;
	PolKitResult            pk_result;
	PolKitError            *pk_error = NULL;
	GError                 *error = NULL;
	DBusError               dbus_error;

	if (!system_bus) {
		system_bus = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
		if (!system_bus) {
			g_warning ("%s", error->message);
			g_error_free (error);

			return FALSE;
		}
	}

	if (!pk_context) {
		PolKitError *pk_error = NULL;
		
		pk_context = polkit_context_new ();
		polkit_context_set_io_watch_functions (pk_context,
						       pk_add_io_watch,
						       pk_remove_io_watch);
		if (!polkit_context_init (pk_context, &pk_error)) {
			polkit_context_unref (pk_context);
			pk_context = NULL;

			if (polkit_error_is_set (pk_error)) {
				g_warning ("%s", polkit_error_get_error_message (pk_error));
				polkit_error_free (pk_error);
			} else {
				g_warning ("Cannot initialize libpolkit");
			}

			return FALSE;
		}
	}

	dbus_error_init (&dbus_error);
	pk_caller = polkit_caller_new_from_pid (dbus_g_connection_get_connection (system_bus),
						getpid (), &dbus_error);
	if (!pk_caller) {
		g_warning ("Cannot get caller from dbus name");

		return FALSE;
	}

	pk_action = polkit_action_new ();
	polkit_action_set_action_id (pk_action, action);
	pk_result = polkit_context_is_caller_authorized (pk_context,
							 pk_action, pk_caller,
							 FALSE, &pk_error);

	polkit_caller_unref (pk_caller);
	polkit_action_unref (pk_action);
	
	if (polkit_error_is_set (pk_error)) {
		g_warning ("%s", polkit_error_get_error_message (pk_error));
		polkit_error_free (pk_error);

		return FALSE;
	}

	return !(pk_result == POLKIT_RESULT_UNKNOWN || pk_result == POLKIT_RESULT_NO);
}

gboolean
cpufreq_utils_selector_is_available (void)
{
	static gboolean cache = FALSE;
	static time_t   last_refreshed = 0;
	time_t          now;

	time (&now);
	if (ABS (now - last_refreshed) > CACHE_VALIDITY_SEC) {
		cache = selector_is_available ("org.gnome.cpufreqselector");
		last_refreshed = now;
	}

	return cache;
}
#else /* !HAVE_POLKIT_GNOME */
gboolean
cpufreq_utils_selector_is_available (void)
{
	struct stat *info;
	gchar       *path = NULL;

	path = g_find_program_in_path ("cpufreq-selector");
	if (!path)
		return FALSE;

	if (geteuid () == 0) {
		g_free (path);
		return TRUE;
	}

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
#endif /* HAVE_POLKIT_GNOME */

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

gboolean
cpufreq_file_get_contents (const gchar *filename,
			   gchar      **contents,
			   gsize       *length,
			   GError     **error)
{
	gint     fd;
	GString *buffer = NULL;
	gchar   *display_filename;
	
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (contents != NULL, FALSE);

	display_filename = g_filename_display_name (filename);

	*contents = NULL;
	if (length)
		*length = 0;

	fd = open (filename, O_RDONLY);
	if (fd < 0) {
		gint save_errno = errno;

		g_set_error (error,
			     G_FILE_ERROR,
			     g_file_error_from_errno (save_errno),
			     "Failed to open file '%s': %s",
			     display_filename,
			     g_strerror (save_errno));
		g_free (display_filename);

		return FALSE;
	}

	while (TRUE) {
		ssize_t bytes_read;
		gchar  buf[1024];
		
		bytes_read = read (fd, buf, sizeof (buf));
		if (bytes_read < 0) { /* Error */
			if (errno != EINTR) {
				gint save_errno = errno;
				
				g_set_error (error,
					     G_FILE_ERROR,
					     g_file_error_from_errno (save_errno),
					     "Failed to read from file '%s': %s",
					     display_filename,
					     g_strerror (save_errno));

				if (buffer)
					g_string_free (buffer, TRUE);
				
				g_free (display_filename);
				close (fd);
				
				return FALSE;
			}
		} else if (bytes_read == 0) { /* EOF */
			break;
		} else {
			if (!buffer)
				buffer = g_string_sized_new (bytes_read);
			buffer = g_string_append_len (buffer, buf, bytes_read);
		}
	}

	g_free (display_filename);

	if (buffer)
		*contents = g_string_free (buffer, FALSE);
	
	if (length)
		*length = strlen (*contents);

	close (fd);

	return TRUE;
}
