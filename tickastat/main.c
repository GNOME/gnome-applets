/* tick-a-stat, a GNOME panel applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "tickastat.h"
#include <libgnomeui/gnome-window-icon.h>

static void event_log_cb(AppletWidget *widget, gpointer data);
static gint close_log_cb(gpointer data);
static void about_cb (AppletWidget *widget, gpointer data);
static void about_line_cb(ModuleData *md, gpointer data, InfoData *id, AppData *ad);
static void destroy_applet(GtkWidget *widget, gpointer data);
static gint applet_save_session(GtkWidget *widget, gchar *privcfgpath,
					gchar *globcfgpath, gpointer data);
static AppData *create_new_app(GtkWidget *applet);
static GtkWidget *applet_start_new_applet(const gchar *goad_id,
				    const char **params, int nparams);

/*
 *-------------------------------------------------------------------
 * 
 *-------------------------------------------------------------------
 */

/*
 *-------------------------------------------------------------------
 * Log file dialog
 *-------------------------------------------------------------------
 */

static void event_log_cb(AppletWidget *widget, gpointer data)
{
	AppData *ad = data;
	static GtkWidget *dialog = NULL;
	GtkWidget *gless;

	if (dialog != NULL)
	{
		gdk_window_show(dialog->window);
		gdk_window_raise(dialog->window);
		return;
	}
	dialog = gnome_dialog_new(_("Tick-a-Stat event log"),
				  GNOME_STOCK_BUTTON_CLOSE, NULL);
	gtk_signal_connect(GTK_OBJECT(dialog), "destroy",
			   GTK_SIGNAL_FUNC(gtk_widget_destroyed), &dialog);
	gnome_dialog_set_close(GNOME_DIALOG(dialog), TRUE);
	gtk_widget_set_usize(dialog, 500, 400);
	gtk_window_set_policy(GTK_WINDOW(dialog), TRUE, TRUE, FALSE);

	gless = gnome_less_new();
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox), gless, TRUE, TRUE, 0);
	gtk_widget_show(gless);

	if (!g_file_exists(ad->log_path) ||
	    !gnome_less_show_file(GNOME_LESS(gless), ad->log_path))
		{
		gnome_less_show_string(GNOME_LESS(gless), "Unable to open event log file.");
		}

	gtk_widget_show(dialog);
	return;
	widget = NULL;
}


/*
 *-------------------------------------------------------------------
 * Log file operations
 *-------------------------------------------------------------------
 */

static gint close_log_cb(gpointer data)
{
	AppData *ad = data;

	if (ad->log_was_written)
		{
		ad->log_was_written = FALSE;
		return TRUE;
		}

	fclose(ad->log_fd);
	ad->log_fd = NULL;
	ad->log_close_timeout_id = -1;

	return FALSE;
}

void print_to_log(gchar *text, AppData *ad)
{
	if (!ad->enable_log || !ad->log_path) return;

	if (!ad->log_fd)
		{
		ad->log_fd= fopen(ad->log_path,"a");
		if (!ad->log_fd)
			{
			printf("Failed to open log file for write: %s\n", ad->log_path);
			return;
			}
		if (ad->log_close_timeout_id == -1)
			{
			ad->log_close_timeout_id = gtk_timeout_add(300, (GtkFunction)close_log_cb, ad);
			}
		}

	fprintf(ad->log_fd, text);
	ad->log_was_written = TRUE;
}

/*
 * Return a time stamp string.
 * If filename_format is TRUE, return a string to stamp a file.
 */
gchar *time_stamp(gint filename_format)
{
	time_t current_time;
        struct tm *time_data;
	gchar date_string[128];

	time(&current_time);
	time_data = localtime(&current_time);

	if (filename_format)
		strftime(date_string, 128, _("%Y%m%d-%H-%M-%S"), time_data);
	else
		strftime(date_string, 128, _("%m-%d-%Y %H:%M:%S"), time_data);

	return g_strdup(date_string);
}

