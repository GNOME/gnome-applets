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

/*
 * p:
 *  eX = Exponential Moving Average
 *  mX = Moving Average
 *  b = Bollinger Bands Overlay
 *  v = Volume Overlay
 *  p = Parabolic SAR overlay
 *  s = Splits Overlay
 * q:
 *  l = Line
 *  c = Candles
 *  b = Bars
 * l:
 *  on = Logarithmic
 *  off = Linear
 * z:
 *  l = Large
 *  m = Medium
 * t:
 *  Xd = X Days
 *  Xm = X Months
 *  Xy = X Years
 * a:
 *  fX = MFI X days
 *  ss = Slow Stochastic
 *  fs = Fast Stochastic
 *  wX = W%R X Days
 *  mX-Y-Z = MACD X Days, Y Days, Signal
 *  pX = ROC X Days
 *  rX = RSI X Days
 *  v = Volume
 *  vm = Volume +MA
 * c:
 *  X = compare with X
 */

#include "config.h"

#include <glib/gi18n.h>

#include "invest-chart.h"
#include "invest-image-retriever.h"

#define AUTOREFRESH_TIMEOUT 20 * 60 * 1000
#define CHART_URL "http://chart.finance.yahoo.com/z?s=%s&c=%s&t=%s&q=%s&l=%s&z=l&p=%s&a=%s"

struct _InvestChart
{
  GtkWindow             parent;

  GtkWidget            *s;
  GtkWidget            *autorefresh;
  GtkWidget            *plot;
  GtkWidget            *progress;
  GtkWidget            *t;
  GtkWidget            *q;
  GtkWidget            *l;
  GtkWidget            *pm5;
  GtkWidget            *pm10;
  GtkWidget            *pm20;
  GtkWidget            *pm50;
  GtkWidget            *pm100;
  GtkWidget            *pm200;
  GtkWidget            *pe5;
  GtkWidget            *pe10;
  GtkWidget            *pe20;
  GtkWidget            *pe50;
  GtkWidget            *pe100;
  GtkWidget            *pe200;
  GtkWidget            *pb;
  GtkWidget            *pp;
  GtkWidget            *ps;
  GtkWidget            *pv;
  GtkWidget            *ar;
  GtkWidget            *af;
  GtkWidget            *ap;
  GtkWidget            *aw;
  GtkWidget            *am;
  GtkWidget            *ass;
  GtkWidget            *afs;
  GtkWidget            *av;
  GtkWidget            *avm;

  guint                 autorefresh_id;

  InvestImageRetriever *retriever;
};

G_DEFINE_TYPE (InvestChart, invest_chart, GTK_TYPE_WINDOW)

static void
completed_cb (InvestImageRetriever *retriever,
              GdkPixbuf            *pixbuf,
              InvestChart          *chart)
{
  GtkImage *image;
  GtkLabel *label;

  image = GTK_IMAGE (chart->plot);
  label = GTK_LABEL (chart->progress);

  if (pixbuf != NULL)
    {
      gtk_image_set_from_pixbuf (image, pixbuf);
      gtk_label_set_text (label, _("Chart downloaded"));
    }
  else
    {
      GdkPixbuf *logo;

      logo = gdk_pixbuf_new_from_file_at_size (ARTDIR "/invest_neutral.svg",
                                               96, 96, NULL);

      gtk_image_set_from_pixbuf (image, logo);
      gtk_label_set_text (label, _("Chart could not be downloaded"));

      g_clear_object (&logo);
    }
}

static gchar *
get_p (InvestChart *chart)
{
  GPtrArray *array;
  gchar **elements;
  gchar *p;

  array = g_ptr_array_new ();

#define ADD_P(variable, value) \
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chart->variable))) \
    g_ptr_array_add (array, g_strdup (value));

  ADD_P (pm5, "m5")
  ADD_P (pm10, "m10")
  ADD_P (pm20, "m20")
  ADD_P (pm50, "m50")
  ADD_P (pm100, "m100")
  ADD_P (pm200, "m200")
  ADD_P (pe5, "e5")
  ADD_P (pe10, "e10")
  ADD_P (pe20, "e20")
  ADD_P (pe50, "e50")
  ADD_P (pe100, "e100")
  ADD_P (pe200, "e200")
  ADD_P (pb, "b")
  ADD_P (pp, "p")
  ADD_P (ps, "s")
  ADD_P (pv, "v")

