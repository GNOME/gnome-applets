/*
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "battstat-applet.h"

#include <stdio.h>

#include <sys/types.h>
#include <sys/wait.h>
#ifdef HAVE_ERR_H
#include <err.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include <libgnome-panel/gp-utils.h>
#include <libnotify/notify.h>

#include "battstat.h"
#include "battstat-upower.h"
#include "pixmaps.h"

G_DEFINE_TYPE (BattstatApplet, battstat_applet, GP_TYPE_APPLET)

static gboolean check_for_updates (gpointer data);
static void about_cb( GSimpleAction *, GVariant *, gpointer );
static void help_cb( GSimpleAction *, GVariant *, gpointer );

static const GActionEntry battstat_menu_actions [] = {
	{ "preferences", prop_cb,  NULL, NULL, NULL },
	{ "help",        help_cb,  NULL, NULL, NULL },
	{ "about",       about_cb, NULL, NULL, NULL },
	{ NULL }
};

#define AC_POWER_STRING _("System is running on AC power")
#define DC_POWER_STRING _("System is running on battery power")

/* The icons for Battery, Critical, AC and Charging */
static GdkPixbuf *statusimage[STATUS_PIXMAP_NUM];

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
  statusimage[STATUS_PIXMAP_BATTERY] =
    gdk_pixbuf_new_from_xpm_data ((const char **) battery_small_xpm);

  statusimage[STATUS_PIXMAP_METER] =
    gdk_pixbuf_new_from_xpm_data ((const char **) battery_small_meter_xpm);

  statusimage[STATUS_PIXMAP_AC] =
    gdk_pixbuf_new_from_xpm_data ((const char **) ac_small_xpm);
   
  statusimage[STATUS_PIXMAP_CHARGE] =
    gdk_pixbuf_new_from_xpm_data ((const char **) charge_small_xpm);
   
  statusimage[STATUS_PIXMAP_WARNING] =
    gdk_pixbuf_new_from_xpm_data ((const char **) warning_small_xpm);
}

/* Our backends may be either event driven or poll-based.
 * If they are event driven then we know this the first time we
 * receive an event.
 */
static gboolean event_driven = FALSE;
static GSList *instances;

static void
status_change_callback (void)
{
  GSList *instance;

  for (instance = instances; instance; instance = instance->next)
  {
    ProgressData *battstat = instance->data;

    if (battstat->timeout_id)
    {
      g_source_remove (battstat->timeout_id);
      battstat->timeout_id = 0;
    }

    check_for_updates (battstat);
  }

  event_driven = TRUE;
}

/* The following two functions keep track of how many instances of the applet
   are currently running.  When the first instance is started, some global
   initialisation is done.  When the last instance exits, cleanup occurs.

   The teardown code here isn't entirely complete (for example, it doesn't
   deallocate the GdkColors or free the GdkPixmaps.  This is OK so long
   as the process quits immediately when the last applet is removed (which
   it does.)
*/
static const char *
static_global_initialisation (ProgressData *battstat)
{
  gboolean first_time;
  const char *err;

  first_time = !instances;

  instances = g_slist_prepend (instances, battstat);

  if (!first_time)
    return NULL;

  initialise_global_pixmaps();
  err = battstat_upower_initialise (status_change_callback);

  return err;
}

static void
static_global_teardown (ProgressData *battstat)
{
  instances = g_slist_remove (instances, battstat);

  /* remaining instances... */
  if (instances)
    return;

  /* instances == 0 */

  battstat_upower_cleanup();
}

/* Pop up an error dialog on the same screen as 'applet' saying 'msg'.
 */
