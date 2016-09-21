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

#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "invest-cache.h"
#include "invest-image-retriever.h"
#include "invest-quotes.h"
#include "invest-quotes-retriever.h"

#define AUTOREFRESH_TIMEOUT 15 * 60 * 1000

struct _InvestQuotes
{
  GtkTreeStore   parent;

  GSettings     *settings;
  GHashTable    *retrievers;

  GVariant      *stocks;
  gchar        **symbols;

  gboolean       simple_quotes_only;

  guint          update_id;

  gint           count;
  gdouble        change;
  gdouble        avg_change;
  gchar         *updated;

  GPtrArray     *currencies;
  GHashTable    *statistics;

  gboolean       valid;
};

enum
{
  COL_SYMBOL,
  COL_LABEL,
  COL_CURRENCY,
  COL_TICKER_ONLY,
  COL_BALANCE,
  COL_BALANCE_PCT,
  COL_VALUE,
  COL_VARIATION_PCT,
  COL_PIXBUF,
  COL_VARIANT,

  NUM_COLS
};

enum
{
  PROP_0,

  PROP_SETTINGS,

  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

enum
{
  UPDATE_ICON,
  UPDATE_TOOLTIP,

  RELOADED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (InvestQuotes, invest_quotes, GTK_TYPE_TREE_STORE)

typedef struct
{
  InvestQuotes *quotes;
  GtkTreeIter   row;
} ImageData;

static void
image_completed_cb (InvestImageRetriever *retriever,
                    GdkPixbuf            *pixbuf,
                    ImageData            *data)
{
  GtkTreeStore *store;
  const gchar *url;

  store = GTK_TREE_STORE (data->quotes);
  url = invest_image_retriever_get_url (retriever);

  gtk_tree_store_set (store, &data->row, COL_PIXBUF, pixbuf, -1);

  g_hash_table_remove (data->quotes->retrievers, url);
}

static void
retrieve_image (InvestQuotes *quotes,
                const gchar  *symbol,
                GtkTreeIter  *row)
{
  gchar *url;
  InvestImageRetriever *retriever;
  ImageData *data;

  if (g_settings_get_boolean (quotes->settings, "hidecharts"))
    return;

  url = g_strdup_printf ("http://ichart.yahoo.com/h?s=%s", symbol);
  retriever = invest_image_retriever_new (url);

  data = g_new0 (ImageData, 1);

  data->quotes = quotes;
  data->row = *row;

  g_signal_connect_data (retriever, "completed",
                         G_CALLBACK (image_completed_cb),
                         data, (GClosureNotify) g_free,
                         G_CONNECT_AFTER);

  g_hash_table_replace (quotes->retrievers, url, retriever);
  invest_image_retriever_start (retriever);
}

typedef struct
{
  gdouble balance;
  gdouble paid;
} StatisticsData;

static void
update_tooltip (InvestQuotes *quotes)
{
  GPtrArray *array;
  gchar *tmp;
  gchar **tooltip;
  gchar *text;
  GHashTableIter iter;
  gpointer key, value;

  array = g_ptr_array_new ();

  if (quotes->count > 0)
    {
      /* Translators: This is share-market jargon. It is the average
       * percentage change of all stock prices. The %s gets replaced
       * with the string value of the change (localized), including
       * the percent sign.
       */
      tmp = g_strdup_printf (_("Average change: %+.2f%%"), quotes->avg_change);
      g_ptr_array_add (array, tmp);
    }

  g_hash_table_iter_init (&iter, quotes->statistics);
  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      const gchar *currency;
      StatisticsData *data;
      gdouble change;

      currency = (const gchar *) key;
      data = (StatisticsData *) value;

      change = data->balance / data->paid * 100;

      /* Translators: This is share-market jargon. It refers to the total
       * difference between the current price and purchase price for all
       * the shares put together for a particular currency. i.e. How much
       * money would be earned if they were sold right now. The first string
       * is the change value, the second the currency, and the third value
       * is the percentage of the change, formatted using user's locale.
       */
      tmp = g_strdup_printf (_("Positions balance: %'+.2f %s (%'+.2f%%)"),
                             data->balance, currency, change);
      g_ptr_array_add (array, tmp);
    }

  if (quotes->updated != NULL)
    {
      tmp = g_strdup_printf (_("Updated at %s"), quotes->updated);
      g_ptr_array_add (array, tmp);
    }

