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
#include <panel-applet.h>
#include <gconf/gconf-client.h>
#include "geyes.h"

#define UPDATE_TIMEOUT 75

EyesApplet eyes_applet = {0};
guint timeout_handle = -1;
GConfClient *client;



/* Applet transparency - Taken (and modified a bit) from Miguel's gen-util
 * printer applet (thanks to Inigo Serna who pointed this code out to me)
 * But that code was wrong - George */
static void 
applet_set_default_back (GtkWidget *dest, GtkWidget *src)
{
#ifdef FIXME
	/* Not ported */
	/* Deprecated - now use gtk_widget_set_style (GtkWidget *widget, GtkStyle *style) */
	gtk_widget_set_rc_style(dest);
        gtk_widget_queue_draw (dest);
	return;
	src = NULL;
#endif
}

static void
applet_set_back_color (GtkWidget *widget, GdkColor *color)
{
        GtkStyle *new_style;
#ifdef FIXME
        new_style = gtk_style_copy (widget->style);
        gtk_style_ref (new_style);
        new_style->bg[GTK_STATE_NORMAL] = *color;
        new_style->bg[GTK_STATE_NORMAL].pixel = 1; /* bogus */
        
        if (new_style->bg_pixmap [GTK_STATE_NORMAL]) {

                /*gdk_imlib_free_pixmap (new_style->bg_pixmap [GTK_STATE_NORMAL]);*/

                new_style->bg_pixmap [GTK_STATE_NORMAL] = NULL;
        }
        
        gtk_widget_set_style (widget, new_style);
        gtk_style_unref (new_style);
        gtk_widget_queue_draw (widget);
#endif
}

static void
applet_set_back_pixmap (GtkWidget *widget, gchar *pixmap)
{
        GdkPixbuf *pixbuf;
        GdkPixmap *pm;
        GtkStyle *new_style;
#ifdef FIXME    
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
        
        pixbuf = gdk_pixbuf_new_from_file (pixmap, NULL);
        /*imlib_image = gdk_imlib_load_image (pixmap);*/
        if (!pixbuf)
                return;
        
        /*gdk_imlib_render (imlib_image, 
                          imlib_image->rgb_width,
                          imlib_image->rgb_height);*/
        /*pm = gdk_imlib_move_image (imlib_image);*/
        gdk_pixbuf_render_pixmap_and_mask (pixbuf,&pm,NULL,0);
        new_style = gtk_style_copy (widget->style);
        gtk_style_ref (new_style);

        if (new_style->bg_pixmap [GTK_STATE_NORMAL]) 
                gdk_imlib_free_pixmap (new_style->bg_pixmap [GTK_STATE_NORMAL]);

        new_style->bg_pixmap [GTK_STATE_NORMAL] = pm;
        gtk_widget_set_style (widget, new_style);
        gtk_style_unref (new_style);
        gdk_pixbuf_unref (pixbuf);
#endif
}

static void
applet_back_change (PanelApplet *a,
		    PanelAppletBackgroundType  type,
		    GdkColor                  *color,
		    const gchar               *pixmap,
		    EyesApplet *applet) 
{
	switch (type) {
	case PANEL_PIXMAP_BACKGOUND:
                applet_set_back_pixmap (applet->fixed, pixmap);
		break;
        case PANEL_COLOR_BACKGROUND:
                applet_set_back_color(applet->hbox, color);
		break;
	default:
                applet_set_default_back (applet->fixed, applet->hbox);
		break;
	}
	return;
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
	gdk_pixbuf_render_to_drawable_alpha (eyes_applet.eye_image, 
					     eyes_applet.eyes[eye_num]->window,
					     0, 0, 
                         		     0, 0,
                         		     eyes_applet.eye_width, 
                         		     eyes_applet.eye_height,
                         		     GDK_PIXBUF_ALPHA_BILEVEL,
                         		     128,
                         		     GDK_RGB_DITHER_NONE,
                         		     0,
                         		     0);
        
	gdk_pixbuf_render_to_drawable_alpha (eyes_applet.pupil_image, 
					     eyes_applet.eyes[eye_num]->window,
					     0, 0, 
                         		     pupil_x - eyes_applet.pupil_width / 2, 
                         		     pupil_y - eyes_applet.pupil_height / 2,
                         		     -1, -1,
                         		     GDK_PIXBUF_ALPHA_BILEVEL,
                         		     128,
                         		     GDK_RGB_DITHER_NONE,
                         		     0,
                         		     0);
 
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
        
        return TRUE;
}

static void
about_cb (BonoboUIComponent *uic, gpointer user_data, const gchar *verbname)
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
                                 _("A goofy little xeyes clone for the GNOME panel."),
                                 authors,
                                 NULL,
                                 NULL,
                                 NULL);
	g_signal_connect (G_OBJECT(about), "destroy",
			    G_CALLBACK(gtk_widget_destroyed), &about);
        gtk_widget_show (about);
}

void
destroy_eyes (void)
{
        int i;
        
        for (i = 0; i < eyes_applet.num_eyes; i++) {
                gtk_widget_destroy (eyes_applet.eyes[i]);
        }
        gtk_widget_destroy (eyes_applet.hbox);
}

