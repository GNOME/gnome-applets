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

gint newwindow = TRUE;

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

/* sesion save signal handler*/
static gint
applet_session_save(GtkWidget *w,
		    const char *cfgpath,
		    const char *globcfgpath)
{
	gnome_config_push_prefix(cfgpath);
	gnome_config_set_bool("newwindow", newwindow);
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

void check_box_toggled(GtkWidget *check, void *vd)
{
	newwindow = GTK_TOGGLE_BUTTON(check)->active;
}

void goto_callback(GtkWidget *entry, GtkWidget *check)
{
	gchar *url;
	gchar *command;
	
        url = gtk_entry_get_text(GTK_ENTRY(entry));
        
        if(newwindow) {
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
        	wait();
        }
}

int
main(int argc, char **argv)
{
	GtkWidget *applet;
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
	applet = applet_widget_new();
	/* in the rare case that the communication with the panel
	   failed, error out */
	if (!applet)
		g_error("Can't create applet!\n");
	
	gnome_config_push_prefix(APPLET_WIDGET(applet)->cfgpath);
	newwindow = gnome_config_get_bool("newwindow=false");
	gnome_config_pop_prefix();
	
	/* create the widget we are going to put on the applet */
	label = gtk_label_new(_("Url:"));
	gtk_widget_show(label);
	
	input = gtk_entry_new();
	gtk_widget_show(input);
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(hbox);
	
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox), input, FALSE, FALSE, 0);
	
	check = gtk_check_button_new_with_label (_("Launch new window"));
	gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(check), newwindow);
	gtk_widget_show(check);
	
	gtk_signal_connect(GTK_OBJECT(check),"toggled",
			   GTK_SIGNAL_FUNC(check_box_toggled),
			   NULL);
	
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), check, FALSE, FALSE, 0);
	
	gtk_signal_connect(GTK_OBJECT(input), "activate",
                           GTK_SIGNAL_FUNC(goto_callback),
                           check);
	
	/* bind the session save signal */
	gtk_signal_connect(GTK_OBJECT(applet),"session_save",
			   GTK_SIGNAL_FUNC(applet_session_save),
			   NULL);
	
	/* add an item to the applet menu */
	applet_widget_register_callback(APPLET_WIDGET(applet),
					"about",
					_("About..."),
					about_cb,
					NULL);
	
	/* add the widget to the applet-widget, and thereby actually
	   putting it "onto" the panel */
	applet_widget_add (APPLET_WIDGET (applet), vbox);
	gtk_widget_show (applet);

	/* special corba main loop */
	applet_widget_gtk_main ();

	return 0;
}
