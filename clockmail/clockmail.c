/* GNOME clock & mailcheck applet
 * (C) 1999 John Ellis
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

static void about_cb (AppletWidget *widget, gpointer data);
static void set_tooltip(struct tm *time_data, AppData *ad);
static void redraw_display(AppData *ad);
static void update_mail_display(int n, AppData *ad, gint force);
static void update_mail_count(AppData *ad, gint force);
static void update_mail_amount_display(AppData *ad, gint force);
static void update_time_count(gint h, gint m, gint s, AppData *ad);
static void update_date_displays(gint year, gint month, gint day, gint weekday, AppData *ad, gint force);
static void update_time_and_date(AppData *ad, gint force);
static gint blink_callback(gpointer data);
static void update_mail_status(AppData *ad, gint force);
static gint update_display(gpointer data);
static void applet_change_back(GtkWidget *applet, PanelBackType type, char *pixmap,
				GdkColor *color, gpointer data);
static void destroy_applet(GtkWidget *widget, gpointer data);
static AppData *create_new_app(GtkWidget *applet);
static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data);

#ifdef HAVE_PANEL_PIXEL_SIZE
static void applet_change_pixel_size(GtkWidget *w, int size, gpointer data);
#endif

static gint applet_save_session(GtkWidget *widget, char *privcfgpath,
					char *globcfgpath, gpointer data);
static GtkWidget * applet_start_new_applet(const gchar *goad_id, const char **params, int nparams);

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
			_("(C) 1999"),
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

static void set_tooltip(struct tm *time_data, AppData *ad)
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
			gtk_tooltips_set_tip (ad->tooltips, ad->applet, buf, NULL);
			g_free(buf);
			}
		else
			gtk_tooltips_set_tip (ad->tooltips, ad->applet, date, NULL);
		ad->old_yday = time_data->tm_yday;
		}
}

static void redraw_display(AppData *ad)
{
	redraw_skin(ad);
}

static void update_mail_display(int n, AppData *ad, gint force)
{
	if (force)
		{
		draw_item(ad->skin->mail, ad->old_n, ad);
		return;
		}
	if (n != ad->old_n)
		{
		draw_item(ad->skin->mail, n, ad);
		ad->old_n = n;
		}
}

static void update_mail_count(AppData *ad, gint force)
{
	if (!ad->skin->messages && !ad->skin->mail_amount) return;

	if (ad->mail_file && !force)
		{
		FILE *f = 0;
		gchar buf[1024];

		f = fopen(ad->mail_file, "r");

		if (f)
			{
			gint c = 0;
			while (fgets(buf, sizeof(buf), f) != NULL)
				{
				if (buf[0] == 'F' && !strncmp(buf, "From ", 5)) c++;
				/* Hack alert! this was added to ignore PINE's false mail entries */
				if (buf[0] == 'X' && !strncmp(buf, "X-IMAP:", 7) && c > 0) c--;
				}
                        fclose(f);
			ad->message_count = c;
                        }
		else
			{
			ad->message_count = 0;
			}
		}

	draw_number(ad->skin->messages, ad->message_count, ad);

	if (ad->skin->mail_amount)
		{
		gint p;

		if (ad->mail_max < 10) ad->mail_max = 10;
		if (ad->message_count >= ad->mail_max)
			p = ad->skin->mail_amount->sections - 1;
		else
			p = (float)ad->message_count / ad->mail_max * ad->skin->mail_amount->sections;
		if (p == 0 && ad->anymail && ad->skin->mail_amount->sections > 1) p = 1;
		draw_item(ad->skin->mail_amount, p, ad);
		}
}

static void update_mail_amount_display(AppData *ad, gint force)
{
	if (ad->mailsize != ad->old_amount || force)
		{
		update_mail_count(ad, force);
		draw_number(ad->skin->mail_count, ad->mailsize / 1024, ad);
		ad->old_amount = ad->mailsize;
		}
}

static void update_time_count(gint h, gint m, gint s, AppData *ad)
{
	if (ad->am_pm_enable)
		{
		if (h > 12) h -= 12;
		if (h == 0) h = 12;
		}
	draw_number(ad->skin->hour, h, ad);
	draw_number(ad->skin->min, m, ad);
	draw_number(ad->skin->sec, s, ad);

	if (h > 12) h -= 12;
	draw_clock(ad->skin->clock, h, m, s, ad);
}

static void update_date_displays(gint year, gint month, gint day, gint weekday, AppData *ad, gint force)
{
	/* no point in drawing things again if they don't change */
	if (ad->old_week == weekday && !force) return;

	draw_number(ad->skin->year, year, ad);
	draw_number(ad->skin->month, month, ad);
	draw_number(ad->skin->day, day, ad);
	draw_item(ad->skin->month_txt, month, ad);
	draw_item(ad->skin->week_txt, weekday, ad);

	ad->old_week = weekday;
}

static void update_time_and_date(AppData *ad, gint force)
{
	time_t current_time;
	struct tm *time_data;

	time(&current_time);

	/* if using a non-local time, calculate it from GMT */
	if (ad->use_gmt)
		{
		current_time += (time_t)ad->gmt_offset * 3600;
		time_data = gmtime(&current_time);
		}
	else
		time_data = localtime(&current_time);

	/* update time */
	update_time_count(time_data->tm_hour,time_data->tm_min, time_data->tm_sec, ad);

	/* update date */
	update_date_displays(time_data->tm_year, time_data->tm_mon, time_data->tm_mday,
				time_data->tm_wday, ad, force);

	/* set tooltip to the date */
	set_tooltip(time_data, ad);
}

