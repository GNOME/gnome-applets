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
#include <popt.h>

#include "cpufreq.h"
#include "cpufreq-sysfs.h"
#include "cpufreq-procfs.h"

gint
main (gint argc, gchar **argv)
{
	   gint        cpu = 0;
	   gchar      *governor = NULL;
	   gulong      frequency = 0;
	   poptContext ctx;
	   gint        nextopt;
	   CPUFreq    *cfq;

	   struct poptOption options[] = {
			 { NULL,        '\0', POPT_ARG_INCLUDE_TABLE, poptHelpOptions, 0, "Help options",     NULL },
			 { "cpu",       'c',  POPT_ARG_INT,           &cpu,            0, "CPU Number",       NULL },
			 { "governor",  'g',  POPT_ARG_STRING,        &governor,       0, "Governor",         NULL },
			 { "frequency", 'f',  POPT_ARG_LONG,          &frequency,      0, "Frequency in KHz", NULL },
			 { NULL,        '\0', 0,                      NULL,            0, NULL,               NULL }
	   };
	   
	   if (geteuid () != 0) {
			 g_print ("You must be root\n");
			 
			 return 1;
	   }
	   
	   g_type_init ();

	   ctx = poptGetContext ("cpufreq-selector", argc, (const gchar **) argv, options, 0);

	   poptReadDefaultConfig (ctx, TRUE);

	   if (argc <= 2) {
			 poptPrintUsage (ctx, stdout, 0);
			 return 1;
	   } 

	   while ((nextopt = poptGetNextOpt (ctx)) >= 0) {
	   }

	   if (nextopt < -1) {
			 g_printerr ("Error on option %s: %s.\nRun '%s --help' to see a "
					   "full list of available command line options.\n",
					   poptBadOption (ctx, 0),
					   poptStrerror (nextopt),
					   argv[0]);
			 return 1;
	   }

	   poptFreeContext(ctx);

	   if (g_file_test ("/sys/devices/system/cpu/cpu0/cpufreq", G_FILE_TEST_EXISTS)) { /* 2.6 kernel */
			 cfq = CPUFREQ (cpufreq_sysfs_new ());
	   } else if (g_file_test ("/proc/cpufreq", G_FILE_TEST_EXISTS)) { /* 2.4 kernel */
			 cfq = CPUFREQ (cpufreq_procfs_new ());
	   } else {
			 g_print ("No cpufreq support\n");
			 return 1;
	   }

	   if (governor)
			 cpufreq_set_governor (cfq, governor);

	   if (frequency != 0)
			 cpufreq_set_frequency (cfq, frequency);

	   g_object_unref (cfq);

	   return 0;
}
