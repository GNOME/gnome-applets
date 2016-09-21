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
#include <stdlib.h>

#include "invest-currencies.h"
#include "invest-preferences.h"

struct _InvestPreferences
{
  GtkDialog     parent;

  GSettings    *settings;
  gulong        settings_changed_id;
  gboolean      ignore_changed_signal;

  GtkWidget    *explanation;

  GtkTreeStore *store;
  GtkWidget    *stocks;

  GtkWidget    *indexexpansion;
  GtkWidget    *hidecharts;

  GtkWidget    *currency;
  gchar        *currency_code;
};

enum
{
  COL_SYMBOL,
  COL_LABEL,
  COL_AMOUNT,
  COL_PRICE,
  COL_COMMISION,
  COL_CURRENCY_RATE,

  NUM_COLS
};

enum
{
  PROP_0,

  PROP_SETTINGS,

  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL };

G_DEFINE_TYPE (InvestPreferences, invest_preferences, GTK_TYPE_DIALOG)

static gchar *
format_currency (const gchar *label,
                 const gchar *code)
{
  if (code == NULL)
    return g_strdup (label);

  return g_strdup_printf ("%s (%s)", label, code);
}

static gboolean
is_group (InvestPreferences *preferences,
          GtkTreeIter       *iter)
{
  gchar *label;
  gboolean is_group;

  label = NULL;
  gtk_tree_model_get (GTK_TREE_MODEL (preferences->store), iter, 1, &label, -1);

  is_group = label == NULL ? TRUE : FALSE;
  g_free (label);

  return is_group;
}

static gboolean
is_stock (InvestPreferences *preferences,
          GtkTreeIter       *iter)
{
  return !is_group (preferences, iter);
}

static GVariant *
get_stocks (InvestPreferences *preferences,
            GtkTreeModel      *model,
            GtkTreeIter       *parent)
{
  GVariantBuilder builder;
  GtkTreeIter iter;
  gboolean valid;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("a{bv}"));

  valid = gtk_tree_model_iter_children (model, &iter, parent);

  while (valid)
    {
      if (is_group (preferences, &iter))
        {
          gchar *name;

          gtk_tree_model_get (model, &iter, COL_SYMBOL, &name, -1);

          g_variant_builder_add (&builder, "{bv}", TRUE,
                                 g_variant_new ("(sv)", name,
                                                get_stocks (preferences,
                                                            model, &iter)));

          g_free (name);
        }
      else
        {
          gchar *symbol;
          gchar *label;
          gdouble amount;
          gdouble price;
          gdouble comission;
          gdouble currency_rate;

          gtk_tree_model_get (model, &iter,
                              COL_SYMBOL, &symbol,
                              COL_LABEL, &label,
                              COL_AMOUNT, &amount,
                              COL_PRICE, &price,
                              COL_COMMISION, &comission,
                              COL_CURRENCY_RATE, &currency_rate,
                              -1);

          g_variant_builder_add (&builder, "{bv}", FALSE,
                                 g_variant_new ("(ssdddd)", symbol, label,
                                                amount, price, comission,
                                                currency_rate));

          g_free (label);
          g_free (symbol);
        }

      valid = gtk_tree_model_iter_next (model, &iter);
    }

  return g_variant_builder_end (&builder);
}

static void
save_stocks (InvestPreferences *preferences)
{
  GtkTreeModel *model;
  GVariant *stocks;

  model = GTK_TREE_MODEL (preferences->store);
  stocks = get_stocks (preferences, model, NULL);

  preferences->ignore_changed_signal = TRUE;
  g_settings_set_value (preferences->settings, "stocks", stocks);
  preferences->ignore_changed_signal = FALSE;
}

