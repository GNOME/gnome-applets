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
#include <panel-applet-gconf.h>
#include "geyes.h"

#define UPDATE_TIMEOUT 75

static void
applet_set_back_pixmap (GtkWidget *widget, const gchar *pixmap)
{
#ifdef FIXME    
        GdkPixbuf *pixbuf;
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
		    EyesApplet                *eyes_applet) 
{
	GtkRcStyle *rc_style = gtk_rc_style_new ();
	gint i;
                
	switch (type) {
	case PANEL_PIXMAP_BACKGROUND:
                applet_set_back_pixmap (eyes_applet->fixed, pixmap);
		break;
        case PANEL_COLOR_BACKGROUND:
                for (i = 0; i < eyes_applet->num_eyes; i++) {
                	gtk_widget_modify_bg (eyes_applet->eyes[i],
					      GTK_STATE_NORMAL,
			    	       	      color);
        	}
		break;
	default:
                for (i = 0; i < eyes_applet->num_eyes; i++) {
                	gtk_widget_modify_style (eyes_applet->eyes[i], rc_style);
        	}
		break;
	}
	gtk_rc_style_unref (rc_style);
	return;
}


/* TODO - Optimize this a bit */
static void 
calculate_pupil_xy (EyesApplet *eyes_applet,
		    gint x, gint y,
		    gint *pupil_x, gint *pupil_y)
{
        double angle;
        double sina;
        double cosa;
        double nx;
        double ny;
        double h;
        
        nx = x - ((double) eyes_applet->eye_width / 2);
        ny = y - ((double) eyes_applet->eye_height / 2);
        
        angle = atan2 (nx, ny);
        h = hypot (nx, ny);
        if (abs (h) 
            < (abs (hypot (eyes_applet->eye_height / 2, eyes_applet->eye_width / 2)) - eyes_applet->wall_thickness - eyes_applet->pupil_height)) {
                *pupil_x = x;
                *pupil_y = y;
                return;
        }
        
        sina = sin (angle);
        cosa = cos (angle);
        
        *pupil_x = hypot ((eyes_applet->eye_height / 2) * cosa, (eyes_applet->eye_width / 2)* sina) * sina;
        *pupil_y = hypot ((eyes_applet->eye_height / 2) * cosa, (eyes_applet->eye_width / 2)* sina) * cosa;
        *pupil_x -= hypot ((eyes_applet->pupil_width / 2) * sina, (eyes_applet->pupil_height / 2)* cosa) * sina;
        *pupil_y -= hypot ((eyes_applet->pupil_width / 2) * sina, (eyes_applet->pupil_height / 2) * cosa) * cosa;
        *pupil_x -= hypot ((eyes_applet->wall_thickness / 2) * sina, (eyes_applet->wall_thickness / 2) * cosa) * sina;
        *pupil_y -= hypot ((eyes_applet->wall_thickness / 2) * sina, (eyes_applet->wall_thickness / 2)* cosa) * cosa;
        
        *pupil_x += (eyes_applet->eye_width / 2);
        *pupil_y += (eyes_applet->eye_height / 2);
}

static void 
draw_eye (EyesApplet *eyes_applet,
	  gint eye_num, 
          gint pupil_x, 
          gint pupil_y)
{
	gdk_pixbuf_render_to_drawable_alpha (eyes_applet->eye_image, 
					     eyes_applet->eyes[eye_num]->window,
					     0, 0, 
                         		     0, 0,
                         		     eyes_applet->eye_width, 
                         		     eyes_applet->eye_height,
                         		     GDK_PIXBUF_ALPHA_BILEVEL,
                         		     128,
                         		     GDK_RGB_DITHER_NONE,
                         		     0,
                         		     0);
        
	gdk_pixbuf_render_to_drawable_alpha (eyes_applet->pupil_image, 
					     eyes_applet->eyes[eye_num]->window,
					     0, 0, 
                         		     pupil_x - eyes_applet->pupil_width / 2, 
                         		     pupil_y - eyes_applet->pupil_height / 2,
                         		     -1, -1,
                         		     GDK_PIXBUF_ALPHA_BILEVEL,
                         		     128,
                         		     GDK_RGB_DITHER_NONE,
                         		     0,
                         		     0);
 
}

