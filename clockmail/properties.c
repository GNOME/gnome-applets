/*###################################################################*/
/*##                         clock & mail applet 0.2.0 alpha       ##*/
/*##       This software is Copyright (C) 1998 by John Ellis.      ##*/
/*## This software falls under the GNU Public License. Please read ##*/
/*##              the COPYING file for more information            ##*/
/*###################################################################*/

#include "clockmail.h"

GtkWidget *propwindow = NULL;
gint P_MILIT_TIME;
gint P_ALWAYS_BLINK;

void property_load(char *path)
{
        gnome_config_push_prefix (path);
        MILIT_TIME   = gnome_config_get_int("hour=0");
	ALWAYS_BLINK = gnome_config_get_int("blink=0");
        gnome_config_pop_prefix ();
}

void property_save(char *path)
{
        gnome_config_push_prefix(path);
        gnome_config_set_int("hour", MILIT_TIME);
        gnome_config_set_int("blink", ALWAYS_BLINK);
	gnome_config_sync();
        gnome_config_pop_prefix();
}

void military_time_cb(GtkWidget *w)
{
	P_MILIT_TIME = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

void always_blink_cb(GtkWidget *w)
{
	P_ALWAYS_BLINK = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

void property_apply_cb( GtkWidget *widget, void *data )
{
        MILIT_TIME = P_MILIT_TIME;
	ALWAYS_BLINK = P_ALWAYS_BLINK;
}

gint property_destroy_cb( GtkWidget *widget, void *data )
{
        propwindow = NULL;
	return FALSE;
}

void property_show(AppletWidget *applet, gpointer data)
{
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;

	if(propwindow)
		{
                gdk_window_raise(propwindow->window);
                return;
		}

	P_MILIT_TIME=MILIT_TIME;
	P_ALWAYS_BLINK = ALWAYS_BLINK;

	propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(propwindow)->dialog.window),
		"ClockMail Settings");
	
	frame = gtk_vbox_new(5, TRUE);

	button = gtk_check_button_new_with_label (_("Display time in 24 hour format"));
	gtk_box_pack_start(GTK_BOX(frame), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), P_MILIT_TIME);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) military_time_cb, NULL);
	gtk_widget_show(button);

	button = gtk_check_button_new_with_label (_("Blink when any mail is waiting. (Not just when mail arrives)"));
	gtk_box_pack_start(GTK_BOX(frame), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), P_ALWAYS_BLINK);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) always_blink_cb, NULL);
	gtk_widget_show(button);

        label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(propwindow),frame ,label);

	gtk_signal_connect( GTK_OBJECT(propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), NULL );
	gtk_signal_connect( GTK_OBJECT(propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), NULL );

        gtk_widget_show_all(propwindow);
} 
