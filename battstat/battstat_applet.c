/* battstat        A GNOME battery meter for laptops. 
 * Copyright (C) 2000 by Jörgen Pehrson <jp@spektr.eu.org>
 * Copyright (C) 2002 Free Software Foundation
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
 * May, 2000. Implemented on FreeBSD 4.0-RELEASE (Compaq Armada M700)
 *
 $Id$
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#ifdef __FreeBSD__
#include <machine/apm_bios.h>
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/param.h>
#include <machine/apmvar.h>
#elif __linux__
#include <apm.h>
#endif

#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/sysctl.h>

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <glade/glade.h>
#include <gnome.h>

#include <panel-applet.h>
#include <panel-applet-gconf.h>

/*#include <status-docklet.h>*/

#include "battstat.h"
#include "pixmaps.h"
#ifdef __linux__
#include "acpi-linux.h"
#endif

#ifndef gettext_noop
#define gettext_noop(String) (String)
#endif

#if defined(__NetBSD__)
#define APMDEVICE "/dev/apm"
#endif

#define GCONF_PATH ""

GtkObject *statusdock;

static const BonoboUIVerb battstat_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("BattstatProperties", prop_cb),
	BONOBO_UI_UNSAFE_VERB ("BattstatHelp",       help_cb),
	BONOBO_UI_UNSAFE_VERB ("BattstatAbout",      about_cb),
	BONOBO_UI_UNSAFE_VERB ("BattstatSuspend",    suspend_cb),
        BONOBO_UI_VERB_END
};

#define AC_POWER_STRING _("System is running on AC power")
#define DC_POWER_STRING _("System is running on battery power")

int pixel_offset_top[]={ 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5 };
int pixel_top_length[]={ 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2 };
int pixel_offset_bottom[]={ 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 38, 38 };
GdkColor green[] = {
  {0,0x7A00,0xDB00,0x7000},
  {0,0x9100,0xE900,0x8500},
  {0,0xA000,0xF100,0x9500},
  {0,0x9600,0xEE00,0x8A00},
  {0,0x8E00,0xE900,0x8100},
  {0,0x8500,0xE500,0x7700},
  {0,0x7C00,0xDF00,0x6E00},
  {0,0x7300,0xDA00,0x6500},
  {0,0x6A00,0xD600,0x5B00},
  {0,0x6000,0xD000,0x5100},
  {0,0x5600,0xCA00,0x4600},
  {0,0x5100,0xC100,0x4200},
  {-1,0x0000,0x0000,0x0000}
};
GdkColor red[] = {
  {0,0xD900,0x7200,0x7400},
  {0,0xE600,0x8800,0x8C00},
  {0,0xF000,0x9600,0x9A00},
  {0,0xEB00,0x8D00,0x9100},
  {0,0xE700,0x8300,0x8800},
  {0,0xE200,0x7A00,0x7F00},
  {0,0xDD00,0x7100,0x7600},
  {0,0xD800,0x6700,0x6D00},
  {0,0xD300,0x5D00,0x6300},
  {0,0xCD00,0x5400,0x5900},
  {0,0xC800,0x4900,0x4F00},
  {0,0xC100,0x4200,0x4700},
  {-1,0x0000,0x0000,0x0000}
};
GdkColor yellow[] = {
  {0,0xD800,0xD900,0x7200},
  {0,0xE600,0xE500,0x8800},
  {0,0xF000,0xEF00,0x9600},
  {0,0xEB00,0xEA00,0x8D00},
  {0,0xE700,0xE600,0x8300},
  {0,0xE200,0xE100,0x7A00},
  {0,0xDD00,0xDC00,0x7100},
  {0,0xD800,0xD700,0x6700},
  {0,0xD300,0xD200,0x5D00},
  {0,0xCD00,0xCC00,0x5400},
  {0,0xC800,0xC600,0x4900},
  {0,0xC100,0xBF00,0x4200},
  {-1,0x0000,0x0000,0x0000}
};
GdkColor orange[] = {
  {0,0xD900,0xAD00,0x7200},
  {0,0xE600,0xBB00,0x8800},
  {0,0xF000,0xC700,0x9600},
  {0,0xEB00,0xC000,0x8D00},
  {0,0xE700,0xB900,0x8300},
  {0,0xE200,0xB300,0x7A00},
  {0,0xDD00,0xAB00,0x7100},
  {0,0xD800,0xA400,0x6700},
  {0,0xD300,0x9E00,0x5D00},
  {0,0xCD00,0x9600,0x5400},
  {0,0xC800,0x8D00,0x4900},
  {0,0xC100,0x8600,0x4200},
  {-1,0x0000,0x0000,0x0000}
};
GdkColor gray[] = {
  {0,0xC400,0xC400,0xC400},
  {0,0xD300,0xD300,0xD300},
  {0,0xDE00,0xDE00,0xDE00},
  {0,0xD800,0xD800,0xD800},
  {0,0xD300,0xD300,0xD300},
  {0,0xCD00,0xCD00,0xCD00},
  {0,0xC700,0xC700,0xC700},
  {0,0xC100,0xC100,0xC100},
  {0,0xBB00,0xBB00,0xBB00},
  {0,0xB500,0xB500,0xB500},
  {0,0xAE00,0xAE00,0xAE00},
  {0,0xA800,0xA800,0xA800},
  {-1,0x0000,0x0000,0x0000}
};
GdkColor darkgreen[] = {
  {0,0x6500,0xC600,0x5B00},
  {0,0x7B00,0xD300,0x6F00},
  {0,0x8A00,0xDB00,0x7F00},
  {0,0x8000,0xD800,0x7400},
  {0,0x7800,0xD400,0x6B00},
  {0,0x6F00,0xCF00,0x6200},
  {0,0x6600,0xCA00,0x5900},
  {0,0x5D00,0xC500,0x5000},
  {0,0x5400,0xC100,0x4600},
  {0,0x4B00,0xBB00,0x3C00},
  {0,0x4100,0xB600,0x3100},
  {0,0x3C00,0xAC00,0x2D00},
  {-1,0x0000,0x0000,0x0000}
};
GdkColor darkorange[] = {
  {0,0xC400,0x9700,0x5C00},
  {0,0xD000,0xA500,0x7200},
  {0,0xDA00,0xB100,0x8000},
  {0,0xD500,0xAA00,0x7700},
  {0,0xD100,0xA300,0x6D00},
  {0,0xCD00,0x9D00,0x6400},
  {0,0xC700,0x9600,0x5B00},
  {0,0xC300,0x8F00,0x5200},
  {0,0xBE00,0x8800,0x4800},
  {0,0xB800,0x8100,0x3F00},
  {0,0xB300,0x7900,0x3400},
  {0,0xAC00,0x7200,0x2D00},
  {-1,0x0000,0x0000,0x0000}
};
GdkColor darkyellow[] = {
  {0,0xC200,0xC400,0x5C00},
  {0,0xD000,0xCF00,0x7200},
  {0,0xDA00,0xD900,0x8000},
  {0,0xD500,0xD400,0x7700},
  {0,0xD100,0xD000,0x6D00},
  {0,0xCD00,0xCB00,0x6400},
  {0,0xC700,0xC600,0x5B00},
  {0,0xC300,0xC200,0x5200},
  {0,0xBE00,0xBD00,0x4800},
  {0,0xB800,0xB700,0x3F00},
  {0,0xB300,0xB200,0x3400},
  {0,0xAC00,0xAA00,0x2D00},
  {-1,0x0000,0x0000,0x0000}
};
GdkColor darkred[] = {
  {0,0xC900,0x6200,0x6400},
  {0,0xD600,0x7800,0x7C00},
  {0,0xDA00,0x8000,0x8500},
  {0,0xD500,0x7700,0x7B00},
  {0,0xD100,0x6D00,0x7200},
  {0,0xCD00,0x6400,0x6900},
  {0,0xC700,0x5B00,0x6100},
  {0,0xC300,0x5200,0x5700},
  {0,0xBE00,0x4800,0x4E00},
  {0,0xB800,0x3F00,0x4400},
  {0,0xB100,0x3200,0x3700},
  {0,0xA200,0x3200,0x3700},
  {-1,0x0000,0x0000,0x0000}
};

