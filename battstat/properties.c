/* -*- Mode: C; tab-width: 2; indent-tabs-mode: t; c-basic-offset: 2 -*- */
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

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk/gdkx.h>

#include <glade/glade.h>
#include <gtk/gtkspinbutton.h>

#include <gconf/gconf-client.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
/*#include <status-docklet.h>*/

#include "battstat.h"

#ifndef gettext_noop
#define gettext_noop(String) (String)
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__)
extern struct apm_power_info apminfo;
#else
extern struct apm_info apminfo;
#endif

#define NEVER_SENSITIVE		"never_sensitive"

/* set sensitive and setup NEVER_SENSITIVE appropriately */
static void
hard_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	gtk_widget_set_sensitive (w, sensitivity);
	g_object_set_data (G_OBJECT (w), NEVER_SENSITIVE,
			   GINT_TO_POINTER ( ! sensitivity));
}


/* set sensitive, but always insensitive if NEVER_SENSITIVE is set */
static void
soft_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	if (g_object_get_data (G_OBJECT (w), NEVER_SENSITIVE))
		gtk_widget_set_sensitive (w, FALSE);
	else
		gtk_widget_set_sensitive (w, sensitivity);
}

static gboolean
key_writable (PanelApplet *applet, const char *key)
{
	gboolean writable;
	char *fullkey;
	static GConfClient *client = NULL;
	if (client == NULL)
		client = gconf_client_get_default ();

	fullkey = panel_applet_gconf_get_full_key (applet, key);

	writable = gconf_client_key_is_writable (client, fullkey, NULL);

	g_free (fullkey);

	return writable;
}

static void
show_bat_toggled (GtkToggleButton *button, gpointer data)
{
  ProgressData   *battstat = data;
  PanelApplet *applet = PANEL_APPLET (battstat->applet);
  gboolean toggled;
  
  toggled = gtk_toggle_button_get_active (button);
 
  if (!( toggled || battstat->showtext || battstat->showstatus)) {
    gtk_toggle_button_set_active (button, !toggled);
    return;
  }

  battstat->showbattery = toggled;
  if (battstat->horizont) 
    battstat->showbattery ?
	gtk_widget_show (battstat->pixmapwid):gtk_widget_hide (battstat->pixmapwid);
  else
    battstat->showbattery ?
	gtk_widget_show (battstat->frameybattery):gtk_widget_hide (battstat->frameybattery);
  panel_applet_gconf_set_bool   (applet, "show_battery", 
  				 battstat->showbattery, NULL);
  				 
}

static void
show_status_toggled (GtkToggleButton *button, gpointer data)
{
  ProgressData   *battstat = data;
  PanelApplet *applet = PANEL_APPLET (battstat->applet);
  gboolean toggled;
  
  toggled = gtk_toggle_button_get_active (button);
  
  if (!( toggled || battstat->showtext || battstat->showbattery)) {
    gtk_toggle_button_set_active (button, !toggled);
    return;
  }
  
  battstat->showstatus = toggled;
	change_orient(applet, battstat->orienttype, battstat);
  
  panel_applet_gconf_set_bool   (applet, "show_status", 
  				 battstat->showstatus, NULL);
  				 
}

static void
show_text_toggled (GtkToggleButton *button, gpointer data)
{
  ProgressData   *battstat = data;
  PanelApplet *applet = PANEL_APPLET (battstat->applet);
  
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (battstat->radio_text_2))
   && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (battstat->check_text)))
	  battstat->showtext = APPLET_SHOW_PERCENT;
  else if (gtk_toggle_button_get_active (
			  GTK_TOGGLE_BUTTON (battstat->radio_text_1)) &&
	   gtk_toggle_button_get_active (
		   	  GTK_TOGGLE_BUTTON (battstat->check_text)))
	  battstat->showtext = APPLET_SHOW_TIME;
  else
	  battstat->showtext = APPLET_SHOW_NONE;
  
  change_orient(applet, battstat->orienttype, battstat);

  gtk_widget_set_sensitive (GTK_WIDGET (battstat->radio_text_1),
		  battstat->showtext);
  gtk_widget_set_sensitive (GTK_WIDGET (battstat->radio_text_2),
		  battstat->showtext);
	
  panel_applet_gconf_set_int   (applet, "show_text", 
  				 battstat->showtext, NULL);
}

static void
suspend_changed (GtkEditable *editable, gpointer data)
{
  ProgressData   *battstat = data;
  PanelApplet *applet = PANEL_APPLET (battstat->applet);
  gchar *cmd;
  
  cmd = gtk_editable_get_chars (editable, 0, -1);
  if (!cmd)
  	return;
  
  if (battstat->suspend_cmd)
    g_free (battstat->suspend_cmd);
  battstat->suspend_cmd = g_strdup(cmd);
  g_free (cmd);
  
  panel_applet_gconf_set_string (applet, "suspend_command", 
  				 battstat->suspend_cmd, NULL);

}