  g_ptr_array_add (array, NULL);
  tooltip = (gchar **) g_ptr_array_free (array, FALSE);

  text = g_strjoinv ("\n", tooltip);
  g_strfreev (tooltip);

  g_signal_emit (quotes, signals[UPDATE_TOOLTIP], 0, text);
  g_free (text);
}

static void
add_balance_change (InvestQuotes *quotes,
                    gdouble       balance,
                    gdouble       change,
                    const gchar  *currency)
{
  StatisticsData *data;

  if (balance == 0.0 && change == 0.0)
    return;

  if (currency == NULL)
    currency = "";

  data = (StatisticsData *) g_hash_table_lookup (quotes->statistics, currency);

  if (data != NULL)
    {
      data->balance += balance;
      data->paid += balance / change * 100;
    }
  else
    {
      data = g_new0 (StatisticsData, 1);

      data->balance = balance;
      data->paid = balance / change * 100;

      g_hash_table_insert (quotes->statistics, g_strdup (currency), data);
    }
}

static InvestQuote *
get_quote (InvestQuotes *quotes,
           InvestCache  *cache,
           const gchar  *symbol)
{
  InvestQuote *quote;

  quote = invest_cache_get (cache, symbol);
  if (quote == NULL)
    return NULL;

  if (quote->currency != NULL)
    {
      gboolean found;
      guint i;

      found = FALSE;
      for (i = 0; i < quotes->currencies->len; i++)
        {
          const gchar *currency;

          currency = (const gchar *) quotes->currencies->pdata[i];

          if (g_strcmp0 (currency, quote->currency) == 0)
            {
              found = TRUE;
              break;
            }
        }

      if (found == FALSE)
        g_ptr_array_add (quotes->currencies, g_strdup (quote->currency));
    }

  return quote;
}

static void
add_quotes (InvestQuotes *quotes,
            InvestCache  *cache,
            GVariant     *stocks,
            GtkTreeIter  *parent)
{
  GtkTreeStore *store;
  GVariantIter iter;
  gboolean group;
  GVariant *variant;

  store = GTK_TREE_STORE (quotes);

  g_variant_iter_init (&iter, stocks);
  while (g_variant_iter_loop (&iter, "{bv}", &group, &variant))
    {
      if (group == TRUE)
        {
          const gchar *name;
          GVariant *list;
          GtkTreeIter row;

          g_variant_get (variant, "(&sv)", &name, &list);

          gtk_tree_store_append (store, &row, parent);
          gtk_tree_store_set (store, &row,
                              COL_LABEL, name,
                              COL_TICKER_ONLY, TRUE,
                              -1);

          add_quotes (quotes, cache, list, &row);
          g_variant_unref (list);
        }
      else
        {
          const gchar *symbol;
          const gchar *label;
          gdouble amount;
          gdouble price;
          gdouble comission;
          gdouble currency_rate;
          InvestQuote *quote;
          GtkTreeIter row;

          g_variant_get (variant, "(&s&sdddd)", &symbol, &label,
                         &amount, &price, &comission, &currency_rate);

          quote = get_quote (quotes, cache, symbol);
          if (quote == NULL)
            continue;

          if (strlen (label) == 0)
            {
              if (strlen (quote->name) != 0)
                label = quote->name;
              else
                label = symbol;
            }

          gtk_tree_store_append (store, &row, parent);

          if (amount == 0.0)
            {
              gtk_tree_store_set (store, &row,
                                  COL_SYMBOL, symbol,
                                  COL_LABEL, label,
                                  COL_CURRENCY, quote->currency,
                                  COL_TICKER_ONLY, TRUE,
                                  COL_BALANCE, 0.0,
                                  COL_BALANCE_PCT, 0.0,
                                  COL_VALUE, quote->last_trade,
                                  COL_VARIATION_PCT, quote->change_pct,
                                  COL_PIXBUF, NULL,
                                  COL_VARIANT, variant,
                                  -1);
            }
          else
            {
              gdouble current;
              gdouble paid;
              gdouble balance;
              gdouble change;

              current = amount * quote->last_trade;
              paid = amount * price + comission;
              balance = current - paid;

              if (paid != 0.0)
                {
                  change = 100 * balance / paid;
                }
              else
                {
                  /* Not technically correct, but it will look more
                   * intuitive than the real result of infinity.
                   */
                  change = 100;
                }

              gtk_tree_store_set (store, &row,
                                  COL_SYMBOL, symbol,
                                  COL_LABEL, label,
                                  COL_CURRENCY, quote->currency,
                                  COL_TICKER_ONLY, FALSE,
                                  COL_BALANCE, balance,
                                  COL_BALANCE_PCT, change,
                                  COL_VALUE, quote->last_trade,
                                  COL_VARIATION_PCT, quote->change_pct,
                                  COL_PIXBUF, NULL,
                                  COL_VARIANT, variant,
                                  -1);

              add_balance_change (quotes, balance,change, quote->currency);
            }

          quotes->change += quote->change_pct;
          quotes->count++;

          retrieve_image (quotes, symbol, &row);
        }
    }
}

static void
populate (InvestQuotes *quotes,
          InvestCache  *cache)
{
  add_quotes (quotes, cache, quotes->stocks, NULL);

  if (quotes->count > 0)
    {
      gint icon;

      quotes->avg_change = quotes->change / quotes->count;

      icon = 0;
      if (quotes->avg_change != 0.0)
        icon = quotes->avg_change / ABS (quotes->avg_change);

      g_signal_emit (quotes, signals[UPDATE_ICON], 0, icon);
    }
  else
    {
      quotes->avg_change = 0.0;
    }

  quotes->valid = TRUE;
}

static void
add_indices (GPtrArray   *symbols,
             GVariant    *stocks,
             GtkTreeIter *parent)
{
  GVariantIter iter;
  gboolean group;
  GVariant *variant;

  g_variant_iter_init (&iter, stocks);
  while (g_variant_iter_loop (&iter, "{bv}", &group, &variant))
    {
      if (group == TRUE)
        {
          const gchar *name;
          GVariant *list;
          GtkTreeIter row;

          g_variant_get (variant, "(&sv)", &name, &list);

          add_indices (symbols, list, &row);
          g_variant_unref (list);
        }
      else
        {
          const gchar *symbol;
          const gchar *label;
          gdouble amount;
          gdouble price;
          gdouble comission;
          gdouble currency_rate;
          gboolean found;
          guint i;

          g_variant_get (variant, "(&s&sdddd)", &symbol, &label,
                         &amount, &price, &comission, &currency_rate);

          if (!g_str_has_prefix (symbol, "^"))
            continue;

          found = FALSE;
          for (i = 0; i < symbols->len; i++)
            {
              const gchar *tmp;

              tmp = (const gchar *) symbols->pdata[i];

              if (g_strcmp0 (symbol, tmp) == 0)
                {
                  found = TRUE;
                  break;
                }
            }

          if (!found)
            g_ptr_array_add (symbols, g_strdup (symbol));
        }
    }
}

static gchar **
get_indices (InvestQuotes *quotes)
{
  GPtrArray *indices;

  indices = g_ptr_array_new ();

  add_indices (indices, quotes->stocks, NULL);
  g_ptr_array_add (indices, NULL);

  return (gchar **) g_ptr_array_free (indices, FALSE);
}

static void
add_symbols (InvestQuotes *quotes,
             GPtrArray    *symbols,
             GVariant     *stocks,
             GtkTreeIter  *parent)
{
  GVariantIter iter;
  gboolean group;
  GVariant *variant;

  g_variant_iter_init (&iter, stocks);
  while (g_variant_iter_loop (&iter, "{bv}", &group, &variant))
    {
      if (group == TRUE)
        {
          const gchar *name;
          GVariant *list;
          GtkTreeIter row;

          g_variant_get (variant, "(&sv)", &name, &list);

          add_symbols (quotes, symbols, list, &row);
          g_variant_unref (list);
        }
      else
        {
          const gchar *symbol;
          const gchar *label;
          gdouble amount;
          gdouble price;
          gdouble comission;
          gdouble currency_rate;
          gboolean found;
          guint i;

          g_variant_get (variant, "(&s&sdddd)", &symbol, &label,
                         &amount, &price, &comission, &currency_rate);

          found = FALSE;
          for (i = 0; i < symbols->len; i++)
            {
              const gchar *tmp;

              tmp = (const gchar *) symbols->pdata[i];

              if (g_strcmp0 (symbol, tmp) == 0)
                {
                  found = TRUE;
                  break;
                }
            }

          if (amount != 0.0)
            quotes->simple_quotes_only = FALSE;

          if (!found)
            g_ptr_array_add (symbols, g_strdup (symbol));
        }
    }
}

static void
load_symbols (InvestQuotes *quotes)
{
  GPtrArray *symbols;

  symbols = g_ptr_array_new ();

  quotes->simple_quotes_only = TRUE;

  add_symbols (quotes, symbols, quotes->stocks, NULL);
  g_ptr_array_add (symbols, NULL);

  g_strfreev (quotes->symbols);
  quotes->symbols = (gchar **) g_ptr_array_free (symbols, FALSE);
}

static void
load_quotes (InvestQuotes *quotes)
{
  GtkTreeStore *store;
  InvestCache *cache;
  GDateTime *time;

  store = GTK_TREE_STORE (quotes);

  gtk_tree_store_clear (store);

  quotes->count = 0;
  quotes->change = 0.0;
  quotes->avg_change = 0.0;
  quotes->valid = FALSE;

  if (quotes->currencies != NULL)
    g_ptr_array_free (quotes->currencies, TRUE);
  quotes->currencies = g_ptr_array_new ();

  g_hash_table_remove_all (quotes->statistics);

  cache = invest_cache_read ("quotes.csv");
  if (cache == NULL)
    return;

  populate (quotes, cache);

  time = g_date_time_new_from_timeval_local (&cache->mtime);
  if (time != NULL)
    {
      g_free (quotes->updated);
      quotes->updated = g_date_time_format (time, "%H:%M");
      g_date_time_unref (time);
    }
  else
    {
      g_free (quotes->updated);
      quotes->updated = NULL;
    }

  update_tooltip (quotes);
}

typedef struct
{
  gchar     *symbol;
  GPtrArray *paths;
} ForeachData;

static gboolean
find_stock_cb (GtkTreeModel *model,
               GtkTreePath  *path,
               GtkTreeIter  *iter,
               gpointer      user_data)
{
  ForeachData *data;
  gchar *symbol;

  data = (ForeachData *) user_data;

  gtk_tree_model_get (model, iter, COL_SYMBOL, &symbol, -1);

  if (g_strcmp0 (symbol, data->symbol) == 0)
    g_ptr_array_add (data->paths, gtk_tree_path_to_string (path));

  g_free (symbol);

  return FALSE;
}

static gboolean
expand_index (InvestQuotes *quotes,
              const gchar  *index)
{
  gchar *filename;
  InvestCache *cache;
  GtkTreeModel *model;
  ForeachData *data;
  gchar **paths;
  gboolean valid;
  guint i;

  filename = g_strdup_printf ("quotes.%s.csv", index);
  cache = invest_cache_read (filename);
  g_free (filename);

  if (cache == NULL)
    return FALSE;

  model = GTK_TREE_MODEL (quotes);
  data = g_new0 (ForeachData, 1);

  data->symbol = g_strdup (index);
  data->paths = g_ptr_array_new ();

  gtk_tree_model_foreach (model, find_stock_cb, data);
  g_ptr_array_add (data->paths, NULL);

  g_free (data->symbol);
  paths = (gchar **) g_ptr_array_free (data->paths, FALSE);
  g_free (data);

  valid = FALSE;
  for (i = 0; paths[i] != NULL; i++)
    {
      guint j;

      for (j = 0; j < cache->n_quotes; j++)
        {
          const gchar *symbol;
          InvestQuote *quote;

          symbol = cache->quotes[j]->symbol;
          quote = get_quote (quotes, cache, symbol);

          if (quote != NULL)
            {
              GtkTreePath *path;
              GtkTreeIter iter;

              path = gtk_tree_path_new_from_string (paths[i]);
              if (gtk_tree_model_get_iter (model, &iter, path))
                {
                  GtkTreeIter row;

                  gtk_tree_store_insert (GTK_TREE_STORE (quotes), &row, &iter, 0);
                  gtk_tree_store_set (GTK_TREE_STORE (quotes), &row,
                                      COL_SYMBOL, symbol,
                                      COL_LABEL, quote->name,
                                      COL_CURRENCY, quote->currency,
                                      COL_TICKER_ONLY, TRUE,
                                      COL_BALANCE, 0.0,
                                      COL_BALANCE_PCT, 0.0,
                                      COL_VALUE, quote->last_trade,
                                      COL_VARIATION_PCT, quote->change_pct,
                                      COL_PIXBUF, NULL,
                                      COL_VARIANT, NULL,
                                      -1);

                  retrieve_image (quotes, symbol, &row);
                }

              gtk_tree_path_free (path);
            }
        }

      valid = TRUE;
    }

  g_strfreev (paths);
  invest_cache_free (cache);

  return valid;
}

static void
load_all_index_quotes (InvestQuotes *quotes)
{
  const gchar *cache;
  gchar *path;
  GDir *dir;
  GRegex *regex;
  const gchar *filename;

  if (!g_settings_get_boolean (quotes->settings, "indexexpansion"))
    return;

  cache = g_get_user_cache_dir ();
  path = g_build_filename (cache, "gnome-applets", "invest-applet", NULL);
  dir = g_dir_open (path, 0, NULL);

  if (dir == NULL)
    {
      g_free (path);
      return;
    }

  regex = g_regex_new ("quotes.([^.]+).csv", 0, 0, NULL);
  filename = g_dir_read_name (dir);

  while (filename != NULL)
    {
      GMatchInfo *match_info;

      g_regex_match (regex, filename, 0, &match_info);

      while (g_match_info_matches (match_info))
        {
          gchar *index;

          index = g_match_info_fetch (match_info, 1);

          if (expand_index (quotes, index) == 0)
            {
              gchar *delete_filename;

              delete_filename = g_build_filename (path, filename, NULL);

              g_unlink (delete_filename);
              g_free (delete_filename);
            }

          g_free (index);

          g_match_info_next (match_info, NULL);
        }

      g_match_info_free (match_info);
      filename = g_dir_read_name (dir);
    }

  g_regex_unref (regex);
  g_dir_close (dir);
  g_free (path);
}

typedef struct
{
  gdouble rate;
} RateData;

static void
convert_currencies (InvestQuotes *quotes,
                    InvestCache  *cache,
                    const gchar  *target_currency)
{
  GHashTable *rates;
  guint i;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gboolean valid;

  /* reset the overall balance */
  g_hash_table_remove_all (quotes->statistics);

  /* collect the rates for the currencies */
  rates = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  for (i = 0; i < cache->n_quotes; i++)
    {
      InvestQuote *quote;
      RateData *data;

      quote = cache->quotes[i];

      data = g_new0 (RateData, 1);
      data->rate = quote->last_trade;

      g_hash_table_insert (rates, g_strndup (quote->symbol, 3), data);
    }

  /* convert all non target currencies */
  model = GTK_TREE_MODEL (quotes);
  valid = gtk_tree_model_get_iter_first (model, &iter);

  while (valid)
    {
      gchar *currency;
      gchar *symbol;
      gdouble balance;
      gdouble change;

      gtk_tree_model_get (model, &iter, COL_CURRENCY, &currency, -1);

      if (currency == NULL)
        {
          valid = gtk_tree_model_iter_next (model, &iter);
          continue;
        }

      gtk_tree_model_get (model, &iter, COL_SYMBOL, &symbol, -1);

      /* Ignore stocks that are currency conversions and only convert
       * stocks that are not in the target currency and if we have a
       * conversion rate.
       */
      if (!(strlen (symbol) == 8 && g_str_has_suffix (symbol, "=X")) &&
          g_strcmp0 (currency, target_currency) != 0 &&
          g_hash_table_lookup (rates, currency) != NULL)
        {
          RateData *data;
          GtkTreeStore *store;
          gboolean ticker_only;
          gdouble value;

          data = (RateData *) g_hash_table_lookup (rates, currency);
          store = GTK_TREE_STORE (quotes);

          gtk_tree_model_get (model, &iter,
                              COL_TICKER_ONLY, &ticker_only,
                              COL_VALUE, &value,
                              -1);

          /* first convert the balance, it needs the original value */
          if (ticker_only == FALSE)
            {
              GVariant *variant;
              gdouble amount;
              gdouble price;
              gdouble comission;
              gdouble currency_rate;
              gdouble current;
              gdouble paid;

              gtk_tree_model_get (model, &iter, COL_VARIANT, &variant, -1);
              g_variant_get (variant, "(&s&sdddd)", NULL, NULL, &amount,
                             &price, &comission, &currency_rate);

              current = 0.0;
              paid = 0.0;

              if (amount != 0.0)
                {
                  /* if the buy rate is invalid, use 1.0 */
                  if (currency_rate <= 0.0)
                    currency_rate = 1.0;

                  /* current value is the current rate * amount * value */
                  current = data->rate * amount * value;

                  /* paid is buy rate * (amount * price + commission) */
                  paid = currency_rate * (amount * price + comission);
                }

              balance = current - paid;

              if (paid != 0.0)
                {
                  change = 100 * balance / paid;
                }
              else
                {
                  /* Not technically correct, but it will look more
                   * intuitive than the real result of infinity.
                   */
                  change = 100;
                }

              gtk_tree_store_set (store, &iter,
                                  COL_BALANCE, balance,
                                  COL_BALANCE_PCT, change,
                                  -1);

              add_balance_change (quotes, balance, change, target_currency);
            }

          /* now, convert the value */
          gtk_tree_store_set (store, &iter,
                              COL_VALUE, value * data->rate,
                              COL_CURRENCY, target_currency,
                              -1);
        }
      else
        {
          gtk_tree_model_get (model, &iter,
                              COL_BALANCE, &balance,
                              COL_BALANCE_PCT, &change,
                              -1);

          add_balance_change (quotes, balance, change, currency);
        }

      g_free (currency);
      g_free (symbol);

      valid = gtk_tree_model_iter_next (model, &iter);
    }

  g_hash_table_destroy (rates);
}

static void
load_currencies (InvestQuotes *quotes)
{
  gchar *target_currency;
  InvestCache *cache;

  /* If there is no target currency, this method should never have
   * been called.
   */
  target_currency = g_settings_get_string (quotes->settings, "currency");
  if (!target_currency || *target_currency == '\0')
    {
      g_free (target_currency);
      return;
    }

  cache = invest_cache_read ("currencies.csv");
  if (cache == NULL)
    {
      g_free (target_currency);
      return;
    }

  convert_currencies (quotes, cache, target_currency);

  invest_cache_free (cache);
  g_free (target_currency);
}

static void
currencies_completed_cb (InvestQuotesRetriever *retriever,
                         gboolean               retrieved,
                         InvestQuotes          *quotes)
{
  if (!retrieved)
    g_warning ("Failed to retrieve currency rates!");
  else
    load_currencies (quotes);

  update_tooltip (quotes);

  g_hash_table_remove (quotes->retrievers, "currencies");
}

static void
retrieve_currencies (InvestQuotes *quotes)
{
  gchar *target_currency;
  GPtrArray *array;
  guint i;
  gchar **symbols;

  target_currency = g_settings_get_string (quotes->settings, "currency");
  if (!target_currency || *target_currency == '\0')
    {
      g_free (target_currency);
      return;
    }

  array = g_ptr_array_new ();
  for (i = 0; i < quotes->currencies->len; i++)
    {
      const gchar *currency;
      gchar *symbol;

      currency = (const gchar *) quotes->currencies->pdata[i];

      if (g_strcmp0 (currency, target_currency) == 0)
        continue;

      symbol = g_strdup_printf ("%s%s=X", currency, target_currency);
      g_ptr_array_add (array, symbol);
    }

  g_free (target_currency);

  g_ptr_array_add (array, NULL);
  symbols = (gchar **) g_ptr_array_free (array, FALSE);

  if (g_strv_length (symbols) > 0)
    {
      gchar *tmp;
      InvestQuotesRetriever *retriever;

      tmp = g_strjoinv (",", symbols);
      retriever = invest_quotes_retriever_new (tmp, "currencies.csv");
      g_free (tmp);

      g_signal_connect (retriever, "completed",
                        G_CALLBACK (currencies_completed_cb),
                        quotes);

      g_hash_table_replace (quotes->retrievers, g_strdup ("currencies"), retriever);
      invest_quotes_retriever_start (retriever);
    }

  g_strfreev (symbols);
}

static void
index_completed_cb (InvestQuotesRetriever *retriever,
                    gboolean               retrieved,
                    InvestQuotes          *quotes)
{
  const gchar *index;

  index = invest_quotes_retriever_get_symbol (retriever);

  if (!retrieved)
    {
      g_warning ("Failed to retrieve quotes for index %s!", index);
      return;
    }

  expand_index (quotes, index);
  retrieve_currencies (quotes);

  g_hash_table_remove (quotes->retrievers, index);
}

static void
expand_indices (InvestQuotes *quotes)
{
  gchar **indices;
  guint i;

  if (!g_settings_get_boolean (quotes->settings, "indexexpansion"))
    {
      retrieve_currencies (quotes);
      return;
    }

  indices = get_indices (quotes);

  for (i = 0; indices[i] != NULL; i++)
    {
      gchar *filename;
      InvestQuotesRetriever *retriever;

      filename = g_strdup_printf ("quotes.%s.csv", indices[i]);
      retriever = invest_quotes_retriever_new (indices[i], filename);
      g_free (filename);

      g_signal_connect (retriever, "completed",
                        G_CALLBACK (index_completed_cb),
                        quotes);

      g_hash_table_replace (quotes->retrievers, g_strdup (indices[i]), retriever);
      invest_quotes_retriever_start (retriever);
    }

  g_strfreev (indices);
}

static void
quotes_completed_cb (InvestQuotesRetriever *retriever,
                     gboolean               retrieved,
                     InvestQuotes          *quotes)
{
  if (retrieved)
    {
      load_quotes (quotes);
      expand_indices (quotes);
    }
  else
    {
      const gchar *text;

      text = _("Invest could not connect to Yahoo! Finance");

      g_signal_emit (quotes, signals[UPDATE_TOOLTIP], 0, text);
    }

  g_hash_table_remove (quotes->retrievers, "quotes");
}

static void
add_update_timeout (InvestQuotes *quotes)
{
  gint interval;
  GSourceFunc func;

  if (quotes->update_id != 0)
    return;

  interval = AUTOREFRESH_TIMEOUT;
  func = (GSourceFunc) invest_quotes_refresh;

  quotes->update_id = g_timeout_add (interval, func, quotes);
}

static void
invest_quotes_reload (InvestQuotes *quotes)
{
  g_clear_pointer (&quotes->stocks, g_variant_unref);
  quotes->stocks = g_settings_get_value (quotes->settings, "stocks");

  g_hash_table_remove_all (quotes->retrievers);

  load_symbols (quotes);
  load_quotes (quotes);
  load_all_index_quotes (quotes);
  load_currencies (quotes);

  g_signal_emit (quotes, signals[RELOADED], 0);

  add_update_timeout (quotes);
  invest_quotes_refresh (quotes);
}

static void
settings_changed_cb (GSettings    *settings,
                     const gchar  *key,
                     InvestQuotes *quotes)
{
  invest_quotes_reload (quotes);
}

static void
network_changed_cb (GNetworkMonitor *monitor,
                    gboolean         available,
                    InvestQuotes    *quotes)
{
  if (!available)
    return;

  add_update_timeout (quotes);
  invest_quotes_refresh (quotes);
}

static void
invest_quotes_constructed (GObject *object)
{
  InvestQuotes *quotes;

  quotes = INVEST_QUOTES (object);

  G_OBJECT_CLASS (invest_quotes_parent_class)->constructed (object);

  g_signal_connect (quotes->settings, "changed",
                    G_CALLBACK (settings_changed_cb), quotes);

  g_signal_connect (g_network_monitor_get_default (), "network-changed",
                    G_CALLBACK (network_changed_cb), quotes);

  invest_quotes_reload (quotes);
}

static void
invest_quotes_dispose (GObject *object)
{
  InvestQuotes *quotes;

  quotes = INVEST_QUOTES (object);

  if (quotes->update_id != 0)
    {
      g_source_remove (quotes->update_id);
      quotes->update_id = 0;
    }

  g_clear_pointer (&quotes->stocks, g_variant_unref);
  g_clear_pointer (&quotes->retrievers, g_hash_table_destroy);
  g_clear_pointer (&quotes->statistics, g_hash_table_destroy);
  g_clear_object (&quotes->settings);

  G_OBJECT_CLASS (invest_quotes_parent_class)->dispose (object);
}

static void
invest_quotes_finalize (GObject *object)
{
  InvestQuotes *quotes;

  quotes = INVEST_QUOTES (object);

  g_strfreev (quotes->symbols);
  g_free (quotes->updated);

  if (quotes->currencies != NULL)
    g_ptr_array_free (quotes->currencies, TRUE);

  G_OBJECT_CLASS (invest_quotes_parent_class)->finalize (object);
}

static void
invest_quotes_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  InvestQuotes *quotes;

