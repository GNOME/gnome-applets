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

#include "cpufreq.h"
#include "cpufreq-sysfs.h"
#include "cpufreq-procfs.h"

gint
main (gint argc, gchar **argv)
{
        GOptionContext *context;
        static gint     cpu = 0;
        static gchar   *governor = NULL;
        static gulong   frequency = 0;
	GError         *error = NULL;
        CPUFreq        *cfq;

        static GOptionEntry options[] = {
                { "cpu",       'c',  0, G_OPTION_ARG_INT,           &cpu,            "CPU Number",       NULL },
                { "governor",  'g',  0, G_OPTION_ARG_STRING,        &governor,       "Governor",         NULL },
                { "frequency", 'f',  0, G_OPTION_ARG_INT,           &frequency,      "Frequency in KHz", NULL },
                { NULL }
        };
                   
        if (geteuid () != 0) {
                g_print ("You must be root\n");
                         
                return 1;
        }
           
	if (argc < 2) {
		g_print ("Missing operand after `cpufreq-selector'\n");
		g_print ("Try `cpufreq-selector --help' for more information.\n");

		return 1;
	}

	g_type_init ();

	context = g_option_context_new ("- CPUFreq Selector");
	g_option_context_add_main_entries (context, options, NULL);
	
	if (! g_option_context_parse (context, &argc, &argv, &error)) {
		if (error) {
			g_print ("%s\n", error->message);
			g_error_free (error);
		}
	}
	
	g_option_context_free (context);
	
        if (g_file_test ("/sys/devices/system/cpu/cpu0/cpufreq", G_FILE_TEST_EXISTS)) { /* 2.6 kernel */
                cfq = CPUFREQ (cpufreq_sysfs_new ());
        } else if (g_file_test ("/proc/cpufreq", G_FILE_TEST_EXISTS)) { /* 2.4 kernel */
                cfq = CPUFREQ (cpufreq_procfs_new ());
        } else {
                g_print ("No cpufreq support\n");
                return 1;
        }

        if (governor) {
                cpufreq_set_governor (cfq, governor);
		g_free (governor);
	}

        if (frequency != 0)
                cpufreq_set_frequency (cfq, frequency);

        g_object_unref (cfq);

        return 0;
}
