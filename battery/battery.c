/*
 * File: applets/battery/battery.c
 *
 * A GNOME panel applet to display the status of a laptop battery.
 *
 * Author: Nat Friedman <ndf@mit.edu>
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


int
main(int argc, char ** argv)
{
  /* Initialize i18n */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  applet_widget_init("battery_applet", VERSION, argc, argv, NULL, 0, NULL);

  /* Create the battery applet widget */
  make_new_battery_applet();

  /* Run .. */
  applet_widget_gtk_main();

  return 0;
} /* main */

/* This function, battery_update, grabs new data about the battery
 * charge and updates all of the display mode pixmaps and labels.
 * There are two modes, and the display elements are updated for both:
 *
 *  1. Charge Graph       This is a graph of the percentage charge of the
 *                          battery.
 *  2. Digital Readout    This is a simple text display of the percentage
 *                          charge of the battery and the time remaining
 *                          until battery death, plus a pretty battery
 *                          shape filled in according to the battery's
 *                          level of charge.
 *
 */
gint
battery_update(gpointer data)
{
  BatteryData * bat = data;
  time_t curr_time;

  /* Data we read about the battery charge */
  char ac_online, percentage, hours_remaining,  minutes_remaining;

  /* Static copies of the last values of this data, so that we only
     update if things have changed. */
  static char last_percentage = -1, last_hours_remaining = -1,
    last_minutes_remaining = -1;
  
  char labelstr[256];
  int height, y, body_perc, i;

  if (!bat->setup)
    return 0;

  /* The battery change information that we grab here will be used by
     both readout mode and graph mode. */
  battery_read_charge(&percentage, &ac_online, &hours_remaining,
		      &minutes_remaining);

#ifdef DEBUG
  fprintf(stderr, "\tRead: %d%%, ac %s, %d hours %d minutes remaining\n",
	  percentage, ac_online ? "online" : "offline", hours_remaining,
	  minutes_remaining);
#endif

  /*
   *
   * Graph Mode 
   *
   */

  /* First check that it is time to update the graph. */
  time(&curr_time);
  if (curr_time > (bat->last_graph_update + bat->graph_interval) ||
      bat->force_update)
    {
      bat->last_graph_update = curr_time;

      if (bat->graph_direction == BATTERY_GRAPH_RIGHT_TO_LEFT)
	{
	  /* Shift the graph samples down */
	  for (i = 0 ; i < (bat->width - 1) ; i++)
	    bat->graph_values[i] = bat->graph_values[i+1];

	  /* Add in the new value */
	  bat->graph_values[bat->width - 1] = percentage;
	}
      else
	{
	  /* Shift the graph samples up */
	  for (i = bat->width - 1; i > 0; i--)
	    bat->graph_values[i] = bat->graph_values[i-1];

	  /* Add in the new value */
	  bat->graph_values[0] = percentage;
	}

      /* Clear the graph pixmap */
      gdk_draw_rectangle(bat->graph_pixmap,
			 bat->graph_area->style->black_gc,
			 TRUE, 0, 0, bat->width, bat->height);

      /* Draw the graph */
      if (ac_online)
	gdk_gc_set_foreground ( bat->gc, &(bat->graph_color_ac_on) );
      else
	gdk_gc_set_foreground ( bat->gc, &(bat->graph_color_ac_off) );


      for (i = 0 ; i < bat->width ; i++)
	gdk_draw_line(bat->graph_pixmap, bat->gc, i,
		      (100 - bat->graph_values[i]) * bat->height / 100,
		      i, bat->height);
    }

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
  if ((last_percentage != percentage) || bat->force_update)
    {
      /* Clear the readout pixmap to grey */
      gdk_draw_rectangle(bat->readout_pixmap,
			 bat->readout_area->style->bg_gc[GTK_STATE_NORMAL],
			 TRUE,
			 0, 0,
			 bat->readout_area->allocation.width,
			 bat->readout_area->allocation.height);

      /* Draw the outline of a battery */
      gdk_draw_lines(bat->readout_pixmap,
		     bat->readout_area->style->black_gc,
		     bat->readout_batt_points, 9);

  
      /* Now fill in the main battery chamber.  If the battery charge is
	 at 100%, fill in the little nipple (positive terminal) at the
	 top.  99% is everything but. */
      if (percentage < 100)
	body_perc = percentage;
      else
	body_perc = 99;

      /* The number of pixels of the main chamber that we fill in */
      height = bat->readout_batt_points[7].y - bat->readout_batt_points[0].y;

      y =  bat->readout_batt_points[0].y +
	(height - ((body_perc * height) / 100));

      if (ac_online)
	gdk_gc_set_foreground(bat->readout_gc,
			      &(bat->readout_color_ac_on));
      else
	gdk_gc_set_foreground(bat->readout_gc,
			      &(bat->readout_color_ac_off));
    
      gdk_draw_rectangle(bat->readout_pixmap,
			 bat->readout_gc,
			 TRUE,
			 bat->readout_batt_points[0].x + 1,
			 y,
			 bat->readout_batt_points[5].x -
			 bat->readout_batt_points[0].x - 1,
			 (height * body_perc) / 100);

      /* Fill in the nipple if appropriate. */
      if (percentage == 100)
	{
	  int nipple_width, nipple_height;

	  nipple_width = bat->readout_batt_points[3].x -
	    bat->readout_batt_points[2].x - 1;
	  nipple_height = bat->readout_batt_points[1].y -
	    bat->readout_batt_points[2].y;

	  gdk_draw_rectangle(bat->readout_pixmap, 
			     bat->readout_gc,
			     TRUE,
			     bat->readout_batt_points[2].x + 1,
			     bat->readout_batt_points[2].y + 1,
			     nipple_width,
			     nipple_height);
	}

    }

  /* Don't update the percentage label unless the percentage has changed. */
  if (last_percentage != percentage)
    {
      /* Now update the labels in readout mode. */
      snprintf(labelstr, sizeof(labelstr), "%d%%", percentage);
      gtk_label_set_text (GTK_LABEL(bat->readout_label_percentage), labelstr);
    }

  if (last_minutes_remaining != minutes_remaining ||
      last_hours_remaining != hours_remaining)
    {
      /* If we cannot update the time-remaining label, then make it blank */
      if (minutes_remaining == -1 || hours_remaining == -1)
	gtk_label_set_text (GTK_LABEL(bat->readout_label_time), "");
      else
	{
	  snprintf(labelstr, sizeof(labelstr), "%d:%02d", hours_remaining,
		   minutes_remaining);
	  gtk_label_set_text (GTK_LABEL(bat->readout_label_time), labelstr);
	}
    }

  /* Update the display */
  battery_expose_handler(bat->graph_area, NULL, bat);

  /* Save these values... */
  last_minutes_remaining = minutes_remaining;
  last_hours_remaining = hours_remaining;
  last_percentage = percentage;

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
battery_expose_handler(GtkWidget * widget, GdkEventExpose * expose,
		       gpointer data)
{
  BatteryData * bat = data;
  GdkPixmap * curr_pixmap;

  if (!bat->setup)
    return FALSE;

  /* Select a pixmap to draw into the applet window based on our
     current mode. */
  if (!strcasecmp(bat->mode_string, BATTERY_MODE_GRAPH))
    {
      curr_pixmap = bat->graph_pixmap;
      gdk_draw_pixmap(/* Drawable */        bat->graph_area->window,
		      /* GC */
      bat->graph_area->style->fg_gc[GTK_WIDGET_STATE(bat->graph_area)],
		  /* Src Drawable */    curr_pixmap,
		  /* X src, Y src */    0, 0,
		  /* X dest, Y dest */  0, 0,
		  /* width */           bat->graph_area->allocation.width,
		  /* height */          bat->graph_area->allocation.height);
    }
  else
    {
      gdk_draw_pixmap(/* Drawable */        bat->readout_area->window,
		      /* GC */
      bat->readout_area->style->fg_gc[GTK_WIDGET_STATE(bat->readout_area)],
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
battery_orient_handler(GtkWidget * w, PanelOrientType o, gpointer data)
{
  BatteryData * bat = data;

  /* FIXME: What do we do here? */

  return FALSE;
} /* battery_orient_handler */

gint
battery_configure_handler(GtkWidget *widget, GdkEventConfigure *event,
			  gpointer data)
{
  BatteryData * bat = data;
  
  /* We have to set up the picture here because the coordinates will
     be all wrong otherwise. */
  battery_setup_picture(bat);

  bat->force_update = TRUE;
  battery_update((gpointer) bat);

  return TRUE;
}  /* battery_configure_handler */

/* Whenever the mode changes, and when the applet first starts up,
   this function is called to hide the out-of-mode parts of the display
   and show the in-mode parts. */
void
battery_set_mode(BatteryData * bat)
{
  if (!strcmp(bat->mode_string, BATTERY_MODE_GRAPH))
    {
      /* If the property dialogue is open, update it */
      if (bat->prop_win)
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bat->mode_radio_graph),
				    1);

      battery_expose_handler(bat->graph_area, NULL, bat);
      gtk_widget_hide_all(bat->readout_frame);
      gtk_widget_show_all(bat->graph_frame);
    }
  else if (!strcmp(bat->mode_string, BATTERY_MODE_READOUT))
    {
      if (bat->prop_win)
	gtk_toggle_button_set_active
	  (GTK_TOGGLE_BUTTON(bat->mode_radio_readout), 1);

      battery_expose_handler(bat->readout_area, NULL, bat);
      gtk_widget_hide_all(bat->graph_frame);
      gtk_widget_show_all(bat->readout_frame);
    }
  else
    {
      g_error(_("Internal error: invalid mode in battery_set_mode"));
    }

  /* When we chagne modes, make sure there's not a fraction of a second
     where our pixmap is displayed as black ... */
  battery_expose_handler(bat->graph_area, NULL, bat);

} /* battery_set_mode */

static void
battery_change_mode(BatteryData * bat)
{
  if (!strcmp(bat->mode_string, BATTERY_MODE_GRAPH))
    bat->mode_string = BATTERY_MODE_READOUT;
  else
    bat->mode_string = BATTERY_MODE_GRAPH;

  battery_set_mode(bat);
} /* battery_change_mode */

static gint
battery_button_press_handler(GtkWidget * w, GdkEventButton * ev,
			     gpointer data)
{
  BatteryData * bat = data;

  battery_change_mode(bat);

  return TRUE;
} /* battery_button_press_handler */

/* This is the function that actually creates the display widgets */
void
make_new_battery_applet(void)
{
  BatteryData * bat;
  GtkWidget * root, * graph_box, * readout_box, * readout_text_vbox;
  GtkWidget * readout_battery_frame;
  gchar * param = "battery_applet";

  bat = g_new0(BatteryData, 1);

  bat->applet = applet_widget_new("battery_applet");

  if (bat->applet == NULL)
    g_error(_("Can't create applet!\n"));

  bat->last_graph_update = 0;
  bat->graph_values = NULL;
  bat->setup = 0;
  bat->force_update = TRUE;

  /* Load all the saved session parameters (or the defaults if none
     exist). */
  if ((APPLET_WIDGET(bat->applet)->privcfgpath) &&
      *(APPLET_WIDGET(bat->applet)->privcfgpath))
    battery_session_load(APPLET_WIDGET(bat->applet)->privcfgpath, bat);
  else
    battery_session_defaults(bat);

  /* All widgets will go into this container. */
  root = gtk_hbox_new(FALSE, 0);

  /* The 'graph frame' is the root widget of the entire graph mode.
     It provides the pretty bevelling and serves as an easy way of
     hiding/showing the entire graph mode. */
  bat->graph_frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(bat->graph_frame), GTK_SHADOW_IN);

  /* The 'readout frame' is the root widget of the entire readout
     mode. */
  bat->readout_frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(bat->readout_frame), GTK_SHADOW_IN);
  
  gtk_box_pack_start_defaults(GTK_BOX(root), bat->graph_frame);
  gtk_box_pack_start_defaults(GTK_BOX(root), bat->readout_frame);

  /* This is the horizontal box which will contain the picture of
     the battery and the text labels. */
  readout_box = gtk_hbox_new(FALSE, 3);

  /* The two labels go into this vbox */
  readout_text_vbox = gtk_vbox_new(FALSE, 3);

  bat->readout_label_percentage = gtk_label_new("");
  gtk_box_pack_start_defaults(GTK_BOX(readout_text_vbox),
			      bat->readout_label_percentage);

  bat->readout_label_time = gtk_label_new("");
  gtk_box_pack_start_defaults(GTK_BOX(readout_text_vbox),
			      bat->readout_label_time);

  /* This is the frame which contains the picture of the battery */
  readout_battery_frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(readout_battery_frame), GTK_SHADOW_OUT);

  /* readout_area is the drawing area into which the little picture of
     the battery gets drawn */
  bat->readout_area = gtk_drawing_area_new();
  gtk_container_add( GTK_CONTAINER(readout_battery_frame), bat->readout_area);

  gtk_box_pack_start_defaults(GTK_BOX(readout_box), readout_battery_frame);
  gtk_box_pack_start_defaults(GTK_BOX(readout_box), readout_text_vbox);

  graph_box = gtk_vbox_new(FALSE, 3);

  /* graph_area is the drawing area into which the graph of the battery
     life gets drawn. */
  bat->graph_area = gtk_drawing_area_new();

  gtk_box_pack_start_defaults( GTK_BOX(graph_box), bat->graph_area );
  gtk_container_add( GTK_CONTAINER(bat->graph_frame), graph_box );
  gtk_container_add( GTK_CONTAINER(bat->readout_frame), readout_box );

  /* Set up the event callbacks for the graph of battery life. */
  gtk_signal_connect( GTK_OBJECT(bat->graph_area), "button_press_event",
		      GTK_SIGNAL_FUNC (battery_button_press_handler), bat);
  gtk_signal_connect ( GTK_OBJECT(bat->graph_area), "expose_event",
		      (GtkSignalFunc)battery_expose_handler, bat);
  gtk_widget_set_events( bat->graph_area, GDK_EXPOSURE_MASK |
			 GDK_BUTTON_PRESS_MASK );

  /* Set up the event callbacks for the readout */
  gtk_signal_connect( GTK_OBJECT(bat->readout_area), "button_press_event",
		      (GtkSignalFunc) battery_button_press_handler, bat);
  gtk_signal_connect ( GTK_OBJECT(bat->readout_area), "expose_event",
		      (GtkSignalFunc)battery_expose_handler, bat);
  gtk_signal_connect ( GTK_OBJECT(bat->readout_area), "configure_event",
		      (GtkSignalFunc)battery_configure_handler, bat);

  gtk_widget_set_events( bat->readout_area, GDK_EXPOSURE_MASK  |
			 GDK_BUTTON_PRESS_MASK );

  /* This will let us know when the panel changes orientation */
  gtk_signal_connect(GTK_OBJECT(bat->applet), "change_orient",
		     GTK_SIGNAL_FUNC(battery_orient_handler),
		     bat);

  applet_widget_add(APPLET_WIDGET(bat->applet), root);

  gtk_signal_connect(GTK_OBJECT(bat->applet),"save_session",
		     GTK_SIGNAL_FUNC(battery_session_save),
		     bat);

  applet_widget_register_stock_callback(APPLET_WIDGET(bat->applet),
					"properties",
					GNOME_STOCK_MENU_PROP,
					("Properties..."),
					battery_properties_window,
					bat);


  gtk_widget_show_all(bat->applet);
  gtk_widget_show(bat->graph_area);
  
  /* Allocate the colors... */
  battery_create_gc(bat);
  battery_setup_colors(bat);

  /* Display only one mode */
  battery_set_mode(bat);

  /* Size things according to the saved settings */
  battery_set_size(bat);

  /* Nothing is drawn until this is set. */
  bat->setup = TRUE;

  gtk_timeout_add(500, (GtkFunction) battery_update,  bat);

} /* make_new_battery_applet */