static void
battstat_error_dialog( GtkWidget *applet, const char *msg )
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new( NULL, 0, GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_OK, "%s", msg);

  gtk_window_set_screen( GTK_WINDOW (dialog),
                         gtk_widget_get_screen (GTK_WIDGET (applet)) );

  g_signal_connect_swapped( G_OBJECT (dialog), "response",
                            G_CALLBACK (gtk_widget_destroy),
                            G_OBJECT (dialog) );

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
				return g_strdup_printf (dngettext (
						GETTEXT_PACKAGE,
						"%d minute (%d%%) remaining",
						"%d minutes (%d%%) remaining",
						mins), mins, info->percent);
			else
				return g_strdup_printf (dngettext (
						GETTEXT_PACKAGE,
						"%d minute until charged (%d%%)",
						"%d minutes until charged (%d%%)",
						mins), mins, info->percent);
		else if (mins == 0)
			if (!info->on_ac_power)
				return g_strdup_printf (dngettext (
						GETTEXT_PACKAGE,
						"%d hour (%d%%) remaining",
						"%d hours (%d%%) remaining",
						hours), hours, info->percent);
			else
				return g_strdup_printf (dngettext (
						GETTEXT_PACKAGE,
						"%d hour until charged (%d%%)",
						"%d hours until charged (%d%%)",
						hours), hours, info->percent);
		else
			if (!info->on_ac_power)
				/* TRANSLATOR: "%d %s %d %s" are "%d hours %d minutes"
				 * Swap order with "%2$s %2$d %1$s %1$d if needed */
				return g_strdup_printf (_("%d %s %d %s (%d%%) remaining"),
						hours, dngettext (GETTEXT_PACKAGE, "hour", "hours", hours),
						mins, dngettext (GETTEXT_PACKAGE, "minute", "minutes", mins),
						info->percent);
			else
				/* TRANSLATOR: "%d %s %d %s" are "%d hours %d minutes"
				 * Swap order with "%2$s %2$d %1$s %1$d if needed */
				return g_strdup_printf (_("%d %s %d %s until charged (%d%%)"),
						hours, dngettext (GETTEXT_PACKAGE, "hour", "hours", hours),
						mins, dngettext (GETTEXT_PACKAGE, "minute", "minutes", mins),
						info->percent);
}

static gboolean
battery_full_notify (GtkWidget *applet)
{
	NotifyNotification *n;
	GError *error = NULL;
	GdkPixbuf *icon;
	gboolean result;
	
	if (!notify_is_initted () && !notify_init (_("Battery Monitor")))
		return FALSE;

  	icon = gtk_icon_theme_load_icon (
			gtk_icon_theme_get_default (),
			"battery",
			48,
			GTK_ICON_LOOKUP_USE_BUILTIN,
			NULL);

	n = notify_notification_new (_("Your battery is now fully recharged"), "", /* "battery" */ NULL);

	/* XXX: it would be nice to pass this as a named icon */
	notify_notification_set_icon_from_pixbuf (n, icon);
	g_object_unref (icon);

	result = notify_notification_show (n, &error);
	
	if (error)
	{
	   g_warning ("%s", error->message);
	   g_error_free (error);
	}

	g_object_unref (G_OBJECT (n));

	return result;
}

/* Show a dialog notifying the user that their battery is done charging.
 */
static void
battery_full_dialog (GtkWidget *applet)
{
  GtkWidget *dialog, *hbox, *image, *label;
  GdkPixbuf *pixbuf;
  gchar *new_label;

  /* first attempt to use libnotify */
  if (battery_full_notify (applet))
	  return;

  dialog = gtk_dialog_new_with_buttons (
		_("Battery Notice"),
		NULL,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		NULL);
  g_signal_connect_swapped (G_OBJECT (dialog), "response",
			    G_CALLBACK (gtk_widget_destroy),
			    G_OBJECT (dialog));

  gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  pixbuf = gtk_icon_theme_load_icon (
		gtk_icon_theme_get_default (),
		"battery",
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
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), hbox);
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

/* Determine if suspend is unsupported.  For the time being this involves
 * distribution-specific magic :(
 */
/* #define HAVE_PMI */
static gboolean
is_suspend_unavailable( void )
{
#ifdef HAVE_PMI
  int status;

  status = system( "pmi query suspend" );

  /* -1 - fail (pmi unavailable?).     return 'false' since we don't know.
   * 0  - success (can suspend).       return 'false' since not unavailable.
   * 1  - success (cannot suspend).    return 'true' since unavailable.
   */
  if( WEXITSTATUS( status ) == 1 )
    return TRUE;
  else
    return FALSE;
#else
  return FALSE; /* return 'false' since we don't know. */
#endif
}

/* Update the text label in the battery low dialog.
 */