static void
lowbatt_toggled (GtkToggleButton *button, gpointer data)
{
  ProgressData   *battstat = data;
  PanelApplet *applet = PANEL_APPLET (battstat->applet);
  
  battstat->lowbattnotification = gtk_toggle_button_get_active (button);
  panel_applet_gconf_set_bool   (applet,"low_battery_notification", 
  				 battstat->lowbattnotification, NULL);  
}

static void
full_toggled (GtkToggleButton *button, gpointer data)
{
  ProgressData   *battstat = data;
  PanelApplet *applet = PANEL_APPLET (battstat->applet);
  
  battstat->fullbattnot = gtk_toggle_button_get_active (button);
  panel_applet_gconf_set_bool   (applet,"full_battery_notification", 
  				 battstat->fullbattnot, NULL);  
}

static void
prefs_help_cb (GtkWindow *dialog)
{
	GError *error = NULL;
	static GnomeProgram *applet_program = NULL;
	
	if (!applet_program) {
		int argc = 1;
		char *argv[2] = { "battstat" };
		applet_program = gnome_program_init ("battstat", VERSION,
						      LIBGNOME_MODULE, argc, argv,
     						      GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);
	}

	gnome_help_display_desktop_on_screen (
			applet_program, "battstat", "battstat", "battstatt-prefs",
			gtk_widget_get_screen (GTK_WIDGET (dialog)),
			&error);

	if (error) {
		GtkWidget *error_dialog;

		error_dialog = gtk_message_dialog_new (
				NULL,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_OK,
				_("There was an error displaying help: %s"),
				error->message);

		g_signal_connect (error_dialog, "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (error_dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (error_dialog),
				       gtk_widget_get_screen (GTK_WIDGET (dialog)));
		gtk_widget_show (error_dialog);
		g_error_free (error);
	}
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
  ProgressData   *battstat = data;
  
  if (id == GTK_RESPONSE_HELP) {
  	prefs_help_cb (GTK_WINDOW (dialog));
	return;
  }
  
  gtk_widget_hide (GTK_WIDGET (battstat->prop_win));
  
}

