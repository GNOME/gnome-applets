/* -*- Mode: C++; c-basic-offset: 8 -*-
 * geyes.c - A cheap xeyes ripoff.
 * Copyright (C) 1999 Dave Camp
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
#include <math.h>
#include <gnome.h>
#include <applet-widget.h>
#include <libgnomeui/gnome-window-icon.h>

#include "geyes.h"

#define UPDATE_TIMEOUT 75

EyesApplet eyes_applet = {0};
guint timeout_handle = -1;

/* Applet transparency - Taken (and modified a bit) from Miguel's gen-util
 * printer applet (thanks to Inigo Serna who pointed this code out to me)
 * But that code was wrong - George */
static void 
applet_set_default_back (GtkWidget *dest, GtkWidget *src)
{
	gtk_widget_set_rc_style(dest);
        gtk_widget_queue_draw (dest);
	return;
	src = NULL;
}

static void
applet_set_back_color (GtkWidget *widget, GdkColor *color)
{
        GtkStyle *new_style;
        new_style = gtk_style_copy (widget->style);
        gtk_style_ref (new_style);
        new_style->bg[GTK_STATE_NORMAL] = *color;
        new_style->bg[GTK_STATE_NORMAL].pixel = 1; /* bogus */
        
        if (new_style->bg_pixmap [GTK_STATE_NORMAL]) {
                gdk_imlib_free_pixmap (new_style->bg_pixmap [GTK_STATE_NORMAL]);
                new_style->bg_pixmap [GTK_STATE_NORMAL] = NULL;
        }
        
        gtk_widget_set_style (widget, new_style);
        gtk_style_unref (new_style);
        gtk_widget_queue_draw (widget);
}

static void
applet_set_back_pixmap (GtkWidget *widget, gchar *pixmap)
{
        GdkImlibImage *imlib_image;
        GdkPixmap *pm;
        GtkStyle *new_style;
        
        if (!pixmap || strcmp (pixmap, "") == 0) {
                new_style = gtk_style_copy (widget->style);
                gtk_style_ref (new_style);
                pm = new_style->bg_pixmap [GTK_STATE_NORMAL];
                if (pm) 
                        gdk_imlib_free_pixmap (pm);
                new_style->bg_pixmap [GTK_STATE_NORMAL] = NULL;
                gtk_widget_set_style (widget, new_style);
                gtk_style_unref (new_style);
                return;
        }
        if (!g_file_exists (pixmap)) 
                return;
        
        imlib_image = gdk_imlib_load_image (pixmap);
        if (!imlib_image)
                return;
        
        gdk_imlib_render (imlib_image, 
                          imlib_image->rgb_width,
                          imlib_image->rgb_height);
        pm = gdk_imlib_move_image (imlib_image);
        new_style = gtk_style_copy (widget->style);
        gtk_style_ref (new_style);
        if (new_style->bg_pixmap [GTK_STATE_NORMAL]) 
                gdk_imlib_free_pixmap (new_style->bg_pixmap [GTK_STATE_NORMAL]);
        new_style->bg_pixmap [GTK_STATE_NORMAL] = pm;
        gtk_widget_set_style (widget, new_style);
        gtk_style_unref (new_style);
        gdk_imlib_destroy_image (imlib_image);
}

static void
applet_back_change (GtkWidget *w, 
                    PanelBackType type,
                    gchar *pixmap,
                    GdkColor *color,
                    EyesApplet *applet) 
{
	switch (type) {
	case PANEL_BACK_PIXMAP:
                applet_set_back_pixmap (applet->fixed, pixmap);
		break;
        case PANEL_BACK_COLOR:
                applet_set_back_color(applet->fixed, color);
		break;
	default:
                applet_set_default_back (applet->fixed, applet->hbox);
		break;
	}
	return;
	w = NULL;
}

/* TODO - Optimize this a bit */
static void 
calculate_pupil_xy  (gint x, gint y, gint *pupil_x, gint *pupil_y)
{
        double angle;
        double sina;
        double cosa;
        double nx;
        double ny;
        double h;
        
        nx = x - ((double)eyes_applet.eye_width / 2);
        ny = y - ((double)eyes_applet.eye_height / 2);
        
        angle = atan2 (nx, ny);
        h = hypot (nx, ny);
        if (abs (h) 
            < (abs (hypot (eyes_applet.eye_height / 2, eyes_applet.eye_width / 2)) - eyes_applet.wall_thickness - eyes_applet.pupil_height)) {
                *pupil_x = x;
                *pupil_y = y;
                return;
        }
        
        sina = sin (angle);
        cosa = cos (angle);
        
        *pupil_x = hypot ((eyes_applet.eye_height / 2) * cosa, (eyes_applet.eye_width / 2)* sina) * sina;
        *pupil_y = hypot ((eyes_applet.eye_height / 2) * cosa, (eyes_applet.eye_width / 2)* sina) * cosa;
        *pupil_x -= hypot ((eyes_applet.pupil_width / 2) * sina, (eyes_applet.pupil_height / 2)* cosa) * sina;
        *pupil_y -= hypot ((eyes_applet.pupil_width / 2) * sina, (eyes_applet.pupil_height / 2) * cosa) * cosa;
        *pupil_x -= hypot ((eyes_applet.wall_thickness / 2) * sina, (eyes_applet.wall_thickness / 2) * cosa) * sina;
        *pupil_y -= hypot ((eyes_applet.wall_thickness / 2) * sina, (eyes_applet.wall_thickness / 2)* cosa) * cosa;
        
        *pupil_x += (eyes_applet.eye_width / 2);
        *pupil_y += (eyes_applet.eye_height / 2);
}

