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

#ifdef __OpenBSD__
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
 
  if (!( toggled || battstat->showpercent || battstat->showstatus)) {
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
  
  if (!( toggled || battstat->showpercent || battstat->showbattery)) {
    gtk_toggle_button_set_active (button, !toggled);
    return;
  }
  
  battstat->showstatus = toggled;
  battstat->showstatus ?
	gtk_widget_show (battstat->statuspixmapwid):
	gtk_widget_hide (battstat->statuspixmapwid);
  
  panel_applet_gconf_set_bool   (applet, "show_status", 
  				 battstat->showstatus, NULL);
  				 
}

static void
show_percent_toggled (GtkToggleButton *button, gpointer data)
{
  ProgressData   *battstat = data;
  PanelApplet *applet = PANEL_APPLET (battstat->applet);
  gboolean toggled;
  
  toggled = gtk_toggle_button_get_active (button);
  
  if (!( toggled || battstat->showstatus || battstat->showbattery)) {
    gtk_toggle_button_set_active (button, !toggled);
    return;
  }
  
  battstat->showpercent = toggled;
  if (battstat->horizont) 
    battstat->showpercent ?
	gtk_widget_show (battstat->percent):gtk_widget_hide (battstat->percent);
  else
    battstat->showpercent ?
	gtk_widget_show (battstat->statuspercent):gtk_widget_hide (battstat->statuspercent);
  panel_applet_gconf_set_bool   (applet, "show_percent", 
  				 battstat->showpercent, NULL);
  				 
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
  if(battstat->lowbattnotification || battstat->fullbattnot)
     soft_set_sensitive(GTK_WIDGET (battstat->beep_toggle), TRUE);
  else
     soft_set_sensitive(GTK_WIDGET (battstat->beep_toggle), FALSE);
}

static void
full_toggled (GtkToggleButton *button, gpointer data)
{
  ProgressData   *battstat = data;
  PanelApplet *applet = PANEL_APPLET (battstat->applet);
  
  battstat->fullbattnot = gtk_toggle_button_get_active (button);
  panel_applet_gconf_set_bool   (applet,"full_battery_notification", 
  				 battstat->fullbattnot, NULL);  
  			
  if(battstat->lowbattnotification || battstat->fullbattnot)
     soft_set_sensitive(GTK_WIDGET (battstat->beep_toggle), TRUE);
  else
     soft_set_sensitive(GTK_WIDGET (battstat->beep_toggle), FALSE);
}

static void
beep_toggled (GtkToggleButton *button, gpointer data)
{
  ProgressData   *battstat = data;
  PanelApplet *applet = PANEL_APPLET (battstat->applet);
  
  battstat->beep = gtk_toggle_button_get_active (button);
  panel_applet_gconf_set_bool   (applet,"beep", 
  				 battstat->beep, NULL);  
}  

