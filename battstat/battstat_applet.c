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

#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <stdio.h>

#ifdef __FreeBSD__
#include <machine/apm_bios.h>
#elif __OpenBSD__
#include <sys/param.h>
#include <machine/apmvar.h>
#elif __linux__
#include <apm.h>
#endif

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

#include <egg-screen-help.h>

/*#include <status-docklet.h>*/

#include "battstat.h"
#include "pixmaps.h"
#include "acpi-linux.h"

#ifndef gettext_noop
#define gettext_noop(String) (String)
#endif

GtkObject *statusdock;

static const BonoboUIVerb battstat_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("BattstatProperties", prop_cb),
	BONOBO_UI_UNSAFE_VERB ("BattstatHelp",       help_cb),
	BONOBO_UI_UNSAFE_VERB ("BattstatAbout",      about_cb),
	BONOBO_UI_UNSAFE_VERB ("BattstatSuspend",    suspend_cb),
        BONOBO_UI_VERB_END
};

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

#ifdef __OpenBSD__
struct apm_power_info apminfo;
#else
struct apm_info apminfo;
#endif

#ifdef __FreeBSD__
void
apm_readinfo (PanelApplet *applet)
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
#elif __OpenBSD__
void
apm_readinfo (PanelApplet *applet)
{
  /* Code for OpenBSD by Joe Ammond <jra@twinight.org>. Using the same
     procedure as for FreeBSD.
  */
  int fd;
  if (DEBUG) g_print("apm_readinfo() (OpenBSD)\n");

  fd = open(APMDEVICE, O_RDONLY);
  if (fd == -1) cleanup (applet, 1);
  if (ioctl(fd, APM_IOC_GETPOWER, &apminfo) == -1)
    err(1, "ioctl(APM_IOC_GETPOWER)");
  close(fd);
}
#elif __linux__

// Declared in acpi-linux.c
gboolean acpi_linux_read(struct apm_info *apminfo);

void
apm_readinfo (PanelApplet *applet)
{
  /* Code for Linux by Thomas Hood <jdthood@mail.com>. apm_read() will
     read from /proc/... instead and we do not need to open the device
     ourselves.
  */
  if (DEBUG) g_print("apm_readinfo() (Linux)\n");

  // ACPI support added by Lennart Poettering <lennart@poettering.de> 10/27/2001
  // First try ACPI kernel interface, than fall back on APM
  if (!acpi_linux_read(&apminfo))
    apm_read(&apminfo);
}
#else
void
apm_readinfo (PanelApplet *applet)
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
  gchar new_label[80];
  gchar new_string[80];
  gchar *status[]={
    /* The following four messages will be displayed as tooltips over
     the battery meter.  
     High = The APM BIOS thinks that the battery charge is High.*/
    gettext_noop ("High"),
    /* Low = The APM BIOS thinks that the battery charge is Low.*/
    gettext_noop ("Low"),
    /* Critical = The APM BIOS thinks that the battery charge is Critical.*/
    gettext_noop ("Critical"),
    /* Charging = The APM BIOS thinks that the battery is recharging.*/
    gettext_noop ("Charging")
  };

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

   apm_readinfo (PANEL_APPLET (battery->applet));
   batterypresent = TRUE;
#ifdef __FreeBSD__
   acline_status = apminfo.ai_acline ? 1 : 0;
   batt_state = apminfo.ai_batt_stat;
   batt_life = apminfo.ai_batt_life;
   charging = (batt_state == 3) ? TRUE : FALSE;
#elif __OpenBSD__
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
      gtk_pixmap_set(GTK_PIXMAP (battery->statuspixmapwid),
                     statusimage[pixmap_index], statusmask[pixmap_index]);
#ifdef FIXME
      gtk_pixmap_set(GTK_PIXMAP (battery->pixmapdockwid),
                     statusimage[pixmap_index], statusmask[pixmap_index]);
