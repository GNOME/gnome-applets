/**************************************************
** New SlashApp Code
** Justin Maurer, justin@slashdot.org
** Stolen from old code by many others, See AUTHORS
**************************************************/

#include "slashapp.h"
#include <ghttp.h>
#include "slashsplash.xpm"
#include "gnotices.xpm"
#include <errno.h>
#include <ctype.h>

int refresh_time = 1800000;	/* frequency of updates (in seconds) */

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data);
#ifdef HAVE_PANEL_PIXEL_SIZE
static void applet_change_pixel_size(GtkWidget *w, int size, gpointer data);
#endif

int main(int argc, char *argv[])
{
	GtkWidget *applet;

	bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE);

	applet_widget_init("slash_applet", VERSION, argc, argv, NULL, 0, NULL);

	applet = applet_widget_new("slash_applet");

	if(!applet)	/* a little error checking...*/
		g_error("Can't create applet!\n");

	create_new_app(applet);

	applet_widget_gtk_main();
	return 0;
}

static void
applet_shown (GtkWidget *widget, AppData *ad)
{
	resized_app_display (ad, TRUE);
}

AppData *create_new_app(GtkWidget *applet)
{
	AppData *ad;
	GtkWidget *icon;
	ad = g_new0(AppData, 1);

	ad->applet = applet;
	ad->article_window = NULL;
	ad->slashapp_dir = check_for_dir(gnome_util_home_file("slashapp"));
	if(!ad->slashapp_dir)
		g_error("Can't create slashapp dir in .gnome/\n");

	ad->orient = ORIENT_UP;
	ad->sizehint = 48;
	ad->width_hint = 200;

	ad->win_width = 200;
	ad->win_height = 48;
	ad->follow_hint_width = FALSE;
	ad->follow_hint_height = TRUE;
	ad->user_width = 200;
	ad->user_height = 48;
	ad->draw_area = NULL;

	/* from tick-a-stat */
        gtk_signal_connect(GTK_OBJECT(ad->applet),"change_orient",
                GTK_SIGNAL_FUNC(applet_change_orient), ad);
#ifdef HAVE_PANEL_PIXEL_SIZE
        gtk_signal_connect(GTK_OBJECT(ad->applet),"change_pixel_size",
                GTK_SIGNAL_FUNC(applet_change_pixel_size), ad);
#endif

	init_app_display(ad);

	gtk_signal_connect_after (GTK_OBJECT (applet), "show",
				  GTK_SIGNAL_FUNC (applet_shown),
				  ad);

        gtk_widget_set_usize(ad->applet, 10, 10);
	
	/* connect all the signals to handlers */
	gtk_signal_connect(GTK_OBJECT(ad->applet), "destroy",
			GTK_SIGNAL_FUNC(destroy_applet), ad);
	gtk_signal_connect(GTK_OBJECT(ad->applet), "save_session",
			GTK_SIGNAL_FUNC(applet_save_session), ad);
	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet), 
			"properties", GNOME_STOCK_MENU_PROP, 
			_("Properties..."), property_show, ad);
	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet), 
			"about", GNOME_STOCK_MENU_ABOUT, 
			_("About..."), about_cb, NULL);
	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet), 
			"articles", GNOME_STOCK_MENU_BOOK_OPEN, 
			_("Articles"), show_article_window, ad);
	applet_widget_register_stock_callback(APPLET_WIDGET(ad->applet), 
			"refresh", GNOME_STOCK_MENU_REFRESH, 
			_("Refresh"), refresh_cb, ad);

	property_load(APPLET_WIDGET(applet)->privcfgpath, ad);
	gtk_timeout_add(refresh_time, get_current_headlines, ad);

	if (ad->rdf_site == RDFSITE_SLASHDOT)
		icon = gnome_pixmap_new_from_xpm_d(slashsplash_xpm);
	else
		icon = gnome_pixmap_new_from_xpm_d(gnotices_xpm);
	add_info_line_with_pixmap(ad, "", icon, 0, FALSE, 1, 0);
	add_info_line(ad, "SlashApp\n", NULL, 0, TRUE, 1, 0);
	add_info_line(ad, _("Loading headlines..."), NULL, 0, FALSE, 1, 20);
	
	gtk_widget_show(ad->applet);
	ad->startup_timeout_id = gtk_timeout_add(5000, startup_delay_cb, ad);

	return ad;
}