  quotes = INVEST_QUOTES (object);

  switch (property_id)
    {
      case PROP_SETTINGS:
        quotes->settings = g_value_dup_object (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
invest_quotes_install_properties (GObjectClass *object_class)
{
  properties[PROP_SETTINGS] =
    g_param_spec_object ("settings", "settings", "settings", G_TYPE_SETTINGS,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
invest_quotes_install_signals (GObjectClass *object_class)
{
  signals[UPDATE_ICON] =
    g_signal_new ("update-icon", INVEST_TYPE_QUOTES, G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_INT);

  signals[UPDATE_TOOLTIP] =
    g_signal_new ("update-tooltip", INVEST_TYPE_QUOTES, G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);

  signals[RELOADED] =
    g_signal_new ("reloaded", INVEST_TYPE_QUOTES, G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void
invest_quotes_class_init (InvestQuotesClass *quotes_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (quotes_class);

  object_class->constructed = invest_quotes_constructed;
  object_class->dispose = invest_quotes_dispose;
  object_class->finalize = invest_quotes_finalize;
  object_class->set_property = invest_quotes_set_property;

  invest_quotes_install_properties (object_class);
  invest_quotes_install_signals (object_class);
}

static void
invest_quotes_init (InvestQuotes *quotes)
{
  GtkTreeStore *store;
  GType *types;

  store = GTK_TREE_STORE (quotes);
  types = g_new0 (GType, NUM_COLS);

  types[COL_SYMBOL] = G_TYPE_STRING;
  types[COL_LABEL] = G_TYPE_STRING;
  types[COL_CURRENCY] = G_TYPE_STRING;
  types[COL_TICKER_ONLY] = G_TYPE_BOOLEAN;
  types[COL_BALANCE] = G_TYPE_DOUBLE;
  types[COL_BALANCE_PCT] = G_TYPE_DOUBLE;
  types[COL_VALUE] = G_TYPE_DOUBLE;
  types[COL_VARIATION_PCT] = G_TYPE_DOUBLE;
  types[COL_PIXBUF] = GDK_TYPE_PIXBUF;
  types[COL_VARIANT] = G_TYPE_VARIANT;

  gtk_tree_store_set_column_types (store, NUM_COLS, types);
  g_free (types);

  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store), COL_LABEL,
                                        GTK_SORT_ASCENDING);

  quotes->retrievers = g_hash_table_new_full (g_str_hash, g_str_equal,
                                              g_free, g_object_unref);

  quotes->statistics = g_hash_table_new_full (g_str_hash, g_str_equal,
                                              g_free, g_free);
}

InvestQuotes *
invest_quotes_new (GSettings *settings)
{
  return g_object_new (INVEST_TYPE_QUOTES,
                       "settings", settings,
                       NULL);
}

gboolean
invest_quotes_has_stocks (InvestQuotes *quotes)
{
  gboolean valid;
  gint i;

  if (!quotes->symbols)
    return FALSE;

  valid = FALSE;
  for (i = 0; quotes->symbols[i] != NULL; i++)
    {
      const gchar *symbol;

      symbol = quotes->symbols[i];

      if (symbol != NULL && symbol[0] != '\0')
        {
          valid = TRUE;
          break;
        }
    }

  return valid;
}

gboolean
invest_quotes_is_valid (InvestQuotes *quotes)
{
  return quotes->valid;
}

gboolean
invest_quotes_simple_quotes_only (InvestQuotes *quotes)
{
  return quotes->simple_quotes_only;
}

gboolean
invest_quotes_refresh (InvestQuotes *quotes)
{
  GNetworkMonitor *nm;
  gchar *symbols;
  InvestQuotesRetriever *retriever;

  nm = g_network_monitor_get_default ();

  if (!g_network_monitor_get_network_available (nm))
    {
      if (quotes->update_id != 0)
        {
          g_source_remove (quotes->update_id);
          quotes->update_id = 0;
        }

      return FALSE;
    }

  if (!invest_quotes_has_stocks (quotes))
    return TRUE;

  symbols = g_strjoinv (",", quotes->symbols);
  retriever = invest_quotes_retriever_new (symbols, "quotes.csv");
  g_free (symbols);

  g_signal_connect (retriever, "completed",
                    G_CALLBACK (quotes_completed_cb),
                    quotes);

  g_hash_table_replace (quotes->retrievers, g_strdup ("quotes"), retriever);
  invest_quotes_retriever_start (retriever);

  return TRUE;
}
