#include <gnome.h>
#include <ghttp.h>
#include <errno.h>
#include "gticker.h"
#include "gticker.xpm"

static AppData *create_new_app(GtkWidget *applet);
static int get_current_headlines(gpointer data);
static int startup_delay_cb(gpointer data);
static void start_new_applet(const gchar *param, gpointer data);
static void destroy_applet(GtkWidget *widget, gpointer data);
static void about_cb (AppletWidget *widget, gpointer data);
static void show_article_window(AppletWidget *widget, gpointer data);	
static void refresh_cb(AppletWidget *widget, gpointer data);
static void destroy_article_window(GtkWidget *w, gpointer data);
static void populate_article_window(AppData *ad);
static void article_button_cb(GtkWidget *button, gpointer data);
static void launch_url(AppData *ad, gchar *url);
static gchar *check_for_dir(char *d);
static gint applet_save_session(GtkWidget *widget, gchar *privcfgpath,
                                gchar *globcfgpath, gpointer data);


int main(int argc, char *argv[])
{
     GtkWidget *applet;
     GList *list;
     bindtextdomain(PACKAGE, GNOMELOCALEDIR);
     textdomain(PACKAGE);

     list = g_list_append(NULL,"gticker_applet");
  applet_widget_init("gticker_applet", NULL, argc, argv, NULL, 0, NULL);
     g_list_free(list);
     applet = applet_widget_new("gticker_applet");
     if (!applet)
             g_error("Can't create applet!\n");
     create_new_app(applet);
     applet_widget_gtk_main();
     return 0;
}

static AppData *create_new_app(GtkWidget *applet)
{
  AppData *appdata;
  appdata = g_new0(AppData, 1);

  appdata->applet = applet;
  appdata->article_window = NULL;
  appdata->gticker_dir = check_for_dir(gnome_util_home_file("gticker"));

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
					show_article_window, appdata);
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"refresh",
					GNOME_STOCK_MENU_REFRESH,
					_("Refresh Articles"),
					refresh_cb, appdata);

  appdata->headline_timeout_id = gtk_timeout_add(1800000, 
		  get_current_headlines, appdata);
  gtk_widget_show(appdata->applet);
  appdata->startup_timeout_id = gtk_timeout_add(5000, startup_delay_cb, 
		  appdata);
  return appdata;
}

static void start_new_applet(const gchar *goad_id, gpointer data)
{
  GtkWidget *applet;
 
  applet = applet_widget_new(goad_id);
          if (!applet)
	    g_error("Can't create applet!\n");
  create_new_app(applet);
}

