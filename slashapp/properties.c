/*###################################################################*/
/*##                                                               ##*/
/*###################################################################*/

#include "slashapp.h"

static void article_delay_cb(GtkObject *adj, gpointer data);
static void slashdot_cb(GtkWidget *w, gpointer data);
static void gnotices_cb(GtkWidget *w, gpointer data);
/* static void browser_window_cb(GtkWidget *w, gpointer data); */
/* static void show_images_cb(GtkWidget *w, gpointer data); */
/* static void show_info_cb(GtkWidget *w, gpointer data); */
/* static void show_department_cb(GtkWidget *w, gpointer data); */
static void smooth_scroll_cb(GtkWidget *w, gpointer data);
static void smooth_type_cb(GtkWidget *w, gpointer data);
static void scroll_delay_cb(GtkObject *adj, gpointer data);
static void scroll_speed_cb(GtkObject *adj, gpointer data);
static void property_apply_cb(GtkWidget *widget, void *nodata, gpointer data);
static gint property_destroy_cb(GtkWidget *widget, gpointer data);

void property_load(gchar *path, AppData *ad)
{
	gnome_config_push_prefix (path);
	ad->smooth_scroll = gnome_config_get_int("slashapp/smooth_scroll=1");
	ad->smooth_type = gnome_config_get_int("slashapp/smooth_type=1");
	ad->scroll_delay = gnome_config_get_int("slashapp/scroll_delay=10");
	ad->scroll_speed = gnome_config_get_int("slashapp/scroll_speed=3");

	ad->show_images = gnome_config_get_int("slashapp/show_images=1");
	ad->show_info = gnome_config_get_int("slashapp/show_info=1");
	ad->show_department = gnome_config_get_int("slashapp/show_department=0");
	ad->article_delay = gnome_config_get_int("slashapp/article_delay=50");
	ad->new_browser_window = gnome_config_get_int("slashapp/new_browser_window=1");
	ad->proxy_enabled = gnome_config_get_int("slashapp/proxy_enabled=0");
	ad->proxy_host = gnome_config_get_string("slashapp/proxy_host=localhost");
	ad->proxy_port = gnome_config_get_string("slashapp/proxy_port=8080");
	ad->rdf_site = gnome_config_get_int("slashapp/rdf_site=1");

        gnome_config_pop_prefix ();
}

void property_save(gchar *path, AppData *ad)
{

        gnome_config_push_prefix(path);
        gnome_config_set_int("slashapp/smooth_scroll", ad->smooth_scroll);
        gnome_config_set_int("slashapp/smooth_type", ad->smooth_type);
        gnome_config_set_int("slashapp/scroll_delay", ad->scroll_delay);
        gnome_config_set_int("slashapp/scroll_speed", ad->scroll_speed);

        gnome_config_set_int("slashapp/show_images", ad->show_images);
        gnome_config_set_int("slashapp/show_info", ad->show_info);
        gnome_config_set_int("slashapp/show_department", ad->show_department);
        gnome_config_set_int("slashapp/article_delay", ad->article_delay);

        gnome_config_set_int("slashapp/new_browser_window", ad->new_browser_window);
	gnome_config_set_int("slashapp/proxy_enabled", ad->proxy_enabled);
	gnome_config_set_string("slashapp/proxy_host", ad->proxy_host);
	gnome_config_set_string("slashapp/proxy_port", ad->proxy_port);
        gnome_config_set_int("slashapp/rdf_site", ad->rdf_site);

	gnome_config_pop_prefix();
	gnome_config_sync();
}

