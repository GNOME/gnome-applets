/* battstat        A GNOME battery meter for laptops. 
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

#ifdef __FreeBSD__
#include <machine/apm_bios.h>
#elif __OpenBSD__
#include <sys/param.h>
#include <machine/apmvar.h>
#elif __linux__
#include <apm.h>
#endif

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <gnome.h>
#include <applet-widget.h>
#include <status-docklet.h>

#include "battstat.h"
#include "pixmaps.h"

#ifndef gettext_noop
#define gettext_noop(String) (String)
#endif

GtkObject *statusdock;

guint pixmap_type=-1;
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
apm_readinfo(void)
{
  /* This is how I read the information from the APM subsystem under
     FreeBSD.  Each time this functions is called (once every second)
     the APM device is opened, read from and then closed.
  */
  int fd;
  if (DEBUG) g_print("apm_readinfo() (FreeBSD)\n");

  fd = open(APMDEVICE, O_RDONLY);
  if (fd == -1) cleanup(1);

  if (ioctl(fd, APMIO_GETINFO, &apminfo) == -1)
    err(1, "ioctl(APMIO_GETINFO)");

  close(fd);
}
#elif __OpenBSD__
void
apm_readinfo(void)
{
  /* Code for OpenBSD by Joe Ammond <jra@twinight.org>. Using the same
     procedure as for FreeBSD.
  */
  int fd;
  if (DEBUG) g_print("apm_readinfo() (OpenBSD)\n");

  fd = open(APMDEVICE, O_RDONLY);
  if (fd == -1) cleanup(1);
  if (ioctl(fd, APM_IOC_GETPOWER, &apminfo) == -1)
    err(1, "ioctl(APM_IOC_GETPOWER)");
  close(fd);
}
#elif __linux__
void
apm_readinfo(void)
{
  /* Code for Linux by Thomas Hood <jdthood@mail.com>. apm_read() will
     read from /proc/... instead and we do not need to open the device
     ourselves.
  */
  if (DEBUG) g_print("apm_readinfo() (Linux)\n");
  apm_read(&apminfo);
}
#else
void
apm_readinfo(void)
{
  if (DEBUG) g_print("apm_readinfo() (Generic)\n Your platform is not supported!\n");
  if (DEBUG) g_print("The applet will not work properly (if at all).\n");

  return;
}
#endif 

void
load_font (gpointer data)
{
  ProgressData *battstat = data;

  if (DEBUG) g_print("load_font()\n");

  if (battstat->font_changed) {
    battstat->percentstyle=gtk_style_copy (GTK_WIDGET (battstat->percent)->style);
    if (battstat->fontname)
      (battstat->percentstyle)->font=gdk_font_load(battstat->fontname);
    else
      (battstat->percentstyle)->font=gdk_font_load ("fixed");
    gtk_widget_set_style (battstat->percent, battstat->percentstyle);
    gtk_widget_set_style (battstat->statuspercent, battstat->percentstyle);
  }
}

void
draw_meter ( gpointer battdata, gpointer meterdata ) 
{

}

