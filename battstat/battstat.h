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

enum statusimagename {BATTERY,AC,FLASH,WARNING};

#ifdef __FreeBSD__
#define APMDEVICE	"/dev/apm"
#endif /* __FreeBSD__ */

#ifdef __OpenBSD__
#define APMDEVICE                "/dev/apm"
#endif /* __OpenBSD__ */

#define PROGLEN 33.0

GdkPixmap *statusimage[4];
GdkBitmap *statusmask[4];

typedef struct _ProgressData {
  GtkWidget *applet;
  GnomeClient *smClient;
  GtkAdjustment *adj;
  GtkStyle *style;
  GtkTooltips *st_tip;
  GtkTooltips *ac_tip;
  GtkTooltips *progress_tip;
  GtkTooltips *progressy_tip;
  GtkWidget *docklabel;
  GtkWidget *aclabel;
  GtkWidget *fontlabel;
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
  GtkWidget *pixmapdockwid;
  GtkWidget *pixmapwidy;
  GtkWidget *ac_event_box;
  GtkWidget *progress_event_box;
  GtkWidget *about_box;
  GnomePropertyBox *prop_win;
  GtkWidget *hbox;
  GtkWidget *hbox1;
  GtkWidget *framestatus;
  GtkWidget *framebattery;
  GtkWidget *frameybattery;
  GtkWidget *percent;
  GtkWidget *eventbattery;
  GtkWidget *eventybattery;
  GtkWidget *eventstatus;
  GtkWidget *eventdock;
  GdkBitmap *statusmask;
  GdkPixmap *status;
  GtkWidget *statuspixmapwid;
  GtkWidget *statusvbox;
  GtkWidget *statuspercent;
  GtkWidget *fontpicker;
  GtkStyle *percentstyle;
  gboolean font_changed;
  gboolean colors_changed;
  gboolean lowbattnotification;
  gboolean fullbattnot;
  gboolean beep;
  gboolean draintop;
  gboolean horizont;
  gboolean own_font;
  gboolean showstatus;
  gboolean showbattery;
  gboolean showpercent;
  gboolean suspend;
  gboolean usedock;
  GtkWidget *suspend_entry;
  gchar *suspend_cmd;
  gchar *fontname;
  gint ered;
  gint eorange;
  gint eyellow;
  guint red_val;
  guint orange_val;
  guint yellow_val;
  gint panelsize;
  PanelOrientType orienttype;
  GtkObject *ered_adj;
  GtkObject *eorange_adj;
  GtkObject *eyellow_adj;
  int pixtimer;
  GtkWidget *font_toggle;
  GtkWidget *lowbatt_toggle;
  GtkWidget *dock_toggle;
  GtkWidget *full_toggle;
  GtkWidget *testpercent;
  GdkPixmap *testpixmap;
  GdkBitmap *testmask;
  GdkPixmap *testpixbuffer;
  GdkBitmap *testpixmask;
  GtkWidget *testevent;
  GdkGC *testpixgc;
  GtkWidget *testpixmapwid;
  GtkWidget *testhscale;
  GtkObject *testadj;
  GtkWidget *beep_toggle;
} ProgressData;