static void article_delay_cb(GtkObject *adj, gpointer data)
{
        AppData *ad = data;

	if (GTK_IS_ADJUSTMENT(adj))
	        ad->p_article_delay = (gint)GTK_ADJUSTMENT(adj)->value;
	else
		ad->p_article_delay = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;

        gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void proxy_host_cb(GtkWidget *entry, gpointer data)
{
	AppData *ad = data;
	ad->p_proxy_host = gtk_entry_get_text(GTK_ENTRY(entry));
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void proxy_port_cb(GtkWidget *entry, gpointer data)
{
	AppData *ad = data;
	ad->p_proxy_port = gtk_entry_get_text(GTK_ENTRY(entry));
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void proxy_enable_cb(GtkWidget *w, gpointer data)
{
        AppData *ad = data;
        ad->p_proxy_enabled = GTK_TOGGLE_BUTTON (w)->active;
	if (ad->p_proxy_enabled) {
		gtk_widget_set_sensitive(ad->proxy_host_widget, TRUE);
		gtk_widget_set_sensitive(ad->proxy_port_widget, TRUE);
	}
	else {
		gtk_widget_set_sensitive(ad->proxy_host_widget, FALSE);
		gtk_widget_set_sensitive(ad->proxy_port_widget, FALSE);
	}
        gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void slashdot_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	if (GTK_TOGGLE_BUTTON (w)->active)
		ad->p_rdf_site = RDFSITE_SLASHDOT;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}


static void gnotices_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	if (GTK_TOGGLE_BUTTON (w)->active)
		ad->p_rdf_site = RDFSITE_GNOTICES;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

/* *** unused ***
static void browser_window_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_new_browser_window = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}
*/

/* *** unused ***
static void show_images_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_show_images = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}
*/

/* *** unused ***
static void show_info_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_show_info = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}
*/

/* *** unused ***
static void show_department_cb(GtkWidget *w, gpointer data)
{
	AppData *ad = data;
	ad->p_show_department = GTK_TOGGLE_BUTTON (w)->active;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}
*/

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

	if (GTK_IS_ADJUSTMENT(adj))
	        ad->p_scroll_delay = (gint)GTK_ADJUSTMENT(adj)->value;
	else
		ad->p_scroll_delay = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;

        gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void scroll_speed_cb(GtkObject *adj, gpointer data)
{
        AppData *ad = data;

	if (GTK_IS_ADJUSTMENT(adj))
	        ad->p_scroll_speed = (gint)GTK_ADJUSTMENT(adj)->value;
	else
		ad->p_scroll_speed = (gint)GTK_ADJUSTMENT(GTK_SPIN_BUTTON(adj)->adjustment)->value;

        gnome_property_box_changed(GNOME_PROPERTY_BOX(ad->propwindow));
}

static void property_apply_cb(GtkWidget *widget, void *nodata, gpointer data)
{
	AppData *ad = data;
	gboolean refresh = FALSE;

	ad->smooth_scroll = ad->p_smooth_scroll;
	ad->smooth_type = ad->p_smooth_type;
	ad->scroll_delay = ad->p_scroll_delay;
	ad->scroll_speed = ad->p_scroll_speed;

	ad->show_images = ad->p_show_images;
	ad->show_info = ad->p_show_info;
	ad->show_department = ad->p_show_department;
	ad->article_delay = ad->p_article_delay;
	ad->new_browser_window = ad->p_new_browser_window;

	g_free (ad->proxy_host);
	g_free (ad->proxy_port);
	ad->proxy_host = g_strdup (ad->p_proxy_host);
	ad->proxy_port = g_strdup (ad->p_proxy_port);
	ad->proxy_enabled = ad->p_proxy_enabled;

	if (ad->rdf_site != ad->p_rdf_site)
		refresh = TRUE;
	ad->rdf_site = ad->p_rdf_site;

	applet_widget_sync_config(APPLET_WIDGET(ad->applet));

	if (refresh)
		refresh_cb (NULL, ad);

	return;
	widget = NULL;
	nodata = NULL;
}

static gint property_destroy_cb(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;
	ad->propwindow = NULL;
	return FALSE;
	widget = NULL;
}

void property_show(AppletWidget *applet, gpointer data)
{
	AppData *ad = data;
	GtkWidget *frame;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *hbox;
	GtkWidget *hbox1;
	GtkWidget *hbox2;
	GtkWidget *entry;
	GtkWidget *label;
	GtkWidget *button;
	GtkObject *adj;
	GtkWidget *spin;
	GSList *list;

	if(ad->propwindow)
		{
                gdk_window_raise(ad->propwindow->window);
                return;
		}

	ad->p_smooth_scroll = ad->smooth_scroll;
	ad->p_smooth_type = ad->smooth_type;
	ad->p_scroll_delay = ad->scroll_delay;
	ad->p_scroll_speed = ad->scroll_speed;

	ad->p_show_images = ad->show_images;
	ad->p_show_info = ad->show_info;
	ad->p_show_department = ad->show_department;
	ad->p_article_delay = ad->article_delay;

	ad->p_new_browser_window = ad->new_browser_window;
	g_free (ad->p_proxy_host);
	g_free (ad->p_proxy_port);
	ad->p_proxy_host = g_strdup (ad->proxy_host);
	ad->p_proxy_port = g_strdup (ad->proxy_port);
	ad->p_rdf_site = ad->rdf_site;
	ad->p_proxy_enabled = ad->proxy_enabled;

	ad->propwindow = gnome_property_box_new();
	gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(ad->propwindow)->dialog.window),
		"Slashapp Settings");
	
	/* general tab */

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);



	frame = gtk_frame_new(_("Site"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	hbox = gtk_hbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);

	button = gtk_radio_button_new_with_label (NULL, _("Slashdot"));
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	if (ad->p_rdf_site == RDFSITE_SLASHDOT)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) slashdot_cb, ad);
	gtk_widget_show(button);

	list = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
	button = gtk_radio_button_new_with_label (list, _("Gnotices"));
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
	if (ad->p_rdf_site == RDFSITE_GNOTICES)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) gnotices_cb, ad);
	gtk_widget_show(button);


	frame = gtk_frame_new(_("Articles"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

/*	button = gtk_check_button_new_with_label (_("Show topic images"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_show_images);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) show_images_cb, ad);
	gtk_widget_show(button);
	
	button = gtk_check_button_new_with_label (_("Show department"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_show_department);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) show_department_cb, ad);
	gtk_widget_show(button);

	button = gtk_check_button_new_with_label (_("Show extra information (Time, Author, Comments)"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_show_info);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) show_info_cb, ad);
	gtk_widget_show(button);
*/
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox, FALSE, FALSE, 0);
	gtk_widget_show(hbox);

        label = gtk_label_new(_("Delay between articles (10 = 1 sec):"));
        gtk_box_pack_start( GTK_BOX(hbox), label, FALSE, FALSE, 0);
        gtk_widget_show(label);

        adj = gtk_adjustment_new( ad->p_article_delay, 0.0, 3000.0 , 10, 10, 10 );
        spin = gtk_spin_button_new( GTK_ADJUSTMENT(adj), 1, 0 );
        gtk_box_pack_start( GTK_BOX(hbox), spin, FALSE, FALSE, 0);
        gtk_signal_connect( GTK_OBJECT(adj),"value_changed",
			GTK_SIGNAL_FUNC(article_delay_cb), ad);
        gtk_signal_connect( GTK_OBJECT(spin),"changed",
			GTK_SIGNAL_FUNC(article_delay_cb), ad);
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

	button = gtk_check_button_new_with_label(_("Use Proxy"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_proxy_enabled);
	gtk_signal_connect(GTK_OBJECT(button), "clicked", 
			GTK_SIGNAL_FUNC(proxy_enable_cb), ad);
	gtk_widget_show(button);
			
	hbox1 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);
	gtk_widget_show(hbox1);

	label = gtk_label_new(_("Proxy Host: "));
	gtk_box_pack_start(GTK_BOX(hbox1), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), ad->p_proxy_host);
	gtk_widget_set_sensitive(entry, ad->p_proxy_enabled);
	gtk_signal_connect(GTK_OBJECT(entry), "changed", 
			GTK_SIGNAL_FUNC(proxy_host_cb), ad);
	ad->proxy_host_widget = entry;
	gtk_box_pack_start(GTK_BOX(hbox1), entry, FALSE, FALSE, 0);
	gtk_widget_show(entry);

	hbox2 = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox1), hbox2, FALSE, FALSE, 0);
	gtk_widget_show(hbox2);

	label = gtk_label_new(_("Proxy Port: "));
	gtk_box_pack_start(GTK_BOX(hbox2), label, FALSE, FALSE, 0);
	gtk_widget_show(label);

	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), ad->p_proxy_port);
	gtk_widget_set_sensitive(entry, ad->p_proxy_enabled);
	gtk_signal_connect(GTK_OBJECT(entry), "changed",
			GTK_SIGNAL_FUNC(proxy_port_cb), ad);
	ad->proxy_port_widget = entry;
	gtk_box_pack_start(GTK_BOX(hbox2), entry, FALSE, FALSE, 0);
	gtk_widget_show(entry);

        label = gtk_label_new(_("(These settings do not take effect until a refresh)"));
        gtk_box_pack_start(GTK_BOX(vbox1), label, FALSE, FALSE, 0);
        gtk_widget_show(label);

        label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),vbox ,label);

	/* display tab */

	vbox = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);

	frame = gtk_frame_new(_("Scrolling"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show(frame);

	vbox1 = gtk_vbox_new(FALSE, 1);
	gtk_container_add(GTK_CONTAINER(frame), vbox1);
	gtk_widget_show(vbox1);

	button = gtk_check_button_new_with_label (_("Smooth scroll"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_smooth_scroll);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) smooth_scroll_cb, ad);
	gtk_widget_show(button);

	button = gtk_check_button_new_with_label (_("Smooth type"));
	gtk_box_pack_start(GTK_BOX(vbox1), button, FALSE, FALSE, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), ad->p_smooth_type);
	gtk_signal_connect (GTK_OBJECT(button),"clicked",(GtkSignalFunc) smooth_type_cb, ad);
	gtk_widget_show(button);

	frame = gtk_frame_new(_("Speed"));
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
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
        gtk_signal_connect( GTK_OBJECT(spin),"changed",
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
        gtk_signal_connect( GTK_OBJECT(spin),"changed",
			GTK_SIGNAL_FUNC(scroll_speed_cb), ad);
        gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_ALWAYS);
        gtk_widget_show(spin);

        label = gtk_label_new(_("Display"));
        gtk_widget_show(frame);
        gnome_property_box_append_page( GNOME_PROPERTY_BOX(ad->propwindow),vbox ,label);

	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"apply", GTK_SIGNAL_FUNC(property_apply_cb), ad);
	gtk_signal_connect( GTK_OBJECT(ad->propwindow),"destroy", GTK_SIGNAL_FUNC(property_destroy_cb), ad );

        gtk_widget_show_all(ad->propwindow);
	return;
	applet = NULL;
} 
