/*
 * The whereami applet simply displays the position of the cursor.  By
 * moving the cursor with button 1 held down, the size of an area on
 * the screen will be displayed.  This applet is a part of the Gnome
 * project.  See htt p://www.gnome.org/ for more information.
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
#include <gnome.h>
#include <panel-applet.h>

static enum { IDLE, POSITION, SIZE } state;

static int panel_size = 48;
static gboolean panel_vertical = FALSE;

static GtkWidget *msg;
static GtkWidget *label;
 
static void
about(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
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
    _("Copyright 1999 John Kodis"), 
    _("A cursor position reporting applet.\n"
      "= Clicking mouse button 1 grabs the cursor.\n"
      "= Dragging with mouse button 1 held down "
      "shows the size of the region."),
   authors,
   NULL,
   NULL,
   NULL);
 
  gtk_widget_show(about_box);
  return;
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
  
  gtk_label_set_text(GTK_LABEL(label), where);
  return;
  data = NULL;
}

static void
button_handler(GtkWidget *widget, GdkEventButton *button, gpointer data)
{
  /* FIXME: this seems to not get called when the pointer is grabbed
  ** and the pointer is outside the button */
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
	  gtk_label_set_text(GTK_LABEL(label), where);
	  last_x = x;
	  last_y = y;
	}
    }
  return 1;
}

static void
change_pixel_size (PanelApplet *applet, gint size, gpointer data)
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
change_orient (PanelApplet *applet, PanelAppletOrient o, gpointer data)
{
	int x,y;
	GdkEventMotion m;
	gdk_window_get_pointer(NULL, &x, &y, NULL);
	m.x_root = x;
	m.y_root = y;

	if(o==PANEL_APPLET_ORIENT_UP || o==PANEL_APPLET_ORIENT_DOWN)
		panel_vertical = FALSE;
	else
		panel_vertical = TRUE;

	motion_handler(msg, &m, NULL);
}

static void
help_cb (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
#ifdef FIXME
    GnomeHelpMenuEntry help_entry = { "whereami_applet", "index.html"};
    gnome_help_display(NULL, &help_entry);
#endif
}

static const BonoboUIVerb whereami_applet_menu_verbs [] = {
        BONOBO_UI_VERB ("Help", help_cb),
        BONOBO_UI_VERB ("About", about),
        BONOBO_UI_VERB_END
};

static const char whereami_applet_menu_xml [] =
	"<popup name=\"button3\">\n"
	"   <menuitem name=\"Item 1\" verb=\"Help\" _label=\"Help\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
	"   <menuitem name=\"Item 2\" verb=\"About\" _label=\"About\"\n"
	"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
	"</popup>\n";

static gboolean
whereami_applet_fill (PanelApplet *applet)
{
  label = gtk_label_new("\n");
  msg = gtk_button_new();
  gtk_container_add (GTK_CONTAINER (msg), label);
  g_signal_connect(G_OBJECT(msg),
    "motion_notify_event", G_CALLBACK (motion_handler), NULL);
  g_signal_connect(G_OBJECT(msg),
    "button_press_event", G_CALLBACK (button_handler), NULL);
  g_signal_connect(G_OBJECT(msg),
    "button_release_event", G_CALLBACK (button_handler), NULL);
  gtk_label_set_text(GTK_LABEL(label), "");
  gtk_widget_show(label);
  gtk_widget_show(msg);
  
  gtk_container_add (GTK_CONTAINER (applet), msg);

  /* Get an idea of the panel size we're working with. */
  panel_size = panel_applet_get_size (applet);
  
  panel_applet_setup_menu (applet,
			   whereami_applet_menu_xml,
			   whereami_applet_menu_verbs,
			   NULL);
			   
  g_signal_connect(G_OBJECT(applet), "change_size",
		   G_CALLBACK(change_pixel_size),
		   NULL);
  g_signal_connect(G_OBJECT(applet), "change_orient",
		   G_CALLBACK(change_orient),
		   NULL);
		   
  gtk_timeout_add(100, (GtkFunction)timeout_handler, msg);
  gtk_widget_show(GTK_WIDGET (applet));
  
  return TRUE;
  
}

static gboolean
whereami_applet_factory (PanelApplet  *applet,
			 const gchar  *iid,
			 gpointer      data)
{
	gboolean retval = FALSE;
    
	if (!strcmp (iid, "OAFIID:GNOME_WhereamiApplet"))
		retval = whereami_applet_fill (applet); 
    
	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_WhereamiApplet_Factory",
			     PANEL_TYPE_APPLET,
			     "Whereami Applet",
			     "0",
			     whereami_applet_factory,
			     NULL)