void
prop_cb (BonoboUIComponent *uic,
	 ProgressData      *battstat,
	 const char        *verb)
{
  GladeXML  *glade_xml;
  GtkWidget *layout_table;
  GtkWidget *preview_hbox;
  GtkWidget *widget;
  guint      percentage;
  GConfClient *client;
  gboolean   inhibit_command_line;
  AtkObject *atk_widget;

  client = gconf_client_get_default ();
  inhibit_command_line = gconf_client_get_bool (client, "/desktop/gnome/lockdown/inhibit_command_line", NULL);

  apm_readinfo (PANEL_APPLET (battstat->applet), battstat);

#ifdef __FreeBSD__
  percentage = apminfo.ai_batt_life;
  if(percentage == 255) percentage = 0;
#elif defined(__NetBSD__) || defined(__OpenBSD__)
  percentage = apminfo.battery_life;
  if(percentage == 255) percentage = 0;
#elif __linux__
  percentage = apminfo.battery_percentage;
  if(percentage < 0) percentage = 0;
#else
  percentage = 100;
#endif

  if (DEBUG) g_print("prop_cb()\n");

   if (battstat->prop_win) { 
     gtk_window_set_screen (GTK_WINDOW (battstat->prop_win),
			    gtk_widget_get_screen (battstat->applet));
     gtk_window_present (GTK_WINDOW (battstat->prop_win));
     return;
   } 

  glade_xml = glade_xml_new (GNOME_GLADEDIR "/battstat_applet.glade", 
                             "battstat_properties", NULL);
  
  battstat->prop_win = GTK_DIALOG (glade_xml_get_widget (glade_xml, 
  				   "battstat_properties"));
  gtk_window_set_screen (GTK_WINDOW (battstat->prop_win),
			 gtk_widget_get_screen (battstat->applet));

  g_signal_connect (G_OBJECT (battstat->prop_win), "delete_event",
		  G_CALLBACK (gtk_true), NULL);
  
  battstat->lowbatt_toggle = glade_xml_get_widget (glade_xml, "lowbatt_toggle");
  g_signal_connect (G_OBJECT (battstat->lowbatt_toggle), "toggled",
  		    G_CALLBACK (lowbatt_toggled), battstat);

  if ( ! key_writable (PANEL_APPLET (battstat->applet), "low_battery_notification")) {
	  hard_set_sensitive (battstat->lowbatt_toggle, FALSE);
  }

  battstat->full_toggle = glade_xml_get_widget (glade_xml, "full_toggle");
  g_signal_connect (G_OBJECT (battstat->full_toggle), "toggled",
  		    G_CALLBACK (full_toggled), battstat);

  if ( ! key_writable (PANEL_APPLET (battstat->applet), "full_battery_notification")) {
	  hard_set_sensitive (battstat->full_toggle, FALSE);
  }
  if(battstat->fullbattnot) {
     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->full_toggle), TRUE);
  }
  if(battstat->lowbattnotification) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->lowbatt_toggle), TRUE);
  }

  battstat->suspend_entry = glade_xml_get_widget (glade_xml, "suspend_entry");
  g_signal_connect (G_OBJECT(battstat->suspend_entry), "changed",
		    G_CALLBACK(suspend_changed), battstat);

  g_warning ("%s", battstat->suspend_cmd ? battstat->suspend_cmd : "ARSE");
  gtk_entry_set_text (GTK_ENTRY (battstat->suspend_entry), battstat->suspend_cmd); 
  if ( ! key_writable (PANEL_APPLET (battstat->applet), "suspend_command") ||
      inhibit_command_line) {
	  hard_set_sensitive (battstat->suspend_entry, FALSE);
	  hard_set_sensitive (glade_xml_get_widget (glade_xml, "suspend_label"), FALSE);
  }
  layout_table = glade_xml_get_widget (glade_xml, "layout_table");

  battstat->radio_lay_batt_on = glade_xml_get_widget (glade_xml, "show_batt_toggle");

  if ( ! key_writable (PANEL_APPLET (battstat->applet), "show_battery"))
	  hard_set_sensitive (battstat->radio_lay_batt_on, FALSE);
  
  if(battstat->radio_lay_batt_on) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->radio_lay_batt_on), TRUE);
  }
  g_signal_connect (G_OBJECT (battstat->radio_lay_batt_on), "toggled",
  		    G_CALLBACK (show_bat_toggled), battstat);

  battstat->radio_lay_status_on = glade_xml_get_widget (glade_xml, "show_status_toggle");
  g_signal_connect (G_OBJECT (battstat->radio_lay_status_on), "toggled",
  		    G_CALLBACK (show_status_toggled), battstat);

  if ( ! key_writable (PANEL_APPLET (battstat->applet), "show_status"))
	  hard_set_sensitive (battstat->radio_lay_status_on, FALSE);
	
  if(battstat->showstatus) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->radio_lay_status_on), TRUE);
  }

  battstat->radio_text_1 = glade_xml_get_widget (glade_xml, "show_text_radio");
  battstat->radio_text_2 = glade_xml_get_widget (glade_xml,
		  "show_text_radio_2");
  battstat->check_text = glade_xml_get_widget (glade_xml,
		  "show_text_remaining");
  
  g_signal_connect (G_OBJECT (battstat->radio_text_1), "toggled",
		  G_CALLBACK (show_text_toggled), battstat);
  g_signal_connect (G_OBJECT (battstat->radio_text_2), "toggled",
		  G_CALLBACK (show_text_toggled), battstat);
  g_signal_connect (G_OBJECT (battstat->check_text), "toggled",
		  G_CALLBACK (show_text_toggled), battstat);
  
  if ( ! key_writable (PANEL_APPLET (battstat->applet), "show_text"))
  {
	  hard_set_sensitive (battstat->check_text, FALSE);
	  hard_set_sensitive (battstat->radio_text_1, FALSE);
	  hard_set_sensitive (battstat->radio_text_2, FALSE);
  }

  if (battstat->showtext == APPLET_SHOW_PERCENT)
  {
	  gtk_toggle_button_set_active (
			  GTK_TOGGLE_BUTTON (battstat->check_text), TRUE);
	  gtk_toggle_button_set_active (
			  GTK_TOGGLE_BUTTON (battstat->radio_text_2), TRUE);
	  gtk_widget_set_sensitive (GTK_WIDGET (battstat->radio_text_1),
			  TRUE);
	  gtk_widget_set_sensitive (GTK_WIDGET (battstat->radio_text_2),
			  TRUE);
  }
  else if (battstat->showtext == APPLET_SHOW_TIME)
  {
	  gtk_toggle_button_set_active (
			  GTK_TOGGLE_BUTTON (battstat->check_text),
			  TRUE);
	  gtk_toggle_button_set_active (
			  GTK_TOGGLE_BUTTON (battstat->radio_text_1),
			  TRUE);
	  gtk_widget_set_sensitive (GTK_WIDGET (battstat->radio_text_1),
			  TRUE);
	  gtk_widget_set_sensitive (GTK_WIDGET (battstat->radio_text_2),
			  TRUE);
  }
  else /* APPLET_SHOW_NONE */
  {
	  gtk_toggle_button_set_active (
			  GTK_TOGGLE_BUTTON (battstat->check_text), FALSE);
	  gtk_widget_set_sensitive (GTK_WIDGET (battstat->radio_text_1),
			  FALSE);
	  gtk_widget_set_sensitive (GTK_WIDGET (battstat->radio_text_2),
			  FALSE);
  }

   gtk_dialog_set_default_response (GTK_DIALOG (battstat->prop_win), GTK_RESPONSE_CLOSE);
   gtk_window_set_resizable (GTK_WINDOW (battstat->prop_win), FALSE);
   gtk_dialog_set_has_separator (GTK_DIALOG (battstat->prop_win), FALSE);
   
   g_signal_connect (G_OBJECT (battstat->prop_win), "response",
   		     G_CALLBACK (response_cb), battstat);
   gtk_widget_show_all (GTK_WIDGET (battstat->prop_win));
}