#if defined(__NetBSD__) || defined(__OpenBSD__)
#define apm_info apm_power_info
#endif
struct apm_info apminfo;

#ifdef __linux__
struct acpi_info acpiinfo;
gboolean using_acpi;
int acpi_count;
#endif

#ifdef __FreeBSD__
void
apm_readinfo (PanelApplet *applet, ProgressData * battstat)
{
  /* This is how I read the information from the APM subsystem under
     FreeBSD.  Each time this functions is called (once every second)
     the APM device is opened, read from and then closed.
  */
  int fd;
  if (DEBUG) g_print("apm_readinfo() (FreeBSD)\n");

  fd = open(APMDEVICE, O_RDONLY);
  if (fd == -1) cleanup (applet, 1);

  if (ioctl(fd, APMIO_GETINFO, &apminfo) == -1)
    err(1, "ioctl(APMIO_GETINFO)");

  close(fd);
}
#elif defined(__NetBSD__) || defined(__OpenBSD__)
void
apm_readinfo (PanelApplet *applet, ProgressData * battstat)
{
  /* Code for OpenBSD by Joe Ammond <jra@twinight.org>. Using the same
     procedure as for FreeBSD.
  */
  int fd;
#if defined(__NetBSD__)
  if (DEBUG) g_print("apm_readinfo() (NetBSD)\n");
#else /* __OpenBSD__ */
  if (DEBUG) g_print("apm_readinfo() (OpenBSD)\n");
#endif

  fd = open(APMDEVICE, O_RDONLY);
  if (fd == -1) cleanup (applet, 1);
  if (ioctl(fd, APM_IOC_GETPOWER, &apminfo) == -1)
    err(1, "ioctl(APM_IOC_GETPOWER)");
  close(fd);
}
#elif __linux__

// Declared in acpi-linux.c
gboolean acpi_linux_read(struct apm_info *apminfo, struct acpi_info *acpiinfo);

gboolean acpi_callback (GIOChannel * chan, GIOCondition cond, gpointer data)
{
  ProgressData * battstat = (ProgressData *) data;

  if (cond & (G_IO_ERR | G_IO_HUP)) {
    acpi_linux_cleanup(&acpiinfo);
    apminfo.battery_percentage = -1;
    return FALSE;
  }
  
  if (acpi_process_event(&acpiinfo)) {
    acpi_linux_read(&apminfo, &acpiinfo);
    pixmap_timeout(data);
  }
  return TRUE;
}

void
apm_readinfo (PanelApplet *applet, ProgressData * battstat)
{
  /* Code for Linux by Thomas Hood <jdthood@mail.com>. apm_read() will
     read from /proc/... instead and we do not need to open the device
     ourselves.
  */
  if (DEBUG) g_print("apm_readinfo() (Linux)\n");

  /* ACPI support added by Lennart Poettering <lennart@poettering.de> 10/27/2001
   * Updated by David Moore <dcm@acm.org> 5/29/2003 to poll less and
   *   use ACPI events. */
  if (using_acpi && acpiinfo.event_fd >= 0) {
    if (acpi_count <= 0) {
      /* Only call this one out of 30 calls to apm_readinfo() (every 30 seconds)
       * since reading the ACPI system takes CPU cycles. */
      acpi_count=30;
      acpi_linux_read(&apminfo, &acpiinfo);
    }
    acpi_count--;
  }
  /* If we lost the file descriptor with ACPI events, try to get it back. */
  else if (using_acpi) {
      if (acpi_linux_init(&acpiinfo)) {
          battstat->acpiwatch = g_io_add_watch (acpiinfo.channel,
              G_IO_IN | G_IO_ERR | G_IO_HUP,
              acpi_callback, battstat);
          acpi_linux_read(&apminfo, &acpiinfo);
      }
  }
  else
    apm_read(&apminfo);
}

#else
void
apm_readinfo (PanelApplet *applet, ProgressData * battstat)
{
  g_print("apm_readinfo() (Generic)\n");
  g_print(
	  /* Message displayed if user tries to run applet
	     on unsupported platform */
	  _("Your platform is not supported!\n"));
  g_print(_("The applet will not work properly (if at all).\n"));
}
#endif


/* void */
/* draw_meter ( gpointer battdata, gpointer meterdata )  */
/* { */

/* } */

static char *get_remaining (struct apm_info apminfo)
{
	int time;
	int hours;
	int mins;
	guint acline_status;
	guint batt_life;

#ifdef __FreeBSD__
	acline_status = apminfo.ai_acline ? 1 : 0;
	time = apminfo.ai_batt_time;
	batt_life = apminfo.ai_batt_life;
#elif defined (__NetBSD__) || defined(__OpenBSD__)
	acline_status = apminfo.ac_state ? 1 : 0;
	time = apminfo.minutes_left;
	batt_life = apminfo.battery_life;
#elif __linux__
	acline_status = apminfo.ac_line_status ? 1 : 0;
	time = apminfo.battery_time;
	batt_life = apminfo.battery_percentage;
#endif

	if (batt_life > 100) batt_life = 100;

	hours = time / 60;
	mins = time % 60;

	if (acline_status && batt_life == 100)
		return g_strdup_printf (_("Battery charged (%d%%)"), batt_life);
	else if (time < 0 && !acline_status)
		return g_strdup_printf (_("Unknown time (%d%%) remaining"), batt_life);
	else if (time < 0 && acline_status)
		return g_strdup_printf (_("Unknown time (%d%%) until charged"), batt_life);
	else
		if (hours == 0)
			if (!acline_status)
				return g_strdup_printf (ngettext (
						"%d minute (%d%%) remaining",
						"%d minutes (%d%%) remaining",
						mins), mins, batt_life);
			else
				return g_strdup_printf (ngettext (
						"%d minute until charged (%d%%)",
						"%d minutes until charged (%d%%)",
						mins), mins, batt_life);
		else if (mins == 0)
			if (!acline_status)
				return g_strdup_printf (ngettext (
						"%d hour (%d%%) remaining",
						"%d hours (%d%%) remaining",
						hours), hours, batt_life);
			else
				return g_strdup_printf (ngettext (
						"%d hour (%d%%) until charged (%d%%)",
						"%d hours (%d%%) until charged (%d%%)",
						hours), hours, batt_life);
		else
			if (!acline_status)
				/* TRANSLATOR: "%d %s %d %s" are "%d hours %d minutes"
				 * Swap order with "%2$s %2$d %1$s %1$d if needed */
				return g_strdup_printf (_("%d %s %d %s (%d%%) remaining"),
						hours, ngettext ("hour", "hours", hours),
						mins, ngettext ("minute", "minutes", mins),
						batt_life);
			else
				/* TRANSLATOR: "%d %s %d %s" are "%d hours %d minutes"
				 * Swap order with "%2$s %2$d %1$s %1$d if needed */
				return g_strdup_printf (_("%d %s %d %s until charged (%d%%)"),
						hours, ngettext ("hour", "hours", hours),
						mins, ngettext ("minute", "minutes", mins),
						batt_life);
}

static void
on_lowbatt_notification_response (GtkWidget *widget, gint arg, GtkWidget **self)
{
	   gtk_widget_destroy (GTK_WIDGET (*self));
	   *self = NULL;
}

