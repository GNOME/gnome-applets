/*
 * GNOME web browser control module
 * (C) 1998 The Free Software Foundation
 *
 * based on:
 * GNOME fish module.
 * code snippets from APPLET_WRITING document
 *
 * Author: Garrett Smith
 *
 */

#include <config.h>
#include <gnome.h>
#include <applet-widget.h>
#include <libgnomeui/gnome-window-icon.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct _webcontrol_properties webcontrol_properties;

struct _webcontrol_properties {
	gint newwindow;			/* do we launch a new window */
	gint showurl;			/* do we show the url label in front */
	gint showcheck;			/* do we show the "Launch new window" checkbox */
};

typedef struct _WebControl WebControl;

struct _WebControl {
	webcontrol_properties properties;
	webcontrol_properties tmp_properties;
	GtkWidget *applet;
	GtkWidget *label;
	GtkWidget *check;
	GtkWidget *clear;
};

static WebControl WC = {
	{FALSE, TRUE, TRUE},
	{-1, -1, -1},
	NULL,
	NULL,
	NULL,
	NULL
};

/*the most important dialog in the whole application*/
/* shamelessly jacked from the fish applet.   --Garrett */
static void
about_cb (AppletWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	static const gchar *authors[2] =
	{"Garrett Smith <gsmith@serv.net>", NULL};

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}
	about = gnome_about_new (_("The Web Browser Controller"),
			VERSION,
			"(C) 1998 the Free Software Foundation",
			authors,
			_("This applet currently sends getURL commands "
			  "to netscape throught the -remote "
			  "interface.  Hopefully later more webrowsers "
			  "will be supported."),
			NULL);
	gtk_signal_connect( GTK_OBJECT(about), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about );
	gtk_widget_show (about);

	return;
	widget = NULL;
	data = NULL;
}


static void check_box_toggled(GtkWidget *check, int *data)
{
	*data = GTK_TOGGLE_BUTTON(check)->active;
}

static void clear_callback(GtkWidget *button, GtkWidget *input)
{
    gtk_entry_set_text(GTK_ENTRY(input), "");
    return;
    button = NULL;
}

static void goto_callback(GtkWidget *entry, GtkWidget *check)
{
	gchar *url;
	gchar *command;
	int status;
	
        url = gtk_entry_get_text(GTK_ENTRY(entry));
        
        if(WC.properties.newwindow) {
        	command = malloc(sizeof(gchar) * (strlen(url) + strlen("openURL(, new-window)") + 1));
        	sprintf(command, "openURL(%s, new-window)", url);
        } else {
        	command = malloc(sizeof(gchar) * (strlen(url) + strlen("openURL()") + 1));
        	sprintf(command, "openURL(%s)", url);
        }
                
        if(fork() == 0) {
        	/* child  */
        	execlp("netscape", "netscape", "-remote", command, NULL);
        } else {
        	wait(&status);
        	if(WEXITSTATUS(status) != 0) {  /* command didn't work */
        		char *argv[3];
        		argv[0] = "netscape";
        		argv[1] = url;
        		argv[2] = NULL;
        		gnome_execute_async (NULL, 2, argv);
        	}
	}
	return;
	check = NULL;
}