#endif
   }

   if(
     !acline_status
     && battery->last_batt_life != 1000
     && battery->last_batt_life > battery->red_val
     && batt_life <= battery->red_val
   ) {
      /* Warn that battery dropped below red_val */
      if(battery->lowbattnotification) {
         snprintf(new_label, sizeof(new_label),_("Battery low (%d%%) and AC is offline"),
                  batt_life);
         gnome_warning_dialog(new_label);
    
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
         gnome_ok_dialog( _("Battery is now fully re-charged!"));
         if (battery->beep)
            gdk_beep();
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

      if(!battery->showbattery && !battery->showpercent) {
	 if(acline_status == 0) {
	    snprintf(new_label, sizeof(new_label),
		     _("System is running on battery power\nBattery: %d%% (%s)"),
		     batt_life, _(status[batt_state]));
	 } else {
	    snprintf(new_label, sizeof(new_label),
		     _("System is running on AC power\nBattery: %d%% (%s)"),
		     batt_life, _(status[batt_state]));
	 }
      } else {
	 if(acline_status == 0) {
	    snprintf(new_label, sizeof(new_label),
		     _("System is running on battery power"));
	 } else {
	    snprintf(new_label, sizeof(new_label),
		     _("System is running on AC power"));
	 }
      }

      gtk_tooltips_set_tip (battery->ac_tip,
			    battery->eventstatus,
			    new_label,
			    NULL);
   
      /* Update the battery meter, tooltip and label */
      
      if (batterypresent) {
	 snprintf(new_label, sizeof(new_label),"%d%%", batt_life);
      } else {
	 snprintf(new_label, sizeof(new_label),_("N/A"));
      }

      gtk_label_set_text (GTK_LABEL (battery->percent), new_label);
      gtk_label_set_text (GTK_LABEL (battery->statuspercent), new_label);
      
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
	   gdk_draw_pixmap(battery->pixmap,
			   battery->pixgc,
			   battery->pixbuffer,
			   0,0,
			   0,0,
			   -1,-1);
	 else
	   gdk_draw_pixmap(battery->pixmapy,
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
	    gtk_pixmap_get (GTK_PIXMAP (battery->pixmapwid), &pixmap, NULL);
	    gdk_draw_pixmap(pixmap,
			    battery->pixgc,
			    battery->pixmap,
			    0,0,
			    0,0,
			    -1,-1);
	    gtk_widget_queue_draw (GTK_WIDGET (battery->pixmapwid));
	 }
	 else {
	    GdkPixmap *pixmap;
	    gtk_pixmap_get (GTK_PIXMAP (battery->pixmapwidy), &pixmap, NULL);
	    gdk_draw_pixmap(pixmap,
			    battery->pixgc,
			    battery->pixmapy,
			    0,0,
			    0,0,
			    -1,-1);
	    gtk_widget_queue_draw (GTK_WIDGET (battery->pixmapwidy));
	 }
      }
      
      if (battery->showstatus == 0) {
	 if (batterypresent) {
	    if(acline_status == 0) {
	       snprintf(new_string, sizeof(new_string), 
			/* This string will display as a tooltip over the battery frame
			 when the computer is using battery power.*/
			_("System is running on battery power. Battery: %d%% (%s)"),
			batt_life,
			_(status[batt_state]));
	    } else {
	       snprintf(new_string, sizeof(new_string), 
			/* This string will display as a tooltip over the battery frame
			 when the computer is using AC power.*/
			_("System is running on AC power. Battery: %d%% (%s)"),
			batt_life,
			_(status[batt_state]));
	    }
	 } else {
	    if(acline_status == 0) {
	       snprintf(new_string, sizeof(new_string), 
			/* This string will display as a tooltip over the
			 battery frame when the computer is using battery
			 power and the battery isn't present. Not a
			 possible combination, I guess... :)*/
			_("System is running on battery power. Battery: Not present"));
	    } else {
	       snprintf(new_string, sizeof(new_string), 
			/* This string will display as a tooltip over the
			 battery frame when the computer is using AC
			 power and the battery isn't present.*/
			_("System is running on AC power. Battery: Not present"));
	    }
	 }
      } else {
	 if (batterypresent) {
	   snprintf(new_string, sizeof(new_string), 
		    /* Displayed as a tooltip over the battery meter when there is
		     a battery present. %d will hold the current charge and %s will
		     hold the status of the battery, (High, Low, Critical, Charging. */
		    (_("Battery: %d%% (%s)")),
		    batt_life,
		    _(status[batt_state]));
	 } else {
	    snprintf(new_string, sizeof(new_string), 
		     /* Displayed as a tooltip over the battery meter when no
		      battery is present. */
		     (_("Battery: Not present")));
	 }
      }
      
      gtk_tooltips_set_tip(battery->progress_tip,
			   battery->eventbattery,
			   gettext(new_string),
			   NULL);
      gtk_tooltips_set_tip(battery->progressy_tip,
			   battery->eventybattery,
			   gettext(new_string),
			   NULL);
      
      if (DEBUG) printf("Percent: %d, Status: %s\n", batt_life, status[batt_state]);
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

   if (pdata->prop_win)
	gtk_widget_destroy (GTK_WIDGET (pdata->prop_win));

   gtk_timeout_remove (pdata->pixtimer);
   pdata->pixtimer = 0;
   pdata->applet = NULL;
   g_object_unref(G_OBJECT (pdata->pixgc));
   g_object_unref(pdata->ac_tip);
   g_object_unref(pdata->progress_tip);
   g_object_unref(pdata->progressy_tip);
   g_object_unref(pdata->testpixgc);
   gdk_pixmap_unref(pdata->pixmap);
   gdk_pixmap_unref(pdata->pixmapy);
   gdk_pixmap_unref(pdata->pixbuffer);
   gdk_bitmap_unref(pdata->mask);
   gdk_bitmap_unref(pdata->masky);
   gdk_bitmap_unref(pdata->pixmask);

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

    egg_help_display_on_screen (
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
  /* FIXME: use (egg|gnome)_screen_help_display
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
		      msg = g_strdup_printf(_("An error occured while launching the Suspend command, the command returned \"%d\"\nPlease try to correct this error"), shell_ret);
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
destroy_about (GtkWidget *w, gpointer data)
{
   ProgressData *battstat = data;
   
   if (DEBUG) g_print("destroy_about()\n");
   
   battstat->about_box = NULL;
   
   return;
}

void
about_cb (BonoboUIComponent *uic,
	  ProgressData      *battstat,
	  const char        *verb)
{
   GtkWidget   *about_box;
   GdkPixbuf   *pixbuf;
   GError      *error = NULL;
   gchar       *file;
   
   const gchar *authors[] = {
	/* if your charset supports it, please replace the "o" in
	 * "Jorgen" into U00F6 */
	_("Jorgen Pehrson <jp@spektr.eu.org>"), 
	"Lennart Poettering <lennart@poettering.de> (Linux ACPI support)",
	"Seth Nickell <snickell@stanford.edu> (GNOME2 port)",
	NULL
   };

   const gchar *documenters[] = {
	NULL
   };

   const gchar *translator_credits = _("translator_credits");
   
   file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "battstat.png", FALSE, NULL);
   pixbuf = gdk_pixbuf_new_from_file (file, &error);
   g_free (file);
   
   if (error) {
   	g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
	g_error_free (error);
   }
   
   about_box = gnome_about_new (
				/* The long name of the applet in the About dialog.*/
				_("Battery Charge Monitor"), 
				VERSION,
				_("(C) 2000 The Gnulix Society, (C) 2002 Free Software Foundation"),
				_("This utility shows the status of your laptop battery."),
				authors,
				documenters,
				strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
				pixbuf);
   
   if (pixbuf)
	g_object_unref (pixbuf);

   gtk_window_set_wmclass (GTK_WINDOW (about_box), "battery charge monitor", "Batter Charge Monitor");
   gtk_window_set_screen (GTK_WINDOW (about_box),
			  gtk_widget_get_screen (battstat->applet));
   gtk_widget_show (about_box);
}