static gint blink_callback(gpointer data)
{
	AppData *ad = data;
	ad->blink_lit++;

	if (ad->blink_lit >= ad->mail_sections - 1)
		{
		ad->blink_lit = 0;
		ad->blink_count++;
		}
	update_mail_display(ad->blink_lit, ad, FALSE);
	redraw_display(ad);

	if (ad->blink_count >= ad->blink_times || !ad->anymail)
		{
		if (ad->always_blink && ad->anymail)
			{
			if (ad->blink_count >= ad->blink_times) ad->blink_count = 1;
			}
		else
			{
			/* reset counters for next time */
			ad->blink_count = 0;
			ad->blink_lit = 0;
			if (ad->anymail)
				update_mail_display(ad->mail_sections - 1, ad, FALSE);
			else
				update_mail_display(0, ad, FALSE);
			redraw_display(ad);
			ad->blinking = FALSE;
			return FALSE;
			}
		}

	return TRUE;
}

static void update_mail_status(AppData *ad, gint force)
{
	if (force)
		{
		update_mail_amount_display(ad, TRUE);
		update_mail_display(0, ad, TRUE);
		return;
		}
	check_mail_file_status (FALSE, ad);

	if (!ad->blinking)
		{
		if (ad->anymail)
			if (ad->newmail || ad->always_blink)
				{
				ad->blinking = TRUE;
				ad->blink_timeout_id = gtk_timeout_add(ad->blink_delay,
						(GtkFunction)blink_callback, ad);
				if (ad->newmail && ad->exec_cmd_on_newmail)
					{
					if (ad->newmail_exec_cmd && strlen(ad->newmail_exec_cmd) > 0)
						gnome_execute_shell(NULL, ad->newmail_exec_cmd);
					}
				}
			else
				{
				update_mail_display(ad->mail_sections - 1, ad, FALSE);
				}
		else
			{
			update_mail_display(0, ad, FALSE);
			}
		}

	update_mail_amount_display(ad, FALSE);
}

void redraw_all(gpointer data)
{
	AppData *ad = data;
	update_time_and_date(ad, TRUE);
	update_mail_status(ad, TRUE);
}

static gint update_display(gpointer data)
{
	AppData *ad = data;

	update_time_and_date(ad, FALSE);
	update_mail_status(ad, FALSE);
	redraw_display(ad);

	return TRUE;
}

void reload_skin(AppData *ad)
{
	if (ad->theme_file && strlen(ad->theme_file) == 0)
		{
		change_to_skin(NULL, ad);
		}
	else if (!change_to_skin(ad->theme_file, ad))
		{
		printf("Failed to load skin %s, loading default\n", ad->theme_file);
		change_to_skin(NULL, ad);
		}
}

static void applet_change_back(GtkWidget *applet, PanelBackType type, char *pixmap,
				GdkColor *color, gpointer data)
{
	AppData *ad = data;
	GtkStyle *ns;

	switch (type)
		{
		case PANEL_BACK_NONE :
			ns = gtk_style_new();
			gtk_style_ref(ns);
			gtk_widget_set_style(GTK_WIDGET(ad->applet), ns);
			gtk_style_unref(ns);
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
	return;
	applet = NULL;
}

/* clean up function to free all the applet's memory and stop timers */
static void destroy_applet(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;

	gtk_timeout_remove(ad->update_timeout_id);
	if (ad->blinking)
		gtk_timeout_remove(ad->blink_timeout_id);

	free_skin(ad->skin_h);
	free_skin(ad->skin_v);

	g_free(ad->mail_file);
	g_free(ad->newmail_exec_cmd);
	g_free(ad->theme_file);
	g_free(ad);
	return;
	widget = NULL;
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
	if (!strcmp(ad->mail_file,"default"))
		{
		if (ad->mail_file) g_free(ad->mail_file);
		ad->mail_file = NULL;
		if (getenv ("MAIL"))
			ad->mail_file = g_strdup(getenv ("MAIL"));
		}

	/* create a tooltip widget to display song title */
	ad->tooltips = gtk_tooltips_new();

	ad->display_area = gtk_drawing_area_new();
	gtk_signal_connect(GTK_OBJECT(ad->display_area), "destroy",
		GTK_SIGNAL_FUNC(destroy_applet), ad);
	gtk_widget_show(ad->display_area);

	applet_widget_add(APPLET_WIDGET(ad->applet), ad->display_area);

	skin_event_init(ad);

	gtk_signal_connect(GTK_OBJECT(ad->applet),"change_orient",
		GTK_SIGNAL_FUNC(applet_change_orient), ad);

#ifdef HAVE_PANEL_PIXEL_SIZE
	gtk_signal_connect(GTK_OBJECT(ad->applet),"change_pixel_size",
		GTK_SIGNAL_FUNC(applet_change_pixel_size), ad);
#endif

	gtk_widget_set_usize(ad->applet, 5, 5); /* so that a large default is not shown */
	gtk_widget_show(ad->applet);

	reload_skin(ad);

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

	update_display(ad);

	ad->update_timeout_id = gtk_timeout_add(1000, (GtkFunction)update_display, ad);
	
	return ad;
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
	
	if(size<PIXEL_SIZE_STANDARD)
		ad->sizehint = SIZEHINT_TINY;
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

static gint applet_save_session(GtkWidget *widget, gchar *privcfgpath,
					gchar *globcfgpath, gpointer data)
{
	AppData *ad = data;
        property_save(privcfgpath, ad);
	return FALSE;
	widget = NULL;
	globcfgpath = NULL;
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

	applet_widget_init("clockmail_applet", VERSION, argc, argv, NULL, 0,
			   NULL);

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
