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
 * File: applets/battery/battery.h
 *
 * Author: Nat Friedman
 *
 */

#ifndef _BATTERY_H
#define _BATTERY_H

#include <applet-widget.h>


typedef struct BatteryData {
  GtkWidget *applet;

  gchar *mode_string;
#define BATTERY_MODE_GRAPH   "graph"
#define BATTERY_MODE_READOUT "readout"

  gint width, height, old_width;

  gint setup;

  gboolean force_update;

  gint low_charge_val;

  gint update_interval;

  guint update_timeout_id;

  /*
   *
   * The Graph
   *
   */

  /* Configurable elements of graph mode... */

  gint graph_direction;
#define BATTERY_GRAPH_LEFT_TO_RIGHT 1
#define BATTERY_GRAPH_RIGHT_TO_LEFT 2

  gchar graph_color_ac_on_s[24];
  GdkColor graph_color_ac_on;

  gchar graph_color_ac_off_s[24];
  GdkColor graph_color_ac_off;

  gchar graph_color_line_s[24];
  GdkColor graph_color_line;

  gchar graph_color_low_s[24];
  GdkColor graph_color_low;

  /* Widgets n stuff... */
  GtkWidget *graph_frame;
  GdkPixmap *graph_pixmap;
  GtkWidget *graph_area;
  GdkGC *gc;
  GdkGC *readout_gc;

  GtkWidget *about_box;

  /* Graph state */
  unsigned char *graph_values;

  /*
   *
   * The Readout
   *
   */

  /* The Configurable elements of the readout */
  gchar readout_color_ac_on_s[24];
  GdkColor readout_color_ac_on;

  gchar readout_color_ac_off_s[24];
  GdkColor readout_color_ac_off;

  gchar readout_color_low_s[24];
  GdkColor readout_color_low;

  /*
   * The low-battery warning.
   */
  gboolean low_warn_enable;
  gint low_warn_val;
  gboolean warned;

  /* Widgets n stuff */
  GtkWidget *readout_frame;
  GtkWidget *readout_label_percentage;
  GtkWidget *readout_label_time;
  GtkWidget *readout_area;
  GdkPixmap *readout_pixmap;

  /* Coordinates for the battery line drawing. */
  GdkPoint readout_batt_points[9];

  /*
   *
   * For the  "Properties" window ...
   *
   */
  GnomePropertyBox *prop_win;

  /* General */
  GtkObject *height_adj, *width_adj;
  GtkWidget *mode_radio_graph, *mode_radio_readout;
  GtkObject *interval_adj,  *low_charge_adj;

  /* Graph mode */
  GtkWidget *dir_radio;
  GnomeColorPicker *graph_ac_on_color_sel;
  GnomeColorPicker *graph_ac_off_color_sel;
  GnomeColorPicker *graph_low_color_sel;
  GnomeColorPicker *graph_line_color_sel;

  /* Readout mode */
  GnomeColorPicker *readout_ac_on_color_sel;
  GnomeColorPicker *readout_ac_off_color_sel;
  GnomeColorPicker *readout_low_color_sel;

  /* Warning */
  GtkWidget *warn_check;
  GtkObject *low_warn_adj;

} BatteryData;

/*
 *
 * Configuration defaults
 *
 */

/* Global configuration parameters */
#define BATTERY_DEFAULT_MODE_STRING     "readout"
#define BATTERY_DEFAULT_HEIGHT          "48"
#define BATTERY_DEFAULT_WIDTH           "48"
#define BATTERY_DEFAULT_UPDATE_INTERVAL "2"
#define BATTERY_DEFAULT_LOW_VAL         "25"

#define BATTERY_DEFAULT_LOW_WARN_ENABLE "true"
#define BATTERY_DEFAULT_LOW_WARN_VAL    "5"

/* The Graph */
#define BATTERY_DEFAULT_GRAPH_DIRECTION "2" /*Right to left */
#define BATTERY_DEFAULT_GRAPH_ACON_COLOR "#00ff00"
#define BATTERY_DEFAULT_GRAPH_ACOFF_COLOR "#0000ff"
#define BATTERY_DEFAULT_GRAPH_LOW_COLOR "#ff0000"
#define BATTERY_DEFAULT_GRAPH_LINE_COLOR "#4d4d4d"

/* The readout */
#define BATTERY_DEFAULT_READOUT_ACON_COLOR "#00ff00"
#define BATTERY_DEFAULT_READOUT_ACOFF_COLOR "#0000ff"
#define BATTERY_DEFAULT_READOUT_LOW_COLOR "#ff0000"
/*
 *
 * Prototypes
 *
 */
void about_cb (AppletWidget *widget, gpointer data);
void destroy_about(GtkWidget *w, gpointer data);

void battery_set_size(BatteryData *bat);
gint battery_update(gpointer data);
gint battery_orient_handler(GtkWidget *w, PanelOrientType o,
				   gpointer data);
gint battery_expose_handler(GtkWidget *widget, GdkEventExpose *expose,
				   gpointer data);
gint battery_configure_handler(GtkWidget *widget, GdkEventConfigure *event,
			       gpointer data);
GtkWidget *make_new_battery_applet (const gchar *goad_id);
void battery_create_gc(BatteryData *bat);
void battery_setup_colors(BatteryData *bat);
void battery_setup_picture(BatteryData *bat);
void battery_set_mode(BatteryData *bat);
GtkWidget *applet_start_new_applet (const gchar *goad_id,
				     const char **params, int nparams);
void battery_warn (BatteryData *bat);

#endif /* _BATTERY_H */