/*
 *-------------------------------------------------------------------
 * applet geometry callbacks
 *-------------------------------------------------------------------
 */

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	AppData *ad = data;
	ad->orient = o;

	if (!ad->draw_area) return; /* we are done if in startup */

	resized_app_display(ad, FALSE);
	return;
	w = NULL;
}

#ifdef HAVE_PANEL_PIXEL_SIZE
static void applet_change_pixel_size(GtkWidget *w, int size, gpointer data)
{
	AppData *ad = data;

	ad->sizehint = size;

	if (!ad->draw_area) return; /* we are done if in startup */

	resized_app_display(ad, FALSE);
        return;
        w = NULL;	
}
#endif

/*
 *-------------------------------------------------------------------
 * generic display applet funcs
 *-------------------------------------------------------------------
 */

static void about_cb(AppletWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	const gchar *authors[5];
	gchar version[32];

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}

	g_snprintf(version, sizeof(version), _("%d.%d.%d"), APPLET_VERSION_MAJ,
		APPLET_VERSION_MIN, APPLET_VERSION_REV);

	authors[0] = _("John Ellis <johne@bellatlantic.net>");
	authors[1] = NULL;

        about = gnome_about_new ( _("Tick-a-Stat"), version,
			_("(C) 1999 John Ellis"),
			authors,
			_("A ticker to display various information and statistics.\n"),
			NULL);
	gtk_signal_connect (GTK_OBJECT(about), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);
	gtk_widget_show (about);
        return;
        widget = NULL;
	data = NULL;
}

static void about_line_cb(ModuleData *md, gpointer data, InfoData *id, AppData *ad)
{
    about_cb(NULL, NULL);
    return;
    data = NULL;
    md = NULL;
    id = NULL;
    ad = NULL;
}

static void help_cb (AppletWidget *w, gpointer data)
{
        GnomeHelpMenuEntry help_entry = { "tickastat_applet",
                                          "index.html" };
        gnome_help_display(NULL, &help_entry);
}

static void destroy_applet(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;
	GList *work;

	if (ad->display_timeout_id != -1) gtk_timeout_remove(ad->display_timeout_id);
	if (ad->log_fd) fclose(ad->log_fd);

	work = ad->modules;
	while(work)
		{
		ModuleData *md = work->data;
		if (md->destroy_func) md->destroy_func(md->internal_data, ad);
		g_free(md);
		work = work->next;
		}
	g_list_free(ad->modules);

	gtk_timeout_remove(ad->display_timeout_id);

	free_all_info_lines(ad);
/* old way?
	gtk_widget_destroy(ad->display_w);
	gtk_widget_destroy(ad->disp_buf_w);
	gtk_widget_destroy(ad->background_w);
*/

	if (ad->display) gdk_pixmap_unref(ad->display);
	if (ad->disp_buf) gdk_pixmap_unref(ad->disp_buf);
	if (ad->background) gdk_pixmap_unref(ad->background);

	g_free(ad);
	return;
	widget = NULL;
}

static gint applet_save_session(GtkWidget *widget, gchar *privcfgpath,
					gchar *globcfgpath, gpointer data)
{
	AppData *ad = data;
	property_save(privcfgpath, ad);
	return FALSE;
	widget = NULL;
	globcfgpath = NULL;
}

