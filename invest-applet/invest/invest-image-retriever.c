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

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "invest-image-retriever.h"

#define CHUNK_SIZE 512 * 1024

struct _InvestImageRetriever
{
  GObject       parent;

  GCancellable *cancellable;

  gchar        *url;

  guint8       *chunk;
  GByteArray   *array;
};

enum
{
  PROP_0,

  PROP_URL,

  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

enum
{
  COMPLETED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (InvestImageRetriever, invest_image_retriever, G_TYPE_OBJECT)

static void
download_cb (GObject      *object,
             GAsyncResult *res,
             gpointer      user_data)
{
  GError *error;
  GdkPixbuf *pixbuf;
  InvestImageRetriever *retriever;

  error = NULL;
  pixbuf = gdk_pixbuf_new_from_stream_finish (res, &error);

  if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
      g_error_free (error);
      return;
    }

  retriever = INVEST_IMAGE_RETRIEVER (user_data);

  if (error != NULL)
    {
      g_signal_emit (retriever, signals[COMPLETED], 0, NULL);
      g_error_free (error);
      return;
    }

  g_signal_emit (retriever, signals[COMPLETED], 0, pixbuf);
  g_object_unref (pixbuf);
}

static void
ready_cb (GObject      *object,
          GAsyncResult *res,
          gpointer      user_data)
{
  GError *error;
  GFileInputStream *stream;
  InvestImageRetriever *retriever;

  error = NULL;
  stream = g_file_read_finish (G_FILE (object), res, &error);

  if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
      g_error_free (error);
      return;
    }

  retriever = INVEST_IMAGE_RETRIEVER (user_data);

  if (error != NULL)
    {
      g_signal_emit (retriever, signals[COMPLETED], 0, NULL);
      g_error_free (error);
      return;
    }

  gdk_pixbuf_new_from_stream_async (G_INPUT_STREAM (stream),
                                    retriever->cancellable,
                                    download_cb, retriever);

  g_object_unref (stream);
}

static void
invest_image_retriever_dispose (GObject *object)
{
  InvestImageRetriever *retriever;

  retriever = INVEST_IMAGE_RETRIEVER (object);

  if (retriever->cancellable != NULL)
    {
      g_cancellable_cancel (retriever->cancellable);
      g_clear_object (&retriever->cancellable);
    }

  G_OBJECT_CLASS (invest_image_retriever_parent_class)->dispose (object);
}

static void
invest_image_retriever_finalize (GObject *object)
{
  InvestImageRetriever *retriever;

  retriever = INVEST_IMAGE_RETRIEVER (object);

  g_free (retriever->url);

  g_free (retriever->chunk);
  g_byte_array_free (retriever->array, TRUE);

  G_OBJECT_CLASS (invest_image_retriever_parent_class)->finalize (object);
}

static void
invest_image_retriever_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  InvestImageRetriever *retriever;

  retriever = INVEST_IMAGE_RETRIEVER (object);

  switch (property_id)
    {
      case PROP_URL:
        retriever->url = g_value_dup_string (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
invest_image_retriever_install_properties (GObjectClass *object_class)
{
  properties[PROP_URL] =
    g_param_spec_string ("url", "url", "url", NULL,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
invest_image_retriever_install_signals (GObjectClass *object_class)
{
  signals[COMPLETED] =
    g_signal_new ("completed", INVEST_TYPE_IMAGE_RETRIEVER, G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL, G_TYPE_NONE, 1, GDK_TYPE_PIXBUF);
}

static void
invest_image_retriever_class_init (InvestImageRetrieverClass *retriever_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (retriever_class);

  object_class->dispose = invest_image_retriever_dispose;
  object_class->finalize = invest_image_retriever_finalize;
  object_class->set_property = invest_image_retriever_set_property;

  invest_image_retriever_install_properties (object_class);
  invest_image_retriever_install_signals (object_class);
}

static void
invest_image_retriever_init (InvestImageRetriever *retriever)
{
  retriever->cancellable = g_cancellable_new ();

  retriever->chunk = g_malloc0 (CHUNK_SIZE + 1);
  retriever->array = g_byte_array_new ();
}

InvestImageRetriever *
invest_image_retriever_new (const gchar *url)
{
  return g_object_new (INVEST_TYPE_IMAGE_RETRIEVER, "url", url, NULL);
}

void
invest_image_retriever_start (InvestImageRetriever *retriever)
{
  GFile *file;

  file = g_file_new_for_uri (retriever->url);
  g_file_read_async (file, 0, retriever->cancellable, ready_cb, retriever);
  g_object_unref (file);
}

const gchar *
invest_image_retriever_get_url (InvestImageRetriever *retriever)
{
  return retriever->url;
}