gint
pixmap_timeout( gpointer data )
{
  ProgressData *battery = data;
  MeterData *meter;
  GdkColor *color, *darkcolor;
  static guint flash=FALSE;
  static guint lastapm=1000;
  static guint lastac=1000;
  static guint lastacstatus=1000;
  guint pix_type_new=1000;
  guint progress_value;
  gint batt_life;
  gint batt_time;
  gint ac_status;
  gint i, x;
  gboolean batterypresent=FALSE;
  gchar new_label[80];
  gchar new_string[80];
  gchar *status[]={
    gettext_noop ("High"),
    gettext_noop ("Low"),
    gettext_noop ("Critical"),
    gettext_noop ("Charging")
  };

  if (DEBUG) g_print("pixmap_timeout()\n");

  apm_readinfo();
#ifdef __FreeBSD__
  pix_type_new = apminfo.ai_acline;
  batt_life = apminfo.ai_batt_life;
  ac_status = apminfo.ai_batt_stat;
  if(pix_type_new < 0) pix_type_new = 0;
  if(batt_life > 100) batt_life = 0;
  if(ac_status == 255) {
    ac_status = 0;
    batterypresent = FALSE;
  } else {
    batterypresent = TRUE;
  }
#elif __OpenBSD__
  pix_type_new = apminfo.ac_state;
  batt_life = apminfo.battery_life;
  ac_status = apminfo.battery_state;
  if(pix_type_new < 0) pix_type_new = 0;
  if(batt_life > 100) batt_life = 0;
  if(ac_status == APM_AC_ON) {
    ac_status = 0;
    batterypresent = FALSE;
  } else {
    batterypresent = TRUE;
  }
#elif __linux__
  pix_type_new = apminfo.ac_line_status;
  batt_life = apminfo.battery_percentage;
  ac_status = apminfo.battery_status;
  batt_time = apminfo.battery_time;

  if(pix_type_new == 255) pix_type_new = 0;
  if(batt_life < 0) batt_life = 0;
  if(ac_status == 255) {
    ac_status = 0;
    batterypresent = FALSE;
  } else {
    batterypresent = TRUE;
  }
#else
  pix_type_new = 1;
  batt_life = 100;
  ac_status = 0;
  batterypresent = TRUE;
#endif
  
  if (pix_type_new != pixmap_type) {
    pixmap_type=pix_type_new;
    if (pixmap_type==BATTERY) {
      if (batt_life <= battery->red_val) {
	pixmap_type=WARNING;
	gtk_pixmap_set(GTK_PIXMAP (battery->statuspixmapwid),
		       statusimage[WARNING], statusmask[WARNING]);
	gtk_pixmap_set(GTK_PIXMAP (battery->pixmapdockwid),
		       statusimage[WARNING], statusmask[WARNING]);
      } else if (batt_life > battery->red_val) {
	pixmap_type=BATTERY;
	gtk_pixmap_set(GTK_PIXMAP (battery->statuspixmapwid),
		       statusimage[BATTERY], statusmask[BATTERY]);
	gtk_pixmap_set(GTK_PIXMAP (battery->pixmapdockwid),
		       statusimage[BATTERY], statusmask[BATTERY]);
      }
    } else {
      pixmap_type=AC;
      gtk_pixmap_set(GTK_PIXMAP (battery->statuspixmapwid),
		     statusimage[AC], statusmask[AC]);
      gtk_pixmap_set(GTK_PIXMAP (battery->pixmapdockwid),
		     statusimage[AC], statusmask[AC]);
    }
  }

  if(ac_status == 3 || flash == TRUE) {
    flash^=-1;
    if(flash) {
      pixmap_type=FLASH;
    } else {
      pixmap_type=AC;
    }
    gtk_pixmap_set(GTK_PIXMAP (battery->statuspixmapwid),
		   statusimage[pixmap_type], statusmask[pixmap_type]);
    gtk_pixmap_set(GTK_PIXMAP (battery->pixmapdockwid),
		   statusimage[pixmap_type], statusmask[pixmap_type]);

  } else if (ac_status != 3 && pixmap_type == FLASH) {
    pixmap_type=AC;
    gtk_pixmap_set(GTK_PIXMAP (battery->statuspixmapwid),
		   statusimage[AC], statusmask[AC]);
    gtk_pixmap_set(GTK_PIXMAP (battery->pixmapdockwid),
		   statusimage[AC], statusmask[AC]);

  }

  if(ac_status != lastac) {
    if(battery->showbattery == 0 && battery->showpercent == 0) {
      snprintf(new_label, sizeof(new_label),
	       _("System is running on %s power\nBattery: %d%% (%s)"),
	       pix_type_new?_("AC"):_("battery"), batt_life, _(status[ac_status]));
    } else {
      snprintf(new_label, sizeof(new_label),_("System is running on %s power"),
	       pix_type_new?_("AC"):_("battery"));
    }
    gtk_tooltips_set_tip (battery->ac_tip,
			  battery->eventstatus,
			  new_label,
			  NULL);
  }
  lastac = ac_status;

  if(batt_life != lastapm || battery->colors_changed) {
    if (batterypresent) 
      snprintf(new_label, sizeof(new_label),"%d%%", batt_life);
    else
      snprintf(new_label, sizeof(new_label),"N/A");

    gtk_label_set_text ( GTK_LABEL (battery->percent), new_label);
    gtk_label_set_text ( GTK_LABEL (battery->statuspercent), new_label);

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
	  if(batt_life > 0) {
	    gdk_draw_line (battery->pixmap,
			   battery->pixgc,
			   pixel_offset_top[i], i+2,
			   pixel_offset_top[i]+progress_value, i+2);
	  }
	}
	else {
	  if(batt_life > 0) {
	    gdk_draw_line (battery->pixmapy,
			   battery->pixgc,
			   i+2, pixel_offset_top[i],
			   i+2, pixel_offset_top[i]+progress_value);
	  }
	}
      }

      if(battery->horizont)
	gdk_draw_pixmap(battery->pixmapwid->window,
			battery->pixgc,
			battery->pixmap,
			0,0,
			0,2,
			-1,-1);
      else
	gdk_draw_pixmap(battery->pixmapwidy->window,
			battery->pixgc,
			battery->pixmapy,
			0,0,
			2,1,
			-1,-1);
    } else {
      progress_value = PROGLEN*batt_life/100.0;

      for(i=0; color[i].pixel!=-1; i++) {
	gdk_gc_set_foreground(battery->pixgc, &color[i]);
	if(battery->horizont) {
	  if(batt_life > 0) {
	    gdk_draw_line (battery->pixmap,
			   battery->pixgc,
			   pixel_offset_bottom[i], i+2,
			   pixel_offset_bottom[i]-progress_value, i+2);
	  }
	}
	else {
	  if(batt_life > 0) {
	    gdk_draw_line (battery->pixmapy,
			   battery->pixgc,
			   i+2, pixel_offset_bottom[i]-1,
			   i+2, pixel_offset_bottom[i]-progress_value);
	  }
	}
      }
      for(i=0; darkcolor[i].pixel!=-1; i++) {
	x=(pixel_offset_bottom[i]-progress_value)-pixel_top_length[i];
	if(x < pixel_offset_top[i]) {
	  x = pixel_offset_top[i];
	}
	if(batt_life > 0) {
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

      if(battery->horizont)
	gdk_draw_pixmap(battery->pixmapwid->window,
			battery->pixgc,
			battery->pixmap,
			0,0,
			0,2,
			-1,-1);
      else
	gdk_draw_pixmap(battery->pixmapwidy->window,
			battery->pixgc,
			battery->pixmapy,
			0,0,
			2,1,
			-1,-1);
    }

    if(pix_type_new == 0 && batt_life <= battery->red_val) {
      if(battery->lowbattnotification) {
	snprintf(new_label, sizeof(new_label),(_("Battery low (%d%%) and AC is %s")),
		 batt_life, pix_type_new?(_("online")):(_("offline")));
	gnome_warning_dialog(new_label);
	
	if(battery->beep)
	  gdk_beep();
	
	battery->lowbattnotification=FALSE;
      }
      gnome_triggers_do ("", NULL, "battstat_applet", "LowBattery", NULL);
    }

    if (battery->showstatus == 0) {
      if (batterypresent)
	snprintf(new_string, sizeof(new_string), (_("System is running on %s power. Battery: %d%% (%s)")),
		 pix_type_new?_("AC"):_("battery"),
		 batt_life,
		 _(status[ac_status]));
      else {
	snprintf(new_string, sizeof(new_string), (_("System is running on %s power. Battery: Not present")),
		 
		 pix_type_new?_("AC"):_("battery"));
      }
    } else {
      if (batterypresent) 
	snprintf(new_string, sizeof(new_string), (_("Battery: %d%% (%s)")),
		 batt_life,
		 _(status[ac_status]));
      else {
	snprintf(new_string, sizeof(new_string), (_("Battery: Not present")));
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

    lastapm=batt_life;
  }
  /* pix_type_new: AC line status, 0=AC not used, 1=AC online 
   */
  if(ac_status != lastacstatus && ac_status != 3 && pix_type_new != 0) {
    gnome_triggers_do ("", NULL, "battstat_applet", "BatteryFull", NULL);
    if(battery->fullbattnot) {
      gnome_ok_dialog(_("Battery is now fully re-charged!"));
      if (battery->beep)
	gdk_beep();
    }
    lastacstatus=ac_status;
  }

  return(TRUE);
}

void
destroy_applet( GtkWidget *widget, gpointer data )
{
  ProgressData *pdata = data;

  if (DEBUG) g_print("destroy_applet()\n");

  gtk_timeout_remove (pdata->pixtimer);
  pdata->pixtimer = 0;
  pdata->applet = NULL;
  gdk_pixmap_unref(pdata->pixmap);
  gdk_pixmap_unref(pdata->pixmapy);
  gdk_pixmap_unref(pdata->pixbuffer);
  gdk_bitmap_unref(pdata->mask);
  gdk_bitmap_unref(pdata->masky);
  gdk_bitmap_unref(pdata->pixmask);
  g_free(pdata);

  return;
  widget = NULL;
}

void
cleanup(int status)
{
  if (DEBUG) g_print("cleanup()\n");

  switch (status) {
  case 1:
    g_error (_("Can't open the APM device!\n\n"
	       "Make sure you have read permission to the\n"
	       "APM device."));
    exit(1);
    break;
  case 2:
    g_error (_("The APM Management subsystem seems to be disabled.\n"
	       "Try executing \"apm -e 1\" (FreeBSD) and see if \n"
	       "that helps.\n"));
    gtk_main_quit();
    break;
  }
}

void
help_cb (AppletWidget *applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry = {
    "battstat_applet", "index.html"
  };
  gnome_help_display (NULL, &help_entry);
}

void
helppref_cb (AppletWidget *applet, gpointer data)
{
  GnomeHelpMenuEntry help_entry = {
    "battstat_applet", "index.html#BATTSTAT_PREFS"
  };
  gnome_help_display (NULL, &help_entry);
}

void
suspend_cb (AppletWidget *applet, gpointer data)
{
  ProgressData *battstat = data;

  if(battstat->suspend_cmd && strlen(battstat->suspend_cmd)>0) {
    gnome_execute_shell(NULL, battstat->suspend_cmd);
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
  w = NULL;
}

void
about_cb (AppletWidget *widget, gpointer data)
{
  ProgressData *battstat = data;
  char *authors[7];
 
  if (DEBUG) g_print("about_cb()\n");

  if (battstat->about_box != NULL) {
    gdk_window_show(battstat->about_box->window);
    gdk_window_raise(battstat->about_box->window);
    return;
  }
  authors[0] = "Jörgen Pehrson <jp@spektr.eu.org>";
  authors[1] = "Patrik Grip-Jansson <patrikj@gnulix.org>";
  authors[2] = "Cezary Jackiewicz <cjackiewicz@poczta.onet.pl>";
  authors[3] = "Joe Ammond <jra@twinight.org>";
  authors[4] = "Mikael Hallendal <micke@codefactory.se>";
  authors[5] = "Thomas Hood <jdthood@mail.com>";
  authors[6] = NULL;

  battstat->about_box =
    gnome_about_new (_("Battery status utility"), VERSION,
                     _(" (C) 2000 The Gnulix Society"),
                     (const char **) authors,
		     _("This utility show the status of your laptop battery."),
                     "battstat-tesla.xpm");

  gtk_signal_connect (GTK_OBJECT (battstat->about_box), "destroy",
                      GTK_SIGNAL_FUNC (destroy_about), battstat);

  gtk_widget_show (battstat->about_box);
  return;
  widget = NULL;
}


gint
applet_save_session(GtkWidget *w,
		    char *privcfgpath,
		    char *globcfgpath,
		    gpointer data)
{
  ProgressData *battstat = data;

  if (DEBUG) g_print("applet_save_session()\n");

  gnome_config_push_prefix (privcfgpath);
  gnome_config_set_int ("batt/red_val", battstat->red_val);
  gnome_config_set_int ("batt/orange_val", battstat->orange_val);
  gnome_config_set_int ("batt/yellow_val", battstat->yellow_val);
  gnome_config_set_string ("batt/fontname", battstat->fontname);
  gnome_config_set_bool ("batt/lowbattnotification", battstat->lowbattnotification);
  gnome_config_set_bool ("batt/fullbattnot", battstat->fullbattnot);
  gnome_config_set_bool ("batt/beep", battstat->beep);
  gnome_config_set_bool ("batt/draintop", battstat->draintop);
  gnome_config_set_bool ("batt/horizont", battstat->horizont);
  gnome_config_set_bool ("batt/showstatus", battstat->showstatus);
  gnome_config_set_bool ("batt/showbattery", battstat->showbattery);
  gnome_config_set_bool ("batt/showpercent", battstat->showpercent);
  gnome_config_set_string ("batt/suspendcommand", battstat->suspend_cmd);
  gnome_config_set_bool ("batt/usedock", battstat->usedock);
  gnome_config_set_bool ("batt/own_font", battstat->own_font);
  gnome_config_pop_prefix ();

  gnome_config_sync();
  gnome_config_drop_all ();

  return FALSE;
  w = NULL;
}

void
change_orient (GtkWidget *w, PanelOrientType o, gpointer data)
{
  ProgressData *battstat = data;
  gchar new_label[80];
  guint batt_life;
  guint pix_type_new=1000;
  guint batterypresent = FALSE;
  guint ac_status;
  gchar *status[]={
    gettext_noop ("High"),
    gettext_noop ("Low"),
    gettext_noop ("Critical"),
    gettext_noop ("Charging")};
  int width;
  battstat->orienttype=o;

  if (DEBUG) g_print("change_orient()\n");

  apm_readinfo();
#ifdef __FreeBSD__
  pix_type_new = apminfo.ai_acline;
  batt_life = apminfo.ai_batt_life;
  ac_status = apminfo.ai_batt_stat;
  if(pix_type_new < 0) pix_type_new = 0;
  if(batt_life == 255) batt_life = 0;
  if(ac_status == 255) {
    ac_status = 0;
    batterypresent = FALSE;
  } else {
    batterypresent = TRUE;
  }
#elif __OpenBSD__
  pix_type_new = apminfo.ac_state;
  batt_life = apminfo.battery_life;
  ac_status = apminfo.battery_state;
  if(pix_type_new < 0) pix_type_new = 0;
  if(batt_life > 100) batt_life = 0;
  if(ac_status == APM_AC_ON) {
    ac_status = 0;
    batterypresent = FALSE;
  } else {
    batterypresent = TRUE;
  }
#elif __linux__
  pix_type_new = apminfo.ac_line_status;
  batt_life = apminfo.battery_percentage;
  ac_status = apminfo.battery_status;
  if(pix_type_new == 255) pix_type_new = 0;
  if(batt_life == -1) batt_life = 0;
  if(ac_status == 255) {
    ac_status = 0;
    batterypresent = FALSE;
  } else {
    batterypresent = TRUE;
  }
#else
  pix_type_new = 1;
  batt_life = 100;
  ac_status = 0;
  batterypresent = TRUE;
#endif

  switch(battstat->orienttype) {
  case ORIENT_UP:
    if(battstat->panelsize<40)
      battstat->horizont=TRUE;
    else
      battstat->horizont=FALSE;
    break;
  case ORIENT_DOWN:
    if(battstat->panelsize<40)
      battstat->horizont=TRUE;
    else
      battstat->horizont=FALSE;
    break;
  case ORIENT_LEFT:
    battstat->horizont=FALSE;
    break;
  case ORIENT_RIGHT:
    battstat->horizont=FALSE;
    break;
  }

  if (!(battstat->showbattery || battstat->showpercent || battstat->showstatus)) {
    gnome_error_dialog(_("You can't hide all elements of the applet!"));
    width=gdk_string_width((battstat->percentstyle)->font,"100%")+46+3;
    gtk_widget_set_usize(battstat->framestatus, 20, 24);
    gtk_widget_set_usize(battstat->framebattery, width, 24);
    gtk_widget_show (battstat->framebattery);
    gtk_widget_show (battstat->pixmapwid);
    gtk_widget_show (battstat->framestatus);
    gtk_widget_show (battstat->statuspixmapwid);
    gtk_widget_show (battstat->percent);
    gtk_widget_hide (battstat->statuspercent);
    gtk_widget_hide (battstat->frameybattery);
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (battstat->radio_lay_batt_on), TRUE);
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (battstat->radio_lay_status_on), TRUE);
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (battstat->radio_lay_percent_on), TRUE);
  } else if (battstat->horizont) {
    /* Horizontal mode */
    gtk_widget_hide (battstat->statuspercent);
    gtk_widget_hide (battstat->frameybattery);
    gtk_widget_show (battstat->framebattery);

    battstat->showstatus ?
      gtk_widget_show (battstat->framestatus):gtk_widget_hide (battstat->framestatus);
    battstat->showpercent ?
      gtk_widget_show (battstat->percent):gtk_widget_hide (battstat->percent);
    battstat->showbattery ?
      gtk_widget_show (battstat->pixmapwid):gtk_widget_hide (battstat->pixmapwid);
    (battstat->showbattery || battstat->showpercent) ?
      gtk_widget_show (battstat->framebattery):gtk_widget_hide (battstat->framebattery);

    if (battstat->showbattery == 0 && battstat->showpercent == 0) {
      snprintf(new_label, sizeof(new_label),
	       _("System is running on %s power. Battery: %d%% (%s)"),
	       pix_type_new?_("AC"):_("battery"), batt_life, _(status[ac_status]));
      gtk_tooltips_set_tip (battstat->ac_tip,
			    battstat->eventstatus,
			    new_label,
			    NULL);
    } else {
      snprintf(new_label, sizeof(new_label),
	       _("System is running on %s power."),
	       pix_type_new?_("AC"):_("battery"));
      gtk_tooltips_set_tip (battstat->ac_tip,
			    battstat->eventstatus,
			    new_label,
			    NULL);
      gtk_widget_set_usize(battstat->framestatus, 20, 24);
      if(battstat->showbattery == 1 && battstat->showpercent == 1) {
	width=gdk_string_width((battstat->percentstyle)->font,"100%")+46+3;
	gtk_widget_set_usize(battstat->framebattery, width, 24);
      } else if(battstat->showbattery == 0 && battstat->showpercent == 1) {
	width=gdk_string_width((battstat->percentstyle)->font,"100%")+6;
	gtk_widget_set_usize(battstat->framebattery, width, 24);
      } else if(battstat->showbattery == 1 && battstat->showpercent == 0) {
	gtk_widget_set_usize(battstat->framebattery, 46, 24);	
      }
    }
  } else {
    /* Square mode */
    width=gdk_string_width((battstat->percentstyle)->font,"100%")+6;

    gtk_widget_hide(battstat->framebattery);
    gtk_widget_show(battstat->frameybattery);
    battstat->showstatus ?
      gtk_widget_show (battstat->framestatus):gtk_widget_hide (battstat->framestatus);
    battstat->showpercent ?
      gtk_widget_show (battstat->statuspercent):gtk_widget_hide (battstat->statuspercent);
    battstat->showbattery ?
      gtk_widget_show (battstat->frameybattery):gtk_widget_hide (battstat->frameybattery);
    battstat->showpercent ?
      gtk_widget_set_usize(battstat->framestatus,width,24):
      gtk_widget_set_usize(battstat->framestatus, 20, 24);

    if(battstat->showstatus == 0 && battstat->showpercent == 1) {
      gtk_widget_show (battstat->framestatus);
      gtk_widget_hide (battstat->statuspixmapwid);
      gtk_widget_set_usize(battstat->framestatus, width, -1);
    }
    if(battstat->showstatus == 1 && battstat->showpercent == 0) {
      gtk_widget_set_usize(battstat->framestatus, 20, 24);
    }
    if(battstat->showstatus == 1 && battstat->showpercent == 1) {
      gtk_widget_set_usize(battstat->framestatus, width, 46);
    }
    if(battstat->showstatus == 1) {
      gtk_widget_show (battstat->statuspixmapwid);      
    }
    if(battstat->showbattery == 0 && battstat->showpercent == 0) {
      snprintf(new_label, sizeof(new_label),
	       _("System is running on %s power. Battery: %d%% (%s)"),
	       pix_type_new?_("AC"):_("battery"), batt_life, _(status[ac_status]));
      gtk_tooltips_set_tip (battstat->ac_tip,
			    battstat->eventstatus,
			    new_label,
			    NULL);
    } else {
      snprintf(new_label, sizeof(new_label),
	       _("System is running on %s power."),
	       pix_type_new?_("AC"):_("battery"));
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
  ignored = NULL;
  page = 0;
}

void
simul_cb(GtkWidget *ignored, gpointer data)
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
    
    gdk_draw_pixmap(battery->testpixmapwid->window,
		    battery->testpixgc,
		    battery->testpixmap,
		    0,0,
		    0,2,
		    -1,-1);
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
    gdk_draw_pixmap(battery->testpixmapwid->window,
		    battery->testpixgc,
		    battery->testpixmap,
		    0,0,
		    0,2,
		    -1,-1);

  }
  return;
  ignored = NULL;
}

