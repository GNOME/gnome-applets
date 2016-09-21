/*
 * Copyright (C) 2004-2005 Raphael Slinckx
 * Copyright (C) 2009-2011 Enrico Minack
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
 *     Enrico Minack <enrico-minack@gmx.de>
 *     Raphael Slinckx <raphael@slinckx.net>
 */

#include "config.h"

#include <gio/gio.h>
#include <string.h>

#include "invest-cache.h"
#include "invest-currencies.h"

typedef enum
{
  QUOTE_STATE_UNQUOTED,
  QUOTE_STATE_QUOTED,
  QUOTE_STATE_QUOTE,
} QuoteState;

static gchar **
get_fields (const gchar *line)
{
  GPtrArray *fields;
  GString *field;
  QuoteState state;
  guint i;

  fields = g_ptr_array_new ();
  field = g_string_new (NULL);
  state = QUOTE_STATE_UNQUOTED;

  for (i = 0; line[i] != '\0'; i++)
    {
      gchar c;

      c = line[i];

      if (c == ',' && state != QUOTE_STATE_QUOTED)
        {
          g_string_append_c (field, '\0');
          g_ptr_array_add (fields, g_string_free (field, FALSE));
          field = g_string_new (NULL);
          state = QUOTE_STATE_UNQUOTED;
          continue;
        }

      if (c == '"')
        {
          if (state == QUOTE_STATE_UNQUOTED)
            {
              state = QUOTE_STATE_QUOTED;
              continue;
            }

          if (state == QUOTE_STATE_QUOTED)
            {
              state = QUOTE_STATE_QUOTE;
              continue;
            }

          if (state == QUOTE_STATE_QUOTE)
            state = QUOTE_STATE_QUOTED;
        }

      if (state == QUOTE_STATE_QUOTE)
        state = QUOTE_STATE_UNQUOTED;

      field = g_string_append_c (field, c);
    }

  g_string_append_c (field, '\0');
  g_ptr_array_add (fields, g_string_free (field, FALSE));
  g_ptr_array_add (fields, NULL);

  return (gchar **) g_ptr_array_free (fields, FALSE);
}

static InvestQuote *
invest_quote_new (const gchar *str)
{
  gchar **fields;
  InvestQuote *quote;
  gdouble tmp;

  if (str == NULL || *str == '\0')
    return NULL;

  fields = get_fields (str);
  if (g_strv_length (fields) != 11)
    {
      g_strfreev (fields);
      return NULL;
    }

  quote = g_new0 (InvestQuote, 1);

  quote->symbol = g_strdup (fields[0]);
  quote->name = g_strdup (fields[1]);
  quote->currency = g_utf8_strup (fields[2], -1);
  quote->last_trade = g_ascii_strtod (fields[3], NULL);
  quote->last_trade_date = g_strdup (fields[4]);
  quote->last_trade_time = g_strdup (fields[5]);
  quote->change = g_ascii_strtod (fields[6], NULL);
  quote->open = g_ascii_strtod (fields[7], NULL);
  quote->hight = g_ascii_strtod (fields[8], NULL);
  quote->low = g_ascii_strtod (fields[9], NULL);
  quote->volume = (gint) g_ascii_strtoll (fields[10], NULL, 10);

  /* FIXME: use p2 instead of c1 to get change in percent */
  tmp = quote->last_trade - quote->change_pct;
  quote->change_pct = tmp != 0.0 ? (quote->change / tmp * 100) : 0.0;

  /* The currency of currency conversion rates like EURUSD=X is wrong in
   * csv this can be fixed easily by reusing the latter currency in the
   * symbol.
   */
  if (strlen (quote->symbol) == 8 && g_str_has_suffix (quote->symbol, "=X"))
    {
      g_free (quote->currency);
      quote->currency = g_strndup (quote->symbol + 3, 3);
    }

  /* Indices should not have a currency, though yahoo says so.
   */
  if (g_str_has_prefix (quote->symbol, "^"))
    {
      g_free (quote->currency);
      quote->currency = g_strdup ("");
    }

  /* Sometimes, funny currencies are returned (special characters), only
   * consider known currencies.
   */
  if (!invest_currencies_contains (quote->currency))
    {
      g_free (quote->currency);
      quote->currency = g_strdup ("");
    }

  g_strfreev (fields);
  return quote;
}

static GPtrArray *
parse_csv (const gchar *csv)
{
  GPtrArray *array;
  gchar **lines;
  guint i;

  array = g_ptr_array_new ();
  lines = g_strsplit (csv, "\n", -1);

  for (i = 0; lines[i] != NULL; i++)
    {
      InvestQuote *quote;

      quote = invest_quote_new (lines[i]);

      if (quote != NULL)
        g_ptr_array_add (array, quote);
    }

  g_strfreev (lines);

  return array;
}

static void
get_mtime (GFile    *file,
           GTimeVal *mtime)
{
  GFileInfo *info;

  info = g_file_query_info (file, "time::modified", 0, NULL, NULL);
  if (info == NULL)
    return;

  g_file_info_get_modification_time (info, mtime);
  g_object_unref (info);
}

static void
invest_quote_free (InvestQuote *quote)
{
  g_free (quote->symbol);
  g_free (quote->name);
  g_free (quote->currency);
  g_free (quote->last_trade_date);
  g_free (quote->last_trade_time);

  g_free (quote);
}

InvestCache *
invest_cache_read (const gchar *filename)
{
  const gchar *dir;
  gchar *path;
  GFile *file;
  gchar *csv;
  GError *error;
  GPtrArray *array;
  InvestCache *cache;

  dir = g_get_user_cache_dir ();
  path = g_build_filename (dir, "gnome-applets", "invest-applet",
                           filename, NULL);

  file = g_file_new_for_path (path);
  g_free (path);

  csv = NULL;
  error = NULL;

  if (!g_file_load_contents (file, NULL, &csv, NULL, NULL, &error))
    {
      g_warning ("Could not load cached file %s: %s",
                 filename, error->message);

      g_free (csv);
      g_error_free (error);
      g_object_unref (file);

      return NULL;
    }

  array = parse_csv (csv);
  g_free (csv);

  cache = g_new0 (InvestCache, 1);

  cache->n_quotes = array->len;
  cache->quotes = (InvestQuote **) g_ptr_array_free (array, FALSE);

  get_mtime (file, &cache->mtime);
  g_object_unref (file);

  return cache;
}

void
invest_cache_free (InvestCache *cache)
{
  guint i;

  if (cache == NULL)
    return;

  for (i = 0; i < cache->n_quotes; i++)
    invest_quote_free (cache->quotes[i]);

  g_free (cache);
}

InvestQuote *
invest_cache_get (InvestCache *cache,
                  const gchar *symbol)

{
  guint i;

  for (i = 0; i < cache->n_quotes; i++)
    {
      if (g_strcmp0 (cache->quotes[i]->symbol, symbol) == 0)
        return cache->quotes[i];
    }

  return NULL;
}
