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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors : Carlos García Campos <carlosgc@gnome.org>
 */

#include <config.h>

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

	if (mcpu > 0) {
		n_cpus = (guint)mcpu;
		return mcpu;
	}

	n_cpus = 1;

	return 1;
}

#define CACHE_VALIDITY_SEC 2

static gboolean
selector_is_available (void)
{
	GDBusProxy             *proxy;
	static GDBusConnection *system_bus = NULL;
	GError                 *error = NULL;
	GVariant               *reply;
	gboolean                result;

	if (!system_bus) {
		system_bus = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, &error);
		if (!system_bus) {
			g_warning ("%s", error->message);
			g_error_free (error);

			return FALSE;
		}
	}

	proxy = g_dbus_proxy_new_sync (system_bus,
	                               G_DBUS_PROXY_FLAGS_NONE,
	                               NULL,
	                               "org.gnome.CPUFreqSelector",
	                               "/org/gnome/cpufreq_selector/selector",
	                               "org.gnome.CPUFreqSelector",
	                               NULL,
	                               &error);

	if (!proxy) {
		g_warning ("%s", error->message);
		g_error_free (error);

		return FALSE;
	}

	reply = g_dbus_proxy_call_sync (proxy, "CanSet", NULL,
	                                G_DBUS_CALL_FLAGS_NONE,
	                                -1, NULL, &error);

	if (!reply) {
		g_warning ("Error calling org.gnome.CPUFreqSelector.CanSet: %s", error->message);
		g_error_free (error);
		result = FALSE;
	} else {
	        g_variant_get (reply, "(b)", &result);
		g_variant_unref (reply);
	}

	g_object_unref (proxy);

	return result;
}

gboolean
cpufreq_utils_selector_is_available (void)
{
	static gboolean cache = FALSE;
	static time_t   last_refreshed = 0;
	time_t          now;

	time (&now);
	if (ABS (now - last_refreshed) > CACHE_VALIDITY_SEC) {
		cache = selector_is_available ();
		last_refreshed = now;
	}

	return cache;
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
		return g_strdup_printf ("%3.2f", ((double) freq / divisor));
}

gchar *
cpufreq_utils_get_frequency_unit (guint freq)
{
	if (freq > 999999) /* freq (kHz) */
		return g_strdup ("GHz");
	else
		return g_strdup ("MHz");
}
