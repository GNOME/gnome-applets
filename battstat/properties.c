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
#include <libgnomeui/gnome-window-icon.h>
#include <gdk/gdkx.h>

#include <glade/glade.h>
#include <gtk/gtkspinbutton.h>

#include <panel-applet.h>
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

void
prop_apply (GtkWidget *w, int page, gpointer data)
{
  ProgressData *battstat = data;
  const gchar *buffer;

  if (DEBUG) g_print("prop_apply()\n");

  //pixmap_type=-1;

  battstat->yellow_val = GTK_ADJUSTMENT (battstat->eyellow_adj)->value;
  battstat->orange_val = GTK_ADJUSTMENT (battstat->eorange_adj)->value;
  battstat->red_val = GTK_ADJUSTMENT (battstat->ered_adj)->value;

  battstat->lowbattnotification = (GTK_TOGGLE_BUTTON (battstat->lowbatt_toggle))->active;
  battstat->draintop = (GTK_TOGGLE_BUTTON (battstat->progdir_radio))->active;
  battstat->colors_changed=TRUE;
  pixmap_timeout(battstat);
  battstat->colors_changed=FALSE;
  battstat->showstatus = (GTK_TOGGLE_BUTTON (battstat->radio_lay_status_on))->active;
  battstat->showpercent = (GTK_TOGGLE_BUTTON (battstat->radio_lay_percent_on))->active;
  battstat->showbattery = (GTK_TOGGLE_BUTTON (battstat->radio_lay_batt_on))->active;
  battstat->usedock = (GTK_TOGGLE_BUTTON (battstat->dock_toggle))->active;

  if(battstat->usedock) {
    gtk_widget_show(battstat->eventdock);
    gtk_widget_show(battstat->pixmapdockwid);
  } else {
    gtk_widget_hide(battstat->eventdock);    
  }
  buffer = gtk_entry_get_text(GTK_ENTRY(battstat->suspend_entry));
  g_free (battstat->suspend_cmd);
  battstat->suspend_cmd = g_strdup(buffer);

  change_orient ( NULL, battstat->orienttype, battstat );

	save_preferences (battstat);

  return;
  w = NULL;
  page = 0;
}

int
prop_cancel (GtkWidget *w, gpointer data)
{
  if (DEBUG) g_print("prop_cancel()\n");

  return FALSE;
  w = NULL;
}

