/*--------------------------------*-C-*---------------------------------*
 *
 *  Copyright 1999, Nat Friedman <nat@nat.org>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 *
 *----------------------------------------------------------------------*/

/*
 * File: applets/battery/battery.c
 *
 * A GNOME panel applet to display the status of a laptop battery.
 *
 * Author: Nat Friedman <nat@nat.org>
 *
 */

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <config.h>
#include <gnome.h>
#include <gdk/gdkx.h>

#include <applet-widget.h>

#include "battery.h"
#include "session.h"
#include "properties.h"
#include "read-battery.h"

#include "bolt.xpm"

int
main (int argc, char ** argv)
{
  const gchar *goad_id;
  GtkWidget *applet;

  /* Initialize i18n */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  applet_widget_init ("battery_applet", VERSION, argc, argv, NULL, 0, NULL);
  applet_factory_new ("battery_applet", NULL,
		     (AppletFactoryActivator) applet_start_new_applet);

  goad_id = goad_server_activation_id ();
  if (! goad_id)
    return 1;

  /* Create the battery applet widget */
  applet = make_new_battery_applet (goad_id);

  /* Run... */
  applet_widget_gtk_main ();

  return 0;
} /* main */

/*
 * This function, battery_update, grabs new data about the battery
 * charge and updates all of the display mode pixmaps and labels.
 * There are two modes, and the display elements are updated for both:
 *
 *     Charge Graph       This is a graph of the percentage charge of the
 *                        battery.
 * 
 *     Digital Readout    This is a simple text display of the percentage
 *                        charge of the battery and the time remaining
 *                        until battery death, plus a pretty battery
 *                        shape filled in according to the battery's
 *                        level of charge.
 *
 */
