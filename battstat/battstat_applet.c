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

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <glade/glade.h>
#include <gnome.h>

#include <panel-applet.h>
#include <panel-applet-gconf.h>

#include "battstat.h"
#include "pixmaps.h"

#ifndef gettext_noop
#define gettext_noop(String) (String)
#endif

#define GCONF_PATH ""

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

static void
initialise_global_pixmaps( void )
{
  GdkDrawable *defaults;

  defaults = gdk_screen_get_root_window( gdk_screen_get_default() );

  statusimage[BATTERY] = gdk_pixmap_create_from_xpm_d( defaults,
                                                       &statusmask[BATTERY],
                                                       NULL,
                                                       battery_small_xpm );

  statusimage[AC] = gdk_pixmap_create_from_xpm_d( defaults,
                                                  &statusmask[AC],
                                                  NULL,
                                                  ac_small_xpm );
   
  statusimage[FLASH] = gdk_pixmap_create_from_xpm_d( defaults,
						     &statusmask[FLASH],
						     NULL,
						     flash_small_xpm );
   
  statusimage[WARNING] = gdk_pixmap_create_from_xpm_d( defaults,
						       &statusmask[WARNING],
						       NULL,
						       warning_small_xpm );
}

static void
allocate_battery_colours( void )
{
  int i;

  for( i = 0; orange[i].pixel != -1; i++ )
  {
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
}

static void
static_global_initialisation()
{
  static int initialised;

  if( initialised )
    return;

  power_management_initialise();
  allocate_battery_colours();
  initialise_global_pixmaps();
  glade_init();

  initialised = 1;
}

static char *
get_remaining (BatteryStatus *info)
{
	int hours;
	int mins;

	hours = info->minutes / 60;
	mins = info->minutes % 60;

	if (info->on_ac_power && info->percent == 100)
		return g_strdup_printf (_("Battery charged (%d%%)"), info->percent);
	else if (info->minutes < 0 && !info->on_ac_power)
		return g_strdup_printf (_("Unknown time (%d%%) remaining"), info->percent);
	else if (time < 0 && !info->on_ac_power)
		return g_strdup_printf (_("Unknown time (%d%%) until charged"), info->percent);
	else
		if (hours == 0)
			if (!info->on_ac_power)
				return g_strdup_printf (ngettext (
						"%d minute (%d%%) remaining",
						"%d minutes (%d%%) remaining",
						mins), mins, info->percent);
			else
				return g_strdup_printf (ngettext (
						"%d minute until charged (%d%%)",
						"%d minutes until charged (%d%%)",
						mins), mins, info->percent);
		else if (mins == 0)
			if (!info->on_ac_power)
				return g_strdup_printf (ngettext (
						"%d hour (%d%%) remaining",
						"%d hours (%d%%) remaining",
						hours), hours, info->percent);
			else
				return g_strdup_printf (ngettext (
						"%d hour (%d%%) until charged (%d%%)",
						"%d hours (%d%%) until charged (%d%%)",
						hours), hours, info->percent);
		else
			if (!info->on_ac_power)
				/* TRANSLATOR: "%d %s %d %s" are "%d hours %d minutes"
				 * Swap order with "%2$s %2$d %1$s %1$d if needed */
				return g_strdup_printf (_("%d %s %d %s (%d%%) remaining"),
						hours, ngettext ("hour", "hours", hours),
						mins, ngettext ("minute", "minutes", mins),
						info->percent);
			else
				/* TRANSLATOR: "%d %s %d %s" are "%d hours %d minutes"
				 * Swap order with "%2$s %2$d %1$s %1$d if needed */
				return g_strdup_printf (_("%d %s %d %s until charged (%d%%)"),
						hours, ngettext ("hour", "hours", hours),
						mins, ngettext ("minute", "minutes", mins),
						info->percent);
}

static void
on_lowbatt_notification_response (GtkWidget *widget, gint arg, GtkWidget **self)
{
	   gtk_widget_destroy (GTK_WIDGET (*self));
	   *self = NULL;
}

static void
battery_full_dialog( void )
{
  GtkWidget *dialog, *hbox, *image, *label;
  GdkPixbuf *pixbuf;

  gchar *new_label;
  dialog = gtk_dialog_new_with_buttons (
		_("Battery Notice"),
		NULL,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		NULL);
  g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
			    G_CALLBACK (gtk_widget_destroy),
			    GTK_OBJECT (dialog));

  gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
  hbox = gtk_hbox_new (FALSE, 6);
  pixbuf = gtk_icon_theme_load_icon (
		gtk_icon_theme_get_default (),
		"gnome-dev-battery",
		48,
		GTK_ICON_LOOKUP_USE_BUILTIN,
		NULL);
  image = gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (pixbuf);
  gtk_box_pack_start (GTK_BOX (hbox), image, TRUE, TRUE, 6);
  new_label = g_strdup_printf (
		"<span weight=\"bold\" size=\"larger\">%s</span>",
 		_("Your battery is now fully recharged"));
  label = gtk_label_new (new_label);
  g_free (new_label);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 6);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), hbox);
  gtk_widget_show_all (dialog);
}

