#include <gnome.h>
#include <ghttp.h>
#include "gticker.h"
#include "gticker.xpm"

static AppData *create_gticker(GtkWidget *applet);
static void start_new_applet(const gchar *param, gpointer data);
static int get_current_headlines(gpointer data);

int main(int argc, char *argv[])
{
     GtkWidget *applet;
     bindtextdomain(PACKAGE, GNOMELOCALEDIR);
     textdomain(PACKAGE);

     applet_widget_init("gticker_applet", NULL, argc, argv, 0, NULL, argv[0], 
		     TRUE, TRUE, start_new_applet, NULL);
     applet = applet_widget_new();
     if (!applet)
             g_error("Can't create applet!\n");
     create_new_applet(applet);
     applet_widget_gtk_main();
     return 0;
}

static AppData *create_gticker(GtkWidget *applet)
{
  AppData *appdata;
  appdata = g_new0(AppData, 1);

  appdata->applet = applet;
  appdata->applet_window = NULL;
  appdata->gtkicker_dir = check_for_dir(gnome_util_home_file("gticker"));

  init_app_display(appdata);
  gtk_signal_connect(GTK_OBJECT(appdata->applet), "destroy",
		     GTK_SIGNAL_FUNC(destroy_applet), appdata);
  property_load(APPLET_WIDGET(applet)->privcfgpath, appdata);
  add_info_line(appdata, "Welcome to GTicker!", NULL, 1, TRUE, 1, 0);
  add_info_line(appdata, _("Loading headlines..."), NULL, 0, FALSE, 1, 20);

  gtk_signal_connect(GTK_OBJECT(applet), "save_session",
		     GTK_SIGNAL_FUNC(applet_save_session),
		     appdata);
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"properties",
					GNOME_STOCK_MENU_PROP,
					_("Properties..."),
					property_show,
					appdata);
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"about",
					GNOME_STOCK_MENU_ABOUT,
					_("About..."),
					about_cb, NULL);
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"articles",
					GNOME_STOCK_MENU_BOOK_OPEN,
					_("Show Article Listing"),
					article_window, appdata);
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"refresh",
					GNOME_STOCK_MENU_REFRESH,
					_("Refresh Articles"),
					refresh_cb, appdata);

  appdata->headline_timeout_id = gtk_timeout_add(1800000, 
		  get_current_headlines, appdata);
  gtk_widget_show(appdata->applet);
  return appdata;
}

static void start_new_applet(const gchar *param, gpointer data)
{
  GtkWidget *applet;
 
  applet = applet_widget_new_with_param(param);
          if (!applet)
	    g_error("Can't create applet!\n");
  create_new_app(applet);
}

static int get_current_headlines(gpointer data)
{
  AppData *appdata = data;
  InfoData *id;
  GtkWidget *icon;
  ghttp_request *req = ghttp_request_new();	
 
  gchar http_server[128] = "slashdot.org"; /* If your domain is longer this.. */
  gchar http_filename[128] = "/ultramode.txt";
  gint http_port = 80;	/* blizzard: make gnome-http support various ports */
    					/* Actually, a bit later, this will 
					   check preferences to get these 
					   values upon declaration */
  gchar final_url[334];
  ghttp_set_uri(req, g_strconcat("http://", http_server, http_filename, NULL));
  
  gchar buf[256];
  gchar headline[128];
  gchar url[128];
  gchar graphicurl[128];
  gchar author[16];
  gchar letterday[6];
  gchar numberday[2];
  gchar time[5];
  gchar timezone[3];
  gchar free1[128];
  gchar free2[128];
  FILE *gticker_file = NULL;
  gchar *filename = g_strconcat(appdata->gticker_dir, "/gticker", NULL);
  gint delay = appdata->article_delay / 10 * (1000 / UPDATE_DELAY);
  
  set_mouse_cursor(appdata, GDK_WATCH);
  while(gtk_events_pending())
    gtk_main_iteration();

  if ((gticker_file = fopen(filename, "w")) == NULL)
    {
      fprintf(stderr, "Failed to open file \"%s\": %s\n",
	      filename, strerror(errno));
      g_free(filename);
      set_mouse_cursor(appdata, GDK_LEFT_PTR);
      return TRUE;
    }

  fclocse(gticker_file); 

}