static gint 
timer_cb (EyesApplet *eyes_applet)
{
        gint x, y;
        gint pupil_x, pupil_y;
        gint i;
        
        for (i = 0; i < eyes_applet->num_eyes; i++) {
		if (GTK_WIDGET_REALIZED (eyes_applet->eyes[i])) {
			gdk_window_get_pointer (eyes_applet->eyes[i]->window, 
						&x, &y, NULL);
			calculate_pupil_xy (eyes_applet, x, y, &pupil_x, &pupil_y);
			draw_eye (eyes_applet, i, pupil_x, pupil_y);
		}
        }
        
        return TRUE;
}

static void
about_cb (BonoboUIComponent *uic,
	  EyesApplet        *eyes_applet,
	  const gchar       *verbname)
{
	static GtkWidget *about = NULL;
	GdkPixbuf	 *pixbuf;
	GError		 *error = NULL;
	gchar		 *file;
        
        static const gchar *authors [] = {
		"Dave Camp <campd@oit.edu>",
		NULL
	};

	const gchar *documenters[] = {
		NULL
	};

	const gchar *translator_credits = _("translator_credits");

	if (about) {
		gtk_window_present (GTK_WINDOW (about));
		return;
	}
        
	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gnome-eyes.png", FALSE, NULL);
	pixbuf = gdk_pixbuf_new_from_file (file, &error);
	g_free (file);
	
	if (error) {
		g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
		g_error_free (error);
	}
	
        about = gnome_about_new (
		_("Geyes"), VERSION,
		_("Copyright (C) 1999 Dave Camp"),
		_("A goofy little xeyes clone for the GNOME panel."),
		authors,
		documenters,
		strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
		pixbuf);
		
	if (pixbuf)
		gdk_pixbuf_unref (pixbuf);
			
	gtk_window_set_wmclass (GTK_WINDOW (about), "geyes", "Geyes");
	g_signal_connect (about, "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &about);
        gtk_widget_show (about);
}

static void
properties_load (EyesApplet *eyes_applet)
{
        gchar *theme_path = NULL;

	theme_path = panel_applet_gconf_get_string (
		eyes_applet->applet, "theme_path", NULL);

	if (theme_path == NULL)
		theme_path = g_strdup (GEYES_THEMES_DIR "Default-tiny");
	
        load_theme (eyes_applet, theme_path);

        g_free (theme_path);
}

void
setup_eyes (EyesApplet *eyes_applet) 
{
	int i;

        eyes_applet->hbox = gtk_hbox_new (FALSE, 0);
        gtk_fixed_put (GTK_FIXED (eyes_applet->fixed), eyes_applet->hbox, 0, 0);

        for (i = 0; i < eyes_applet->num_eyes; i++) {
                eyes_applet->eyes[i] = gtk_drawing_area_new ();
                
                if (eyes_applet->eyes[i] == NULL)
                        g_error ("Error creating geyes\n");
               
		gtk_widget_set_size_request (GTK_WIDGET (eyes_applet->eyes[i]),
					     eyes_applet->eye_width,
					     eyes_applet->eye_height);
 
                gtk_widget_show (eyes_applet->eyes[i]);
                
		gtk_box_pack_start (GTK_BOX (eyes_applet->hbox), 
                                    eyes_applet->eyes [i],
                                    TRUE,
                                    TRUE,
                                    0);
                
                gtk_widget_realize (eyes_applet->eyes[i]);
		draw_eye (eyes_applet, i,
			  eyes_applet->eye_width / 2,
                          eyes_applet->eye_height / 2);
                
        }
        gtk_widget_show (eyes_applet->hbox);
}

void
destroy_eyes (EyesApplet *eyes_applet)
{
	gtk_widget_destroy (eyes_applet->hbox);
	eyes_applet->hbox = NULL;
}

static EyesApplet *
create_eyes (PanelApplet *applet)
{
	EyesApplet *eyes_applet = g_new0 (EyesApplet, 1);

        eyes_applet->applet = applet;
        eyes_applet->fixed  = gtk_fixed_new ();

	gtk_container_add (GTK_CONTAINER (applet), eyes_applet->fixed);

        properties_load (eyes_applet);

        setup_eyes (eyes_applet);

	return eyes_applet;
}

static void
destroy_cb (GtkObject *object, EyesApplet *eyes_applet)
{
	g_return_if_fail (eyes_applet);

	gtk_timeout_remove (eyes_applet->timeout_id);
	if (eyes_applet->tooltips)
		g_object_unref (eyes_applet->tooltips);
	eyes_applet->timeout_id = 0;
	if (eyes_applet->eye_image)
		g_object_unref (eyes_applet->eye_image);
	eyes_applet->eye_image = NULL;
	if (eyes_applet->pupil_image)
		g_object_unref (eyes_applet->pupil_image);
	eyes_applet->pupil_image = NULL;
	if (eyes_applet->theme_dir)
		g_free (eyes_applet->theme_dir);
	eyes_applet->theme_dir = NULL;
	if (eyes_applet->theme_name)
		g_free (eyes_applet->theme_name);
	eyes_applet->theme_name = NULL;
	if (eyes_applet->eye_filename)
		g_free (eyes_applet->eye_filename);
	eyes_applet->eye_filename = NULL;
	if (eyes_applet->pupil_filename)
		g_free (eyes_applet->pupil_filename);
	eyes_applet->pupil_filename = NULL;
	
	g_free (eyes_applet);
}

static void
help_cb (BonoboUIComponent *uic,
	 EyesApplet        *eyes_applet,
	 const char        *verbname)
{
	GError *error = NULL;
	gnome_help_display("geyes",NULL,&error);

	if (error) {
		g_warning ("help error: %s\n", error->message);
		g_error_free (error);
		error = NULL;
	}
}


static const BonoboUIVerb geyes_applet_menu_verbs [] = {
        BONOBO_UI_UNSAFE_VERB ("Props", properties_cb),
        BONOBO_UI_UNSAFE_VERB ("Help", help_cb),
        BONOBO_UI_UNSAFE_VERB ("About", about_cb),

        BONOBO_UI_VERB_END
};

static void
set_atk_name_description (GtkWidget *widget, const gchar *name,
    const gchar *description)
{
	AtkObject *aobj;
   
	aobj = gtk_widget_get_accessible (widget);
	/* Check if gail is loaded */
	if (GTK_IS_ACCESSIBLE (aobj) == FALSE)
		return;

	atk_object_set_name (aobj, name);
	atk_object_set_description (aobj, description);
}

static gboolean
geyes_applet_fill (PanelApplet *applet)
{
	EyesApplet *eyes_applet;
	
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-eyes.png");
	
        eyes_applet = create_eyes (applet);

	panel_applet_add_preferences (applet, "/schemas/apps/geyes/prefs", NULL);

        eyes_applet->timeout_id = gtk_timeout_add (
		UPDATE_TIMEOUT, (GtkFunction) timer_cb, eyes_applet);
			
	panel_applet_setup_menu_from_file (eyes_applet->applet,
                                           NULL,
				           "GNOME_GeyesApplet.xml",
                                           NULL,
				           geyes_applet_menu_verbs,
				           eyes_applet);

	eyes_applet->tooltips = gtk_tooltips_new ();
	g_object_ref (eyes_applet->tooltips);
	gtk_object_sink (GTK_OBJECT (eyes_applet->tooltips));
	gtk_tooltips_set_tip (eyes_applet->tooltips, GTK_WIDGET (eyes_applet->applet), _("Geyes"), NULL);

	set_atk_name_description (GTK_WIDGET (eyes_applet->applet), _("Geyes"), 
			_("The eyes look in the direction of the mouse pointer"));

	gtk_widget_show_all (GTK_WIDGET (eyes_applet->applet));

	g_signal_connect (eyes_applet->applet,
			  "change_background",
			  G_CALLBACK (applet_back_change),
			  eyes_applet);
	g_signal_connect (eyes_applet->fixed,
			  "destroy",
			  G_CALLBACK (destroy_cb),
			  eyes_applet);
	  
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
			     PANEL_TYPE_APPLET,
			     "geyes",
			     "0",
			     geyes_applet_factory,
			     NULL)
