/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Cambridge, MA 
 * 02139, USA.
 *
 */

#include "clockmail.h"
#include <libgnomeui/gnome-window-icon.h>
#include "skin.h"
#include "update.h"
#include "display.h"

static void about_cb (AppletWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	const gchar *authors[2];

	if (about != NULL)
		{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
		}
	authors[0] = _("John Ellis <johne@bellatlantic.net>");
	authors[1] = NULL;

        about = gnome_about_new ( _("Clock and Mail Notify Applet"), VERSION,
			_("(C) 2000"),
			authors,
			_("Released under the GNU general public license.\n"
			"Basic digital clock with date in a tooltip. "
			"Optional 12/24 time display. Mail blinking can be "
			"for any unread mail, or only briefly when new mail arrives."),
			NULL);
	gtk_signal_connect( GTK_OBJECT(about), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about );
	gtk_widget_show (about);
	return;
	widget = NULL;
	data = NULL;
}

static void help_cb (AppletWidget *applet, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "clockmail_applet", "index.html" };
	gnome_help_display (NULL, &help_entry);
}

void launch_mail_reader(gpointer data)
{
	AppData *ad = data;
	if (ad->reader_exec_cmd && strlen(ad->reader_exec_cmd) > 0)
		gnome_execute_shell(NULL, ad->reader_exec_cmd);
}

/*
 * Get file modification time, based upon the code
 * of Byron C. Darrah for coolmail and reused on fvwm95
 * I updated this by adding the old/new time check so the newmail notifier works
 * the way I wanted to work (eg. newmail is true only when mail has arrived since last check)
 * added a reset command in the event the mail_file has changed from the properties dialog
 */
void check_mail_file_status (int reset, AppData *ad)
{
	struct stat s;
	off_t newsize;
	time_t newtime;
	int status;
	int checktime;

	if (reset)
		{
		ad->mailsize = 0;
		ad->oldtime = 0;
		return;
		}

	/* no need to continue if no mail item is available */
	if (!ad->skin->mail && !ad->skin->mail_count) return;

	/* check if mail file contains something */
	if (!ad->mail_file) return;

	status = stat (ad->mail_file, &s);
	if (status < 0){
		ad->mailsize = 0;
		ad->anymail = ad->newmail = ad->unreadmail = 0;
		return;
	}
	
	newsize = s.st_size;
	newtime = s.st_mtime;
	ad->anymail = newsize > 0;
	ad->unreadmail = (s.st_mtime >= s.st_atime && newsize > 0);

	checktime = (newtime > ad->oldtime);
	if (newsize > ad->mailsize && ad->unreadmail && checktime){
		ad->newmail = 1;
		ad->mailcleared = 0;
	} else
		ad->newmail = 0;
	ad->oldtime = newtime;
	ad->mailsize = newsize;
}

void set_tooltip(struct tm *time_data, AppData *ad)
{
	if (time_data->tm_yday != ad->old_yday)
		{
		gchar date[128];
		strftime(date, 128, _("%a, %b %d"), time_data);

		if (ad->use_gmt)
			{
			gchar *buf;
			if (ad->gmt_offset == 0)
				{
				buf = g_strconcat (date, _(" (GMT)"), NULL);
				}
			else
				{
				gchar gmt_text[32];
				g_snprintf(gmt_text, sizeof(gmt_text), 
					   _(" (GMT %+d)"), ad->gmt_offset);
				buf = g_strconcat (date, gmt_text, NULL);
				}
			applet_widget_set_tooltip(APPLET_WIDGET(ad->applet), buf);
			g_free(buf);
			}
		else
			{
			applet_widget_set_tooltip(APPLET_WIDGET(ad->applet), date);
			}
		ad->old_yday = time_data->tm_yday;
		}
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

/* clean up function to free all the applet's memory and stop timers */
static void destroy_applet(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;

	gtk_timeout_remove(ad->update_timeout_id);
	if (ad->blinking) gtk_timeout_remove(ad->blink_timeout_id);

	skin_free(ad->skin);

	g_free(ad->mail_file);
	g_free(ad->newmail_exec_cmd);
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

static AppData *create_new_app(GtkWidget *applet)
{
        AppData *ad;

        ad = g_new0(AppData, 1);

	ad->applet = applet;
	ad->propwindow = NULL;
	ad->orient = ORIENT_UP;
	ad->sizehint = SIZEHINT_STANDARD;

	ad->mailsize = 0;
	ad->oldtime = 0;
	ad->old_week = -1;
	ad->old_n = 0;
	ad->blink_lit = 0;
	ad->blink_count = 0;

	ad->use_gmt = FALSE;
	ad->gmt_offset = 0;
	ad->mail_max = 100;

	ad->message_count = 0;

	/* (duration = BLINK_DELAY / 1000 * BLINK_TIMES) */
	ad->blink_delay = 166;
	ad->blink_times = 16;

	ad->am_pm_enable = FALSE;
	ad->always_blink = FALSE;

	/* mail file location */
	ad->mail_file = NULL;

	/* execute a command on button press */
	ad->reader_exec_cmd = NULL;

	/* execute a command on new mail */
	ad->newmail_exec_cmd = NULL;
	ad->exec_cmd_on_newmail = FALSE;

	ad->skin = NULL;

	ad->theme_file = NULL;

	property_load(APPLET_WIDGET(applet)->privcfgpath, ad);

	/* get mail filename from environment if not specified in the session file */
	if (ad->mail_file && !strcmp(ad->mail_file,"default"))
		{
		if (ad->mail_file) g_free(ad->mail_file);
		ad->mail_file = NULL;
		if (getenv ("MAIL"))
			ad->mail_file = g_strdup(getenv ("MAIL"));
		}

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

	display_events_init(ad);

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

	ad->update_timeout_id = gtk_timeout_add(1000, (GtkFunction)update_display, ad);
	
	return ad;
}

static GtkWidget * applet_start_new_applet(const gchar *goad_id,
					   const char **params, int nparams)
{
	GtkWidget *applet;

	g_return_val_if_fail(!strcmp(goad_id, "clockmail_applet"), NULL);

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

	applet_widget_init("clockmail_applet", VERSION, argc, argv, NULL, 0,
			   NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-clockmail.png");
	applet_factory_new("clockmail_applet_factory", NULL,
			   applet_start_new_applet);

	goad_id = (char *)goad_server_activation_id();
	if(goad_id && !strcmp(goad_id, "clockmail_applet")) {
		applet = applet_widget_new("clockmail_applet");
		if (!applet)
			g_error("Can't create applet!\n");

		create_new_app(applet);
	}

	applet_widget_gtk_main();
	return 0;
}
