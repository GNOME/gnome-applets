/*###################################################################*/
/*##                                                               ##*/
/*###################################################################*/

#include "slashapp.h"
#include "http_get.h"

static void flush_newline_chars(gchar *text, gint max);
static int get_current_headlines(gpointer data);
static int startup_delay_cb(gpointer data);
static void about_cb (AppletWidget *widget, gpointer data);
static void destroy_applet(GtkWidget *widget, gpointer data);
static gint applet_save_session(GtkWidget *widget, gchar *privcfgpath,
					gchar *globcfgpath, gpointer data);
static AppData *create_new_app(GtkWidget *applet);
static void applet_start_new_applet(const gchar *param, gpointer data);

static void flush_newline_chars(gchar *text, gint max)
{
	gchar *p = text;
	gint c = 0;
	while (p[0] != '\0' && c <= max)
		{
		if (p[0] == '\n')
			p[0] = '\0';
		else
			{
			p++;
			c++;
			if (c >= max) p[0] = '\0';
			}
		}
}

static int get_current_headlines(gpointer data)
{
	AppData *ad = data;
	gchar buf[256];
	gchar headline[128];
	gchar url[128];
	gchar entrydate[64];
	gchar author[32];
	gchar department[128];
	gchar category[32];
	FILE *slash_file = NULL;

	if ((slash_file = fopen("slashnews", "w")) == NULL)
		{
		fprintf(stderr, "Failed to open file \"%s\": %s\n",
				"slashnews", strerror(errno));
		return TRUE;
		}
	http_get_to_file("slashdot.org", 80, "/ultramode.txt", slash_file);
	fclose(slash_file);

	/* refresh the headlines in the display */
	if ((slash_file = fopen("slashnews", "r")) == NULL)
		{
		fprintf(stderr, "Failed to open file \"%s\": %s\n",
				"slashnews", strerror(errno));
		return TRUE;
		}

	/* clear the current headlines from display list */
	remove_all_lines(ad);

	while (fgets(buf, sizeof(buf), slash_file) != NULL)
		{
		if (strcmp(buf, "%%\n") == 0) 
			{
			if (fgets(buf, sizeof(buf), slash_file) != NULL)
				{
				gchar *text;
				strncpy(headline, buf, 80);
				flush_newline_chars(headline, 80);
				g_print("%d long: %s\n", strlen(headline), headline);
				fgets(buf, sizeof(buf), slash_file);
				strncpy(url, buf, 120);
				flush_newline_chars(url, 120);
				fgets(buf, sizeof(buf), slash_file);
				strncpy(entrydate, buf, 64);
				flush_newline_chars(entrydate, 23);
				fgets(buf, sizeof(buf), slash_file);
				strncpy(author, buf, 10);
				flush_newline_chars(author, 8);
				fgets(buf, sizeof(buf), slash_file);
				strncpy(department, buf, 80);
				flush_newline_chars(department, 80);
				fgets(buf, sizeof(buf), slash_file);
				strncpy(category, buf, 20);
				flush_newline_chars(category, 16);

				/* add the headline */
				text = g_strconcat(headline, "\n  [", entrydate, " - ", author, "]", NULL);
				add_info_line(ad, text, NULL, 0, FALSE, FALSE, 30);
				/* a space separater, could include a graphic divider too */
				add_info_line(ad, "  ", NULL, 0, FALSE, FALSE, 0);
				g_free(text);
				}
			}
		}

	fclose(slash_file);

	return TRUE;
}

static int startup_delay_cb(gpointer data)
{
	AppData *ad = data;
	get_current_headlines(ad);
	ad->startup_timeout_id = 0;
	return FALSE;	/* return false to stop this timeout callback, needed only once */
}

static void about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	const gchar *authors[8];
	gchar version[32];

	sprintf(version,_("%d.%d.%d"),APPLET_VERSION_MAJ,
		APPLET_VERSION_MIN, APPLET_VERSION_REV);

	authors[0] = _("Justin Maurer <mike911@clark.net>");
	authors[1] = _("Craig Small <csmall@small.dropbear.co.uk>");
	authors[2] = _("John Ellis <johne@bellatlantic.net> - Display engine");
	authors[3] = NULL;

        about = gnome_about_new ( _("Slash Applet"), version,
			_("(C) 1998"),
			authors,
			_("Released under the GNU general public license.\n"
			"Display scroller for slashapp. "),
			NULL);
	gtk_widget_show (about);
}


static void destroy_applet(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;

	gtk_timeout_remove(ad->display_timeout_id);
	gtk_timeout_remove(ad->headline_timeout_id);
	if (ad->startup_timeout_id > 0) gtk_timeout_remove(ad->startup_timeout_id);

	free_all_info_lines(ad->text);
	gtk_widget_destroy(ad->display_w);
	gtk_widget_destroy(ad->disp_buf_w);
	gtk_widget_destroy(ad->background_w);
	g_free(ad);
}

static gint applet_save_session(GtkWidget *widget, gchar *privcfgpath,
					gchar *globcfgpath, gpointer data)
{
	AppData *ad = data;
	property_save(privcfgpath, ad);
        return FALSE;
}

static AppData *create_new_app(GtkWidget *applet)
{
	AppData *ad;
	ad = g_new0(AppData, 1);

	ad->applet = applet;

	init_app_display(ad);
        gtk_signal_connect(GTK_OBJECT(ad->applet), "destroy",
                GTK_SIGNAL_FUNC(destroy_applet), ad);

	property_load(APPLET_WIDGET(applet)->privcfgpath, ad);

	add_info_line(ad, "Slashdot.org Applet\n", NULL, 0, TRUE, 1, 0);
	add_info_line(ad, "Loading headlines........\n", NULL, 0, FALSE, 1, 20);


/* applet signals */
        gtk_signal_connect(GTK_OBJECT(applet),"save_session",
                                GTK_SIGNAL_FUNC(applet_save_session),
                                ad);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
                                              "properties",
                                              GNOME_STOCK_MENU_PROP,
                                              _("Properties..."),
                                              property_show,
                                              ad);
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
                                              "about",
                                              GNOME_STOCK_MENU_ABOUT,
                                              _("About..."),
                                              about_cb, NULL);

	ad->headline_timeout_id = gtk_timeout_add(1800000, get_current_headlines, ad);

        gtk_widget_show(ad->applet);

	/* this is so the app is displayed first before calling the download command */
	ad->startup_timeout_id = gtk_timeout_add(5000, startup_delay_cb, ad);

	return ad;
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

	applet_widget_init("scroll_applet", NULL, argc, argv, 0, NULL,
			argv[0], TRUE, TRUE, applet_start_new_applet, NULL);

	applet = applet_widget_new();
	if (!applet)
		g_error("Can't create applet!\n");

	create_new_app(applet);

	applet_widget_gtk_main();
	return 0;
}
