/* GNOME Esound Monitor Control applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <config.h>
#include <sound-monitor.h>

static void about_cb (AppletWidget *widget, gpointer data);
static void manager_cb (AppletWidget *widget, gpointer data);
static void redraw_display(AppData *ad);
static gint update_display(gpointer data);
static void applet_change_back(GtkWidget *applet, PanelBackType type, char *pixmap,
				GdkColor *color, gpointer data);
static void destroy_applet(GtkWidget *widget, gpointer data);
static AppData *create_new_app(GtkWidget *applet);
static void reload_theme(AppData *ad);
static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data);

#ifdef HAVE_PANEL_PIXEL_SIZE
static void applet_change_pixel_size(GtkWidget *w, int size, gpointer data);
#endif

static gint applet_save_session(GtkWidget *widget, char *privcfgpath,
					char *globcfgpath, gpointer data);
static GtkWidget * applet_start_new_applet(const gchar *goad_id, const char **params, int nparams);

static void about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	const gchar *authors[4];

	authors[0] = _("John Ellis <johne@bellatlantic.net>");
	authors[1] = NULL;

        about = gnome_about_new ( _("Sound Monitor Applet"), VERSION,
			_("(C) 1999 John Ellis"),
			authors,
			_("Sound monitor interface to Esound\n\n"
			"Released under the GNU general public license."),
			NULL);
	gtk_widget_show (about);
}

static void esd_control_cb (AppletWidget *widget, gpointer data)
{
	AppData *ad = data;

	if (ad->esd_status == ESD_STATUS_ERROR)
		{
		esd_sound_control(ESD_CONTROL_START, ad);
		}
	else if (ad->esd_status == ESD_STATUS_STANDBY)
		{
		esd_sound_control(ESD_CONTROL_RESUME, ad);
		}
	else if (ad->esd_status == ESD_STATUS_READY)
		{
		esd_sound_control(ESD_CONTROL_STANDBY, ad);
		}
}

static void manager_cb (AppletWidget *widget, gpointer data)
{
	AppData *ad = data;
	manager_window_show(ad);
}

static void sync_esd_menu_item(AppData *ad)
{
	gchar *menu_text = "";
	gchar *stock_type;
	if (ad->esd_status == ESD_STATUS_READY || ad->esd_status == ESD_STATUS_AUTOSTANDBY)
		{
		menu_text = _("Place Esound in standby");
		stock_type = GNOME_STOCK_MENU_STOP;
		}
	else if (ad->esd_status == ESD_STATUS_STANDBY)
		{
		menu_text = _("Resume Esound");
		stock_type = GNOME_STOCK_MENU_VOLUME;
		}
	else	
		{
		menu_text = _("Start Esound");
		stock_type = GNOME_STOCK_MENU_VOLUME;
		}

	applet_widget_unregister_callback (APPLET_WIDGET(ad->applet),
						"control");
	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet),
						"control",
						stock_type,
						menu_text,
						esd_control_cb,
						ad);
	ad->esd_status_menu = ad->esd_status;
}

void reset_fps_timeout(AppData *ad)
{
	gtk_timeout_remove(ad->update_timeout_id);
	ad->update_timeout_id = gtk_timeout_add(1000 / ad->refresh_fps, (GtkFunction)update_display, ad);
}

void set_widget_modes(AppData *ad)
{
	/* just to test */
	set_vu_item_mode(ad->skin->vu_left, ad->peak_mode, ad->falloff_speed);
	set_vu_item_mode(ad->skin->vu_right, ad->peak_mode, ad->falloff_speed);
	set_scope_item_scale(ad->skin->scope, ad->scope_scale);
}

static void redraw_display(AppData *ad)
{
	redraw_skin(ad);
}

static void do_zero_volume_check(AppData *ad)
{
	if (ad->vu_l != 0 || ad->vu_r != 0)
		{
		ad->vu_l = ad->vu_r = 0;
		ad->new_vu_data = TRUE;
		}
}

