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
};

static WebControl WC = {
	{FALSE, TRUE, TRUE},
	{-1, -1, -1},
	NULL,
	NULL,
	NULL
};

/*the most important dialog in the whole application*/
/* shamelessly jacked from the fish applet.   --Garrett */
void
about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	gchar *authors[2];
	const gchar * author_format = _("%s the Fish");
	
	authors[0] = _("Garrett Smith <gsmith@serv.net>");
	authors[1] = NULL;

	about = gnome_about_new (_("The Web Browser Controller"), "0.1",
			"(C) 1998 the Free Software Foundation",
			authors,
			_("This applet currently sends getURL commands "
			  "to netscape throught the -remote "
			  "interface.  Hopefully later more webrowsers "
			  "will be supported."),
			NULL);
	gtk_widget_show (about);

	return;
}

void check_box_toggled(GtkWidget *check, int *data)
{
	*data = GTK_TOGGLE_BUTTON(check)->active;
}

void goto_callback(GtkWidget *entry, GtkWidget *check)
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
}

void create_widget() {
	GtkWidget *input;
	GtkWidget *hbox, *vbox;
	
	/* create the widget we are going to put on the applet */
	WC.label = gtk_label_new(_("Url:"));
	if(WC.properties.showurl)
		gtk_widget_show(WC.label);
	
	input = gtk_entry_new();
	gtk_widget_show(input);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	
	gtk_box_pack_start(GTK_BOX(hbox), WC.label, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox), input, FALSE, FALSE, 0);
	
	WC.check = gtk_check_button_new_with_label (_("Launch new window"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(WC.check), WC.properties.newwindow);
	if(WC.properties.showcheck)
		gtk_widget_show(WC.check);
	
	gtk_signal_connect(GTK_OBJECT(WC.check),"toggled",
			   GTK_SIGNAL_FUNC(check_box_toggled),
			   &WC.properties.newwindow);
	
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), WC.check, FALSE, FALSE, 0);
	
	gtk_signal_connect(GTK_OBJECT(input), "activate",
                           GTK_SIGNAL_FUNC(goto_callback),
                           WC.check);
	
	/* add the widget to the applet-widget, and thereby actually
	   putting it "onto" the panel */
	applet_widget_add (APPLET_WIDGET (WC.applet), vbox);
}

static void
apply_cb(GnomePropertyBox * pb, gint page, gpointer data)
{
	gchar * new_name;

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
}

void
properties_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget * pb;
	GtkWidget * vbox;
	GtkWidget *urlcheck, *launchcheck;

	pb = gnome_property_box_new();

	gtk_window_set_title(GTK_WINDOW(pb), _("WebControl Properties"));

	vbox = gtk_vbox_new(GNOME_PAD, FALSE);
	gtk_container_border_width(GTK_CONTAINER(vbox), GNOME_PAD);

	urlcheck = gtk_check_button_new_with_label (_("Display URL label"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(urlcheck), WC.properties.showurl);
	gtk_signal_connect(GTK_OBJECT(urlcheck),"toggled",
			   GTK_SIGNAL_FUNC(check_box_toggled),
			   &WC.tmp_properties.showurl);
	gtk_signal_connect_object(GTK_OBJECT(urlcheck), "toggled",
				  GTK_SIGNAL_FUNC(gnome_property_box_changed),
				  GTK_OBJECT(pb));
	
	launchcheck = gtk_check_button_new_with_label (_("Display \"launch new window\" option"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(launchcheck), WC.properties.showcheck);
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

	gtk_widget_show_all(pb);
}

/* sesion save signal handler*/
static gint
applet_session_save(GtkWidget *w,
		    const char *cfgpath,
		    const char *globcfgpath)
{	
	gnome_config_push_prefix(cfgpath);
	gnome_config_set_bool("newwindow", WC.properties.newwindow);
	gnome_config_set_bool("showurl", WC.properties.showurl);
	gnome_config_set_bool("showcheck", WC.properties.showcheck);
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
}

int
main(int argc, char **argv)
{
	GtkWidget *label;
	GtkWidget *input;
	GtkWidget *hbox, *vbox;
	GtkWidget *check;
		
	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	/* intialize, this will basically set up the applet, corba and
	   call gnome_init */
	applet_widget_init_defaults("webcontrol_applet", NULL, argc, argv, 0,
				    NULL, argv[0]);

	/* create a new applet_widget */
	WC.applet = applet_widget_new();
	/* in the rare case that the communication with the panel
	   failed, error out */
	if (!WC.applet)
		g_error("Can't create applet!\n");
	
	gnome_config_push_prefix(APPLET_WIDGET(WC.applet)->cfgpath);
	WC.properties.newwindow = gnome_config_get_bool("newwindow=false");
	WC.properties.showurl = gnome_config_get_bool("showurl=true");
	WC.properties.showcheck = gnome_config_get_bool("showcheck=true");
	gnome_config_pop_prefix();
	
	create_widget();
	
	/* bind the session save signal */
	gtk_signal_connect(GTK_OBJECT(WC.applet),"session_save",
			   GTK_SIGNAL_FUNC(applet_session_save),
			   NULL);
	
	/* add an item to the applet menu */
	applet_widget_register_callback(APPLET_WIDGET(WC.applet),
					"about",
					_("About..."),
					about_cb,
					NULL);
	
	/* add an item to the applet menu */
	applet_widget_register_callback(APPLET_WIDGET(WC.applet),
					"properties",
					_("Properties..."),
					properties_cb,
					NULL);
	
	/* add the widget to the applet-widget, and thereby actually
	   putting it "onto" the panel */
	gtk_widget_show (WC.applet);

	/* special corba main loop */
	applet_widget_gtk_main ();

	return 0;
}