static void
add_clicked_cb (InvestPreferences *preferences,
                gboolean           group,
                gboolean           save)
{
  GtkTreeView *tree_view;
  GtkTreeSelection *selection;
  GtkTreeStore *store;
  GtkTreeModel *model;
  gboolean has_parent;
  GtkTreeIter parent;
  GtkTreePath *path;
  GtkTreeViewColumn *column;

  tree_view = GTK_TREE_VIEW (preferences->stocks);
  selection = gtk_tree_view_get_selection (tree_view);
  store = preferences->store;
  has_parent = FALSE;
  path = NULL;
  column = NULL;

  if (gtk_tree_selection_get_selected (selection, &model, &parent))
    {
      GtkTreeIter iter;

      has_parent = TRUE;

      while (has_parent && is_stock (preferences, &parent))
        {
          has_parent = gtk_tree_model_iter_parent (model, &iter, &parent);

          if (has_parent)
            {
              parent = iter;
            }
        }
    }

  if (group == TRUE)
    {
      GtkTreeIter iter;

      gtk_tree_store_append (store, &iter, has_parent ? &parent : NULL);
      gtk_tree_store_set (store, &iter, COL_SYMBOL, _("Stock Group"), -1);

      path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_view_set_cursor (tree_view, path, NULL, FALSE);
      add_clicked_cb (preferences, FALSE, FALSE);

      gtk_tree_view_expand_row (tree_view, path, FALSE);
    }
  else
    {
      GtkTreeIter iter;

      gtk_tree_store_append (store, &iter, has_parent ? &parent : NULL);
      gtk_tree_store_set (store, &iter,
                          COL_SYMBOL, "YHOO",
                          COL_LABEL, "Yahoo! Inc.",
                          COL_AMOUNT, 0.0,
                          COL_PRICE, 0.0,
                          COL_COMMISION, 0.0,
                          COL_CURRENCY_RATE, 0.0,
                          -1);

      if (has_parent)
        {
          path = gtk_tree_model_get_path (model, &parent);
          gtk_tree_view_expand_row (tree_view, path, FALSE);
        }

      path = gtk_tree_model_get_path (model, &iter);
    }

  column = gtk_tree_view_get_column (tree_view, COL_SYMBOL);
  gtk_tree_view_set_cursor (tree_view, path, column, TRUE);

  if (save == TRUE)
    {
      save_stocks (preferences);
    }
}

static void
addstock_clicked_cb (GtkButton         *button,
                     InvestPreferences *preferences)
{
  add_clicked_cb (preferences, FALSE, TRUE);
}

static void
addgroup_clicked_cb (GtkButton         *button,
                     InvestPreferences *preferences)
{
  add_clicked_cb (preferences, TRUE, TRUE);
}

static void
remove_clicked_cb (GtkButton         *button,
                   InvestPreferences *preferences)
{
  gboolean removed;
  GtkTreeView *tree_view;
  GtkTreeSelection *selection;
  GList *selected_rows;
  GtkTreeModel *model;
  GList *row;

  removed = FALSE;
  tree_view = GTK_TREE_VIEW (preferences->stocks);
  selection = gtk_tree_view_get_selection (tree_view);

  selected_rows = gtk_tree_selection_get_selected_rows (selection, &model);

  for (row = selected_rows; row != NULL; row = row->next)
    {
      GtkTreePath *path;
      GtkTreeIter iter;

      path = (GtkTreePath *) row->data;

      if (!gtk_tree_model_get_iter (model, &iter, path))
        continue;

      if (gtk_tree_model_iter_n_children (model, &iter) > 0)
        {
          GtkDialogFlags flags;
          GtkWidget *dialog;
          GtkWidget *content_area;
          const gchar *text;
          GtkWidget *label;
          gint response;

          flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

          /* Translators: Asks the user to confirm deletion of a group of stocks */
          dialog = gtk_dialog_new_with_buttons (_("Delete entire stock group?"),
                                                GTK_WINDOW (preferences), flags,
                                                _("_Cancel"), GTK_RESPONSE_REJECT,
                                                _("_OK"), GTK_RESPONSE_ACCEPT,
                                                NULL);

          content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

          gtk_widget_set_margin_start (content_area, 10);
          gtk_widget_set_margin_end (content_area, 10);
          gtk_widget_set_margin_bottom (content_area, 10);

          /* Translators: stocks can be grouped together into a "stock group".
           * The user wants to delete a group, but the group still contains
           * stocks. By deleting the group, also all stocks will be removed
           * from configuration.
           */
          text = _("This removes all stocks contained in this stock group!\n"
                   "Do you really want to remove this stock group?");
          label = gtk_label_new (text);

          gtk_box_pack_start (GTK_BOX (content_area), label, TRUE, TRUE, 10);
          gtk_widget_show (label);

          response = gtk_dialog_run (GTK_DIALOG (dialog));
          gtk_widget_destroy (dialog);

          if (response == GTK_RESPONSE_REJECT)
            continue;
        }

      gtk_tree_store_remove (preferences->store, &iter);
      removed = TRUE;
    }

  g_list_free_full (selected_rows, (GDestroyNotify) gtk_tree_path_free);

  if (removed == TRUE)
    {
      save_stocks (preferences);
    }
}

