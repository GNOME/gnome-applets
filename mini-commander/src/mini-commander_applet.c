 /*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <oliver@maruhn.com>,
 *               2002 Sun Microsystems
 *
 * Authors: Oliver Maruhn <oliver@maruhn.com>,
 *          Mark McLoughlin <mark@skynet.ie>
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

#include <gtk/gtk.h>
#include <panel-applet.h>
#include <gconf/gconf-client.h>
#include <libgnomeui/gnome-window-icon.h>
#include "mini-commander_applet.h"
#include "preferences.h"
#include "command_line.h"
#include "about.h"
#include "help.h"

#include "browser-mini.xpm"
#include "history-mini.xpm"

#define COMMANDLINE_BROWSER_STOCK "commandline-browser"
#define COMMANDLINE_HISTORY_STOCK "commandline-history"
 
#define COMMANDLINE_DEFAULT_ICON_SIZE 6

static GtkIconSize button_icon_size = 0;

static const BonoboUIVerb mini_commander_menu_verbs [] = {
        BONOBO_UI_UNSAFE_VERB ("Props", mc_show_preferences),
        BONOBO_UI_UNSAFE_VERB ("Help",  show_help),
        BONOBO_UI_UNSAFE_VERB ("About", about_box),

        BONOBO_UI_VERB_END
};

typedef struct {
    char *stock_id;
    const char **icon_data;
} CommandLineStockIcon;

static CommandLineStockIcon items[] = {
    { COMMANDLINE_BROWSER_STOCK, browser_mini_xpm },
    { COMMANDLINE_HISTORY_STOCK, history_mini_xpm }
};

static void
register_command_line_stock_icons (GtkIconFactory *factory)
{
    gint i;

    for (i = 0; i < G_N_ELEMENTS (items); ++i) {
       GtkIconSet *icon_set;
       GdkPixbuf *pixbuf;

       pixbuf = gdk_pixbuf_new_from_xpm_data ((items[i].icon_data));

       icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
       gtk_icon_factory_add (factory, items[i].stock_id, icon_set);

       gtk_icon_set_unref (icon_set);
       g_object_unref (G_OBJECT (pixbuf));
    }

}

static void
command_line_init_stock_icons ()
{

    GtkIconFactory *factory;

    factory = gtk_icon_factory_new ();
    gtk_icon_factory_add_default (factory);

    register_command_line_stock_icons (factory);

    button_icon_size = gtk_icon_size_register ("mini-commander-icon",
                                                 COMMANDLINE_DEFAULT_ICON_SIZE,
                                                 COMMANDLINE_DEFAULT_ICON_SIZE);
    g_object_unref (factory);

}

void
set_atk_name_description (GtkWidget  *widget,
			  const char *name,
			  const char *description)
{	
    AtkObject *aobj;
	
    aobj = gtk_widget_get_accessible (widget);
    if (GTK_IS_ACCESSIBLE (aobj) == FALSE)
        return;

    atk_object_set_name (aobj, name);
    atk_object_set_description (aobj, description);
}

/* This is a hack around the fact that gtk+ doesn't
 * propogate button presses on button2/3.
 */
static gboolean
button_press_hack (GtkWidget      *widget,
		   GdkEventButton *event,
		   MCData         *mc)
{
    if (event->button == 3 || event->button == 2) {
	gtk_propagate_event (GTK_WIDGET (mc->applet), (GdkEvent *) event);
	return TRUE;
    }

    return FALSE;
}

/* Send button presses on the applet to the entry. This makes Fitts' law work (ie click on the bottom
** most pixel and the key press will be sent to the entry */
static gboolean
send_button_to_entry_event (GtkWidget *widget, GdkEventButton *event, MCData *mc)
{

	if (event->button == 1) {
		gtk_widget_grab_focus (mc->entry);
		return TRUE;
	}
	return FALSE;

}