void
adj_value_changed_cb ( GtkAdjustment *ignored, gpointer data )
{
  ProgressData *battstat = data;

  GTK_ADJUSTMENT (battstat->ered_adj)->upper=(int)GTK_ADJUSTMENT (battstat->eorange_adj)->value-1;
  GTK_ADJUSTMENT (battstat->eorange_adj)->lower=(int)GTK_ADJUSTMENT (battstat->ered_adj)->value+1;
  GTK_ADJUSTMENT (battstat->eorange_adj)->upper=(int)GTK_ADJUSTMENT (battstat->eyellow_adj)->value-1;
  GTK_ADJUSTMENT (battstat->eyellow_adj)->lower=(int)GTK_ADJUSTMENT (battstat->eorange_adj)->value+1;

  gnome_property_box_changed (GNOME_PROPERTY_BOX (battstat->prop_win));

  battstat->colors_changed = TRUE;
  simul_cb(NULL, battstat);
  battstat->colors_changed = FALSE;

  return;
}

void
toggle_value_changed_cb ( GtkToggleButton *ignored, gpointer data )
{
  ProgressData *battstat = data;

  if (DEBUG) g_print("toggle_value_changed()\n");

  battstat->draintop = (GTK_TOGGLE_BUTTON (battstat->progdir_radio))->active;
  battstat->lowbattnotification = (GTK_TOGGLE_BUTTON (battstat->lowbatt_toggle))->active;
  battstat->fullbattnot = (GTK_TOGGLE_BUTTON (battstat->full_toggle))->active;
  battstat->beep = (GTK_TOGGLE_BUTTON (battstat->beep_toggle))->active;
  battstat->showstatus = (GTK_TOGGLE_BUTTON (battstat->radio_lay_status_on))->active;
  battstat->showpercent = (GTK_TOGGLE_BUTTON (battstat->radio_lay_percent_on))->active;
  battstat->showbattery = (GTK_TOGGLE_BUTTON (battstat->radio_lay_batt_on))->active;
  battstat->own_font = (GTK_TOGGLE_BUTTON (battstat->font_toggle))->active;

  if(battstat->lowbattnotification || battstat->fullbattnot)
    gtk_widget_set_sensitive(GTK_WIDGET (battstat->beep_toggle), TRUE);
  else
    gtk_widget_set_sensitive(GTK_WIDGET (battstat->beep_toggle), FALSE);

  if(battstat->own_font) {
    gtk_widget_set_sensitive(GTK_WIDGET (battstat->fontlabel), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET (battstat->fontpicker), TRUE);
  } else {
    gtk_widget_set_sensitive(GTK_WIDGET (battstat->fontlabel), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET (battstat->fontpicker), FALSE);
  }
  gnome_property_box_changed (GNOME_PROPERTY_BOX (battstat->prop_win));
  return;
  ignored = NULL;
}


