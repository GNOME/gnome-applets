/*
 * File: applets/battery/battery.h
 *
 * Author: Nat Friedman
 *
 */

#ifndef _BATTERY_H
#define _BATTERY_H

#include <applet-widget.h>


/*
 *
 * Miguel likes closures, so let's give him closures...
 *
 */
typedef struct BatteryData {
  GtkWidget * applet;

  gchar * mode_string;
#define BATTERY_MODE_GRAPH   "graph"
#define BATTERY_MODE_READOUT "readout"
  gint width, height;
  gint old_width;

  gint setup;

  /*
   *
   * The Graph
   *
   */

  /* Configurable elements of graph mode... */
  gint graph_interval;
  gint graph_direction;
#define BATTERY_GRAPH_LEFT_TO_RIGHT 1
#define BATTERY_GRAPH_RIGHT_TO_LEFT 2
  gchar graph_color_ac_on_s[24];
  GdkColor graph_color_ac_on;
  gchar graph_color_ac_off_s[24];
  GdkColor graph_color_ac_off;

  /* Widgets n stuff... */
  GtkWidget * graph_frame;
  GdkPixmap * graph_pixmap;
  GtkWidget * graph_area;
  GdkGC *gc;
  GdkGC *readout_gc;

  /* Graph state */
  unsigned char * graph_values;
  time_t last_graph_update;

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

  /* Widgets n stuff */
  GtkWidget * readout_frame;
  GtkWidget * readout_label_percentage;
  GtkWidget * readout_label_time;
  GtkWidget * readout_area;
  GdkPixmap * readout_pixmap;

 /* Coordinates for the battery line drawing. */
  GdkPoint readout_batt_points[9];

  /*
   *
   * For the  "Properties" window ...
   *
   */
  GtkWidget * prop_win;

  /* General */
  GtkObject * height_adj, * width_adj;
  GtkWidget * mode_radio_graph, * mode_radio_readout;

  /* Graph mode */
  GtkObject * graph_speed_adj;
  GtkWidget * dir_radio;
  GnomeColorPicker * graph_ac_on_color_sel;
  GnomeColorPicker * graph_ac_off_color_sel;

  /* Readout mode */
  GnomeColorPicker * readout_ac_on_color_sel;
  GnomeColorPicker * readout_ac_off_color_sel;


} BatteryData;

/*
 *
 * Configuration defaults
 *
 */

/* Global configuration parameters */
#define BATTERY_DEFAULT_MODE_STRING   "readout"
#define BATTERY_DEFAULT_HEIGHT        "50"
#define BATTERY_DEFAULT_WIDTH         "75"


/* The Graph */
#define BATTERY_DEFAULT_GRAPH_INTERVAL "5"
#define BATTERY_DEFAULT_GRAPH_DIRECTION "2" /* Right to left */
#define BATTERY_DEFAULT_GRAPH_ACON_COLOR "#ffff00"
#define BATTERY_DEFAULT_GRAPH_ACOFF_COLOR "#008b8b"

/* The readout */
#define BATTERY_DEFAULT_READOUT_ACON_COLOR "#ffff00"
#define BATTERY_DEFAULT_READOUT_ACOFF_COLOR "#ff0000"

/*
 *
 * Prototypes
 *
 */
void battery_set_size(BatteryData * bat);
gint battery_update(gpointer data);
gint battery_orient_handler(GtkWidget * w, PanelOrientType o,
				   gpointer data);
gint battery_expose_handler(GtkWidget * widget, GdkEventExpose * expose,
				   gpointer data);
gint battery_configure_handler(GtkWidget *widget, GdkEventConfigure *event,
			       gpointer data);
void make_new_battery_applet(void);
void battery_create_gc(BatteryData * bat);
void battery_setup_colors(BatteryData * bat);
void battery_setup_picture(BatteryData * bat);
void battery_set_mode(BatteryData * bat);

void applet_start_new_applet(const gchar *param, gpointer data);

#endif /* _BATTERY_H */