void
battery_setup_picture(BatteryData * bat)
{
  gint readout_width, readout_height;

  readout_width = bat->readout_area->allocation.width;
  readout_height = bat->readout_area->allocation.height;

  /*  Set up the line segments for the battery picture.  The points are
      numbered as follows:
             2_ 3
         0 __| |__ 5
         8 | 1 4 |
           |     |
           |     |
           |     |
           |_____|
          7       6
  */

  /* 0 */
  bat->readout_batt_points[0].x = readout_width / 4;
  bat->readout_batt_points[0].y = readout_height / 6;

  /* 1 */
  bat->readout_batt_points[1].x = readout_width * 0.42;
  bat->readout_batt_points[1].y = readout_height / 6; 

  /* 2 */
  bat->readout_batt_points[2].x = readout_width * 0.42;
  bat->readout_batt_points[2].y = readout_height / 8;

  /* 3 */
  bat->readout_batt_points[3].x = readout_width * 0.59;
  bat->readout_batt_points[3].y = readout_height / 8;

  /* 4 */
  bat->readout_batt_points[4].x = readout_width * 0.59;
  bat->readout_batt_points[4].y = readout_height / 6;

  /* 5 */
  bat->readout_batt_points[5].x = readout_width * 0.75;
  bat->readout_batt_points[5].y = readout_height / 6;

  /* 6 */
  bat->readout_batt_points[6].x = readout_width * 0.75;
  bat->readout_batt_points[6].y = readout_height * 0.83;

  /* 7 */
  bat->readout_batt_points[7].x = readout_width * 0.25;
  bat->readout_batt_points[7].y = readout_height * 0.83;

  /* 8 */
  bat->readout_batt_points[8].x = readout_width / 4;
  bat->readout_batt_points[8].y = readout_height / 6;
} /* battery_setup_picture */

