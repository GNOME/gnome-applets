/*
 * GNOME web browser control module
 * (C) 2000 The Free Software Foundation
 *
 * based on:
 * GNOME fish module.
 * code snippets from APPLET_WRITING document
 *
 * Authors: Garrett Smith
 *          Rusty Geldmacher
 *
 */

#include "webcontrol.h"

static WebControl WC = 
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

extern webcontrol_properties props;
extern webcontrol_properties tmp_props;

static void
about_cb (AppletWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	static const gchar *authors[3] = {"Garrett Smith <gsmith@serv.net>", 
					  "Rusty Geldmacher <rusty@wpi.edu>",
					  NULL};

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}
		
	about = gnome_about_new (_("The Web Browser Controller"),
				 VERSION,
				 _("(C) 2000 the Free Software Foundation"),
				 authors,
				 _("This applet sends typed in URLs to "
				   "the browser of your choice."),
				 NULL);
						
	gtk_signal_connect (GTK_OBJECT (about), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);

	gtk_widget_show (about);
}


static void 
clear_cb (GtkWidget *button, GtkWidget *input)
{
    gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (WC.input)->entry), "");
}

static void 
goto_cb (GtkWidget *entry, gpointer data)
{
	gchar *url;
	int status;

        url = gtk_editable_get_chars (GTK_EDITABLE (GTK_COMBO (WC.input)->entry), 
				      0, -1);	
        
        if (fork () == 0) {
        	/* child  */
		if (props.newwindow) 
		{
			execlp ("gnome-moz-remote", "gnome-moz-remote", "--newwin", url, NULL);
		} 
		else 
		{
			execlp ("gnome-moz-remote", "gnome-moz-remote", url, NULL);
		}
		g_warning (_("gnome-moz-remote not found or unable to launch it"));
		/* something went wrong, perhaps gnome-moz-remote was not found */
		_exit (1);
        } 
	else 
	{
        	wait (&status);
        	if (WEXITSTATUS (status) != 0)  /* command didn't work, use normal url show */
		{				/* routine */
			gnome_url_show (url);
        	}
	}

	g_free (url);

	return;
}


/* function for adding the url to the history. adapted from
 * the code for a similar function in Gnapster */
static void
add_url_to_history (GtkWidget *entry, GtkWidget *combo)
{
	char *url, 
	     *str_t = NULL;
	GList *ptr;
	GtkWidget *list, *child, *lbl;

	/* get the url from the entry box, if nothing, return */
	url = gtk_entry_get_text (GTK_ENTRY (entry));
	if (!url || !(*url))
		return;

	/* make sure it's not a duplicate, if it is return */
	for (ptr = GTK_LIST (GTK_COMBO (combo)->list)->children; 
	     ptr; ptr = ptr->next) 
	{
		child = ptr->data;
		if (!child)
			continue;
      
		lbl = GTK_BIN (child)->child;
      
		str_t = GTK_LABEL (lbl)->label;
      
		if (!strcmp (str_t, url))
			return;
	}
	
	str_t = g_strdup (url);

	list = gtk_list_item_new_with_label (str_t);
	gtk_widget_show (list);

	gtk_container_add (GTK_CONTAINER (GTK_COMBO (combo)->list), list);

	/* keep the list the required size */
	ptr = GTK_LIST (GTK_COMBO (combo)->list)->children;
	if (g_list_length (ptr) > props.hist_len)
	{
		ptr = g_list_first (ptr);
		gtk_container_remove (GTK_CONTAINER (GTK_COMBO (combo)->list),
				      GTK_WIDGET (ptr->data));		
	}

	gtk_entry_set_text (GTK_ENTRY(entry), str_t);
   
	g_free (str_t); 

	return;
}


