/*
 * Copyright (C) 2004 Carlos García Campos
 * Copyright (C) 2016 Alberts Muktupāvels
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Alberts Muktupāvels <alberts.muktupavels@gmail.com>
 *     Carlos García Campos <carlosgc@gnome.org>
 */

#include "config.h"

#include <cpufreq.h>
#include <stdlib.h>

#include "cpufreq-monitor.h"

#define CPUFREQ_MONITOR_INTERVAL 1

#ifdef HAVE_GET_FREQUENCIES
typedef struct cpufreq_frequencies CPUFreqFrequencyList;
#define cpufreq_get_available_frequencies(cpu) cpufreq_get_frequencies ("available", cpu)
#define cpufreq_put_available_frequencies(first) cpufreq_put_frequencies (first)
#else
typedef struct cpufreq_available_frequencies CPUFreqFrequencyList;
#endif

typedef struct cpufreq_policy                CPUFreqPolicy;
typedef struct cpufreq_available_governors   CPUFreqGovernorList;

struct _CPUFreqMonitor
{
  GObject   parent;

  guint     cpu;
  gboolean  online;
  gint      cur_freq;
  gint      max_freq;
  gchar    *governor;
  GList    *available_freqs;
  GList    *available_govs;
  guint     timeout_handler;

  gboolean  changed;
};

enum
{
  PROP_0,
  PROP_CPU,
};

enum
{
  SIGNAL_CHANGED,
  N_SIGNALS
};

static guint signals[N_SIGNALS];

G_DEFINE_TYPE (CPUFreqMonitor, cpufreq_monitor, G_TYPE_OBJECT)

#ifdef HAVE_IS_CPU_ONLINE
extern int cpupower_is_cpu_online (unsigned int cpu);
#endif

static gboolean
monitor_run (CPUFreqMonitor *monitor)
{
  CPUFreqPolicy *policy;
  gint freq;

  policy = cpufreq_get_policy (monitor->cpu);

  if (!policy)
    {
      /* Check whether it failed because cpu is not online. */
#ifdef HAVE_IS_CPU_ONLINE
      if (cpupower_is_cpu_online (monitor->cpu) != 1)
#else
      if (!cpufreq_cpu_exists (monitor->cpu))
#endif
        {
          monitor->online = FALSE;
          return TRUE;
        }

      return FALSE;
    }

  monitor->online = TRUE;

  freq = cpufreq_get_freq_kernel (monitor->cpu);
  if (freq != monitor->cur_freq)
    {
      monitor->cur_freq = freq;
      monitor->changed = TRUE;
    }

  if (monitor->governor)
    {
      if (g_ascii_strcasecmp (monitor->governor, policy->governor) != 0)
        {
          g_free (monitor->governor);

          monitor->governor = g_strdup (policy->governor);
          monitor->changed = TRUE;
        }
    }
  else
    {
      monitor->governor = g_strdup (policy->governor);
      monitor->changed = TRUE;
    }

  cpufreq_put_policy (policy);

  return TRUE;
}

static gboolean
cpufreq_monitor_run_cb (gpointer user_data)
{
  CPUFreqMonitor *monitor;
  gboolean retval;

  monitor = CPUFREQ_MONITOR (user_data);
  retval = monitor_run (monitor);

  if (monitor->changed)
    {
      g_signal_emit (monitor, signals[SIGNAL_CHANGED], 0);
      monitor->changed = FALSE;
    }

  return retval;
}

static gint
compare (gconstpointer a,
         gconstpointer b)
{
  gint aa;
  gint bb;

  aa = atoi ((gchar *) a);
  bb = atoi ((gchar *) b);

  if (aa == bb)
    return 0;
  else if (aa > bb)
    return -1;
  else
    return 1;
}

static void
cpufreq_monitor_constructed (GObject *object)
{
  CPUFreqMonitor *monitor;
  gulong max_freq;
  gulong min_freq;

  monitor = CPUFREQ_MONITOR (object);

  G_OBJECT_CLASS (cpufreq_monitor_parent_class)->constructed (object);

  if (cpufreq_get_hardware_limits (monitor->cpu, &min_freq, &max_freq) != 0)
    {
      g_warning ("Error getting CPUINFO_MAX");
      max_freq = -1;
    }

  monitor->max_freq = max_freq;
}

static void
cpufreq_monitor_finalize (GObject *object)
{
  CPUFreqMonitor *monitor;

  monitor = CPUFREQ_MONITOR (object);

  if (monitor->timeout_handler > 0)
    {
      g_source_remove (monitor->timeout_handler);
      monitor->timeout_handler = 0;
    }

  if (monitor->governor)
    {
      g_free (monitor->governor);
      monitor->governor = NULL;
    }

  if (monitor->available_freqs)
    {
      g_list_free_full (monitor->available_freqs, g_free);
      monitor->available_freqs = NULL;
    }

  if (monitor->available_govs)
    {
      g_list_free_full (monitor->available_govs, g_free);
      monitor->available_govs = NULL;
    }

  G_OBJECT_CLASS (cpufreq_monitor_parent_class)->finalize (object);
}

static void
cpufreq_monitor_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *spec)
{
  CPUFreqMonitor *monitor;

  monitor = CPUFREQ_MONITOR (object);

  switch (prop_id)
    {
      case PROP_CPU:
        g_value_set_uint (value, monitor->cpu);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
        break;
    }
}