static gboolean
stocks_key_press_event_cb (GtkWidget         *widget,
                           GdkEventKey       *event,
                           InvestPreferences *preferences)
{
  if (event->keyval == 65535)
    remove_clicked_cb (NULL, preferences);

  return FALSE;
}

typedef struct
{
  InvestPreferences *preferences;
  gint               column;
  GType              type;
} CellData;

static CellData *
cell_data_new (InvestPreferences *preferences,
               gint               column,
               GType              type)
{
  CellData *data;

  data = g_new0 (CellData, 1);

  data->preferences = preferences;
  data->column = column;
  data->type = type;

  return data;
}

static void
cell_data_free (gpointer  data,
                GClosure *closure)
{
  g_free (data);
}

static void
edited_cb (GtkCellRendererText *renderer,
           gchar               *path,
           gchar               *new_text,
           CellData            *data)
{
  InvestPreferences *preferences;
  GtkTreeStore *store;
  GtkTreeModel *model;
  GtkTreeIter iter;

  preferences = data->preferences;
  store = preferences->store;
  model = GTK_TREE_MODEL (store);

  if (!gtk_tree_model_get_iter_from_string (model, &iter, path))
    return;

  if (data->column != 0 && is_group (preferences, &iter))
    return;

  if (data->column == 0 && is_stock (preferences, &iter))
    {
      gchar *text;

      text = g_utf8_strup (new_text, -1);

      if (text != NULL && text[0] != '\0')
        gtk_tree_store_set (store, &iter, data->column, text, -1);
      g_free (text);
    }
  else if (data->column < 2)
    {
      if (new_text != NULL && new_text[0] != '\0')
        gtk_tree_store_set (store, &iter, data->column, new_text, -1);
    }
  else
    {
      gdouble value;

      value = g_ascii_strtod (new_text, NULL);

			gtk_tree_store_set (store, &iter, data->column, value, -1);
    }

  save_stocks (preferences);
}

static void
cell_data_func (GtkTreeViewColumn *tree_column,
                GtkCellRenderer   *cell,
                GtkTreeModel      *tree_model,
                GtkTreeIter       *iter,
                gpointer           data)
{
  CellData *cell_data;
  InvestPreferences *preferences;

  cell_data = (CellData *) data;
  preferences = cell_data->preferences;

  if (is_group (preferences, iter))
    {
      if (cell_data->column == 0)
        {
          gchar *text;
          gchar *markup;

          gtk_tree_model_get (tree_model, iter, cell_data->column, &text, -1);

          markup = g_strdup_printf ("<b>%s</b>", text);
          g_free (text);

          g_object_set (cell, "markup", markup, NULL);
          g_free (markup);
        }
      else
        {
          g_object_set (cell, "text", "", NULL);
        }
    }
  else
    {
      gchar *text;

      if (cell_data->type == G_TYPE_DOUBLE)
        {
          gdouble val;

          gtk_tree_model_get (tree_model, iter, cell_data->column, &val, -1);

          text = g_strdup_printf ("%.2f", val);
        }
      else
        {
          gtk_tree_model_get (tree_model, iter, cell_data->column, &text, -1);
        }

      g_object_set (cell, "text", text, NULL);
      g_free (text);
    }
}

