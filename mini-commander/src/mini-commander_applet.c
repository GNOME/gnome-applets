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

#include <config.h>
#include <string.h>
#include <gnome.h>
#include <panel-applet.h>
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

static void applet_destroy_signal(GtkWidget *widget, gpointer data);
static gint applet_detached_signal(GtkHandleBox *hb, GtkWidget *widget, gpointer data);
static gint applet_attached_signal(GtkHandleBox *hb, GtkWidget *widget, gpointer data);
static gint applet_orient_changed_cb(GtkWidget *widget, gpointer data);
static void applet_pixel_size_changed_cb(GtkWidget *widget, int size, gpointer data);

static const BonoboUIVerb mini_commander_menu_verbs[] = {
        BONOBO_UI_VERB("Props", properties_box),
        BONOBO_UI_VERB("Help", show_help),
        BONOBO_UI_VERB("About", about_box),

        BONOBO_UI_VERB_END
};

static void
applet_destroy_signal(GtkWidget *widget, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    
    /* applet will be destroyed; save settings now */
    save_session();

   
    if (mcdata->label_timeout > 0)
    	gtk_timeout_remove (GPOINTER_TO_INT (mcdata->label_timeout));
#if 0 /* Freeing these prevents the applet from restarting - not sure what's up */
  
    g_free (mcdata->prop);
    g_free (mcdata);
  
#endif
    /* go on */
    return;  
}

static gint
applet_detached_signal(GtkHandleBox *hb, GtkWidget *widget, gpointer data)
{
    PanelApplet *applet = data;
    properties *prop;
    
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    
    /* applet has been detached; make it smaller */

    /*gtk_widget_set_usize(GTK_WIDGET(applet),
			 20,
			 prop->normal_size_y);*/
  
    /* go on */
    return FALSE;  
    widget = NULL;
    data = NULL;
}

static gint
applet_attached_signal(GtkHandleBox *hb, GtkWidget *widget, gpointer data)
{
    PanelApplet *applet = data;
    properties *prop;
    
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    
    /* applet has been detached; restore original size */

    /*gtk_widget_set_usize(GTK_WIDGET(applet),
			 prop->normal_size_x,
			 prop->normal_size_y);*/
  
    /* go on */
    return FALSE;  
    widget = NULL;
    data = NULL;
}

static gint
applet_orient_changed_cb(GtkWidget *widget, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = data;
    static int counter = 0;

    if(counter++ > 0)
	show_message((gchar *) _("orient. changed")); 

    /* go on */
    return FALSE;  
    widget = NULL;
    data = NULL;
}

/*this is called when the panel size changes*/
static void
applet_pixel_size_changed_cb(GtkWidget *widget, int size, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = data;
    properties *prop = mcdata->prop;

   
    show_message((gchar *) _("size changed")); 

    prop->normal_size_y = size;
    if(size <= GNOME_Vertigo_PANEL_X_SMALL)
	{
	    prop->show_frame = FALSE;
	    prop->flat_layout = TRUE;
	} 
    else if(size <= GNOME_Vertigo_PANEL_SMALL)
	{

	    prop->show_frame = TRUE;
	    prop->flat_layout = TRUE;
	} 
    else
	{
	    prop->show_frame = TRUE;
	    prop->flat_layout = FALSE;
	}

    redraw_applet(mcdata);
    return;
    widget = NULL;
    data = NULL;
}

