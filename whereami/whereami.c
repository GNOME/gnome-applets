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

static enum { IDLE, POSITION, SIZE } state;

static int panel_size = 48;
static gboolean panel_vertical = FALSE;

static GtkWidget *msg;
 
static void
about(AppletWidget *applet, gpointer data)
{
  static GtkWidget *about_box = NULL;
  static const char *authors[] = { "John Kodis <kodis@jagunet.com>", NULL };

  if (about_box != NULL)
  {
  	gdk_window_show(about_box->window);
	gdk_window_raise(about_box->window);
	return;
  }
  about_box = gnome_about_new(
    _("Where Am I?"), VERSION,
    _("Copyright 1999 John Kodis"), authors,
    _("A cursor position reporting applet.\n"
      "= Clicking mouse button 1 grabs the cursor.\n"
      "= Dragging with mouse button 1 held down "
      "shows the size of the region."),
    NULL);
  gtk_signal_connect( GTK_OBJECT(about_box), "destroy",
		      GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_box );

  gtk_widget_show(about_box);
  return;
  applet = NULL;
  data = NULL;
}

static void
motion_handler(GtkWidget *widget, GdkEventMotion *motion, gpointer data)
{
  static int x, y;
  char where[100];

  switch (state)
    {
    case IDLE:
      gdk_window_get_pointer(NULL, &x, &y, NULL);
      if(panel_size < 48) {
	      if(panel_vertical)
		      g_snprintf(where, sizeof(where), "%d\n%d", x, y);
	      else
		      g_snprintf(where, sizeof(where), "x:%d y:%d", x, y);
      } else
	      g_snprintf(where, sizeof(where), "x:%d\ny:%d", x, y);
      break;
    case POSITION:
      x = motion->x_root;
      y = motion->y_root;
      if(panel_size < 48) {
	      if(panel_vertical)
		      g_snprintf(where, sizeof(where), "%d\n%d", x, y);
	      else
		      g_snprintf(where, sizeof(where), "x:%d y:%d", x, y);
      } else
	      g_snprintf(where, sizeof(where), "x:%d\ny:%d", x, y);
      break;
    case SIZE:
      if(panel_size < 48) {
	      if(panel_vertical)
		      g_snprintf(where, sizeof(where), "%+d\n%+d",
			      (int)motion->x_root-x, (int)motion->y_root-y);
	      else
		      g_snprintf(where, sizeof(where), "x:%+d y:%+d",
			      (int)motion->x_root-x, (int)motion->y_root-y);
      } else
	      g_snprintf(where, sizeof(where), "x:%+d\ny:%+d",
		      (int)motion->x_root-x, (int)motion->y_root-y);
      break;
    }
  
  gtk_label_set_text(GTK_LABEL(GTK_BIN(widget)->child), where);
  return;
  data = NULL;
}

static void
button_handler(GtkWidget *widget, GdkEventButton *button, gpointer data)
{
  if (button->button == 1)
    {
      if (state == IDLE && button->type == GDK_BUTTON_RELEASE)
	{
	  GdkEventMask events =  GDK_POINTER_MOTION_MASK |
	    GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;
	  GdkCursor *cursor = gdk_cursor_new(GDK_CROSSHAIR);
	  gdk_pointer_grab(
	    widget->window, TRUE, /* owner_events */
	    events, NULL,         /* confine_to */
	    cursor, GDK_CURRENT_TIME);
	  gdk_cursor_destroy(cursor);
	  motion_handler(widget, NULL, NULL);
	  state = POSITION;
	}

      if (state == POSITION && button->type == GDK_BUTTON_PRESS)
	state = SIZE;

      if (state == SIZE && button->type == GDK_BUTTON_RELEASE)
	{
	  gdk_pointer_ungrab(GDK_CURRENT_TIME);
	  state = IDLE;
	}
    }
  return;
  data = NULL;
}

static int
timeout_handler(GtkWidget *button)
{
  if (state == IDLE)
    {
      static int x, last_x, y, last_y;
      gdk_window_get_pointer(NULL, &x, &y, NULL);
      if (x != last_x || y != last_y)
	{
	  char where[100];
	  if(panel_size < 48) {
		  if(panel_vertical)
			  g_snprintf(where, sizeof(where), "%d\n%d", x, y);
		  else
			  g_snprintf(where, sizeof(where), "x:%d y:%d", x, y);
	  } else
		  g_snprintf(where, sizeof(where), "x:%d\ny:%d", x, y);
	  gtk_label_set_text(GTK_LABEL(GTK_BIN(button)->child), where);
	  last_x = x;
	  last_y = y;
	}
    }
  return 1;
}

static void
change_pixel_size (GtkWidget *applet, int size)
{
	int x,y;
	GdkEventMotion m;
	gdk_window_get_pointer(NULL, &x, &y, NULL);
	m.x_root = x;
	m.y_root = y;

	panel_size = size;

	motion_handler(msg, &m, NULL);
}

static void
change_orient (GtkWidget *applet, PanelOrientType o)
{
	int x,y;
	GdkEventMotion m;
	gdk_window_get_pointer(NULL, &x, &y, NULL);
	m.x_root = x;
	m.y_root = y;

	if(o==ORIENT_UP || o==ORIENT_DOWN)
		panel_vertical = FALSE;
	else
		panel_vertical = TRUE;

	motion_handler(msg, &m, NULL);
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
    GnomeHelpMenuEntry help_entry = { "whereami_applet", "index.html"};
    gnome_help_display(NULL, &help_entry);
}

int
main(int argc, char **argv)
{
  GtkWidget *applet;
  char *applet_name = _("whereami_applet");

  bindtextdomain(PACKAGE, GNOMELOCALEDIR);
  textdomain(PACKAGE);
  applet_widget_init(applet_name, VERSION, argc, argv, NULL, 0, NULL);

  applet = applet_widget_new(applet_name);
  
  /* Get an idea of the panel size we're working with. */
  panel_size = applet_widget_get_panel_pixel_size(APPLET_WIDGET(applet));
  
  msg = gtk_button_new_with_label("\n");
  gtk_signal_connect(GTK_OBJECT(msg),
    "motion_notify_event", motion_handler, NULL);
  gtk_signal_connect(GTK_OBJECT(msg),
    "button_press_event", button_handler, NULL);
  gtk_signal_connect(GTK_OBJECT(msg),
    "button_release_event", button_handler, NULL);
  gtk_label_set_text(GTK_LABEL(GTK_BIN(msg)->child), "");
  gtk_widget_show(msg);

  gtk_signal_connect(GTK_OBJECT(applet), "change_pixel_size",
		     GTK_SIGNAL_FUNC(change_pixel_size),
		     NULL);
  gtk_signal_connect(GTK_OBJECT(applet), "change_orient",
		     GTK_SIGNAL_FUNC(change_orient),
		     NULL);
  applet_widget_add(APPLET_WIDGET(applet), msg);
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"help",
					GNOME_STOCK_PIXMAP_HELP,
					_("Help"), help_cb, NULL);
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
    "about", GNOME_STOCK_MENU_ABOUT, _("About..."), about, NULL);

  gtk_timeout_add(100, (GtkFunction)timeout_handler, msg);
  gtk_widget_show(applet);

  applet_widget_gtk_main();
  return EXIT_SUCCESS;
}