static int get_current_headlines(gpointer data)
{
  AppData *appdata = data;
  InfoData *id;
  GdkImlibImage *splash = NULL;
  GtkWidget *splash_pixmap = NULL;
  ghttp_request *req = ghttp_request_new();	
 
  gchar http_server[128] = "mike911.clark.net";
  gchar http_filename[128] = "/~acf/gticker/gticker.txt";
  gint http_port = 80;
  gchar buf[256];
  gchar headline[128];
  gchar url[128], splashurl[128];
  gchar graphicurl[128];
  gchar author[16];
  gchar letterday[6];
  gchar numberday[2];
  gchar time[5];
  gchar timezone[3];
  gchar free1[128];
  gchar free2[128];
  FILE *gticker_file = NULL, *splash_file = NULL;
  gchar *filename = g_strconcat(appdata->gticker_dir, "/", "headlines", NULL);
  gchar *splashfilename = g_strconcat(appdata->gticker_dir, "/", "splashurl", 
		  NULL);
  gint delay = appdata->article_delay / 10 * (1000 / UPDATE_DELAY);
 
  ghttp_set_uri(req, g_strconcat("http://", http_server, http_filename, NULL));
  ghttp_set_header(req, http_hdr_Connection, "close");
  ghttp_prepare(req);
  ghttp_process(req);

  if ((gticker_file = fopen(filename, "w")) == NULL) {
	  fprintf(stderr, "Failed to open file \"%s\": %s\n", filename, 
			  strerror(errno));
	  g_free(filename);
	  set_mouse_cursor(appdata, GDK_LEFT_PTR);
	  return TRUE;
  }
  fprintf(stderr, "body len is %d\n", ghttp_get_body_len(req));
  fwrite(ghttp_get_body(req), ghttp_get_body_len(req), 1, gticker_file);
  fclose(gticker_file);
  ghttp_close(req);
  ghttp_clean(req);

  if ((gticker_file = fopen(filename, "r")) == NULL) {
	fprintf(stderr, "Failed to open file \"%s\": %s\n", filename,
			strerror(errno));
	g_free(filename);
	set_mouse_cursor(appdata, GDK_LEFT_PTR);
	return TRUE;
  }

  remove_all_lines(appdata);
  fgets(buf, sizeof(buf), gticker_file);
  fgets(buf, sizeof(buf), gticker_file);
  strncpy(splashurl, buf, 80);
  /* get rid of that pesky \n */
  splashurl[strlen(splashurl) -1] = '\0';
  ghttp_set_uri(req, splashurl);
  ghttp_set_header(req, http_hdr_Connection, "close");
  ghttp_prepare(req); 
  ghttp_process(req);
  if ((splash_file = fopen(splashfilename, "w")) == NULL) {
	  fprintf(stderr, "Failed to open file \"%s\": %s\n", splashfilename,
			  strerror(errno));
	  g_free(splashfilename);
	  set_mouse_cursor(appdata, GDK_LEFT_PTR);
	  return TRUE;
  }
 
  g_print("About to fwrite()\n");
  g_print("i'm writing this many bits: %d\n!", fwrite(ghttp_get_body(req), 
		  ghttp_get_body_len(req), 1, splash_file));
  fclose(splash_file);
  ghttp_close(req);
  ghttp_clean(req);

  if ((splash_file = fopen(splashfilename, "r")) == NULL) {
	  fprintf(stderr, "Failed to open file \"%s\": %s\n", splashfilename,
			  strerror(errno));
	  g_free(splashfilename);
	  set_mouse_cursor(appdata, GDK_LEFT_PTR);
	  return TRUE;
  }
  
  g_print("1\n");
  splash = gdk_imlib_load_image(splashfilename);
  g_print("2\n");
  gdk_imlib_render(splash, splash->rgb_width, splash->rgb_height);
  g_print("3\n");
  splash_pixmap = gtk_pixmap_new(splash->pixmap, splash->shape_mask);
  g_print("4\n");
  id = add_info_line_with_pixmap(appdata, "", splash_pixmap, 0, FALSE, 1, 
		  delay);
  g_print("5\n");
  fclose(splash_file); 
  g_print("6\n");
  if (req)
    ghttp_request_destroy(req);
  return TRUE;
}

static void destroy_applet(GtkWidget *widget, gpointer data)
{
        AppData *ad = data;
        gtk_timeout_remove(ad->display_timeout_id);
        gtk_timeout_remove(ad->headline_timeout_id);
        if (ad->startup_timeout_id > 0) 
			gtk_timeout_remove(ad->startup_timeout_id);
        free_all_info_lines(ad);
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

static void about_cb (AppletWidget *widget, gpointer data)
{
        GtkWidget *about;
        const gchar *authors[3];
        gchar version[32];
        sprintf(version,_("%d.%d.%d"),APPLET_VERSION_MAJ, APPLET_VERSION_MIN, 
			APPLET_VERSION_REV);
        authors[0] = _("Justin Maurer <justin@openprojects.net>");
        authors[1] = _("John Ellis <johne@bellatlantic.net>");
	authors[2] = NULL;
	about = gnome_about_new ( _("GTicker"), version,_("(C) 1998"), authors,
_("Ticker for the GNOME Project\n"), NULL);
	gtk_widget_show (about);
}

static void show_article_window(AppletWidget *widget, gpointer data)
{
        AppData *ad = data;
        if (ad->article_window)
                {
	                gdk_window_raise(ad->article_window->window);
	                return;
	        }

	ad->article_window = gnome_dialog_new(_("GTicker Article List"),
				                  GNOME_STOCK_BUTTON_CLOSE, 
						  NULL);
        gtk_widget_set_usize(ad->article_window, 400, 350);
        ad->article_list = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (ad->article_list),
	                                GTK_POLICY_AUTOMATIC, 
					GTK_POLICY_AUTOMATIC);
        gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(ad->article_window)->vbox),
        ad->article_list,TRUE,TRUE,0);
        gtk_widget_show(ad->article_list);
        gtk_object_set_user_data(GTK_OBJECT(ad->article_list), NULL);
        gnome_dialog_set_close (GNOME_DIALOG(ad->article_window), TRUE);
        gtk_signal_connect(GTK_OBJECT(ad->article_window), "destroy",
                          (GtkSignalFunc) destroy_article_window, ad);
        populate_article_window(ad);
        gtk_widget_show(ad->article_window);
}