static gint update_display(gpointer data)
{
	gint redraw = FALSE;
	AppData *ad = data;
	gint draw_scope = FALSE;

	/* do status check */

	ad->esd_status_check_count++;
	if (ad->esd_status_check_count > ad->refresh_fps) /* 1 second checks should do */
		{
		gint new_status = esd_check_status(ad);
		ad->esd_status_check_count = 0;
		if (ad->esd_status != new_status)
			{
			ad->esd_status = new_status;
			if (ad->esd_status == ESD_STATUS_ERROR)
				{
				/* problem must have occured, disconnect from esd now! */
				stop_sound(ad);
				gnome_error_dialog("Sound Monitor has lost the connection to the Esound daemon,\n this usually means the daemon has exited and can simply be restarted.\n\n Or the daemon has crashed and needs some attention...");
				}
			}
		}

	if (ad->esd_status != ESD_STATUS_READY && !ad->new_vu_data)
		{
		do_zero_volume_check(ad);
		}

	if (ad->esd_status == ESD_STATUS_READY)
		{
		if (ad->new_vu_data)
			ad->no_data_check_count = 0;
		else if (ad->no_data_check_count < ad->refresh_fps / 3) /* 1/3 second with no sound */
			ad->no_data_check_count++;
		else
			{
			do_zero_volume_check(ad);
			if (!ad->scope_flat)
				{
				ad->scope_flat = TRUE;
				draw_scope = TRUE;
				}
			}
		}

	/* do our display writes */

	if (ad->new_vu_data)
		{
		draw_scope_item(ad->skin->scope, ad, ad->scope_flat);
		ad->scope_flat = FALSE;
		redraw= TRUE;
		}
	else if (draw_scope)
		{
		draw_scope_item(ad->skin->scope, ad, ad->scope_flat);
		redraw= TRUE;
		}

	if (ad->new_vu_data || ad->prev_vu_changed)
		{
		redraw = (draw_vu_item(ad->skin->vu_left, ad->vu_l, ad) || redraw);
		redraw = (draw_vu_item(ad->skin->vu_right, ad->vu_r, ad) || redraw);

		redraw = (draw_item_by_percent(ad->skin->meter_left, ad->vu_l, ad) || redraw);
		redraw = (draw_item_by_percent(ad->skin->meter_right, ad->vu_r, ad) || redraw);

		ad->prev_vu_changed = redraw;

		ad->new_vu_data = FALSE;
		}

	redraw = (draw_item(ad->skin->status, ad->esd_status, ad) || redraw);

	if (redraw) redraw_display(ad);

	if (ad->esd_status_menu != ad->esd_status) sync_esd_menu_item(ad);

	return TRUE;
}

static void applet_change_back(GtkWidget *applet, PanelBackType type, char *pixmap,
				GdkColor *color, gpointer data)
{
	AppData *ad = data;
	GtkStyle *ns;

	switch (type)
		{
		case PANEL_BACK_NONE :
			gtk_widget_set_rc_style(GTK_WIDGET(ad->applet));
			break;
		case PANEL_BACK_COLOR :
			ns = gtk_style_copy(GTK_WIDGET(ad->applet)->style);
			gtk_style_ref(ns);
			ns->bg[GTK_STATE_NORMAL] = *color;
			gtk_widget_set_style(GTK_WIDGET(ad->applet), ns);
			gtk_style_unref(ns);
			break;
		case PANEL_BACK_PIXMAP :
			if (g_file_exists (pixmap))
				{
				GdkImlibImage *im = gdk_imlib_load_image (pixmap);
				if (im)
					{
					GdkPixmap *pmap;
					gdk_imlib_render (im, im->rgb_width, im->rgb_height);
					pmap = gdk_imlib_copy_image (im);
					gdk_imlib_destroy_image (im);
					gdk_window_set_back_pixmap(ad->applet->window, pmap, FALSE);
					gdk_window_clear(ad->applet->window);
					gdk_pixmap_unref(pmap);
					}
				}
			break;
		}
}

/* clean up function to free all the applet's memory and stop io,timers */
static void destroy_applet(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;

	manager_window_close(ad);

	gtk_timeout_remove(ad->update_timeout_id);

	stop_sound(ad);

	free_skin(ad->skin);

	g_free(ad->esd_host);
	g_free(ad->theme_file);
	g_free(ad);
}

