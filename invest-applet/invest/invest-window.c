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
#include "invest-widget.h"
#include "invest-window.h"

struct _InvestWindow
{
  GtkWindow     parent;

  GSettings    *settings;
  InvestQuotes *quotes;
};

enum
{
  PROP_0,

  PROP_SETTINGS,
  PROP_QUOTES,

  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

G_DEFINE_TYPE (InvestWindow, invest_window, GTK_TYPE_WINDOW)

static void
invest_window_constructed (GObject *object)
{
  InvestWindow *window;
  GtkWidget *scrolled;
  GtkWidget *invest;

  G_OBJECT_CLASS (invest_window_parent_class)->constructed (object);

  window = INVEST_WINDOW (object);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window), scrolled);
  gtk_widget_show (scrolled);

  invest = invest_widget_new (window->settings, window->quotes);
  gtk_container_add (GTK_CONTAINER (scrolled), invest);
  gtk_widget_show (invest);

  gtk_widget_set_size_request (GTK_WIDGET (window), 520, 220);
  gtk_container_set_border_width (GTK_CONTAINER (window), 4);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
}

static void
invest_window_dispose (GObject *object)
{
  InvestWindow *window;

  window = INVEST_WINDOW (object);

  g_clear_object (&window->settings);
  g_clear_object (&window->quotes);

  G_OBJECT_CLASS (invest_window_parent_class)->dispose (object);
}

static void
invest_window_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  InvestWindow *window;

  window = INVEST_WINDOW (object);

  switch (property_id)
    {
      case PROP_SETTINGS:
        window->settings = g_value_dup_object (value);
        break;

      case PROP_QUOTES:
        window->quotes = g_value_dup_object (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
invest_window_install_properties (GObjectClass *object_class)
{
  properties[PROP_SETTINGS] =
    g_param_spec_object ("settings", "settings", "settings", G_TYPE_SETTINGS,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
                        G_PARAM_STATIC_STRINGS);

  properties[PROP_QUOTES] =
    g_param_spec_object ("quotes", "quotes", "quotes", INVEST_TYPE_QUOTES,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
invest_window_class_init (InvestWindowClass *window_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (window_class);

  object_class->constructed = invest_window_constructed;
  object_class->dispose = invest_window_dispose;
  object_class->set_property = invest_window_set_property;

  invest_window_install_properties (object_class);
}

static void
invest_window_init (InvestWindow *window)
{
  GtkWindow *gtk_window;

  gtk_window = GTK_WINDOW (window);

  gtk_window_set_decorated (gtk_window, FALSE);
  gtk_window_set_type_hint (gtk_window, GDK_WINDOW_TYPE_HINT_DOCK);
  gtk_window_stick (gtk_window);
}

GtkWidget *
invest_window_new (GSettings    *settings,
                   InvestQuotes *quotes)
{
  return g_object_new (INVEST_TYPE_WINDOW,
                       "type", GTK_WINDOW_TOPLEVEL,
                       "settings", settings,
                       "quotes", quotes,
                       NULL);
}
