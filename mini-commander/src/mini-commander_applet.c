/*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <oliver@maruhn.com>
 *
 * Author: Oliver Maruhn <oliver@maruhn.com>
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
#include "terminal.h"
#include "message.h"
#include "exec.h"
#include "about.h"
#include "help.h"

#include "browser-mini.xpm"
#include "history-mini.xpm"

GtkWidget *applet;

static gint applet_destroy_signal(GtkWidget *widget, gpointer data);
static gint applet_detached_signal(GtkWidget *widget, gpointer data);
static gint applet_attached_signal(GtkWidget *widget, gpointer data);
static gint applet_orient_changed_cb(GtkWidget *widget, gpointer data);
#ifdef HAVE_PANEL_PIXEL_SIZE
static void applet_pixel_size_changed_cb(GtkWidget *widget, int size, gpointer data);
#endif

static gint
applet_destroy_signal(GtkWidget *widget, gpointer data)
{
    /* applet will be destroyed; save settings now */
    save_session();
    /* go on */
    return FALSE;  
    widget = NULL;
    data = NULL;
}

static gint
applet_detached_signal(GtkWidget *widget, gpointer data)
{
    /* applet has been detached; make it smaller */

    gtk_widget_set_usize(GTK_WIDGET(applet),
			 20,
			 prop.normal_size_y);
  
    /* go on */
    return FALSE;  
    widget = NULL;
    data = NULL;
}

static gint
applet_attached_signal(GtkWidget *widget, gpointer data)
{
    /* applet has been detached; restore original size */

    gtk_widget_set_usize(GTK_WIDGET(applet),
			 prop.normal_size_x,
			 prop.normal_size_y);
  
    /* go on */
    return FALSE;  
    widget = NULL;
    data = NULL;
}

static gint
applet_orient_changed_cb(GtkWidget *widget, gpointer data)
{
    static int counter = 0;

    if(counter++ > 0)
	show_message((gchar *) _("orient. changed")); 

    /* go on */
    return FALSE;  
    widget = NULL;
    data = NULL;
}

#ifdef HAVE_PANEL_PIXEL_SIZE
/*this is when the panel size changes*/
static void
applet_pixel_size_changed_cb(GtkWidget *widget, int size, gpointer data)
{
    static int counter = 0;

    if(counter++ > 0)
	show_message((gchar *) _("size changed")); 

    prop.normal_size_y = size;
    if(size <= PIXEL_SIZE_TINY)
	{
	  if(counter > 1) /* ignore size-changed-signal at startup */
	      prop.show_frame = FALSE;
	    prop.flat_layout = TRUE;
	} 
    else if(size <= PIXEL_SIZE_SMALL)
	{
	    if(counter > 1) /* ignore size-changed-signal at startup */
	      prop.show_frame = TRUE;
	    prop.flat_layout = TRUE;
	} 
    else
	{
	    if(counter > 1) /* ignore size-changed-signal at startup */
	      prop.show_frame = TRUE;
	    prop.flat_layout = FALSE;
	}

    redraw_applet();
    return;
    widget = NULL;
    data = NULL;
}
#endif

void
redraw_applet(void)
{
    GtkWidget *hbox, *hbox_buttons;
    GtkWidget *button;
    GtkWidget *frame;
    GtkWidget *frame2;
    GtkWidget *handle;
    GtkWidget *icon;
    GtkWidget *vbox;
    int size_frames = 0;
    int size_status_line = 18;

    static GtkWidget *applet_inner_vbox = NULL;
    static GtkWidget *applet_vbox = NULL;
    static int first_time = TRUE;   

    /* recalculate sizes */
    if(prop.show_handle)
	size_frames += 0;
    if(prop.show_frame)
	size_frames += 6;
    if(prop.flat_layout) 
	size_status_line = 0;
    prop.cmd_line_size_y = prop.normal_size_y - size_status_line - size_frames;   

    if(!applet_vbox)
	{
	    applet_vbox = gtk_vbox_new(FALSE, 0);
	    gtk_container_set_border_width(GTK_CONTAINER(applet_vbox), 0);
	}

    if(applet_inner_vbox)
	gtk_widget_destroy(GTK_WIDGET(applet_inner_vbox)); 

    applet_inner_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(applet_inner_vbox), 0);
    /* in case we get destroyed elsewhere */
    gtk_signal_connect(GTK_OBJECT(applet_inner_vbox),"destroy",
		       GTK_SIGNAL_FUNC(gtk_widget_destroyed),
		       &applet_inner_vbox);


    /*

      Gtk_style *style;
      style = malloc(sizeof(Gtk_style));
      style->bg_pixmap[GTK_STATE_NORMAL] = bg_pixmap;
      gtk_widget_push_style (style);
    */
    
    if(prop.flat_layout) 
	vbox = gtk_hbox_new(FALSE, 0);
    else
	vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);
    
    /* add command line; position: top */
    if(1)
	init_command_entry();
    else
	terminal_init();