static void
battery_low_dialog( ProgressData *battery, BatteryStatus *info )
{
  GtkWidget *dialog, *hbox, *image, *label;
  gchar *new_string, *new_label;
  GdkPixbuf *pixbuf;

  new_string = get_remaining (info);
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
		    G_CALLBACK (on_lowbatt_notification_response),
		    &battery->lowbattnotificationdialog);

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
  battery->lowbattnotification=FALSE;
}

void
update_tooltip( ProgressData *battstat, BatteryStatus *info )
{
  gchar *powerstring;
  gchar *remaining;
  gchar *tiptext;

  if (info->present)
  {
    if (info->on_ac_power)
      powerstring = AC_POWER_STRING;
    else
      powerstring = DC_POWER_STRING;

    remaining = get_remaining (info);

    tiptext = g_strdup_printf ("%s\n%s", powerstring, remaining);
    g_free (remaining);
  }
  else
  {
    if (info->on_ac_power)
      tiptext = g_strdup_printf ("%s\n%s", AC_POWER_STRING,
                                 _("No battery present"));
    else
      tiptext = g_strdup_printf ("%s\n%s", DC_POWER_STRING,
                                 _("Battery status unknown"));
  }

  gtk_tooltips_set_tip (battstat->tip, battstat->applet, tiptext, NULL);
  g_free (tiptext);
}

static void
update_battery_image( ProgressData *battstat, BatteryStatus *info )
{
  GdkColor *color, *darkcolor;
  GdkPixmap *pixmap;
  GdkBitmap *pixmask;
  guint progress_value;
  gint i, x;
  int batt_life = info->percent;

  if (!battstat->showbattery)
    return;

  if (batt_life <= battstat->red_val) {
    color = red;
    darkcolor = darkred;
  } else if (batt_life <= battstat->orange_val) {
    color = orange;
    darkcolor = darkorange;
  } else if (batt_life <= battstat->yellow_val) {
    color = yellow;
    darkcolor = darkyellow;
  } else {
    color = green;
    darkcolor = darkgreen;
  }
      
  if (battstat->horizont)
    pixmap = gdk_pixmap_create_from_xpm_d( battstat->applet->window, &pixmask,
                                           NULL, battery_gray_xpm );
  else
    pixmap = gdk_pixmap_create_from_xpm_d( battstat->applet->window, &pixmask,
                                           NULL, battery_y_gray_xpm );

  if (battstat->draintop) {
    progress_value = PROGLEN * batt_life / 100.0;
	    
    for (i = 0; color[i].pixel != -1; i++)
    {
      gdk_gc_set_foreground (battstat->pixgc, &color[i]);

      if (battstat->horizont)
        gdk_draw_line (pixmap, battstat->pixgc, pixel_offset_top[i], i + 2,
                       pixel_offset_top[i] + progress_value, i + 2);
      else
        gdk_draw_line (pixmap, battstat->pixgc, i + 2, pixel_offset_top[i],
                       i + 2, pixel_offset_top[i] + progress_value);
    }
  }
  else
  {
    progress_value = PROGLEN * batt_life / 100.0;

    for (i = 0; color[i].pixel != -1; i++)
    {
      gdk_gc_set_foreground (battstat->pixgc, &color[i]);

      if (battstat->horizont)
        gdk_draw_line (pixmap, battstat->pixgc, pixel_offset_bottom[i], i + 2,
                       pixel_offset_bottom[i] - progress_value, i + 2);
      else
        gdk_draw_line (pixmap, battstat->pixgc, i + 2,
                       pixel_offset_bottom[i] - 1, i + 2,
                       pixel_offset_bottom[i] - progress_value);
    }

    for (i = 0; darkcolor[i].pixel != -1; i++)
    {
      x = pixel_offset_bottom[i] - progress_value - pixel_top_length[i];
      if (x < pixel_offset_top[i])
        x = pixel_offset_top[i];

      if (progress_value < 33)
      {
        gdk_gc_set_foreground (battstat->pixgc, &darkcolor[i]);
		     
        if (battstat->horizont)
          gdk_draw_line (pixmap, battstat->pixgc,
                         pixel_offset_bottom[i] - progress_value - 1,
                         i + 2, x, i + 2);
        else
          gdk_draw_line (pixmap, battstat->pixgc, i + 2,
                         pixel_offset_bottom[i] - progress_value - 1,
                         i + 2, x);
      }
    }
  }

  gtk_image_set_from_pixmap( GTK_IMAGE(battstat->battery),
                             pixmap, pixmask );
}

