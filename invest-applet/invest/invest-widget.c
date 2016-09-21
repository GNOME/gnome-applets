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

#include "invest-chart.h"
#include "invest-widget.h"

#define LIGHT -3
#define MEDIUM -1

struct _InvestWidget
{
  GtkTreeView        parent;

  GSettings         *settings;
  InvestQuotes      *quotes;

  GtkTreeViewColumn *chart;
  GtkTreeViewColumn *gain;
  GtkTreeViewColumn *gain_pct;

  gulong             changed_id;
  gulong             reloaded_id;
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
  PROP_QUOTES,

  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

G_DEFINE_TYPE (InvestWidget, invest_widget, GTK_TYPE_TREE_VIEW)

static const gchar *positive[] =
  {
    "#ffffff", "#ad7fa8", "#75507b", "#5c3566", "#729fcf",
    "#3465a4", "#204a87", "#8ae234", "#73d216", "#4e9a06"
  };

static const gchar *negative[] =
  {
    "#ffffff", "#fce94f", "#e9b96e", "#fcaf3e", "#c17d11",
    "#f57900", "#ce5c00", "#ef2929", "#cc0000", "#a40000"
  };

static void
row_activated_cb (GtkTreeView       *tree_view,
                  GtkTreePath       *path,
                  GtkTreeViewColumn *column,
                  InvestWidget      *widget)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  gchar *symbol;

  model = GTK_TREE_MODEL (widget->quotes);

  if (!gtk_tree_model_get_iter (model, &iter, path))
    return;

  gtk_tree_model_get (model, &iter, COL_SYMBOL, &symbol, -1);

  if (symbol == NULL)
    return;

  invest_chart_show_chart (symbol);
  g_free (symbol);
}

static gboolean
is_selected (InvestWidget *widget,
             GtkTreeIter  *iter)
{
  GtkTreeView *tree_view;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter selected;
  GtkTreePath *a;
  GtkTreePath *b;
  gboolean ret;

  tree_view = GTK_TREE_VIEW (widget);
  selection = gtk_tree_view_get_selection (tree_view);

  if (!gtk_tree_selection_get_selected (selection, &model, &selected))
    return FALSE;

  a = gtk_tree_model_get_path (GTK_TREE_MODEL (widget->quotes), iter);
  b = gtk_tree_model_get_path (model, &selected);

  ret = FALSE;
  if (gtk_tree_path_compare (a, b) == 0)
    ret = TRUE;

  gtk_tree_path_free (a);
  gtk_tree_path_free (b);

  return ret;
}

static const gchar *
get_color (InvestWidget *widget,
           GtkTreeIter  *iter,
           gint          field)
{
  gdouble value;
  gint intensity;

  gtk_tree_model_get (GTK_TREE_MODEL (widget->quotes), iter,
                      field, &value, -1);

  intensity = MEDIUM;
  if (is_selected (widget, iter))
    intensity = LIGHT;

  if (value < 0.0)
    return negative[G_N_ELEMENTS (negative) + intensity];

  return positive[G_N_ELEMENTS (positive) + intensity];
}

static void
changed_hidecharts_cb (GSettings    *settings,
                       const gchar  *key,
                       InvestWidget *widget)
{
  gboolean hidecharts;

  hidecharts = g_settings_get_boolean (widget->settings, key);
  gtk_tree_view_column_set_visible (widget->chart, !hidecharts);
}

static void
reloaded_cb (InvestQuotes *quotes,
             InvestWidget *widget)
{
  gboolean simple_quotes_only;

  simple_quotes_only = invest_quotes_simple_quotes_only (quotes);

  gtk_tree_view_column_set_visible (widget->gain, !simple_quotes_only);
  gtk_tree_view_column_set_visible (widget->gain_pct, !simple_quotes_only);
}

static gboolean
is_group (InvestWidget *widget,
          GtkTreeIter  *iter)
{
  gchar *symbol;
  gboolean is_group;

  symbol = NULL;
  gtk_tree_model_get (GTK_TREE_MODEL (widget->quotes), iter,
                      COL_SYMBOL, &symbol, -1);

  is_group = symbol == NULL ? TRUE : FALSE;
  g_free (symbol);

  return is_group;
}

static gboolean
is_stock (InvestWidget *widget,
          GtkTreeIter  *iter)
{
  return !is_group (widget, iter);
}

static void
ticker_cell_data_func (GtkTreeViewColumn *tree_column,
                       GtkCellRenderer   *cell,
                       GtkTreeModel      *tree_model,
                       GtkTreeIter       *iter,
                       gpointer           data)
{
  InvestWidget *widget;
  gchar *label;

  widget = INVEST_WIDGET (data);

  gtk_tree_model_get (GTK_TREE_MODEL (widget->quotes), iter,
                      COL_LABEL, &label, -1);

  if (is_stock (widget, iter))
    {
      g_object_set (cell, "text", label, NULL);
      g_free (label);
    }
  else
    {
      gchar *markup;

      markup = g_strdup_printf ("<b>%s</b>", label);
      g_free (label);

      g_object_set (cell, "markup", markup, NULL);
      g_free (markup);
    }
}