static void
create_cell (InvestPreferences *preferences,
             GtkTreeView       *tree_view,
             gint               column,
             const gchar       *name,
             GType              type)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *view_column;
  CellData *data;

  renderer = gtk_cell_renderer_text_new ();
  view_column = gtk_tree_view_column_new_with_attributes (name, renderer,
                                                          "text", column,
                                                          NULL);

  if (type == G_TYPE_DOUBLE)
    gtk_cell_renderer_set_alignment (renderer, 1.0, 0.5);

  g_object_set (renderer, "editable", TRUE, NULL);

  data = cell_data_new (preferences, column, type);
  g_signal_connect_data (renderer, "edited", G_CALLBACK (edited_cb),
                         data, cell_data_free, G_CONNECT_AFTER);

  if (type == G_TYPE_STRING)
    gtk_tree_view_column_set_sort_column_id (view_column, column);

  data = cell_data_new (preferences, column, type);
  gtk_tree_view_column_set_cell_data_func (view_column, renderer,
                                           cell_data_func, data, g_free);

  gtk_tree_view_append_column (tree_view, view_column);
}

static void
add_exchange_column (InvestPreferences *preferences)
{
  GtkTreeView *tree_view;

  tree_view = GTK_TREE_VIEW (preferences->stocks);

  if (gtk_tree_view_get_n_columns (tree_view) != NUM_COLS)
    {
      create_cell (preferences, tree_view,
                   COL_CURRENCY_RATE, _("Currency Rate"), G_TYPE_DOUBLE);
    }
}

static void
remove_exchange_column (InvestPreferences *preferences)
{
  GtkTreeView *tree_view;

  tree_view = GTK_TREE_VIEW (preferences->stocks);

  if (gtk_tree_view_get_n_columns (tree_view) == NUM_COLS)
    {
      GtkTreeViewColumn *column;

      column = gtk_tree_view_get_column (tree_view, COL_CURRENCY_RATE);

      gtk_tree_view_remove_column (tree_view, column);
    }
}

static void
pick_currency (InvestPreferences *preferences,
               const gchar       *currency,
               gboolean           save)
{
  gchar *code;
  gchar *label;

  code = g_strdup (currency);

  if (code == NULL)
    {
      label = g_strdup ("");
      remove_exchange_column (preferences);
    }
  else
    {
      gint i;

      for (i = 0; currencies[i].code != NULL; i++)
        {
          if (g_strcmp0 (code, currencies[i].code) == 0)
            {
              label = format_currency (currencies[i].label, code);
              break;
            }
        }

      if (label == NULL)
        label = g_strdup ("");

      add_exchange_column (preferences);
    }

  g_free (preferences->currency_code);
  preferences->currency_code = code;

  gtk_entry_set_text (GTK_ENTRY (preferences->currency), label);
  g_free (label);

  if (save)
    {
      g_settings_set_string (preferences->settings, "currency",
                             code != NULL ? code : "");
    }
}

static void
match_currency (InvestPreferences *preferences)
{
  GtkEntry *entry;
  gchar *text;
  guint16 length;
  gint i;

  entry = GTK_ENTRY (preferences->currency);
  length = gtk_entry_get_text_length (entry);

  if (length == 0)
    {
      pick_currency (preferences, NULL, TRUE);

      return;
    }
  else if (length == 3)
    {
      text = g_utf8_strup (gtk_entry_get_text (entry), -1);

      for (i = 0; currencies[i].code != NULL; i++)
        {
          if (g_strcmp0 (text, currencies[i].code) == 0)
            {
              pick_currency (preferences, text, TRUE);
              g_free (text);

              return;
            }
        }

      g_free (text);
    }
  else
    {
      text = g_utf8_strup (gtk_entry_get_text (entry), -1);

      for (i = 0; currencies[i].code != NULL; i++)
        {
          gchar *label;
          gchar *formatted;
          gboolean found;

          label = g_utf8_strup (currencies[i].label, -1);
          formatted = format_currency (label, currencies[i].code);
          found = FALSE;

          if (g_strcmp0 (label, text) == 0 ||
              g_strcmp0 (formatted, text) == 0)
            {
              pick_currency (preferences, currencies[i].code, TRUE);
              found = TRUE;
            }

          g_free (formatted);
          g_free (label);

          if (found)
            {
              g_free (text);
              return;
            }
        }

      g_free (text);
    }

  pick_currency (preferences, preferences->currency_code, TRUE);
}

