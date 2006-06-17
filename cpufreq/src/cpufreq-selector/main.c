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

#include <glib-object.h>
#include <sys/types.h>
#include <unistd.h>

#include "cpufreq-selector.h"
#include "cpufreq-selector-sysfs.h"
#include "cpufreq-selector-procfs.h"

static gint    cpu = 0;
static gchar  *governor = NULL;
static gulong  frequency = 0;

static const GOptionEntry options[] = {
	{ "cpu",       'c', 0, G_OPTION_ARG_INT,    &cpu,       "CPU Number",       NULL },
	{ "governor",  'g', 0, G_OPTION_ARG_STRING, &governor,  "Governor",         NULL },
	{ "frequency", 'f', 0, G_OPTION_ARG_INT,    &frequency, "Frequency in KHz", NULL },
	{ NULL }
};

gint
main (gint argc, gchar **argv)
{
	CPUFreqSelector *selector;
        GOptionContext  *context;
	GError          *error = NULL;

        if (geteuid () != 0) {
                g_printerr ("You must be root\n");
                         
                return 1;
        }
           
	if (argc < 2) {
		g_printerr ("Missing operand after `cpufreq-selector'\n");
		g_printerr ("Try `cpufreq-selector --help' for more information.\n");

		return 1;
	}

	g_type_init ();

	context = g_option_context_new ("- CPUFreq Selector");
	g_option_context_add_main_entries (context, options, NULL);
	
	if (! g_option_context_parse (context, &argc, &argv, &error)) {
		if (error) {
			g_printerr ("%s\n", error->message);
			g_error_free (error);
			error = NULL;
		}
	}
	
	g_option_context_free (context);
	
        if (g_file_test ("/sys/devices/system/cpu/cpu0/cpufreq", G_FILE_TEST_EXISTS)) { /* 2.6 kernel */
                selector = cpufreq_selector_sysfs_new (cpu);
        } else if (g_file_test ("/proc/cpufreq", G_FILE_TEST_EXISTS)) { /* 2.4 kernel */
                selector = cpufreq_selector_procfs_new (cpu);
        } else {
                g_printerr ("No cpufreq support\n");
                return 1;
        }

        if (governor) {
                cpufreq_selector_set_governor (selector, governor, &error);

		if (error) {
			g_printerr ("%s\n", error->message);
			g_error_free (error);
			error = NULL;
		}
	}

        if (frequency != 0) {
                cpufreq_selector_set_frequency (selector, frequency, &error);

		if (error) {
			g_printerr ("%s\n", error->message);
			g_error_free (error);
			error = NULL;
		}
	}
	
        g_object_unref (selector);

        return 0;
}
