/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <config.h>
#include "sound-monitor.h"
#include "skin.h"
#include "update.h"
#include "esdcalls.h"
#include "manager.h"

static void about_cb (AppletWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	const gchar *authors[4];

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}
	authors[0] = _("John Ellis <johne@bellatlantic.net>");
	authors[1] = NULL;

        about = gnome_about_new ( _("Sound Monitor Applet"), VERSION,
			_("(C) 2000 John Ellis"),
			authors,
			_("Sound monitor interface to Esound\n\n"
			"Released under the GNU general public license."),
			NULL);
	gtk_signal_connect( GTK_OBJECT(about), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about );
	gtk_widget_show (about);
	return;
}

static void esd_control_cb (AppletWidget *widget, gpointer data)
{
	AppData *ad = data;

	if (ad->esd_status == Status_Error)
		{
		if (esd_control(Control_Start, ad->esd_host));
		sound_free(ad->sound);
		ad->sound = sound_init(ad->esd_host, ad->monitor_input);
		}
	else if (ad->esd_status == Status_Standby)
		{
		esd_control(Control_Resume, ad->esd_host);
		}
	else if (ad->esd_status == Status_Ready)
		{
		esd_control(Control_Standby, ad->esd_host);
		}
	return;
}

static void manager_cb (AppletWidget *widget, gpointer data)
{
	AppData *ad = data;
	manager_window_show(ad);
	return;
}

void sync_esd_menu_item(AppData *ad)
{
	gchar *menu_text = "";
	gchar *stock_type;
	switch (ad->esd_status)
		{
		case Status_Ready:
		case Status_AutoStandby:
			menu_text = _("Place Esound in standby");
			stock_type = GNOME_STOCK_MENU_STOP;
			break;
		case Status_Standby:
			menu_text = _("Resume Esound");
			stock_type = GNOME_STOCK_MENU_VOLUME;
			break;
		default:
			menu_text = _("Start Esound");
			stock_type = GNOME_STOCK_MENU_VOLUME;
			break;
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

void reload_skin(AppData *ad)
{
	if (ad->theme_file && strlen(ad->theme_file) == 0)
		{
		skin_set(NULL, ad);
		}
	else if (!skin_set(ad->theme_file, ad))
		{
		printf("Failed to load skin %s, loading default\n", ad->theme_file);
		skin_set(NULL, ad);
		}
}

/* clean up function to free all the applet's memory and stop io,timers */
static void destroy_applet(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;

	manager_window_close(ad);

	gtk_timeout_remove(ad->update_timeout_id);

	sound_free(ad->sound);

	skin_free(ad->skin);

	g_free(ad->esd_host);
	g_free(ad->theme_file);
	g_free(ad);
	return;
}

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	AppData *ad = data;
	ad->orient = o;

	reload_skin(ad);
	return;
	w = NULL;
}

#ifdef HAVE_PANEL_PIXEL_SIZE
static void applet_change_pixel_size(GtkWidget *w, int size, gpointer data)
{
	AppData *ad = data;

	if(size<PIXEL_SIZE_SMALL)
		ad->sizehint = SIZEHINT_TINY;
	else if(size<PIXEL_SIZE_STANDARD)
		ad->sizehint = SIZEHINT_SMALL;
	else if(size<PIXEL_SIZE_LARGE)
		ad->sizehint = SIZEHINT_STANDARD;
	else if(size<PIXEL_SIZE_HUGE)
		ad->sizehint = SIZEHINT_LARGE;
	else
		ad->sizehint = SIZEHINT_HUGE;

	reload_skin(ad);
	return;
        w = NULL;
}
#endif

#ifdef HAVE_PANEL_DRAW_SIGNAL
static void applet_do_draw(GtkWidget *w, gpointer data)
{
	AppData *ad = data;

	applet_skin_backing_sync(ad);
}
#endif

static gint applet_save_session(GtkWidget *widget, gchar *privcfgpath,
				gchar *globcfgpath, gpointer data)
{
	AppData *ad = data;
        property_save(privcfgpath, ad);
	return FALSE;
	widget = NULL;
	globcfgpath = NULL;
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "sound-monitor_applet", 
					  "index.html" };
	gnome_help_display(NULL, &help_entry);
}

static AppData *create_new_app(GtkWidget *applet)
{
        AppData *ad;

        ad = g_new0(AppData, 1);

	ad->applet = applet;
	ad->propwindow = NULL;
	ad->orient = ORIENT_UP;
	ad->sizehint = SIZEHINT_STANDARD;

	ad->esd_status = Status_Error;
	ad->esd_status_menu = Status_Error;

	ad->sound = NULL;

	ad->esd_host = NULL;

	ad->refresh_fps = 10;
	ad->falloff_speed = 3;
	ad->peak_mode = PeakMode_Active;
	ad->scope_scale = 5;
	ad->draw_scope_as_segments = TRUE;

	ad->skin = NULL;

	ad->theme_file = NULL;

	property_load(APPLET_WIDGET(applet)->privcfgpath, ad);

	ad->display = gtk_drawing_area_new();
	gtk_signal_connect(GTK_OBJECT(ad->display), "destroy",
		GTK_SIGNAL_FUNC(destroy_applet), ad);
	gtk_widget_show(ad->display);

	gtk_signal_connect(GTK_OBJECT(ad->applet),"change_orient",
		GTK_SIGNAL_FUNC(applet_change_orient), ad);

#ifdef HAVE_PANEL_PIXEL_SIZE
	gtk_signal_connect(GTK_OBJECT(ad->applet),"change_pixel_size",
		GTK_SIGNAL_FUNC(applet_change_pixel_size), ad);
#endif

	applet_widget_add(APPLET_WIDGET(ad->applet), ad->display);

#ifdef HAVE_PANEL_DRAW_SIGNAL
	applet_widget_send_draw(APPLET_WIDGET(ad->applet), TRUE);

	gtk_signal_connect(GTK_OBJECT(ad->applet),"do_draw",
		GTK_SIGNAL_FUNC(applet_do_draw), ad);
#endif

	gtk_widget_set_usize(ad->applet, 5, 5); /* so that a large default is not shown */
	gtk_widget_show(ad->applet);

	reload_skin(ad);

	gtk_signal_connect(GTK_OBJECT(ad->applet),"save_session",
		GTK_SIGNAL_FUNC(applet_save_session), ad);

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

	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet),
						"properties",
						GNOME_STOCK_MENU_PROP,
						_("Properties..."),
						property_show,
						ad);

	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet),
					      "help",
					      GNOME_STOCK_PIXMAP_HELP,
					      _("Help"), help_cb, NULL);

	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet),
						"about",
						GNOME_STOCK_MENU_ABOUT,
						_("About..."),
						about_cb,
						NULL);

	update_display(ad);

	ad->sound = sound_init(ad->esd_host, ad->monitor_input);

	ad->update_timeout_id = gtk_timeout_add(1000 / ad->refresh_fps, (GtkFunction)update_display, ad);
	
	return ad;
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
	params = NULL;
	nparams = 0;
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