static gboolean
currency_focus_out_event_cb (GtkWidget         *widget,
                             GdkEventFocus     *event,
                             InvestPreferences *preferences)
{
  match_currency (preferences);

  return FALSE;
}

static void
currency_activate_cb (GtkEntry          *entry,
                      InvestPreferences *preferences)
{
  match_currency (preferences);
}

static void
prefs_dialog_response_cb (GtkDialog *dialog,
                          gint       response_id,
                          gpointer   user_data)
{
  GdkScreen *screen;

  screen = gdk_screen_get_default ();

  switch (response_id)
    {
      case 1:
        gtk_show_uri (screen, "help:invest-applet/invest-applet-usage",
                      GDK_CURRENT_TIME, NULL);
        break;

      default:
        gtk_widget_destroy (GTK_WIDGET (dialog));
        break;
    }
}

static void
setup_stocks (InvestPreferences *preferences)
{
  GtkTreeView *tree_view;
  GtkTreeSortable *sortable;

  tree_view = GTK_TREE_VIEW (preferences->stocks);

  preferences->store = gtk_tree_store_new (NUM_COLS, G_TYPE_STRING, G_TYPE_STRING,
                                           G_TYPE_DOUBLE, G_TYPE_DOUBLE,
                                           G_TYPE_DOUBLE, G_TYPE_DOUBLE);

  sortable = GTK_TREE_SORTABLE (preferences->store);
  gtk_tree_sortable_set_sort_column_id (sortable, 0, GTK_SORT_ASCENDING);

  gtk_tree_view_set_model (tree_view, GTK_TREE_MODEL (preferences->store));

  create_cell (preferences, tree_view, COL_SYMBOL, _("Symbol"), G_TYPE_STRING);
  create_cell (preferences, tree_view, COL_LABEL, _("Label"), G_TYPE_STRING);
  create_cell (preferences, tree_view, COL_AMOUNT, _("Amount"), G_TYPE_DOUBLE);
  create_cell (preferences, tree_view, COL_PRICE, _("Price"), G_TYPE_DOUBLE);
  create_cell (preferences, tree_view, COL_COMMISION, _("Commission"), G_TYPE_DOUBLE);

  if (preferences->currency_code != NULL)
    add_exchange_column (preferences);
}

static gboolean
match_selected_cb (GtkEntryCompletion *widget,
                   GtkTreeModel       *model,
                   GtkTreeIter        *iter,
                   InvestPreferences  *preferences)
{
  GValue value = G_VALUE_INIT;

  gtk_tree_model_get_value (model, iter, 1, &value);

  pick_currency (preferences, g_value_get_string (&value), TRUE);

  return TRUE;
}

static gboolean
match_func (GtkEntryCompletion *completion,
            const gchar        *key,
            GtkTreeIter        *iter,
            gpointer            user_data)
{
  GValue value = G_VALUE_INIT;
  GtkTreeModel *model;
  gchar *label;
  gchar **tokens;
  gchar **words;
  gboolean match;
  gint i;

  model = gtk_entry_completion_get_model (completion);
  gtk_tree_model_get_value (model, iter, 0, &value);

  label = g_utf8_strdown (g_value_get_string (&value), -1);
  tokens = g_strsplit (label, " ", -1);
  g_free (label);

  words = g_strsplit (key, " ", -1);
  match = TRUE;

  for (i = 0; words[i] != NULL; i++)
    {
      gboolean found;
      gint j;

      found = FALSE;

      for (j = 0; tokens[j] != NULL; j++)
        {
          const gchar *token;

          token = tokens[j];
          if (g_str_has_prefix (token, "("))
            token++;

          if (g_str_has_prefix (token, words[i]))
            {
              found = TRUE;
              break;
            }
        }

      if (found == FALSE)
        {
          match = FALSE;
          break;
        }
    }

  g_strfreev (tokens);
  g_strfreev (words);

  return match;
}

