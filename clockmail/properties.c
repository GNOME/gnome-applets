/*###################################################################*/
/*##                         clock & mail applet 0.1.1 alpha       ##*/
/*###################################################################*/

#include "clockmail.h"

GtkWidget *propwindow = NULL;
gint P_AM_PM_ENABLE;
gint P_ALWAYS_BLINK;

void property_load(char *path)
{
        gnome_config_push_prefix (path);
        AM_PM_ENABLE = gnome_config_get_int("12hour=0");
	ALWAYS_BLINK = gnome_config_get_int("blink=0");
        gnome_config_pop_prefix ();
}

void property_save(char *path)
{
        gnome_config_push_prefix(path);
        gnome_config_set_int("12hour", AM_PM_ENABLE);
        gnome_config_set_int("blink", ALWAYS_BLINK);
	gnome_config_sync();
        gnome_config_pop_prefix();
}

void am_pm_time_cb(GtkWidget *w)
{
	P_AM_PM_ENABLE = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

void always_blink_cb(GtkWidget *w)
{
	P_ALWAYS_BLINK = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

void property_apply_cb( GtkWidget *widget, void *data )
{
        AM_PM_ENABLE = P_AM_PM_ENABLE;
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

	P_AM_PM_ENABLE=AM_PM_ENABLE;
	P_ALWAYS_BLINK = ALWAYS_BLINK;

	propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(propwindow)->dialog.window),
		"ClockMail Settings");
	
	frame = gtk_vbox_new(5, TRUE);

	button = gtk_check_button_new_with_label (_("Display time in 12 hour format (AM/PM)"));
	gtk_box_pack_start(GTK_BOX(frame), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), P_AM_PM_ENABLE);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) am_pm_time_cb, NULL);
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
