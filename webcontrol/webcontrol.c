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

#include <config.h>
#include "webcontrol.h"
#include "session.h"


WebControl WC = 
{
	NULL,
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
				 _("An applet to launch URLs in a web browser, "
				   "search using various web sites and "
				   "have a ton of fun!"),
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


/* builds an array for the command to lauch the browser 
 * pass in an empty array of gchar pointers to put commands in
 * returns the number of items in the array */
static void
build_browser_command (GPtrArray *command, gchar *url)
{
	gchar *temp;

	temp = B_LIST_COMMAND (props.curr_browser);
	g_ptr_array_add (command, temp);

	if (props.newwindow)
	{
		if (B_LIST_NEWWIN (props.curr_browser))
		{
			temp = B_LIST_NEWWIN (props.curr_browser);
			g_ptr_array_add (command, temp);
		}
	}
	else
	{	       
		if (B_LIST_NO_NEWWIN (props.curr_browser))
		{
			temp = B_LIST_NO_NEWWIN (props.curr_browser);
			g_ptr_array_add (command, temp);
		}
	}

	g_ptr_array_add (command, url);

	return;
}


/* function to open a URL in the currently selected browser */
static void
open_url (gchar *url)
{
	GPtrArray *command;
	gint status;

	command = g_ptr_array_new ();
	build_browser_command (command, url);

	status = gnome_execute_async (NULL, command->len, (char **) command->pdata);
	
	if (status == -1)
		gnome_url_show (url);
	
	if (command)
		g_ptr_array_free (command, TRUE);

	return;
}


/* function for adding the url to the history. adapted from
 * the code for a similar function in Gnapster */
static void
add_url_to_history (const gchar *url)
{
	gchar *str_t = NULL;
	GList *ptr;
	GtkWidget *list, *child, 
		  *lbl, *entry, *combo;

	if (!url || !(*url))
		return;       

	combo = WC.input;
	entry = GTK_COMBO (combo)->entry;

	/* make sure it's not a duplicate, if it is return */
	for (ptr = GTK_LIST (GTK_COMBO (combo)->list)->children; 
	     ptr; ptr = ptr->next) 
	{
		child = ptr->data;
		if (!child)
			continue;
      
		lbl = GTK_BIN (child)->child;
      
		str_t = GTK_LABEL (lbl)->label;
      
		if (str_t && !strcmp (str_t, url))
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

	gtk_entry_set_text (GTK_ENTRY (entry), str_t);
   
	g_free (str_t); 

	return;
}


static void 
goto_cb (GtkWidget *entry, GtkWidget *check)
{
	gchar *url;

	url = gtk_editable_get_chars (GTK_EDITABLE (GTK_COMBO (WC.input)->entry), 
				      0, -1);	
        
	add_url_to_history (url);

	if (props.use_mime)
		gnome_url_show (url);
	else
		open_url (url);
	
	g_free (url);

	return;
}


void
clear_url_history (GtkButton *button, gpointer combo)
{
	gtk_list_clear_items (GTK_LIST (GTK_COMBO (combo)->list), 0, -1);
	
	return;
}


void 
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
		gtk_widget_show (WC.label);
	}

	/* URL combo box */
	if (WC.input == NULL)
	{
		WC.input = gtk_combo_new (); 
		gtk_combo_disable_activate (GTK_COMBO (WC.input));
		gtk_widget_show (WC.input);
		gtk_signal_connect (GTK_OBJECT (GTK_COMBO (WC.input)->entry), 
				    "activate", GTK_SIGNAL_FUNC (goto_cb),
				    (gpointer) props.newwindow);	     
	}

	gtk_widget_set_usize (GTK_WIDGET (GTK_COMBO (WC.input)->entry), 
			      props.width, 0);

	/* "GO" Button */
	if (WC.go == NULL)
	{
		WC.go = gtk_button_new_with_label (_(" GO "));
		gtk_widget_show (WC.go);
		gtk_signal_connect (GTK_OBJECT (WC.go), 
				    "clicked", GTK_SIGNAL_FUNC (goto_cb),
				    (gpointer) props.newwindow);	
		gtk_box_pack_end (GTK_BOX (topbox), WC.go, FALSE, FALSE, 2);
	}
	if (props.show_go)
	{
		gtk_widget_show (WC.go);
	}
	else
	{
		gtk_widget_hide (WC.go);
	}
	
	/* Clear button */
	if (WC.clear == NULL)
	{
		WC.clear = gtk_button_new_with_label (_(" Clear "));
		gtk_signal_connect (GTK_OBJECT (WC.clear), "clicked",
				    GTK_SIGNAL_FUNC (clear_cb),
				    WC.input);		
		if (props.clear_top)
		{
			gtk_box_pack_end (GTK_BOX (topbox), WC.clear, FALSE, 
					    FALSE, 2);
		}
		else
		{
			gtk_box_pack_start (GTK_BOX (bottombox), WC.clear, FALSE, 
					    FALSE, 2);
		}
	}
	       
	if (props.show_clear)
	{
		gtk_widget_show (WC.clear);	       
	}
	else
	{
		gtk_widget_hide (WC.clear);
	}
	
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

	wc_load_session ();

	draw_applet ();

	wc_load_history (GTK_COMBO (WC.input));

	/* bind the session save signal */
	gtk_signal_connect (GTK_OBJECT (WC.applet), "save_session",
			    GTK_SIGNAL_FUNC (wc_save_session),
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