static void
setup_completion (InvestPreferences *preferences)
{
  GtkListStore *store;
  GtkEntryCompletion *completion;
  gint i;

  store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
  completion = gtk_entry_completion_new ();

  for (i = 0; currencies[i].code != NULL; i++)
    {
      gchar *formatted;
      GtkTreeIter iter;

      formatted = format_currency (currencies[i].label, currencies[i].code);

      gtk_list_store_append (store, &iter);
      gtk_list_store_set (store, &iter, 0, formatted,
                          1, currencies[i].code, -1);

      g_free (formatted);
    }

  g_signal_connect (completion, "match-selected",
                    G_CALLBACK (match_selected_cb), preferences);

  gtk_entry_completion_set_match_func (completion, match_func, NULL, NULL);
  gtk_entry_completion_set_model (completion, GTK_TREE_MODEL (store));
  gtk_entry_completion_set_text_column (completion, 0);
  gtk_entry_set_completion (GTK_ENTRY (preferences->currency), completion);
}

static void
add_to_store (GtkTreeStore *store,
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

          gtk_tree_store_append (store, &row, parent);
          gtk_tree_store_set (store, &row, COL_SYMBOL, name, -1);

          add_to_store (store, list, &row);
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
          GtkTreeIter row;

          g_variant_get (variant, "(&s&sdddd)", &symbol, &label,
                         &amount, &price, &comission, &currency_rate);

          gtk_tree_store_append (store, &row, parent);
          gtk_tree_store_set (store, &row,
                              COL_SYMBOL, symbol,
                              COL_LABEL, label,
                              COL_AMOUNT, amount,
                              COL_PRICE, price,
                              COL_COMMISION, comission,
                              COL_CURRENCY_RATE, currency_rate,
                              -1);
        }
    }
}

static void
settings_changed_cb (GSettings         *settings,
                     const gchar       *key,
                     InvestPreferences *preferences)
{
  if (preferences->ignore_changed_signal == TRUE)
    return;

  if (key == NULL || g_strcmp0 (key, "currency") == 0)
    {
      gboolean found;
      gchar *currency;
      gint i;

      found = FALSE;
      currency = g_settings_get_string (settings, "currency");

      for (i = 0; currencies[i].code != NULL; i++)
        {
          if (g_strcmp0 (currency, currencies[i].code) == 0)
            {
              found = TRUE;
              pick_currency (preferences, currency, FALSE);
              break;
            }
        }

      if (!found)
        {
          pick_currency (preferences, NULL, FALSE);
        }

      g_free (currency);
    }

  if (key == NULL || g_strcmp0 (key, "stocks") == 0)
    {
      GVariant *stocks;

      stocks = g_settings_get_value (preferences->settings, "stocks");

      gtk_tree_store_clear (preferences->store);
      add_to_store (preferences->store, stocks, NULL);
      g_variant_unref (stocks);
    }
}

static void
invest_preferences_constructed (GObject *object)
{
  InvestPreferences *preferences;

  preferences = INVEST_PREFERENCES (object);

  G_OBJECT_CLASS (invest_preferences_parent_class)->constructed (object);

  setup_stocks (preferences);
  setup_completion (preferences);

  preferences->settings_changed_id =
    g_signal_connect (preferences->settings, "changed",
                      G_CALLBACK (settings_changed_cb), preferences);

  g_settings_bind (preferences->settings, "indexexpansion",
                   preferences->indexexpansion, "active",
                   G_SETTINGS_BIND_DEFAULT);

  g_settings_bind (preferences->settings, "hidecharts",
                   preferences->hidecharts, "active",
                   G_SETTINGS_BIND_DEFAULT);

  settings_changed_cb (preferences->settings, NULL, preferences);
}

