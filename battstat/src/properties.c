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

extern void apm_readinfo(void);
extern void adj_value_changed_cb(GtkAdjustment *, gpointer);
extern void toggle_value_changed_cb(GtkToggleButton *, gpointer);
extern void font_set_cb(GtkWidget *, int, gpointer);
extern void simul_cb(GtkWidget *, gpointer);
extern void helppref_cb(AppletWidget *, gpointer);
extern void load_font(gpointer);
extern gint pixmap_timeout(gpointer);
extern void change_orient(GtkWidget *, PanelOrientType, gpointer);
extern guint pixmap_type;

#ifdef __OpenBSD__
extern struct apm_power_info apminfo;
#else
extern struct apm_info apminfo;
#endif

void
prop_apply (GtkWidget *w, int page, gpointer data)
{
  ProgressData *battstat = data;
  gchar *buffer;

  if (DEBUG) g_print("prop_apply()\n");

  pixmap_type=-1;

  battstat->yellow_val = GTK_ADJUSTMENT (battstat->eyellow_adj)->value;
  battstat->orange_val = GTK_ADJUSTMENT (battstat->eorange_adj)->value;
  battstat->red_val = GTK_ADJUSTMENT (battstat->ered_adj)->value;

  battstat->own_font = (GTK_TOGGLE_BUTTON (battstat->font_toggle))->active;

  if(battstat->font_changed) {
    battstat->fontname = gnome_font_picker_get_font_name (GNOME_FONT_PICKER (battstat->fontpicker));
    load_font(battstat);
    battstat->font_changed=FALSE;
  }
  if (battstat->fontname)
    (battstat->percentstyle)->font=gdk_font_load(battstat->fontname);
  else
    (battstat->percentstyle)->font=gdk_font_load ("fixed");
  gtk_widget_set_style (battstat->testpercent, battstat->percentstyle);

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
  if(strlen(battstat->suspend_cmd)>0) {
    applet_widget_callback_set_sensitive (APPLET_WIDGET (battstat->applet),
					  "suspend",
					  TRUE);
    gnome_warning_dialog(_("You have chosen to enable the Suspend function. This can potentionally be a security risc."));
  } else {
    applet_widget_callback_set_sensitive (APPLET_WIDGET (battstat->applet),
					  "suspend",
					  FALSE);
  }

  change_orient ( NULL, battstat->orienttype, battstat );

  applet_widget_sync_config (APPLET_WIDGET (battstat->applet));

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
prop_cb (AppletWidget *applet, gpointer data)
{
  ProgressData *battstat = data;
  GtkWidget *table, *label, *redspin, *orangespin, *yellowspin;
  GtkWidget *frame, *frame1, *hbox, *vbox, *vbox2, *hbox2, *radio;
  GtkWidget *batterypixwid, *statuspixwid, *vbox3;
  GtkStyle *teststyle;
  gchar new_label[10];
  guint percentage;

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
  battstat->prop_win = GNOME_PROPERTY_BOX (gnome_property_box_new ());

  gnome_dialog_close_hides (GNOME_DIALOG (& (battstat->prop_win->dialog)), TRUE);

  gtk_window_set_title (
			GTK_WINDOW (&GNOME_PROPERTY_BOX (battstat->prop_win)->dialog.window),
			_("Battstat Settings"));
 
  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (battstat->prop_win), frame,
				  gtk_label_new (_("General")));
  hbox = gtk_hbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_frame_set_label(GTK_FRAME (frame), (_("Battery color levels (%)")));
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, TRUE, 0);

  vbox3 = gtk_vbox_new (FALSE, 2);
  gtk_container_add(GTK_CONTAINER (frame), vbox3);

  table = gtk_table_new (3, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 10);
  gtk_box_pack_start (GTK_BOX (vbox3), table, FALSE, TRUE, 0);
  label = gtk_label_new(_("Yellow:"));
  battstat->eyellow_adj = gtk_adjustment_new (battstat->eyellow, 1.0, 100, 1, 8, 8 );
  GTK_ADJUSTMENT (battstat->eyellow_adj)->value = battstat->yellow_val;
  yellowspin = gtk_spin_button_new (GTK_ADJUSTMENT (battstat->eyellow_adj), 1, 0);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_table_attach (GTK_TABLE (table), yellowspin, 1, 2, 0, 1,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy ( GTK_SPIN_BUTTON (yellowspin),
				      GTK_UPDATE_ALWAYS );
  gtk_signal_connect (GTK_OBJECT (battstat->eyellow_adj), "value_changed",
		      GTK_SIGNAL_FUNC (adj_value_changed_cb), battstat);

  label = gtk_label_new(_("Orange:"));
  battstat->eorange_adj = gtk_adjustment_new (battstat->eorange, 1.0, 100, 1, 8, 8 );
  GTK_ADJUSTMENT (battstat->eorange_adj)->value = battstat->orange_val;
  orangespin = gtk_spin_button_new (GTK_ADJUSTMENT (battstat->eorange_adj), 1, 0);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_table_attach (GTK_TABLE (table), orangespin, 1, 2, 1, 2,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy ( GTK_SPIN_BUTTON (orangespin),
				      GTK_UPDATE_ALWAYS );
  gtk_signal_connect (GTK_OBJECT (battstat->eorange_adj), "value_changed",
		      GTK_SIGNAL_FUNC (adj_value_changed_cb), battstat);
  label = gtk_label_new(_("Red:"));
  battstat->ered_adj = gtk_adjustment_new (battstat->ered, 1.0, 100, 1, 8, 8 );
  GTK_ADJUSTMENT (battstat->ered_adj)->value = battstat->red_val;
  redspin = gtk_spin_button_new (GTK_ADJUSTMENT (battstat->ered_adj), 1, 0);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_table_attach (GTK_TABLE (table), redspin, 1, 2, 2, 3,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_spin_button_set_update_policy ( GTK_SPIN_BUTTON (redspin),
				      GTK_UPDATE_ALWAYS );
  gtk_signal_connect (GTK_OBJECT (battstat->ered_adj), "value_changed",
		      GTK_SIGNAL_FUNC (adj_value_changed_cb), battstat);

  frame1 = gtk_frame_new (_("Preview"));
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);
  gtk_box_pack_start (GTK_BOX (vbox3), frame1, FALSE, TRUE, 0);
  vbox = gtk_vbox_new(FALSE, 5);
  gtk_container_add (GTK_CONTAINER(frame1), vbox);

  table = gtk_table_new (1, 3, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 5);

  frame = gtk_frame_new (NULL);
  gtk_widget_set_usize( frame, 72, 24);
  gtk_frame_set_shadow_type ( GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
  gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 0, 1,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);

  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (frame), hbox2);

  battstat->testevent = gtk_event_box_new();
  gtk_box_pack_start(GTK_BOX(hbox2), battstat->testevent, FALSE, TRUE, 0);

  teststyle = gtk_widget_get_style( GTK_WIDGET (battstat->prop_win) );
  battstat->testpixmap = gdk_pixmap_create_from_xpm_d( battstat->applet->window,
						       &battstat->testmask,
						       &teststyle->bg[GTK_STATE_NORMAL],
						       battery_gray_xpm ); 
  battstat->testpixbuffer = gdk_pixmap_create_from_xpm_d( battstat->applet->window,
							  &battstat->testpixmask,
							  &teststyle->bg[GTK_STATE_NORMAL],
							  battery_gray_xpm ); 
  battstat->testpixgc=gdk_gc_new(battstat->testpixmap);
  battstat->testpixmapwid = gtk_pixmap_new(battstat->testpixmap, battstat->testmask );
  gtk_container_add (GTK_CONTAINER (battstat->testevent), GTK_WIDGET (battstat->testpixmapwid));

  battstat->testpercent = gtk_label_new ("0%");
  gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (battstat->testpercent), FALSE, TRUE, 0);
  if (battstat->fontname)
    (battstat->percentstyle)->font=gdk_font_load(battstat->fontname);
  else
    (battstat->percentstyle)->font=gdk_font_load ("fixed");
  gtk_widget_set_style (battstat->testpercent, battstat->percentstyle);

  battstat->testadj = gtk_adjustment_new(percentage, 0, 100, 1, 1, 0);
  battstat->testhscale = gtk_hscale_new (GTK_ADJUSTMENT (battstat->testadj));
  gtk_scale_set_draw_value (GTK_SCALE (battstat->testhscale), FALSE);
  gtk_signal_connect(GTK_OBJECT(battstat->testadj), "value_changed",
		     (GtkSignalFunc)simul_cb, battstat);

  gtk_box_pack_start (GTK_BOX (vbox), battstat->testhscale, TRUE, TRUE, 0);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_frame_set_label(GTK_FRAME (frame), (_("Warnings")));
  gtk_box_pack_start (GTK_BOX (vbox2), frame, FALSE, TRUE, 0);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add(GTK_CONTAINER (frame), vbox);
  battstat->lowbatt_toggle = gtk_check_button_new_with_label (_("Notify me when battery charge is low"));
  battstat->full_toggle = gtk_check_button_new_with_label (_("Notify me when battery is fully re-charged"));
  battstat->beep_toggle = gtk_check_button_new_with_label (_("Beep when either of the above is true"));
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
  gtk_box_pack_start(GTK_BOX(vbox), battstat->lowbatt_toggle, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), battstat->full_toggle, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), battstat->beep_toggle, FALSE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (battstat->lowbatt_toggle), "toggled",
                      GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);
  gtk_signal_connect (GTK_OBJECT (battstat->full_toggle), "toggled",
                      GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);
  gtk_signal_connect (GTK_OBJECT (battstat->beep_toggle), "toggled",
                      GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_frame_set_label(GTK_FRAME (frame), (_("Fonts")));
  gtk_box_pack_start (GTK_BOX (vbox2), frame, FALSE, FALSE, 0);

  table = gtk_table_new (3, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 10);
  gtk_table_set_col_spacings (GTK_TABLE (table), 10);
  gtk_container_add (GTK_CONTAINER (frame), table);
  
  battstat->font_toggle = gtk_check_button_new_with_label (_("Use my own font instead of the default GNOME font"));

  battstat->fontlabel = gtk_label_new(_("Percentage text:"));
  battstat->fontpicker = gnome_font_picker_new ();
 
  gnome_font_picker_set_font_name (GNOME_FONT_PICKER (battstat->fontpicker),
				   battstat->fontname);

  gnome_font_picker_set_mode (GNOME_FONT_PICKER (battstat->fontpicker),
			      GNOME_FONT_PICKER_MODE_FONT_INFO);
  gnome_font_picker_fi_set_use_font_in_label (GNOME_FONT_PICKER (battstat->fontpicker),
					      TRUE, 10);

  if(battstat->own_font == 0) {
    gtk_widget_set_sensitive(GTK_WIDGET (battstat->fontpicker), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET (battstat->fontlabel), FALSE);
  }

  gtk_signal_connect (GTK_OBJECT (battstat->fontpicker), "font_set",
		      GTK_SIGNAL_FUNC (font_set_cb), battstat);

  gtk_table_attach (GTK_TABLE (table), battstat->font_toggle, 0, 2, 0, 1,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);  
  gtk_table_attach (GTK_TABLE (table), battstat->fontlabel, 0, 1, 1, 2,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);
  gtk_table_attach (GTK_TABLE (table), battstat->fontpicker, 1, 2, 1, 2,
		    (GtkAttachOptions) (0),
		    (GtkAttachOptions) (0), 0, 0);

  frame = gtk_frame_new (_("Suspend"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX (vbox2), frame, TRUE, TRUE, 0);
  hbox = gtk_hbox_new(FALSE, 2);
  gtk_container_add(GTK_CONTAINER (frame), hbox);
  label = gtk_label_new(_("Suspend command:"));
  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 5);
  battstat->suspend_entry = gtk_entry_new_with_max_length(255);
  gtk_box_pack_start(GTK_BOX(hbox), battstat->suspend_entry, TRUE, TRUE, 5);
  if(strlen(battstat->suspend_cmd)>0) {
    gtk_entry_set_text(GTK_ENTRY(battstat->suspend_entry), 
		       battstat->suspend_cmd);
  }

  frame = gtk_frame_new (NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gnome_property_box_append_page (GNOME_PROPERTY_BOX (battstat->prop_win), frame,
				  gtk_label_new (_("Appearance")));

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(frame), hbox);
  gtk_container_set_border_width (GTK_CONTAINER (hbox),3);

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

  frame = gtk_frame_new (_("Progress bar direction"));
  gtk_box_pack_start(GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  vbox2 = gtk_vbox_new(FALSE, 2);
  gtk_container_add(GTK_CONTAINER(frame), vbox2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox2), 5);
  battstat->progdir_radio = gtk_radio_button_new_with_label (NULL, _("Move towards top"));
  radio = gtk_radio_button_new_with_label_from_widget
    (GTK_RADIO_BUTTON (battstat->progdir_radio), _("Move towards bottom"));
  if(battstat->draintop) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->progdir_radio), 1);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), 1);   
  }
 
  gtk_box_pack_start(GTK_BOX (vbox2), battstat->progdir_radio, FALSE, FALSE, 0); 
  gtk_box_pack_start(GTK_BOX (vbox2), radio, FALSE, FALSE, 0);

  frame = gtk_frame_new (_("Status dock"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width ( GTK_CONTAINER (frame), 5);
  battstat->dock_toggle = gtk_check_button_new_with_label (_("Use the status dock"));
  gtk_container_add (GTK_CONTAINER (frame), battstat->dock_toggle);

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);

  frame = gtk_frame_new (_("Applet layout"));
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width ( GTK_CONTAINER (frame), 5);
  table = gtk_table_new (4, 3, FALSE);
  gtk_container_add (GTK_CONTAINER (frame), table);

  snprintf(new_label, sizeof(new_label),"%d%%", percentage);

  label = gtk_label_new (new_label);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  battstat->radio_lay_batt_on = gtk_radio_button_new_with_label(NULL, NULL);
  gtk_table_attach (GTK_TABLE (table),
		    battstat->radio_lay_batt_on,
		    1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  radio = gtk_radio_button_new_with_label_from_widget
    (GTK_RADIO_BUTTON (battstat->radio_lay_batt_on), NULL);
  if(battstat->showbattery) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->radio_lay_batt_on), 1);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), 1);   
  }
  gtk_table_attach (GTK_TABLE (table), radio, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  battstat->radio_lay_status_on = gtk_radio_button_new_with_label(NULL, NULL);
  gtk_table_attach (GTK_TABLE (table), battstat->radio_lay_status_on, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  radio = gtk_radio_button_new_with_label_from_widget
    (GTK_RADIO_BUTTON (battstat->radio_lay_status_on), NULL);
  if(battstat->showstatus) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->radio_lay_status_on), 1);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), 1);   
  }
  gtk_table_attach (GTK_TABLE (table), radio, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  battstat->radio_lay_percent_on = gtk_radio_button_new_with_label(NULL, NULL);
  gtk_table_attach (GTK_TABLE (table), battstat->radio_lay_percent_on, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  radio = gtk_radio_button_new_with_label_from_widget
    (GTK_RADIO_BUTTON (battstat->radio_lay_percent_on), NULL);
  if(battstat->showpercent) {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (battstat->radio_lay_percent_on), 1);
  } else {
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), 1);   
  }
  gtk_table_attach (GTK_TABLE (table), radio, 2, 3, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  label = gtk_label_new (_("Show"));
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  label = gtk_label_new (_("Hide"));
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  batterypixwid = gtk_pixmap_new (battstat->pixmap, battstat->mask );
  gtk_table_attach (GTK_TABLE (table), batterypixwid, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);
  statuspixwid = gtk_pixmap_new (statusimage[AC],
				 statusmask[AC] );
  gtk_table_attach (GTK_TABLE (table), statuspixwid, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (GTK_EXPAND), 0, 0);

  if(battstat->usedock) {
    gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (battstat->dock_toggle), TRUE);
  }
  gtk_signal_connect (GTK_OBJECT (battstat->dock_toggle), "toggled",
                      GTK_SIGNAL_FUNC (toggle_value_changed_cb), battstat);
  gtk_signal_connect (GTK_OBJECT (battstat->font_toggle), "toggled",
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

  return;
}