#undef ADD_p

  g_ptr_array_add (array, NULL);
  elements = (gchar **) g_ptr_array_free (array, FALSE);

  p = g_strjoinv (",", elements);
  g_strfreev (elements);

  return p;
}

static gchar *
get_a (InvestChart *chart)
{
  GPtrArray *array;
  gchar **elements;
  gchar *a;

  array = g_ptr_array_new ();

#define ADD_A(variable, value) \
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (chart->variable))) \
    g_ptr_array_add (array, g_strdup (value));

  ADD_A (ar, "r14")
  ADD_A (af, "f14")
  ADD_A (ap, "p12")
  ADD_A (aw, "w14")
  ADD_A (am, "m26-12-9")
  ADD_A (ass, "ss")
  ADD_A (afs, "fs")
  ADD_A (av, "v")
  ADD_A (avm, "vm")

#undef ADD_A

  g_ptr_array_add (array, NULL);
  elements = (gchar **) g_ptr_array_free (array, FALSE);

  a = g_strjoinv (",", elements);
  g_strfreev (elements);

  return a;
}

static gboolean
refresh_chart (InvestChart *chart)
{
  const gchar *text;
  gchar *tickers;
  gchar **symbols;
  gchar *tail;
  gchar *title;
  const gchar *t, *q, *l;
  gchar *s, *c, *p, *a;
  gchar *url;

  text = gtk_entry_get_text (GTK_ENTRY (chart->s));
  tickers = g_strstrip (g_utf8_strup (text, -1));

  if (!tickers || *tickers == '\0')
    return TRUE;

  symbols = g_strsplit (tickers, " ", -1);
  g_free (tickers);

  tail = g_strjoinv (" / ", symbols);
  title = g_strdup_printf (_("Financial Chart - %s"), tail);
  g_free (tail);

  gtk_window_set_title (GTK_WINDOW (chart), title);
  g_free (title);

  t = gtk_combo_box_get_active_id (GTK_COMBO_BOX (chart->t));
  q = gtk_combo_box_get_active_id (GTK_COMBO_BOX (chart->q));
  l = gtk_combo_box_get_active_id (GTK_COMBO_BOX (chart->l));

  s = g_strdup (symbols[0]);
  c = g_strdup ("");

  if (g_strv_length (symbols) > 1)
    {
      guint i;

      for (i = 1; i < g_strv_length (symbols); i++)
        {
          gchar *tmp;

          if (*c == '\0')
            tmp = g_strdup (symbols[i]);
          else
            tmp = g_strdup_printf ("%s,%s", c, symbols[i]);

          g_free (c);
          c = tmp;
        }
    }

  g_strfreev (symbols);

  p = get_p (chart);
  a = get_a (chart);

  url = g_strdup_printf (CHART_URL, s, c, t, q, l, p, a);
  g_free (s); g_free (c); g_free (p); g_free (a);

  gtk_label_set_text (GTK_LABEL (chart->progress), _("Opening Chart"));
  gtk_widget_show (chart->progress);

  g_clear_object (&chart->retriever);
  chart->retriever = invest_image_retriever_new (url);
  g_free (url);

  g_signal_connect (chart->retriever, "completed",
                    G_CALLBACK (completed_cb), chart);

  invest_image_retriever_start (chart->retriever);

  return TRUE;
}

static void
autorefresh_toggled_cb (GtkToggleButton *togglebutton,
                        InvestChart     *chart)
{
  gboolean active;

  active = gtk_toggle_button_get_active (togglebutton);

  if (!active && chart->autorefresh_id != 0)
    {
      g_source_remove (chart->autorefresh_id);
      chart->autorefresh_id = 0;
    }

  if (active && chart->autorefresh_id == 0)
    {
      chart->autorefresh_id = g_timeout_add (AUTOREFRESH_TIMEOUT,
                                             (GSourceFunc) refresh_chart,
                                             chart);

      refresh_chart (chart);
    }
}