gint
battery_update (gpointer data)
{
  BatteryData * bat = data;

  /* Data we read about the battery charge */
  char ac_online, percentage, hours_remaining,  minutes_remaining;

  /*
   * Static copies of the last values of the battery data, so that we
   * only update the display if the data have changed.
   */
  static char last_percentage = -1, last_hours_remaining = -1,
    last_minutes_remaining = -1, last_ac_online = -1;

  int graph_height, graph_width, i;

  time_t curr_time;

  if (!bat->setup)
    return 0;

  /*
   * The battery change information that we grab here will be used by
   * both readout mode and graph mode.
   */
  if (! battery_read_charge (&percentage, &ac_online, &hours_remaining,
			     &minutes_remaining))
    {
      gnome_error_dialog
	(_("Error querying battery charge.  "
	   "Make sure that your kernel was "
	   "built with APM support."));

      /*
       * FIXME: What's the proper way to exit an applet?  If it's an
       * in-process server, then exit() will kill the panel too!  So
       * there should be some way for an applet to request that it be
       * removed.
       */
      exit (1);
    }

  /*
   * Warn if the battery is low.
   */
  if (bat->low_warn_enable && (percentage <= bat->low_warn_val)
      && (! bat->warned))
    {
      gnome_warning_dialog (_("The battery is low."));
      bat->warned = TRUE;
    }
  else if ((percentage > bat->low_warn_val) && bat->warned)
    bat->warned = FALSE;

  /*
   * If the battery has charged up fully, let the user know.
   * Personally, I find this feature annoying.  But a user requested
   * it, and the user is always right. :-)
   */
  if (bat->full_notify_enable && (percentage == 100)
      && (last_percentage < 100) && (last_percentage > 0)
      && ac_online && (! bat->notified))
    {
      gnome_ok_dialog (_("The battery is fully charged."));
      bat->notified = TRUE;
    }
  else if ((percentage < 100) && bat->notified)
    bat->notified = FALSE;

  /*
   *
   * Graph Mode 
   *
   */

  /* First check that it is time to update the graph. */
  time (&curr_time);

  if (bat->graph_direction == BATTERY_GRAPH_RIGHT_TO_LEFT)
    {
      /* Shift the graph samples down */
      for (i = 0 ; i < (bat->width - 1) ; i++)
	bat->graph_values[i] = bat->graph_values[i + 1];

	  /* Add in the new value */
      bat->graph_values[bat->width - 1] = percentage;
    }
  else
    {
      /* Shift the graph samples up */
      for (i = bat->width - 1; i > 0; i--)
	bat->graph_values[i] = bat->graph_values[i - 1];

	  /* Add in the new value */
      bat->graph_values[0] = percentage;
    }

  /*
   * Figure out the size of the graph area.  Ideally we could just
   * use the graph_area->allocation data, but those values aren't
   * set correcty until AFTER the graph area widget is shown.  We
   * can't just use the graph->width and graph->height, since the
   * bevelling of the frame around the pixmap removes a couple of
   * pixels.
   *
   * So it would be nice if there were some way to find out the
   * allocated size without actually showing the graph area
   * widget.  I can't figure out how to do that, however.  So I
   * set the allocation width and height to zero in the battery
   * initialization, and if they're zero here, I use the
   * bat->width and bat->height, which are the requisitioned
   * applet sizes.  These should only be slightly off, so it's
   * a minor visual quirk, and only lasts until the next update.
   *
   * And if the allocated height/width is greater than bat->width
   * and bat->height, we should use the lesser of the values, for
   * when the user resizes the applet.
   */
  graph_height = bat->graph_area->allocation.height;
  graph_width = bat->graph_area->allocation.width;
  if ((graph_height == 0) || (graph_height > bat->height))
    graph_height = bat->height;
  if ((graph_width == 0) || (graph_width > bat->width))
    graph_width = bat->width;

  /* Clear the graph pixmap */
  gdk_draw_rectangle (bat->graph_pixmap,
		      bat->graph_area->style->black_gc,
		      TRUE, 0, 0, bat->width, bat->height);

  /* Draw the graph */
  if (ac_online)
    gdk_gc_set_foreground (bat->gc, & (bat->graph_color_ac_on));
  else
    gdk_gc_set_foreground (bat->gc, & (bat->graph_color_ac_off));

  if (percentage <= bat->low_charge_val)
    gdk_gc_set_foreground (bat->gc, & (bat->graph_color_low));

  for (i = 0 ; i < graph_width ; i++)
    {
      gdk_draw_line (bat->graph_pixmap, bat->gc, i,
		     (100 - bat->graph_values[i]) * graph_height / 100,
		     i, graph_height);
    }

  /*
   * Draw the graph ticks on top of the graph.
   */
  gdk_gc_set_foreground (bat->gc, & (bat->graph_color_line));

  gdk_draw_line (bat->graph_pixmap, bat->gc,
		 0, (75 * graph_height / 100),
		 bat->width, (75 * graph_height / 100));

  gdk_draw_line (bat->graph_pixmap, bat->gc,
		 0, (50 * graph_height / 100),
		 bat->width, (50 * graph_height / 100));

  gdk_draw_line (bat->graph_pixmap, bat->gc,
		 0, (25 * graph_height / 100),
		 bat->width, (25 * graph_height / 100));
      
  /*
   *
   * Readout Mode
   *
   */

  /*
   * Only update the little picture of the battery if the battery
   * charge percentage has changed since the last time
   * battery_update() ran.
   */
  if ( (last_percentage != percentage) || (last_ac_online != ac_online)
       || bat->force_update)
    {
      int height, y, body_perc, i;
      int nipple_width, nipple_height;

      /* Clear the readout pixmap to grey. */
      gdk_draw_rectangle (bat->readout_pixmap,
			  bat->readout_area->style->bg_gc[GTK_STATE_NORMAL],
			  TRUE,
			  0, 0,
			  bat->readout_area->allocation.width,
			  bat->readout_area->allocation.height);

      /* Draw the outline of the battery picture. */
      gdk_draw_lines (bat->readout_pixmap,
		      bat->readout_area->style->black_gc,
		      bat->readout_batt_points, 9);

      /*
       * Determine the height of the body (everything but the nipple)
       * of the battery picture.
       */
      height = bat->readout_batt_points[7].y - bat->readout_batt_points[0].y;

      /*
       * Fill the battery in white.
       */
      gdk_draw_rectangle (bat->readout_pixmap,
			  bat->readout_area->style->white_gc,
			  TRUE,
			  bat->readout_batt_points[0].x + 1,
			  bat->readout_batt_points[0].y + 1,
			  bat->readout_batt_points[5].x -
			  bat->readout_batt_points[0].x - 1,
			  height - 1);

      /*
       * Now fill in the main battery chamber.  If the battery charge
       * is at 100%, fill in the little nipple (positive terminal) at
       * the top.  99% is everything but the nipple.
       */
      if (percentage < 100)
	body_perc = percentage;
      else
	body_perc = 99;

      /* The number of pixels of the main chamber that we fill in. */
      y =  bat->readout_batt_points[0].y +
	(height - ( (body_perc * height) / 100));

      if (ac_online)
	gdk_gc_set_foreground (bat->readout_gc,
			       & (bat->readout_color_ac_on));
      else
	gdk_gc_set_foreground (bat->readout_gc,
			       & (bat->readout_color_ac_off));

      if (percentage <= bat->low_charge_val)
	gdk_gc_set_foreground (bat->readout_gc,
			       & (bat->readout_color_low));
    
      gdk_draw_rectangle (bat->readout_pixmap,
			  bat->readout_gc,
			  TRUE,
			  bat->readout_batt_points[0].x + 1,
			  y,
			  bat->readout_batt_points[5].x -
			  bat->readout_batt_points[0].x - 1,
			  (height * body_perc) / 100);

      /* Fill in the nipple if appropriate. */
      nipple_width = bat->readout_batt_points[3].x -
	bat->readout_batt_points[2].x - 1;
      nipple_height = bat->readout_batt_points[1].y -
	bat->readout_batt_points[2].y;

      gdk_draw_rectangle (bat->readout_pixmap,
			  (percentage == 100) ? bat->readout_gc :
			  bat->readout_area->style->white_gc,
			  TRUE,
			  bat->readout_batt_points[2].x + 1,
			  bat->readout_batt_points[2].y + 1,
			  nipple_width, nipple_height);

      /*
       * Now blit the lightning-bolt image onto the battery if the AC
       * is plugged in.
       */
      if (ac_online)
	{
	  GdkGC *gc;
	  int dest_x, dest_y;

	  dest_y =
	    ((bat->readout_batt_points[0].y +
	      bat->readout_batt_points[7].y) / 2) -
	    (BOLT_HEIGHT / 2);

	  dest_x = 1 + 
	    ((bat->readout_batt_points[0].x +
	      bat->readout_batt_points[5].x) / 2) -
	    (BOLT_WIDTH / 2);

	  gc = gdk_gc_new (bat->readout_area->window);

	  /*
	   * The "clip mask" is a bitmap which specifies which regions
	   * of the drawing should be transparent.  Those are denoted
	   * as "c None" in the .xpm file.  As it turns out, the
	   * "transparent_color" argument to
	   * gnome_pixmap_new_from_xpm_d() specifies a color which
	   * should REPLACE all transparent pixels in the pixmap, not
	   * a color which should BECOME transparent.
	   */
	  gdk_gc_set_clip_mask (gc, bat->bolt_mask);
	  gdk_gc_set_clip_origin (gc, dest_x, dest_y);

	  gdk_draw_pixmap (bat->readout_pixmap, gc, bat->bolt_pixmap,
			   0, 0, dest_x, dest_y,
			   BOLT_WIDTH, BOLT_HEIGHT);
	}
    }

  /* Don't update the percentage label unless the percentage has changed. */
  if (last_percentage != percentage || bat->force_update)
    {
      char labelstr [256];

      /* Now update the labels in readout mode. */

      strcpy (labelstr, "    ");

      /*
       * In order to fit the text into a small applet (e.g. 24x24), we
       * have to remove the '%' from the end of the percentage label.
       */
      if (bat->width < 28 && (percentage == 100))
	snprintf (labelstr, sizeof (labelstr), "%d", percentage);
      else
	snprintf (labelstr, sizeof (labelstr), "%d%%", percentage);

      /* Make sure it's 4 spaces long. */
      if (labelstr [3] == '\0')
	labelstr[3] = ' ';

      gtk_label_set_text (GTK_LABEL (bat->readout_label_percentage), labelstr);
      gtk_label_set_text (GTK_LABEL (bat->readout_label_percentage_small),
			  labelstr);
    }

  if (last_minutes_remaining != minutes_remaining ||
      last_hours_remaining != hours_remaining)
    {
      char labelstr [256];

      /*
       * If we cannot update the time-remaining label, then make it blank.
       */
      if (minutes_remaining == -1 || hours_remaining == -1 ||
	  (minutes_remaining == 0 && hours_remaining == 0))
	{
	  gtk_label_set_text (GTK_LABEL (bat->readout_label_time), "");
	}
      else
	{
	  snprintf (labelstr, sizeof (labelstr), "%d:%02d", hours_remaining,
		    minutes_remaining);
	  gtk_label_set_text (GTK_LABEL (bat->readout_label_time), labelstr);
	}
    }

  /* Update the display. */
  battery_expose_handler (bat->graph_area, NULL, bat);

  /* Save these values. */
  last_minutes_remaining = minutes_remaining;
  last_hours_remaining = hours_remaining;
  last_percentage = percentage;
  last_ac_online = ac_online;

  bat->force_update = FALSE;

  return TRUE;
} /* battery_update */


