/*
 * Copyright (C) 2000 by Jörgen Pehrson <jp@spektr.eu.org>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 $Id$
 */

#ifndef _battstat_h_
#define _battstat_h_

#include "battstat-applet.h"

#define PROGLEN 33.0

/* I made these multipliers up
 *  --davyd
 */
#define ORANGE_MULTIPLIER 1.5
#define YELLOW_MULTIPLIER 2.5

#define BATTSTAT_GSCHEMA              "org.gnome.gnome-applets.battstat"
#define KEY_RED_VALUE                 "red-value"
#define KEY_RED_VALUE_IS_TIME         "red-value-is-time"
#define KEY_LOW_BATTERY_NOTIFICATION  "low-battery-notification"
#define KEY_FULL_BATTERY_NOTIFICATION "full-battery-notification"
#define KEY_BEEP                      "beep"
#define KEY_DRAIN_FROM_TOP            "drain-from-top"
#define KEY_SHOW_STATUS               "show-status"
#define KEY_SHOW_BATTERY              "show-battery"
#define KEY_SHOW_TEXT                 "show-text"

typedef enum
{
  APPLET_SHOW_NONE,
  APPLET_SHOW_PERCENT,
  APPLET_SHOW_TIME
} AppletTextType;

typedef enum
{
  STATUS_PIXMAP_BATTERY,
  STATUS_PIXMAP_METER,
  STATUS_PIXMAP_AC,
  STATUS_PIXMAP_CHARGE,
  STATUS_PIXMAP_WARNING,
  STATUS_PIXMAP_NUM
} StatusPixmapIndex;

typedef struct
{
  gboolean on_ac_power;
  gboolean charging;
  gboolean present;
  gint minutes;
  gint percent;
} BatteryStatus;

typedef enum
{
  LAYOUT_NONE,
  LAYOUT_LONG,
  LAYOUT_TOPLEFT,
  LAYOUT_TOP,
  LAYOUT_LEFT,
  LAYOUT_CENTRE,
  LAYOUT_RIGHT,
  LAYOUT_BOTTOM
} LayoutLocation;

typedef struct
{
  LayoutLocation status;
  LayoutLocation text;
  LayoutLocation battery;
} LayoutConfiguration;

typedef struct _BattstatApplet
{
  GpApplet parent;

  GSettings *settings;

  /* these are used by properties.c */
  GtkWidget *radio_ubuntu_battery;
  GtkWidget *radio_traditional_battery;
  GtkWidget *radio_text_1;
  GtkWidget *radio_text_2;
  GtkWidget *check_text;
  GtkWidget *lowbatt_toggle;
  GtkWidget *full_toggle;
  GtkWidget *hbox_ptr;

  /* flags set from gsettings or the properties dialog */
  gint red_val;
  gint orange_val;
  gint yellow_val;
  gboolean red_value_is_time;
  gboolean lowbattnotification;
  gboolean fullbattnot;
  gboolean beep;
  gboolean draintop;
  gboolean showstatus;
  gboolean showbattery;
  AppletTextType showtext;

  /* label changed type (% <-> h:mm) and must be refreshed */
  gboolean refresh_label;

  /* the main table that contains the visual elements */
  GtkWidget *grid;

  /* the visual elements */
  GtkWidget *battery;
  GtkWidget *status;
  GtkWidget *percent;

  /* dialog boxes that might be displayed */
  GtkWidget *prop_win;
  GtkWidget *battery_low_dialog;

  /* text label inside the low battery dialog */
  GtkLabel *battery_low_label;

  /* our height/width as given to us by size_allocate */
  gint width, height;

  /* should the battery meter be drawn horizontally? */
  gboolean horizont;

  /* the current layout of the visual elements inside the table */
  LayoutConfiguration layout;

  /* g_timeout source identifier */
  int timeout_id;
  int timeout;

  /* last_* for the benefit of the check_for_updates function */
  gint last_batt_life;
  gint last_acline_status;
  StatusPixmapIndex last_pixmap_index;
  gint last_charging;
  gint last_minutes;
  gboolean last_present;
} ProgressData;

/* properties.c */
void prop_cb (GSimpleAction *, GVariant *, gpointer);

/* battstat_applet.c */
void reconfigure_layout( ProgressData *battstat );

#endif /* _battstat_h_ */
