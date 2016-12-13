/*
 * GNOME CPUFreq Applet
 * Copyright (C) 2008 Carlos Garcia Campos <carlosgc@gnome.org>
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
 */

#include "config.h"

#include <gio/gio.h>

#include "cpufreq-selector.h"

struct _CPUFreqSelector {
	GObject          parent;

	GDBusConnection *system_bus;
	GDBusProxy      *proxy;
};

struct _CPUFreqSelectorClass {
	GObjectClass parent_class;
};

G_DEFINE_TYPE (CPUFreqSelector, cpufreq_selector, G_TYPE_OBJECT)

static void
cpufreq_selector_finalize (GObject *object)
{
	CPUFreqSelector *selector = CPUFREQ_SELECTOR (object);

	g_clear_object (&selector->proxy);
	g_clear_object (&selector->system_bus);

	G_OBJECT_CLASS (cpufreq_selector_parent_class)->finalize (object);
}

static void
cpufreq_selector_class_init (CPUFreqSelectorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = cpufreq_selector_finalize;
}

static void
cpufreq_selector_init (CPUFreqSelector *selector)
{
}

CPUFreqSelector *
cpufreq_selector_get_default (void)
{
	static CPUFreqSelector *selector = NULL;

	if (!selector)
		selector = CPUFREQ_SELECTOR (g_object_new (CPUFREQ_TYPE_SELECTOR, NULL));

	return selector;
}

typedef enum {
	FREQUENCY,
	GOVERNOR
} CPUFreqSelectorCall;

typedef struct {
	CPUFreqSelector *selector;
	
	CPUFreqSelectorCall call;

	guint  cpu;
	guint  frequency;
	gchar *governor;

	guint32 parent_xid;
} SelectorAsyncData;

static void
selector_async_data_free (SelectorAsyncData *data)
{
	if (!data)
		return;

	g_free (data->governor);
	g_free (data);
}

static gboolean
cpufreq_selector_connect_to_system_bus (CPUFreqSelector *selector,
					GError         **error)
{
	if (selector->system_bus)
		return TRUE;

	selector->system_bus = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, error);

	return (selector->system_bus != NULL);
}

static gboolean
cpufreq_selector_create_proxy (CPUFreqSelector  *selector,
                               GError          **error)
{
	if (selector->proxy)
		return TRUE;

	selector->proxy = g_dbus_proxy_new_sync (selector->system_bus,
	                                         G_DBUS_PROXY_FLAGS_NONE,
	                                         NULL,
	                                         "org.gnome.CPUFreqSelector",
	                                         "/org/gnome/cpufreq_selector/selector",
	                                         "org.gnome.CPUFreqSelector",
	                                         NULL,
	                                         error);

	return (selector->proxy != NULL);
}

static void
set_frequency_cb (GObject      *source,
                  GAsyncResult *result,
                  gpointer      user_data)
{
	SelectorAsyncData *data;
	GError            *error;

	data = (SelectorAsyncData *) user_data;

	error = NULL;
	g_dbus_proxy_call_finish (G_DBUS_PROXY (source), result, &error);

	if (error != NULL) {
		g_warning ("%s", error->message);
		g_error_free (error);
	}

	selector_async_data_free (data);
}

static void
selector_set_frequency_async (SelectorAsyncData *data)
{
	GError *error = NULL;

	if (!cpufreq_selector_connect_to_system_bus (data->selector, &error)) {
		g_warning ("%s", error->message);
		g_error_free (error);

		selector_async_data_free (data);

		return;
	}

	if (!cpufreq_selector_create_proxy (data->selector, &error)) {
		g_warning ("%s", error->message);
		g_error_free (error);

		selector_async_data_free (data);

		return;
	}

	g_dbus_proxy_call (data->selector->proxy,
	                   "SetFrequency",
	                   g_variant_new ("(uu)", data->cpu, data->frequency),
	                   G_DBUS_CALL_FLAGS_NONE,
	                   G_MAXINT,
	                   NULL,
	                   set_frequency_cb,
	                   data);
}

void
cpufreq_selector_set_frequency_async (CPUFreqSelector *selector,
				      guint            cpu,
				      guint            frequency,
				      guint32          parent)
{
	SelectorAsyncData *data;

	data = g_new0 (SelectorAsyncData, 1);

	data->selector = selector;
	data->call = FREQUENCY;
	data->cpu = cpu;
	data->frequency = frequency;
	data->parent_xid = parent;

	selector_set_frequency_async (data);
}

static void
set_governor_cb (GObject      *source,
                 GAsyncResult *result,
                 gpointer      user_data)
{
	SelectorAsyncData *data;
	GError            *error;

	data = (SelectorAsyncData *) user_data;

	error = NULL;
	g_dbus_proxy_call_finish (G_DBUS_PROXY (source), result, &error);

	if (error != NULL) {
		g_warning ("%s", error->message);
		g_error_free (error);
	}

	selector_async_data_free (data);
}

static void
selector_set_governor_async (SelectorAsyncData *data)
{
	GError *error = NULL;

	if (!cpufreq_selector_connect_to_system_bus (data->selector, &error)) {
		g_warning ("%s", error->message);
		g_error_free (error);

		selector_async_data_free (data);

		return;
	}

	if (!cpufreq_selector_create_proxy (data->selector, &error)) {
		g_warning ("%s", error->message);
		g_error_free (error);

		selector_async_data_free (data);

		return;
	}

	g_dbus_proxy_call (data->selector->proxy,
	                   "SetGovernor",
	                   g_variant_new ("(us)", data->cpu, data->governor),
	                   G_DBUS_CALL_FLAGS_NONE,
	                   G_MAXINT,
	                   NULL,
	                   set_governor_cb,
	                   data);
}

void
cpufreq_selector_set_governor_async (CPUFreqSelector *selector,
				     guint            cpu,
				     const gchar     *governor,
				     guint32          parent)
{
	SelectorAsyncData *data;

	data = g_new0 (SelectorAsyncData, 1);

	data->selector = selector;
	data->call = GOVERNOR;
	data->cpu = cpu;
	data->governor = g_strdup (governor);
	data->parent_xid = parent;

	selector_set_governor_async (data);
}
