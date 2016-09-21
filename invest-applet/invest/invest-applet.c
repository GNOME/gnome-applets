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

#include "invest-applet.h"
#include "invest-preferences.h"
#include "invest-quotes.h"
#include "invest-window.h"

struct _InvestApplet
{
  PanelApplet   parent;

  GSettings    *settings;

  GtkWidget    *icon;
  GtkWidget    *preferences;

  InvestQuotes *quotes;

  GtkWidget    *window;
};

struct _InvestAppletClass
{
  PanelAppletClass parent_class;
};

G_DEFINE_TYPE (InvestApplet, invest_applet, PANEL_TYPE_APPLET)

static void
update_popup_position (InvestApplet *applet)
{
  GtkWindow *popup_window;
  GdkDisplay *display;
  GdkWindow *applet_window;
  GdkMonitor *monitor;
  GdkRectangle geometry;
  GtkAllocation allocation;
  gint x, y, w, h;
  GdkGravity gravity;

  g_assert (applet->window != NULL);

  popup_window = GTK_WINDOW (applet->window);

  display = gdk_display_get_default ();
  applet_window = gtk_widget_get_window (GTK_WIDGET (applet));
  monitor = gdk_display_get_monitor_at_window (display, applet_window);

  gdk_monitor_get_geometry (monitor, &geometry);

  gtk_widget_get_allocation (GTK_WIDGET (applet), &allocation);
  gdk_window_get_origin (applet_window, &x, &y);
  gtk_window_get_size (popup_window, &w, &h);

  gravity = GDK_GRAVITY_NORTH_WEST;
  switch (panel_applet_get_orient (PANEL_APPLET (applet)))
    {
      case PANEL_APPLET_ORIENT_RIGHT:
        x += allocation.width;
        if ((y + h) > geometry.y + geometry.height)
          y -= (y + h) - (geometry.y + geometry.height);

        if ((y + h) > (geometry.height / 2))
          gravity = GDK_GRAVITY_SOUTH_WEST;
        else
          gravity = GDK_GRAVITY_NORTH_WEST;
        break;

      case PANEL_APPLET_ORIENT_LEFT:
        x -= w;
        if ((y + h) > geometry.y + geometry.height)
          y -= (y + h) - (geometry.y + geometry.height);

        if ((y + h) > (geometry.height / 2))
          gravity = GDK_GRAVITY_SOUTH_EAST;
        else
          gravity = GDK_GRAVITY_NORTH_EAST;
        break;

      case PANEL_APPLET_ORIENT_DOWN:
        y += allocation.height;
        if ((x + w) > geometry.x + geometry.width)
          x -= (x + w) - (geometry.x + geometry.width);

        gravity = GDK_GRAVITY_NORTH_WEST;
        break;

      case PANEL_APPLET_ORIENT_UP:
        y -= h;
        if ((x + w) > geometry.x + geometry.width)
          x -= (x + w) - (geometry.x + geometry.width);

        gravity = GDK_GRAVITY_SOUTH_WEST;
        break;

      default:
        break;
    }

  gtk_window_move (popup_window, x, y);
  gtk_window_set_gravity (popup_window, gravity);
}

static void
preferences_destroy_cb (GtkWidget    *widget,
                        InvestApplet *applet)
{
  applet->preferences = NULL;
}

static void
show_preferences (InvestApplet *applet,
                  const gchar  *explanation)
{
  InvestPreferences *preferences;

  if (applet->preferences != NULL)
    {
      preferences = INVEST_PREFERENCES (applet->preferences);

      invest_preferences_set_explanation (preferences, explanation);
      gtk_window_present (GTK_WINDOW (applet->preferences));

      return;
    }

  applet->preferences = invest_preferences_new (applet->settings);
  preferences = INVEST_PREFERENCES (applet->preferences);

  g_signal_connect (preferences, "destroy",
                    G_CALLBACK (preferences_destroy_cb), applet);

  invest_preferences_set_explanation (preferences, explanation);
  gtk_window_present (GTK_WINDOW (applet->preferences));
}

