#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gnome.h>
#include <gdk/gdkx.h>

#include "applet-lib.h"
#include "applet-widget.h"

#include "battery.h"
#include "session.h"

void
battery_session_load(gchar * cfgpath, BatteryData * bat)
{

  /* We specify that we want the properties for this applet ... */
  gnome_config_push_prefix (cfgpath);

  /* Global configurable parameters */
  bat->mode_string = gnome_config_get_string_with_default
    ("battery/mode=" BATTERY_DEFAULT_MODE_STRING, NULL);

  bat->width = gnome_config_get_int_with_default
    ("battery/width=" BATTERY_DEFAULT_WIDTH, NULL);
  bat->height = gnome_config_get_int_with_default
    ("battery/height=" BATTERY_DEFAULT_HEIGHT, NULL);

  /* The graph */
  
  bat->graph_interval = gnome_config_get_int_with_default
    ("graph/interval=" BATTERY_DEFAULT_GRAPH_INTERVAL, NULL);
  bat->graph_direction = gnome_config_get_int_with_default
    ("graph/direction=" BATTERY_DEFAULT_GRAPH_DIRECTION, NULL);

  strncpy(bat->graph_color_ac_on_s,
	  gnome_config_get_string_with_default
	  ("graph/ac_on_color=" BATTERY_DEFAULT_GRAPH_ACON_COLOR, NULL),
	  sizeof(bat->graph_color_ac_on_s));

  strncpy(bat->graph_color_ac_off_s,
	  gnome_config_get_string_with_default
	  ("graph/ac_off_color="  BATTERY_DEFAULT_GRAPH_ACOFF_COLOR, NULL),
	  sizeof(bat->graph_color_ac_off_s));

  /* The readout */
  strncpy(bat->readout_color_ac_on_s,
	  gnome_config_get_string_with_default
	  ("readout/ac_on_color="  BATTERY_DEFAULT_READOUT_ACON_COLOR, NULL),
	  sizeof(bat->readout_color_ac_on_s));

  strncpy(bat->readout_color_ac_off_s,
	  gnome_config_get_string_with_default
	  ("readout/ac_off_color=" BATTERY_DEFAULT_READOUT_ACOFF_COLOR, NULL),
	  sizeof(bat->readout_color_ac_off_s));

  gnome_config_pop_prefix ();
} /* battery_session_load */

void
battery_session_save(GtkWidget * w,
		     const char * privcfgpath,
		     const char * globcfgpath,
		     gpointer data)
{
  BatteryData * bat = data;
  char col[24];

  gnome_config_push_prefix (privcfgpath);

  /* Global configurable parameters */
  gnome_config_set_string("battery/mode", bat->mode_string);
  gnome_config_set_int("battery/width", bat->width);
  gnome_config_set_int("battery/height", bat->height);

  /* The graph */
  gnome_config_set_int("graph/interval", bat->graph_interval);
  gnome_config_set_int("graph/direction", bat->graph_direction);

  gnome_config_set_string("graph/ac_off_color",
			  bat->graph_color_ac_off_s);
  gnome_config_set_string("graph/ac_on_color",
			  bat->graph_color_ac_on_s);
  

  /* The readout */
  gnome_config_set_string("readout/ac_on_color",
			  bat->readout_color_ac_on_s);
  gnome_config_set_string("readout/ac_off_color",
			  bat->readout_color_ac_off_s);

  gnome_config_pop_prefix ();

  gnome_config_sync();
  gnome_config_drop_all();

} /* battery_session_save */

void
battery_session_defaults(BatteryData * bat)
{
  /* Global configurable parameters */
  bat->mode_string =  BATTERY_DEFAULT_MODE_STRING;
  bat->width = atoi(BATTERY_DEFAULT_WIDTH);
  bat->height = atoi(BATTERY_DEFAULT_HEIGHT);

  /* The Graph */
  bat->graph_interval = atoi(BATTERY_DEFAULT_GRAPH_INTERVAL);
  bat->graph_direction = atoi(BATTERY_DEFAULT_GRAPH_DIRECTION);
  strncpy(bat->graph_color_ac_on_s, BATTERY_DEFAULT_GRAPH_ACON_COLOR,
	  sizeof(bat->graph_color_ac_on_s));
  strncpy(bat->graph_color_ac_off_s, BATTERY_DEFAULT_GRAPH_ACOFF_COLOR,
	  sizeof(bat->graph_color_ac_off_s));

  /* The Readout */
  strncpy(bat->readout_color_ac_on_s, BATTERY_DEFAULT_READOUT_ACON_COLOR,
	  sizeof(bat->readout_color_ac_on_s));
	  
  strncpy(bat->readout_color_ac_off_s, BATTERY_DEFAULT_READOUT_ACOFF_COLOR,
	  sizeof(bat->readout_color_ac_on_s));

} /* battery_session_defaults */






