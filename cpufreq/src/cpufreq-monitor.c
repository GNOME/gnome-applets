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

#include "cpufreq-monitor.h"
#include "cpufreq-monitor-protected.h"

#define PARENT_TYPE G_TYPE_OBJECT

#define CPUFREQ_MONITOR_GET_PROTECTED(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TYPE_CPUFREQ_MONITOR, CPUFreqMonitorProtected))

enum {
	   PROP_0,
	   PROP_CPU
};

static void   cpufreq_monitor_init                    (CPUFreqMonitor *monitor);
static void   cpufreq_monitor_class_init              (CPUFreqMonitorClass *klass);
static void   cpufreq_monitor_finalize                (GObject *object);

static void   cpufreq_monitor_set_property            (GObject  *object, guint prop_id,
											const GValue *value, GParamSpec *spec);
static void   cpufreq_monitor_get_property            (GObject  *object, guint prop_id,
											GValue *value, GParamSpec *spec);

static gchar *cpufreq_monitor_get_human_readable_freq (gint freq);
static gchar *cpufreq_monitor_get_human_readable_unit (gint freq);
static gchar *cpufreq_monitor_get_human_readable_perc (gint fmax, gint fmin);

static void   cpufreq_monitor_free_data               (CPUFreqMonitor *monitor);

static GObjectClass *parent_class = NULL;

typedef struct _CPUFreqMonitorProtected CPUFreqMonitorProtected;

GType cpufreq_monitor_get_type ()
{
	   static GType type = 0;

	   if (!type) {
			 static const GTypeInfo info = {
				    sizeof (CPUFreqMonitorClass),
				    (GBaseInitFunc) NULL,
				    (GBaseFinalizeFunc) NULL,
				    (GClassInitFunc) cpufreq_monitor_class_init,
				    NULL,
				    NULL,
				    sizeof (CPUFreqMonitor),
				    0,
				    (GInstanceInitFunc) cpufreq_monitor_init
			 };

			 type = g_type_register_static (PARENT_TYPE, "CPUFreqMonitor",
									  &info, G_TYPE_FLAG_ABSTRACT);
	   }

	   return type;
}

static void
cpufreq_monitor_init (CPUFreqMonitor *monitor)
{
	   CPUFreqMonitorProtected *private;

	   g_return_if_fail (IS_CPUFREQ_MONITOR (monitor));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (monitor);
	   private->freq = NULL;
	   private->perc = NULL;
	   private->unit = NULL;
	   private->governor = NULL;
	   private->available_freqs = NULL;
	   private->timeout_handler = 0;
}

static void
cpufreq_monitor_class_init (CPUFreqMonitorClass *klass)
{
	   GObjectClass        *object_class = G_OBJECT_CLASS (klass);
	   CPUFreqMonitorClass *monitor_class = CPUFREQ_MONITOR_CLASS (klass);

	   parent_class = g_type_class_peek_parent (klass);
	   
	   object_class->set_property = cpufreq_monitor_set_property;
	   object_class->get_property = cpufreq_monitor_get_property;

	   /* Public virtual methods */
	   monitor_class->run = NULL;
	   monitor_class->get_available_frequencies = NULL;

	   /* Protected methods */
	   monitor_class->get_human_readable_freq = cpufreq_monitor_get_human_readable_freq;
	   monitor_class->get_human_readable_unit = cpufreq_monitor_get_human_readable_unit;
	   monitor_class->get_human_readable_perc = cpufreq_monitor_get_human_readable_perc;

	   monitor_class->free_data = cpufreq_monitor_free_data;

	   /* Porperties */
	   g_object_class_install_property (object_class, PROP_CPU,
								 g_param_spec_uint ("cpu", NULL, NULL,
												0, /* MIN_UINT */
												G_MAXUINT,
												0, /* Default */
												G_PARAM_CONSTRUCT | G_PARAM_WRITABLE ));

	   /* Protected attributes */
	   g_type_class_add_private (klass, sizeof (CPUFreqMonitorProtected));

	   /* Signals */
	   monitor_class->signals[CHANGED] =
			 g_signal_new ("changed",
						G_TYPE_FROM_CLASS (klass),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET (CPUFreqMonitorClass, changed),
						NULL, NULL,
						g_cclosure_marshal_VOID__VOID,
						G_TYPE_NONE, 0);
	   
	   object_class->finalize = cpufreq_monitor_finalize;
}

static void
free_string (gpointer str, gpointer gdata)
{
	   if (str) g_free (str);
}

