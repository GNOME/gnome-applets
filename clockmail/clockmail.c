/*###################################################################*/
/*##                         clock & mail applet 0.1.5             ##*/
/*###################################################################*/

#include "clockmail.h"

#include "backgrnd.xpm"
#include "digmed.xpm"
#include "mailpics.xpm"

static void about_cb (AppletWidget *widget, gpointer data);
static void set_tooltip(gchar *newtext, AppData *ad);
static void redraw_display(AppData *ad);
static void update_mail_display(int n, AppData *ad);
static void draw_big_digit(int n,int x,int y, AppData *ad);
static void update_time_count(gint h, gint m, AppData *ad);
static gint blink_callback(AppData *ad);
static gint update_display(AppData *ad);
static void create_pixmaps(AppData *ad);
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

static void set_tooltip(gchar *newtext, AppData *ad)
{
	if (!ad->tiptext)
		gtk_tooltips_set_tip (ad->tooltips, ad->applet, newtext, NULL);
	else if (strcmp(newtext,ad->tiptext) != 0)
		gtk_tooltips_set_tip (ad->tooltips, ad->applet, newtext, NULL);

	if (ad->tiptext) g_free (ad->tiptext);
	ad->tiptext = strdup(newtext);
}

static void redraw_display(AppData *ad)
{
	gdk_window_set_back_pixmap(ad->display_area->window,ad->display,FALSE);
	gdk_window_clear(ad->display_area->window);
}

static void update_mail_display(int n, AppData *ad)
{
	if (n != ad->old_n)
		{
		switch (n)
		{
		case 0:
			gdk_draw_pixmap (ad->display,
				ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
				ad->mailpics, 0, 0, 2, 21, 42, 18);
			break;
		case 1:
			gdk_draw_pixmap (ad->display,
				ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
				ad->mailpics, 0, 18, 2, 21, 42, 18);
			break;
		case 2:
			gdk_draw_pixmap (ad->display,
				ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
				ad->mailpics, 0, 36, 2, 21, 42, 18);
			break;
		default:
			break;
		}
		ad->old_n = n;
		}
}


static void draw_big_digit(int n,int x,int y, AppData *ad)
{
	if (n == -1) gdk_draw_pixmap (ad->display,
			ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
			ad->digmed, 90, 0, x, y, 9, 16);
	if (n >= 0 && n <= 9)
		gdk_draw_pixmap (ad->display,
			ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
			ad->digmed, n*9, 0, x, y, 9, 16);
}

static void update_time_count(gint h, gint m, AppData *ad)
{
	gint hl,hr;
	gint ml,mr;

	if (ad->am_pm_enable)
		{
		if (h > 12) h -= 12;
		if (h == 0) h = 12;
		}

	hl = h / 10;
	hr = h - (hl * 10);
	if (hl == 0) hl = -1;
	ml = m / 10;
	mr = m - (ml * 10);
	draw_big_digit(hl,2,3, ad);
	draw_big_digit(hr,12,3, ad);
	draw_big_digit(ml,25,3, ad);
	draw_big_digit(mr,35,3, ad);
}

static gint blink_callback(AppData *ad)
{
	if (ad->blink_count == 0) ad->blink_lit = FALSE;

	ad->blink_count++;

	if (ad->blink_lit)
		{
		ad->blink_lit = FALSE;
		update_mail_display(1, ad);
		}
	else
		{
		ad->blink_lit = TRUE;
		update_mail_display(0, ad);
		}
	redraw_display(ad);

	if (ad->blink_count >= ad->blink_times || !ad->anymail)
		{
		if (ad->always_blink && ad->anymail)
			{
			if (ad->blink_count >= ad->blink_times) ad->blink_count = 1;
			}
		else
			{
			ad->blink_count = 0;
			if (ad->anymail)
				update_mail_display(2, ad);
			else
				update_mail_display(0, ad);
			redraw_display(ad);
			ad->blinking = FALSE;
			return FALSE;
			}
		}

	return TRUE;
}

static gint update_display(AppData *ad)
{
	time_t current_time;
	struct tm *time_data;
	gchar date[128];

	time(&current_time);
	time_data = localtime(&current_time);
	strftime(date, 128, "%a, %b %d", time_data);

	/* update time */
	update_time_count(time_data->tm_hour,time_data->tm_min, ad);

	/* now check mail */
	check_mail_file_status (FALSE, ad);

	if (!ad->blinking)
		{
		if (ad->anymail)
			if (ad->newmail || ad->always_blink)
				{
				update_mail_display(1, ad);
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
				update_mail_display(2, ad);
				}
		else
			{
			update_mail_display(0, ad);
			}
		redraw_display(ad);
		}

	/* set tooltip to the date */
	set_tooltip(date, ad);

	return TRUE;
}

static void create_pixmaps(AppData *ad)
{
	GdkBitmap *mask;
	GtkStyle *style;
	style = gtk_widget_get_style(ad->applet);

	ad->display_back = gdk_pixmap_create_from_xpm_d(ad->display_area->window, &mask,
		&style->bg[GTK_STATE_NORMAL], (gchar **)backgrnd_xpm);
	ad->digmed = gdk_pixmap_create_from_xpm_d(ad->display_area->window, &mask,
		&style->bg[GTK_STATE_NORMAL], (gchar **)digmed_xpm);
	ad->mailpics = gdk_pixmap_create_from_xpm_d(ad->display_area->window, &mask,
		&style->bg[GTK_STATE_NORMAL], (gchar **)mailpics_xpm);
}

static AppData *create_new_app(GtkWidget *applet)
{
        AppData *ad;

        ad = g_new(AppData, 1);

	ad->applet = applet;

	ad->oldsize = 0;
	ad->oldtime = 0;
	ad->tiptext = NULL;
	ad->old_n = 0;
	ad->blink_lit = 0;
	ad->blink_count = 0;

	/* (duration = BLINK_DELAY / 1000 * BLINK_TIMES) */
	ad->blink_delay = 750;
	ad->blink_times = 20;

	ad->am_pm_enable = FALSE;
	ad->always_blink = FALSE;

	/* mail file location */
	ad->mail_file = NULL;

	/* execute a command on new mail */
	ad->newmail_exec_cmd = NULL;
	ad->exec_cmd_on_newmail = FALSE;

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
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   ad);

	/* create a tooltip widget to display song title */
	ad->tooltips = gtk_tooltips_new();
	set_tooltip(_("date"), ad);

	/* frame for all widgets */
	ad->frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(ad->frame), GTK_SHADOW_IN);
	gtk_widget_show(ad->frame);

	ad->display_area = gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(ad->display_area),46,42);
	gtk_container_add(GTK_CONTAINER(ad->frame),ad->display_area);
	gtk_widget_show(ad->display_area);

	applet_widget_add(APPLET_WIDGET(ad->applet), ad->frame);

	gtk_widget_realize(ad->display_area);

	ad->display = gdk_pixmap_new(ad->display_area->window,46,42,-1);

	create_pixmaps(ad);

	gtk_widget_show(ad->frame);

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

	gdk_draw_pixmap (ad->display,
		ad->display_area->style->fg_gc[GTK_WIDGET_STATE(ad->display_area)],
		ad->display_back, 0, 0, 0, 0, 46, 42);

	update_display(ad);

	ad->update_timeout_id = gtk_timeout_add(10000, (GtkFunction)update_display, ad);
	
	return ad;
}

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
/*	AppData *ad = data;
	orient change (nothing is done in this applet yet) */
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