static AppData *create_new_app(GtkWidget *applet)
{
	AppData *ad;
	GList *work;
	gchar *buf;
	InfoData *id;

	ad = g_new0(AppData, 1);

	ad->applet = applet;
	ad->modules = modules_build_list(ad);

	ad->log_fd = NULL;
	ad->log_close_timeout_id = -1;
	ad->log_path = gnome_util_home_file("tickastat");

	if (!g_file_exists(ad->log_path))
		{
		if (mkdir(ad->log_path, 0755) < 0)
			g_print(_("unable to create user directory: %s\n"), ad->log_path);
		}

	buf = ad->log_path;
	ad->log_path = g_concat_dir_and_file(buf, "eventlog");
	g_free(buf);

	ad->orient = ORIENT_UP;
#ifdef HAVE_PANEL_PIXEL_SIZE
	ad->sizehint = applet_widget_get_panel_pixel_size(APPLET_WIDGET(ad->applet));
#else
	ad->sizehint = 48;
#endif
	ad->width_hint = 200;

	ad->win_width = 200;
	ad->win_height = 48;
	ad->follow_hint_width = FALSE;
	ad->follow_hint_height = TRUE;
	ad->user_width = 200;
	ad->user_height = 48;
	ad->draw_area = NULL;

	property_load(APPLET_WIDGET(applet)->privcfgpath, ad);

        gtk_signal_connect(GTK_OBJECT(ad->applet), "destroy",
                GTK_SIGNAL_FUNC(destroy_applet), ad);


/* applet geometry signals*/
	gtk_signal_connect(GTK_OBJECT(ad->applet),"change_orient",
                GTK_SIGNAL_FUNC(applet_change_orient), ad);
#ifdef HAVE_PANEL_PIXEL_SIZE
	gtk_signal_connect(GTK_OBJECT(ad->applet),"change_pixel_size",
                GTK_SIGNAL_FUNC(applet_change_pixel_size), ad);
#endif

	gtk_widget_set_usize(ad->applet, 10, 10);

        gtk_widget_show(ad->applet);
	init_app_display(ad);

	id = add_info_line(ad, "Tick-a-Stat\n", NULL, 0, TRUE, 1, 0, 10);
	set_info_signals(id, NULL, about_line_cb, NULL, NULL, NULL, NULL);
	id = add_info_line(ad, _("The unique information and status ticker."), NULL, 0, FALSE, 1, 20, 10);
	set_info_signals(id, NULL, about_line_cb, NULL, NULL, NULL, NULL);
	add_info_line(ad, "\n \n \n ", NULL, 0, FALSE, 1, 0, 10);

/* applet signals */
        gtk_signal_connect(GTK_OBJECT(applet),"save_session",
                                GTK_SIGNAL_FUNC(applet_save_session),
                                ad);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
                                              "event_log",
                                              GNOME_STOCK_MENU_INDEX,
                                              _("Event log..."),
                                              event_log_cb, ad);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
                                              "properties",
                                              GNOME_STOCK_MENU_PROP,
                                              _("Properties..."),
                                              property_show,
                                              ad);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
                                              "help",
                                              GNOME_STOCK_PIXMAP_HELP,
                                              _("Help"),
                                              help_cb, NULL);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
                                              "about",
                                              GNOME_STOCK_MENU_ABOUT,
                                              _("About..."),
                                              about_cb, NULL);


	/* init/start the module functions */
	work = ad->modules;
	while (work)
		{
		ModuleData *md = work->data;

		if (md->start_func) md->start_func(md->internal_data, ad);

		work = work->next;
		}

	return ad;
}

static GtkWidget *applet_start_new_applet(const gchar *goad_id,
				    const char **params, int nparams)
{
	GtkWidget *applet;

	g_return_val_if_fail(!strcmp(goad_id, "tickastat_applet"), NULL);

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
	gchar *ver_string;

	/* Initialize the i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	ver_string = g_strdup_printf("%d.%d.%d", APPLET_VERSION_MAJ, APPLET_VERSION_MIN, APPLET_VERSION_REV);
	applet_widget_init("tickastat_applet", ver_string, argc, argv, NULL,
			   0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-ticker.png");

	applet_factory_new("tickastat_applet_factory", NULL,
                           applet_start_new_applet);

        goad_id = (char *)goad_server_activation_id();
        if(goad_id && !strcmp(goad_id, "tickastat_applet"))
		{
		applet = applet_widget_new("tickastat_applet");
		if (!applet)
			g_error("Can't create applet!\n");

		create_new_app(applet);
		}

        applet_widget_gtk_main();
        return 0;
}