void
mc_applet_draw (MCData *mc)
{
    GtkWidget *icon;
    GtkWidget *button;
    GtkWidget *hbox_buttons;
    GtkWidget *frame;
    MCPreferences prefs = mc->preferences;
    int        size_frames = 0;
    gchar     *command_text = NULL;

    if (mc->preferences.show_frame)
	size_frames += 6;

    if (mc->entry != NULL)
	command_text = g_strdup (gtk_editable_get_chars (GTK_EDITABLE (mc->entry), 0, -1));

    mc->cmd_line_size_y = mc->preferences.normal_size_y - size_frames;   

    if (mc->applet_box) {
        gtk_widget_destroy (mc->applet_box);	
    }
    mc->applet_box = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (mc->applet_box), 0);

    mc_create_command_entry (mc);

    if (command_text != NULL) {
	gtk_entry_set_text (GTK_ENTRY (mc->entry), command_text);
	g_free (command_text);
    }

    /* hbox for message label and buttons */
    if (prefs.normal_size_y > GNOME_Vertigo_PANEL_X_SMALL) 
	hbox_buttons = gtk_vbox_new (TRUE, 0);
    else
	hbox_buttons = gtk_hbox_new (TRUE, 0);

    /* add file-browser button */
    button = gtk_button_new ();
    
    g_signal_connect (button, "clicked",
		      G_CALLBACK (mc_show_file_browser), mc);
    g_signal_connect (button, "button_press_event",
		      G_CALLBACK (button_press_hack), mc);

    icon = gtk_image_new_from_stock (COMMANDLINE_BROWSER_STOCK,   								      button_icon_size);
    gtk_container_add (GTK_CONTAINER (button), icon);

    gtk_tooltips_set_tip (mc->tooltips, button, _("Browser"), NULL);
    gtk_box_pack_start (GTK_BOX (hbox_buttons), button, TRUE, TRUE, 0);
	
    set_atk_name_description (button,
			      _("Browser"),
			      _("Click this button to start the browser"));

    /* add history button */
    button = gtk_button_new ();
    
    g_signal_connect (button, "clicked",
		      G_CALLBACK (mc_show_history), mc);
    g_signal_connect (button, "button_press_event",
		      G_CALLBACK (button_press_hack), mc);

    icon = gtk_image_new_from_stock (COMMANDLINE_HISTORY_STOCK, 								     button_icon_size);
    gtk_container_add (GTK_CONTAINER (button), icon);

    gtk_tooltips_set_tip (mc->tooltips, button, _("History"), NULL);
    gtk_box_pack_end (GTK_BOX (hbox_buttons), button, TRUE, TRUE, 0);

    set_atk_name_description (button,
			      _("History"),
			      _("Click this button for the list of previous commands"));
    

    if (mc->preferences.show_handle) {
	GtkWidget *handle;
	GtkWidget *hbox;

	/* add a handle box to allow moving away this appplet from the panel */
	handle = gtk_handle_box_new ();
	gtk_box_pack_start (GTK_BOX (mc->applet_box), handle, TRUE, TRUE, 0);
	
	hbox = gtk_hbox_new (FALSE, 2);
	
	if (mc->preferences.show_frame) {
	    
	    frame = gtk_frame_new (NULL);
	    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	    gtk_container_add (GTK_CONTAINER (handle), frame);
	    gtk_container_add (GTK_CONTAINER (frame), hbox);

	    gtk_box_pack_start (GTK_BOX (hbox), mc->entry, TRUE, TRUE, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), hbox_buttons, TRUE, TRUE, 0);
	} else {
	    gtk_container_add (GTK_CONTAINER (handle), hbox);
	    gtk_box_pack_start (GTK_BOX (hbox), mc->entry, TRUE, TRUE, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), hbox_buttons, TRUE, TRUE, 0);
	}
    } else {
        GtkWidget *hbox;
        
        hbox = gtk_hbox_new (FALSE, 2);
        gtk_container_set_border_width (GTK_CONTAINER (hbox), 0);
	if (mc->preferences.show_frame) {
	   
  	    frame = gtk_frame_new (NULL);
	    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_OUT);
	    gtk_box_pack_start (GTK_BOX (mc->applet_box), frame, TRUE, TRUE, 0);
	    gtk_container_add (GTK_CONTAINER (frame), hbox);
		    
	    gtk_box_pack_start (GTK_BOX (hbox), mc->entry, TRUE, TRUE, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), hbox_buttons, TRUE, TRUE, 0);
	
	} else {
	    gtk_box_pack_start (GTK_BOX (mc->applet_box), hbox, TRUE, TRUE, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), mc->entry, TRUE, TRUE, 0);
	    gtk_box_pack_start (GTK_BOX (hbox), hbox_buttons, TRUE, TRUE, 0);
	}
    }

    
    gtk_container_add (GTK_CONTAINER (mc->applet), mc->applet_box);
    
    gtk_widget_show_all (mc->applet_box);
}

static void
mc_destroyed (GtkWidget *widget,
	      MCData    *mc)
{
    GConfClient *client;
    int          i;

    client = gconf_client_get_default ();
    for (i = 0; i < MC_NUM_LISTENERS; i++) {
	gconf_client_notify_remove (client, mc->listeners [i]);
	mc->listeners [i] = 0;
    }
    g_object_unref (client);

    g_object_unref (mc->tooltips);	

    mc_macros_free (mc->preferences.macros);

    if (mc->prefs_dialog.dialog)
        gtk_widget_destroy (mc->prefs_dialog.dialog);

    if (mc->prefs_dialog.dialog)
        g_object_unref (mc->prefs_dialog.macros_store);

    if (mc->file_select)
        gtk_widget_destroy (mc->file_select);
    
    g_free (mc);
}

static void
mc_pixel_size_changed (PanelApplet *applet,
		       guint        size,
		       MCData      *mc)
{
    mc->preferences.normal_size_y = size;

    mc_applet_draw (mc);
}

static gboolean
mini_commander_applet_fill (PanelApplet *applet)
{
    MCData *mc;
    
    gnome_window_icon_set_default_from_file (GNOME_ICONDIR "/gnome-mini-commander.png");
    
    mc = g_new0 (MCData, 1);
    mc->applet = applet;

    panel_applet_add_preferences (applet, "/schemas/apps/mini-commander/prefs", NULL);
    panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
    mc_load_preferences (mc);
    command_line_init_stock_icons ();

    mc->tooltips = gtk_tooltips_new ();
    g_object_ref (mc->tooltips);
    gtk_object_sink (GTK_OBJECT (mc->tooltips));
  
    g_signal_connect (mc->applet, "change_size",
		      G_CALLBACK (mc_pixel_size_changed), mc);
    mc_pixel_size_changed (mc->applet, panel_applet_get_size (applet), mc);

    gtk_widget_show (GTK_WIDGET (mc->applet));
    
    g_signal_connect (mc->applet, "destroy", G_CALLBACK (mc_destroyed), mc); 
    g_signal_connect (mc->applet, "button_press_event",
    			       G_CALLBACK (send_button_to_entry_event), mc);    

    panel_applet_setup_menu_from_file (mc->applet,
				       NULL,
				       "GNOME_MiniCommanderApplet.xml",
				       NULL,
				       mini_commander_menu_verbs,
				       mc);

    set_atk_name_description (GTK_WIDGET (applet),
			      _("Mini-Commander applet"),
			      _("This applet adds a command line to the panel"));
    
    return TRUE;
}

static gboolean
mini_commander_applet_factory (PanelApplet *applet,
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
                             "command-line",
                             "0",
                             mini_commander_applet_factory,
                             NULL)