static void refresh_cb(AppletWidget *widget, gpointer data)
{
        AppData *ad = data;
        if (ad->startup_timeout_id > 0) return;
        ad->startup_timeout_id = gtk_timeout_add(5000, startup_delay_cb, ad);
}

static void destroy_article_window(GtkWidget *w, gpointer data)
{
       AppData *ad = data;
       ad->article_window = NULL;
}

static int startup_delay_cb(gpointer data)
{
        AppData *ad = data;
        get_current_headlines(ad);
        ad->startup_timeout_id = 0;
        return FALSE;
	/* return false to stop this timeout callback, needed only once */
}

static void populate_article_window(AppData *ad)
{
        GtkWidget *vbox;
        GtkWidget *hbox;
        GtkWidget *label;
        GtkWidget *button;
        GtkWidget *pixmap;
        GList *list;
        gint added = FALSE;
 
	if (!ad->article_window) return;

	vbox = gtk_object_get_user_data(GTK_OBJECT(ad->article_list));
        if (vbox) gtk_widget_destroy(vbox);
        vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(ad->article_list), vbox);
	gtk_widget_show(vbox);
											gtk_object_set_user_data(GTK_OBJECT(ad->article_list), vbox);
        list = ad->text;

        while(list)
                {
	              InfoData *id = list->data;
	                if (id && id->data && id->show_count == 0)
                          {
                            if (added)
                               {
                                 GtkWidget *sep = gtk_hseparator_new();
                                 gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, 
						 FALSE, 0);
				 gtk_widget_show(sep);
				}
	                    hbox = gtk_hbox_new(FALSE, 5);
	                    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, 
					    FALSE, 0);
			    gtk_widget_show(hbox);
                            label = gtk_label_new(id->text);
                            gtk_label_set_justify (GTK_LABEL(label), 
					    GTK_JUSTIFY_LEFT);
			    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, 
					    FALSE, 5);
			    gtk_widget_show(label);
                            button = gtk_button_new();
                            gtk_object_set_user_data(GTK_OBJECT(button), 
					    id->data);
		            gtk_signal_connect(GTK_OBJECT(button), "clicked",
		            (GtkSignalFunc) article_button_cb, ad);
		            gtk_box_pack_end(GTK_BOX(hbox), button, FALSE, 
					    FALSE, 5);
			    gtk_widget_show(button);
                            pixmap = gnome_stock_pixmap_widget_new(ad->
					    article_window, 
					    GNOME_STOCK_PIXMAP_JUMP_TO);
                            gtk_container_add(GTK_CONTAINER(button), pixmap);
	                    gtk_widget_show(pixmap);
                            added = TRUE;
                          }
                list = list->next;
                }
        if (!added)
                {
	               hbox = gtk_hbox_new(FALSE, 5);
	               gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	               gtk_widget_show(hbox);
	               label = gtk_label_new(_("No articles"));
	               gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 
				       0);
	               gtk_widget_show(label);
										                }
}

static void article_button_cb(GtkWidget *button, gpointer data)
{
        AppData *ad = data;
        gchar *url;
        url = gtk_object_get_user_data(GTK_OBJECT(button));
        launch_url(ad, url);
}

static void launch_url(AppData *ad, gchar *url)
{
        gchar *command;
        char *argv[8];
        int status;
        if (ad->new_browser_window)
	        command = g_strconcat("openURL(", url, ",new-window)", NULL);
	else
	        command = g_strconcat("openURL(", url, ")", NULL);
        argv[0] = "netscape";
        argv[1] = "-remote";
        argv[2] = command;
        argv[3] = NULL;

	/* based on the web control applet */
        if(fork() == 0)
		{
	          /* child  */
        	  execvp (argv[0], argv);
		}
        else
                {
	          wait(&status);
	          if(WEXITSTATUS(status) != 0)
	              {
	                /* command didn't work */
			argv[0] = "netscape";
		        argv[1] = url;
		        argv[2] = NULL;
		        if (gnome_execute_async (NULL, 2, argv) < 0)
		   	     {
				     printf("failed to start browser\n");
			     }
			}
		}
	g_free(command);
}

static gchar *check_for_dir(char *d)
{
        if (!g_file_exists(d))
 	       {
		       g_print(_("creating user directory: %s\n"), d);
		       if (mkdir( d, 0755 ) < 0)
		         {
		            g_print(_("unable to create user directory: 
					    %s\n"), d);
			    g_free(d);
			    d = NULL;
			 }
										                }
	return d;
}

