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
 * File: applets/battery/session.c
 *
 * This file contains the routines which save and load the applet
 * preferences.
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
#include <gnome.h>
#include <gdk/gdkx.h>

#include <applet-widget.h>

#include "battery.h"
#include "session.h"

void
battery_session_load(gchar * cfgpath, BatteryData * bat)
{

  /* We specify that we want the properties for this applet ... */
  gnome_config_push_prefix (cfgpath);

  /* Global configurable parameters */
  bat->mode_string = gnome_config_get_string
    ("battery/mode=" BATTERY_DEFAULT_MODE_STRING);

  bat->follow_panel_size = gnome_config_get_bool
    ("battery/follow_panel_size=" BATTERY_DEFAULT_FOLLOW_PANEL_SIZE);

  bat->width = gnome_config_get_int
    ("battery/width=" BATTERY_DEFAULT_WIDTH);

  bat->height = gnome_config_get_int
    ("battery/height=" BATTERY_DEFAULT_HEIGHT);

  bat->update_interval = gnome_config_get_int
    ("battery/interval=" BATTERY_DEFAULT_UPDATE_INTERVAL);

  bat->low_charge_val = gnome_config_get_int
    ("battery/low_charge_val=" BATTERY_DEFAULT_LOW_VAL);

  bat->low_warn_val = gnome_config_get_int
    ("battery/low_warn_val=" BATTERY_DEFAULT_LOW_WARN_VAL);

  bat->low_warn_enable = gnome_config_get_bool
    ("battery/low_warn_enable=" BATTERY_DEFAULT_LOW_WARN_ENABLE);

  bat->full_notify_enable = gnome_config_get_bool
    ("battery/full_notify_enable=" BATTERY_DEFAULT_FULL_NOTIFY_ENABLE);

  /* The graph */
  bat->graph_direction = gnome_config_get_int
    ("graph/direction=" BATTERY_DEFAULT_GRAPH_DIRECTION);

  strncpy(bat->graph_color_ac_on_s,
	  gnome_config_get_string
	  ("graph/ac_on_color=" BATTERY_DEFAULT_GRAPH_ACON_COLOR),
	  sizeof(bat->graph_color_ac_on_s));

  strncpy(bat->graph_color_ac_off_s,
	  gnome_config_get_string
	  ("graph/ac_off_color="  BATTERY_DEFAULT_GRAPH_ACOFF_COLOR),
	  sizeof(bat->graph_color_ac_off_s));

  strncpy(bat->graph_color_line_s,
	  gnome_config_get_string
	  ("graph/line_color="  BATTERY_DEFAULT_GRAPH_LINE_COLOR),
	  sizeof(bat->graph_color_line_s));

  strncpy(bat->graph_color_low_s,
	  gnome_config_get_string
	  ("graph/low_color="  BATTERY_DEFAULT_GRAPH_LOW_COLOR),
	  sizeof(bat->graph_color_low_s));

  strncpy(bat->graph_color_no_s,
	  gnome_config_get_string
	  ("graph/no_color="  BATTERY_DEFAULT_GRAPH_NO_COLOR),
	  sizeof(bat->graph_color_no_s));

  /* The readout */
  strncpy(bat->readout_color_ac_on_s,
	  gnome_config_get_string
	  ("readout/ac_on_color="  BATTERY_DEFAULT_READOUT_ACON_COLOR),
	  sizeof(bat->readout_color_ac_on_s));

  strncpy (bat->readout_color_ac_off_s,
	   gnome_config_get_string
	   ("readout/ac_off_color=" BATTERY_DEFAULT_READOUT_ACOFF_COLOR),
	   sizeof(bat->readout_color_ac_off_s));

  strncpy (bat->readout_color_low_s,
	   gnome_config_get_string
	   ("readout/low_color=" BATTERY_DEFAULT_READOUT_LOW_COLOR),
	   sizeof(bat->readout_color_low_s));

  strncpy (bat->readout_color_no_s,
	   gnome_config_get_string
	   ("readout/no_color=" BATTERY_DEFAULT_READOUT_NO_COLOR),
	   sizeof(bat->readout_color_no_s));

  gnome_config_pop_prefix ();
} /* battery_session_load */