static void
build_our_plug(StatusDocklet *docklet, GtkWidget *plug, gpointer data)
{
  ProgressData *battstat = data;
  
  battstat->eventdock = gtk_event_box_new ();
  gtk_widget_show (battstat->eventdock);
  gtk_container_add (GTK_CONTAINER(plug), battstat->eventdock);

  battstat->pixmapdockwid = gtk_pixmap_new ( statusimage[BATTERY], statusmask[BATTERY] );

  if(battstat->usedock) {
    gtk_widget_show(battstat->eventdock);
    gtk_widget_show(battstat->pixmapdockwid);
  } else {
    gtk_widget_hide(battstat->pixmapdockwid);    
    gtk_widget_hide(battstat->eventdock);    
  }

  gtk_container_add(GTK_CONTAINER(battstat->eventdock), battstat->pixmapdockwid);

  battstat->st_tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (battstat->st_tip,
			battstat->eventdock,
			"Test",
			NULL);
  
  return;
}

#ifdef HAVE_PANEL_PIXEL_SIZE
void
applet_change_pixel_size(GtkWidget *w, int size, gpointer data)
{
  ProgressData *battstat = data;
  
  if (DEBUG) g_print("applet_change_pixel_size()\n");

  battstat->panelsize=size;

  battstat->colors_changed=TRUE;
  change_orient(w, battstat->orienttype, battstat);
  pixmap_timeout( battstat );
  battstat->colors_changed=FALSE;
}
#endif