/*
 * This function, battery_expose, is called whenever a portion of the
 * applet window has been exposed and needs to be redrawn.  In this
 * function, we just blit the appropriate portion of the appropriate
 * pixmap onto the window.  The appropriate pixmap is chosen based on
 * the applet's "mode."
 *
 */
gint
battery_expose_handler (GtkWidget * widget, GdkEventExpose * expose,
			gpointer data)
{
  BatteryData * bat = data;
  GdkPixmap * curr_pixmap;

  if (!bat->setup)
    return FALSE;

  /* Select a pixmap to draw into the applet window based on our
     current mode. */
  if (!strcasecmp (bat->mode_string, BATTERY_MODE_GRAPH))
    {
      curr_pixmap = bat->graph_pixmap;
      gdk_draw_pixmap (/* Drawable */        bat->graph_area->window,
		       /* GC */
       bat->graph_area->style->fg_gc[GTK_WIDGET_STATE (bat->graph_area)],
	       /* Src Drawable */    curr_pixmap,
	       /* X src, Y src */    0, 0,
	       /* X dest, Y dest */  0, 0,
	       /* width */           bat->graph_area->allocation.width,
	       /* height */          bat->graph_area->allocation.height);
    }
  else
    {
      gdk_draw_pixmap (/* Drawable */        bat->readout_area->window,
		       /* GC */
       bat->readout_area->style->fg_gc[GTK_WIDGET_STATE (bat->readout_area)],
	       /* Src Drawable */    bat->readout_pixmap,
	       /* X src, Y src */    0, 0,
	       /* X dest, Y dest */  0, 0,
	       /* width */           bat->readout_area->allocation.width,
	       /* height */          bat->readout_area->allocation.height);
    }

  return FALSE; 
} /* battery_expose_handler */