static void
set_icon (InvestApplet *applet,
          gint          change)
{
  GError *error;
  GdkPixbuf *pixbuf;

  error = NULL;

  if (change == 1)
    {
      pixbuf = gdk_pixbuf_new_from_file_at_size (ARTDIR "/invest-22_up.png",
                                                 -1, -1, &error);
    }
  else if (change == 0)
    {
      pixbuf = gdk_pixbuf_new_from_file_at_size (ARTDIR "/invest-22_neutral.png",
                                                 -1, -1, &error);
    }
  else
    {
      pixbuf = gdk_pixbuf_new_from_file_at_size (ARTDIR "/invest-22_down.png",
                                                 -1, -1, &error);
    }

  if (error)
    {
      g_warning ("%s", error->message);
      g_error_free (error);
    }

  gtk_image_set_from_pixbuf (GTK_IMAGE (applet->icon), pixbuf);
  g_clear_object (&pixbuf);
}

static void
setup_applet (InvestApplet *applet)
{
  GtkWidget *hbox;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add (GTK_CONTAINER (applet), hbox);

  applet->icon = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (hbox), applet->icon);
  gtk_widget_show (applet->icon);

  set_icon (applet, 0);
}

static void
refresh_cb (GSimpleAction *action,
            GVariant      *parameter,
            gpointer       user_data)
{
  InvestApplet *applet;

  applet = INVEST_APPLET (user_data);

  invest_quotes_refresh (applet->quotes);
}

static void
preferences_cb (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  InvestApplet *applet;

  applet = INVEST_APPLET (user_data);

  show_preferences (applet, NULL);
}

static void
help_cb (GSimpleAction *action,
         GVariant      *parameter,
         gpointer       user_data)
{
  GdkScreen *screen;

  screen = gdk_screen_get_default ();

  gtk_show_uri (screen, "help:invest-applet", GDK_CURRENT_TIME, NULL);
}

static const gchar *authors[] = {
  "Raphael Slinckx <raphael@slinckx.net>",
  "Enrico Minack <enrico-minack@gmx.de>",
  NULL
};

static void
about_cb (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
  const gchar *copyright;
  GdkPixbuf *logo;

  copyright = "Copyright © 2004-2005 Raphael Slinckx.\nCopyright © 2009-2011 Enrico Minack.";
  logo = gdk_pixbuf_new_from_file_at_size (ARTDIR "/invest_neutral.svg", 96, 96, NULL);

  gtk_show_about_dialog (NULL,
                         "authors", authors,
                         "comments", _("Track your invested money."),
                         "copyright", copyright,
                         "logo", logo,
                         "program-name", _("Invest"),
                         "version", VERSION,
                         "translator-credits", _("translator-credits"),
                         NULL);

  g_clear_object (&logo);
}

static const GActionEntry menu_actions[] = {
  { "refresh",     refresh_cb },
  { "preferences", preferences_cb },
  { "help",        help_cb },
  { "about",       about_cb }
};

static void
setup_menu (PanelApplet *applet)
{
  GSimpleActionGroup *action_group;
  const gchar *file;

  action_group = g_simple_action_group_new ();
  file = UIDIR "/invest-applet-menu.xml";

  g_action_map_add_action_entries (G_ACTION_MAP (action_group), menu_actions,
                                   G_N_ELEMENTS (menu_actions), applet);

  panel_applet_setup_menu_from_file (applet, file, action_group,
                                     GETTEXT_PACKAGE);

  gtk_widget_insert_action_group (GTK_WIDGET (applet), "invest",
                                  G_ACTION_GROUP (action_group));

  g_object_unref (action_group);
}

static void
update_icon_cb (InvestQuotes *quotes,
                gint          icon,
                InvestApplet *applet)
{
  set_icon (applet, icon);
}

static void
update_tooltip_cb (InvestQuotes *quotes,
                   const gchar  *text,
                   InvestApplet *applet)
{
  gtk_widget_set_tooltip_text (GTK_WIDGET (applet), text);
}