static gint
load_preferences(gpointer data)
{
  ProgressData *battstat = data;

  if (DEBUG) g_print("load_preferences()\n");

  gnome_config_push_prefix(APPLET_WIDGET(battstat->applet)->privcfgpath);
  battstat->red_val = gnome_config_get_int("batt/red_val=15");
  battstat->orange_val=gnome_config_get_int("batt/orange_val=25");
  battstat->yellow_val=gnome_config_get_int("batt/yellow_val=40");
  battstat->fontname=gnome_config_get_string("batt/fontname=-adobe-helvetica-medium-r-normal-*-*-100-*-*-p-*-*-*");
  battstat->lowbattnotification=gnome_config_get_bool("batt/lowbattnotification=true");
  battstat->fullbattnot=gnome_config_get_bool("batt/fullbattnot=true");
  battstat->beep=gnome_config_get_bool("batt/beep=false");
  battstat->draintop=gnome_config_get_bool("batt/draintop=false");
  battstat->horizont=gnome_config_get_bool("batt/horizont=true");
  battstat->showstatus=gnome_config_get_bool("batt/showstatus=true");
  battstat->showbattery=gnome_config_get_bool("batt/showbattery=true");
  battstat->showpercent=gnome_config_get_bool("batt/showpercent=true");
  battstat->suspend_cmd=gnome_config_get_string("batt/suspendcommand=");
  battstat->usedock=gnome_config_get_bool("batt/usedock=false");
  battstat->own_font=gnome_config_get_bool("batt/own_font=false");
  gnome_config_pop_prefix();

  return FALSE;
}