/* This handler gets called whenever the panel changes orientations.
   When the applet is set up, we get an initial call, too. */
gint
battery_orient_handler (GtkWidget * w, PanelOrientType o, gpointer data)
{
  BatteryData * bat = data;

  /* FIXME: What do we do here? */

  return FALSE;
} /* battery_orient_handler */

gint
battery_configure_handler (GtkWidget *widget, GdkEventConfigure *event,
			   gpointer data)
{
  BatteryData * bat = data;
  
  /* We have to set up the picture here because the coordinates will
     be all wrong otherwise. */
  battery_setup_picture (bat);

  bat->force_update = TRUE;
  battery_update ( (gpointer) bat);

  return TRUE;
}  /* battery_configure_handler */

/* Whenever the mode changes, and when the applet first starts up,
   this function is called to hide the out-of-mode parts of the display
   and show the in-mode parts. */
void
battery_set_mode (BatteryData * bat)
{
  if (!strcmp (bat->mode_string, BATTERY_MODE_GRAPH))
    {
      /* If the property dialogue is open, update it. */
      if (bat->prop_win)
	gtk_toggle_button_set_active
	  (GTK_TOGGLE_BUTTON (bat->mode_radio_graph), 1);

      battery_expose_handler (bat->graph_area, NULL, bat);
      gtk_widget_hide_all (bat->readout_frame);
      gtk_widget_show_all (bat->graph_frame);
    }
  else if (!strcmp (bat->mode_string, BATTERY_MODE_READOUT))
    {
      if (bat->prop_win)
	gtk_toggle_button_set_active
	  (GTK_TOGGLE_BUTTON (bat->mode_radio_readout), 1);

      battery_expose_handler (bat->readout_area, NULL, bat);
      gtk_widget_hide_all (bat->graph_frame);
      gtk_widget_show_all (bat->readout_frame);
    }
  else
    {
      g_error (_("Internal error: invalid mode in battery_set_mode"));
    }

  /*
   * If the applet is small enough, we have to hide a few of the
   * widgets to make it fit.
   */
  if ((bat->height < 28) || (bat->width < 28))
    {
      gtk_widget_hide (bat->readout_label_percentage);
      gtk_widget_show (bat->readout_label_percentage_small);
      gtk_widget_hide (bat->readout_area);
    }
  else
    {
      gtk_widget_show (bat->readout_label_percentage);
      gtk_widget_hide (bat->readout_label_percentage_small);
      gtk_widget_show (bat->readout_area);
    }

  if (bat->width < 48)
    gtk_widget_hide (bat->readout_label_time);
  else
    gtk_widget_show (bat->readout_label_time);

  /*
   * When we change modes, make sure there's not a fraction of a second
   * where our pixmap is displayed as black ...
   */
  battery_expose_handler (bat->graph_area, NULL, bat);

} /* battery_set_mode */