static void
invest_preferences_dispose (GObject *object)
{
  InvestPreferences *prefs;

  prefs = INVEST_PREFERENCES (object);

  if (prefs->settings_changed_id > 0)
    {
      g_signal_handler_disconnect (prefs->settings, prefs->settings_changed_id);
      prefs->settings_changed_id = 0;
    }

  g_clear_object (&prefs->store);
  g_clear_object (&prefs->settings);

  G_OBJECT_CLASS (invest_preferences_parent_class)->dispose (object);
}

static void
invest_preferences_finalize (GObject *object)
{
  InvestPreferences *preferences;

  preferences = INVEST_PREFERENCES (object);

  g_free (preferences->currency_code);

  G_OBJECT_CLASS (invest_preferences_parent_class)->finalize (object);
}

static void
invest_preferences_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  InvestPreferences *preferences;

  preferences = INVEST_PREFERENCES (object);

  switch (property_id)
    {
      case PROP_SETTINGS:
        preferences->settings = g_value_dup_object (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
invest_preferences_install_properties (GObjectClass *object_class)
{
  properties[PROP_SETTINGS] =
    g_param_spec_object ("settings", "settings", "settings", G_TYPE_SETTINGS,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
                        G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
invest_preferences_bind_template (GtkWidgetClass *widget_class)
{
  gchar *contents;
  gsize length;
  GBytes *bytes;

  g_file_get_contents (BUILDERDIR "/prefs-dialog.ui", &contents, &length, NULL);

  bytes = g_bytes_new_take (contents, length);
  gtk_widget_class_set_template (widget_class, bytes);
  g_bytes_unref (bytes);

  gtk_widget_class_bind_template_child (widget_class, InvestPreferences, explanation);

  gtk_widget_class_bind_template_callback (widget_class, addstock_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, addgroup_clicked_cb);
  gtk_widget_class_bind_template_callback (widget_class, remove_clicked_cb);

  gtk_widget_class_bind_template_child (widget_class, InvestPreferences, stocks);
  gtk_widget_class_bind_template_callback (widget_class, stocks_key_press_event_cb);

  gtk_widget_class_bind_template_child (widget_class, InvestPreferences, indexexpansion);
  gtk_widget_class_bind_template_child (widget_class, InvestPreferences, hidecharts);

  gtk_widget_class_bind_template_child (widget_class, InvestPreferences, currency);
  gtk_widget_class_bind_template_callback (widget_class, currency_focus_out_event_cb);
  gtk_widget_class_bind_template_callback (widget_class, currency_activate_cb);

  gtk_widget_class_bind_template_callback (widget_class, prefs_dialog_response_cb);
}

static void
invest_preferences_class_init (InvestPreferencesClass *preferences_class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = G_OBJECT_CLASS (preferences_class);
  widget_class = GTK_WIDGET_CLASS (preferences_class);

  object_class->constructed = invest_preferences_constructed;
  object_class->dispose = invest_preferences_dispose;
  object_class->finalize = invest_preferences_finalize;
  object_class->set_property = invest_preferences_set_property;

  invest_preferences_install_properties (object_class);
  invest_preferences_bind_template (widget_class);
}

static void
invest_preferences_init (InvestPreferences *preferences)
{
  GtkWidget *widget;

  widget = GTK_WIDGET (preferences);

  gtk_widget_init_template (widget);
}

GtkWidget *
invest_preferences_new (GSettings *settings)
{
  return g_object_new (INVEST_TYPE_PREFERENCES,
                       "settings", settings,
                       NULL);
}

void
invest_preferences_set_explanation (InvestPreferences *preferences,
                                    const gchar       *explanation)
{
  if (explanation == NULL)
    {
      gtk_widget_hide (preferences->explanation);
    }
  else
    {
      gtk_label_set_markup (GTK_LABEL (preferences->explanation), explanation);
      gtk_widget_show (preferences->explanation);
    }
}
