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

/*
 * Not used yet... 
 */
typedef struct _MeterData {
  GdkPixmap *pixbuffer;
  GdkBitmap *pixmask;
  GdkColor *color;
  GdkColor *darkcolor;
} MeterData;

typedef struct _ProgressData {
  GtkWidget *applet;
  GtkStyle *style;
  GtkTooltips *ac_tip;
  GtkTooltips *progress_tip;
  GtkTooltips *progressy_tip;
  GtkWidget *progdir_radio;
  GtkWidget *radio_orient_horizont;
  GtkWidget *radio_lay_batt_on;
  GtkWidget *radio_lay_status_on;
  GtkWidget *radio_lay_percent_on;
  GdkPixmap *pixbuffer;
  GdkBitmap *pixmask;
  GdkPixmap *pixbuffery;
  GdkBitmap *pixmasky;
  GdkPixmap *pixmap;
  GdkPixmap *pixmapy;
  GdkBitmap *masky;
  GdkGC *pixgc;
  GdkBitmap *mask;
  GtkWidget *pixmapwid;
  GtkWidget *pixmapwidy;
  GtkWidget *about_dialog;
  GtkDialog *prop_win;
  GtkWidget *hbox;
  GtkWidget *hbox1;
  GtkWidget *frameybattery;
  GtkWidget *framebattery;
  GtkWidget *framestatus;
  GtkWidget *percent;
  GtkWidget *eventbattery;
  GtkWidget *eventybattery;
  GtkWidget *eventstatus;
  GdkPixmap *status;
  GtkWidget *statuspixmapwid;
  GtkWidget *statusvbox;
  GtkWidget *statuspercent;

  gboolean colors_changed;
  gboolean lowbattnotification;
  gboolean fullbattnot;
  gboolean beep;
  gboolean draintop;
  gboolean horizont;
  gboolean showstatus;
  gboolean showbattery;
  gboolean showpercent;

  GtkWidget *suspend_entry;
  char *suspend_cmd;
  char *fontname;
  guint red_val;
  guint orange_val;
  guint yellow_val;
  int panelsize;
  PanelAppletOrient orienttype;
  GtkObject *ered_adj;
  GtkObject *eorange_adj;
  GtkObject *eyellow_adj;
  int pixtimer;
  int acpiwatch;

  GtkWidget *lowbatt_toggle;
  GtkWidget *full_toggle;
  GtkWidget *testpercent;
  GdkPixmap *testpixmap;
  GdkBitmap *testmask;
  GdkPixmap *testpixbuffer;
  GdkBitmap *testpixmask;
  GdkGC *testpixgc;
  GtkWidget *testpixmapwid;
  GtkObject *testadj;
  GtkWidget *beep_toggle;
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

/*guint pixmap_type;*/

extern char * battery_gray_xpm[];

void prop_cb (BonoboUIComponent *, ProgressData *, const char *);
int prop_cancel (GtkWidget *, gpointer);

void apm_readinfo(PanelApplet *, ProgressData *);
void adj_value_changed_cb(GtkAdjustment *, gpointer);
void simul_cb(GtkWidget *, gpointer);
void helppref_cb(PanelApplet *, gpointer);
gint pixmap_timeout(gpointer);
void change_orient(PanelApplet *, PanelAppletOrient, ProgressData *);
void destroy_applet( GtkWidget *, gpointer);
void cleanup(PanelApplet *, int);
void help_cb (BonoboUIComponent *, ProgressData *, const char *);
void suspend_cb (BonoboUIComponent *, ProgressData *, const char *);
void destroy_about (GtkWidget *, gpointer);
void about_cb (BonoboUIComponent *, ProgressData *, const char *);
void change_size(PanelApplet *, gint, gpointer);
void change_background(PanelApplet *a, PanelAppletBackgroundType type, 
		GdkColor *color, GdkPixmap *pixmap, ProgressData *battstat);	
gint create_layout(ProgressData *battstat);
void load_preferences(ProgressData *battstat);