gint
init_applet(int argc, char *argv[], gpointer data)
{
  ProgressData *battstat = data;

  if (DEBUG) g_print("init_applet()\n");
  applet_widget_init(PACKAGE, VERSION, argc, argv, NULL, 0, NULL);
  battstat->applet = applet_widget_new(PACKAGE);
  if (!battstat->applet) {
    g_error (_("Can't create applet!\n"));
    destroy_applet(battstat->applet, battstat);
  }
  battstat->font_changed=TRUE;
  return FALSE;
}

gint
create_layout(int argc, char *argv[], gpointer data)
{
  int i;
  ProgressData *battstat = data;

  if (DEBUG) g_print("create_layout()\n");

  battstat->hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show(battstat->hbox1);

  applet_widget_add (APPLET_WIDGET (battstat->applet), battstat->hbox1);

  battstat->framestatus = gtk_frame_new(NULL);
  gtk_widget_set_usize( battstat->framestatus, 20, 24);
  gtk_frame_set_shadow_type ( GTK_FRAME (battstat->framestatus), GTK_SHADOW_ETCHED_IN);
  gtk_widget_show(battstat->framestatus);
  gtk_box_pack_start( GTK_BOX(battstat->hbox1), battstat->framestatus, FALSE, TRUE, 0);
 
  battstat->eventstatus = gtk_event_box_new ();
  gtk_widget_show (battstat->eventstatus);
  gtk_container_add (GTK_CONTAINER (battstat->framestatus), battstat->eventstatus);
 
  battstat->statusvbox = gtk_vbox_new ( FALSE, 5);
  gtk_widget_show (battstat->statusvbox);
  gtk_container_add(GTK_CONTAINER(battstat->eventstatus), battstat->statusvbox);

  battstat->framebattery = gtk_frame_new(NULL);
  gtk_widget_set_usize( battstat->framebattery, -1, 24);
  gtk_frame_set_shadow_type ( GTK_FRAME (battstat->framebattery), GTK_SHADOW_ETCHED_IN);
  gtk_widget_show(battstat->framebattery);
  gtk_box_pack_start(GTK_BOX(battstat->hbox1), battstat->framebattery, FALSE, TRUE, 0);

  battstat->eventbattery = gtk_event_box_new();
  gtk_widget_show (battstat->eventbattery);
  gtk_container_add (GTK_CONTAINER (battstat->framebattery), battstat->eventbattery);

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
  gtk_widget_set_usize( battstat->frameybattery, 24, 46);
  gtk_frame_set_shadow_type ( GTK_FRAME (battstat->frameybattery), GTK_SHADOW_ETCHED_IN);
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
  battstat->statuspercent = gtk_label_new("0%");
  gtk_box_pack_start (GTK_BOX (battstat->statusvbox), battstat->statuspercent, FALSE, TRUE, 0);

  /* Alloc battery colors */
  for(i=0; orange[i].pixel!=-1; i++) {
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &darkorange[i], FALSE, TRUE);
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &darkyellow[i], FALSE, TRUE);
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &darkred[i], FALSE, TRUE);
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &darkgreen[i], FALSE, TRUE);
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &orange[i], FALSE, TRUE);
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &yellow[i], FALSE, TRUE);
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &red[i], FALSE, TRUE);
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &green[i], FALSE, TRUE);
    gdk_colormap_alloc_color (gdk_colormap_get_system (), &gray[i], FALSE, TRUE);
  }

  /* Set the default tooltips.. */
  battstat->ac_tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (battstat->ac_tip,
			battstat->eventstatus,
			"",
			NULL);
  battstat->progress_tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (battstat->progress_tip,
			battstat->eventbattery,
			"",
			NULL);
  battstat->progressy_tip = gtk_tooltips_new ();
  gtk_tooltips_set_tip (battstat->progressy_tip,
			battstat->eventybattery,
			"",
			NULL);

  applet_widget_register_stock_callback (APPLET_WIDGET (battstat->applet),
                                         "properties",
                                         GNOME_STOCK_MENU_PROP,
                                         _("Properties..."),
                                         prop_cb,
                                         battstat);
  applet_widget_register_stock_callback (APPLET_WIDGET (battstat->applet),
                                         "about",
                                         GNOME_STOCK_MENU_ABOUT,
                                         _("About..."),
                                         about_cb,
                                         battstat);
  applet_widget_register_stock_callback (APPLET_WIDGET (battstat->applet),
                                         "help",
                                         GNOME_STOCK_PIXMAP_HELP,
                                         _("Help"),
                                         help_cb,
                                         battstat);
  applet_widget_register_callback ( APPLET_WIDGET (battstat->applet),
				    "suspend",
				    _("Suspend laptop"),
				    suspend_cb,
				    battstat);
  if(strlen(battstat->suspend_cmd)> 0) {
    applet_widget_callback_set_sensitive (APPLET_WIDGET (battstat->applet),
					  "suspend",
					  TRUE);
  } else {
    applet_widget_callback_set_sensitive (APPLET_WIDGET (battstat->applet),
					  "suspend",
					  FALSE);
  }
  gtk_signal_connect (GTK_OBJECT(battstat->applet),"save_session",
		      GTK_SIGNAL_FUNC(applet_save_session),
		      battstat);


  statusdock = status_docklet_new();

  gtk_signal_connect(GTK_OBJECT(statusdock), "build_plug",
		     GTK_SIGNAL_FUNC(build_our_plug),
		     battstat);
  status_docklet_run(STATUS_DOCKLET(statusdock));