static void
cpufreq_monitor_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *spec)
{
  CPUFreqMonitor *monitor;

  monitor = CPUFREQ_MONITOR (object);

  switch (prop_id)
    {
      case PROP_CPU:
        cpufreq_monitor_set_cpu (monitor, g_value_get_uint (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
        break;
    }
}

static void
cpufreq_monitor_class_init (CPUFreqMonitorClass *monitor_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (monitor_class);

  object_class->constructed = cpufreq_monitor_constructed;
  object_class->finalize = cpufreq_monitor_finalize;
  object_class->get_property = cpufreq_monitor_get_property;
  object_class->set_property = cpufreq_monitor_set_property;

  signals[SIGNAL_CHANGED] =
    g_signal_new ("changed", G_TYPE_FROM_CLASS (monitor_class),
                  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  g_object_class_install_property (object_class, PROP_CPU,
                                   g_param_spec_uint ("cpu", "", "",
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_CONSTRUCT |
                                                      G_PARAM_READWRITE));
}

static void
cpufreq_monitor_init (CPUFreqMonitor *monitor)
{
}

CPUFreqMonitor *
cpufreq_monitor_new (guint cpu)
{
  return g_object_new (CPUFREQ_TYPE_MONITOR, "cpu", cpu, NULL);
}

void
cpufreq_monitor_run (CPUFreqMonitor *monitor)
{
  g_return_if_fail (CPUFREQ_IS_MONITOR (monitor));

  if (monitor->timeout_handler > 0)
    return;

  monitor->timeout_handler = g_timeout_add_seconds (CPUFREQ_MONITOR_INTERVAL,
                                                    cpufreq_monitor_run_cb,
                                                    monitor);
}

GList *
cpufreq_monitor_get_available_frequencies (CPUFreqMonitor *monitor)
{
  CPUFreqFrequencyList *freqs;
  CPUFreqFrequencyList *freq;

  g_return_val_if_fail (CPUFREQ_IS_MONITOR (monitor), NULL);

  if (!monitor->online)
    return NULL;

  if (monitor->available_freqs)
    return monitor->available_freqs;

  freqs = cpufreq_get_available_frequencies (monitor->cpu);

  if (!freqs)
    return NULL;

  for (freq = freqs; freq; freq = freq->next)
    {
      gchar *frequency;

      frequency = g_strdup_printf ("%lu", freq->frequency);

      if (!g_list_find_custom (monitor->available_freqs, frequency, compare))
        {
          monitor->available_freqs = g_list_prepend (monitor->available_freqs,
                                                     g_strdup (frequency));
        }

      g_free (frequency);
    }

  monitor->available_freqs = g_list_sort (monitor->available_freqs, compare);
  cpufreq_put_available_frequencies (freqs);

  return monitor->available_freqs;
}

GList *
cpufreq_monitor_get_available_governors (CPUFreqMonitor *monitor)
{
  CPUFreqGovernorList *govs;
  CPUFreqGovernorList *gov;

  g_return_val_if_fail (CPUFREQ_IS_MONITOR (monitor), NULL);

  if (!monitor->online)
    return NULL;

  if (monitor->available_govs)
    return monitor->available_govs;

  govs = cpufreq_get_available_governors (monitor->cpu);

  if (!govs)
    return NULL;

  for (gov = govs; gov; gov = gov->next)
    {
      monitor->available_govs = g_list_prepend (monitor->available_govs,
                                                g_strdup (gov->governor));
    }

  cpufreq_put_available_governors (govs);

  return monitor->available_govs;
}

guint
cpufreq_monitor_get_cpu (CPUFreqMonitor *monitor)
{
  g_return_val_if_fail (CPUFREQ_IS_MONITOR (monitor), 0);

  return monitor->cpu;
}

void
cpufreq_monitor_set_cpu (CPUFreqMonitor *monitor,
                         guint           cpu)
{
  g_return_if_fail (CPUFREQ_IS_MONITOR (monitor));

  if (cpu != monitor->cpu)
    {
      monitor->cpu = cpu;
      monitor->changed = TRUE;
    }
}

const gchar *
cpufreq_monitor_get_governor (CPUFreqMonitor *monitor)
{
  g_return_val_if_fail (CPUFREQ_IS_MONITOR (monitor), NULL);

  return monitor->governor;
}

gint
cpufreq_monitor_get_frequency (CPUFreqMonitor *monitor)
{
  g_return_val_if_fail (CPUFREQ_IS_MONITOR (monitor), -1);

  return monitor->cur_freq;
}

gint
cpufreq_monitor_get_percentage (CPUFreqMonitor *monitor)
{
  g_return_val_if_fail (CPUFREQ_IS_MONITOR (monitor), -1);

  if (monitor->max_freq > 0)
    return ((monitor->cur_freq * 100) / monitor->max_freq);

  return -1;
}

gboolean
cpufreq_monitor_get_hardware_limits (CPUFreqMonitor *monitor,
                                     gulong         *min,
                                     gulong         *max)
{
  g_return_val_if_fail (CPUFREQ_IS_MONITOR (monitor), FALSE);

  if (!monitor->online)
    return FALSE;

  if (cpufreq_get_hardware_limits (monitor->cpu, min, max) != 0)
    return FALSE;

  return TRUE;
}
