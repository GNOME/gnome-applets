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
#include "cpufreq.h"

#define PARENT_TYPE G_TYPE_OBJECT

#define CPUFREQ_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TYPE_CPUFREQ, CPUFreqPrivate))

static void cpufreq_init         (CPUFreq *cfq);
static void cpufreq_class_init   (CPUFreqClass *klass);
static void cpufreq_finalize     (GObject *object);

static void cpufreq_set_property (GObject *object, guint prop_id,
						    const GValue *value, GParamSpec *spec);
static void cpufreq_get_property (GObject *object, guint prop_id,
						    GValue *value, GParamSpec *spec);

static GObjectClass *parent_class = NULL;

enum {
	   PROP_0,
	   PROP_N_CPU,
	   PROP_SC_MAX,
	   PROP_SC_MIN,
	   PROP_GOVERNOR
};

typedef struct _CPUFreqPrivate CPUFreqPrivate;

struct _CPUFreqPrivate
{
	   guint  n_cpu;
	   gint   sc_max;
	   gint   sc_min;
	   gchar *governor;
};

GType cpufreq_get_type ()
{
	   static GType type = 0;

	   if (!type) {
			 static const GTypeInfo info = {
				    sizeof (CPUFreqClass),
				    (GBaseInitFunc) NULL,
				    (GBaseFinalizeFunc) NULL,
				    (GClassInitFunc) cpufreq_class_init,
				    NULL,
				    NULL,
				    sizeof (CPUFreq),
				    0,
				    (GInstanceInitFunc) cpufreq_init
			 };

			 type = g_type_register_static (PARENT_TYPE, "CPUFreq",
									  &info, G_TYPE_FLAG_ABSTRACT);
	   }

	   return type;
}

static void
cpufreq_init (CPUFreq *cfq)
{
	   CPUFreqPrivate *private;
	   
	   g_return_if_fail (IS_CPUFREQ (cfq));

	   private = CPUFREQ_GET_PRIVATE (cfq);
	   private->n_cpu = 0;
	   private->sc_max = 0;
	   private->sc_min = 0;
	   private->governor = NULL;
}

static void
cpufreq_class_init (CPUFreqClass *klass)
{
	   GObjectClass *object_class = G_OBJECT_CLASS (klass);

	   parent_class = g_type_class_peek_parent (klass);

	   g_type_class_add_private (klass, sizeof (CPUFreqPrivate));

	   object_class->set_property = cpufreq_set_property;
	   object_class->get_property = cpufreq_get_property;

	   g_object_class_install_property (object_class, PROP_N_CPU,
								 g_param_spec_uint ("n_cpu", NULL, NULL,
												0, G_MAXUINT, 0,
												G_PARAM_READWRITE));
	   g_object_class_install_property (object_class, PROP_SC_MAX,
								 g_param_spec_int ("sc_max", NULL, NULL,
											    0, G_MAXINT, 0,
											    G_PARAM_READWRITE));
	   g_object_class_install_property (object_class, PROP_SC_MIN,
								 g_param_spec_int ("sc_min", NULL, NULL,
											    0, G_MAXINT, 0,
											    G_PARAM_READWRITE));
	   g_object_class_install_property (object_class, PROP_GOVERNOR,
								 g_param_spec_string ("governor", NULL, NULL,
												  NULL, G_PARAM_READWRITE));
	   
	   object_class->finalize = cpufreq_finalize;
}

static void
cpufreq_finalize (GObject *object)
{
	   CPUFreqPrivate *private;
	   
	   g_return_if_fail (IS_CPUFREQ (object));

	   private = CPUFREQ_GET_PRIVATE (object);

	   if (private->governor) {
			 g_free (private->governor);
			 private->governor = NULL;
	   }
					   
	   if (G_OBJECT_CLASS (parent_class)->finalize)
			 (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
cpufreq_set_property (GObject *object, guint prop_id, const GValue *value,
				  GParamSpec *spec)
{
	   CPUFreqPrivate *private;

	   g_return_if_fail (IS_CPUFREQ (object));

	   private = CPUFREQ_GET_PRIVATE (object);

	   switch (prop_id) {
	   case PROP_N_CPU:
			 private->n_cpu = g_value_get_uint (value);
			 break;
	   case PROP_SC_MAX:
			 private->sc_max = g_value_get_int (value);
			 break;
	   case PROP_SC_MIN:
			 private->sc_min = g_value_get_int (value);
			 break;
	   case PROP_GOVERNOR:
			 if (private->governor) g_free (private->governor);
			 private->governor = g_value_dup_string (value);
			 break;
	   default:
			 break;
	   }
}

static void
cpufreq_get_property (GObject  *object, guint prop_id, GValue *value,
				  GParamSpec *spec)
{
	   CPUFreqPrivate *private;

	   g_return_if_fail (IS_CPUFREQ (object));

	   private = CPUFREQ_GET_PRIVATE (object);

	   switch (prop_id) {
	   case PROP_N_CPU:
			 g_value_set_uint (value, private->n_cpu);
			 break;
	   case PROP_SC_MAX:
			 g_value_set_int (value, private->sc_max);
			 break;
	   case PROP_SC_MIN:
			 g_value_set_int (value, private->sc_min);
			 break;
	   case PROP_GOVERNOR:
			 g_value_set_string (value, private->governor);
			 break;
	   default:
			 G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
	   }
}

void
cpufreq_set_governor (CPUFreq *cfq, const gchar *governor)
{
	   g_return_if_fail (IS_CPUFREQ (cfq));

	   if (CPUFREQ_GET_CLASS (cfq)->set_governor) {
			 return CPUFREQ_GET_CLASS (cfq)->set_governor (cfq, governor);
	   } else {
			 return;
	   }
}

void
cpufreq_set_frequency (CPUFreq *cfq, const gint frequency)
{
	   g_return_if_fail (IS_CPUFREQ (cfq));

	   if (CPUFREQ_GET_CLASS (cfq)->set_frequency) {
			 return CPUFREQ_GET_CLASS (cfq)->set_frequency (cfq, frequency);
	   } else {
			 return;
	   }
}