gint
pixmap_timeout( gpointer data )
{
  ProgressData *battery = data;
/*   MeterData *meter; */
  GdkColor *color, *darkcolor;
  guint batt_life;
  guint acline_status;
  guint batt_state;
  guint progress_value;
  guint pixmap_index;
  guint charging;
  gint i, x;
  gboolean batterypresent;
  gchar *new_label;
  gchar *new_string;
  gchar *rem_time = NULL;
  GtkWidget *hbox, *image, *label, *dialog;
  GdkPixbuf *pixbuf;
  
  if (DEBUG) g_print("pixmap_timeout()\n");

  /*
    Mental note to self explaining the expected values below.
    acline_status: 
      0 = Running on battery
      1 = Running on AC

    batt_life:
      0-100 Percentage of battery charge left

    batt_state:
      0 = High
      1 = Low
      2 = Critical
      3 = Charging
  */

   apm_readinfo (PANEL_APPLET (battery->applet), battery);
   batterypresent = TRUE;
#ifdef __FreeBSD__
   acline_status = apminfo.ai_acline ? 1 : 0;
   batt_state = apminfo.ai_batt_stat;
   batt_life = apminfo.ai_batt_life;
   charging = (batt_state == 3) ? TRUE : FALSE;
#elif defined(__NetBSD__) || defined(__OpenBSD__)
   acline_status = apminfo.ac_state ? 1 : 0;
   batt_state = apminfo.battery_state;
   batt_life = apminfo.battery_life;
   charging = (batt_state == 3) ? TRUE : FALSE;
#elif __linux__
   acline_status = apminfo.ac_line_status ? 1 : 0;
   batt_state = apminfo.battery_status;
   batt_life = (guint) apminfo.battery_percentage;
   charging = (apminfo.battery_flags & 0x8) ? TRUE : FALSE;
#else
   acline_status = 1;
   batt_state = 0;
   batt_life = 100;
   charging = TRUE;
   batterypresent = FALSE;
#endif
  if(batt_state > 3) {
    batt_state = 0;
    batterypresent = FALSE;
  }
  if(batt_life == (guint)-1) {
    batt_life = 0;
    batterypresent = FALSE;
  }
  if(batt_life > 100) batt_life = 100;
  if(batt_life == 100) charging = FALSE;
  if(!acline_status) charging = FALSE;
 
   battery->flash = battery->flash ? FALSE : TRUE;

   pixmap_index = (acline_status) ?
              (charging && battery->flash ? FLASH : AC) :
              (batt_life <= battery->red_val ? WARNING : BATTERY)
   ;

   if ( pixmap_index != battery->last_pixmap_index ) {
      gtk_image_set_from_pixmap(GTK_IMAGE (battery->statuspixmapwid),
				statusimage[pixmap_index], statusmask[pixmap_index]);
#ifdef FIXME
      gtk_image_set_from_pixmap(GTK_IMAGE (battery->pixmapdockwid),
				statusimage[pixmap_index], statusmask[pixmap_index]);
#endif
   }

   if(
     !acline_status
     && battery->last_batt_life != 1000
     && battery->last_batt_life > battery->red_val
     && batt_life <= battery->red_val
     && batterypresent
   ) {
      /* Warn that battery dropped below red_val */
      if(battery->lowbattnotification) {
	 new_string = get_remaining (apminfo);
	 new_label = g_strdup_printf (
			 "<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s. %s",
			 _("Your battery is running low"),
			 new_string,
			 _("To avoid losing work please power off, suspend or plug your laptop in."));
	 g_free (new_string);
	 battery->lowbattnotificationdialog = gtk_dialog_new_with_buttons (
			 _("Battery Notice"),
			 NULL,
			 GTK_DIALOG_DESTROY_WITH_PARENT,
			 GTK_STOCK_OK,
			 GTK_RESPONSE_ACCEPT,
			 NULL);
	 g_signal_connect (G_OBJECT (battery->lowbattnotificationdialog), "response",
			 G_CALLBACK (on_lowbatt_notification_response), &battery->lowbattnotificationdialog);
	 gtk_container_set_border_width (GTK_CONTAINER (battery->lowbattnotificationdialog), 6);
	 gtk_dialog_set_has_separator (GTK_DIALOG (battery->lowbattnotificationdialog), FALSE);
	 hbox = gtk_hbox_new (FALSE, 6);
	 pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
			 "gnome-dev-battery",
			 48,
			 GTK_ICON_LOOKUP_USE_BUILTIN,
			 NULL);
	 image = gtk_image_new_from_pixbuf (pixbuf);
	 g_object_unref (pixbuf);
	 gtk_box_pack_start (GTK_BOX (hbox), image, TRUE, TRUE, 6);
	 label = gtk_label_new (new_label);
	 gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	 gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	 g_free (new_label);
	 gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 6);
	 gtk_container_add (GTK_CONTAINER (GTK_DIALOG (battery->lowbattnotificationdialog)->vbox), hbox);
	 
	 gtk_window_set_keep_above (GTK_WINDOW (battery->lowbattnotificationdialog), TRUE);
	 gtk_window_stick (GTK_WINDOW (battery->lowbattnotificationdialog));
			
	 gtk_widget_show_all (battery->lowbattnotificationdialog);
    
         if(battery->beep)
            gdk_beep();
    
          battery->lowbattnotification=FALSE;
      }
      gnome_triggers_do ("", NULL, "battstat_applet", "LowBattery", NULL);
   }

   if(
      battery->last_charging
      && battery->last_acline_status
      && battery->last_acline_status!=1000
      && !charging 
      && acline_status
      && batterypresent
      && batt_life > 99
   ) {
      /* Inform that battery now fully charged */
      gnome_triggers_do ("", NULL, "battstat_applet", "BatteryFull", NULL);
      if(battery->fullbattnot) {
	 gchar *new_label;
	 dialog = gtk_dialog_new_with_buttons (
			 _("Battery Notice"),
			 NULL,
			 GTK_DIALOG_DESTROY_WITH_PARENT,
			 GTK_STOCK_OK,
			 GTK_RESPONSE_ACCEPT,
			 NULL);
	 g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
			 G_CALLBACK (gtk_widget_destroy), GTK_OBJECT (dialog));
	 gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
	 gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	 hbox = gtk_hbox_new (FALSE, 6);
	 pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
			 "gnome-dev-battery",
			 48,
			 GTK_ICON_LOOKUP_USE_BUILTIN,
			 NULL);
	 image = gtk_image_new_from_pixbuf (pixbuf);
	 g_object_unref (pixbuf);
	 gtk_box_pack_start (GTK_BOX (hbox), image, TRUE, TRUE, 6);
	 new_label = g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>",
			 _("Your battery is now fully recharged"));
	 label = gtk_label_new (new_label);
	 g_free (new_label);
	 gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	 gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	 gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 6);
	 gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), hbox);
	 gtk_widget_show_all (dialog);
	 
         if (battery->beep)
            gdk_beep();
       }
   }

   if (   !battery->last_charging
       && charging
       && acline_status
       && batterypresent
      ) {
	   /*
	    * the battery is charging again, reset the dialog display flag
	    * Thanks to Richard Kinder <r_kinder@yahoo.com> for this patch.
	    */
	   battery->lowbattnotification = panel_applet_gconf_get_bool (
			   PANEL_APPLET(battery->applet),
			   GCONF_PATH "low_battery_notification", NULL);

	   if (battery->lowbattnotificationdialog)
	   {
		   /* we can remove the battery warning dialog */
		   gtk_widget_destroy (battery->lowbattnotificationdialog);
		   battery->lowbattnotificationdialog = NULL;
	   }
   }

   if(
      acline_status != battery->last_acline_status
      || batt_life != battery->last_batt_life
      || batt_state != battery->last_batt_state
      || battery->colors_changed )
   {
      /* Something changed */

      /* Update the tooltip */

      if(!battery->showbattery && !battery->showtext) {
	 gchar *new_string;
	 new_string = get_remaining (apminfo);

	 if(acline_status == 0) {
		new_label = g_strdup_printf ("%s\n%s",
				DC_POWER_STRING,
				new_string);
	 } else {
		new_label = g_strdup_printf ("%s\n%s",
				AC_POWER_STRING,
				new_string);
	 }
	 g_free (new_string);
      } else {
	 if(acline_status == 0) {
		new_label = g_strdup (DC_POWER_STRING);
	 } else {
		new_label = g_strdup (AC_POWER_STRING);
	 }
      }

      gtk_tooltips_set_tip (battery->ac_tip,
			    battery->eventstatus,
			    new_label,
			    NULL);
      g_free (new_label);
   
      /* Update the battery meter, tooltip and label */
      
      if (batterypresent && battery->showtext == APPLET_SHOW_PERCENT)
	      new_label = g_strdup_printf ("%d%%", batt_life);
      else if (batterypresent && battery->showtext == APPLET_SHOW_TIME)
      {
	      if (acline_status && batt_life == 100)
		      new_label = g_strdup ("-:--");
	      else
	      {
		      int time;
		      time = apminfo.battery_time;
		      new_label = g_strdup_printf ("%d:%02d", time/60, time%60);
	      }
      }
      else
	      new_label = g_strdup (_("N/A"));

      gtk_label_set_text (GTK_LABEL (battery->percent), new_label);
      gtk_label_set_text (GTK_LABEL (battery->statuspercent), new_label);
      g_free (new_label);
      
      if (batt_life <= battery->red_val) {
	 color=red;
	 darkcolor=darkred;
      } else if (batt_life <= battery->orange_val) {
	 color=orange;
	 darkcolor=darkorange;
      } else if (batt_life <= battery->yellow_val) {
	 color=yellow;
	 darkcolor=darkyellow;
      } else {
	 color=green;
	 darkcolor=darkgreen;
      }
      
      if(battery->showbattery) {
	 if(battery->horizont)
	   gdk_draw_drawable(battery->pixmap,
			     battery->pixgc,
			     battery->pixbuffer,
			     0,0,
			     0,0,
			     -1,-1);
	 else
	   gdk_draw_drawable(battery->pixmapy,
			     battery->pixgc,
			     battery->pixbuffery,
			     0,0,
			     0,0,
			     -1,-1);
	 
	 if(battery->draintop) {
	    progress_value = PROGLEN*batt_life/100.0;
	    
	    for(i=0; color[i].pixel!=-1; i++) {
	       gdk_gc_set_foreground(battery->pixgc, &color[i]);
	       if(battery->horizont) {
		  gdk_draw_line (battery->pixmap,
				 battery->pixgc,
				 pixel_offset_top[i], i+2,
				 pixel_offset_top[i]+progress_value, i+2);
	       }
	       else {
		  gdk_draw_line (battery->pixmapy,
				 battery->pixgc,
				 i+2, pixel_offset_top[i],
				 i+2, pixel_offset_top[i]+progress_value);
	       }
	    }
	 } else {
	    progress_value = PROGLEN*batt_life/100.0;
	    
	    for(i=0; color[i].pixel!=-1; i++) {
	       gdk_gc_set_foreground(battery->pixgc, &color[i]);
	       if(battery->horizont) {
		  gdk_draw_line (battery->pixmap,
				 battery->pixgc,
				 pixel_offset_bottom[i], i+2,
				 pixel_offset_bottom[i]-progress_value, i+2);
	       }
	       else {
		  gdk_draw_line (battery->pixmapy,
				 battery->pixgc,
				 i+2, pixel_offset_bottom[i]-1,
				 i+2, pixel_offset_bottom[i]-progress_value);
	       }
	    }
	    for(i=0; darkcolor[i].pixel!=-1; i++) {
	       x=(pixel_offset_bottom[i]-progress_value)-pixel_top_length[i];
	       if(x < pixel_offset_top[i]) {
		  x = pixel_offset_top[i];
	       }
	       if(progress_value < 33) {
		  gdk_gc_set_foreground(battery->pixgc, &darkcolor[i]);
		     
		  if(battery->horizont)
		     gdk_draw_line (battery->pixmap,
				    battery->pixgc,
				    (pixel_offset_bottom[i]-progress_value)-1, i+2,
				    x, i+2);
		  else
		     gdk_draw_line (battery->pixmapy,
				    battery->pixgc,
				    i+2, (pixel_offset_bottom[i]-progress_value)-1,
				    i+2, x);
	       }
	    }
	 }

	 if(battery->horizont) {
	    GdkPixmap *pixmap;
	    gtk_image_get_pixmap (GTK_IMAGE (battery->pixmapwid), &pixmap, NULL);
	    gdk_draw_drawable(pixmap,
			      battery->pixgc,
			      battery->pixmap,
			      0,0,
			      0,0,
			      -1,-1);
	    gtk_widget_queue_draw (GTK_WIDGET (battery->pixmapwid));
	 }
	 else {
	    GdkPixmap *pixmap;
	    gtk_image_get_pixmap (GTK_IMAGE (battery->pixmapwidy), &pixmap, NULL);
	    gdk_draw_drawable(pixmap,
			      battery->pixgc,
			      battery->pixmapy,
			      0,0,
			      0,0,
			      -1,-1);
	    gtk_widget_queue_draw (GTK_WIDGET (battery->pixmapwidy));
	 }
      }
      
      rem_time = get_remaining(apminfo);
      
      if (battery->showstatus == 0) {
	 if (batterypresent) {
	    if(acline_status == 0) {
			/* This string will display as a tooltip over the battery frame
			 when the computer is using battery power. */
		new_string = g_strdup_printf ("%s\n%s",
				DC_POWER_STRING, rem_time);
	    } else {
			/* This string will display as a tooltip over the battery frame
			 when the computer is using AC power. */
		new_string = g_strdup_printf ("%s\n%s",
			AC_POWER_STRING, rem_time);
	    }
	 } else {
	    if(acline_status == 0) {
			/* This string will display as a tooltip over the
			 battery frame when the computer is using battery
			 power and the battery isn't present. Not a
			 possible combination, I guess... :)*/
		new_string = g_strdup_printf ("%s\n%s",
				DC_POWER_STRING,
				_("Battery status unknown"));
	    } else {
			/* This string will display as a tooltip over the
			 battery frame when the computer is using AC
			 power and the battery isn't present.*/
		new_string = g_strdup_printf ("%s\n%s",
				AC_POWER_STRING,
				_("No battery present"));
	    }
	 }
      } else {
	 if (batterypresent) {
		    /* Displayed as a tooltip over the battery meter when there is
		     a battery present. %d will hold the current charge and %s will
		     contains a string of the time remaining */
		new_string = g_strdup (rem_time);
	 } else {
		     /* Displayed as a tooltip over the battery meter when no
		      battery is present. */
		new_string = g_strdup (_("No battery present"));
	 }
      }
      
      g_free (rem_time);
      
      gtk_tooltips_set_tip(battery->progress_tip,
			   battery->eventbattery,
			   gettext(new_string),
			   NULL);
      gtk_tooltips_set_tip(battery->progressy_tip,
			   battery->eventybattery,
			   gettext(new_string),
			   NULL);
      g_free (new_string);
   }

   battery->last_charging = charging;
   battery->last_batt_state = batt_state;
   battery->last_batt_life = batt_life;
   battery->last_acline_status = acline_status;
   battery->last_pixmap_index = pixmap_index;

   
   return TRUE;
}