/*     gtk_box_pack_start(GTK_BOX(vbox), entry_command, FALSE, FALSE, 0); */

    /* hbox for message label and buttons */
    hbox = gtk_hbox_new(FALSE, 0);
    
    /* add message label */
    init_message_label();

    /* do not center text but put it to bottom instead */
    gtk_misc_set_alignment(GTK_MISC(label_message), 0.0, 1.0);
    gtk_box_pack_start(GTK_BOX(hbox), label_message, TRUE, TRUE, 0);

    if(prop.flat_layout) 
	hbox_buttons = gtk_vbox_new(TRUE, 0);
    else
	hbox_buttons = gtk_hbox_new(TRUE, 0);

    /* add file-browser button */
    button = gtk_button_new();
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(show_file_browser_signal),
		       NULL);
    gtk_widget_set_usize(GTK_WIDGET(button), 13, 10);
    icon = gnome_pixmap_new_from_xpm_d (browser_mini_xpm);
    gtk_container_add(GTK_CONTAINER(button), icon);
    applet_widget_set_widget_tooltip(APPLET_WIDGET(applet),
				     GTK_WIDGET(button),
				     _("Browser"));
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button, TRUE, TRUE, 0);

    /* add history button */
    button = gtk_button_new();
    gtk_signal_connect(GTK_OBJECT(button), "button_press_event",
		       GTK_SIGNAL_FUNC(show_history_signal),
		       NULL);
    gtk_widget_set_usize(GTK_WIDGET(button), 13, 10);
    icon = gnome_pixmap_new_from_xpm_d (history_mini_xpm);
    gtk_container_add(GTK_CONTAINER(button), icon);
    applet_widget_set_widget_tooltip(APPLET_WIDGET(applet),
				     GTK_WIDGET(button),
				     _("History"));
    gtk_box_pack_end(GTK_BOX(hbox_buttons), button, TRUE, TRUE, 0);

    /* add buttons into frame */
    frame = gtk_frame_new(NULL);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 1);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(frame), hbox_buttons);
    gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);

    /* put message label and history/file-browser button into vbox */
    gtk_box_pack_end(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    if (prop.show_handle)
	{
	    if (prop.show_frame)
		{
		    /* inner frame */
		    frame = gtk_frame_new(NULL);
		    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
		    gtk_container_add(GTK_CONTAINER(frame), vbox);
		}
	    
	    /* add a handle box to allow moving away this appplet from the
	       panel */
	    handle = gtk_handle_box_new();
	    gtk_signal_connect(GTK_OBJECT(handle), "child_detached",
			       GTK_SIGNAL_FUNC(applet_detached_signal),
			       NULL);
	    gtk_signal_connect(GTK_OBJECT(handle), "child_attached",
			       GTK_SIGNAL_FUNC(applet_attached_signal),
			       NULL);
	    if (prop.show_frame)
		gtk_container_add(GTK_CONTAINER(handle), frame);
	    else
		gtk_container_add(GTK_CONTAINER(handle), vbox);
	    
	    if (prop.show_frame)
		{
		    /* outer frame */
		    frame2 = gtk_frame_new(NULL);
		    gtk_frame_set_shadow_type(GTK_FRAME(frame2), GTK_SHADOW_IN);
		    gtk_container_add(GTK_CONTAINER(frame2), handle);
		}
	    
	    /* there was trouble with the tooltip */
	    /* applet_widget_set_tooltip(APPLET_WIDGET(applet),  _("Mini-Commander")); */
	    
	    if (prop.show_frame)
		gtk_box_pack_start(GTK_BOX(applet_inner_vbox), frame2, TRUE, TRUE, 0);
	    else
		gtk_box_pack_start(GTK_BOX(applet_inner_vbox), handle, TRUE, TRUE, 0);
	} 
    else 
	{
	    if (prop.show_frame)
		{
		    /* inner frame */
		    frame = gtk_frame_new(NULL);
		    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
		    gtk_container_add(GTK_CONTAINER(frame), vbox);
		    
		    /* outer frame */
		    frame2 = gtk_frame_new(NULL);
		    gtk_frame_set_shadow_type(GTK_FRAME(frame2), GTK_SHADOW_IN);
		    gtk_container_add(GTK_CONTAINER(frame2), frame);
		    
		    gtk_box_pack_start(GTK_BOX(applet_inner_vbox), frame2, TRUE, TRUE, 0);
		}
	    else
		gtk_box_pack_start(GTK_BOX(applet_inner_vbox), vbox, TRUE, TRUE, 0);
    }

    gtk_box_pack_start(GTK_BOX(applet_vbox), applet_inner_vbox, TRUE, TRUE, 0);

    if(first_time)
	applet_widget_add (APPLET_WIDGET (applet), applet_vbox);
    first_time = FALSE;

    gtk_widget_set_usize(GTK_WIDGET(applet), prop.normal_size_x, prop.normal_size_y);

    /* allow pasting into the input box by packing it after
       applet_widdget_add has bound the middle mouse button (idea taken
       from the applet Web_control by Garrett Smith) */
#if 1
    gtk_box_pack_start(GTK_BOX(vbox), entry_command, FALSE, FALSE, 0);
#else
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(frame), terminal_zvt);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
#endif

    gtk_widget_show_all(applet_vbox);
    gtk_widget_show_all(applet_inner_vbox);
  
    gtk_widget_show_all(applet);
    
}