void
prop_cb (PanelApplet *applet, gpointer data)
{
  ProgressData   *battstat = data;
  GladeXML       *glade_xml;
  GtkWidget      *layout_table;
  GtkWidget      *preview_hbox;
  GtkStyle       *teststyle;
	GtkWidget      *widget;
  GtkWidget      *batterypixwid, *statuspixwid;
  guint           percentage;

  apm_readinfo();

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
     gtk_widget_show (GTK_WIDGET (battstat->prop_win)); 
     gdk_window_raise (GTK_WIDGET (battstat->prop_win)->window); 
     return;
   } 

  glade_xml = glade_xml_new (GLADE_DIR "/battstat_applet.glade", 
                             "battstat_properties", NULL);
  
  battstat->prop_win = GNOME_PROPERTY_BOX (glade_xml_get_widget (glade_xml, 
																																 "battstat_properties"));
	
	gnome_dialog_close_hides (GNOME_DIALOG (battstat->prop_win), TRUE);

  widget = glade_xml_get_widget (glade_xml, "yellow_spin");
  
  battstat->eyellow_adj = GTK_OBJECT (gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)));

   gtk_signal_connect (GTK_OBJECT (battstat->eyellow_adj), "value_changed",
		       GTK_SIGNAL_FUNC (adj_value_changed_cb), battstat);

  widget = glade_xml_get_widget (glade_xml, "orange_spin");
  
  battstat->eorange_adj = GTK_OBJECT (gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)));
	
  gtk_signal_connect (GTK_OBJECT (battstat->eorange_adj), "value_changed",
		      GTK_SIGNAL_FUNC (adj_value_changed_cb), battstat);

  widget = glade_xml_get_widget (glade_xml, "red_spin");
  
  battstat->ered_adj = GTK_OBJECT (gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (widget)));
	
  gtk_signal_connect (GTK_OBJECT (battstat->ered_adj), "value_changed",
		      GTK_SIGNAL_FUNC (adj_value_changed_cb), battstat);

  preview_hbox = glade_xml_get_widget (glade_xml, "preview_hbox");
  
  teststyle = gtk_widget_get_style( GTK_WIDGET (battstat->prop_win) );

  battstat->testpixmap = gdk_pixmap_create_from_xpm_d (battstat->applet->window,
						       &battstat->testmask,
						       &teststyle->bg[GTK_STATE_NORMAL],
						       battery_gray_xpm); 

  battstat->testpixbuffer = gdk_pixmap_create_from_xpm_d (battstat->applet->window,
							  &battstat->testpixmask,
							  &teststyle->bg[GTK_STATE_NORMAL],
							  battery_gray_xpm); 

  battstat->testpixgc = gdk_gc_new (battstat->testpixmap);

  battstat->testpixmapwid = gtk_pixmap_new (battstat->testpixmap, 
					    battstat->testmask );

  gtk_box_pack_start (GTK_BOX (preview_hbox), 
		      GTK_WIDGET (battstat->testpixmapwid),
		      FALSE, TRUE, 0);
	
	battstat->testpercent = glade_xml_get_widget (glade_xml, "preview_label");
  
  widget = glade_xml_get_widget (glade_xml, "preview_hscale");

  battstat->testadj = GTK_OBJECT (gtk_range_get_adjustment (GTK_RANGE (widget)));
  
  gtk_signal_connect (battstat->testadj, "value_changed",
		      (GtkSignalFunc)simul_cb, battstat);
	
  battstat->lowbatt_toggle = glade_xml_get_widget (glade_xml, "lowbatt_toggle");

  battstat->full_toggle = glade_xml_get_widget (glade_xml, "full_toggle");
  battstat->beep_toggle = glade_xml_get_widget (glade_xml, "beep_toggle");
  
  if(battstat->beep) {
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (battstat->beep_toggle), TRUE);
  }
   
  if(battstat->fullbattnot) {
     gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (battstat->full_toggle), TRUE);
  }
  if(battstat->lowbattnotification) {
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (battstat->lowbatt_toggle), TRUE);
  }
  if(battstat->lowbattnotification == 0 && battstat->fullbattnot == 0) {
    gtk_widget_set_sensitive(GTK_WIDGET (battstat->beep_toggle), FALSE);
  }

  gtk_signal_connect (GTK_OBJECT (battstat->lowbatt_toggle), "toggled",
                      GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);

  gtk_signal_connect (GTK_OBJECT (battstat->full_toggle), "toggled",
                      GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);

  gtk_signal_connect (GTK_OBJECT (battstat->beep_toggle), "toggled",
                      GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);

  battstat->suspend_entry = glade_xml_get_widget (glade_xml, "suspend_entry");

  battstat->progdir_radio = glade_xml_get_widget (glade_xml, "dir_radio_top");
	widget = glade_xml_get_widget (glade_xml, "dir_radio_bottom");
	
  if(battstat->draintop) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->progdir_radio), 1);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), 1);
  }
 
  battstat->dock_toggle = glade_xml_get_widget (glade_xml, "dock_toggle");

	layout_table = glade_xml_get_widget (glade_xml, "layout_table");
	
  battstat->radio_lay_batt_on = glade_xml_get_widget (glade_xml, "show_batt_radio");

	widget = glade_xml_get_widget (glade_xml, "hide_batt_radio");
	
  if(battstat->showbattery) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->radio_lay_batt_on), 1);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), 1);
  }

  battstat->radio_lay_status_on = glade_xml_get_widget (glade_xml, "show_status_radio");
	
  widget = glade_xml_get_widget (glade_xml, "hide_status_radio");

  if(battstat->showstatus) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->radio_lay_status_on), 1);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), 1);   
  }

  battstat->radio_lay_percent_on = glade_xml_get_widget (glade_xml, "show_percent_radio");

  widget = glade_xml_get_widget (glade_xml, "hide_percent_radio");

  if(battstat->showpercent) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->radio_lay_percent_on), 1);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), 1);   
  }

  batterypixwid = gtk_pixmap_new (battstat->pixmap, battstat->mask);
  gtk_table_attach (GTK_TABLE (layout_table), batterypixwid, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);

  statuspixwid = gtk_pixmap_new (statusimage[AC], statusmask[AC]);

  gtk_table_attach (GTK_TABLE (layout_table), statuspixwid, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);

  if(battstat->usedock) {
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (battstat->dock_toggle), TRUE);
  }

   gtk_signal_connect (GTK_OBJECT (battstat->dock_toggle), "toggled",
		       GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);

   gtk_signal_connect (GTK_OBJECT(battstat->suspend_entry), "changed",
		       GTK_SIGNAL_FUNC( toggle_value_changed_cb), battstat);
   
   gtk_signal_connect (GTK_OBJECT (battstat->progdir_radio), "toggled",
		       GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);
   
   gtk_signal_connect (GTK_OBJECT (battstat->radio_lay_batt_on), "toggled",
		       GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);
   
   gtk_signal_connect (GTK_OBJECT (battstat->radio_lay_status_on), "toggled",
		       GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);
   
   gtk_signal_connect (GTK_OBJECT (battstat->radio_lay_percent_on), "toggled",
		       GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);
   
   gtk_signal_connect (GTK_OBJECT (battstat->prop_win), "apply",
		       GTK_SIGNAL_FUNC (prop_apply), battstat);
   
   gtk_signal_connect (GTK_OBJECT (battstat->prop_win), "destroy",
		       GTK_SIGNAL_FUNC (prop_cancel), battstat);
   
   gtk_signal_connect (GTK_OBJECT (battstat->prop_win), "delete_event",
		       GTK_SIGNAL_FUNC (prop_cancel), battstat); 
   
   gtk_signal_connect (GTK_OBJECT (battstat->prop_win), "help",
		       GTK_SIGNAL_FUNC (helppref_cb), battstat);
   
   gtk_widget_show_all (GTK_WIDGET (battstat->prop_win));
   
   gtk_adjustment_value_changed(GTK_ADJUSTMENT (battstat->testadj));
}