static void
battery_change_mode (BatteryData * bat)
{
  if (!strcmp (bat->mode_string, BATTERY_MODE_GRAPH))
    {
      g_free (bat->mode_string);
      bat->mode_string = strdup (BATTERY_MODE_READOUT);
    }
  else
    {
      g_free (bat->mode_string);
      bat->mode_string = strdup (BATTERY_MODE_GRAPH);
    }

  battery_set_mode (bat);
} /* battery_change_mode */

static gint
battery_button_press_handler (GtkWidget * w, GdkEventButton * ev,
			      gpointer data)
{
  BatteryData * bat = data;

  battery_change_mode (bat);

  return TRUE;
} /* battery_button_press_handler */

GtkWidget *
applet_start_new_applet (const gchar *goad_id, const char **params,
			 int nparams)
{
  return make_new_battery_applet (goad_id);
} /* applet_start_new_applet */

/* This is the function that actually creates the display widgets */
GtkWidget *
make_new_battery_applet (const gchar *goad_id)
{
  BatteryData * bat;
  GtkWidget *root, *graph_box, *readout_box, *readout_text_table;
  GtkWidget *readout_battery_frame;
  GtkStyle *label_style;
  gchar * param = "battery_applet";

  bat = g_new0 (BatteryData, 1);

  bat->applet = applet_widget_new (goad_id);

  if (bat->applet == NULL)
    g_error (_("Can't create applet!\n"));

  bat->graph_values = NULL;
  bat->setup = 0;
  bat->force_update = TRUE;

  /*
   * Load all the saved session parameters (or the defaults if none
   * exist).
   */
  if ( (APPLET_WIDGET (bat->applet)->privcfgpath) &&
       * (APPLET_WIDGET (bat->applet)->privcfgpath))
    battery_session_load (APPLET_WIDGET (bat->applet)->privcfgpath, bat);
  else
    battery_session_defaults (bat);

  /* All widgets will go into this container. */
  root = gtk_hbox_new (FALSE, 0);

  /*
   * The 'graph frame' is the root widget of the entire graph mode.
   * It provides the pretty bevelling and serves as an easy way of
   * hiding/showing the entire graph mode.
   */
  bat->graph_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (bat->graph_frame), GTK_SHADOW_IN);

  /*
   * The 'readout frame' is the root widget of the entire readout
   * mode.
   */
  bat->readout_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (bat->readout_frame), GTK_SHADOW_IN);
     
  gtk_box_pack_start_defaults (GTK_BOX (root), bat->graph_frame);
  gtk_box_pack_start_defaults (GTK_BOX (root), bat->readout_frame);

  /*
   * This is the horizontal box which will contain the picture of
   * the battery and the text labels.
   */
  readout_box = gtk_hbox_new (FALSE, 0);

  /* The three labels go into this table. */
  readout_text_table = gtk_table_new (3, 0, TRUE);
  
  bat->readout_label_percentage = gtk_label_new ("");  
  bat->readout_label_percentage_small = gtk_label_new ("");
  bat->readout_label_time = gtk_label_new ("");

  label_style = gtk_style_copy (GTK_WIDGET (bat->readout_label_time)->style);
  label_style->font = gdk_font_load ("6x10");
  GTK_WIDGET (bat->readout_label_time)->style = label_style;
  GTK_WIDGET (bat->readout_label_percentage)->style = label_style;

  label_style = gtk_style_copy (GTK_WIDGET (bat->readout_label_time)->style);
  label_style->font = gdk_font_load ("5x8");
  GTK_WIDGET (bat->readout_label_percentage_small)->style = label_style;

  gtk_table_attach (GTK_TABLE (readout_text_table),
		    bat->readout_label_percentage_small,
		    0, 1, 0, 1, 0, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (readout_text_table),
		    bat->readout_label_percentage,
		    0, 1, 1, 2, 0, 0, 0, 0);
  gtk_table_attach (GTK_TABLE (readout_text_table), bat->readout_label_time,
		    0, 1, 2, 3, 0, 0, 0, 0);

  /* This is the frame which contains the picture of the battery. */
  readout_battery_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (readout_battery_frame),
			     GTK_SHADOW_NONE);

  /*
   * readout_area is the drawing area into which the little picture of
   * the battery gets drawn.
   */
  bat->readout_area = gtk_drawing_area_new ();
  gtk_container_add (GTK_CONTAINER (readout_battery_frame),
		     bat->readout_area);

  gtk_box_pack_start_defaults (GTK_BOX (readout_box), readout_battery_frame);
  gtk_box_pack_start_defaults (GTK_BOX (readout_box), readout_text_table);

  graph_box = gtk_vbox_new (FALSE, 0);

  /*
   * graph_area is the drawing area into which the graph of the battery
   * life gets drawn.
   */
  bat->graph_area = gtk_drawing_area_new ();

  gtk_box_pack_start_defaults (GTK_BOX (graph_box), bat->graph_area);
  gtk_container_add (GTK_CONTAINER (bat->graph_frame), graph_box);
  gtk_container_add (GTK_CONTAINER (bat->readout_frame), readout_box);

  /* Set up the mode-changing callback */
  gtk_signal_connect (GTK_OBJECT (bat->applet), "button_press_event",
		      (GtkSignalFunc) battery_button_press_handler, bat);
  gtk_widget_set_events (GTK_WIDGET (bat->applet), GDK_BUTTON_PRESS_MASK);


  /* Set up the event callbacks for the graph of battery life. */
  gtk_signal_connect (GTK_OBJECT (bat->graph_area), "expose_event",
		      (GtkSignalFunc)battery_expose_handler, bat);
  gtk_widget_set_events (bat->graph_area, GDK_EXPOSURE_MASK |
			 GDK_BUTTON_PRESS_MASK);

  /* Set up the event callbacks for the readout */
  gtk_signal_connect (GTK_OBJECT (bat->readout_area), "expose_event",
		      (GtkSignalFunc)battery_expose_handler, bat);
  gtk_signal_connect (GTK_OBJECT (bat->readout_area), "configure_event",
		      (GtkSignalFunc)battery_configure_handler, bat);
  gtk_widget_set_events (bat->readout_area, GDK_EXPOSURE_MASK | GDK_CONFIGURE);

  /* This will let us know when the panel changes orientation */
  gtk_signal_connect (GTK_OBJECT (bat->applet), "change_orient",
		      GTK_SIGNAL_FUNC (battery_orient_handler),
		      bat);

  applet_widget_add (APPLET_WIDGET (bat->applet), root);

  gtk_signal_connect (GTK_OBJECT (bat->applet), "save_session",
		      GTK_SIGNAL_FUNC (battery_session_save),
		      bat);

  applet_widget_register_stock_callback (APPLET_WIDGET (bat->applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."),
					 about_cb,
					 bat);

  applet_widget_register_stock_callback (APPLET_WIDGET (bat->applet),
					 "properties",
					 GNOME_STOCK_MENU_PROP,
					 ("Properties..."),
					 battery_properties_window,
					 bat);

  gtk_widget_show_all (bat->applet);


  /* Allocate the colors... */
  battery_create_gc (bat);
  battery_setup_colors (bat);

  /*
   * Load the pixmap of the lightning bolt.  This is defined in
   * bolt.xpm.
   */
  bat->bolt_pixmap = gdk_pixmap_create_from_xpm_d (bat->readout_area->window,
		   & bat->bolt_mask, NULL, bolt_xpm);
						   

  /* Nothing is drawn until this is set. */
  bat->setup = TRUE;

  /*
   * These will get reset to the correct values when the graph area is
   * mapped.  See the comment in battery_update().
   */
  bat->graph_area->allocation.height = 0;
  bat->graph_area->allocation.width = 0;
  
  /* Size things according to the saved settings. */
  battery_set_size (bat);

  /* Display only one mode. */
  battery_set_mode (bat);

  bat->update_timeout_id = gtk_timeout_add (1000 * bat->update_interval,
					    (GtkFunction) battery_update, bat);

  bat->force_update = TRUE;
  battery_update (bat);

  return bat->applet;
} /* make_new_battery_applet */