static void 
draw_eye (gint eye_num, 
          gint pupil_x, 
          gint pupil_y)
{
        gdk_draw_pixmap (eyes_applet.pixmap [eye_num],
                         eyes_applet.applet->style->black_gc,
                         eyes_applet.eye_image->pixmap,
                         0, 0, 
                         0, 0,
                         eyes_applet.eye_width, 
                         eyes_applet.eye_height);
        
        if (eyes_applet.pupil_image->shape_mask) {
                gdk_gc_set_clip_mask (eyes_applet.applet->style->black_gc, 
                                      eyes_applet.pupil_image->shape_mask);
                gdk_gc_set_clip_origin (eyes_applet.applet->style->black_gc, 
                                        pupil_x - eyes_applet.pupil_width / 2, 
                                        pupil_y - eyes_applet.pupil_height / 2);
        }
        gdk_draw_pixmap (eyes_applet.pixmap [eye_num],
                         eyes_applet.applet->style->black_gc,
                         eyes_applet.pupil_image->pixmap,
                         0, 0, 
                         pupil_x - eyes_applet.pupil_width / 2, 
                         pupil_y - eyes_applet.pupil_height / 2,
                         -1, -1);
        if (eyes_applet.pupil_image->shape_mask) {
                gdk_gc_set_clip_mask (eyes_applet.applet->style->black_gc, 
                                      NULL);
                gdk_gc_set_clip_origin (eyes_applet.applet->style->black_gc, 
                                        0, 0);
        }
        
}

static gint 
timer_cb (void)
{
        gint x, y;
        gint pupil_x, pupil_y;
        gint i;
        
        for (i = 0; i < eyes_applet.num_eyes; i++) {
                gdk_window_get_pointer (eyes_applet.eyes[i]->window, 
                                        &x, &y, NULL);
                calculate_pupil_xy (x, y, &pupil_x, &pupil_y);
                draw_eye (i, pupil_x, pupil_y);
        }
        
        for (i = 0; i < eyes_applet.num_eyes; i++) {
                gdk_draw_pixmap(eyes_applet.eyes[i]->window,
                                eyes_applet.eyes[i]->style->fg_gc[GTK_WIDGET_STATE (eyes_applet.eyes[i])],
                                eyes_applet.pixmap[i],
                                0, 0,
                                0, 0,
                                eyes_applet.eye_width, eyes_applet.eye_height);
                gdk_window_shape_combine_mask (eyes_applet.eyes[i]->window,
                                               eyes_applet.eye_image->shape_mask,
                                               0, 
                                               0);
        }
        return TRUE;
}

static void
about_cb (void)
{
        static GtkWidget *about = NULL;
        const gchar *authors [] = {N_("Dave Camp <campd@oit.edu>"), NULL};

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}
        
        about = gnome_about_new (_("gEyes"), VERSION,
				 _("Copyright (C) 1999 Dave Camp"),
                                 authors,
                                 _("A goofy little xeyes clone for the GNOME panel."),
                                 NULL);
	gtk_signal_connect (GTK_OBJECT(about), "destroy",
			    GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);
        gtk_widget_show (about);
}

void
destroy_eyes (void)
{
        int i;
        gtk_signal_disconnect_by_func (GTK_OBJECT (eyes_applet.applet),
                                       GTK_SIGNAL_FUNC (applet_back_change),
                                       &eyes_applet);
		
		
        for (i = 0; i < eyes_applet.num_eyes; i++) {
                gdk_pixmap_unref (eyes_applet.pixmap[i]);
                gtk_widget_destroy (eyes_applet.eyes[i]);
        }
        gtk_widget_destroy (eyes_applet.hbox);
        gtk_widget_destroy (eyes_applet.fixed);
}

static void
properties_save (gchar *path)
{
        gnome_config_push_prefix (path);
        gnome_config_set_string ("gEyes/theme-path", 
                                 eyes_applet.theme_name);
        gnome_config_sync ();
        gnome_config_drop_all ();
        gnome_config_pop_prefix ();
}