int 
main(int argc, char **argv)
{
    /* install signal handler */
    init_exec_signal_handler();
    
    /* initialize the i18n stuff */
    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);
    
    /* intialize, this will basically set up the applet, corba and
       call gnome_init */
    applet_widget_init("mini-commander_applet", VERSION, argc, argv, NULL, 0,
		       NULL);

    /* create a new applet_widget */
    applet = applet_widget_new("mini-commander_applet");   
    /* in the rare case that the communication with the panel
       failed, error out */
    if (!applet)
	{
	    g_error("Can't create applet!\n");
	    exit(1);
	}
    gtk_signal_connect(GTK_OBJECT(applet),
		       "change_orient",
		       GTK_SIGNAL_FUNC(applet_orient_changed_cb),
		       NULL);

#ifdef HAVE_PANEL_PIXEL_SIZE
    /*we have to bind change_pixel_size before we do applet_widget_add 
      since we need to get an initial change_pixel_size signal to set our
      initial size, and we get that during the _add call*/
    gtk_signal_connect(GTK_OBJECT(applet),
		       "change_pixel_size",
		       GTK_SIGNAL_FUNC(applet_pixel_size_changed_cb),
		       NULL);
#endif
    
    load_session();
    
    gtk_signal_connect(GTK_OBJECT(applet), "destroy",
		       (GtkSignalFunc) applet_destroy_signal,
		       NULL); 
    
    /* bind the session save signal */
    gtk_signal_connect(GTK_OBJECT(applet),
		       "save_session",
		       GTK_SIGNAL_FUNC(save_session_signal),
		       NULL);

    redraw_applet();

    /* add items to applet menu */
    applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					  "properties",
					  GNOME_STOCK_MENU_PROP,
					  _("Properties..."),
					  properties_box,
					  NULL);
    
    applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					  "help",
					  GNOME_STOCK_PIXMAP_HELP,
					  _("Help"),
					  show_help,
					  NULL);

    applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					  "about",
					  GNOME_STOCK_MENU_ABOUT,
					  _("About..."),
					  about_box,
					  NULL);
      
    show_message((gchar *) _("ready...")); 
 
    /* special corba main loop */
    applet_widget_gtk_main ();
    
    return 0;
}