static void create_widget(void)
{
	GtkWidget *input;
	GtkWidget *topbox, *bottombox, *vbox;
	
	/* create the widget we are going to put on the applet */
	WC.label = gtk_label_new(_("Url:"));
	if(WC.properties.showurl)
		gtk_widget_show(WC.label);
	WC.clear = gtk_button_new_with_label(_("Clear"));
	gtk_widget_show(WC.clear);
	
	input = gtk_entry_new();
	gtk_widget_show(input);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	
	topbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(topbox);
	bottombox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(bottombox);
	
	gtk_box_pack_start(GTK_BOX(topbox), WC.label, FALSE, FALSE, 3);
	
	WC.check = gtk_check_button_new_with_label (_("Launch new window"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(WC.check), WC.properties.newwindow);
	if(WC.properties.showcheck)
		gtk_widget_show(WC.check);
	
	gtk_box_pack_start(GTK_BOX(bottombox), WC.check, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(bottombox), WC.clear, FALSE, FALSE, 3);
	gtk_signal_connect(GTK_OBJECT(WC.check),"toggled",
			   GTK_SIGNAL_FUNC(check_box_toggled),
			   &WC.properties.newwindow);
	
	gtk_box_pack_start(GTK_BOX(vbox), topbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), bottombox, FALSE, FALSE, 0);
	
	gtk_signal_connect(GTK_OBJECT(input), "activate",
                           GTK_SIGNAL_FUNC(goto_callback),
                           WC.check);

	gtk_signal_connect(GTK_OBJECT(WC.clear), "clicked",
                           GTK_SIGNAL_FUNC(clear_callback),
                           input);
	
	/* add the widget to the applet-widget, and thereby actually
	   putting it "onto" the panel */
	applet_widget_add (APPLET_WIDGET (WC.applet), vbox);

	/*we want to allow pasting into the input box so we pack it after
	  applet_widdget_add has bound the middle button*/
	gtk_box_pack_start(GTK_BOX(topbox), input, FALSE, FALSE, 0);
}

static void
apply_cb(GnomePropertyBox * pb, gint page, gpointer data)
{
	/* gchar * new_name; */

	if (page != -1) return; /* Only honor global apply */
	
	if(WC.tmp_properties.showurl != -1) {
		WC.properties.showurl = WC.tmp_properties.showurl;
		if(WC.properties.showurl == FALSE)
			gtk_widget_hide(WC.label);
		else
			gtk_widget_show(WC.label);
		WC.tmp_properties.showurl = -1;
	}
	
	if(WC.tmp_properties.showcheck != -1) {
		WC.properties.showcheck = WC.tmp_properties.showcheck;
		if(WC.properties.showcheck == FALSE)
			gtk_widget_hide(WC.check);
		else
			gtk_widget_show(WC.check);
		WC.tmp_properties.showcheck = -1;
	}
	
	gtk_widget_queue_resize(WC.applet);
	return;
	pb = NULL;
	data = NULL;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { 
		"webcontrol_applet", "index.html#WEBCONTROL-APPLET-PREFS"
	};
	gnome_help_display (NULL, &help_entry);
}

