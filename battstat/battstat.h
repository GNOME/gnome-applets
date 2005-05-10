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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 $Id$
 */


#include <glib.h>
#include <gtk/gtk.h>

#define DEBUG 0

#define PROGLEN 33.0

typedef enum
{
  APPLET_SHOW_NONE,
  APPLET_SHOW_PERCENT,
  APPLET_SHOW_TIME
} AppletTextType;

typedef enum
{
  STATUS_PIXMAP_BATTERY,
  STATUS_PIXMAP_AC,
  STATUS_PIXMAP_CHARGE,
  STATUS_PIXMAP_WARNING,
  STATUS_PIXMAP_NUM
} StatusPixmapIndex;

typedef enum
{
  BATTERY_HIGH     = 0,
  BATTERY_LOW      = 1,
  BATTERY_CRITICAL = 2,
  BATTERY_CHARGING = 3
} BatteryState;

typedef struct
{
  BatteryState state;
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

typedef struct _ProgressData {
  GtkWidget *applet;

  /* these are used by properties.c */
  GtkWidget *radio_lay_batt_on;
  GtkWidget *radio_lay_status_on;
  GtkWidget *radio_text_1;
  GtkWidget *radio_text_2;
  GtkWidget *check_text;
  GtkWidget *suspend_entry;
  GtkWidget *lowbatt_toggle;
  GtkWidget *full_toggle;

  /* flags set from gconf or the properties dialog */
  guint red_val;
  guint orange_val;
  guint yellow_val;
  gboolean lowbattnotification;
  gboolean fullbattnot;
  gboolean beep;
  gboolean draintop;
  gboolean showstatus;
  gboolean showbattery;
  AppletTextType showtext;
  char *suspend_cmd;

  /* label changed type (% <-> h:mm) and must be refreshed */
  gboolean refresh_label;

  /* tooltip attached to 'applet' */
  GtkTooltips *tip;

  /* so we don't have to alloc/dealloc this every refresh */
  GdkGC *pixgc;

  /* the main table that contains the visual elements */
  GtkWidget *table;

  /* the visual elements */
  GtkWidget *battery;
  GtkWidget *status;
  GtkWidget *percent;

  /* dialog boxes that might be displayed */
  GtkWidget *about_dialog;
  GtkDialog *prop_win;
  GtkWidget *battery_low_dialog;

  /* text label inside the low battery dialog */
  GtkLabel *battery_low_label;

  /* our height/width as given to us by size_allocate */
  gint width, height;

  /* should the battery meter be drawn horizontally? */
  gboolean horizont;

  /* on a vertical or horizontal panel? (up/down/left/right) */
  PanelAppletOrient orienttype;

  /* the current layout of the visual elements inside the table */
  LayoutConfiguration layout;

  /* gtk_timeout identifier */
  int pixtimer;

  /* last_* for the benefit of the check_for_updates function */
  guint last_batt_life;
  guint last_acline_status;
  guint last_batt_state;
  StatusPixmapIndex last_pixmap_index;
  guint last_charging;
  guint last_minutes;
} ProgressData;

/* properties.c */
void prop_cb (BonoboUIComponent *, ProgressData *, const char *);

/* battstat_applet.c */
void reconfigure_layout( ProgressData *battstat );

/* power-management.c */
const char *power_management_getinfo( BatteryStatus *status );
const char *power_management_initialise( void );
void power_management_cleanup( void );
