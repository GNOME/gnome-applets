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
#include <glib/gi18n.h>

#include "invest-quotes-retriever.h"

#define QUOTES_URL "http://finance.yahoo.com/d/quotes.csv?s=%s&f=snc4l1d1t1c1ohgv"
#define CHUNK_SIZE 512 * 1024

struct _InvestQuotesRetriever
{
  GObject       parent;

  GCancellable *cancellable;

  gchar        *symbol;
  gchar        *filename;

  guint8       *chunk;
  GByteArray   *array;
};

enum
{
  PROP_0,

  PROP_SYMBOL,
  PROP_FILENAME,

  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

enum
{
  COMPLETED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (InvestQuotesRetriever, invest_quotes_retriever, G_TYPE_OBJECT)

static void
write_cb (GObject      *object,
          GAsyncResult *res,
          gpointer      user_data)
{
  GError *error;
  gsize bytes_written;
  gboolean result;
  InvestQuotesRetriever *retriever;

  error = NULL;
  result = g_output_stream_write_all_finish (G_OUTPUT_STREAM (object), res,
                                             &bytes_written, &error);

  if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
      g_error_free (error);
      return;
    }

  retriever = INVEST_QUOTES_RETRIEVER (user_data);

  g_output_stream_close (G_OUTPUT_STREAM (object), NULL, NULL);
  g_signal_emit (retriever, signals[COMPLETED], 0, result);
  g_clear_error (&error);
}

static void
save_cb (GObject      *object,
         GAsyncResult *res,
         gpointer      user_data)
{
  GError *error;
  GFileOutputStream *stream;
  InvestQuotesRetriever *retriever;

  error = NULL;
  stream = g_file_replace_finish (G_FILE (object), res, &error);

  if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
      g_error_free (error);
      return;
    }

  retriever = INVEST_QUOTES_RETRIEVER (user_data);

  if (error != NULL)
    {
      g_signal_emit (retriever, signals[COMPLETED], 0, FALSE);
      g_error_free (error);
      return;
    }

  g_output_stream_write_all_async (G_OUTPUT_STREAM (stream),
                                   (const void *) retriever->array->data,
                                   retriever->array->len,
                                   0, retriever->cancellable,
                                   write_cb, retriever);

  g_object_unref (stream);
}

static void
download_cb (GObject      *object,
             GAsyncResult *res,
             gpointer      user_data)
{
  GError *error;
  gssize done;
  InvestQuotesRetriever *retriever;

  error = NULL;
  done = g_input_stream_read_finish (G_INPUT_STREAM (object), res, &error);

  if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
      g_error_free (error);
      return;
    }

  retriever = INVEST_QUOTES_RETRIEVER (user_data);

  if (error != NULL)
    {
      g_signal_emit (retriever, signals[COMPLETED], 0, FALSE);
      g_error_free (error);
      return;
    }

  if (done > 0)
    {
      g_byte_array_append (retriever->array, retriever->chunk, done);

      g_free (retriever->chunk);
      retriever->chunk = g_malloc0 (CHUNK_SIZE + 1);

      g_input_stream_read_async (G_INPUT_STREAM (object),
                                 retriever->chunk, CHUNK_SIZE,
                                 0, retriever->cancellable,
                                 download_cb, retriever);
    }
  else if (done == 0)
    {
      const gchar *dir;
      gchar *path;
      GFile *file;

      dir = g_get_user_cache_dir ();

      path = g_build_filename (dir, "gnome-applets", "invest-applet", NULL);
      g_mkdir_with_parents (path, 0700);
      g_free (path);

      path = g_build_filename (dir, "gnome-applets", "invest-applet",
                               retriever->filename, NULL);

      file = g_file_new_for_path (path);
      g_free (path);

      g_file_replace_async (file, NULL, FALSE, 0, 0,
                            retriever->cancellable,
                            save_cb, retriever);

      g_object_unref (file);
    }
  else
    {
      g_assert_not_reached ();
    }
}