static void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data){
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


gchar *check_for_dir(char *d)
{
	if(!g_file_exists(d)) {
		g_print(_("Creating user directory: %s\n"), d);
		if (mkdir(d, 0755) < 0) {
			g_print(_("Unable to create user directory: %s\n"), d);
			g_free(d);
			d = NULL;
		}	
	}
	return d;
}

void destroy_applet(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;

	if (ad->display_timeout_id > 0)
		gtk_timeout_remove(ad->display_timeout_id);
	ad->display_timeout_id = 0;

	if (ad->headline_timeout_id > 0)
		gtk_timeout_remove(ad->headline_timeout_id);
	ad->headline_timeout_id = 0;

	if (ad->startup_timeout_id > 0)
		gtk_timeout_remove(ad->startup_timeout_id);
	ad->startup_timeout_id = 0;

/* FIXME: workaround is to comment out. if applets ever become in-process 
 * CORBA servers, this will leak memory. for the time being, everything 
 * is fine, though */


	if (ad->display_w != NULL)
		gtk_widget_destroy(ad->display_w);
	ad->display_w = NULL;

	if (ad->disp_buf_w != NULL)
		gtk_widget_destroy(ad->disp_buf_w);
	ad->disp_buf_w = NULL;

	if (ad->background_w != NULL)
		gtk_widget_destroy(ad->background_w);
	ad->background_w = NULL;

	g_free(ad);
	return;
	widget = NULL;
}

gint applet_save_session(GtkWidget *widget, gchar *privcfgpath, 
			 gchar *globpath, gpointer data)
{
	AppData *ad = data;
	property_save(privcfgpath, ad);
	return FALSE;
	widget = NULL;
}

void about_cb(AppletWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	const gchar *authors[5];
	gchar version[32];

	if (about != NULL)
	{
		gtk_widget_show_now (about);
		gdk_window_raise(about->window);
		return;
	}

	g_snprintf (version, sizeof (version),
		    _("%d.%d.%d"), APPLET_VERSION_MAJ, 
		    APPLET_VERSION_MIN, APPLET_VERSION_REV);
	authors[0] = _("Justin Maurer <justin@helixcode.com>");
	authors[1] = _("John Ellis <johne@bellatlantic.net>");
	authors[2] = _("Craig Small <csmall@eye-net.com.au>");
	authors[3] = _("Frederic Devernay <devernay@istar.fr>");
	authors[4] = NULL;

	about = gnome_about_new(_("SlashApp"), version,
			_("(C) 1998-2000"),
			authors,
			_("A stock ticker-like applet\n"),
			NULL);
	gtk_signal_connect( GTK_OBJECT(about), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about );
	gtk_widget_show(about);
	return;
	widget = NULL;
	data = NULL;
}

void show_article_window(AppletWidget *applet, gpointer data)
{
	AppData *ad = data;

	if(ad->article_window) {
		gdk_window_raise(ad->article_window->window);
		return;
	}

	ad->article_window = gnome_dialog_new(_("SlashApp Article List"), 
			GNOME_STOCK_BUTTON_CLOSE, NULL);
	gtk_widget_set_usize(ad->article_window, 400, 350);
	gnome_dialog_set_close(GNOME_DIALOG(ad->article_window), TRUE);
	
	ad->article_list = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ad->article_list),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(ad->article_window)->vbox), 
			ad->article_list, TRUE, TRUE, 0);
	gtk_widget_show(ad->article_list);

	gtk_object_set_user_data(GTK_OBJECT(ad->article_list), NULL);
	gtk_signal_connect(GTK_OBJECT(ad->article_list), "destroy",
			(GtkSignalFunc) destroy_article_window, ad);

	populate_article_window(ad);
	gtk_widget_show(ad->article_window);
	return;
	applet = NULL;
}