void
destroy_applet (GtkWidget *widget, gpointer data)
{
   ProgressData *pdata = data;
   
   if (DEBUG) g_print("destroy_applet()\n");

   if (pdata->about_dialog)
	gtk_widget_destroy (pdata->about_dialog);

   if (pdata->prop_win)
	gtk_widget_destroy (GTK_WIDGET (pdata->prop_win));

   gtk_timeout_remove (pdata->pixtimer);
   pdata->pixtimer = 0;
   pdata->applet = NULL;
   g_object_unref(G_OBJECT (pdata->pixgc));
   g_object_unref(pdata->ac_tip);
   g_object_unref(pdata->progress_tip);
   g_object_unref(pdata->progressy_tip);
   g_object_unref(pdata->pixmap);
   g_object_unref(pdata->pixmapy);
   g_object_unref(pdata->pixbuffer);
   g_object_unref(pdata->mask);
   g_object_unref(pdata->masky);
   g_object_unref(pdata->pixmask);
#ifdef __linux__
   if (using_acpi) {
     if (pdata->acpiwatch != 0)
         g_source_remove(pdata->acpiwatch);
     pdata->acpiwatch = 0;
     acpi_linux_cleanup(&acpiinfo);
   }
#endif

   if (pdata->suspend_cmd)
   	g_free (pdata->suspend_cmd);
   
   g_free (pdata);

   return;
}

static void
battstat_error_dialog (PanelApplet *applet,
		       char        *msg)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new(NULL,
			0,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			msg);
	gtk_window_set_screen (GTK_WINDOW (dialog),
			       gtk_widget_get_screen (GTK_WIDGET (applet)));

	gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);
}

