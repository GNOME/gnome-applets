
/*
#
#   GNotes!
#   Copyright (C) 1999 dres <dres@debian.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "gnotes_applet.h"

#define DEFAULT_HEIGHT "100"
#define DEFAULT_WIDTH "100"
#define DEFAULT_X "0"
#define DEFAULT_Y "0"
#define DEFAULT_ONBOTTOM "0"


GtkWidget *propwindow;
gnotes_prefs defaults;

void gnotes_copy_defaults_to_defaults(gnotes_prefs *from, gnotes_prefs *to)
{
    to->height    = from->height;
    to->width     = from->width;
    to->x         = from->x;
    to->y         = from->y;
    to->onbottom  = from->onbottom;
}

void gnotes_preferences_load(const char *path, GNotes *gnotes)
{
    gnome_config_push_prefix(path);

    gnotes->defaults.height = gnome_config_get_int("height=" DEFAULT_HEIGHT);
    gnotes->defaults.width = gnome_config_get_int("width=" DEFAULT_WIDTH);
    gnotes->defaults.x = gnome_config_get_int("x=" DEFAULT_X);
    gnotes->defaults.y = gnome_config_get_int("y=" DEFAULT_Y);
    gnotes->defaults.onbottom = gnome_config_get_bool("onbottom=false");
    
    gnome_config_pop_prefix();
};

void gnotes_preferences_save(const char *path, GNotes *gnotes)
{
    g_debug("Pushing to path: %s", path);

    /* check to see whether we have a valid path */
    if(path[0] != '/')
    {
        return;
    }
    
    gnome_config_push_prefix(path);

    gnome_config_set_int("height", gnotes->defaults.height); 
    gnome_config_set_int("width", gnotes->defaults.width); 
    gnome_config_set_int("x", gnotes->defaults.x); 
    gnome_config_set_int("y", gnotes->defaults.y); 
    gnome_config_set_bool("onbottom", gnotes->defaults.onbottom);

    gnome_config_sync();
    gnome_config_drop_all();
    gnome_config_pop_prefix();
};

static void property_apply_cb(GtkWidget *wid, gpointer nodata, gpointer data)
{
    GNotes *gnotes = (GNotes*)data;

    gnotes_copy_defaults_to_defaults(&defaults, &gnotes->defaults);

    applet_widget_sync_config(APPLET_WIDGET(gnotes->applet));
}

static gint property_destroy_cb(GtkWidget *wid, gpointer data)
{
    propwindow = NULL;
    return FALSE;
}

static void update_height_cb(GtkWidget *wid, GtkWidget *data)
{
    defaults.height =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data));
    gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void update_width_cb(GtkWidget *wid, GtkWidget *data)
{
    defaults.width =
        gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data));
    gnome_property_box_changed(GNOME_PROPERTY_BOX(propwindow));
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "gnotes_applet",
					  "index.html#GNOTES-PROPERTIES" };
	gnome_help_display(NULL, &help_entry);
}

void properties_show(AppletWidget *applet, gpointer data)
{
    static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
    
    GtkWidget *frame;
    GtkWidget *height_box;
    GtkWidget *width_box;
    GtkWidget *height_spinner;
    GtkWidget *width_spinner;
    GtkObject *height_adj;
    GtkObject *width_adj;
    GtkWidget *tmp_label;

    GNotes *gnotes = (GNotes*)data;
    
    if(propwindow != 0)
    {
        gdk_window_raise(propwindow->window);
        return;
    }

    help_entry.name = gnome_app_id;

    gnotes_copy_defaults_to_defaults(&gnotes->defaults, &defaults);
    
    propwindow = gnome_property_box_new();

    gtk_window_set_title(
        GTK_WINDOW(&GNOME_PROPERTY_BOX(propwindow)->dialog.window),
        _("GNotes Settings"));

    frame = gtk_vbox_new(FALSE, 5);

    /* HEIGHT PART */
    height_box = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(frame), height_box, FALSE, FALSE, 5);
    gtk_widget_show(height_box);

    tmp_label = gtk_label_new(_("Default Height"));
    gtk_box_pack_start(GTK_BOX(height_box), tmp_label, FALSE, FALSE, 5);
    gtk_widget_show(tmp_label);
    
    height_adj = gtk_adjustment_new(defaults.height, 10.0, 500.0, 1, 1, 1);
    
    height_spinner = gtk_spin_button_new(GTK_ADJUSTMENT(height_adj), 1, 0);
    gtk_box_pack_start(GTK_BOX(height_box), height_spinner, FALSE, FALSE, 5);
    gtk_widget_show(height_spinner);

    gtk_signal_connect(GTK_OBJECT(height_adj), "value_changed",
                       GTK_SIGNAL_FUNC(update_height_cb), height_spinner);
    gtk_signal_connect(GTK_OBJECT(height_spinner), "changed",
                       GTK_SIGNAL_FUNC(update_height_cb), height_spinner);

    /* WIDTH PART */
    width_box = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(frame), width_box, FALSE, FALSE, 5);
    gtk_widget_show(width_box);
    
    tmp_label = gtk_label_new(_("Default Width"));
    gtk_box_pack_start(GTK_BOX(width_box), tmp_label, FALSE, FALSE, 5);
    gtk_widget_show(tmp_label);

    width_adj = gtk_adjustment_new(defaults.width, 10.0, 500.0, 1, 1, 1);
    
    width_spinner = gtk_spin_button_new(GTK_ADJUSTMENT(width_adj), 1, 0);
    gtk_box_pack_start(GTK_BOX(width_box), width_spinner, FALSE, FALSE, 5);
    gtk_widget_show(width_spinner);

    gtk_signal_connect(GTK_OBJECT(width_adj), "value_changed",
                       GTK_SIGNAL_FUNC(update_width_cb), width_spinner);
    gtk_signal_connect(GTK_OBJECT(width_spinner), "changed",
                       GTK_SIGNAL_FUNC(update_width_cb), width_spinner);


    tmp_label = gtk_label_new(_("General"));
    gtk_widget_show(frame);
    gnome_property_box_append_page(GNOME_PROPERTY_BOX(propwindow), frame,
                                   tmp_label);

    gtk_signal_connect(GTK_OBJECT(propwindow), "apply",
                       GTK_SIGNAL_FUNC(property_apply_cb), gnotes);
    gtk_signal_connect(GTK_OBJECT(propwindow), "destroy",
                       GTK_SIGNAL_FUNC(property_destroy_cb), NULL);
    gtk_signal_connect( GTK_OBJECT(propwindow), "help",
                        GTK_SIGNAL_FUNC(phelp_cb), NULL);

    gtk_widget_show(propwindow);
    
}