static void
add_column_ticker (InvestWidget *widget,
                   GtkTreeView  *tree_view)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Ticker"), renderer, NULL);

  gtk_tree_view_column_set_cell_data_func (column, renderer,
                                           ticker_cell_data_func,
                                           widget, NULL);

  gtk_tree_view_column_set_sort_column_id (column, COL_LABEL);
  gtk_tree_view_append_column (tree_view, column);
}

static void
last_cell_data_func (GtkTreeViewColumn *tree_column,
                     GtkCellRenderer   *cell,
                     GtkTreeModel      *tree_model,
                     GtkTreeIter       *iter,
                     gpointer           data)
{
  InvestWidget *widget;
  gdouble value;
  gchar *currency;

  widget = INVEST_WIDGET (data);

  gtk_tree_model_get (GTK_TREE_MODEL (widget->quotes), iter,
                      COL_VALUE, &value, COL_CURRENCY, &currency, -1);

  if (currency == NULL)
    {
      g_object_set (cell, "text", "", NULL);
    }
  else
    {
      gchar *text;

      text = g_strdup_printf ("%'.2f %s", value, currency);

      g_object_set (cell, "text", text, NULL);
      g_free (text);
    }

  g_free (currency);
}

static void
add_column_last (InvestWidget *widget,
                 GtkTreeView  *tree_view)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Last"), renderer, NULL);

  gtk_cell_renderer_set_alignment (renderer, 1.0, 0.5);

  gtk_tree_view_column_set_cell_data_func (column, renderer,
                                           last_cell_data_func,
                                           widget, NULL);

  gtk_tree_view_append_column (tree_view, column);
}

static void
change_cell_data_func (GtkTreeViewColumn *tree_column,
                       GtkCellRenderer   *cell,
                       GtkTreeModel      *tree_model,
                       GtkTreeIter       *iter,
                       gpointer           data)
{
  InvestWidget *widget;
  const gchar *color;
  gdouble value;
  gchar *markup;

  widget = INVEST_WIDGET (data);

  if (is_group (widget, iter))
    {
      g_object_set (cell, "text", "", NULL);
      return;
    }

  color = get_color (widget, iter, COL_VARIATION_PCT);
  gtk_tree_model_get (GTK_TREE_MODEL (widget->quotes), iter,
                      COL_VARIATION_PCT, &value, -1);

  markup = g_strdup_printf ("<span foreground='%s'>%'+.2f%%</span>",
                            color, value);

  g_object_set (cell, "markup", markup, NULL);
  g_free (markup);
}

static void
add_column_change (InvestWidget *widget,
                   GtkTreeView  *tree_view)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Change %"), renderer, NULL);

  gtk_cell_renderer_set_alignment (renderer, 1.0, 0.5);

  gtk_tree_view_column_set_cell_data_func (column, renderer,
                                           change_cell_data_func,
                                           widget, NULL);

  gtk_tree_view_column_set_sort_column_id (column, COL_VARIATION_PCT);
  gtk_tree_view_append_column (tree_view, column);
}

static void
add_column_chart (InvestWidget *widget,
                  GtkTreeView  *tree_view)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Chart"), renderer,
                                                     "pixbuf", COL_PIXBUF,
                                                     NULL);

  widget->chart = column;

  gtk_tree_view_append_column (tree_view, column);
}

static void
gain_cell_data_func (GtkTreeViewColumn *tree_column,
                     GtkCellRenderer   *cell,
                     GtkTreeModel      *tree_model,
                     GtkTreeIter       *iter,
                     gpointer           data)
{
  InvestWidget *widget;
  gboolean ticker_only;

  widget = INVEST_WIDGET (data);

  gtk_tree_model_get (GTK_TREE_MODEL (widget->quotes), iter,
                      COL_TICKER_ONLY, &ticker_only, -1);

  if (ticker_only)
    {
      g_object_set (cell, "text", "", NULL);
    }
  else
    {
      const gchar *color;
      gdouble value;
      gchar *currency;
      gchar *markup;

      color = get_color (widget, iter, COL_BALANCE);
      gtk_tree_model_get (GTK_TREE_MODEL (widget->quotes), iter,
                          COL_BALANCE, &value, COL_CURRENCY, &currency,
                          -1);

      markup = g_strdup_printf ("<span foreground='%s'>%'+.2f %s</span>",
                                color, value, currency);

      g_free (currency);

      g_object_set (cell, "markup", markup, NULL);
      g_free (markup);
    }
}