void
redraw_applet(MCData *mcdata)
{
    PanelApplet *applet = mcdata->applet;
    properties *prop = mcdata->prop;
    GtkWidget *hbox, *hbox_buttons;
    GtkWidget *button;
    GtkWidget *frame;
    GtkWidget *frame2;
    GtkWidget *handle;
    GtkWidget *icon;
    GtkWidget *vbox;
    GtkTooltips *tooltips;
    int size_frames = 0;
    int size_status_line = 18;
    gboolean first_time = FALSE;

    tooltips = gtk_tooltips_new ();
    
    /* recalculate sizes */
    if(prop->show_handle)
	size_frames += 0;
    if(prop->show_frame)
	size_frames += 6;
    if(prop->flat_layout) 
	size_status_line = 0;
    prop->cmd_line_size_y = prop->normal_size_y - size_status_line - size_frames;   

    if(!mcdata->applet_vbox)
	{
	    mcdata->applet_vbox = gtk_vbox_new(FALSE, 0);
	    gtk_container_set_border_width(GTK_CONTAINER(mcdata->applet_vbox), 0);
	    first_time = TRUE;
	}

    if(mcdata->applet_inner_vbox)
      /* clean up */
      gtk_widget_destroy(GTK_WIDGET(mcdata->applet_inner_vbox)); 

    mcdata->applet_inner_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(mcdata->applet_inner_vbox), 0);
    /* in case we get destroyed elsewhere */
    gtk_signal_connect(GTK_OBJECT(mcdata->applet_inner_vbox),"destroy",
		       GTK_SIGNAL_FUNC(gtk_widget_destroyed),
		       &mcdata->applet_inner_vbox);


    /*

      Gtk_style *style;
      style = malloc(sizeof(Gtk_style));
      style->bg_pixmap[GTK_STATE_NORMAL] = bg_pixmap;
      gtk_widget_push_style (style);
    */
    
    if(prop->flat_layout) 
	vbox = gtk_hbox_new(FALSE, 0);
    else
	vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);
   
    /* add command line; position: top */
    mcdata->entry = init_command_entry(mcdata);

/*     gtk_box_pack_start(GTK_BOX(vbox), entry_command, FALSE, FALSE, 0); */

    /* hbox for message label and buttons */
    hbox = gtk_hbox_new(FALSE, 0);
#ifdef MESSAGES_NOT_NEEDED_FOR_NOW    
    /* add message label */
    init_message_label(mcdata);

    /* do not center text but put it to bottom instead */
    gtk_misc_set_alignment(GTK_MISC(label_message), 0.0, 1.0);
    gtk_box_pack_start(GTK_BOX(hbox), label_message, TRUE, TRUE, 0);
#endif
    if(prop->flat_layout && (prop->normal_size_y > GNOME_Vertigo_PANEL_X_SMALL)) 
	hbox_buttons = gtk_vbox_new(TRUE, 0);
    else
	hbox_buttons = gtk_hbox_new(TRUE, 0);

    /* add file-browser button */
    button = gtk_button_new();
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(show_file_browser_signal),
		       applet);
    gtk_widget_set_usize(GTK_WIDGET(button), 13, 10);
    icon = gnome_pixmap_new_from_xpm_d (browser_mini_xpm);
    gtk_container_add(GTK_CONTAINER(button), icon);

    gtk_tooltips_set_tip (tooltips, button, _("Browser"), NULL);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), button, TRUE, TRUE, 0);

    /* add history button */
    button = gtk_button_new();
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(show_history_signal),
		       applet);
    gtk_widget_set_usize(GTK_WIDGET(button), 13, 10);
    icon = gnome_pixmap_new_from_xpm_d (history_mini_xpm);
    gtk_container_add(GTK_CONTAINER(button), icon);

    gtk_tooltips_set_tip (tooltips, button, _("History"), NULL);
    gtk_box_pack_end(GTK_BOX(hbox_buttons), button, TRUE, TRUE, 0);

    /* add buttons into frame */
    frame = gtk_frame_new(NULL);
    gtk_container_set_border_width(GTK_CONTAINER(frame), 1);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(frame), hbox_buttons);
    gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);

    /* put message label and history/file-browser button into vbox */
    gtk_box_pack_end(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

    if (prop->show_handle)
	{
	    if (prop->show_frame)
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
			       applet);
	    gtk_signal_connect(GTK_OBJECT(handle), "child_attached",
			       GTK_SIGNAL_FUNC(applet_attached_signal),
			       applet);
	    if (prop->show_frame)
		gtk_container_add(GTK_CONTAINER(handle), frame);
	    else
		gtk_container_add(GTK_CONTAINER(handle), vbox);
	    
	    if (prop->show_frame)
		{
		    /* outer frame */
		    frame2 = gtk_frame_new(NULL);
		    gtk_frame_set_shadow_type(GTK_FRAME(frame2), GTK_SHADOW_IN);
		    gtk_container_add(GTK_CONTAINER(frame2), handle);
		}
	    
	    /* there was trouble with the tooltip */
	    /* applet_widget_set_tooltip(APPLET_WIDGET(applet),  _("Mini-Commander")); */
	    
	    if (prop->show_frame)
		gtk_box_pack_start(GTK_BOX(mcdata->applet_inner_vbox), frame2, TRUE, TRUE, 0);
	    else
		gtk_box_pack_start(GTK_BOX(mcdata->applet_inner_vbox), handle, TRUE, TRUE, 0);
	} 
    else 
	{
	    if (prop->show_frame)
		{
		    /* inner frame */
		    frame = gtk_frame_new(NULL);
		    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
		    gtk_container_add(GTK_CONTAINER(frame), vbox);
		    
		    /* outer frame */
		    frame2 = gtk_frame_new(NULL);
		    gtk_frame_set_shadow_type(GTK_FRAME(frame2), GTK_SHADOW_IN);
		    gtk_container_add(GTK_CONTAINER(frame2), frame);
		    
		    gtk_box_pack_start(GTK_BOX(mcdata->applet_inner_vbox), frame2, TRUE, TRUE, 0);
		}
	    else
		gtk_box_pack_start(GTK_BOX(mcdata->applet_inner_vbox), vbox, TRUE, TRUE, 0);
    }

    gtk_box_pack_start(GTK_BOX(mcdata->applet_vbox), mcdata->applet_inner_vbox, 
    		       TRUE, TRUE, 0);

    if (first_time)
        gtk_container_add(GTK_CONTAINER(applet), mcdata->applet_vbox);
  
    /*gtk_widget_set_usize(GTK_WIDGET(applet), prop->normal_size_x, prop->normal_size_y);*/

    /* allow pasting into the input box by packing it after
       applet_widdget_add has bound the middle mouse button (idea taken
       from the applet Web_control by Garrett Smith) */