static void
ready_cb (GObject      *object,
          GAsyncResult *res,
          gpointer      user_data)
{
  GError *error;
  GFileInputStream *stream;
  InvestQuotesRetriever *retriever;

  error = NULL;
  stream = g_file_read_finish (G_FILE (object), res, &error);

  if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
      g_error_free (error);
      return;
    }

  retriever = INVEST_QUOTES_RETRIEVER (user_data);

  if (error != NULL)
    {
      g_signal_emit (retriever, signals[COMPLETED], 0, FALSE);
      g_error_free (error);
      return;
    }

  g_input_stream_read_async (G_INPUT_STREAM (stream),
                             retriever->chunk, CHUNK_SIZE,
                             0, retriever->cancellable,
                             download_cb, retriever);

  g_object_unref (stream);
}

static void
invest_quotes_retriever_dispose (GObject *object)
{
  InvestQuotesRetriever *retriever;

  retriever = INVEST_QUOTES_RETRIEVER (object);

  if (retriever->cancellable != NULL)
    {
      g_cancellable_cancel (retriever->cancellable);
      g_clear_object (&retriever->cancellable);
    }

  G_OBJECT_CLASS (invest_quotes_retriever_parent_class)->dispose (object);
}

static void
invest_quotes_retriever_finalize (GObject *object)
{
  InvestQuotesRetriever *retriever;

  retriever = INVEST_QUOTES_RETRIEVER (object);

  g_free (retriever->symbol);
  g_free (retriever->filename);

  g_free (retriever->chunk);
  g_byte_array_free (retriever->array, TRUE);

  G_OBJECT_CLASS (invest_quotes_retriever_parent_class)->finalize (object);
}

static void
invest_quotes_retriever_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  InvestQuotesRetriever *retriever;

  retriever = INVEST_QUOTES_RETRIEVER (object);

  switch (property_id)
    {
      case PROP_SYMBOL:
        retriever->symbol = g_value_dup_string (value);
        break;

      case PROP_FILENAME:
        retriever->filename = g_value_dup_string (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
invest_quotes_retriever_install_properties (GObjectClass *object_class)
{
  properties[PROP_SYMBOL] =
    g_param_spec_string ("symbol", "symbol", "symbol", NULL,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
                        G_PARAM_STATIC_STRINGS);

  properties[PROP_FILENAME] =
    g_param_spec_string ("filename", "filename", "filename", NULL,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
invest_quotes_retriever_install_signals (GObjectClass *object_class)
{
  signals[COMPLETED] =
    g_signal_new ("completed", INVEST_TYPE_QUOTES_RETRIEVER, G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

static void
invest_quotes_retriever_class_init (InvestQuotesRetrieverClass *retriever_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (retriever_class);

  object_class->dispose = invest_quotes_retriever_dispose;
  object_class->finalize = invest_quotes_retriever_finalize;
  object_class->set_property = invest_quotes_retriever_set_property;

  invest_quotes_retriever_install_properties (object_class);
  invest_quotes_retriever_install_signals (object_class);
}

static void
invest_quotes_retriever_init (InvestQuotesRetriever *retriever)
{
  retriever->cancellable = g_cancellable_new ();

  retriever->chunk = g_malloc0 (CHUNK_SIZE + 1);
  retriever->array = g_byte_array_new ();
}

InvestQuotesRetriever *
invest_quotes_retriever_new (const gchar *symbol,
                             const gchar *filename)
{
  return g_object_new (INVEST_TYPE_QUOTES_RETRIEVER,
                       "symbol", symbol,
                       "filename", filename,
                       NULL);
}

void
invest_quotes_retriever_start (InvestQuotesRetriever *retriever)
{
  gchar *uri;
  GFile *file;

  uri = g_strdup_printf (QUOTES_URL, retriever->symbol);
  file = g_file_new_for_uri (uri);
  g_free (uri);

  g_file_read_async (file, 0, retriever->cancellable, ready_cb, retriever);
  g_object_unref (file);
}

const gchar *
invest_quotes_retriever_get_symbol (InvestQuotesRetriever *retriever)
{
  return retriever->symbol;
}