static void
battery_low_update_text( ProgressData *battstat, BatteryStatus *info )
{
  const char *suggest;
  gchar *remaining, *new_label;
  GtkRequisition size;

  /* If we're not displaying the dialog then don't update it. */
  if( battstat->battery_low_label == NULL ||
      battstat->battery_low_dialog == NULL )
    return;

  gtk_widget_get_preferred_size (GTK_WIDGET (battstat->battery_low_label), NULL, &size);

  /* If the label has never been set before, the width will be 0.  If it
     has been set before (width > 0) then we want to keep the size of
     the old widget (to keep the dialog from changing sizes) so we set it
     explicitly here.
   */
  if( size.width > 0 )
    gtk_widget_set_size_request( GTK_WIDGET( battstat->battery_low_label ),
                                 size.width, size.height );

  if (info->minutes < 0 && !info->on_ac_power)
  {
	  /* we don't know the remaining time */
	  remaining = g_strdup_printf (_("You have %d%% of your total battery "
				  	 "capacity remaining."), info->percent);
  }
  else
  {
	  remaining = g_strdup_printf( dngettext(
                                 GETTEXT_PACKAGE,
                                 "You have %d minute of battery power "
				   "remaining (%d%% of the total capacity).",
                                 "You have %d minutes of battery power "
				   "remaining (%d%% of the total capacity).",
                                 info->minutes ),
                               info->minutes,info->percent );
  }

  if( is_suspend_unavailable() )
    /* TRANSLATORS: this is a list, it is left as a single string
     * to allow you to make it appear like a list would in your
     * locale.  This is if the laptop does not support suspend. */
    suggest = _("To avoid losing your work:\n"
                " \xE2\x80\xA2 plug your laptop into external power, or\n"
                " \xE2\x80\xA2 save open documents and shut your laptop down."
               );
  else
    /* TRANSLATORS: this is a list, it is left as a single string
     * to allow you to make it appear like a list would in your
     * locale.  This is if the laptop supports suspend. */
    suggest = _("To avoid losing your work:\n"
                " \xE2\x80\xA2 suspend your laptop to save power,\n"
                " \xE2\x80\xA2 plug your laptop into external power, or\n"
                " \xE2\x80\xA2 save open documents and shut your laptop down."
               );

  new_label = g_strdup_printf(
		"<span weight=\"bold\" size=\"larger\">%s</span>\n\n%s\n\n%s",
		_("Your battery is running low"), remaining, suggest );

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
  gtk_dialog_set_default_response( GTK_DIALOG (battery->battery_low_dialog),
                                   GTK_RESPONSE_ACCEPT );

  g_signal_connect_swapped( G_OBJECT (battery->battery_low_dialog),
                            "response",
                            G_CALLBACK (battery_low_dialog_destroy),
                            battery );

  gtk_container_set_border_width (GTK_CONTAINER (battery->battery_low_dialog),
		  6);
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
		 "battery",
		 48,
		 GTK_ICON_LOOKUP_USE_BUILTIN,
		 NULL);
  image = gtk_image_new_from_pixbuf (pixbuf);
  g_object_unref (pixbuf);
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 6);
  gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
  label = gtk_label_new ("");
  battery->battery_low_label = GTK_LABEL( label );
  gtk_label_set_line_wrap( battery->battery_low_label, TRUE );
  gtk_label_set_selectable( battery->battery_low_label, TRUE );
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 6);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area (GTK_DIALOG (battery->battery_low_dialog))), hbox);
	 
  gtk_window_set_keep_above (GTK_WINDOW (battery->battery_low_dialog), TRUE);
  gtk_window_stick (GTK_WINDOW (battery->battery_low_dialog));
  gtk_window_set_focus_on_map (GTK_WINDOW (battery->battery_low_dialog),
		  FALSE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (battery->battery_low_dialog),
		  TRUE);

  battery_low_update_text( battery, info );

  gtk_window_set_position (GTK_WINDOW (battery->battery_low_dialog),
		  GTK_WIN_POS_CENTER);
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

  gtk_widget_set_tooltip_text (GTK_WIDGET (battstat), tiptext);
  g_free (tiptext);
}

static void
pixbuf_draw_line( GdkPixbuf *pixbuf, GdkColor *colour, int x1, int y1, int x2, int y2)
{
  guchar *pixels = gdk_pixbuf_get_pixels (pixbuf);
  int stride = gdk_pixbuf_get_rowstride (pixbuf);
  int channels = gdk_pixbuf_get_n_channels (pixbuf);
  guchar r, g, b, a;
  int i, n;

  r = colour->red >> 8;
  g = colour->green >> 8;
  b = colour->blue >> 8;
  a = 0xFF;

  pixels += stride * y1 + 4 * x1;

  if (x1 == x2)
  {
    /* stride = gdk_pixbuf_get_rowstride (pixbuf); */
    n = y2 - y1;
  }
  else if (y1 == y2)
  {
    stride = gdk_pixbuf_get_n_channels (pixbuf);
    n = x2 - x1;
  }
  else
    g_assert_not_reached ();

  for (i = 0; i < n; i++)
  {
    pixels[0] = r;
    pixels[1] = g;
    pixels[2] = b;
    if (channels == 4)
      pixels[3] = a;

    pixels += stride;
  }
}