void populate_article_window(AppData *ad)
{
	GtkWidget *vbox, *hbox;
	GtkWidget *label, *button;
	GtkWidget *pixmap;
	GList *list;
	gint added = FALSE;


	if(!ad->article_window) return;

        vbox = gtk_object_get_user_data(GTK_OBJECT(ad->article_list));
        if (vbox)
		gtk_widget_destroy(vbox);
        vbox = gtk_vbox_new(FALSE, 5);
	gtk_scrolled_window_add_with_viewport(
			GTK_SCROLLED_WINDOW(ad->article_list), vbox);
	gtk_widget_show(vbox);
	gtk_object_set_user_data(GTK_OBJECT(ad->article_list), vbox);

	list = ad->text;

	while(list) {
		InfoData *id = list->data;

		if(id && id->data && id->show_count == 0) {
				if(added) {
					GtkWidget *sep = gtk_hseparator_new();
					gtk_box_pack_start(GTK_BOX(vbox), sep, 
							FALSE, FALSE, 0);
					gtk_widget_show(sep);
				}
			hbox = gtk_hbox_new(FALSE, 5);
			gtk_box_pack_start(GTK_BOX(vbox), hbox,
					FALSE, FALSE, 0);
			gtk_widget_show(hbox);

			label = gtk_label_new(id->text);
			gtk_label_set_justify(GTK_LABEL(label), 
					GTK_JUSTIFY_LEFT);
			gtk_box_pack_start(GTK_BOX(hbox), label,
					FALSE, FALSE, 5);
			gtk_widget_show(label);

			button = gtk_button_new();
			gtk_object_set_user_data(GTK_OBJECT(button), id->data);
			gtk_signal_connect(GTK_OBJECT(button), "clicked",
					(GtkSignalFunc) article_button_cb, ad);
			gtk_box_pack_end(GTK_BOX(hbox), button, 
					FALSE, FALSE, 5);
			gtk_widget_show(button);

			pixmap = gnome_stock_pixmap_widget_new(
					ad->article_window, 
					GNOME_STOCK_PIXMAP_JUMP_TO);
			gtk_container_add(GTK_CONTAINER(button), pixmap);
			gtk_widget_show(pixmap);

			added = TRUE;
		}

		list = list->next;
	}

	if(!added) {
		hbox = gtk_hbox_new(FALSE, 5);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
		gtk_widget_show(hbox);

		label = gtk_label_new(_("No articles"));
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
		gtk_widget_show(label);
	}
}

void refresh_cb(AppletWidget *widget, gpointer data)
{
	AppData *ad = data;
	GtkWidget *icon;
	
	remove_all_lines(ad);
	ad->info_current = NULL;
	ad->info_next = NULL;
	if (ad->rdf_site == RDFSITE_SLASHDOT)
		icon = gnome_pixmap_new_from_xpm_d(slashsplash_xpm);
	else
		icon = gnome_pixmap_new_from_xpm_d(gnotices_xpm);
        add_info_line_with_pixmap(ad, "", icon, 0, FALSE, 1, 0);
			
	if(ad->startup_timeout_id > 0)
		return;
	ad->startup_timeout_id = gtk_timeout_add(5000, startup_delay_cb, ad);
	return;
	widget = NULL;
}

int startup_delay_cb(gpointer data)
{
	AppData *ad = data;
	get_current_headlines(ad); 
	ad->startup_timeout_id = 0;
	return FALSE;
}

int get_current_headlines(gpointer data)
{
	AppData *ad = data;
	FILE *file;
	gchar *filename;

	filename = g_strconcat(ad->slashapp_dir, "/", "headlines", NULL);
	file = fopen(filename, "w");
	g_free (filename);
	if(file) {
		http_get_to_file(ad->host, ad->port, ad->resource, file, data); 
		fclose(file);
		parse_headlines(data);
	}
	return TRUE;
}

void parse_headlines(gpointer data)
{
	AppData *ad = data;
	xmlDocPtr doc;
	struct stat s;
        gint delay = ad->article_delay / 10 * (1000 / UPDATE_DELAY);
	
	stat(gnome_util_home_file("slashapp/headlines"), &s);
	if (s.st_size == 0) {
		g_warning(_("Unable to parse document\n"));
		add_info_line(ad, _("Can't parse XML. Net connection down?"),
			      NULL, 0, FALSE, FALSE, delay);
		return;
	}
						
	doc=xmlParseFile(gnome_util_home_file("slashapp/headlines"));	

	if (doc==NULL) {
		g_warning(_("Unable to parse document\n"));
		add_info_line(ad, _("Can't parse XML. Net connection down?"), 
				NULL, 0, FALSE, FALSE, delay);
		return;
	}

	tree_walk(doc->root, data); /* the bulk of the work) */
}