void
cleanup (PanelApplet *applet,
	 int          status)
{
   if (DEBUG) g_print("cleanup()\n");
   
   switch (status) {
    case 1:
      battstat_error_dialog (
	       applet,
	       /* Displayed if the APM device couldn't be opened. (Used under *BSD)*/
	       _("Can't open the APM device!\n\n"
		 "Make sure you have read permission to the\n"
		 "APM device."));
      exit(1);
      break;
    case 2:
      battstat_error_dialog (
	       applet,
	       /* Displayed if the APM system is disabled (Used under *BSD)*/	     
	       _("The APM Management subsystem seems to be disabled.\n"
		 "Try executing \"apm -e 1\" (FreeBSD) and see if \n"
		 "that helps.\n"));
      gtk_main_quit();
      break;
   }
}

void
help_cb (BonoboUIComponent *uic,
	 ProgressData      *battstat,
	 const char        *verb)
{
    GError *error = NULL;

    gnome_help_display_on_screen (
		"battstat", NULL,
		gtk_widget_get_screen (battstat->applet),
		&error);

    if (error) { /* FIXME: the user needs to see this */
        g_warning ("help error: %s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
}

void
helppref_cb (PanelApplet *applet, gpointer data)
{
  /* FIXME: use gnome_help_display_on_screen()
       GnomeHelpMenuEntry help_entry = {
      "battstat_applet", "index.html#BATTSTAT_PREFS"
   };
   gnome_help_display (NULL, &help_entry);
  */
}

void
suspend_cb (BonoboUIComponent *uic,
	    ProgressData      *battstat,
	    const char        *verb)
{
   if(battstat->suspend_cmd && strlen(battstat->suspend_cmd)>0) {
      GError *err = NULL;
      gboolean ret;
      gint shell_ret = 0;

      ret = g_spawn_command_line_sync(battstat->suspend_cmd,
		      NULL, NULL, &shell_ret, &err);
      if (ret == FALSE || shell_ret != 0)
      {
	      gchar *msg;

	      if (err != NULL)
	      {
		      msg = g_strdup_printf(_("An error occured while launching the Suspend command: %s\nPlease try to correct this error"), err->message);
	      } else {
		      /* Probably because the shell_ret is != 0 */
		      if (battstat->suspend_cmd)
		          msg = g_strdup_printf(_("The Suspend command '%s' was unsuccessful."), battstat->suspend_cmd);
		      else
		          msg = g_strdup_printf(_("The Suspend command was unsuccessful."));
	      }
	      battstat_error_dialog (PANEL_APPLET (battstat->applet), msg);
	      g_free(msg);
	      if (err != NULL)
		      g_error_free(err);
      }
   } else {
      battstat_error_dialog (PANEL_APPLET (battstat->applet), _("Suspend command wasn't setup correctly in the preferences.\nPlease change the preferences and try again."));
   }
   
   return;
}

void
about_cb (BonoboUIComponent *uic,
	  ProgressData      *battstat,
	  const char        *verb)
{
   GdkPixbuf   *pixbuf;
   
   const gchar *authors[] = {
	/* if your charset supports it, please replace the "o" in
	 * "Jorgen" into U00F6 */
	_("Jorgen Pehrson <jp@spektr.eu.org>"), 
	"Lennart Poettering <lennart@poettering.de> (Linux ACPI support)",
	"Seth Nickell <snickell@stanford.edu> (GNOME2 port)",
	"Davyd Madeley <davyd@madeley.id.au>",
	NULL
   };

   const gchar *documenters[] = {
        "Jorgen Pehrson <jp@spektr.eu.org>",
        "Trevor Curtis <tcurtis@somaradio.ca>",
	NULL
   };

   const gchar *translator_credits = _("translator_credits");

   if (battstat->about_dialog) {
	gtk_window_set_screen (GTK_WINDOW (battstat->about_dialog),
			       gtk_widget_get_screen (GTK_WIDGET (battstat->applet)));
	   
	gtk_window_present (GTK_WINDOW (battstat->about_dialog));
	return;
   }
   
   pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
		   "battstat", 48, 0, NULL);
   
   battstat->about_dialog = gnome_about_new (
				/* The long name of the applet in the About dialog.*/
				_("Battery Charge Monitor"), 
				VERSION,
				_("(C) 2000 The Gnulix Society, (C) 2002-2004 Free Software Foundation"),
				_("This utility shows the status of your laptop battery."),
				authors,
				documenters,
				strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
				pixbuf);
   
   if (pixbuf)
	g_object_unref (pixbuf);

   gtk_window_set_wmclass (GTK_WINDOW (battstat->about_dialog), "battery charge monitor", "Batter Charge Monitor");
   gtk_window_set_screen (GTK_WINDOW (battstat->about_dialog),
			  gtk_widget_get_screen (battstat->applet));

   g_signal_connect (battstat->about_dialog,
		     "destroy",
		     G_CALLBACK (gtk_widget_destroyed),
		     &battstat->about_dialog);

   gtk_widget_show (battstat->about_dialog);
}

void
change_orient (PanelApplet       *applet,
	       PanelAppletOrient  orient,
	       ProgressData      *battstat)
{
   gchar *new_label;
   guint acline_status;
   guint batt_state;
   guint batt_life;
   gchar *time_string;

   battstat->orienttype = orient;
   
   if (DEBUG) g_print("change_orient()\n");

   apm_readinfo(PANEL_APPLET (applet), battstat);
#ifdef __FreeBSD__
   acline_status = apminfo.ai_acline ? 1 : 0;
   batt_state = apminfo.ai_batt_stat;
   batt_life = apminfo.ai_batt_life;
#elif defined(__NetBSD__) || defined(__OpenBSD__)
   acline_status = apminfo.ac_state ? 1 : 0;
   batt_state = apminfo.battery_state;
   batt_life = apminfo.battery_life;
#elif __linux__
   acline_status = apminfo.ac_line_status ? 1 : 0;
   batt_state = apminfo.battery_status;
   batt_life = apminfo.battery_percentage;
#else
   acline_status = 1;
   batt_state = 0;
   batt_life = 100;
#endif
   time_string = get_remaining (apminfo);
   if(batt_state > 3) batt_state = 0;
   if(batt_life > 100) batt_life = 100;

   switch(battstat->orienttype) {
    case PANEL_APPLET_ORIENT_UP:
    case PANEL_APPLET_ORIENT_DOWN:
      if(battstat->panelsize<46)
	battstat->horizont=TRUE;
      else
	battstat->horizont=FALSE;
      break;
    case PANEL_APPLET_ORIENT_LEFT:
    case PANEL_APPLET_ORIENT_RIGHT:
      if ( (!battstat->showtext && battstat->panelsize>=60) || (battstat->showtext && battstat->panelsize>=90) )
	  battstat->horizont=TRUE;
      else 
	battstat->horizont=FALSE;
      break;
   }
   
   if (battstat->horizont) {
      /* Horizontal mode */
      gtk_widget_hide (battstat->statuspercent);
      gtk_widget_hide (battstat->frameybattery);
      gtk_widget_show (battstat->framebattery);
      
      battstat->showstatus ?
	gtk_widget_show (battstat->statuspixmapwid):
	gtk_widget_hide (battstat->statuspixmapwid);
      battstat->showtext ?
	gtk_widget_show (battstat->percent):gtk_widget_hide (battstat->percent);
      battstat->showbattery ?
	gtk_widget_show (battstat->pixmapwid):gtk_widget_hide (battstat->pixmapwid);
      (battstat->showbattery || battstat->showtext) ?
	gtk_widget_show (battstat->framebattery):gtk_widget_hide (battstat->framebattery);
      
      if (battstat->showbattery == 0 && battstat->showtext == 0) {
	 if(acline_status == 0) {
		     /* This string will display as a tooltip over the status frame
		      when the computer is using battery power and the battery meter
	  	      and percent meter is hidden by the user. */
		new_label = g_strdup_printf ("%s\n%s",
				DC_POWER_STRING, time_string);
	 } else {
		     /* This string will display as a tooltip over the status frame
		      when the computer is using AC power and the battery meter
		      and percent meter is hidden by the user. */
		new_label = g_strdup_printf ("%s\n%s",
				AC_POWER_STRING, time_string);
	 }
	 gtk_tooltips_set_tip (battstat->ac_tip,
			       battstat->eventstatus,
			       new_label,
			       NULL);
	 g_free (new_label);
	 g_free (time_string);
      } else {
	 if(acline_status == 0) {
	    /* 0 = Battery power */
		     /* This string will display as a tooltip over the status frame
		      when the computer is using battery power.*/
		new_label = g_strdup (DC_POWER_STRING);
	 } else {
	    /* 1 = AC power. I should really test it explicitly here. */
		     /* This string will display as a tooltip over the status frame
		      when the computer is using AC power.*/
		new_label = g_strdup(AC_POWER_STRING);
	 }
	 gtk_tooltips_set_tip (battstat->ac_tip,
			       battstat->eventstatus,
			       new_label,
			       NULL);
	 g_free (new_label);
#ifdef FIXME
	 gtk_widget_set_usize(battstat->framestatus, 20, 24);
	 if(battstat->showbattery == 1 && battstat->showpercent == 1) {
	   /*	    width=gdk_string_width((battstat->percentstyle)->font,"100%")+46+3;*/
	    gtk_widget_set_usize(battstat->framebattery, width, 24);
	 } else if(battstat->showbattery == 0 && battstat->showpercent == 1) {
	   /*	    width=gdk_string_width((battstat->percentstyle)->font,"100%")+6;*/
	    gtk_widget_set_usize(battstat->framebattery, width, 24);
	 } else if(battstat->showbattery == 1 && battstat->showpercent == 0) {
	    gtk_widget_set_usize(battstat->framebattery, 46, 24);	
	 }
#endif
      }
   } else {
      /* Square mode */
     /*      width=gdk_string_width((battstat->percentstyle)->font,"100%")+6;*/
      
      gtk_widget_hide(battstat->framebattery);
      gtk_widget_show(battstat->frameybattery);
      battstat->showstatus ?
	gtk_widget_show (battstat->statuspixmapwid):
	gtk_widget_hide (battstat->statuspixmapwid);
      battstat->showtext ?
	gtk_widget_show (battstat->statuspercent):gtk_widget_hide (battstat->statuspercent);
      battstat->showbattery ?
	gtk_widget_show (battstat->frameybattery):gtk_widget_hide (battstat->frameybattery);
#ifdef FIXME
      battstat->showpercent ?
	gtk_widget_set_usize(battstat->framestatus,width,24):
      gtk_widget_set_usize(battstat->framestatus, 20, 24);
#endif      
      if(battstat->showstatus == 0 && battstat->showtext == 1) {
	 gtk_widget_show (battstat->framestatus);
	 gtk_widget_hide (battstat->statuspixmapwid);
#ifdef FIXME
	 gtk_widget_set_usize(battstat->framestatus, width, -1);
#endif
      }
#ifdef FIXME
      if(battstat->showstatus == 1 && battstat->showpercent == 0) {
	 gtk_widget_set_usize(battstat->framestatus, 20, 24);
      }
      if(battstat->showstatus == 1 && battstat->showpercent == 1) {
	 gtk_widget_set_usize(battstat->framestatus, width, 46);
      }
      if(battstat->showstatus == 1) {
	 gtk_widget_show (battstat->statuspixmapwid);      
      }
#endif
      if(battstat->showbattery == 0 && battstat->showtext == 0) {
	 if(acline_status == 0) {
	    /* 0 = Battery power */
		     /* This string will display as a tooltip over the status frame
		      when the computer is using battery power and the battery meter
		      and percent meter is hidden by the user. */
		new_label = g_strdup_printf ("%s\n%s",
				DC_POWER_STRING, time_string);
	 } else {
	    /* 1 = AC power. I should really test it explicitly here. */
		     /* This string will display as a tooltip over the status frame
		      when the computer is using AC power and the battery meter
		      and percent meter is hidden by the user. */
		new_label = g_strdup_printf ("%s\n%s",
				AC_POWER_STRING, time_string);
	 }
	 gtk_tooltips_set_tip (battstat->ac_tip,
			       battstat->eventstatus,
			       new_label,
			       NULL);
	 g_free (new_label);
      } else {
	 if(acline_status == 0) {
	    /* 0 = Battery power */
		     /* This string will display as a tooltip over the status frame
		      when the computer is using battery power.*/
		new_label = g_strdup (DC_POWER_STRING);
	 } else {
	    /* 1 = AC power. I should really test it explicitly here. */
		     /* This string will display as a tooltip over the status frame
		      when the computer is using AC power.*/
		new_label = g_strdup (AC_POWER_STRING);
	 }
	 gtk_tooltips_set_tip (battstat->ac_tip,
			       battstat->eventstatus,
			       new_label,
			       NULL);
	 g_free (new_label);
	 g_free (time_string);
      }
   }
   
   return;
}

void
size_allocate(PanelApplet *applet, GtkAllocation *allocation, gpointer data)
{
   ProgressData *battstat = data;
   
   if (DEBUG) g_print("applet_change_pixel_size()\n");
   
   if (battstat->orienttype == PANEL_APPLET_ORIENT_LEFT || battstat->orienttype == PANEL_APPLET_ORIENT_RIGHT) {
     if (battstat->panelsize == allocation->width)
       return;
     battstat->panelsize = allocation->width;
   } else {
     if (battstat->panelsize == allocation->height)
       return;
     battstat->panelsize = allocation->height;
   }
   
   battstat->colors_changed=TRUE;
   change_orient(applet, battstat->orienttype, battstat);
   pixmap_timeout( battstat );
   battstat->colors_changed=FALSE;
}

static void
change_background (PanelApplet *a,
		   PanelAppletBackgroundType type,
		   GdkColor *color,
		   GdkPixmap *pixmap,
		   ProgressData *battstat)
{
	/* taken from the Trash Applet */
	GtkRcStyle *rc_style;
	GtkStyle *style;

	/* reset style */
	gtk_widget_set_style (GTK_WIDGET (battstat->applet), NULL);
	gtk_widget_set_style (GTK_WIDGET (battstat->eventstatus), NULL);
	gtk_widget_set_style (GTK_WIDGET (battstat->eventbattery), NULL);
	rc_style = gtk_rc_style_new ();
	gtk_widget_modify_style (GTK_WIDGET (battstat->applet), rc_style);
	gtk_widget_modify_style (GTK_WIDGET (battstat->eventstatus), rc_style);
	gtk_widget_modify_style (GTK_WIDGET (battstat->eventbattery), rc_style);
	g_object_unref (rc_style);

	switch (type) {
		case PANEL_COLOR_BACKGROUND:
			gtk_widget_modify_bg (GTK_WIDGET (battstat->applet),
					GTK_STATE_NORMAL, color);
			gtk_widget_modify_bg (
					GTK_WIDGET (battstat->eventstatus),
					GTK_STATE_NORMAL, color);
			gtk_widget_modify_bg (
					GTK_WIDGET (battstat->eventbattery),
					GTK_STATE_NORMAL, color);
			break;

		case PANEL_PIXMAP_BACKGROUND:
			style = gtk_style_copy
				(GTK_WIDGET (battstat->applet)->style);
			if (style->bg_pixmap[GTK_STATE_NORMAL])
				g_object_unref
					(style->bg_pixmap[GTK_STATE_NORMAL]);
			style->bg_pixmap[GTK_STATE_NORMAL] = g_object_ref
				(pixmap);
			gtk_widget_set_style (GTK_WIDGET (battstat->applet),
					style);
			gtk_widget_set_style (
					GTK_WIDGET (battstat->eventstatus),
					style);
			gtk_widget_set_style (
					GTK_WIDGET (battstat->eventbattery),
					style);
			break;

		case PANEL_NO_BACKGROUND:
		default:
			break;
	}
}

static gboolean
button_press_cb (GtkWidget *widget, GdkEventButton *event, ProgressData *battstat)
{
	if (event->button == 1 && event->type == GDK_2BUTTON_PRESS) 
		suspend_cb (NULL, battstat, NULL);

	return FALSE;
}

static gboolean
key_press_cb (GtkWidget *widget, GdkEventKey *event, ProgressData *battstat)
{
	switch (event->keyval) {
	case GDK_KP_Enter:
	case GDK_ISO_Enter:
	case GDK_3270_Enter:
	case GDK_Return:
	case GDK_space:
	case GDK_KP_Space:
		suspend_cb (NULL, battstat, NULL);
		return TRUE;

	default:
		break;
	}

	return FALSE;
}

void
load_preferences(ProgressData *battstat)
{
  PanelApplet *applet = PANEL_APPLET (battstat->applet);

  if (DEBUG) g_print("load_preferences()\n");
  
  battstat->red_val = panel_applet_gconf_get_int (applet, GCONF_PATH "red_value", NULL);
  battstat->red_val = CLAMP (battstat->red_val, 0, 100);
  battstat->orange_val = panel_applet_gconf_get_int (applet, GCONF_PATH "orange_value", NULL);
  battstat->orange_val = CLAMP (battstat->orange_val, 0, 100);
  battstat->yellow_val = panel_applet_gconf_get_int (applet, GCONF_PATH "yellow_value", NULL);
  battstat->yellow_val = CLAMP (battstat->yellow_val, 0, 100);
  battstat->lowbattnotification = panel_applet_gconf_get_bool (applet, GCONF_PATH "low_battery_notification", NULL);
  battstat->fullbattnot = panel_applet_gconf_get_bool (applet, GCONF_PATH "full_battery_notification", NULL);
  battstat->beep = panel_applet_gconf_get_bool (applet, GCONF_PATH "beep", NULL);
  battstat->draintop = panel_applet_gconf_get_bool (applet, GCONF_PATH "drain_from_top", NULL);
  
  battstat->showstatus = panel_applet_gconf_get_bool (applet, GCONF_PATH "show_status", NULL);
  battstat->showbattery = panel_applet_gconf_get_bool (applet, GCONF_PATH "show_battery", NULL);
  battstat->showtext = panel_applet_gconf_get_int (applet, GCONF_PATH "show_text", NULL);
  battstat->suspend_cmd = panel_applet_gconf_get_string (applet, GCONF_PATH "suspend_command", NULL);
  
}

gint
create_layout(ProgressData *battstat)
{
   int i;
   GtkWidget *vbox;
   
   if (DEBUG) g_print("create_layout()\n");
   
   battstat->framestatus = gtk_frame_new(NULL);
   /*gtk_widget_set_usize( battstat->framestatus, 20, 24);*/
   gtk_frame_set_shadow_type ( GTK_FRAME (battstat->framestatus), 
			       GTK_SHADOW_NONE);
   gtk_widget_show(battstat->framestatus);
   gtk_box_pack_start( GTK_BOX(battstat->hbox1), battstat->framestatus, 
		       TRUE, TRUE, 0);
   
   battstat->eventstatus = gtk_event_box_new ();
   gtk_widget_show (battstat->eventstatus);
   gtk_container_add (GTK_CONTAINER (battstat->framestatus), 
		      battstat->eventstatus);
   
   /* Intermediate vbox to get the 'status' and 'charge' indicators centered. */
   vbox = gtk_vbox_new (FALSE, 0);
   gtk_container_add(GTK_CONTAINER(battstat->eventstatus),
		     vbox);
   battstat->statusvbox = gtk_vbox_new ( FALSE, 5);
   gtk_box_pack_start (GTK_BOX(vbox), battstat->statusvbox, TRUE, FALSE, 0);
   gtk_widget_show_all (vbox);
   
   battstat->framebattery = gtk_frame_new(NULL);
   /*gtk_widget_set_usize( battstat->framebattery, -1, 24);*/
   gtk_frame_set_shadow_type ( GTK_FRAME (battstat->framebattery), 
			       GTK_SHADOW_NONE);
   gtk_widget_show(battstat->framebattery);
   gtk_box_pack_start(GTK_BOX(battstat->hbox1), battstat->framebattery, 
		      TRUE, TRUE, 0);
   
   battstat->eventbattery = gtk_event_box_new();
   gtk_widget_show (battstat->eventbattery);
   gtk_container_add (GTK_CONTAINER (battstat->framebattery), 
		      battstat->eventbattery);
   
   battstat->hbox = gtk_hbox_new (FALSE, 2);
   gtk_widget_show (battstat->hbox);
   gtk_container_add (GTK_CONTAINER (battstat->eventbattery), battstat->hbox);
   
   gtk_widget_show(battstat->applet);
   
   battstat->style = gtk_widget_get_style( battstat->applet );
   
   battstat->pixmap = gdk_pixmap_create_from_xpm_d( battstat->applet->window,
						    &battstat->mask,
						    &battstat->style->bg[GTK_STATE_NORMAL],
						    battery_gray_xpm );
   battstat->pixmapy = gdk_pixmap_create_from_xpm_d( battstat->applet->window,
						     &battstat->masky,
						     &battstat->style->bg[GTK_STATE_NORMAL],
						     battery_y_gray_xpm );
   battstat->pixbuffer = gdk_pixmap_create_from_xpm_d( battstat->applet->window,
						       &battstat->pixmask,
						       &battstat->style->bg[GTK_STATE_NORMAL],
						       battery_gray_xpm ); 
   battstat->pixbuffery = gdk_pixmap_create_from_xpm_d( battstat->applet->window,
							&battstat->pixmasky,
							&battstat->style->bg[GTK_STATE_NORMAL],
							battery_y_gray_xpm ); 
   battstat->pixgc=gdk_gc_new(battstat->pixmap);
   
   battstat->pixmapwid = gtk_image_new_from_pixmap( battstat->pixmap,
						    battstat->mask );
   battstat->pixmapwidy = gtk_image_new_from_pixmap( battstat->pixmapy,
						     battstat->masky );
   gtk_box_pack_start (GTK_BOX (battstat->hbox), GTK_WIDGET (battstat->pixmapwid), TRUE, FALSE, 0);
   gtk_widget_show ( GTK_WIDGET (battstat->pixmapwid) );

   battstat->frameybattery = gtk_frame_new(NULL);
   /*gtk_widget_set_usize( battstat->frameybattery, 24, 46);*/
   gtk_frame_set_shadow_type ( GTK_FRAME (battstat->frameybattery), GTK_SHADOW_NONE);
   gtk_box_pack_start(GTK_BOX(battstat->hbox1), battstat->frameybattery, TRUE, FALSE, 0);
   battstat->eventybattery = gtk_event_box_new();
   gtk_widget_show (battstat->eventybattery);
   gtk_container_add (GTK_CONTAINER (battstat->frameybattery), battstat->eventybattery);
   gtk_container_add (GTK_CONTAINER (battstat->eventybattery),
                      GTK_WIDGET (battstat->pixmapwidy));

   gtk_widget_show ( GTK_WIDGET (battstat->pixmapwidy) );
   
   g_signal_connect(G_OBJECT(battstat->pixmapwid),
		    "destroy",
		    G_CALLBACK(destroy_applet),
		    battstat);
   
   battstat->percent = gtk_label_new ("0%");
   gtk_widget_show(battstat->percent);
   gtk_box_pack_start (GTK_BOX(battstat->hbox), battstat->percent, TRUE, TRUE, 0);
   
   statusimage[BATTERY] = gdk_pixmap_create_from_xpm_d( battstat->applet->window,
							&statusmask[BATTERY],
							&battstat->style->bg[GTK_STATE_NORMAL],
							battery_small_xpm );
   statusimage[AC] = gdk_pixmap_create_from_xpm_d( battstat->applet->window,
						   &statusmask[AC],
						   &battstat->style->bg[GTK_STATE_NORMAL],
						   ac_small_xpm );
   
   statusimage[FLASH] = gdk_pixmap_create_from_xpm_d( battstat->applet->window,
						      &statusmask[FLASH],
						      &battstat->style->bg[GTK_STATE_NORMAL],
						      flash_small_xpm );
   
   statusimage[WARNING] = gdk_pixmap_create_from_xpm_d( battstat->applet->window,
							&statusmask[WARNING],
							&battstat->style->bg[GTK_STATE_NORMAL],
							warning_small_xpm );
   battstat->status=statusimage[BATTERY];
   battstat->statuspixmapwid = gtk_image_new_from_pixmap ( battstat->status, statusmask[BATTERY] );
   gtk_box_pack_start (GTK_BOX (battstat->statusvbox), battstat->statuspixmapwid, FALSE, TRUE, 0);
   gtk_widget_show ( battstat->statuspixmapwid );

   battstat->statuspercent = gtk_label_new("100%");
   gtk_box_pack_start (GTK_BOX (battstat->statusvbox), battstat->statuspercent, FALSE, TRUE, 0);
   gtk_widget_show (battstat->statuspercent);
 
   /* Alloc battery colors */
   for(i=0; orange[i].pixel!=-1; i++) {
      gdk_colormap_alloc_color (gdk_colormap_get_system(), 
				&darkorange[i], FALSE, TRUE);
      
      gdk_colormap_alloc_color (gdk_colormap_get_system(), 
				&darkyellow[i], FALSE, TRUE);
      
      gdk_colormap_alloc_color (gdk_colormap_get_system (), 
				&darkred[i], FALSE, TRUE);
      
      gdk_colormap_alloc_color (gdk_colormap_get_system (), 
				&darkgreen[i], FALSE, TRUE);
      
      gdk_colormap_alloc_color (gdk_colormap_get_system (), 
				&orange[i], FALSE, TRUE);
      
      gdk_colormap_alloc_color (gdk_colormap_get_system (), 
				&yellow[i], FALSE, TRUE);
      
      gdk_colormap_alloc_color (gdk_colormap_get_system (), 
				&red[i], FALSE, TRUE);
      
      gdk_colormap_alloc_color (gdk_colormap_get_system (), 
				&green[i], FALSE, TRUE);
      
      gdk_colormap_alloc_color (gdk_colormap_get_system (), 
				&gray[i], FALSE, TRUE);
   }
   
   /* Set the default tooltips.. */
   battstat->ac_tip = gtk_tooltips_new ();
   g_object_ref (battstat->ac_tip);
   gtk_object_sink (GTK_OBJECT (battstat->ac_tip));
   gtk_tooltips_set_tip (battstat->ac_tip,
			 battstat->eventstatus,
			 "",
			 NULL);
   battstat->progress_tip = gtk_tooltips_new ();
   g_object_ref (battstat->progress_tip);
   gtk_object_sink (GTK_OBJECT (battstat->progress_tip));
   gtk_tooltips_set_tip (battstat->progress_tip,
			 battstat->eventbattery,
			 "",
			 NULL);
   battstat->progressy_tip = gtk_tooltips_new ();
   g_object_ref (battstat->progressy_tip);
   gtk_object_sink (GTK_OBJECT (battstat->progressy_tip));
   gtk_tooltips_set_tip (battstat->progressy_tip,
			 battstat->eventybattery,
			 "",
			 NULL);

   g_signal_connect (battstat->applet,
		     "change_orient",
		     G_CALLBACK(change_orient),
		     battstat);

   g_signal_connect (battstat->applet,
		     "size_allocate",
   		     G_CALLBACK (size_allocate),
		     battstat);

   g_signal_connect (battstat->applet,
		     "change_background",
		     G_CALLBACK (change_background),
		     battstat);

   g_signal_connect (battstat->applet,
		     "button_press_event",
   		     G_CALLBACK (button_press_cb),
		     battstat);

   g_signal_connect (battstat->applet,
		     "key_press_event",
		     G_CALLBACK (key_press_cb),
		     battstat);

   return FALSE;
}

static gboolean
battstat_applet_fill (PanelApplet *applet)
{
  ProgressData *battstat;
  struct stat statbuf;
  AtkObject *atk_widget;

  if (DEBUG) g_print("main()\n");
  
  gtk_window_set_default_icon_name ("battstat");
  
  panel_applet_add_preferences (applet, "/schemas/apps/battstat-applet/prefs", NULL);
  panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

#ifdef __linux__
  if (acpi_linux_init(&acpiinfo)) {
    using_acpi = TRUE;
    acpi_count = 0;
  }
  else
    using_acpi = FALSE;

  /* If neither ACPI nor APM could be read, but ACPI does seem to be
   * installed, warn the user how to get ACPI working. */
  if (!using_acpi && (apm_exists() == 1) &&
          (stat("/proc/acpi", &statbuf) == 0)) {
      battstat_error_dialog (
	       applet,
	       _("Can't access ACPI events in /var/run/acpid.socket! "
		 "Make sure the ACPI subsystem is working and "
		 "the acpid daemon is running."));
      using_acpi = TRUE;
      acpi_count = 0;
  }
#endif
  apm_readinfo (applet, NULL);

#ifdef __FreeBSD__
  if(apminfo.ai_status == 0) cleanup (applet, 2);
#endif
  
  battstat = g_new0 (ProgressData, 1);
  
  battstat->applet = GTK_WIDGET (applet);
  gtk_widget_realize (battstat->applet);

  battstat->hbox1 = gtk_hbox_new (FALSE, 1);
  gtk_widget_show(battstat->hbox1);

  battstat->flash = FALSE;
  battstat->last_batt_life = 1000;
  battstat->last_acline_status = 1000;
  battstat->last_batt_state = 1000;
  battstat->last_pixmap_index = 1000;
  battstat->last_charging = 1000;
  battstat->colors_changed = TRUE;
  battstat->suspend_cmd = NULL;
  battstat->orienttype = panel_applet_get_orient (applet);
  battstat->panelsize = panel_applet_get_size (applet);
  battstat->horizont = TRUE;
  battstat->about_dialog = NULL;

  glade_init ();
  
  load_preferences(battstat);
  create_layout(battstat);

  pixmap_timeout(battstat);
  change_orient (applet, battstat->orienttype, battstat );

  battstat->pixtimer = gtk_timeout_add (1000, pixmap_timeout, battstat);
#ifdef __linux__
  /* Watch for ACPI events and handle them immediately with acpi_callback(). */
  if (using_acpi && acpiinfo.event_fd >= 0) {
    battstat->acpiwatch = g_io_add_watch (acpiinfo.channel,
        G_IO_IN | G_IO_ERR | G_IO_HUP,
        acpi_callback, battstat);
  }
#endif

  gtk_container_add (GTK_CONTAINER (battstat->applet), battstat->hbox1);

  panel_applet_setup_menu_from_file (PANEL_APPLET (battstat->applet), 
  			             NULL,
                                     "GNOME_BattstatApplet.xml",
                                     NULL,
                                     battstat_menu_verbs,
                                     battstat);

  if (panel_applet_get_locked_down (PANEL_APPLET (battstat->applet))) {
	  BonoboUIComponent *popup_component;

	  popup_component = panel_applet_get_popup_component (PANEL_APPLET (battstat->applet));

	  bonobo_ui_component_set_prop (popup_component,
					"/commands/BattstatProperties",
					"hidden", "1",
					NULL);
  }

  atk_widget = gtk_widget_get_accessible (battstat->applet);
  if (GTK_IS_ACCESSIBLE (atk_widget)) {
	  atk_object_set_name (atk_widget, _("Battery Charge Monitor"));
	  atk_object_set_description(atk_widget, _("Monitor a laptop's remaining power"));
  }

  return TRUE;
}


static gboolean
battstat_applet_factory (PanelApplet *applet,
			 const gchar          *iid,
			 gpointer              data)
{
  gboolean retval = FALSE;
  
  if (!strcmp (iid, "OAFIID:GNOME_BattstatApplet"))
    retval = battstat_applet_fill (applet);
  
  return retval;
}


PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_BattstatApplet_Factory",
			     PANEL_TYPE_APPLET,
                             "battstat",
                             "0",
                             battstat_applet_factory,
                             NULL)
      