static void
update_percent_label( ProgressData *battstat, BatteryStatus *info )
{
  gchar *new_label;
  gchar *new_string;
  gchar *rem_time = NULL;

  if (info->present && battstat->showtext == APPLET_SHOW_PERCENT)
    new_label = g_strdup_printf ("%d%%", info->percent);
  else if (info->present && battstat->showtext == APPLET_SHOW_TIME)
  {
    if (info->on_ac_power && info->percent == 100)
      new_label = g_strdup ("-:--");
    else
    {
      int time;
      time = info->minutes;
      new_label = g_strdup_printf ("%d:%02d", time/60, time%60);
    }
  }
  else
    new_label = g_strdup (_("N/A"));

  gtk_label_set_text (GTK_LABEL (battstat->percent), new_label);
  g_free (new_label);
}

static void
possibly_update_status_icon( ProgressData *battstat, BatteryStatus *info )
{
  guint pixmap_index;

  battstat->flash = !battstat->flash;

  pixmap_index = (info->on_ac_power) ?
                 (info->charging && battstat->flash ? FLASH : AC) :
                 (info->percent <= battstat->red_val ? WARNING : BATTERY);

  if ( pixmap_index != battstat->last_pixmap_index )
  {
    gtk_image_set_from_pixmap (GTK_IMAGE (battstat->status),
                               statusimage[pixmap_index],
                               statusmask[pixmap_index]);
    battstat->last_pixmap_index = pixmap_index;
  }
}

gint
check_for_updates( gpointer data )
{
  ProgressData *battstat = data;
  BatteryStatus info;
  
  if (DEBUG) g_print("check_for_updates()\n");

  power_management_getinfo( &info );

  possibly_update_status_icon( battstat, &info );

  if( !info.on_ac_power &&
      battstat->last_batt_life != 1000 &&
      battstat->last_batt_life > battstat->red_val &&
      info.percent <= battstat->red_val &&
      info.present )
  {
    /* Warn that battery dropped below red_val */
    if(battstat->lowbattnotification)
      battery_low_dialog(battstat, &info);

    if(battstat->beep)
      gdk_beep();
    
    gnome_triggers_do ("", NULL, "battstat_applet", "LowBattery", NULL);
  }

  if( battstat->last_charging &&
      battstat->last_acline_status &&
      battstat->last_acline_status!=1000 &&
      !info.charging &&
      info.on_ac_power &&
      info.present &&
      info.percent > 99)
  {
    /* Inform that battery now fully charged */
    gnome_triggers_do ("", NULL, "battstat_applet", "BatteryFull", NULL);

    if(battstat->fullbattnot)
      battery_full_dialog();
 
    if (battstat->beep)
      gdk_beep();
  }

  if( !battstat->last_charging &&
      info.charging &&
      info.on_ac_power &&
      info.present)
  {
    /*
     * the battery is charging again, reset the dialog display flag
     * Thanks to Richard Kinder <r_kinder@yahoo.com> for this patch.
     */
    battstat->lowbattnotification = panel_applet_gconf_get_bool (
                           PANEL_APPLET(battstat->applet),
                           GCONF_PATH "low_battery_notification", NULL);

    if (battstat->lowbattnotificationdialog)
    {
      /* we can remove the battery warning dialog */
      gtk_widget_destroy (battstat->lowbattnotificationdialog);
      battstat->lowbattnotificationdialog = NULL;
    }
  }

  if( info.on_ac_power != battstat->last_acline_status ||
      info.percent != battstat->last_batt_life ||
      info.state != battstat->last_batt_state ||
      battstat->colors_changed )
  {
    /* Something changed */

    /* Update the tooltip */
    update_tooltip( battstat, &info );

    /* Update the battery meter image */
    update_battery_image( battstat, &info );

    /* Update the label */
    update_percent_label( battstat, &info );
  }

  battstat->last_charging = info.charging;
  battstat->last_batt_state = info.state;
  battstat->last_batt_life = info.percent;
  battstat->last_acline_status = info.on_ac_power;
  
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
   g_object_unref(pdata->tip);

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
  if (DEBUG) g_print("change_orient()\n");

  if( orient != battstat->orienttype )
  {
    battstat->orienttype = orient;
    reconfigure_layout( battstat );
  }
}