static void
add_column_gain (InvestWidget *widget,
                 GtkTreeView  *tree_view)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Gain"), renderer, NULL);

  gtk_cell_renderer_set_alignment (renderer, 1.0, 0.5);

  gtk_tree_view_column_set_cell_data_func (column, renderer,
                                           gain_cell_data_func,
                                           widget, NULL);

  widget->gain = column;

  gtk_tree_view_column_set_sort_column_id (column, COL_BALANCE);
  gtk_tree_view_append_column (tree_view, column);
}

static void
gain_pct_cell_data_func (GtkTreeViewColumn *tree_column,
                         GtkCellRenderer   *cell,
                         GtkTreeModel      *tree_model,
                         GtkTreeIter       *iter,
                         gpointer           data)
{
  InvestWidget *widget;
  gboolean ticker_only;

  widget = INVEST_WIDGET (data);

  gtk_tree_model_get (GTK_TREE_MODEL (widget->quotes), iter,
                      COL_TICKER_ONLY, &ticker_only, -1);

  if (ticker_only)
    {
      g_object_set (cell, "text", "", NULL);
    }
  else
    {
      const gchar *color;
      gdouble value;
      gchar *markup;

      color = get_color (widget, iter, COL_BALANCE_PCT);
      gtk_tree_model_get (GTK_TREE_MODEL (widget->quotes), iter,
                          COL_BALANCE_PCT, &value, -1);

      markup = g_strdup_printf ("<span foreground='%s'>%'+.2f%%</span>",
                                color, value);

      g_object_set (cell, "markup", markup, NULL);
      g_free (markup);
    }
}

static void
add_column_gain_pct (InvestWidget *widget,
                     GtkTreeView  *tree_view)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Gain %"), renderer, NULL);

  gtk_cell_renderer_set_alignment (renderer, 1.0, 0.5);

  gtk_tree_view_column_set_cell_data_func (column, renderer,
                                           gain_pct_cell_data_func,
                                           widget, NULL);

  widget->gain_pct = column;

  gtk_tree_view_column_set_sort_column_id (column, COL_BALANCE_PCT);
  gtk_tree_view_append_column (tree_view, column);
}

static void
invest_widget_constructed (GObject *object)
{
  InvestWidget *widget;
  GtkTreeView *tree_view;
  GtkTreeModel *model;

  G_OBJECT_CLASS (invest_widget_parent_class)->constructed (object);

  widget = INVEST_WIDGET (object);
  tree_view = GTK_TREE_VIEW (widget);
  model = GTK_TREE_MODEL (widget->quotes);

  add_column_ticker (widget, tree_view);
  add_column_last (widget, tree_view);
  add_column_change (widget, tree_view);
  add_column_chart (widget, tree_view);
  add_column_gain (widget, tree_view);
  add_column_gain_pct (widget, tree_view);

  widget->changed_id =
    g_signal_connect (widget->settings,"changed::hidecharts",
                      G_CALLBACK (changed_hidecharts_cb), widget);

  widget->reloaded_id =
    g_signal_connect (widget->quotes, "reloaded",
                      G_CALLBACK (reloaded_cb), widget);

  changed_hidecharts_cb (widget->settings, "hidecharts", widget);
  reloaded_cb (widget->quotes, widget);

  gtk_tree_view_set_hover_selection (tree_view, TRUE);
  gtk_tree_view_set_model (tree_view, model);
}

static void
invest_widget_dispose (GObject *object)
{
  InvestWidget *widget;

  widget = INVEST_WIDGET (object);

  if (widget->changed_id != 0)
    {
      g_signal_handler_disconnect (widget->settings, widget->changed_id);
      widget->changed_id = 0;
    }

  if (widget->reloaded_id != 0)
    {
      g_signal_handler_disconnect (widget->quotes, widget->reloaded_id);
      widget->reloaded_id = 0;
    }

  g_clear_object (&widget->settings);
  g_clear_object (&widget->quotes);

  G_OBJECT_CLASS (invest_widget_parent_class)->dispose (object);
}

static void
invest_widget_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  InvestWidget *widget;

  widget = INVEST_WIDGET (object);

  switch (property_id)
    {
      case PROP_SETTINGS:
        widget->settings = g_value_dup_object (value);
        break;

      case PROP_QUOTES:
        widget->quotes = g_value_dup_object (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
invest_widget_install_properties (GObjectClass *object_class)
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
invest_widget_class_init (InvestWidgetClass *widget_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (widget_class);

  object_class->constructed = invest_widget_constructed;
  object_class->dispose = invest_widget_dispose;
  object_class->set_property = invest_widget_set_property;

  invest_widget_install_properties (object_class);
}

static void
invest_widget_init (InvestWidget *widget)
{
  g_signal_connect (widget, "row-activated",
                    G_CALLBACK (row_activated_cb), widget);
}

GtkWidget *
invest_widget_new (GSettings    *settings,
                   InvestQuotes *quotes)
{
  return g_object_new (INVEST_TYPE_WIDGET,
                       "settings", settings,
                       "quotes", quotes,
                       NULL);
}
