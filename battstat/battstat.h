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
 * May, 2000. Implemented on FreeBSD 4.0R (Compaq Armada M700)
 *
$Id$
 */

#define DEBUG 0

#ifdef __FreeBSD__
#define APMDEVICE	"/dev/apm"
#endif /* __FreeBSD__ */

#ifdef __OpenBSD__
#define APMDEVICE       "/dev/apm"
#endif /* __OpenBSD__ */

#define PROGLEN 33.0

enum {
	APPLET_SHOW_NONE,
	APPLET_SHOW_PERCENT,
	APPLET_SHOW_TIME
};

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
  GtkStyle *style;
  GtkWidget *progdir_radio;
  GtkWidget *radio_orient_horizont;
  GtkWidget *radio_lay_batt_on;
  GtkWidget *radio_lay_status_on;
  GtkWidget *radio_text_1;
  GtkWidget *radio_text_2;
  GtkWidget *check_text;

  GtkTooltips *tip;

  GtkWidget *about_dialog;
  GtkDialog *prop_win;

  GdkGC *pixgc;

  GtkWidget *table;

  GtkWidget *battery;
  GtkWidget *status;
  GtkWidget *percent;

  gboolean colors_changed;
  gboolean lowbattnotification;
  gboolean fullbattnot;
  gboolean beep;
  gboolean draintop;
  gboolean showstatus;
  gboolean showbattery;
  int showtext;

  gint width, height;
  gboolean horizont;

  GtkWidget *suspend_entry;
  char *suspend_cmd;
  char *fontname;
  guint red_val;
  guint orange_val;
  guint yellow_val;
  int panelsize;
  PanelAppletOrient orienttype;
  LayoutConfiguration layout;
  int pixtimer;

  GtkWidget *lowbatt_toggle;
  GtkWidget *full_toggle;
  GtkWidget *lowbattnotificationdialog;

  /* last_* for the benefit of the timeout functions */
  guint flash;
  guint last_batt_life;
  guint last_acline_status;
  guint last_batt_state;
  guint last_pixmap_index;
  guint last_charging;
} ProgressData;

enum statusimagename {BATTERY,AC,FLASH,WARNING};

GdkPixmap *statusimage[4];
GdkBitmap *statusmask[4];

extern char * battery_gray_xpm[];

void prop_cb (BonoboUIComponent *, ProgressData *, const char *);
int prop_cancel (GtkWidget *, gpointer);

void adj_value_changed_cb(GtkAdjustment *, gpointer);
void simul_cb(GtkWidget *, gpointer);
void helppref_cb(PanelApplet *, gpointer);
gint check_for_updates(gpointer);
void change_orient(PanelApplet *, PanelAppletOrient, ProgressData *);
void destroy_applet( GtkWidget *, gpointer);
void cleanup(PanelApplet *, int);
void help_cb (BonoboUIComponent *, ProgressData *, const char *);
void suspend_cb (BonoboUIComponent *, ProgressData *, const char *);
void destroy_about (GtkWidget *, gpointer);
void about_cb (BonoboUIComponent *, ProgressData *, const char *);
void load_preferences(ProgressData *battstat);

/* power-management.c */
const char *power_management_getinfo( BatteryStatus *status );
const char *power_management_initialise( void );
void power_management_cleanup( void );


void reconfigure_layout( ProgressData *battstat );