void
size_allocate( PanelApplet *applet, GtkAllocation *allocation,
               ProgressData *battstat)
{
  if (DEBUG) g_print("applet_change_pixel_size()\n");

  if( battstat->width == allocation->width &&
      battstat->height == allocation->height )
    return;

  battstat->width = allocation->width;
  battstat->height = allocation->height;
  reconfigure_layout( battstat );
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
	rc_style = gtk_rc_style_new ();
	gtk_widget_modify_style (GTK_WIDGET (battstat->applet), rc_style);
	g_object_unref (rc_style);

	switch (type) {
		case PANEL_COLOR_BACKGROUND:
			gtk_widget_modify_bg (GTK_WIDGET (battstat->applet),
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

static void
table_layout_attach( GtkTable *table, LayoutLocation loc, GtkWidget *child )
{
  GtkAttachOptions flags;

  flags = GTK_FILL | GTK_EXPAND;

  switch( loc )
  {
    case LAYOUT_LONG:
      gtk_table_attach( table, child, 1, 2, 0, 2, flags, flags, 2, 2 );
      break;

    case LAYOUT_TOPLEFT:
      gtk_table_attach( table, child, 0, 1, 0, 1, flags, flags, 2, 2 );
      break;

    case LAYOUT_TOP:
      gtk_table_attach( table, child, 1, 2, 0, 1, flags, flags, 2, 2 );
      break;

    case LAYOUT_LEFT:
      gtk_table_attach( table, child, 0, 1, 1, 2, flags, flags, 2, 2 );
      break;

    case LAYOUT_CENTRE:
      gtk_table_attach( table, child, 1, 2, 1, 2, flags, flags, 2, 2 );
      break;

    case LAYOUT_RIGHT:
      gtk_table_attach( table, child, 2, 3, 1, 2, flags, flags, 2, 2 );
      break;

    case LAYOUT_BOTTOM:
      gtk_table_attach( table, child, 1, 2, 2, 3, flags, flags, 2, 2 );
      break;

    default:
      break;
  }
}

gint
reconfigure_layout( ProgressData *battstat )
{
  gboolean up_down_order = FALSE;
  gboolean do_square = FALSE;
  LayoutConfiguration c;
  int battery_horiz = 0;
  int needwidth;

  /* decide if we are doing to do 'square' mode */
  switch( battstat->orienttype )
  {
    case PANEL_APPLET_ORIENT_UP:
    case PANEL_APPLET_ORIENT_DOWN:
      up_down_order = TRUE;

      /* need to be at least 46px tall to do square mode on a horiz. panel */
      if( battstat->height >= 46 )
        do_square = TRUE;
      break;

    case PANEL_APPLET_ORIENT_LEFT:
    case PANEL_APPLET_ORIENT_RIGHT:
      /* we need 64px to fix the text beside anything */
      if( battstat->showtext )
        needwidth = 64;
      /* we need 48px to fix the icon and battery side-by-side */
      else
        needwidth = 48;

      if( battstat->width >= needwidth )
        do_square = TRUE;
      break;
  }

  c.status = c.text = c.battery = LAYOUT_NONE;

  if( do_square )
  {
    if( battstat->showbattery )
    {
       c.battery = LAYOUT_LONG;

      if( battstat->showstatus )
        c.status = LAYOUT_TOPLEFT;

      if( battstat->showtext )
        c.text = LAYOUT_LEFT;
    }
    else
    {
      /* We have enough room to do 'square' mode but the battery meter is
       * not requested.  We can, instead, take up the extra space by stacking
       * our items in the opposite order that we normally would (ie: stack
       * horizontally on a vertical panel and vertically on horizontal)
       */
      up_down_order = !up_down_order;
      do_square = FALSE;
    }
  }

  if( !do_square )
  {
    if( up_down_order )
    {
      if( battstat->showstatus )
        c.status = LAYOUT_LEFT;
      if( battstat->showbattery )
        c.battery = LAYOUT_CENTRE;
      if( battstat->showtext )
        c.text = LAYOUT_RIGHT;

      battery_horiz = 1;
    }
    else
    {
      if( battstat->showstatus )
        c.status = LAYOUT_TOP;
      if( battstat->showbattery )
        c.battery = LAYOUT_CENTRE;
      if( battstat->showtext )
        c.text = LAYOUT_BOTTOM;
    }
  }

  if( battery_horiz == battstat->horizont &&
      !memcmp( &c, &battstat->layout, sizeof (LayoutConfiguration) ) )
    return;

  if( battstat->layout.text )
    gtk_container_remove( GTK_CONTAINER( battstat->table ),
                          battstat->percent );
  if( battstat->layout.status )
    gtk_container_remove( GTK_CONTAINER( battstat->table ),
                          battstat->status );
  if( battstat->layout.battery )
    gtk_container_remove( GTK_CONTAINER( battstat->table ),
                          battstat->battery );


  table_layout_attach( GTK_TABLE(battstat->table),
                       c.battery, battstat->battery );
  table_layout_attach( GTK_TABLE(battstat->table),
                       c.status, battstat->status );
  table_layout_attach( GTK_TABLE(battstat->table),
                       c.text, battstat->percent );

  battstat->horizont = battery_horiz;
  battstat->layout = c;

  gtk_widget_show_all( battstat->applet );
  check_for_updates( battstat );
}

gint
create_layout(ProgressData *battstat)
{
  if (DEBUG) g_print("create_layout()\n");

  battstat->table = gtk_table_new (3, 3, FALSE);

  battstat->percent = gtk_label_new ("0%");

  battstat->status = gtk_image_new_from_pixmap ( statusimage[BATTERY], statusmask[BATTERY] );

  battstat->battery = gtk_image_new();

  gtk_widget_realize (battstat->applet);
  battstat->pixgc = gdk_gc_new( battstat->applet->window );

  battstat->style = gtk_widget_get_style( battstat->applet );

  g_signal_connect(G_OBJECT(battstat->applet),
                   "destroy",
                   G_CALLBACK(destroy_applet),
                   battstat);


  gtk_widget_ref( battstat->status );
  gtk_widget_ref( battstat->percent );
  gtk_widget_ref( battstat->battery );
  gtk_object_sink( GTK_OBJECT( battstat->status ) );
  gtk_object_sink( GTK_OBJECT( battstat->percent ) );
  gtk_object_sink( GTK_OBJECT( battstat->battery ) );

  battstat->layout.status = LAYOUT_NONE;
  battstat->layout.text = LAYOUT_NONE;
  battstat->layout.battery = LAYOUT_NONE;

  gtk_container_add (GTK_CONTAINER (battstat->applet), battstat->table);
  gtk_widget_show_all (battstat->applet);

   /* Set the default tooltips.. */
  battstat->tip = gtk_tooltips_new ();
  g_object_ref (battstat->tip);
  gtk_object_sink (GTK_OBJECT (battstat->tip));
  gtk_tooltips_set_tip (battstat->tip,
			battstat->applet,
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
  AtkObject *atk_widget;

  if (DEBUG) g_print("main()\n");
  
  gtk_window_set_default_icon_name ("battstat");
  
  panel_applet_add_preferences (applet, "/schemas/apps/battstat-applet/prefs", NULL);
  panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);

  
  battstat = g_new0 (ProgressData, 1);
  
  battstat->applet = GTK_WIDGET (applet);
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

  load_preferences (battstat);
  create_layout (battstat);
  check_for_updates (battstat);
  reconfigure_layout( battstat );
  change_orient (applet, battstat->orienttype, battstat);
  battstat->pixtimer = gtk_timeout_add (1000, check_for_updates, battstat);

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

  static_global_initialisation();
  
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
      