void
change_orient (PanelApplet       *applet,
	       PanelAppletOrient  orient,
	       ProgressData      *battstat)
{
   gchar new_label[80];
   guint acline_status;
   guint batt_state;
   guint batt_life;
   gchar *status[]={
      /* The following four messages will be displayed as tooltips over
       the battery meter.  
       High = The APM BIOS thinks that the battery charge is High.*/
      gettext_noop ("High"),
      /* Low = The APM BIOS thinks that the battery charge is Low.*/
      gettext_noop ("Low"),
      /* Critical = The APM BIOS thinks that the battery charge is Critical.*/
      gettext_noop ("Critical"),
      /* Charging = The APM BIOS thinks that the battery is recharging.*/
      gettext_noop ("Charging")};

   battstat->orienttype = orient;
   
   if (DEBUG) g_print("change_orient()\n");

   apm_readinfo(PANEL_APPLET (applet));
#ifdef __FreeBSD__
   acline_status = apminfo.ai_acline ? 1 : 0;
   batt_state = apminfo.ai_batt_stat;
   batt_life = apminfo.ai_batt_life;
#elif __OpenBSD__
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
   if(batt_state > 3) batt_state = 0;
   if(batt_life > 100) batt_life = 100;

   switch(battstat->orienttype) {
    case PANEL_APPLET_ORIENT_UP:
      if(battstat->panelsize<40)
	battstat->horizont=TRUE;
      else
	battstat->horizont=FALSE;
      break;
    case PANEL_APPLET_ORIENT_DOWN:
      if(battstat->panelsize<40)
	battstat->horizont=TRUE;
      else
	battstat->horizont=FALSE;
      break;
    case PANEL_APPLET_ORIENT_LEFT:
      battstat->horizont=FALSE;
      break;
    case PANEL_APPLET_ORIENT_RIGHT:
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
      battstat->showpercent ?
	gtk_widget_show (battstat->percent):gtk_widget_hide (battstat->percent);
      battstat->showbattery ?
	gtk_widget_show (battstat->pixmapwid):gtk_widget_hide (battstat->pixmapwid);
      (battstat->showbattery || battstat->showpercent) ?
	gtk_widget_show (battstat->framebattery):gtk_widget_hide (battstat->framebattery);
      
      if (battstat->showbattery == 0 && battstat->showpercent == 0) {
	 if(acline_status == 0) {
	    snprintf(new_label, sizeof(new_label),
		     /* This string will display as a tooltip over the status frame
		      when the computer is using battery power and the battery meter
		      and percent meter is hidden by the user.*/
		     _("System is running on battery power. Battery: %d%% (%s)"),
		     batt_life, _(status[batt_state]));
	 } else {
	    snprintf(new_label, sizeof(new_label),
		     /* This string will display as a tooltip over the status frame
		      when the computer is using AC power and the battery meter
		      and percent meter is hidden by the user.*/
		     _("System is running on AC power. Battery: %d%% (%s)"),
		     batt_life, _(status[batt_state]));      
	 }
	 gtk_tooltips_set_tip (battstat->ac_tip,
			       battstat->eventstatus,
			       new_label,
			       NULL);
      } else {
	 if(acline_status == 0) {
	    /* 0 = Battery power */
	    snprintf(new_label, sizeof(new_label),
		     /* This string will display as a tooltip over the status frame
		      when the computer is using battery power.*/
		     _("System is running on battery power"));
	 } else {
	    /* 1 = AC power. I should really test it explicitly here. */
	    snprintf(new_label, sizeof(new_label),
		     /* This string will display as a tooltip over the status frame
		      when the computer is using AC power.*/
		     _("System is running on AC power"));
	 }
	 gtk_tooltips_set_tip (battstat->ac_tip,
			       battstat->eventstatus,
			       new_label,
			       NULL);
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
      battstat->showpercent ?
	gtk_widget_show (battstat->statuspercent):gtk_widget_hide (battstat->statuspercent);
      battstat->showbattery ?
	gtk_widget_show (battstat->frameybattery):gtk_widget_hide (battstat->frameybattery);
#ifdef FIXME
      battstat->showpercent ?
	gtk_widget_set_usize(battstat->framestatus,width,24):
      gtk_widget_set_usize(battstat->framestatus, 20, 24);
#endif      
      if(battstat->showstatus == 0 && battstat->showpercent == 1) {
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
      if(battstat->showbattery == 0 && battstat->showpercent == 0) {
	 if(acline_status == 0) {
	    /* 0 = Battery power */
	    snprintf(new_label, sizeof(new_label),
		     /* This string will display as a tooltip over the status frame
		      when the computer is using battery power and the battery meter
		      and percent meter is hidden by the user.*/
		     _("System is running on battery power\nBattery: %d%% (%s)"),
		     batt_life, _(status[batt_state]));
	 } else {
	    /* 1 = AC power. I should really test it explicitly here. */
	    snprintf(new_label, sizeof(new_label),
		     /* This string will display as a tooltip over the status frame
		      when the computer is using AC power and the battery meter
		      and percent meter is hidden by the user.*/
		     _("System is running on AC power\nBattery: %d%% (%s)"),
		     batt_life, _(status[batt_state]));
	 }
	 gtk_tooltips_set_tip (battstat->ac_tip,
			       battstat->eventstatus,
			       new_label,
			       NULL);
      } else {
	 if(acline_status == 0) {
	    /* 0 = Battery power */
	    snprintf(new_label, sizeof(new_label),
		     /* This string will display as a tooltip over the status frame
		      when the computer is using battery power.*/
		     _("System is running on battery power"));
	 } else {
	    /* 1 = AC power. I should really test it explicitly here. */
	    snprintf(new_label, sizeof(new_label),
		     /* This string will display as a tooltip over the status frame
		      when the computer is using AC power.*/
		     _("System is running on AC power"));
	 }
	 gtk_tooltips_set_tip (battstat->ac_tip,
			       battstat->eventstatus,
			       new_label,
			       NULL);
      }
   }
   
   return;
}


void
font_set_cb (GtkWidget *ignored, int page, gpointer data)
{
   ProgressData *battstat = data;
   
   if (DEBUG) g_print("font_set_cb()\n");
   battstat->font_changed = TRUE;
   gnome_property_box_changed (GNOME_PROPERTY_BOX (battstat->prop_win));
   
   return;
}

void
simul_cb (GtkWidget *ignored, gpointer data)
{
   ProgressData *battery = data;
   gchar new_label[40];
   GdkColor *color, *darkcolor;
   gint slidervalue;
   gint prefred, preforange, prefyellow, progress_value, i, x;

   slidervalue=(int)GTK_ADJUSTMENT (battery->testadj)->value;
   prefyellow = (int) GTK_ADJUSTMENT (battery->eyellow_adj)->value;
   preforange =(int) GTK_ADJUSTMENT (battery->eorange_adj)->value;
   prefred =(int) GTK_ADJUSTMENT (battery->ered_adj)->value;
   
   snprintf(new_label,
	    sizeof(new_label),
	    "%d%%",
	    slidervalue);
   gtk_label_set_text ( GTK_LABEL (battery->testpercent), new_label);
   
   if (slidervalue <= prefred) {
      color=red;
      darkcolor=darkred;
   } else if (slidervalue <= preforange) {
      color=orange;
      darkcolor=darkorange;
   } else if (slidervalue<= prefyellow) {
      color=yellow;
      darkcolor=darkyellow;
   } else {
      color=green;
      darkcolor=darkgreen;
   }
   
   gdk_draw_pixmap(battery->testpixmap,
		   battery->testpixgc,
		   battery->testpixbuffer,
		   0,0,
		   0,0,
		   -1,-1);
   if(battery->draintop) {
      progress_value = PROGLEN*slidervalue/100.0;
      
      for(i=0; color[i].pixel!=-1; i++) {
	 gdk_gc_set_foreground(battery->testpixgc, &color[i]);
	 gdk_draw_line (battery->testpixmap,
			battery->testpixgc,
			pixel_offset_top[i], i+2,
			pixel_offset_top[i]+progress_value, i+2);
      }
   } else {
      progress_value = PROGLEN*slidervalue/100.0;
      
      for(i=0; color[i].pixel!=-1; i++) {
	 gdk_gc_set_foreground(battery->testpixgc, &color[i]);
	 gdk_draw_line (battery->testpixmap,
			battery->testpixgc,
			pixel_offset_bottom[i], i+2,
			pixel_offset_bottom[i]-progress_value, i+2);
      }
      
      
      for(i=0; darkcolor[i].pixel!=-1; i++) {
	 x=(pixel_offset_bottom[i]-progress_value)-pixel_top_length[i];
	 if(x < pixel_offset_top[i]) {
	    x = pixel_offset_top[i];
	 }
	 if(progress_value < 33) {
	    gdk_gc_set_foreground(battery->testpixgc, &darkcolor[i]);
	    
	    gdk_draw_line (battery->testpixmap,
			   battery->testpixgc,
			   (pixel_offset_bottom[i]-progress_value)-1, i+2,
			   x, i+2);
	 }
      }
   }
   gtk_widget_queue_draw (GTK_WIDGET (battery->testpixmapwid));
}

void
adj_value_changed_cb (GtkAdjustment *ignored, gpointer data)
{
   ProgressData *battstat = data;
   PanelApplet *applet = PANEL_APPLET (battstat->applet);
   
   GTK_ADJUSTMENT (battstat->ered_adj)->upper=(int)GTK_ADJUSTMENT (battstat->eorange_adj)->value-1;
   GTK_ADJUSTMENT (battstat->eorange_adj)->lower=(int)GTK_ADJUSTMENT (battstat->ered_adj)->value+1;
   GTK_ADJUSTMENT (battstat->eorange_adj)->upper=(int)GTK_ADJUSTMENT (battstat->eyellow_adj)->value-1;
   GTK_ADJUSTMENT (battstat->eyellow_adj)->lower=(int)GTK_ADJUSTMENT (battstat->eorange_adj)->value+1;
   
   battstat->yellow_val = GTK_ADJUSTMENT (battstat->eyellow_adj)->value;
   battstat->orange_val = GTK_ADJUSTMENT (battstat->eorange_adj)->value;
   battstat->red_val = GTK_ADJUSTMENT (battstat->ered_adj)->value;
   panel_applet_gconf_set_int (applet, "red_value", 
   			       battstat->red_val, NULL);
   panel_applet_gconf_set_int (applet, "orange_value",
   			       battstat->orange_val,  NULL);
   panel_applet_gconf_set_int (applet, "yellow_value", 
   			       battstat->yellow_val, NULL);
   battstat->colors_changed = TRUE;
   simul_cb(NULL, battstat);
   pixmap_timeout (battstat);
   battstat->colors_changed = FALSE;
}


void
change_size(PanelApplet *applet, gint size, gpointer data)
{
   ProgressData *battstat = data;
   
   if (DEBUG) g_print("applet_change_pixel_size()\n");
   
   battstat->panelsize=size;
   
   battstat->colors_changed=TRUE;
   change_orient(applet, battstat->orienttype, battstat);
   pixmap_timeout( battstat );
   battstat->colors_changed=FALSE;
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


#define GCONF_PATH ""

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
  battstat->showpercent = panel_applet_gconf_get_bool (applet, GCONF_PATH "show_percent", NULL);
  battstat->suspend_cmd = panel_applet_gconf_get_string (applet, GCONF_PATH "suspend_command", NULL);
  
}

gint
create_layout(ProgressData *battstat)
{
   int i;
   
   if (DEBUG) g_print("create_layout()\n");
   
   battstat->framestatus = gtk_frame_new(NULL);
   /*gtk_widget_set_usize( battstat->framestatus, 20, 24);*/
   gtk_frame_set_shadow_type ( GTK_FRAME (battstat->framestatus), 
			       GTK_SHADOW_NONE);
   gtk_widget_show(battstat->framestatus);
   gtk_box_pack_start( GTK_BOX(battstat->hbox1), battstat->framestatus, 
		       FALSE, TRUE, 0);
   
   battstat->eventstatus = gtk_event_box_new ();
   gtk_widget_show (battstat->eventstatus);
   gtk_container_add (GTK_CONTAINER (battstat->framestatus), 
		      battstat->eventstatus);
   
   battstat->statusvbox = gtk_vbox_new ( FALSE, 5);
   gtk_widget_show (battstat->statusvbox);
   gtk_container_add(GTK_CONTAINER(battstat->eventstatus), 
		     battstat->statusvbox);
   
   battstat->framebattery = gtk_frame_new(NULL);
   /*gtk_widget_set_usize( battstat->framebattery, -1, 24);*/
   gtk_frame_set_shadow_type ( GTK_FRAME (battstat->framebattery), 
			       GTK_SHADOW_NONE);
   gtk_widget_show(battstat->framebattery);
   gtk_box_pack_start(GTK_BOX(battstat->hbox1), battstat->framebattery, 
		      FALSE, TRUE, 0);
   
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
   
   battstat->pixmapwid = gtk_pixmap_new( battstat->pixmap,
					 battstat->mask );
   battstat->pixmapwidy = gtk_pixmap_new( battstat->pixmapy,
					  battstat->masky );
   gtk_box_pack_start (GTK_BOX (battstat->hbox), GTK_WIDGET (battstat->pixmapwid), FALSE, TRUE, 0);
   gtk_widget_show ( GTK_WIDGET (battstat->pixmapwid) );

   battstat->frameybattery = gtk_frame_new(NULL);
   /*gtk_widget_set_usize( battstat->frameybattery, 24, 46);*/
   gtk_frame_set_shadow_type ( GTK_FRAME (battstat->frameybattery), GTK_SHADOW_NONE);
   gtk_box_pack_start(GTK_BOX(battstat->hbox1), battstat->frameybattery, FALSE, TRUE, 0);
   battstat->eventybattery = gtk_event_box_new();
   gtk_widget_show (battstat->eventybattery);
   gtk_container_add (GTK_CONTAINER (battstat->frameybattery), battstat->eventybattery);
   gtk_container_add (GTK_CONTAINER (battstat->eventybattery),
                      GTK_WIDGET (battstat->pixmapwidy));

   gtk_widget_show ( GTK_WIDGET (battstat->pixmapwidy) );
   
   gtk_signal_connect(GTK_OBJECT(battstat->pixmapwid), "destroy",
		      GTK_SIGNAL_FUNC(destroy_applet), battstat);
   
   battstat->percent = gtk_label_new ("0%");
   gtk_widget_show(battstat->percent);
   gtk_box_pack_start (GTK_BOX(battstat->hbox), battstat->percent, FALSE, TRUE, 0);
   
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
   battstat->statuspixmapwid = gtk_pixmap_new( battstat->status, statusmask[BATTERY] );
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

   gtk_signal_connect(GTK_OBJECT(battstat->applet),"change_orient",
		      GTK_SIGNAL_FUNC(change_orient),
		      battstat);
   g_signal_connect (G_OBJECT (battstat->applet), "change_size",
   		     G_CALLBACK (change_size), battstat);
   		     
   g_signal_connect (battstat->applet, "button_press_event",
   			     G_CALLBACK (button_press_cb), battstat);
   g_signal_connect (battstat->applet, "key_press_event",
   			     G_CALLBACK (key_press_cb), battstat);

   return FALSE;
}

static gboolean
battstat_applet_fill (PanelApplet *applet)
{
  ProgressData *battstat;

  if (DEBUG) g_print("main()\n");
  
  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/battstat.png");
  
  panel_applet_add_preferences (applet, "/schemas/apps/battstat-applet/prefs", NULL);
  panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

  apm_readinfo (applet);
  
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
  
  glade_gnome_init ();
  
  load_preferences(battstat);
  create_layout(battstat);

  pixmap_timeout(battstat);
  change_orient (applet, battstat->orienttype, battstat );

  battstat->pixtimer = gtk_timeout_add (1000, pixmap_timeout, battstat);

  gtk_container_add (GTK_CONTAINER (battstat->applet), battstat->hbox1);

  panel_applet_setup_menu_from_file (PANEL_APPLET (battstat->applet), 
  			             NULL,
                                     "GNOME_BattstatApplet.xml",
                                     NULL,
                                     battstat_menu_verbs,
                                     battstat);

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
      