void
destroy_about (GtkWidget *w, gpointer data)
{
  BatteryData *bat = data;
} /* destroy_about */

void
about_cb (AppletWidget *widget, gpointer data)
{
  BatteryData *bat = data;
  char *authors[2];
  
  authors[0] = "Nat Friedman <nat@nat.org>";
  authors[1] = NULL;

  bat->about_box =
    gnome_about_new (_("The GNOME Battery Monitor Applet"), VERSION,
		     _(" (C) 1997-1998 The Free Software Foundation"),
		     (const char **) authors,
	     _("This applet monitors the charge of your laptop's battery.  "
		       "Click on it to change display modes."),
		     NULL);

  gtk_signal_connect (GTK_OBJECT (bat->about_box), "destroy",
		      GTK_SIGNAL_FUNC (destroy_about), bat);

  gtk_widget_show (bat->about_box);
} /* about_cb */

void
battery_setup_picture (BatteryData * bat)
{
  gint readout_width, readout_height;

  readout_width = bat->readout_area->allocation.width;
  readout_height = bat->readout_area->allocation.height;

  /*  Set up the line segments for the battery picture.  The points are
   *  numbered as follows:
   *       2_ 3
   *   0 __| |__ 5
   *   8 | 1 4 |
   *     |     |
   *     |     |
   *     |     |
   *     |_____|
   *    7       6
   */

  /* 0 */
  bat->readout_batt_points[0].x = readout_width * 0.10;
  bat->readout_batt_points[0].y = readout_height / 6;

  /* 1 */
  bat->readout_batt_points[1].x = readout_width * 0.35;
  bat->readout_batt_points[1].y = readout_height / 6; 

  /* 2 */
  bat->readout_batt_points[2].x = readout_width * 0.35;
  bat->readout_batt_points[2].y = readout_height / 8;

  /* 3 */
  bat->readout_batt_points[3].x = readout_width * 0.65;
  bat->readout_batt_points[3].y = readout_height / 8;

  /* 4 */
  bat->readout_batt_points[4].x = readout_width * 0.65;
  bat->readout_batt_points[4].y = readout_height / 6;

  /* 5 */
  bat->readout_batt_points[5].x = readout_width * 0.90;
  bat->readout_batt_points[5].y = readout_height / 6;

  /* 6 */
  bat->readout_batt_points[6].x = readout_width * 0.90;
  bat->readout_batt_points[6].y = readout_height * 0.83;

  /* 7 */
  bat->readout_batt_points[7].x = readout_width * 0.10;
  bat->readout_batt_points[7].y = readout_height * 0.83;

  /* 8 */
  bat->readout_batt_points[8].x = readout_width * 0.10;
  bat->readout_batt_points[8].y = readout_height / 6;

} /* battery_setup_picture */