/* Redraw the battery meter image.
 */
static void
update_battery_image (ProgressData *battstat, int batt_percent, int batt_time)
{
  GdkColor *color, *darkcolor;
  GdkPixbuf *pixbuf;
  guint progress_value;
  guint i;
  gint x;
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

  /* Depending on if the meter is horizontally oriented start out with the
     appropriate XPM image (from pixmaps.h)
  */
  if (battstat->horizont)
    pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) battery_gray_xpm);
  else
    pixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) battery_y_gray_xpm);

  /* The core code responsible for painting the battery meter.  For each
     colour in our gradient array, draw a vertical or horizontal line
     depending on the current orientation of the meter.
  */
  if (battstat->draintop) {
    progress_value = PROGLEN * batt_life / 100.0;
	    
    for( i = 0; i < G_N_ELEMENTS( orange ); i++ )
    {
      if (battstat->horizont)
        pixbuf_draw_line (pixbuf, &color[i], pixel_offset_top[i], i + 2,
                       pixel_offset_top[i] + progress_value, i + 2);
      else
        pixbuf_draw_line (pixbuf, &color[i], i + 2, pixel_offset_top[i],
                       i + 2, pixel_offset_top[i] + progress_value);
    }
  }
  else
  {
    progress_value = PROGLEN * batt_life / 100.0;

    for( i = 0; i < G_N_ELEMENTS( orange ); i++)
    {
      if (battstat->horizont)
        pixbuf_draw_line (pixbuf, &color[i], pixel_offset_bottom[i], i + 2,
                          pixel_offset_bottom[i] - progress_value, i + 2);
      else
        pixbuf_draw_line (pixbuf, &color[i], i + 2,
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
        if (battstat->horizont)
          pixbuf_draw_line (pixbuf, &darkcolor[i],
                            pixel_offset_bottom[i] - progress_value - 1,
                            i + 2, x, i + 2);
        else
          pixbuf_draw_line (pixbuf, &darkcolor[i], i + 2,
                            pixel_offset_bottom[i] - progress_value - 1,
                            i + 2, x);
      }
    }
  }

  /* Store our newly created pixmap into the GtkImage.  This results in
     the last reference to the old pixmap/mask being dropped.
  */
  gtk_image_set_from_pixbuf( GTK_IMAGE(battstat->battery),
                             pixbuf);

  /* The GtkImage does not assume a reference to the pixmap or mask;
     you still need to unref them if you own references. GtkImage will
     add its own reference rather than adopting yours.
  */
  g_object_unref( G_OBJECT(pixbuf) );
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
    GdkPixbuf *meter;
    guint progress_value;
    gint i, x;

    /* Pull in a clean version of the icons so that we don't paint over
       top of the same icon over and over.
     */
    meter = gdk_pixbuf_copy ( statusimage[STATUS_PIXMAP_METER]);

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
      if( i >= 2 && i <= 7 )
	x = 17;
      else
	x = 16;

      pixbuf_draw_line( meter, &colour[(i * 13 / 10)],
                        i + 1, x - progress_value,
                        i + 1, x );
    }

    /* force a redraw immediately */
    gtk_image_set_from_pixbuf( GTK_IMAGE (battstat->status),
                               meter);

    /* free our private pixmap copy */
    g_object_unref( G_OBJECT( meter ) );
    battstat->last_pixmap_index = STATUS_PIXMAP_METER;
  }
  else if( pixmap_index != battstat->last_pixmap_index )
  {
    gtk_image_set_from_pixbuf (GTK_IMAGE (battstat->status),
                               statusimage[pixmap_index]);
    battstat->last_pixmap_index = pixmap_index;
  }
}

/* Gets called as a gtk_timeout once per second.  Checks for updates and
   makes any changes as appropriate.
 */
