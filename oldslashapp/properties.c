/*###################################################################*/
/*##                                                               ##*/
/*###################################################################*/

#include "slashapp.h"

static void smooth_scroll_cb(GtkWidget *w, gpointer data);
static void smooth_type_cb(GtkWidget *w, gpointer data);
static void scroll_delay_cb(GtkObject *adj, gpointer data);
static void scroll_speed_cb(GtkObject *adj, gpointer data);
static void property_apply_cb(GtkWidget *widget, void *nodata, gpointer data);
static gint property_destroy_cb(GtkWidget *widget, gpointer data);

void property_load(gchar *path, AppData *ad)
{
	gnome_config_push_prefix (path);
	ad->smooth_scroll = gnome_config_get_int("display/smooth_scroll=1");
	ad->smooth_type = gnome_config_get_int("display/smooth_type=1");
	ad->scroll_delay = gnome_config_get_int("display/scroll_delay=10");
	ad->scroll_speed = gnome_config_get_int("display/scroll_speed=3");
        gnome_config_pop_prefix ();
}

void property_save(gchar *path, AppData *ad)
{
        gnome_config_push_prefix(path);
        gnome_config_set_int("display/smooth_scroll", ad->smooth_scroll);
        gnome_config_set_int("display/smooth_type", ad->smooth_type);
        gnome_config_set_int("display/scroll_delay", ad->scroll_delay);
        gnome_config_set_int("display/scroll_speed", ad->scroll_speed);
	gnome_config_sync();
        gnome_config_pop_prefix();
}

static void smooth_scroll_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_smooth_scroll = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void smooth_type_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_smooth_type = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void scroll_delay_cb(GtkObject *adj, gpointer data)
{
        AppData *ad = data;
        ad->p_scroll_delay = (gint)GTK_ADJUSTMENT(adj)->value;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void scroll_speed_cb(GtkObject *adj, gpointer data)
{
        AppData *ad = data;
        ad->p_scroll_speed = (gint)GTK_ADJUSTMENT(adj)->value;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void property_apply_cb(GtkWidget *widget, void *nodata, gpointer data)
{
	AppData *ad = data;

	ad->smooth_scroll = ad->p_smooth_scroll;
	ad->smooth_type = ad->p_smooth_type;
	ad->scroll_delay = ad->p_scroll_delay;
	ad->scroll_speed = ad->p_scroll_speed;


	applet_widget_sync_config(APPLET_WIDGET(ad->applet));
}

static gint property_destroy_cb(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;
	ad->propwindow = NULL;
	return FALSE;
}

void property_show(AppletWidget *applet, gpointer data)
{
	AppData *ad = data;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *button;
	GtkObject *adj;
	GtkWidget *spin;

	if(ad->propwindow)
		{
                gdk_window_raise(ad->propwindow->window);
                return;
		}

	ad->p_smooth_scroll = ad->smooth_scroll;
	ad->p_smooth_type = ad->smooth_type;
	ad->p_scroll_delay = ad->scroll_delay;
	ad->p_scroll_speed = ad->scroll_speed;

	ad->propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(ad->propwindow)->dialog.window),
		"Display test Settings");
	
	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);

	frame = gtk_frame_new(_("Scrolling"));
	gtk_container_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	button = gtk_check_button_new_with_label (_("Smooth scroll"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), ad->p_smooth_scroll);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) smooth_scroll_cb, ad);
	gtk_widget_show(button);

	button = gtk_check_button_new_with_label (_("Smooth type"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(button), ad->p_smooth_type);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) smooth_type_cb, ad);
	gtk_widget_show(button);

	frame = gtk_frame_new(_("Speed"));
	gtk_container_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Delay when wrapping text:"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
        gtk_widget_show(label);

        adj = gtk_adjustment_new(ad->p_scroll_delay, 0.0, 50.0, 1, 1, 1 );
        spin = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1, 0 );
        gtk_box_pack_start( GTK_BOX(hbox), spin, FALSE, FALSE, 0);
        gtk_signal_connect( GTK_OBJECT(adj),"value_changed",
			GTK_SIGNAL_FUNC(scroll_delay_cb), ad);
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Scroll speed between lines (Smooth scroll):"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
        gtk_widget_show(label);

        adj = gtk_adjustment_new( ad->p_scroll_speed, 1.0, (float)ad->scroll_height - 1, 1, 1, 1 );
        spin = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1, 0 );
        gtk_box_pack_start( GTK_BOX(hbox), spin, FALSE, FALSE, 0);
        gtk_signal_connect( GTK_OBJECT(adj),"value_changed",
			GTK_SIGNAL_FUNC(scroll_speed_cb), ad);
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

        label = gtk_label_new(_("Display"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),vbox ,label);

	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), ad);
	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), ad );

        gtk_widget_show_all(ad->propwindow);
} 