static AppData *create_new_app(GtkWidget *applet)
{
        AppData *ad;

        ad = g_new0(AppData, 1);

	ad->applet = applet;
	ad->propwindow = NULL;
	ad->orient = ORIENT_UP;
	ad->sizehint = SIZEHINT_STANDARD;

	ad->esd_status = ESD_STATUS_ERROR;
	ad->esd_status_menu = ESD_STATUS_ERROR;

	ad->sound_fd = -1;
	ad->sound_input_cb_id = -1;
	ad->esd_host = NULL;

	ad->refresh_fps = 10;
	ad->falloff_speed = 3;
	ad->peak_mode = PEAK_MODE_ACTIVE;
	ad->scope_scale = 5;
	ad->draw_scope_as_segments = TRUE;

	ad->skin = NULL;

	ad->theme_file = NULL;

	property_load(APPLET_WIDGET(applet)->privcfgpath, ad);

	/* create a tooltip widget */
	ad->tooltips = gtk_tooltips_new();

	ad->display_area = gtk_drawing_area_new();
	gtk_signal_connect(GTK_OBJECT(ad->display_area), "destroy",
		GTK_SIGNAL_FUNC(destroy_applet), ad);
	gtk_widget_show(ad->display_area);

	applet_widget_add(APPLET_WIDGET(ad->applet), ad->display_area);

	gtk_signal_connect(GTK_OBJECT(ad->applet),"change_orient",
		GTK_SIGNAL_FUNC(applet_change_orient), ad);

#ifdef HAVE_PANEL_PIXEL_SIZE
	gtk_signal_connect(GTK_OBJECT(ad->applet),"change_pixel_size",
		GTK_SIGNAL_FUNC(applet_change_pixel_size), ad);
#endif

	gtk_widget_set_usize(ad->applet, 5, 5); /* so that a large default is not shown */
	gtk_widget_show(ad->applet);

	reload_theme(ad);

	gtk_signal_connect(GTK_OBJECT(ad->applet),"save_session",
		GTK_SIGNAL_FUNC(applet_save_session), ad);
	gtk_signal_connect(GTK_OBJECT(ad->applet),"back_change",
		GTK_SIGNAL_FUNC(applet_change_back), ad);

	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet),
						"properties",
						GNOME_STOCK_MENU_PROP,
						_("Properties..."),
						property_show,
						ad);

	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet),
						"about",
						GNOME_STOCK_MENU_ABOUT,
						_("About..."),
						about_cb,
						NULL);

	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet),
						"manager",
						GNOME_STOCK_MENU_BOOK_OPEN,
						_("Manager..."),
						manager_cb,
						ad);

	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet),
						"control",
						GNOME_STOCK_MENU_VOLUME,
						_("Start Esound"),
						esd_control_cb,
						ad);


	update_display(ad);

	set_widget_modes(ad);

	init_sound(ad);

	ad->update_timeout_id = gtk_timeout_add(1000 / ad->refresh_fps, (GtkFunction)update_display, ad);
	
	return ad;
}

static void reload_theme(AppData *ad)
{
	if (ad->theme_file && strlen(ad->theme_file) == 0)
		{
		change_to_skin(NULL, ad);
		}
	else if (!change_to_skin(ad->theme_file, ad))
		{
		printf("Failed to load theme %s, loading default\n", ad->theme_file);
		change_to_skin(NULL, ad);
		}
}

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	AppData *ad = data;
	ad->orient = o;

	if (!ad->skin) return; /* we are done if in startup */

	reload_theme(ad);
}

#ifdef HAVE_PANEL_PIXEL_SIZE
static void applet_change_pixel_size(GtkWidget *w, int size, gpointer data)
{
	AppData *ad = data;

	if(size<PIXEL_SIZE_STANDARD)
		ad->sizehint = SIZEHINT_TINY;
	else if(size<PIXEL_SIZE_LARGE)
		ad->sizehint = SIZEHINT_STANDARD;
	else if(size<PIXEL_SIZE_HUGE)
		ad->sizehint = SIZEHINT_LARGE;
	else
		ad->sizehint = SIZEHINT_HUGE;

	if (!ad->skin) return; /* we are done if in startup */

	reload_theme(ad);
}
#endif

static gint applet_save_session(GtkWidget *widget, gchar *privcfgpath,
					gchar *globcfgpath, gpointer data)
{
	AppData *ad = data;
        property_save(privcfgpath, ad);
        return FALSE;
}

static GtkWidget * applet_start_new_applet(const gchar *goad_id,
					   const char **params, int nparams)
{
	GtkWidget *applet;

	g_return_val_if_fail(!strcmp(goad_id, "sound-monitor_applet"), NULL);

	applet = applet_widget_new(goad_id);

	if (!applet)
	  g_error("Can't create applet!\n");

	create_new_app(applet);

	return applet;
}

int main (int argc, char *argv[])
{
	GtkWidget *applet;
	char *goad_id;

	/* Initialize the i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init("sound-monitor_applet", VERSION, argc, argv, NULL, 0, NULL);

	applet_factory_new("sound-monitor_applet_factory", NULL,
			   applet_start_new_applet);

	goad_id = (char *)goad_server_activation_id();
	if(goad_id && !strcmp(goad_id, "sound-monitor_applet")) {
		applet = applet_widget_new("sound-monitor_applet");
		if (!applet)
			g_error("Can't create applet!\n");

		create_new_app(applet);
	}

	applet_widget_gtk_main();
	return 0;
}