#ifdef HAVE_PANEL_PIXEL_SIZE
  gtk_signal_connect(GTK_OBJECT(battstat->applet),"change_pixel_size",
		     GTK_SIGNAL_FUNC(applet_change_pixel_size),
		     battstat);
#endif

  gtk_signal_connect(GTK_OBJECT(battstat->applet),"change_orient",
		     GTK_SIGNAL_FUNC(change_orient),
		     battstat);

  return FALSE;
}

int
main(int argc, char *argv[])
{
  ProgressData *battstat;

  if (DEBUG) g_print("main()\n");

  apm_readinfo();

#ifdef __FreeBSD__
  if(apminfo.ai_status == 0) cleanup(2);
#endif

  battstat = g_malloc( sizeof(ProgressData) );

  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain(PACKAGE);

  battstat->colors_changed = TRUE;
  battstat->suspend_cmd = FALSE;
  battstat->panelsize = 48;
  init_applet(argc, argv, battstat);
  load_preferences(battstat);
  create_layout(argc, argv, battstat);
  load_font(battstat);
  pixmap_timeout(battstat);
  gtk_signal_emit_by_name(GTK_OBJECT(battstat->applet), "change_orient", battstat, NULL);
#ifdef HAVE_PANEL_PIXEL_SIZE
  gtk_signal_emit_by_name(GTK_OBJECT(battstat->applet), "change_pixel_size", battstat, NULL);
#endif
  battstat->pixtimer = gtk_timeout_add (1000, pixmap_timeout, battstat);

  applet_widget_gtk_main();
  return(0);
}