static void
s_activate_cb (GtkEntry    *entry,
               InvestChart *chart)
{
  refresh_chart (chart);
}

static void
s_changed_cb (GtkEditable *editable,
              InvestChart *chart)
{
  refresh_chart (chart);
}

static void
changed_cb (GtkComboBox *widget,
            InvestChart *chart)
{
  refresh_chart (chart);
}

static void
toggled_cb (GtkToggleButton *togglebutton,
            InvestChart     *chart)
{
  refresh_chart (chart);
}

static void
invest_chart_constructed (GObject *object)
{
  InvestChart *chart;
  GdkPixbuf *logo;

  chart = INVEST_CHART (object);

  G_OBJECT_CLASS (invest_chart_parent_class)->constructed (object);

  gtk_window_set_title (GTK_WINDOW (chart), _("Financial Chart"));

  logo = gdk_pixbuf_new_from_file_at_size (ARTDIR "/invest_neutral.svg", 96, 96, NULL);
  gtk_image_set_from_pixbuf (GTK_IMAGE (chart->plot), logo);
  g_clear_object (&logo);

  autorefresh_toggled_cb (GTK_TOGGLE_BUTTON (chart->autorefresh), chart);
}

static void
invest_chart_dispose (GObject *object)
{
  InvestChart *chart;

  chart = INVEST_CHART (object);

  if (chart->autorefresh_id != 0)
    {
      g_source_remove (chart->autorefresh_id);
      chart->autorefresh_id = 0;
    }

  g_clear_object (&chart->retriever);

  G_OBJECT_CLASS (invest_chart_parent_class)->dispose (object);
}

static void
invest_chart_bind_template (GtkWidgetClass *widget_class)
{
  gchar *contents;
  gsize length;
  GBytes *bytes;

  g_file_get_contents (BUILDERDIR "/financialchart.ui",
                       &contents, &length, NULL);

  bytes = g_bytes_new_take (contents, length);
  gtk_widget_class_set_template (widget_class, bytes);
  g_bytes_unref (bytes);

  gtk_widget_class_bind_template_child (widget_class, InvestChart, s);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, autorefresh);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, plot);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, progress);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, t);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, q);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, l);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pm5);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pm10);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pm20);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pm50);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pm100);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pm200);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pe5);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pe10);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pe20);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pe50);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pe100);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pe200);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pb);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pp);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, ps);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, pv);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, ar);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, af);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, ap);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, aw);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, am);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, ass);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, afs);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, av);
  gtk_widget_class_bind_template_child (widget_class, InvestChart, avm);

  gtk_widget_class_bind_template_callback (widget_class, autorefresh_toggled_cb);
  gtk_widget_class_bind_template_callback (widget_class, s_activate_cb);
  gtk_widget_class_bind_template_callback (widget_class, s_changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, changed_cb);
  gtk_widget_class_bind_template_callback (widget_class, toggled_cb);
}

static void
invest_chart_class_init (InvestChartClass *chart_class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = G_OBJECT_CLASS (chart_class);
  widget_class = GTK_WIDGET_CLASS (chart_class);

  object_class->constructed = invest_chart_constructed;
  object_class->dispose = invest_chart_dispose;

  invest_chart_bind_template (widget_class);
}

static void
invest_chart_init (InvestChart *chart)
{
  GtkWidget *widget;

  widget = GTK_WIDGET (chart);

  gtk_widget_init_template (widget);
}

void
invest_chart_show_chart (const gchar *symbol)
{
  InvestChart *chart;

  chart = g_object_new (INVEST_TYPE_CHART, NULL);

  gtk_entry_set_text (GTK_ENTRY (chart->s), symbol);
  gtk_window_present (GTK_WINDOW (chart));
}
