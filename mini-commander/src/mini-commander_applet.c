/*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <om@linuxhq.com>
 *
 * Author: Oliver Maruhn <om@linuxhq.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string.h>
#include <config.h>
#include <gnome.h>
#include <applet-widget.h>

#include "mini-commander_applet.h"
#include "preferences.h"
#include "command_line.h"
#include "message.h"
#include "exec.h"
#include "about.h"
#include "help.h"

GtkWidget *applet;

static int appletDestroy_signal(GtkWidget *widget, gpointer data);
static int appletDetached_signal(GtkWidget *widget, gpointer data);
static int appletAttached_signal(GtkWidget *widget, gpointer data);


int 
appletDestroy_signal(GtkWidget *widget, gpointer data)
{
    /* applet will be destroyed; save settings now */
    saveSession();
    /* go on */
    return FALSE;  
}

int 
appletDetached_signal(GtkWidget *widget, gpointer data)
{
    /* applet has been detached; make it smaller */

    gtk_widget_set_usize(GTK_WIDGET(applet),
			 20,
			 prop.normalSizeY);
  
    /* go on */
    return FALSE;  
}

int 
appletAttached_signal(GtkWidget *widget, gpointer data)
{
    /* applet has been detached; restore original size */

    gtk_widget_set_usize(GTK_WIDGET(applet),
			 prop.normalSizeX,
			 prop.normalSizeY);
  
    /* go on */
    return FALSE;  
}

static void
drawApplet(void)
{
    /* not needed yet */
}

int 
main(int argc, char **argv)
{
    GtkWidget *vbox;
    GtkWidget *hbox, *hboxButtons;
    GtkWidget *button;
    GtkWidget *frame;
    GtkWidget *frame2;
    GtkWidget *handle;

    GtkStyle *style;
    GdkColor color;
    
    /* install signal handler */
    initExecSignalHandler();
    
    /* initialize the i18n stuff */
    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);
    
    /* intialize, this will basically set up the applet, corba and
       call gnome_init */
    applet_widget_init("mini-commander_applet", VERSION, argc, argv, NULL, 0,
		       NULL);
    
    /* create a new applet_widget */
    
    /*
      style = malloc(sizeof(GtkStyle));
      style->bg_pixmap[GTK_STATE_NORMAL] = bgPixmap;
      gtk_widget_push_style (style);
    */
    applet = applet_widget_new("mini-commander_applet");
    /*
      gtk_widget_pop_style ();
    */
    
    /* in the rare case that the communication with the panel
       failed, error out */
    if (!applet)
	{
	    g_error("Can't create applet!\n");
	    exit(1);
	}
    
    loadSession();
    
    gtk_signal_connect(GTK_OBJECT(applet), "destroy",
		       (GtkSignalFunc) appletDestroy_signal,
		       NULL); 
    
    /* bind the session save signal */
    gtk_signal_connect(GTK_OBJECT(applet),
		       "save_session",
		       GTK_SIGNAL_FUNC(saveSession_signal),
		       NULL);
    
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_border_width(GTK_CONTAINER(vbox), 0);
    
    /* add command line; position: top */
    initCommandEntry();
    gtk_box_pack_start(GTK_BOX(vbox), entryCommand, FALSE, FALSE, 0);

    /* hbox for message label and buttons */
    hbox = gtk_hbox_new(FALSE, 0);
    
    /* add message label */
    initMessageLabel();
    /* do not center text but put it to bottom instead */
    gtk_misc_set_alignment(GTK_MISC(labelMessage), 0.0, 1.0);
    gtk_box_pack_start(GTK_BOX(hbox), labelMessage, TRUE, TRUE, 0);

    hboxButtons = gtk_hbox_new(TRUE, 0);

    /* add file-browser button */
    /* FIXME: icon needed */
    button = gtk_button_new();
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(showFileBrowser_signal),
		       NULL);
    gtk_widget_set_usize(GTK_WIDGET(button), 12, 10);
    gtk_box_pack_start(GTK_BOX(hboxButtons), button, TRUE, TRUE, 0);

    /* add history button */
    /* FIXME: icon needed */
    button = gtk_button_new();
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(showHistory_signal),
		       NULL);
    gtk_widget_set_usize(GTK_WIDGET(button), 12, 10);
    gtk_box_pack_end(GTK_BOX(hboxButtons), button, TRUE, TRUE, 0);

    /* add buttons into frame */
    frame = gtk_frame_new(NULL);
    gtk_container_border_width(GTK_CONTAINER(frame), 1);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(frame), hboxButtons);
    gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);

    /* put message label and history/file-browser button into vbox */
    gtk_box_pack_end(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);


    /* inner frame */
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
    gtk_container_add(GTK_CONTAINER(frame), vbox);

    /* add a handle box to allow moving away this appplet from the
       panel */
    handle = gtk_handle_box_new();
    gtk_signal_connect(GTK_OBJECT(handle), "child_detached",
		       GTK_SIGNAL_FUNC(appletDetached_signal),
		       NULL);
    gtk_signal_connect(GTK_OBJECT(handle), "child_attached",
		       GTK_SIGNAL_FUNC(appletAttached_signal),
		       NULL);
    gtk_container_add(GTK_CONTAINER(handle), frame);
    
    /* outer frame */
    frame2 = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame2), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(frame2), handle);

    applet_widget_set_tooltip(APPLET_WIDGET(applet),  _("Mini-Commander"));
    applet_widget_add (APPLET_WIDGET (applet), frame2);
    gtk_widget_set_usize(GTK_WIDGET(applet), prop.normalSizeX, prop.normalSizeY);
    
    gtk_widget_show_all(applet);
    
    /* add items to applet menu */
    applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					  "properties",
					  GNOME_STOCK_MENU_PROP,
					  _("Properties..."),
					  propertiesBox,
					  NULL);
    
    applet_widget_register_callback(APPLET_WIDGET(applet),
				    "help",
				    _("Help"),
				    showHelp,
				    NULL);

    applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					  "about",
					  GNOME_STOCK_MENU_ABOUT,
					  _("About"),
					  aboutBox,
					  NULL);
    
    showMessage((gchar *) _("ready...")); 
 
    /* special corba main loop */
    applet_widget_gtk_main ();
    
    return 0;
}