static void
properties_load (gchar *path)
{
        gchar *theme_path;
        gchar *config_str;
        gnome_config_push_prefix (path);
        config_str = g_strdup_printf ("gEyes/theme-path=%sDefault", 
                                      GEYES_THEMES_DIR);
        theme_path = gnome_config_get_string_with_default (config_str, NULL);
        load_theme (theme_path);
        g_free (config_str);
}


static gint
save_session_cb (GtkWidget *widget,
                 gchar *privcfgpath,
                 gchar *globcfgpath)
{
        properties_save (privcfgpath);
	return FALSE;
	widget = NULL;
	globcfgpath = NULL;
}

void 
create_eyes (void)
{
        gint i;
        gint color_depth; 
        
        eyes_applet.fixed = gtk_fixed_new ();
        
        gtk_signal_connect (GTK_OBJECT (eyes_applet.applet), "back_change", 
                            GTK_SIGNAL_FUNC (applet_back_change), 
                            &eyes_applet);
        
        eyes_applet.hbox = gtk_hbox_new (FALSE, FALSE);
        
        for (i = 0; i < eyes_applet.num_eyes; i++) {
                eyes_applet.eyes[i] = gtk_drawing_area_new ();
                
                if (eyes_applet.eyes[i] == NULL) {
                        g_error ("Error creating geyes\n");
                        exit (1);
                }
                
                gtk_drawing_area_size (GTK_DRAWING_AREA (eyes_applet.eyes[i]),
                                       eyes_applet.eye_width, 
                                       eyes_applet.eye_height);
                
                gtk_widget_show (eyes_applet.eyes[i]);
                
                color_depth = 
                        gdk_window_get_visual (eyes_applet.applet->window)->depth;
                eyes_applet.pixmap[i] = gdk_pixmap_new (eyes_applet.eyes[i]->window,
                                                        eyes_applet.eye_width,
                                                        eyes_applet.eye_height,
                                                        color_depth);
                
                draw_eye (i, eyes_applet.eye_width / 2,
                          eyes_applet.eye_height / 2);
                
                gtk_box_pack_start (GTK_BOX (eyes_applet.hbox), 
                                    eyes_applet.eyes [i],
                                    TRUE,
                                    TRUE,
                                    0);
        }
        gtk_fixed_put (GTK_FIXED (eyes_applet.fixed), eyes_applet.hbox, 0, 0);
        applet_widget_add (APPLET_WIDGET (eyes_applet.applet), 
                           eyes_applet.fixed);
        gtk_widget_show (eyes_applet.hbox);
        gtk_widget_show (eyes_applet.fixed);
}

static gint
delete_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_timeout_remove (timeout_handle);
	timeout_handle = -1;
	return FALSE;
	widget = NULL;
	data = NULL;
	event = NULL;
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
    static GnomeHelpMenuEntry help_entry = { "geyes_applet", "index.html"};
    gnome_help_display(NULL, &help_entry);
}


static void
create_eyes_applet (void)
{
	eyes_applet.applet = applet_widget_new ("geyes_applet");
	
	if (!eyes_applet.applet)
		g_error ("Can't create applet!\n");
        
        gtk_signal_connect (GTK_OBJECT (eyes_applet.applet),
                            "save_session",
                            GTK_SIGNAL_FUNC (save_session_cb), 
                            NULL);
	gtk_signal_connect (GTK_OBJECT (eyes_applet.applet),
			    "delete_event",
			    GTK_SIGNAL_FUNC (delete_cb),
			    NULL);

        applet_widget_register_stock_callback (APPLET_WIDGET (eyes_applet.applet),
                                               "properties",
                                               GNOME_STOCK_MENU_PROP,
                                               _("Properties..."),
                                               (AppletCallbackFunc)properties_cb,
                                               NULL);        
	applet_widget_register_stock_callback (APPLET_WIDGET (eyes_applet.applet),
					       "help",
					       GNOME_STOCK_PIXMAP_HELP,
					       _("Help"),
					       help_cb, NULL);
        applet_widget_register_stock_callback (APPLET_WIDGET (eyes_applet.applet),
                                               "about",
                                               GNOME_STOCK_MENU_ABOUT,
                                               _("About..."),
                                               (AppletCallbackFunc)about_cb,
                                               NULL);
        
        gtk_widget_realize (eyes_applet.applet);
}

int 
main (int argc, char *argv[])
{
        
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
        textdomain (PACKAGE);
        
        applet_widget_init ("geyes_applet", VERSION, argc, argv, NULL, 0, NULL);
        gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-eyes.png");
        create_eyes_applet ();
        properties_load (APPLET_WIDGET (eyes_applet.applet)->privcfgpath);
        create_eyes ();
        
        timeout_handle = gtk_timeout_add (UPDATE_TIMEOUT,
			(GtkFunction)timer_cb, NULL);										  
        
        gtk_widget_show (eyes_applet.applet);
        
        applet_widget_gtk_main ();
        
        return 0;
}