static void
progdir_toggled (GtkToggleButton *button, gpointer data)
{
  ProgressData   *battstat = data;
  PanelApplet *applet = PANEL_APPLET (battstat->applet);
  
  battstat->draintop = gtk_toggle_button_get_active (button); 
  panel_applet_gconf_set_bool (applet,"drain_from_top", 
  			       battstat->draintop, NULL); 

  battstat->colors_changed = TRUE;			       
  pixmap_timeout(battstat); 
  battstat->colors_changed = FALSE;				 
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

	egg_help_display_desktop_on_screen (
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
  gboolean   writable;
  GConfClient *client;
  gboolean   inhibit_command_line;

  client = gconf_client_get_default ();
  inhibit_command_line = gconf_client_get_bool (client, "/desktop/gnome/lockdown/inhibit_command_line", NULL);

  apm_readinfo (PANEL_APPLET (battstat->applet), battstat);

#ifdef __FreeBSD__
  percentage = apminfo.ai_batt_life;
  if(percentage == 255) percentage = 0;
#elif __OpenBSD__
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

  glade_xml = glade_xml_new (GLADE_DIR "/battstat_applet.glade", 
                             "battstat_properties", NULL);
  
  battstat->prop_win = GTK_DIALOG (glade_xml_get_widget (glade_xml, 
  				   "battstat_properties"));
  gtk_window_set_screen (GTK_WINDOW (battstat->prop_win),
			 gtk_widget_get_screen (battstat->applet));

  writable = key_writable (PANEL_APPLET (battstat->applet), "red_value");
  writable = writable && key_writable (PANEL_APPLET (battstat->applet), "orange_value");
  writable = writable && key_writable (PANEL_APPLET (battstat->applet), "yellow_value");
	
  widget = glade_xml_get_widget (glade_xml, "yellow_spin");
  if ( ! writable) {
	  hard_set_sensitive (widget, FALSE);
	  hard_set_sensitive (glade_xml_get_widget (glade_xml, "yellow_label"), FALSE);
  }
  
  battstat->eyellow_adj = GTK_OBJECT (gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)));

  g_signal_connect (G_OBJECT (battstat->prop_win), "delete_event", G_CALLBACK (gtk_true), NULL);

  g_signal_connect (G_OBJECT (battstat->eyellow_adj), "value_changed",
										G_CALLBACK (adj_value_changed_cb), battstat);

  widget = glade_xml_get_widget (glade_xml, "orange_spin");
  if ( ! writable) {
	  hard_set_sensitive (widget, FALSE);
	  hard_set_sensitive (glade_xml_get_widget (glade_xml, "orange_label"), FALSE);
  }
  
  battstat->eorange_adj = GTK_OBJECT (gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)));
	
  g_signal_connect (G_OBJECT (battstat->eorange_adj), "value_changed",
										G_CALLBACK (adj_value_changed_cb), battstat);

  widget = glade_xml_get_widget (glade_xml, "red_spin");
  if ( ! writable) {
	  hard_set_sensitive (widget, FALSE);
	  hard_set_sensitive (glade_xml_get_widget (glade_xml, "red_label"), FALSE);
  }
  
  battstat->ered_adj = GTK_OBJECT (gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)));
	
  g_signal_connect (G_OBJECT (battstat->ered_adj), "value_changed",
										G_CALLBACK (adj_value_changed_cb), battstat);

  preview_hbox = glade_xml_get_widget (glade_xml, "preview_hbox");

  battstat->testpixmap = gdk_pixmap_create_from_xpm_d (battstat->applet->window,
						       &battstat->testmask,
						       NULL,
						       battery_gray_xpm); 

  battstat->testpixbuffer = gdk_pixmap_create_from_xpm_d (battstat->applet->window,
							  &battstat->testpixmask,
							  NULL,
							  battery_gray_xpm); 

  battstat->testpixgc = gdk_gc_new (battstat->testpixmap);

  battstat->testpixmapwid = gtk_image_new_from_pixmap (battstat->testpixmap, 
																											 battstat->testmask);

  gtk_box_pack_start (GTK_BOX (preview_hbox), 
		      GTK_WIDGET (battstat->testpixmapwid),
		      FALSE, TRUE, 0);
	
  battstat->testpercent = glade_xml_get_widget (glade_xml, "preview_label");
  
  widget = glade_xml_get_widget (glade_xml, "preview_hscale");

  battstat->testadj = GTK_OBJECT (gtk_range_get_adjustment (GTK_RANGE (widget)));
  
  g_signal_connect (G_OBJECT(battstat->testadj), "value_changed",
										G_CALLBACK(simul_cb), battstat);
	
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
  		    
  battstat->beep_toggle = glade_xml_get_widget (glade_xml, "beep_toggle");
  g_signal_connect (G_OBJECT (battstat->beep_toggle), "toggled",
  		    G_CALLBACK (beep_toggled), battstat);

  if ( ! key_writable (PANEL_APPLET (battstat->applet), "beep")) {
	  hard_set_sensitive (battstat->beep_toggle, FALSE);
  }
  
  if(battstat->beep) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->beep_toggle), TRUE);
  }
   
  if(battstat->fullbattnot) {
     gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->full_toggle), TRUE);
  }
  if(battstat->lowbattnotification) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->lowbatt_toggle), TRUE);
  }
  if(battstat->lowbattnotification == 0 && battstat->fullbattnot == 0) {
    soft_set_sensitive(GTK_WIDGET (battstat->beep_toggle), FALSE);
  }

  battstat->suspend_entry = glade_xml_get_widget (glade_xml, "suspend_entry");
  g_signal_connect (G_OBJECT(battstat->suspend_entry), "changed",
		    G_CALLBACK(suspend_changed), battstat);

  if ( ! key_writable (PANEL_APPLET (battstat->applet), "suspend_command") ||
      inhibit_command_line) {
	  hard_set_sensitive (battstat->suspend_entry, FALSE);
	  hard_set_sensitive (glade_xml_get_widget (glade_xml, "suspend_label"), FALSE);
  }

  battstat->progdir_radio = glade_xml_get_widget (glade_xml, "dir_radio_top");
  g_signal_connect (G_OBJECT (battstat->progdir_radio), "toggled",
  		    G_CALLBACK (progdir_toggled), battstat);
  widget = glade_xml_get_widget (glade_xml, "dir_radio_bottom");

  if ( ! key_writable (PANEL_APPLET (battstat->applet), "drain_from_top")) {
	  hard_set_sensitive (battstat->progdir_radio, FALSE);
	  hard_set_sensitive (widget, FALSE);
  }
	
  if(battstat->draintop) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->progdir_radio), 1);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), 1);
  }
#if 0 
  battstat->dock_toggle = glade_xml_get_widget (glade_xml, "dock_toggle");
#endif
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

  battstat->radio_lay_percent_on = glade_xml_get_widget (glade_xml, "show_percent_toggle");
  g_signal_connect (G_OBJECT (battstat->radio_lay_percent_on), "toggled",
  		    G_CALLBACK (show_percent_toggled), battstat);
  if ( ! key_writable (PANEL_APPLET (battstat->applet), "show_percent"))
	  hard_set_sensitive (battstat->radio_lay_percent_on, FALSE);

  if(battstat->showpercent) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->radio_lay_percent_on), TRUE);
  }

   gtk_dialog_set_default_response (GTK_DIALOG (battstat->prop_win), GTK_RESPONSE_CLOSE);
   gtk_window_set_resizable (GTK_WINDOW (battstat->prop_win), FALSE);
   gtk_dialog_set_has_separator (GTK_DIALOG (battstat->prop_win), FALSE);
   
   g_signal_connect (G_OBJECT (battstat->prop_win), "response",
   		     G_CALLBACK (response_cb), battstat);
   gtk_widget_show_all (GTK_WIDGET (battstat->prop_win));
   
   gtk_adjustment_value_changed(GTK_ADJUSTMENT (battstat->testadj));
}