void
battery_set_size(BatteryData * bat)
{
  gtk_widget_set_usize(bat->readout_area, bat->width * 0.4, bat->height);
  gtk_widget_set_usize(bat->graph_frame, bat->width, bat->height);
  gtk_widget_set_usize(bat->readout_frame, bat->width, bat->height);

  /* If pixmaps have already been allocated, then free them here before
     creating new ones. */
  if (bat->setup)
    {
      gdk_pixmap_unref(bat->graph_pixmap);
      gdk_pixmap_unref(bat->readout_pixmap);
    }

  bat->graph_pixmap = gdk_pixmap_new(bat->graph_area->window,
				     bat->width, bat->height,
			     gtk_widget_get_visual(bat->graph_area)->depth);

  gdk_draw_rectangle(bat->graph_pixmap, bat->graph_area->style->black_gc,
		     TRUE, 0, 0,  bat->width, bat->height);

  bat->readout_pixmap = gdk_pixmap_new(bat->readout_area->window,
				       bat->width,   bat->height,
		       gtk_widget_get_visual(bat->readout_area)->depth);

  gdk_draw_rectangle(bat->readout_pixmap, bat->readout_area->style->black_gc,
		     TRUE, 0, 0, bat->width, bat->height);

  /* If we've been resized, don't throw away the old graph data */
  if (bat->graph_values != NULL)
    { 
      unsigned char * new_vals;

      new_vals = (unsigned char *) g_malloc(bat->width);
      if (new_vals == NULL)
	g_error(_("Could not allocate space for graph values"));

      memset(new_vals, 0, bat->width);

      memcpy(new_vals, bat->graph_values, bat->old_width);

      g_free(bat->graph_values);

      bat->graph_values = new_vals;
    }
  else
    {
      bat->graph_values = (unsigned char *) g_malloc(bat->width);
      memset(bat->graph_values, 0, bat->width); 
    }

  bat->old_width = bat->width;

  bat->force_update = TRUE;

  battery_update((gpointer)bat);

} /* battery_set_size */