void
battery_set_size (BatteryData * bat)
{
  gtk_widget_set_usize (bat->readout_area, bat->width * 0.35, bat->height);
  gtk_widget_set_usize (bat->graph_frame, bat->width, bat->height);
  gtk_widget_set_usize (bat->readout_frame, bat->width, bat->height);

  /*
   * If the applet is small enough, we have to hide a few of the
   * display widgets so that things fit in a sane manner.  This is
   * important for PDA's, which won't have a lot of screen geometry.
   */
  if (bat->height < 28 || bat->width < 28)
    {
      gtk_widget_hide (bat->readout_label_percentage);
      gtk_widget_show (bat->readout_label_percentage_small);
      gtk_widget_hide (bat->readout_area);
    }
  else
    {
      gtk_widget_show (bat->readout_label_percentage);
      gtk_widget_hide (bat->readout_label_percentage_small);
      gtk_widget_show (bat->readout_area);
    }

  if (bat->width < 48)
    gtk_widget_hide (bat->readout_label_time);
  else
    gtk_widget_show (bat->readout_label_time);

  /*
   * If pixmaps have already been allocated, then free them here
   * before creating new ones.  */
  if (bat->graph_pixmap != NULL)
    gdk_pixmap_unref (bat->graph_pixmap);
  if (bat->readout_pixmap != NULL)
    gdk_pixmap_unref (bat->readout_pixmap);

  bat->graph_pixmap = gdk_pixmap_new (bat->graph_area->window,
				      bat->width,
				      bat->height,
		      gtk_widget_get_visual (bat->graph_area)->depth);

  gdk_draw_rectangle (bat->graph_pixmap,
		      bat->graph_area->style->black_gc,
		      TRUE, 0, 0,
		      bat->graph_area->allocation.width,
		      bat->graph_area->allocation.height);

  bat->readout_pixmap = gdk_pixmap_new (bat->readout_area->window,
					bat->width,
					bat->height,
			gtk_widget_get_visual (bat->readout_area)->depth);

  gdk_draw_rectangle (bat->readout_pixmap,
		      bat->readout_area->style->black_gc,
		      TRUE, 0, 0,
		      bat->readout_area->allocation.width,
		      bat->readout_area->allocation.height);

  /* If we've been resized, don't throw away the old graph data */
  if (bat->graph_values != NULL)
    { 
      unsigned char *new_vals;

      new_vals = (unsigned char *) g_malloc (bat->width);
      if (new_vals == NULL)
	g_error (_("Could not allocate space for graph values"));

      memset (new_vals, 0, bat->width);

      if (bat->width > bat->old_width)
	memcpy (new_vals + (bat->width - bat->old_width),
		bat->graph_values, bat->old_width);
      else
	memcpy (new_vals, bat->graph_values, bat->width);

      g_free (bat->graph_values);

      bat->graph_values = new_vals;
    }
  else
    {
      bat->graph_values = (unsigned char *) g_malloc (bat->width);
      memset (bat->graph_values, 0, bat->width); 
    }

  bat->old_width = bat->width;

  bat->force_update = TRUE;

  battery_update ( (gpointer) bat);

} /* battery_set_size */