static gboolean
check_for_updates( gpointer data )
{
  ProgressData *battstat = data;
  BatteryStatus info;

  battstat_upower_get_battery_info (&info);

  if (!event_driven)
  {
    int timeout;

    /* if on AC and not event driven scale back the polls to once every 10 */
    if (info.on_ac_power)
      timeout = 10;
    else
      timeout = 2;

    if (timeout != battstat->timeout)
    {
      battstat->timeout = timeout;

      if (battstat->timeout_id)
        g_source_remove (battstat->timeout_id);

      battstat->timeout_id = g_timeout_add_seconds (battstat->timeout,
                                            check_for_updates,
                                            battstat);
    }
  }


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
    if(battstat->fullbattnot)
    {
      battery_full_dialog (GTK_WIDGET (battstat));
 
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
       battstat->last_acline_status != info.on_ac_power ||
       battstat->last_present != info.present ||
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
  battstat->last_present = info.present;

  return TRUE;
}

/* Called when the user selects the 'help' menu item.
 */
static void
help_cb (GSimpleAction *action,
         GVariant      *parameter,
         gpointer       user_data)
{
  gp_applet_show_help (GP_APPLET (user_data), NULL);
}

/* Called when the user selects the 'about' menu item.
 */
static void
about_cb (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

/* Rotate text on side panels.  Called on initial startup and when the
 * orientation changes (ie: the panel we were on moved or we moved to
 * another panel).
 */
static void
setup_text_orientation (ProgressData    *battstat,
                        GtkPositionType  position)
{
  if (position == GTK_POS_LEFT)
    gtk_label_set_angle( GTK_LABEL( battstat->percent ), 90 );
  else if (position == GTK_POS_RIGHT)
    gtk_label_set_angle( GTK_LABEL( battstat->percent ), 270 );
  else
    gtk_label_set_angle( GTK_LABEL( battstat->percent ), 0 );
}

static void
placement_changed_cb (GpApplet        *applet,
                      GtkOrientation   orientation,
                      GtkPositionType  position,
                      BattstatApplet  *self)
{
  /* The applet changing orientation very likely involves the layout
   * being changed to better fit the new shape of the panel.
   */
  setup_text_orientation (self, position);
  reconfigure_layout (self);
}

/* This is delivered when our size has changed.  This happens when the applet
   is just created or if the size of the panel has changed.
*/
static void
size_allocate (GtkWidget     *widget,
               GtkAllocation *allocation,
               ProgressData  *battstat)
{
  /* Ignore the update if we already know. */
  if( battstat->width == allocation->width &&
      battstat->height == allocation->height )
    return;

  battstat->width = allocation->width;
  battstat->height = allocation->height;

  /* The applet changing size could result in the layout changing. */
  reconfigure_layout( battstat );
}

/* Get our settings out of GSettings.
 */
static void
load_preferences(ProgressData *battstat)
{
  battstat->red_val = g_settings_get_int (battstat->settings, KEY_RED_VALUE);
  battstat->red_val = CLAMP (battstat->red_val, 0, 100);
  battstat->red_value_is_time = g_settings_get_boolean (battstat->settings, KEY_RED_VALUE_IS_TIME);

  /* automatically calculate orangle and yellow values from the red value */
  battstat->orange_val = battstat->red_val * ORANGE_MULTIPLIER;
  battstat->orange_val = CLAMP (battstat->orange_val, 0, 100);
  
  battstat->yellow_val = battstat->red_val * YELLOW_MULTIPLIER;
  battstat->yellow_val = CLAMP (battstat->yellow_val, 0, 100);

  battstat->lowbattnotification = g_settings_get_boolean (battstat->settings, KEY_LOW_BATTERY_NOTIFICATION);
  battstat->fullbattnot = g_settings_get_boolean (battstat->settings, KEY_FULL_BATTERY_NOTIFICATION);
  battstat->beep = g_settings_get_boolean (battstat->settings, KEY_BEEP);
  battstat->draintop = g_settings_get_boolean (battstat->settings, KEY_DRAIN_FROM_TOP);
  
  battstat->showstatus = g_settings_get_boolean (battstat->settings, KEY_SHOW_STATUS);
  battstat->showbattery = g_settings_get_boolean (battstat->settings, KEY_SHOW_BATTERY);

  /* for miagration from older versions */
  if (battstat->showstatus && battstat->showbattery)
	  battstat->showbattery = FALSE;
  
  battstat->showtext = g_settings_get_int (battstat->settings, KEY_SHOW_TEXT);
}

static void
grid_layout_attach (GtkGrid        *grid,
                    LayoutLocation  loc,
                    GtkWidget      *child)
{
  gtk_widget_set_hexpand (child, TRUE);
  gtk_widget_set_vexpand (child, TRUE);

  switch( loc )
  {
    case LAYOUT_LONG:
      gtk_grid_attach (grid, child, 1, 0, 1, 2);
      break;

    case LAYOUT_TOPLEFT:
      gtk_grid_attach (grid, child, 0, 0, 1, 1);
      break;

    case LAYOUT_TOP:
      gtk_grid_attach (grid, child, 1, 0, 1, 1);
      break;

    case LAYOUT_LEFT:
      gtk_grid_attach (grid, child, 0, 1, 1, 1);
      break;

    case LAYOUT_CENTRE:
      gtk_grid_attach (grid, child, 1, 1, 1, 1);
      break;

    case LAYOUT_RIGHT:
      gtk_grid_attach (grid, child, 2, 1, 1, 1);
      break;

    case LAYOUT_BOTTOM:
      gtk_grid_attach (grid, child, 1, 2, 1, 1);
      break;

    case LAYOUT_NONE:
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
  switch (gp_applet_get_orientation (GP_APPLET (battstat)))
  {
    case GTK_ORIENTATION_HORIZONTAL:
      up_down_order = TRUE;

      /* Need to be at least 46px tall to do square mode on a horiz. panel */
      if( battstat->height >= 46 )
        do_square = TRUE;
      break;

    case GTK_ORIENTATION_VERTICAL:
      /* We need 64px to fix the text beside anything. */
      if( battstat->showtext )
        needwidth = 64;
      /* We need 48px to fix the icon and battery side-by-side. */
      else
        needwidth = 48;

      if( battstat->width >= needwidth )
        do_square = TRUE;
      break;

    default:
      g_assert_not_reached ();
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
      gtk_container_remove (GTK_CONTAINER (battstat->grid), battstat->percent);
    if( battstat->layout.status )
      gtk_container_remove (GTK_CONTAINER (battstat->grid), battstat->status);
    if( battstat->layout.battery )
      gtk_container_remove (GTK_CONTAINER (battstat->grid), battstat->battery);

    /* Attach the elements to their new locations. */
    grid_layout_attach (GTK_GRID (battstat->grid), c.battery, battstat->battery);
    grid_layout_attach (GTK_GRID (battstat->grid), c.status, battstat->status);
    grid_layout_attach (GTK_GRID (battstat->grid), c.text, battstat->percent);

    gtk_widget_show_all (GTK_WIDGET (battstat));
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
  /* Allocate the four widgets that we need. */
  battstat->grid = gtk_grid_new ();
  battstat->percent = gtk_label_new( "" );
  battstat->status = gtk_image_new();
  battstat->battery = gtk_image_new();

  gtk_grid_set_column_spacing (GTK_GRID (battstat->grid), 2);
  gtk_grid_set_row_spacing (GTK_GRID (battstat->grid), 2);

  gp_add_text_color_class (battstat->percent);

  /* When you first get a pointer to a newly created GtkWidget it has one
     'floating' reference.  When you first add this widget to a container
     the container adds a real reference and removes the floating reference
     if one exists.  Since we insert/remove these widgets from the table
     when our layout is reconfigured, we need to keep our own 'real'
     reference to each widget.  This adds a real reference to each widget
     and "sinks" the floating reference.
  */
  g_object_ref_sink (battstat->status);
  g_object_ref_sink (battstat->percent);
  g_object_ref_sink (battstat->battery);

  /* Let reconfigure_layout know that the table is currently empty. */
  battstat->layout.status = LAYOUT_NONE;
  battstat->layout.text = LAYOUT_NONE;
  battstat->layout.battery = LAYOUT_NONE;

  /* Put the table directly inside the applet and show everything. */
  gtk_container_add (GTK_CONTAINER (battstat), battstat->grid);
  gtk_widget_show_all (GTK_WIDGET (battstat));

  /* Attach all sorts of signals to the applet. */

  g_signal_connect (battstat,
                    "placement-changed",
                    G_CALLBACK(placement_changed_cb),
                    battstat);

  g_signal_connect (battstat,
		    "size_allocate",
   		    G_CALLBACK (size_allocate),
		    battstat);

  return FALSE;
}

static void
battstat_applet_fill (BattstatApplet *self)
{
  ProgressData *battstat;
  AtkObject *atk_widget;
  GAction *action;
  const char *menu_resource;
  const char *err;

  battstat = (ProgressData *) self;

  /* Some starting values... */
  battstat->settings = gp_applet_settings_new (GP_APPLET (self), BATTSTAT_GSCHEMA);
  battstat->refresh_label = TRUE;
  battstat->last_batt_life = 1000;
  battstat->last_acline_status = 1000;
  battstat->last_pixmap_index = 1000;
  battstat->last_charging = 1000;
  battstat->horizont = TRUE;
  battstat->battery_low_dialog = NULL;
  battstat->battery_low_label = NULL;
  battstat->timeout = -1;
  battstat->timeout_id = 0;

  /* The first received size_allocate event will cause a reconfigure. */
  battstat->height = -1;
  battstat->width = -1;

  load_preferences (battstat);
  create_layout (battstat);
  setup_text_orientation (battstat, gp_applet_get_position (GP_APPLET (self)));

  menu_resource = GRESOURCE_PREFIX "/ui/battstat-applet-menu.ui";
  gp_applet_setup_menu_from_resource (GP_APPLET (self),
                                      menu_resource,
                                      battstat_menu_actions);

  action = gp_applet_menu_lookup_action (GP_APPLET (self), "preferences");
  g_object_bind_property (self, "locked-down", action, "enabled",
                          G_BINDING_DEFAULT|G_BINDING_INVERT_BOOLEAN|G_BINDING_SYNC_CREATE);

  atk_widget = gtk_widget_get_accessible (GTK_WIDGET (self));
  if (GTK_IS_ACCESSIBLE (atk_widget)) {
	  atk_object_set_name (atk_widget, _("Battery Charge Monitor"));
	  atk_object_set_description(atk_widget, _("Monitor a laptop's remaining power"));
  }

  if ((err = static_global_initialisation (battstat)))
    battstat_error_dialog (GTK_WIDGET (self), err);
}

static void
battstat_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (battstat_applet_parent_class)->constructed (object);
  battstat_applet_fill (BATTSTAT_APPLET (object));
}

static void
battstat_applet_dispose (GObject *object)
{
  BattstatApplet *self;

  self = BATTSTAT_APPLET (object);

  g_clear_pointer (&self->prop_win, gtk_widget_destroy);

  if (self->battery_low_dialog != NULL)
    {
      battery_low_dialog_destroy (self);
      self->battery_low_dialog = NULL;
    }

  if (self->timeout_id != 0)
    {
      g_source_remove (self->timeout_id);
      self->timeout_id = 0;
    }

  g_clear_object (&self->settings);

  g_clear_object (&self->status);
  g_clear_object (&self->percent);
  g_clear_object (&self->battery);

  static_global_teardown (self);

  G_OBJECT_CLASS (battstat_applet_parent_class)->dispose (object);
}

static void
battstat_applet_class_init (BattstatAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = battstat_applet_constructed;
  object_class->dispose = battstat_applet_dispose;
}

static void
battstat_applet_init (BattstatApplet *self)
{
  gp_applet_set_flags (GP_APPLET (self), GP_APPLET_FLAGS_EXPAND_MINOR);
}

void
battstat_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **documenters;
  const char *copyright;

  comments = _("This utility shows the status of your laptop battery.\n\n"
               "UPower backend enabled.");

  authors = (const char *[])
    {
      "J\xC3\xB6rgen Pehrson <jp@spektr.eu.org>",
      "Lennart Poettering <lennart@poettering.de> (Linux ACPI support)",
      "Seth Nickell <snickell@stanford.edu> (GNOME2 port)",
      "Davyd Madeley <davyd@madeley.id.au>",
      "Ryan Lortie <desrt@desrt.ca>",
      "Joe Marcus Clarke <marcus@FreeBSD.org> (FreeBSD ACPI support)",
      NULL
    };

  documenters = (const char *[])
    {
      "J\xC3\xB6rgen Pehrson <jp@spektr.eu.org>",
      "Trevor Curtis <tcurtis@somaradio.ca>",
      "Davyd Madeley <davyd@madeley.id.au>",
      NULL
    };

  copyright = "\xC2\xA9 2000 The Gnulix Society, "
              "\xC2\xA9 2002-2005 Free Software Foundation and "
              "others";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