#if 1
    gtk_box_pack_start(GTK_BOX(vbox), mcdata->entry, FALSE, FALSE, 0);
#else
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(frame), terminal_zvt);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
#endif

    gtk_widget_show_all(GTK_WIDGET (applet));

   
}

static gboolean
mini_commander_applet_fill(PanelApplet *applet)
{
    MCData *mcdata;
    
    mcdata = g_new0 (MCData, 1);
    mcdata->applet = applet;
  
    /* install signal handler */
    init_exec_signal_handler();

    g_signal_connect(GTK_OBJECT(applet),
		     "change_orient",
		     G_CALLBACK(applet_orient_changed_cb),
		     mcdata);

    /*we have to bind change_pixel_size before we do applet_widget_add 
      since we need to get an initial change_pixel_size signal to set our
      initial size, and we get that during the _add call*/
	g_signal_connect(G_OBJECT(applet),
		     "change_size",
		     G_CALLBACK(applet_pixel_size_changed_cb),
		     mcdata);
    
    mcdata->prop = load_session();
    g_object_set_data (G_OBJECT (applet), "prop", mcdata->prop);
    
    g_signal_connect(G_OBJECT(applet), "destroy",
		     G_CALLBACK(applet_destroy_signal),
		     mcdata); 

    applet_pixel_size_changed_cb(NULL, panel_applet_get_size (applet), mcdata);

    panel_applet_setup_menu_from_file (applet,
			    NULL, /* opt. datadir */
			    "GNOME_MiniCommanderApplet.xml",
			    NULL,
			    mini_commander_menu_verbs,
			    mcdata);
#if 0      
    show_message((gchar *) _("ready...")); 
#endif 
    return TRUE;
}


static gboolean
mini_commander_applet_factory(PanelApplet *applet,
			      const gchar *iid,
			      gpointer     data)
{
        gboolean retval = FALSE;

        if (!strcmp (iid, "OAFIID:GNOME_MiniCommanderApplet"))
                retval = mini_commander_applet_fill(applet); 
    
        return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_MiniCommanderApplet_Factory",
			     PANEL_TYPE_APPLET,
                             "Mini-Commander",
                             "0",
                             mini_commander_applet_factory,
                             NULL)
