/*###################################################################*/
/*##                         clock & mail applet 0.2.0             ##*/
/*###################################################################*/

#include "clockmail.h"

static void about_cb (AppletWidget *widget, gpointer data);
static void set_tooltip(struct tm *time_data, AppData *ad);
static void redraw_display(AppData *ad);
static void update_mail_display(int n, AppData *ad);
static void update_time_count(gint h, gint m, gint s, AppData *ad);
static gint blink_callback(gpointer data);
static gint update_display(gpointer data);
static void destroy_applet(GtkWidget *widget, gpointer data);
static AppData *create_new_app(GtkWidget *applet);
static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data);
static gint applet_save_session(GtkWidget *widget, char *privcfgpath,
					char *globcfgpath, gpointer data);
static void applet_start_new_applet(const gchar *param, gpointer data);

static void about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	const gchar *authors[2];
	gchar version[32];

	sprintf(version,_("%d.%d.%d"),CLOCKMAIL_APPLET_VERSION_MAJ,
		CLOCKMAIL_APPLET_VERSION_MIN, CLOCKMAIL_APPLET_VERSION_REV);

	authors[0] = _("John Ellis (gqview@geocities.com)");
	authors[1] = NULL;

        about = gnome_about_new ( _("Clock and Mail Notify Applet"), version,
			_("(C) 1998"),
			authors,
			_("Released under the GNU general public license.\n"
			"Basic digital clock with date in a tooltip. "
			"Optional 12/24 time display. Mail blinking can be "
			"for any unread mail, or only briefly when new mail arrives."),
			NULL);
	gtk_widget_show (about);
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
		ad->oldsize = 0;
		ad->oldtime = 0;
		return;
		}

	/* no need to continue if a mail item is not available */
	if (!ad->skin->mail) return;

	/* check if mail file contains something */
	if (!ad->mail_file) return;

	status = stat (ad->mail_file, &s);
	if (status < 0){
		ad->oldsize = 0;
		ad->anymail = ad->newmail = ad->unreadmail = 0;
		return;
	}
	
	newsize = s.st_size;
	newtime = s.st_mtime;
	ad->anymail = newsize > 0;
	ad->unreadmail = (s.st_mtime >= s.st_atime && newsize > 0);

	checktime = (newtime > ad->oldtime);
	if (newsize >= ad->oldsize && ad->unreadmail && checktime){
		ad->newmail = 1;
		ad->mailcleared = 0;
	} else
		ad->newmail = 0;
	ad->oldtime = newtime;
	ad->oldsize = newsize;
}

static void set_tooltip(struct tm *time_data, AppData *ad)
{
	if (time_data->tm_yday != ad->old_yday)
		{
		gchar date[128];
		strftime(date, 128, "%a, %b %d", time_data);

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
				sprintf(gmt_text, _(" (GMT %+d)"), ad->gmt_offset);
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

static void update_mail_display(int n, AppData *ad)
{
	if (n != ad->old_n)
		{
		draw_item(ad->skin->mail, n, ad);
		ad->old_n = n;
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
}

static void update_date_displays(gint year, gint month, gint day, gint weekday, AppData *ad)
{
	/* no point in drawing things again if they don't change */
	if (ad->old_week == weekday) return;

	draw_number(ad->skin->year, year, ad);
	draw_number(ad->skin->month, month, ad);
	draw_number(ad->skin->day, day, ad);
	draw_item(ad->skin->month_txt, month, ad);
	draw_item(ad->skin->week_txt, weekday, ad);

	ad->old_week = weekday;
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
	update_mail_display(ad->blink_lit, ad);
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
				update_mail_display(ad->mail_sections - 1, ad);
			else
				update_mail_display(0, ad);
			redraw_display(ad);
			ad->blinking = FALSE;
			return FALSE;
			}
		}

	return TRUE;
}

static gint update_display(gpointer data)
{
	AppData *ad = data;
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
				time_data->tm_wday, ad);

	/* set tooltip to the date */
	set_tooltip(time_data, ad);

	/* now check mail */
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
				update_mail_display(ad->mail_sections - 1, ad);
				}
		else
			{
			update_mail_display(0, ad);
			}
		}

	redraw_display(ad);

	return TRUE;
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
}

static AppData *create_new_app(GtkWidget *applet)
{
        AppData *ad;

        ad = g_new0(AppData, 1);

	ad->applet = applet;
	ad->propwindow = NULL;
	ad->orient = ORIENT_UP;

	ad->oldsize = 0;
	ad->oldtime = 0;
	ad->old_week = -1;
	ad->old_n = 0;
	ad->blink_lit = 0;
	ad->blink_count = 0;

	ad->use_gmt = FALSE;
	ad->gmt_offset = 0;

	/* (duration = BLINK_DELAY / 1000 * BLINK_TIMES) */
	ad->blink_delay = 166;
	ad->blink_times = 16;

	ad->am_pm_enable = FALSE;
	ad->always_blink = FALSE;

	/* mail file location */
	ad->mail_file = NULL;

	/* execute a command on new mail */
	ad->newmail_exec_cmd = NULL;
	ad->exec_cmd_on_newmail = FALSE;

	ad->skin = NULL;
	ad->skin_h = NULL;
	ad->skin_v = NULL;

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

	gtk_signal_connect(GTK_OBJECT(ad->applet),"change_orient",
		GTK_SIGNAL_FUNC(applet_change_orient), ad);

	/* create a tooltip widget to display song title */
	ad->tooltips = gtk_tooltips_new();

	ad->display_area = gtk_drawing_area_new();
	gtk_signal_connect(GTK_OBJECT(ad->display_area), "destroy",
		GTK_SIGNAL_FUNC(destroy_applet), ad);
	gtk_widget_show(ad->display_area);

	applet_widget_add(APPLET_WIDGET(ad->applet), ad->display_area);

	gtk_widget_realize(ad->display_area);

	if (!change_to_skin(ad->theme_file, ad))
		{
		printf("Failed to load skin %s, loading default\n", ad->theme_file);
		change_to_skin(NULL, ad);
		}

	gtk_widget_show(ad->applet);

	gtk_signal_connect(GTK_OBJECT(ad->applet),"save_session",
		GTK_SIGNAL_FUNC(applet_save_session), ad);

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
	if (ad->orient == ORIENT_LEFT || ad->orient == ORIENT_RIGHT)
		{
		if (ad->skin_v && ad->skin != ad->skin_v)
			{
			ad->skin = ad->skin_v;
			sync_window_to_skin(ad);
			}
		}
	else
		{
		if (ad->skin != ad->skin_h)
			{
			ad->skin = ad->skin_h;
			sync_window_to_skin(ad);
			}
		}
}

static gint applet_save_session(GtkWidget *widget, gchar *privcfgpath,
					gchar *globcfgpath, gpointer data)
{
	AppData *ad = data;
        property_save(privcfgpath, ad);
        return FALSE;
}

static void applet_start_new_applet(const gchar *param, gpointer data)
{
	GtkWidget *applet;

	applet = applet_widget_new_with_param(param);
		if (!applet)
			g_error("Can't create applet!\n");

	create_new_app(applet);
}

int main (int argc, char *argv[])
{
	GtkWidget *applet;

	/* Initialize the i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init("clockmail_applet", NULL, argc, argv, 0, NULL,
			argv[0], TRUE, TRUE, applet_start_new_applet, NULL);

	applet = applet_widget_new();
	if (!applet)
		g_error("Can't create applet!\n");

	create_new_app(applet);

	applet_widget_gtk_main();
	return 0;
}