int
battery_session_save(GtkWidget * w,
		     const char * privcfgpath,
		     const char * globcfgpath,
		     gpointer data)
{
  BatteryData * bat = data;
  /* char col[24]; */

  gnome_config_push_prefix (privcfgpath);

  /* Global configurable parameters */
  gnome_config_set_string ("battery/mode", bat->mode_string);
  gnome_config_set_bool ("battery/follow_panel_size", bat->follow_panel_size);
  gnome_config_set_int ("battery/width", bat->width);
  gnome_config_set_int ("battery/height", bat->height);
  gnome_config_set_int ("battery/interval", bat->update_interval);
  gnome_config_set_int ("battery/low_charge_val", bat->low_charge_val);
  gnome_config_set_int ("battery/low_warn_val", bat->low_warn_val);
  gnome_config_set_bool ("battery/low_warn_enable",
			 bat->low_warn_enable);
  gnome_config_set_bool ("battery/full_notify_enable",
			 bat->full_notify_enable);

  /* The graph */
  gnome_config_set_int("graph/direction", bat->graph_direction);

  gnome_config_set_string("graph/ac_off_color",
			  bat->graph_color_ac_off_s);
  gnome_config_set_string("graph/ac_on_color",
			  bat->graph_color_ac_on_s);
  gnome_config_set_string("graph/low_color",
			  bat->graph_color_low_s);
  gnome_config_set_string("graph/no_color",
			  bat->graph_color_no_s);
  

  /* The readout */
  gnome_config_set_string("readout/ac_on_color",
			  bat->readout_color_ac_on_s);
  gnome_config_set_string("readout/ac_off_color",
			  bat->readout_color_ac_off_s);
  gnome_config_set_string("readout/low_color",
			  bat->readout_color_low_s);
  gnome_config_set_string("readout/no_color",
			  bat->readout_color_no_s);

  gnome_config_pop_prefix ();

  gnome_config_sync ();
  gnome_config_drop_all ();

  return FALSE;
} /* battery_session_save */

void
battery_session_defaults(BatteryData * bat)
{
  /* Global configurable parameters */
  bat->mode_string =  BATTERY_DEFAULT_MODE_STRING;
  bat->width = atoi (BATTERY_DEFAULT_WIDTH);
  bat->height = atoi (BATTERY_DEFAULT_HEIGHT);
  bat->update_interval = atoi (BATTERY_DEFAULT_UPDATE_INTERVAL);
  bat->low_charge_val = atoi (BATTERY_DEFAULT_LOW_VAL);
  bat->low_warn_val = atoi (BATTERY_DEFAULT_LOW_WARN_VAL);

  if (BATTERY_DEFAULT_LOW_WARN_ENABLE != NULL && !strcmp (BATTERY_DEFAULT_LOW_WARN_ENABLE, "true"))
    bat->low_warn_enable = TRUE;
  else
    bat->low_warn_enable = FALSE;

  if (BATTERY_DEFAULT_FULL_NOTIFY_ENABLE != NULL && !strcmp (BATTERY_DEFAULT_FULL_NOTIFY_ENABLE, "true"))
    bat->full_notify_enable = TRUE;
  else
    bat->full_notify_enable = FALSE;

  /* The Graph */

  bat->graph_direction = atoi (BATTERY_DEFAULT_GRAPH_DIRECTION);

  strncpy(bat->graph_color_ac_on_s, BATTERY_DEFAULT_GRAPH_ACON_COLOR,
	  sizeof(bat->graph_color_ac_on_s));

  strncpy(bat->graph_color_ac_off_s, BATTERY_DEFAULT_GRAPH_ACOFF_COLOR,
	  sizeof(bat->graph_color_ac_off_s));

  strncpy(bat->graph_color_low_s, BATTERY_DEFAULT_GRAPH_LOW_COLOR,
	  sizeof(bat->graph_color_low_s));

  strncpy(bat->graph_color_no_s, BATTERY_DEFAULT_GRAPH_LOW_COLOR,
	  sizeof(bat->graph_color_no_s));

  strncpy(bat->graph_color_line_s, BATTERY_DEFAULT_GRAPH_LINE_COLOR,
	  sizeof(bat->graph_color_line_s));

  /* The Readout */
  strncpy(bat->readout_color_ac_on_s, BATTERY_DEFAULT_READOUT_ACON_COLOR,
	  sizeof(bat->readout_color_ac_on_s));
	  
  strncpy(bat->readout_color_ac_off_s, BATTERY_DEFAULT_READOUT_ACOFF_COLOR,
	  sizeof(bat->readout_color_ac_on_s));

  strncpy(bat->readout_color_low_s, BATTERY_DEFAULT_READOUT_LOW_COLOR,
	  sizeof(bat->readout_color_low_s));

  strncpy(bat->readout_color_no_s, BATTERY_DEFAULT_READOUT_LOW_COLOR,
	  sizeof(bat->readout_color_no_s));

} /* battery_session_defaults */