static void
cpufreq_monitor_finalize (GObject *object)
{
	   CPUFreqMonitorProtected *private;

	   g_return_if_fail (IS_CPUFREQ_MONITOR (object));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (object);

	   if (private->timeout_handler > 0)
			 g_source_remove (private->timeout_handler);

	   cpufreq_monitor_free_data (CPUFREQ_MONITOR (object));

	   if (private->available_freqs) {
			 g_list_foreach (private->available_freqs,
						  free_string, NULL);
			 g_list_free (private->available_freqs);
			 private->available_freqs = NULL;
	   }

	   if (G_OBJECT_CLASS (parent_class)->finalize)
			 (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static gchar *
cpufreq_monitor_get_human_readable_freq (gint freq)
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

static gchar *
cpufreq_monitor_get_human_readable_unit (gint freq)
{
	   if (freq > 999999) /* freq (kHz) */
			 return g_strdup ("GHz");
	   else
			 return g_strdup ("MHz");
}

static gchar *
cpufreq_monitor_get_human_readable_perc (gint fmax, gint fmin)
{
	   if (fmax > 0)
			 return g_strdup_printf ("%d%%", (fmin * 100) / fmax);
	   else
			 return NULL;
}

static void
cpufreq_monitor_set_property (GObject  *object, guint prop_id, const GValue *value,
						GParamSpec *spec)
{
	   CPUFreqMonitor          *monitor;
	   CPUFreqMonitorProtected *private;

	   g_return_if_fail (IS_CPUFREQ_MONITOR (object));

	   monitor = CPUFREQ_MONITOR (object);
	   private = CPUFREQ_MONITOR_GET_PROTECTED (monitor);

	   switch (prop_id) {
	   case PROP_CPU:
			 private->cpu = g_value_get_uint (value);
			 break;
	   default:
			 break;
	   }
}

/* Is it neccesary?? */
static void
cpufreq_monitor_get_property (GObject  *object, guint prop_id, GValue *value,
						GParamSpec *spec)
{
	   CPUFreqMonitor          *monitor;
	   CPUFreqMonitorProtected *private;

	   g_return_if_fail (IS_CPUFREQ_MONITOR (object));

	   monitor = CPUFREQ_MONITOR (object);
	   private = CPUFREQ_MONITOR_GET_PROTECTED (monitor);

	   switch (prop_id) {
	   case PROP_CPU:
			 g_value_set_uint (value, private->cpu);
			 break;
	   default:
			 G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
	   }
}

static void
cpufreq_monitor_free_data (CPUFreqMonitor *monitor)
{
	   CPUFreqMonitorProtected *private;
	   
	   g_return_if_fail (IS_CPUFREQ_MONITOR (monitor));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (monitor);
	   
	   if (private->freq) {
			 g_free (private->freq);
			 private->freq = NULL;
	   }

	   if (private->perc) {
			 g_free (private->perc);
			 private->perc = NULL;
	   }

	   if (private->unit) {
			 g_free (private->unit);
			 private->unit = NULL;
	   }

	   if (private->governor) {
			 g_free (private->governor);
			 private->governor = NULL;
	   }
}

void
cpufreq_monitor_run (CPUFreqMonitor *monitor)
{
	   g_return_if_fail (IS_CPUFREQ_MONITOR (monitor));

	   if (CPUFREQ_MONITOR_GET_CLASS (monitor)->run) {
			 return CPUFREQ_MONITOR_GET_CLASS (monitor)->run (monitor);
	   } else {
			 return;
	   }
}

GList *
cpufreq_monitor_get_available_frequencies (CPUFreqMonitor *monitor)
{
	   g_return_if_fail (IS_CPUFREQ_MONITOR (monitor));

	   if (CPUFREQ_MONITOR_GET_CLASS (monitor)->get_available_frequencies) {
			 return CPUFREQ_MONITOR_GET_CLASS (monitor)->get_available_frequencies (monitor);
	   } else {
			 return;
	   }
}

guint
cpufreq_monitor_get_cpu (CPUFreqMonitor *monitor)
{
	   CPUFreqMonitorProtected *private;
	   
	   g_return_if_fail (IS_CPUFREQ_MONITOR (monitor));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (monitor);

	   return private->cpu;
}

gchar *
cpufreq_monitor_get_governor (CPUFreqMonitor *monitor)
{
	   CPUFreqMonitorProtected *private;
	   
	   g_return_if_fail (IS_CPUFREQ_MONITOR (monitor));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (monitor);

	   return g_strdup (private->governor);
}

gchar *
cpufreq_monitor_get_frequency (CPUFreqMonitor *monitor)
{
	   CPUFreqMonitorProtected *private;
	   
	   g_return_if_fail (IS_CPUFREQ_MONITOR (monitor));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (monitor);

	   return g_strdup (private->freq);
}

gchar *
cpufreq_monitor_get_percentage (CPUFreqMonitor *monitor)
{
	   CPUFreqMonitorProtected *private;
	   
	   g_return_if_fail (IS_CPUFREQ_MONITOR (monitor));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (monitor);

	   return g_strdup (private->perc);
}

gchar *
cpufreq_monitor_get_unit (CPUFreqMonitor *monitor)
{
	   CPUFreqMonitorProtected *private;
	   
	   g_return_if_fail (IS_CPUFREQ_MONITOR (monitor));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (monitor);

	   return g_strdup (private->unit);
}

void
cpufreq_monitor_set_cpu (CPUFreqMonitor *monitor, guint cpu)
{
	   CPUFreqMonitorProtected *private;
	   
	   g_return_if_fail (IS_CPUFREQ_MONITOR (monitor));

	   private = CPUFREQ_MONITOR_GET_PROTECTED (monitor);

	   private->cpu = cpu;
}

	   