static void
properties_cb (AppletWidget *widget, gpointer data)
{
	static GtkWidget * pb = NULL;
	GtkWidget * vbox;
	GtkWidget *urlcheck, *launchcheck;

	/* Stop the property box from being opened multiple times */
	if (pb != NULL)
	{
		gdk_window_show( GTK_WIDGET(pb)->window );
		gdk_window_raise( GTK_WIDGET(pb)->window );
		return;
	}
	pb = gnome_property_box_new();

	gtk_window_set_title(GTK_WINDOW(pb), _("WebControl Properties"));

	vbox = gtk_vbox_new(GNOME_PAD, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD);

	urlcheck = gtk_check_button_new_with_label (_("Display URL label"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(urlcheck), WC.properties.showurl);
	gtk_signal_connect(GTK_OBJECT(urlcheck),"toggled",
			   GTK_SIGNAL_FUNC(check_box_toggled),
			   &WC.tmp_properties.showurl);
	gtk_signal_connect_object(GTK_OBJECT(urlcheck), "toggled",
				  GTK_SIGNAL_FUNC(gnome_property_box_changed),
				  GTK_OBJECT(pb));
	
	launchcheck = gtk_check_button_new_with_label (_("Display \"launch new window\" option"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(launchcheck), WC.properties.showcheck);
	gtk_signal_connect(GTK_OBJECT(launchcheck),"toggled",
			   GTK_SIGNAL_FUNC(check_box_toggled),
			   &WC.tmp_properties.showcheck);
	gtk_signal_connect_object(GTK_OBJECT(launchcheck), "toggled",
				  GTK_SIGNAL_FUNC(gnome_property_box_changed),
				  GTK_OBJECT(pb));
	
	gtk_box_pack_start(GTK_BOX(vbox), urlcheck, FALSE, FALSE, GNOME_PAD);
	gtk_box_pack_start(GTK_BOX(vbox), launchcheck, TRUE, TRUE, GNOME_PAD);

	gnome_property_box_append_page(GNOME_PROPERTY_BOX(pb), vbox,
				       gtk_label_new(_("Look")));

	gtk_signal_connect(GTK_OBJECT(pb), "apply", GTK_SIGNAL_FUNC(apply_cb),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(pb), "destroy",
			  gtk_widget_destroyed,
			  (gpointer) &pb);
	gtk_signal_connect(GTK_OBJECT(pb), "help",
			   phelp_cb, NULL);
	gtk_widget_show_all(pb);
	return;
	widget = NULL;
	data = NULL;
}

static void 
show_help_cb (AppletWidget *applet, gpointer data)
{
	static GnomeHelpMenuEntry help_entry = { NULL, "index.html"};

	help_entry.name = gnome_app_id;
    
	gnome_help_display (NULL, &help_entry);
}

/* sesion save signal handler*/
static gint
applet_save_session(GtkWidget *w,
		    const char *privcfgpath,
		    const char *globcfgpath)
{	
	gnome_config_push_prefix(privcfgpath);
	gnome_config_set_bool("web/newwindow", WC.properties.newwindow);
	gnome_config_set_bool("web/showurl", WC.properties.showurl);
	gnome_config_set_bool("web/showcheck", WC.properties.showcheck);
	gnome_config_pop_prefix();

	gnome_config_sync();
	/* you need to use the drop_all here since we're all writing to
	   one file, without it, things might not work too well */
	gnome_config_drop_all();
		
	/* make sure you return FALSE, otherwise your applet might not
	   work compeltely, there are very few circumstances where you
	   want to return TRUE. This behaves similiar to GTK events, in
	   that if you return FALSE it means that you haven't done
	   everything yourself, meaning you want the panel to save your
	   other state such as the panel you are on, position,
	   parameter, etc ... */
	return FALSE;
	w = NULL;
	globcfgpath = NULL;
}

int
main(int argc, char **argv)
{
       /*
        * GtkWidget *label;
	* GtkWidget *input;
	* GtkWidget *hbox, *vbox;
	* GtkWidget *check;
	*/
	
	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	/* intialize, this will basically set up the applet, corba and
	   call gnome_init */
	applet_widget_init("webcontrol_applet", VERSION, argc, argv,
				    NULL, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-mnemonic.png");

	/* create a new applet_widget */
	WC.applet = applet_widget_new("webcontrol_applet");
	/* in the rare case that the communication with the panel
	   failed, error out */
	if (!WC.applet)
		g_error("Can't create applet!\n");
	
	gnome_config_push_prefix(APPLET_WIDGET(WC.applet)->privcfgpath);
	WC.properties.newwindow = gnome_config_get_bool("web/newwindow=false");
	WC.properties.showurl = gnome_config_get_bool("web/showurl=true");
	WC.properties.showcheck = gnome_config_get_bool("web/showcheck=true");
	gnome_config_pop_prefix();
	
	create_widget();
	
	/* bind the session save signal */
	gtk_signal_connect(GTK_OBJECT(WC.applet),"save_session",
			   GTK_SIGNAL_FUNC(applet_save_session),
			   NULL);

	/* add an item to the applet menu */
	applet_widget_register_stock_callback(APPLET_WIDGET(WC.applet),
					      "properties",
					      GNOME_STOCK_MENU_PROP,
					      _("Properties..."),
					      properties_cb,
					      NULL);

	applet_widget_register_stock_callback(APPLET_WIDGET(WC.applet),
					      "help",
					      GNOME_STOCK_PIXMAP_HELP,
					      _("Help"),
					      show_help_cb,
					      NULL);
	
	applet_widget_register_stock_callback(APPLET_WIDGET(WC.applet),
					      "about",
					      GNOME_STOCK_MENU_ABOUT,
					      _("About..."),
					      about_cb,
					      NULL);
	
	/* add the widget to the applet-widget, and thereby actually
	   putting it "onto" the panel */
	gtk_widget_show (WC.applet);

	/* special corba main loop */
	applet_widget_gtk_main ();

	return 0;
}