void
battery_create_gc(BatteryData * bat)
{
  bat->gc = gdk_gc_new( bat->graph_area->window );
  gdk_gc_copy( bat->gc, bat->graph_area->style->white_gc );

  bat->readout_gc = gdk_gc_new(bat->readout_area->window);
  gdk_gc_copy( bat->readout_gc, bat->readout_area->style->white_gc );
} /* battery_create_gc */

void 
battery_setup_colors(BatteryData * bat)
{
  GdkColormap *colormap;

  colormap = gtk_widget_get_colormap(bat->graph_area);
                
  gdk_color_parse(bat->readout_color_ac_on_s,
		  &(bat->readout_color_ac_on));
  gdk_color_alloc(colormap, &(bat->readout_color_ac_on));

  gdk_color_parse(bat->readout_color_ac_off_s,
		  &(bat->readout_color_ac_off));
  gdk_color_alloc(colormap, &(bat->readout_color_ac_off));

  gdk_color_parse(bat->graph_color_ac_on_s,
		  &(bat->graph_color_ac_on));
  gdk_color_alloc(colormap, &(bat->graph_color_ac_on));

  gdk_color_parse(bat->graph_color_ac_off_s,
		  &(bat->graph_color_ac_off));
  gdk_color_alloc(colormap, &(bat->graph_color_ac_off));

} /* battery_setup_colors */

/* When we get a command to start a new widget. */
void
applet_start_new_applet(const gchar *goad_id, gpointer data)
{
  make_new_battery_applet();
} /* applet_start_new_applet */