static void
properties_save ()
{
	gconf_client_set_string (client, "/applets/gEyes/theme-path", eyes_applet.theme_name, NULL);
}

static void
properties_load ()
{
        gchar *theme_path = NULL;

	theme_path = gconf_client_get_string(client, "/applets/gEyes/theme-path", NULL);
	/* FIXME: should install gconf schemas to get defaults*/
	if (theme_path == NULL)
		theme_path = g_strdup (GEYES_THEMES_DIR"Default");
	
        load_theme (theme_path);
        g_free (theme_path);
}


static gint
save_session_cb (GtkWidget *widget,
                 gchar *privcfgpath,
                 gchar *globcfgpath)
{
        properties_save ();
	return FALSE;
	widget = NULL;
	globcfgpath = NULL;
}

void 
create_eyes (void)
{
	/* Ported */
        gint i;
        gint color_depth; 
        
        eyes_applet.hbox = gtk_hbox_new (FALSE, 0);
        gtk_fixed_put (GTK_FIXED (eyes_applet.fixed), eyes_applet.hbox, 0, 0);
        
        for (i = 0; i < eyes_applet.num_eyes; i++) {
                eyes_applet.eyes[i] = gtk_drawing_area_new ();
                
                if (eyes_applet.eyes[i] == NULL) {
                        g_error ("Error creating geyes\n");
                        exit (1);
                }
               
		gtk_widget_set_size_request (GTK_WIDGET (eyes_applet.eyes[i]),
					     eyes_applet.eye_width,
					     eyes_applet.eye_height);
 
                gtk_widget_show (eyes_applet.eyes[i]);
                
                color_depth = (gdk_window_get_visual (eyes_applet.applet->window))->depth;
                
		gtk_box_pack_start (GTK_BOX (eyes_applet.hbox), 
                                    eyes_applet.eyes [i],
                                    TRUE,
                                    TRUE,
                                    0);
                
                gtk_widget_realize (eyes_applet.eyes[i]);
		draw_eye (i, eyes_applet.eye_width / 2,
                          eyes_applet.eye_height / 2);
                
        }
        gtk_widget_show (eyes_applet.hbox);
        
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
help_cb (BonoboUIComponent *uic, gpointer user_data, const gchar *verbname)
{
#ifdef FIXME
    static GnomeHelpMenuEntry help_entry = { "geyes_applet", "index.html"};
    gnome_help_display(NULL, &help_entry);
#endif
}


static const BonoboUIVerb geyes_applet_menu_verbs [] = {
        BONOBO_UI_VERB ("Props", properties_cb),
        BONOBO_UI_VERB ("Help", help_cb),
        BONOBO_UI_VERB ("About", about_cb),

        BONOBO_UI_VERB_END
};

static const char geyes_applet_menu_xml [] =
	"<popup name=\"button3\">\n"
	"   <menuitem name=\"Item 1\" verb=\"Props\" _label=\"Properties\"/>\n"
	"             pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
	"   <menuitem name=\"Item 2\" verb=\"Help\" _label=\"Help\"/>\n"
	"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
	"   <menuitem name=\"Item 3\" verb=\"About\" _label=\"About\"/>\n"
	"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
	"</popup>\n";

static gboolean
geyes_applet_fill (PanelApplet *applet)
{
	
	client = gconf_client_get_default ();
	gconf_client_add_dir(client,
                        "/extra/test/directory",
                        GCONF_CLIENT_PRELOAD_NONE,
                        NULL);
       	
       	gdk_rgb_init ();
       	
        properties_load ();
        
        eyes_applet.fixed = gtk_fixed_new ();
        eyes_applet.applet = GTK_WIDGET (applet);

	gtk_container_add (GTK_CONTAINER (applet), eyes_applet.fixed);
	
        create_eyes ();        
        
        timeout_handle = gtk_timeout_add (UPDATE_TIMEOUT,
			(GtkFunction)timer_cb, NULL);
			
	panel_applet_setup_menu (PANEL_APPLET (eyes_applet.applet),
				 geyes_applet_menu_xml,
				 geyes_applet_menu_verbs,
				 NULL);
	
	gtk_widget_show_all (eyes_applet.applet);

	g_signal_connect (G_OBJECT (eyes_applet.applet), "change_background",
			  G_CALLBACK (applet_back_change), &eyes_applet);
	g_signal_connect (G_OBJECT (eyes_applet.applet), "destroy_event",
			  G_CALLBACK (delete_cb), NULL);
	  
	return TRUE;
	
}

static gboolean
geyes_applet_factory (PanelApplet *applet,
		      const gchar *iid,
		      gpointer     data)
{
	gboolean retval = FALSE;
    
	if (!strcmp (iid, "OAFIID:GNOME_GeyesApplet"))
		retval = geyes_applet_fill (applet); 
    
	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_GeyesApplet_Factory",
			     "Big brother is watching you",
			     "0",
			     geyes_applet_factory,
			     NULL)
