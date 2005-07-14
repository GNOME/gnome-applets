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

#include <gtk/gtk.h>

#include <libgnome/libgnome.h>

#include <libgnomeui/libgnomeui.h>

#include <glade/glade.h>

#include <panel-applet.h>
#include <panel-applet-gconf.h>

#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#endif

#include "battstat.h"
#include "pixmaps.h"

#ifndef gettext_noop
#define gettext_noop(String) (String)
#endif

#define GCONF_PATH ""

static void about_cb( BonoboUIComponent *, ProgressData *, const char * );
static void help_cb( BonoboUIComponent *, ProgressData *, const char * );

static const BonoboUIVerb battstat_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("BattstatProperties", prop_cb),
	BONOBO_UI_UNSAFE_VERB ("BattstatHelp",       help_cb),
	BONOBO_UI_UNSAFE_VERB ("BattstatAbout",      about_cb),
        BONOBO_UI_VERB_END
};

#define AC_POWER_STRING _("System is running on AC power")
#define DC_POWER_STRING _("System is running on battery power")

/* The icons for Battery, Critical, AC and Charging */
static GdkPixmap *statusimage[STATUS_PIXMAP_NUM];
static GdkBitmap *statusmask[STATUS_PIXMAP_NUM];

/* Assuming a horizontal battery, the colour is drawn into it one horizontal
   line at a time as a vertical gradient.  The following arrays decide where
   each horizontal line starts (the length of the lines varies with the
   percentage battery life remaining).
*/
static const int pixel_offset_top[]={ 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5 };
static const int pixel_top_length[]={ 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2 };
static const int pixel_offset_bottom[]={ 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 38, 38 };


/* The following array is the colour of each line.  The (slightly) varying
   colours are what makes for the gradient effect.  The 'dark' colours are
   used to draw the end of the bar, giving it more of a 3D look.  The code
   assumes that all of these arrays will have the same number of elements.
*/
static GdkColor green[] = {
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
};

static GdkColor red[] = {
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
};

static GdkColor yellow[] = {
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
};

static GdkColor orange[] = {
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
};

static GdkColor darkgreen[] = {
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
};

static GdkColor darkorange[] = {
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
};

static GdkColor darkyellow[] = {
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
};

static GdkColor darkred[] = {
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
};

/* Initialise the global static variables that store our status pixmaps from
   their XPM format (as stored in pixmaps.h).  This should only be done once
   since they are global variables.
*/
static void
initialise_global_pixmaps( void )
{
  GdkDrawable *defaults;

  defaults = gdk_screen_get_root_window( gdk_screen_get_default() );

  statusimage[STATUS_PIXMAP_BATTERY] =
    gdk_pixmap_create_from_xpm_d( defaults, &statusmask[STATUS_PIXMAP_BATTERY],
                                  NULL, battery_small_xpm );

  statusimage[STATUS_PIXMAP_METER] =
    gdk_pixmap_create_from_xpm_d( defaults, &statusmask[STATUS_PIXMAP_METER],
                                  NULL, battery_small_meter_xpm );

  statusimage[STATUS_PIXMAP_AC] =
    gdk_pixmap_create_from_xpm_d( defaults, &statusmask[STATUS_PIXMAP_AC],
                                  NULL, ac_small_xpm );
   
  statusimage[STATUS_PIXMAP_CHARGE] =
    gdk_pixmap_create_from_xpm_d( defaults, &statusmask[STATUS_PIXMAP_CHARGE],
                                  NULL, charge_small_xpm );
   
  statusimage[STATUS_PIXMAP_WARNING] =
    gdk_pixmap_create_from_xpm_d( defaults, &statusmask[STATUS_PIXMAP_WARNING],
                                  NULL, warning_small_xpm );
}

/* For non-truecolour displays, each GdkColor has to have a palette entry
   allocated for it.  This should only be done once for the entire display.
*/
static void
allocate_battery_colours( void )
{
  GdkColormap *colourmap;
  int i;

  colourmap = gdk_colormap_get_system();

  /* assumed: all the colour arrays have the same number of elements */
  for( i = 0; i < G_N_ELEMENTS( orange ); i++ )
  {
     gdk_colormap_alloc_color( colourmap, &darkorange[i], FALSE, TRUE );
     gdk_colormap_alloc_color( colourmap, &darkyellow[i], FALSE, TRUE );
     gdk_colormap_alloc_color( colourmap, &darkred[i], FALSE, TRUE );
     gdk_colormap_alloc_color( colourmap, &darkgreen[i], FALSE, TRUE );
     gdk_colormap_alloc_color( colourmap, &orange[i], FALSE, TRUE );
     gdk_colormap_alloc_color( colourmap, &yellow[i], FALSE, TRUE );
     gdk_colormap_alloc_color( colourmap, &red[i], FALSE, TRUE );
     gdk_colormap_alloc_color( colourmap, &green[i], FALSE, TRUE );
  }
}

