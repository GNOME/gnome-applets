/*
 * Copyright (C) 2004 Carlos Garcia Campos
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

#include "cpufreq-selector.h"

typedef enum
{
  CPUFREQ_SELECTOR_ERROR_INVALID_CPU,
  CPUFREQ_SELECTOR_ERROR_INVALID_GOVERNOR,
  CPUFREQ_SELECTOR_ERROR_SET_FREQUENCY
} CPUFreqSelectorError;

#define CPUFREQ_SELECTOR_ERROR cpufreq_selector_error_quark ()

#ifdef HAVE_GET_FREQUENCIES
typedef struct cpufreq_frequencies CPUFreqFrequencyList;
#define cpufreq_get_available_frequencies(cpu) cpufreq_get_frequencies ("available", cpu)
#define cpufreq_put_available_frequencies(first) cpufreq_put_frequencies (first)
#else
typedef struct cpufreq_available_frequencies CPUFreqFrequencyList;
#endif

typedef struct cpufreq_policy                CPUFreqPolicy;
typedef struct cpufreq_available_governors   CPUFreqGovernorList;

struct _CPUFreqSelector
{
  GObject parent;

  guint   cpu;
};

enum
{
  PROP_0,
  PROP_CPU
};

G_DEFINE_TYPE (CPUFreqSelector, cpufreq_selector, G_TYPE_OBJECT)

static GQuark
cpufreq_selector_error_quark (void)
{
  static GQuark error_quark = 0;

  if (error_quark == 0)
    error_quark = g_quark_from_static_string ("cpufreq-selector-error-quark");

  return error_quark;
}

static guint
cpufreq_selector_get_valid_frequency (CPUFreqSelector *selector,
                                      guint            frequency)
{
  gint dist;
  guint retval;
  CPUFreqFrequencyList *freqs;
  CPUFreqFrequencyList *freq;

  dist = G_MAXINT;
  retval = 0;

  freqs = cpufreq_get_available_frequencies (selector->cpu);

  if (!freqs)
    return 0;

  for (freq = freqs; freq; freq = freq->next)
    {
      gint current_dist;

      if (freq->frequency == frequency)
        {
          cpufreq_put_available_frequencies (freqs);

          return frequency;
        }

      if (freq->frequency > frequency)
        current_dist = freq->frequency - frequency;
      else
        current_dist = frequency - freq->frequency;

      if (current_dist < dist)
        {
          dist = current_dist;
          retval = freq->frequency;
        }
    }

  return retval;
}

static gboolean
cpufreq_selector_validate_governor (CPUFreqSelector *selector,
                                    const gchar     *governor)
{
  CPUFreqGovernorList *govs;
  CPUFreqGovernorList *gov;

  govs = cpufreq_get_available_governors (selector->cpu);
  if (!govs)
    return FALSE;

  for (gov = govs; gov; gov = gov->next)
    {
      if (g_ascii_strcasecmp (gov->governor, governor) == 0)
        {
          cpufreq_put_available_governors (govs);

          return TRUE;
        }
    }

  cpufreq_put_available_governors (govs);

  return FALSE;
}

static void
cpufreq_selector_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *spec)
{
  CPUFreqSelector *selector;

  selector = CPUFREQ_SELECTOR (object);

  switch (prop_id)
    {
      case PROP_CPU:
        g_value_set_uint (value, selector->cpu);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
        break;
    }
}

static void
cpufreq_selector_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *spec)
{
  CPUFreqSelector *selector;

  selector = CPUFREQ_SELECTOR (object);

  switch (prop_id)
    {
      case PROP_CPU:
        selector->cpu = g_value_get_uint (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, spec);
        break;
    }
}

static void
cpufreq_selector_class_init (CPUFreqSelectorClass *selector_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (selector_class);

  object_class->get_property = cpufreq_selector_get_property;
  object_class->set_property = cpufreq_selector_set_property;

  g_object_class_install_property (object_class, PROP_CPU,
                                   g_param_spec_uint ("cpu", NULL, NULL,
                                                      0, G_MAXUINT, 0,
                                                      G_PARAM_CONSTRUCT_ONLY |
                                                      G_PARAM_READWRITE));
}

static void
cpufreq_selector_init (CPUFreqSelector *selector)
{
}

CPUFreqSelector *
cpufreq_selector_new (guint cpu)
{
  return g_object_new (CPUFREQ_TYPE_SELECTOR, "cpu", cpu, NULL);
}

gboolean
cpufreq_selector_set_frequency (CPUFreqSelector  *selector,
                                guint             frequency,
                                GError          **error)
{
  guint freq;

  g_return_val_if_fail (CPUFREQ_IS_SELECTOR (selector), FALSE);
  g_return_val_if_fail (frequency > 0, FALSE);

  freq = cpufreq_selector_get_valid_frequency (selector, frequency);

  if (cpufreq_set_frequency (selector->cpu, freq) != 0)
    {
      g_set_error (error, CPUFREQ_SELECTOR_ERROR,
                   CPUFREQ_SELECTOR_ERROR_SET_FREQUENCY,
                   "Cannot set frequency '%d'", frequency);

      return FALSE;
    }

  return TRUE;
}

gboolean
cpufreq_selector_set_governor (CPUFreqSelector  *selector,
                               const gchar      *governor,
                               GError          **error)
{
  g_return_val_if_fail (CPUFREQ_IS_SELECTOR (selector), FALSE);
  g_return_val_if_fail (governor != NULL, FALSE);

  if (!cpufreq_selector_validate_governor (selector, governor))
    {
      g_set_error (error, CPUFREQ_SELECTOR_ERROR,
                   CPUFREQ_SELECTOR_ERROR_INVALID_GOVERNOR,
                   "Invalid governor '%s'", governor);

      return FALSE;
    }

  if (cpufreq_modify_policy_governor (selector->cpu, (gchar *) governor) != 0)
    {
      g_set_error (error, CPUFREQ_SELECTOR_ERROR,
                   CPUFREQ_SELECTOR_ERROR_INVALID_GOVERNOR,
                   "Invalid governor '%s'", governor);

      return FALSE;
    }

  return FALSE;
}
