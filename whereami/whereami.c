/*
 * The whereami applet simply displays the position of the cursor.  By
 * moving the cursor with button 1 held down, the size of an area on
 * the screen will be displayed.  This applet is a part of the Gnome
 * project.  See http://www.gnome.org/ for more information.
 *
 * Copyright (C) 1999 John Kodis <kodis@jagunet.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */

#include <config.h>
#include <applet-widget.h>

static char *idle_msg = N_("Where\nAm I?");

static enum { idle, position, size } state;
 
static void
about(AppletWidget *applet, gpointer data)
{
  GtkWidget *about_box;
  static const char *authors[] = { "John Kodis <kodis@jagunet.com>", NULL };

  about_box = gnome_about_new(
    _("Where Am I?"), VERSION,
    _("Copyright 1999 John Kodis"), authors,
    _("A cursor position reporting applet."), NULL);

  gtk_widget_show(about_box);
}

static void
motion_handler(GtkWidget *widget, GdkEventMotion *motion, gpointer data)
{
  static int x, y;
  char where[100];
 
  if (state == position)
    sprintf(where, "%+d\n%+d", x = motion->x_root, y = motion->y_root);
  else
    sprintf(where, "%+d\n%+d", (int)motion->x_root-x, (int)motion->y_root-y);
  
  gtk_label_set_text(GTK_LABEL(GTK_BIN(widget)->child), where);
}

static void
button_handler(GtkWidget *widget, GdkEventButton *button, gpointer data)
{
  if (state == idle && button->type == GDK_BUTTON_RELEASE)
    {
      GdkEventMask events =  GDK_POINTER_MOTION_MASK |
	GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;
      GdkCursor *cursor = gdk_cursor_new(GDK_CROSSHAIR);
      gdk_pointer_grab(
	widget->window, TRUE /* owner_events */,
	events, NULL /* confine_to */,
	cursor, GDK_CURRENT_TIME);
      gdk_cursor_destroy(cursor);
      /* FIX THIS: we should report the real cursor position here. */
      gtk_label_set_text(GTK_LABEL(GTK_BIN(widget)->child), "+0\n+0");
      state = position;
    }

  if (state == position && button->type == GDK_BUTTON_PRESS)
    state = size;

  if (state == size && button->type == GDK_BUTTON_RELEASE)
    {
      gdk_pointer_ungrab(GDK_CURRENT_TIME);
      gtk_label_set_text(GTK_LABEL(GTK_BIN(widget)->child), idle_msg);
      state = idle;
    }
}

int
main(int argc, char **argv)
{
  GtkWidget *msg, *applet;
  char *applet_name = _("whereami_applet");

  bindtextdomain(PACKAGE, GNOMELOCALEDIR);
  textdomain(PACKAGE);

  applet_widget_init(applet_name, VERSION, argc, argv, NULL, 0, NULL);

  msg = gtk_button_new_with_label(idle_msg);
  gtk_signal_connect(GTK_OBJECT(msg),
    "motion_notify_event", motion_handler, NULL);
  gtk_signal_connect(GTK_OBJECT(msg),
    "button_press_event", button_handler, NULL);
  gtk_signal_connect(GTK_OBJECT(msg),
    "button_release_event", button_handler, NULL);
  gtk_widget_show(msg);

  applet = applet_widget_new(applet_name);
  applet_widget_add(APPLET_WIDGET(applet), msg);
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
    "about", GNOME_STOCK_MENU_ABOUT, _("About..."), about, NULL);
  gtk_widget_show(applet);

  applet_widget_gtk_main();
  return 0;
}