extern void 
draw_applet (void)
{
	static GtkWidget *topbox = NULL;
	static GtkWidget *bottombox = NULL;
	static GtkWidget *vbox = NULL;
	static gboolean appletDrawn = FALSE;

	/* create the widget we are going to put on the applet */

	/* if run for first time, create boxes for other widgets */
	if (!vbox)
	{
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_widget_show (vbox);
	}
	if (!topbox)
	{
		topbox = gtk_hbox_new (FALSE, 0);
		gtk_widget_show (topbox);
	}
	if (!bottombox)
	{
		bottombox = gtk_hbox_new (FALSE, 0);
		gtk_widget_show (bottombox);
	}

	/* URL label */
	if (WC.label == NULL)
	{
		WC.label = gtk_label_new (_("URL:"));
		gtk_box_pack_start (GTK_BOX (topbox), WC.label, 
				    FALSE, FALSE, 3);
	}

	if (props.show_url)
	{
		gtk_widget_show (WC.label);
	}
	else
	{
		gtk_widget_hide (WC.label);
	}

	/* URL combo box */
	if (WC.input == NULL)
	{
		WC.input = gtk_combo_new (); 
		gtk_combo_disable_activate (GTK_COMBO (WC.input));
		gtk_widget_show (WC.input);
		gtk_signal_connect (GTK_OBJECT (GTK_COMBO (WC.input)->entry),
				    "activate",
				    GTK_SIGNAL_FUNC (add_url_to_history),
				    GTK_COMBO (WC.input));
		gtk_signal_connect (GTK_OBJECT (GTK_COMBO (WC.input)->entry), 
				    "activate", GTK_SIGNAL_FUNC (goto_cb),
				    NULL);
	}

	gtk_widget_set_usize (GTK_WIDGET (GTK_COMBO (WC.input)->entry), 
			      props.width, 0);


	/* Show new window check box */
	if (WC.check == NULL)
	{
		WC.check = gtk_check_button_new_with_label (_("Launch new window"));
		gtk_signal_connect (GTK_OBJECT (WC.check),"toggled",
				    GTK_SIGNAL_FUNC (check_box_toggled),
				    &props.newwindow);
		gtk_box_pack_start (GTK_BOX (bottombox), WC.check, FALSE, 
				    FALSE, 3);
	}

	if (props.show_check)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (WC.check), 
					      props.newwindow);
		gtk_widget_show (WC.check);
	}	
	else
	{
		gtk_widget_hide (WC.check);
	}

	/* Clear button */
	if (WC.clear == NULL)
	{
		WC.clear = gtk_button_new_with_label (_(" Clear "));
		gtk_signal_connect (GTK_OBJECT (WC.clear), "clicked",
				    GTK_SIGNAL_FUNC (clear_cb),
				    WC.input);		
		gtk_box_pack_start (GTK_BOX (bottombox), WC.clear, FALSE, 
				    FALSE, 3);
	}
	if (props.show_clear)
	{
		gtk_widget_show (WC.clear);	       
	}
	else
	{
		gtk_widget_hide (WC.clear);
	}
	
	/* if this is the first time drawing the applet, put all widgets on 
           the applet together and add it to the panel, then indicate that
           that applet has actually been drawn once before. */
	if (!appletDrawn)
	{
		gtk_box_pack_start (GTK_BOX (vbox), topbox, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (vbox), bottombox, FALSE, FALSE, 0);

		applet_widget_add (APPLET_WIDGET (WC.applet), vbox);
	
		/* add the widget to the applet-widget, and thereby actually
		   putting it "onto" the panel */
		gtk_widget_show (WC.applet);

		/* we want to allow pasting into the input box so we pack it 
		   after applet_widget_add has bound the middle button */
		gtk_box_pack_start (GTK_BOX (topbox), WC.input, FALSE, FALSE, 0);

		appletDrawn = TRUE;
	}	
}


static void 
show_help_cb (AppletWidget *applet, gpointer data)
{
	static GnomeHelpMenuEntry help_entry = { NULL, "index.html"};

	help_entry.name = gnome_app_id;
    
	gnome_help_display (NULL, &help_entry);
}

static gint
applet_save_session (GtkWidget *w,
		     const char *privcfgpath,
		     const char *globcfgpath)
{	
	gnome_config_push_prefix (privcfgpath);
	gnome_config_set_bool ("web/newwindow", props.newwindow);
	gnome_config_set_bool ("web/show_url", props.show_url);
	gnome_config_set_bool ("web/show_check", props.show_check);
	gnome_config_pop_prefix ();

	gnome_config_sync ();
	/* you need to use the drop_all here since we're all writing to
	   one file, without it, things might not work too well */
	gnome_config_drop_all ();
		
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
main (int argc, char **argv)
{
	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	/* intialize, this will basically set up the applet, corba and
	   call gnome_init */
	applet_widget_init ("webcontrol_applet", VERSION, argc, argv,
			    NULL, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-mnemonic.png");

	/* create a new applet_widget */
	WC.applet = applet_widget_new ("webcontrol_applet");
	/* in the rare case that the communication with the panel
	   failed, error out */
	if (!WC.applet)
		g_error ("Can't create applet!\n");
	
	gnome_config_push_prefix (APPLET_WIDGET (WC.applet)->privcfgpath);
	props.newwindow = gnome_config_get_bool ("web/newwindow=false");
	props.show_url = gnome_config_get_bool ("web/show_url=true");
	/* props.show_check = gnome_config_get_bool ("web/show_check=true"); */
	gnome_config_pop_prefix ();
	
	draw_applet ();
	
	/* bind the session save signal */
	gtk_signal_connect (GTK_OBJECT (WC.applet),"save_session",
			    GTK_SIGNAL_FUNC (applet_save_session),
			    NULL);

	/* add an item to the applet menu */
	applet_widget_register_stock_callback (APPLET_WIDGET (WC.applet),
					       "properties",
					       GNOME_STOCK_MENU_PROP,
					       _("Properties..."),
					       properties_box,
					       NULL);

	applet_widget_register_stock_callback (APPLET_WIDGET (WC.applet),
					       "help",
					       GNOME_STOCK_PIXMAP_HELP,
					       _("Help"),
					       show_help_cb,
					       NULL);
	
	applet_widget_register_stock_callback (APPLET_WIDGET (WC.applet),
					       "about",
					       GNOME_STOCK_MENU_ABOUT,
					       _("About..."),
					       about_cb,
					       NULL);
	
	/* special corba main loop */
	applet_widget_gtk_main ();

	return 0;
}
