/* The following two functions keep track of how many instances of the applet
   are currently running.  When the first instance is started, some global
   initialisation is done.  When the last instance exits, cleanup occurs.

   The teardown code here isn't entirely complete (for example, it doesn't
   deallocate the GdkColors or free the GdkPixmaps.  This is OK so long
   as the process quits immediately when the last applet is removed (which
   it does.)
*/
static int instances;

static const char *
static_global_initialisation(void)
{
  const char *err;

  if( instances++ )
    return NULL;

  allocate_battery_colours();
  initialise_global_pixmaps();
  glade_init();
  err = power_management_initialise();

  return err;
}

static void
static_global_teardown(void)
{
  if( --instances )
    return;

  /* instances == 0 */

  power_management_cleanup();
}

/* Pop up an error dialog on the same screen as 'applet' saying 'msg'.
 */
static void
battstat_error_dialog( GtkWidget *applet, const char *msg )
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new( NULL, 0, GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_OK, msg);

  gtk_window_set_screen( GTK_WINDOW (dialog),
                         gtk_widget_get_screen (GTK_WIDGET (applet)) );

  g_signal_connect_swapped( GTK_OBJECT (dialog), "response",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (dialog) );

  gtk_widget_show_all( dialog );
}

/* Format a string describing how much time is left to fully (dis)charge
   the battery. The return value must be g_free()d.
*/
static char *
get_remaining (BatteryStatus *info)
{
	int hours;
	int mins;

	hours = info->minutes / 60;
	mins = info->minutes % 60;

	if (info->on_ac_power && !info->charging)
		return g_strdup_printf (_("Battery charged (%d%%)"), info->percent);
	else if (info->minutes < 0 && !info->on_ac_power)
		return g_strdup_printf (_("Unknown time (%d%%) remaining"), info->percent);
	else if (info->minutes < 0 && info->on_ac_power)
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
						"%d hour until charged (%d%%)",
						"%d hours until charged (%d%%)",
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

static gboolean
battery_full_notify (void)
{
#ifdef HAVE_LIBNOTIFY
	static NotifyIcon *icon = NULL;
	
	if (!notify_is_initted () && !notify_init (_("Battery Monitor")))
		return FALSE;

	/* XXX: this icon is not found in the theme... strange */
	/* if (!icon)
		icon = notify_icon_new_from_uri ("gnome-dev-battery"); */
	
	if (!notify_send_notification (NULL,
				"device",
				NOTIFY_URGENCY_NORMAL,
				_("Your battery is now fully recharged"),
				NULL,		/* body text */
				icon,		/* icon */
				TRUE, 0,	/* expiry, server default */
				NULL,		/* hints */
				NULL,		/* no user_data */
				0))		/* no actions */
		return FALSE;

	return TRUE;
#else
	return FALSE;
#endif
}

/* Show a dialog notifying the user that their battery is done charging.
 */
static void
battery_full_dialog( void )
{
  /* first attempt to use libnotify */
  if (battery_full_notify ())
	  return;
  
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
  gtk_window_set_keep_above (GTK_WINDOW (dialog), TRUE);
  gtk_window_stick (GTK_WINDOW (dialog));
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_focus_on_map (GTK_WINDOW (dialog), FALSE);
  gtk_widget_show_all (dialog);
}

/* Destroy the low battery notification dialog and mark it as such.
 */
static void
battery_low_dialog_destroy( ProgressData *battstat )
{
  gtk_widget_destroy( battstat->battery_low_dialog );
  battstat->battery_low_dialog = NULL;
  battstat->battery_low_label = NULL;
}

/* Update the text label in the battery low dialog.
 */
static void
battery_low_update_text( ProgressData *battstat, BatteryStatus *info )
{
  gchar *remaining, *new_label;
  GtkRequisition size;

  /* If we're not displaying the dialog then don't update it. */
  if( battstat->battery_low_label == NULL ||
      battstat->battery_low_dialog == NULL )
    return;

  gtk_widget_size_request( GTK_WIDGET( battstat->battery_low_label ), &size );

  /* If the label has never been set before, the width will be 0.  If it
     has been set before (width > 0) then we want to keep the size of
     the old widget (to keep the dialog from changing sizes) so we set it
     explicitly here.
   */
  if( size.width > 0 )
    gtk_widget_set_size_request( GTK_WIDGET( battstat->battery_low_label ),
                                 size.width, size.height );

  remaining = g_strdup_printf( ngettext(
                                 "You have %d minute of battery power "
				   "remaining (%d%% of the total capacity).",
                                 "You have %d minutes of battery power "
				   "remaining (%d%% of the total capacity).",
                                 info->minutes ),
                               info->minutes,info->percent );

  new_label = g_strdup_printf (
		"<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s\n\n%s",
		_("Your battery is running low"),
		remaining,
		/* TRANSLATORS: this is a list, it is left as a single string
		 * to allow you to make it appear like a list would in your
		 * locale. */
		_("To avoid losing your work:\n"
		  " \xE2\x80\xA2 suspend your laptop to save power,\n"
		  " \xE2\x80\xA2 plug your laptop into external power, or\n"
		  " \xE2\x80\xA2 save open documents and shut your laptop down."
		  ));

  gtk_label_set_markup( battstat->battery_low_label, new_label );
  g_free( remaining );
  g_free( new_label );
}

/* Show a dialog notifying the user that their battery is running low.
 */
static void
battery_low_dialog( ProgressData *battery, BatteryStatus *info )
{
  GtkWidget *hbox, *image, *label;
  GtkWidget *vbox;
  GdkPixbuf *pixbuf;

  /* If the dialog is already displayed then don't display it again. */
  if( battery->battery_low_dialog != NULL )
    return;

  battery->battery_low_dialog = gtk_dialog_new_with_buttons (
		 _("Battery Notice"),
		 NULL,
		 GTK_DIALOG_DESTROY_WITH_PARENT,
		 GTK_STOCK_OK,
		 GTK_RESPONSE_ACCEPT,
		 NULL);

  g_signal_connect_swapped( GTK_OBJECT (battery->battery_low_dialog),
                            "response",
                            G_CALLBACK (battery_low_dialog_destroy),
                            battery );

  gtk_container_set_border_width (GTK_CONTAINER (battery->battery_low_dialog),
		  6);
  gtk_dialog_set_has_separator (GTK_DIALOG (battery->battery_low_dialog),
		  FALSE);
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
		 "gnome-dev-battery",
		 48,
		 GTK_ICON_LOOKUP_USE_BUILTIN,
		 NULL);
  image = gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (pixbuf);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 6);
  gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
  label = gtk_label_new ("");
  battery->battery_low_label = GTK_LABEL( label );
  gtk_label_set_line_wrap( battery->battery_low_label, TRUE );
  gtk_label_set_selectable( battery->battery_low_label, TRUE );
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 6);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (battery->battery_low_dialog)->vbox), hbox);
	 
  gtk_window_set_keep_above (GTK_WINDOW (battery->battery_low_dialog), TRUE);
  gtk_window_stick (GTK_WINDOW (battery->battery_low_dialog));
  gtk_window_set_focus_on_map (GTK_WINDOW (battery->battery_low_dialog),
		  FALSE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (battery->battery_low_dialog),
		  TRUE);

  battery_low_update_text( battery, info );

  gtk_widget_show_all (battery->battery_low_dialog);
}