int http_get_to_file(gchar *host, gint port, gchar *resource, FILE *file, gpointer data)	
{
	int length = -1;
	ghttp_request *request = NULL;
	gchar s_port[8];
	gchar *body, *uri = NULL;
	gchar *proxy_uri = NULL;
	char *rdf_uri;
	AppData *ad = data;

	g_snprintf(s_port, sizeof (s_port), "%d", port); /* int to (g)char */
	uri = g_strconcat("http://", host, ":", s_port, "/", resource, NULL);
	request = ghttp_request_new();
	
	if (ad->proxy_enabled) {
		proxy_uri = g_strconcat("http://", ad->proxy_host, ":", 
				         ad->proxy_port, NULL);
		ghttp_set_proxy(request, proxy_uri);
		g_free (proxy_uri);
	}
	
	if(!request){
		g_warning(_("Unable to initialize request. Shouldn't happen\n"));
		if(request) ghttp_request_destroy(request);
		g_free(uri);
		return length;
	}
/*	if(proxy && (ghttp_set_proxy(request, proxy) != 0)) {
		g_warning(_("Unable to set proxy.\n"));
                if(request) ghttp_request_destroy(request);
                g_free(uri);
		return length;
	} 
	if(ghttp_set_uri(request, uri) != 0) {
		g_warning(_("Unable to set URI (URL)\n"));
                if(request) ghttp_request_destroy(request);
                g_free(uri);
		return length;
	} */
	
	if (ad->rdf_site == RDFSITE_SLASHDOT)
		rdf_uri = "http://slashdot.org:80/slashdot.rdf";
	else
		rdf_uri = "http://gnotices.gnome.org/gnome-news/rdf";
	ghttp_set_uri(request, rdf_uri);
	
	ghttp_set_header(request, http_hdr_Connection, "close");
	
	if(ghttp_prepare(request) != 0) {
	        if(request) ghttp_request_destroy(request);
	        g_free(uri);
		g_warning(_("Unable to prepare request.\n"));
		return length;
	}
	if(ghttp_process(request) != ghttp_done) {
		g_warning(_("Unable to process request.\n"));
                if(request) ghttp_request_destroy(request);
                g_free(uri);
		return length;
	}

	length = ghttp_get_body_len(request);
	body = ghttp_get_body(request);

	if(body!=NULL)
		fwrite(body, length, 1, file);
	g_free(uri);

	return length;
}

void tree_walk(xmlNodePtr root, gpointer data)
{
	AppData *ad = data;
	InfoData *id; 
	xmlNodePtr walk = root->childs;
	/* xmlNodePtr image = NULL; */
	xmlNodePtr item[16];
	int i=0;
	int items=0, itemcount=0;
	gint delay = ad->article_delay / 10 * (1000 / UPDATE_DELAY);

	while(walk!=NULL) {
		if(g_strcasecmp(walk->name, "item")==0 && items<16) {
			item[i++] = walk;
			itemcount++;
		}
	walk=walk->next;
	}

	remove_all_lines(ad);
	ad->info_current = NULL;
	ad->info_next = NULL;
	for(i=0;i<itemcount;i++) {
		const char *title = layer_find(item[i]->childs, "title", _("No title"));
		const char *url = layer_find(item[i]->childs, "link", _("No url"));
/*		char *time = layer_find(item[i]->childs, "time", _("No time"));
		char *author = layer_find(item[i]->childs, "author", 
				_("No author"));
		char *image = layer_find(item[i]->childs, "iamge", _("No image"));
*/
                id = add_info_line(ad, title, NULL, 0, FALSE, FALSE, delay); 
/*		set_info_signals(id, click_headline_cb, g_free, NULL, NULL, url);
 */

		set_info_click_signal(id, click_headline_cb, g_strdup(url), 
				g_free);

		add_info_line(ad, "", NULL, 0, FALSE, FALSE, delay);
	}
}

void destroy_article_window(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;
	ad->article_window = NULL;
	return;
	widget = NULL;
}

void article_button_cb(GtkWidget *widget, gpointer data)
{
	AppData *ad = data;
	gchar *url = gtk_object_get_user_data(GTK_OBJECT(widget));
	gnome_url_show(url);
	return;
	ad = NULL;
}
			
const char *layer_find(xmlNodePtr node, const char *match, const char *fail)
{
	while(node!=NULL) {
		if(g_strcasecmp(node->name, match) == 0) {
			if(node->childs != NULL && 
			   node->childs->content != NULL) 
				return node->childs->content;
			else
				return fail;
		}
		node = node->next;
	}
	return fail;
}

void click_headline_cb (gpointer data, InfoData *id, AppData *ad)
{
	gchar *url = id->data;
	if(url)
	      gnome_url_show(url);
	return;
	ad = NULL;
}