void
battery_create_gc (BatteryData * bat)
{
  bat->gc = gdk_gc_new (bat->graph_area->window);
  gdk_gc_copy (bat->gc, bat->graph_area->style->white_gc);

  bat->readout_gc = gdk_gc_new (bat->readout_area->window);
  gdk_gc_copy (bat->readout_gc, bat->readout_area->style->white_gc);

  gdk_gc_set_function (bat->gc, GDK_COPY);
} /* battery_create_gc */

void 
battery_setup_colors (BatteryData * bat)
{
  GdkColormap *colormap;

  /*
   * FIXME: We should use gdk_color_change if we've already set up the
   * colors.
   */

  colormap = gtk_widget_get_colormap (bat->graph_area);

  /* Readout */
  gdk_color_parse (bat->readout_color_ac_on_s,
		   & (bat->readout_color_ac_on));
  gdk_color_alloc (colormap, & (bat->readout_color_ac_on));

  gdk_color_parse (bat->readout_color_ac_off_s,
		   & (bat->readout_color_ac_off));
  gdk_color_alloc (colormap, & (bat->readout_color_ac_off));

  gdk_color_parse (bat->readout_color_low_s,
		   & (bat->readout_color_low));
  gdk_color_alloc (colormap, & (bat->readout_color_low));


  /* Graph */
  gdk_color_parse (bat->graph_color_ac_on_s,
		   & (bat->graph_color_ac_on));
  gdk_color_alloc (colormap, & (bat->graph_color_ac_on));

  gdk_color_parse (bat->graph_color_ac_off_s,
		   & (bat->graph_color_ac_off));
  gdk_color_alloc (colormap, & (bat->graph_color_ac_off));

  gdk_color_parse (bat->graph_color_low_s,
		   & (bat->graph_color_low));
  gdk_color_alloc (colormap, & (bat->graph_color_low));

  gdk_color_parse (bat->graph_color_line_s,
		   & (bat->graph_color_line));
  gdk_color_alloc (colormap, & (bat->graph_color_line));
} /* battery_setup_colors */