static void
invest_applet_setup (PanelApplet *papplet)
{
  InvestApplet *applet;
  const gchar *schema;

  applet = INVEST_APPLET (papplet);

  schema = "org.gnome.gnome-applets.invest";
  applet->settings = panel_applet_settings_new (papplet, schema);

  setup_applet (applet);
  setup_menu (papplet);

  applet->quotes = invest_quotes_new (applet->settings);

  g_signal_connect (applet->quotes, "update-icon",
                    G_CALLBACK (update_icon_cb), applet);

  g_signal_connect (applet->quotes, "update-tooltip",
                    G_CALLBACK (update_tooltip_cb), applet);

  gtk_widget_show_all (GTK_WIDGET (applet));
}

static void
popup_size_allocate_cb (GtkWidget     *widget,
                        GtkAllocation *allocation,
                        InvestApplet  *applet)
{
  update_popup_position (applet);
}

static gboolean
button_press_event_cb (GtkWidget      *widget,
                       GdkEventButton *event,
                       InvestApplet   *applet)
{
  if (event->button != 1 || event->type != GDK_BUTTON_PRESS)
    return FALSE;

  if (!invest_quotes_has_stocks (applet->quotes))
    {
      const gchar *explanation;

      explanation = _("<b>You have not entered any stock information yet</b>");

      g_clear_pointer (&applet->window, gtk_widget_destroy);
      show_preferences (applet, explanation);
    }
  else if (!invest_quotes_is_valid (applet->quotes))
    {
      GtkWidget *dialog;
      GtkMessageDialog *message;
      const gchar *markup;
      const gchar *text;

      dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_INFO,
                                       GTK_BUTTONS_CLOSE, NULL);

      message = GTK_MESSAGE_DIALOG (dialog);
      markup = _("<b>No stock quotes are currently available</b>");
      text = _("The server could not be contacted. The computer is either "
               "offline or the servers are down. Try again later.");

      gtk_message_dialog_set_markup (message, markup);
      gtk_message_dialog_format_secondary_text (message, "%s", text);

      g_signal_connect_swapped (dialog, "response",
                                G_CALLBACK (gtk_widget_destroy),
                                dialog);

      g_clear_pointer (&applet->window, gtk_widget_destroy);
      gtk_window_present (GTK_WINDOW (dialog));
    }
  else
    {
      if (applet->window != NULL)
        {
          gtk_widget_destroy (applet->window);
          applet->window = NULL;
        }
      else
        {
          applet->window = invest_window_new (applet->settings,
                                              applet->quotes);

          g_signal_connect (applet->window, "size-allocate",
                            G_CALLBACK (popup_size_allocate_cb),
                            applet);

          gtk_window_present (GTK_WINDOW (applet->window));
        }
    }

  return FALSE;
}

static void
change_orient_cb (PanelApplet       *papplet,
                  PanelAppletOrient  orient,
                  InvestApplet      *applet)
{
  if (applet->window != NULL)
    update_popup_position (applet);
}

static void
invest_applet_dispose (GObject *object)
{
  InvestApplet *applet;

  applet = INVEST_APPLET (object);

  g_clear_object (&applet->quotes);
  g_clear_pointer (&applet->preferences, gtk_widget_destroy);
  g_clear_pointer (&applet->window, gtk_widget_destroy);
  g_clear_object (&applet->settings);

  G_OBJECT_CLASS (invest_applet_parent_class)->dispose (object);
}

static void
invest_applet_class_init (InvestAppletClass *applet_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (applet_class);

  object_class->dispose = invest_applet_dispose;
}

static void
invest_applet_init (InvestApplet *applet)
{
  panel_applet_set_flags (PANEL_APPLET (applet), PANEL_APPLET_EXPAND_MINOR);

  g_signal_connect (applet, "button-press-event",
                    G_CALLBACK (button_press_event_cb),
                    applet);

  g_signal_connect (applet, "change-orient",
                    G_CALLBACK (change_orient_cb),
                    applet);
}

static gboolean
invest_applet_factory (PanelApplet *applet,
                       const gchar *iid,
                       gpointer     data)
{
  if (g_strcmp0 (iid, "InvestApplet") != 0)
    return FALSE;

  invest_applet_setup (applet);

  return TRUE;
}

PANEL_APPLET_IN_PROCESS_FACTORY ("InvestAppletFactory", INVEST_TYPE_APPLET,
                                 invest_applet_factory, NULL)