/* Update the text of the tooltip from the provided info.
 */
static void
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

/* Redraw the battery meter image.
 */
static void
update_battery_image (ProgressData *battstat, int batt_percent, int batt_time)
{
  GdkColor *color, *darkcolor;
  GdkPixmap *pixmap;
  GdkBitmap *pixmask;
  guint progress_value;
  gint i, x;
  int batt_life;

  if (!battstat->showbattery)
    return;

  batt_life = !battstat->red_value_is_time ? batt_percent : batt_time;

  if (batt_life <= battstat->red_val)
  {
    color = red;
    darkcolor = darkred;
  }
  else if (batt_life <= battstat->orange_val)
  {
    color = orange;
    darkcolor = darkorange;
  }
  else if (batt_life <= battstat->yellow_val)
  {
    color = yellow;
    darkcolor = darkyellow;
  }
  else
  {
    color = green;
    darkcolor = darkgreen;
  }

  /* We keep this pixgc allocated so we don't have to alloc/free it every
     time.  A widget has to be realized before it has a valid ->window so
     we do that here for battstat->applet just in case it's not already done.
  */
  if( battstat->pixgc == NULL )
  {
    gtk_widget_realize( battstat->applet );
    battstat->pixgc = gdk_gc_new( battstat->applet->window );
  }

  /* Depending on if the meter is horizontally oriented start out with the
     appropriate XPM image (from pixmaps.h)
  */
  if (battstat->horizont)
    pixmap = gdk_pixmap_create_from_xpm_d( battstat->applet->window, &pixmask,
                                           NULL, battery_gray_xpm );
  else
    pixmap = gdk_pixmap_create_from_xpm_d( battstat->applet->window, &pixmask,
                                           NULL, battery_y_gray_xpm );

  /* The core code responsible for painting the battery meter.  For each
     colour in our gradient array, draw a vertical or horizontal line
     depending on the current orientation of the meter.
  */
  if (battstat->draintop) {
    progress_value = PROGLEN * batt_life / 100.0;
	    
    for( i = 0; i < G_N_ELEMENTS( orange ); i++ )
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

    for( i = 0; i < G_N_ELEMENTS( orange ); i++)
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

    for( i = 0; i < G_N_ELEMENTS( orange ); i++ )
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

  /* Store our newly created pixmap into the GtkImage.  This results in
     the last reference to the old pixmap/mask being dropped.
  */
  gtk_image_set_from_pixmap( GTK_IMAGE(battstat->battery),
                             pixmap, pixmask );

  /* The GtkImage does not assume a reference to the pixmap or mask;
     you still need to unref them if you own references. GtkImage will
     add its own reference rather than adopting yours.
  */
  g_object_unref( G_OBJECT(pixmap) );
  g_object_unref( G_OBJECT(pixmask) );
}

/* Update the text label that either shows the percentage of time left.
 */
static void
update_percent_label( ProgressData *battstat, BatteryStatus *info )
{
  gchar *new_label;

  if (info->present && battstat->showtext == APPLET_SHOW_PERCENT)
    new_label = g_strdup_printf ("%d%%", info->percent);
  else if (info->present && battstat->showtext == APPLET_SHOW_TIME)
  {
    /* Fully charged or unknown (-1) time remaining displays -:-- */
    if ((info->on_ac_power && info->percent == 100) || info->minutes < 0)
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

/* Utility function to create a copy of a GdkPixmap */
static GdkPixmap *
copy_gdk_pixmap( GdkPixmap *src, GdkGC *gc )
{
  gint height, width;
  GdkPixmap *dest;

  gdk_drawable_get_size( GDK_DRAWABLE( src ), &width, &height );

  dest = gdk_pixmap_new( GDK_DRAWABLE( src ), width, height, -1 );

  gdk_draw_drawable( GDK_DRAWABLE( dest ), gc, GDK_DRAWABLE( src ),
                     0, 0, 0, 0, width, height );

  return dest;
}

/* Determine what status icon we ought to be displaying and change the
   status icon to display it if it is different from what we are currently
   showing.
 */
static void
possibly_update_status_icon( ProgressData *battstat, BatteryStatus *info )
{
  StatusPixmapIndex pixmap_index;
  int batt_life, last_batt_life;

  batt_life = !battstat->red_value_is_time ? info->percent : info->minutes;
  last_batt_life = !battstat->red_value_is_time ? battstat->last_batt_life :
	  					 battstat->last_minutes;

  if( info->on_ac_power )
  {
    if( info->charging )
      pixmap_index = STATUS_PIXMAP_CHARGE;
    else
      pixmap_index = STATUS_PIXMAP_AC;
  }
  else /* on battery */
  {
    if (batt_life > battstat->red_val)
      pixmap_index = STATUS_PIXMAP_BATTERY;
    else
      pixmap_index = STATUS_PIXMAP_WARNING;
  }

  /* If we are showing the full length battery meter then the status icon
     should display static icons.  If we are not showing the full meter
     then the status icon will show a smaller meter if we are on battery.
   */
  if( !battstat->showbattery && 
      (pixmap_index == STATUS_PIXMAP_BATTERY ||
       pixmap_index == STATUS_PIXMAP_WARNING) )
    pixmap_index = STATUS_PIXMAP_METER;


  /* Take care of drawing the smaller meter. */
  if( pixmap_index == STATUS_PIXMAP_METER &&
      (batt_life != last_batt_life ||
       battstat->last_pixmap_index != STATUS_PIXMAP_METER) )
  {
    GdkColor *colour;
    GdkPixmap *meter;
    guint progress_value;
    gint i, x;

    /* We keep this pixgc allocated so we don't have to alloc/free it every
       time.  A widget has to be realized before it has a valid ->window so
       we do that here for battstat->applet just in case it's not already done.
    */
    if( battstat->pixgc == NULL )
    {
      gtk_widget_realize( battstat->applet );
      battstat->pixgc = gdk_gc_new( battstat->applet->window );
    }

    /* Pull in a clean version of the icons so that we don't paint over
       top of the same icon over and over.  We neglect to free/update the
       statusmask here since it will always stay the same.
     */
    meter = copy_gdk_pixmap( statusimage[STATUS_PIXMAP_METER],
                             battstat->pixgc );

    if (batt_life <= battstat->red_val)
    {
      colour = red;
    }
    else if (batt_life <= battstat->orange_val)
    {
      colour = orange;
    }
    else if (batt_life <= battstat->yellow_val)
    {
      colour = yellow;
    }
    else
    {
      colour = green;
    }

    progress_value = 12 * info->percent / 100.0;
    
    for( i = 0; i < 10; i++ )
    {
      gdk_gc_set_foreground( battstat->pixgc, &colour[(i * 13 / 10)] );

      if( i >= 2 && i <= 7 )
	x = 17;
      else
	x = 16;

      gdk_draw_line( meter, battstat->pixgc,
		     i + 1, x - progress_value,
		     i + 1, x );
    }

    /* force a redraw immediately */
    gtk_image_set_from_pixmap( GTK_IMAGE (battstat->status),
                               meter, statusmask[STATUS_PIXMAP_METER] );

    /* free our private pixmap copy */
    g_object_unref( G_OBJECT( meter ) );
    battstat->last_pixmap_index = STATUS_PIXMAP_METER;
  }
  else if( pixmap_index != battstat->last_pixmap_index )
  {
    gtk_image_set_from_pixmap (GTK_IMAGE (battstat->status),
                               statusimage[pixmap_index],
                               statusmask[pixmap_index]);
    battstat->last_pixmap_index = pixmap_index;
  }
}

/* Gets called as a gtk_timeout once per second.  Checks for updates and
   makes any changes as appropriate.
 */
static gint
check_for_updates( gpointer data )
{
  ProgressData *battstat = data;
  BatteryStatus info;
  const char *err;

  if (DEBUG) g_print("check_for_updates()\n");

  if( (err = power_management_getinfo( &info )) )
    battstat_error_dialog( battstat->applet, err );

  possibly_update_status_icon( battstat, &info );

  if (!info.on_ac_power &&
      battstat->last_batt_life != 1000 &&
      (
        /* if percentage drops below red_val */
        (!battstat->red_value_is_time &&
         battstat->last_batt_life > battstat->red_val &&
         info.percent <= battstat->red_val) ||
	/* if time drops below red_val */
	(battstat->red_value_is_time &&
	 battstat->last_minutes > battstat->red_val &&
	 info.minutes <= battstat->red_val)
      ) &&
      info.present)
  {
    /* Warn that battery dropped below red_val */
    if(battstat->lowbattnotification)
    {
      battery_low_dialog(battstat, &info);

      if(battstat->beep)
        gdk_beep();
    }
    
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
    {
      battery_full_dialog();
 
      if (battstat->beep)
        gdk_beep();
    }
  }

  /* If the warning dialog is displayed and we just got plugged in then
     stop displaying it.
   */
  if( battstat->battery_low_dialog && info.on_ac_power )
    battery_low_dialog_destroy( battstat );

  if( info.on_ac_power != battstat->last_acline_status ||
      info.percent != battstat->last_batt_life ||
      info.minutes != battstat->last_minutes ||
      info.charging != battstat->last_charging )
  {
    /* Update the tooltip */
    update_tooltip( battstat, &info );

    /* If the warning dialog box is currently displayed, update that too. */
    if( battstat->battery_low_dialog != NULL )
      battery_low_update_text( battstat, &info );
  }

  if( info.percent != battstat->last_batt_life )
  {
    /* Update the battery meter image */
    update_battery_image (battstat, info.percent, info.minutes);
  }

  if( (battstat->showtext == APPLET_SHOW_PERCENT &&
       battstat->last_batt_life != info.percent) ||
      (battstat->showtext == APPLET_SHOW_TIME &&
       battstat->last_minutes != info.minutes) ||
       battstat->refresh_label ) /* set by properties dialog */
  {
    /* Update the label */
    update_percent_label( battstat, &info );

    /* done */
    battstat->refresh_label = FALSE;
  }

  battstat->last_charging = info.charging;
  battstat->last_batt_life = info.percent;
  battstat->last_minutes = info.minutes;
  battstat->last_acline_status = info.on_ac_power;

  return TRUE;
}

/* Gets called when the user removes the applet from the panel.  Clean up
   all instance-specific data and call the global teardown function to
   decrease our applet count (and possibly perform global cleanup)
 */
static void
destroy_applet( GtkWidget *widget, ProgressData *battstat )
{
  if (DEBUG) g_print("destroy_applet()\n");

  if (battstat->about_dialog)
    gtk_widget_destroy (battstat->about_dialog);

  if (battstat->prop_win)
    gtk_widget_destroy (GTK_WIDGET (battstat->prop_win));

  if( battstat->battery_low_dialog )
    battery_low_dialog_destroy( battstat );

  gtk_timeout_remove( battstat->pixtimer );

  if( battstat->pixgc )
    g_object_unref( G_OBJECT(battstat->pixgc) );

  g_object_unref( G_OBJECT(battstat->status) );
  g_object_unref( G_OBJECT(battstat->percent) );
  g_object_unref( G_OBJECT(battstat->battery) );
  g_object_unref( G_OBJECT(battstat->tip) );

  g_free( battstat );

  static_global_teardown();
}

/* Common function invoked by the 'Help' context menu item and the 'Help'
 * button in the preferences dialog.
 */
void
battstat_show_help( ProgressData *battstat, const char *section )
{
  GError *error = NULL;

  gnome_help_display_on_screen( "battstat", section,
                                gtk_widget_get_screen( battstat->applet ),
                                &error );

  if( error )
  {
    char *message;

    message = g_strdup_printf( _("There was an error displaying help: %s"),
                               error->message );
    battstat_error_dialog( battstat->applet, message );
    g_error_free( error );
    g_free( message );
  }
}


/* Called when the user selects the 'help' menu item.
 */
static void
help_cb( BonoboUIComponent *uic, ProgressData *battstat, const char *verb )
{
  battstat_show_help( battstat, NULL );
}

/* Called when the user selects the 'about' menu item.
 */
static void
about_cb( BonoboUIComponent *uic, ProgressData *battstat, const char *verb )
{
  const gchar *authors[] = {
    "J\xC3\xB6rgen Pehrson <jp@spektr.eu.org>", 
    "Lennart Poettering <lennart@poettering.de> (Linux ACPI support)",
    "Seth Nickell <snickell@stanford.edu> (GNOME2 port)",
    "Davyd Madeley <davyd@madeley.id.au>",
    "Ryan Lortie <desrt@desrt.ca>",
    "Joe Marcus Clarke <marcus@FreeBSD.org> (FreeBSD ACPI support)",
    NULL
   };

  const gchar *documenters[] = {
    "J\xC3\xB6rgen Pehrson <jp@spektr.eu.org>",
    "Trevor Curtis <tcurtis@somaradio.ca>",
    NULL
  };

  gtk_show_about_dialog( NULL,
    "name",                _("Battery Charge Monitor"), 
    "version",             VERSION,
    "copyright",           "\xC2\xA9 2000 The Gnulix Society, "
                           "\xC2\xA9 2002-2005 Free Software Foundation and "
                           "others",
    "comments",            _("This utility shows the status of your laptop "
                             "battery."),
    "authors",             authors,
    "documenters",         documenters,
    "translator-credits",  _("translator-credits"),
    "logo-icon-name",      "gnome-dev-battery",
    NULL );
}

/* This signal is delivered by the panel when the orientation of the applet
   has changed.  This is either because the applet has just been created,
   has just been moved to a new panel or the panel that the applet was on
   has changed orientation.
*/
static void
change_orient (PanelApplet       *applet,
	       PanelAppletOrient  orient,
	       ProgressData      *battstat)
{
  if (DEBUG) g_print("change_orient()\n");

  /* Ignore the update if we already know. */
  if( orient != battstat->orienttype )
  {
    battstat->orienttype = orient;

    /* The applet changing orientation very likely involves the layout
       being changed to better fit the new shape of the panel.
    */
    reconfigure_layout( battstat );
  }
}

/* This is delivered when our size has changed.  This happens when the applet
   is just created or if the size of the panel has changed.
*/
static void
size_allocate( PanelApplet *applet, GtkAllocation *allocation,
               ProgressData *battstat)
{
  if (DEBUG) g_print("applet_change_pixel_size()\n");

  /* Ignore the update if we already know. */
  if( battstat->width == allocation->width &&
      battstat->height == allocation->height )
    return;

  battstat->width = allocation->width;
  battstat->height = allocation->height;

  /* The applet changing size could result in the layout changing. */
  reconfigure_layout( battstat );
}

/* Some vaguely magic/evil code to handle coloured or transparent panels.
   The panel currently doesn't emit these signals often enough, so sometimes
   we don't get properly updated.
*/
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
	gtk_rc_style_unref (rc_style);

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
			g_object_unref (style);
			break;

		case PANEL_NO_BACKGROUND:
		default:
			break;
	}
}

/* Get our settings out of gconf.
 */
static void
load_preferences(ProgressData *battstat)
{
  PanelApplet *applet = PANEL_APPLET (battstat->applet);

  if (DEBUG) g_print("load_preferences()\n");
  
  battstat->red_val = panel_applet_gconf_get_int (applet, GCONF_PATH "red_value", NULL);
  battstat->red_val = CLAMP (battstat->red_val, 0, 100);
  battstat->red_value_is_time = panel_applet_gconf_get_bool (applet,
		  GCONF_PATH "red_value_is_time",
		  NULL);

  /* automatically calculate orangle and yellow values from the red value */
  battstat->orange_val = battstat->red_val * ORANGE_MULTIPLIER;
  battstat->orange_val = CLAMP (battstat->orange_val, 0, 100);
  
  battstat->yellow_val = battstat->red_val * YELLOW_MULTIPLIER;
  battstat->yellow_val = CLAMP (battstat->yellow_val, 0, 100);

  battstat->lowbattnotification = panel_applet_gconf_get_bool (applet, GCONF_PATH "low_battery_notification", NULL);
  battstat->fullbattnot = panel_applet_gconf_get_bool (applet, GCONF_PATH "full_battery_notification", NULL);
  battstat->beep = panel_applet_gconf_get_bool (applet, GCONF_PATH "beep", NULL);
  battstat->draintop = panel_applet_gconf_get_bool (applet, GCONF_PATH "drain_from_top", NULL);
  
  battstat->showstatus = panel_applet_gconf_get_bool (applet, GCONF_PATH "show_status", NULL);
  battstat->showbattery = panel_applet_gconf_get_bool (applet, GCONF_PATH "show_battery", NULL);

  /* for miagration from older versions */
  if (battstat->showstatus && battstat->showbattery)
	  battstat->showbattery = FALSE;
  
  battstat->showtext = panel_applet_gconf_get_int (applet, GCONF_PATH "show_text", NULL);
}

/* Convenience function to attach a child widget to a GtkTable in the
   position indicated by 'loc'.  This is very special-purpose for 3x3
   tables and only supports positions that are used in this applet.
 */
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

/* The layout has (maybe) changed.  Calculate what layout we ought to be
   using and update some things if anything has changed.  This is called
   from size/orientation change callbacks and from the preferences dialog
   when elements get added or removed.
 */
void
reconfigure_layout( ProgressData *battstat )
{
  gboolean up_down_order = FALSE;
  gboolean do_square = FALSE;
  LayoutConfiguration c;
  int battery_horiz = 0;
  int needwidth;

  /* Decide if we are doing to do 'square' mode. */
  switch( battstat->orienttype )
  {
    case PANEL_APPLET_ORIENT_UP:
    case PANEL_APPLET_ORIENT_DOWN:
      up_down_order = TRUE;

      /* Need to be at least 46px tall to do square mode on a horiz. panel */
      if( battstat->height >= 46 )
        do_square = TRUE;
      break;

    case PANEL_APPLET_ORIENT_LEFT:
    case PANEL_APPLET_ORIENT_RIGHT:
      /* We need 64px to fix the text beside anything. */
      if( battstat->showtext )
        needwidth = 64;
      /* We need 48px to fix the icon and battery side-by-side. */
      else
        needwidth = 48;

      if( battstat->width >= needwidth )
        do_square = TRUE;
      break;
  }

  /* Default to no elements being displayed. */
  c.status = c.text = c.battery = LAYOUT_NONE;

  if( do_square )
  {
    /* Square mode is only useful if we have the battery meter shown. */
    if( battstat->showbattery )
    {
       c.battery = LAYOUT_LONG;

      /* if( battstat->showstatus ) */ /* make this always true */
        c.status = LAYOUT_TOPLEFT;

      if( battstat->showtext )
        c.text = LAYOUT_LEFT;
    }
    else
    {
      /* We have enough room to do 'square' mode but the battery meter is
         not requested.  We can, instead, take up the extra space by stacking
         our items in the opposite order that we normally would (ie: stack
         horizontally on a vertical panel and vertically on horizontal).
      */
      up_down_order = !up_down_order;
      do_square = FALSE;
    }
  }

  if( !do_square )
  {
    if( up_down_order )
    {
      /* Stack horizontally for top and bottom panels. */
      /* if( battstat->showstatus ) */ /* make this always true */
        c.status = LAYOUT_LEFT;
      if( battstat->showbattery )
        c.battery = LAYOUT_CENTRE;
      if( battstat->showtext )
        c.text = LAYOUT_RIGHT;

      battery_horiz = 1;
    }
    else
    {
      /* Stack vertically for left and right panels. */
      /* if( battstat->showstatus ) */ /* make this always true */
        c.status = LAYOUT_TOP;
      if( battstat->showbattery )
        c.battery = LAYOUT_CENTRE;
      if( battstat->showtext )
        c.text = LAYOUT_BOTTOM;
    }
  }
  
  if( memcmp( &c, &battstat->layout, sizeof (LayoutConfiguration) ) )
  {
    /* Something in the layout has changed.  Rebuild. */

    /* Start by removing any elements in the table from the table. */
    if( battstat->layout.text )
      gtk_container_remove( GTK_CONTAINER( battstat->table ),
                            battstat->percent );
    if( battstat->layout.status )
      gtk_container_remove( GTK_CONTAINER( battstat->table ),
                            battstat->status );
    if( battstat->layout.battery )
      gtk_container_remove( GTK_CONTAINER( battstat->table ),
                            battstat->battery );

    /* Attach the elements to their new locations. */
    table_layout_attach( GTK_TABLE(battstat->table),
                         c.battery, battstat->battery );
    table_layout_attach( GTK_TABLE(battstat->table),
                         c.status, battstat->status );
    table_layout_attach( GTK_TABLE(battstat->table),
                         c.text, battstat->percent );

    gtk_widget_show_all( battstat->applet );
  }

  /* If we are showing the battery meter and we weren't showing it before or
     if the orientation has changed, we had better update it right now.
  */
  if( (c.battery && !battstat->layout.battery) ||
       battery_horiz != battstat->horizont )
  {
    battstat->horizont = battery_horiz;
    update_battery_image (battstat,
		    battstat->last_batt_life, battstat->last_minutes);
  }

  battstat->layout = c;

  /* Check for generic updates. This is required, for example, to make sure
     the text label is immediately updated to show the time remaining or
     percentage.
  */
  check_for_updates( battstat );
}

/* Allocate the widgets for the applet and connect our signals.
 */
static gint
create_layout(ProgressData *battstat)
{
  if (DEBUG) g_print("create_layout()\n");

  /* Allocate the four widgets that we need. */
  battstat->table = gtk_table_new( 3, 3, FALSE );
  battstat->percent = gtk_label_new( "" );
  battstat->status = gtk_image_new();
  battstat->battery = gtk_image_new();

  /* When you first get a pointer to a newly created GtkWidget it has one
     'floating' reference.  When you first add this widget to a container
     the container adds a real reference and removes the floating reference
     if one exists.  Since we insert/remove these widgets from the table
     when our layout is reconfigured, we need to keep our own 'real'
     reference to each widget.  This adds a real reference to each widget
     and "sinks" the floating reference.
  */
  gtk_widget_ref( battstat->status );
  gtk_widget_ref( battstat->percent );
  gtk_widget_ref( battstat->battery );
  gtk_object_sink( GTK_OBJECT( battstat->status ) );
  gtk_object_sink( GTK_OBJECT( battstat->percent ) );
  gtk_object_sink( GTK_OBJECT( battstat->battery ) );

  /* Let reconfigure_layout know that the table is currently empty. */
  battstat->layout.status = LAYOUT_NONE;
  battstat->layout.text = LAYOUT_NONE;
  battstat->layout.battery = LAYOUT_NONE;

  /* Put the table directly inside the applet and show everything. */
  gtk_container_add (GTK_CONTAINER (battstat->applet), battstat->table);
  gtk_widget_show_all (battstat->applet);

  /* Set up the tooltip widget (which we will fill in later). */
  battstat->tip = gtk_tooltips_new ();
  g_object_ref (battstat->tip);
  gtk_object_sink (GTK_OBJECT (battstat->tip));
  gtk_tooltips_set_tip (battstat->tip,
			battstat->applet,
			"",
			NULL);

  /* Attach all sorts of signals to the applet. */
  g_signal_connect(G_OBJECT(battstat->applet),
                   "destroy",
                   G_CALLBACK(destroy_applet),
                   battstat);

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

  return FALSE;
}

/* Called by the factory to fill in the fields for the applet.
 */
static gboolean
battstat_applet_fill (PanelApplet *applet)
{
  ProgressData *battstat;
  AtkObject *atk_widget;
  const char *err;

  if (DEBUG) g_print("main()\n");

  if( (err = static_global_initialisation()) )
    battstat_error_dialog( GTK_WIDGET (applet), err );

  gtk_window_set_default_icon_name ("gnome-dev-battery");
  
  panel_applet_add_preferences (applet, "/schemas/apps/battstat-applet/prefs",
                                NULL);
  panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
  
  battstat = g_new0 (ProgressData, 1);

  /* Some starting values... */
  battstat->applet = GTK_WIDGET (applet);
  battstat->refresh_label = TRUE;
  battstat->last_batt_life = 1000;
  battstat->last_acline_status = 1000;
  battstat->last_pixmap_index = 1000;
  battstat->last_charging = 1000;
  battstat->orienttype = panel_applet_get_orient (applet);
  battstat->horizont = TRUE;
  battstat->battery_low_dialog = NULL;
  battstat->battery_low_label = NULL;
  battstat->about_dialog = NULL;
  battstat->pixgc = NULL;

  /* The first received size_allocate event will cause a reconfigure. */
  battstat->height = -1;
  battstat->width = -1;

  load_preferences (battstat);
  create_layout (battstat);

  battstat->pixtimer = gtk_timeout_add (1000, check_for_updates, battstat);

  panel_applet_setup_menu_from_file (PANEL_APPLET (battstat->applet), 
  			             DATADIR,
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

/* Boilerplate... */
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
      

