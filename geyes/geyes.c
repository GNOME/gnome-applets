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
#include <egg-screen-help.h>
#include "geyes.h"

#define UPDATE_TIMEOUT 100

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
                applet_set_back_pixmap (eyes_applet->vbox, pixmap);
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
        double temp;
     
        nx = x - ((double) eyes_applet->eye_width / 2);
        ny = y - ((double) eyes_applet->eye_height / 2);
       
        h = hypot (nx, ny);
        if (abs (h) 
            < (abs (hypot (eyes_applet->eye_height / 2, eyes_applet->eye_width / 2)) - eyes_applet->wall_thickness - eyes_applet->pupil_height)) {
                *pupil_x = x;
                *pupil_y = y;
                return;
        }
        
        angle = atan2 (nx, ny);
        sina = sin (angle);
        cosa = cos (angle);
        
        temp = hypot ((eyes_applet->eye_height / 2) * cosa, (eyes_applet->eye_width / 2)* sina);
        *pupil_x = temp * sina;
        *pupil_y = temp * cosa;
       
        temp = hypot ((eyes_applet->pupil_width / 2) * sina, (eyes_applet->pupil_height / 2)* cosa);
        *pupil_x -= temp * sina;
        *pupil_y -= temp * cosa;
        
        temp = hypot ((eyes_applet->wall_thickness / 2) * sina, (eyes_applet->wall_thickness / 2) * cosa);
        *pupil_x -= temp * sina;
        *pupil_y -= temp * cosa;
        
        *pupil_x += (eyes_applet->eye_width / 2);
        *pupil_y += (eyes_applet->eye_height / 2);
}

static void 
draw_eye (EyesApplet *eyes_applet,
	  gint eye_num, 
          gint pupil_x, 
          gint pupil_y)
{
	GdkPixbuf *pixbuf;
	GdkRectangle rect, r1, r2;

	pixbuf = gdk_pixbuf_copy (eyes_applet->eye_image);
	r1.x = pupil_x - eyes_applet->pupil_width / 2;
	r1.y = pupil_y - eyes_applet->pupil_height / 2;
	r1.width = eyes_applet->pupil_width;
	r1.height = eyes_applet->pupil_height;
	r2.x = 0;
	r2.y = 0;
	r2.width = eyes_applet->eye_width;
	r2.height = eyes_applet->eye_height;
	gdk_rectangle_intersect (&r1, &r2, &rect);
	gdk_pixbuf_composite (eyes_applet->pupil_image, pixbuf, 
					   rect.x,
					   rect.y,
					   rect.width,
				      	   rect.height,
				      	   pupil_x - eyes_applet->pupil_width / 2,
					   pupil_y - eyes_applet->pupil_height / 2, 1.0, 1.0,
				      	   GDK_INTERP_BILINEAR,
				           255);
	gtk_image_set_from_pixbuf (GTK_IMAGE (eyes_applet->eyes[eye_num]),
						  pixbuf);
	g_object_unref (pixbuf);

}

static gint 
timer_cb (EyesApplet *eyes_applet)
{
        gint x, y;
        gint pupil_x, pupil_y;
        gint i;
        
        for (i = 0; i < eyes_applet->num_eyes; i++) {
		if (GTK_WIDGET_REALIZED (eyes_applet->eyes[i])) {
			gtk_widget_get_pointer (eyes_applet->eyes[i], 
						&x, &y);
			if ((x != eyes_applet->pointer_last_x) || (y != eyes_applet->pointer_last_y)) { 
				calculate_pupil_xy (eyes_applet, x, y, &pupil_x, &pupil_y);
				draw_eye (eyes_applet, i, pupil_x, pupil_y);
			}
		}
        }
        eyes_applet->pointer_last_x = x;
        eyes_applet->pointer_last_y = y;
        return TRUE;
}

static void
about_cb (BonoboUIComponent *uic,
	  EyesApplet        *eyes_applet,
	  const gchar       *verbname)
{
	GdkPixbuf	 *pixbuf;
	GError		 *error = NULL;
	gchar		 *file;
        
        static const gchar *authors [] = {
		"Dave Camp <campd@oit.edu>",
		NULL
	};

	const gchar *documenters[] = {
                "Arjan Scherpenisse <acscherp@wins.uva.nl>",
                "Telsa Gwynne <hobbit@aloss.ukuu.org.uk>",
                "Sun GNOME Documentation Team <gdocteam@sun.com>",
		NULL
	};

	const gchar *translator_credits = _("translator_credits");

	if (eyes_applet->about_dialog) {
		gtk_window_present (GTK_WINDOW (eyes_applet->about_dialog));
		return;
	}
        
	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gnome-eyes.png", FALSE, NULL);
	pixbuf = gdk_pixbuf_new_from_file (file, &error);
	g_free (file);
	
	if (error) {
		g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
		g_error_free (error);
	}
	
        eyes_applet->about_dialog = gnome_about_new (
		_("Geyes"), VERSION,
		_("Copyright (C) 1999 Dave Camp"),
		_("A goofy little xeyes clone for the GNOME panel."),
		authors,
		documenters,
		strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
		pixbuf);
		
	if (pixbuf)
		gdk_pixbuf_unref (pixbuf);
			
	gtk_window_set_wmclass (GTK_WINDOW (eyes_applet->about_dialog), "geyes", "Geyes");
	gtk_window_set_screen (GTK_WINDOW (eyes_applet->about_dialog),
			       gtk_widget_get_screen (GTK_WIDGET (eyes_applet->applet)));
	g_signal_connect (eyes_applet->about_dialog, "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &eyes_applet->about_dialog);
	gtk_widget_show (eyes_applet->about_dialog);
}

static int
properties_load (EyesApplet *eyes_applet)
{
        gchar *theme_path = NULL;

	theme_path = panel_applet_gconf_get_string (
		eyes_applet->applet, "theme_path", NULL);

	if (theme_path == NULL)
		theme_path = g_strdup (GEYES_THEMES_DIR "Default-tiny");
	
        if (load_theme (eyes_applet, theme_path) == FALSE) {
		g_free (theme_path);

		return FALSE;
	}

        g_free (theme_path);
	
	return TRUE;
}

void
setup_eyes (EyesApplet *eyes_applet) 
{
	int i;

        eyes_applet->hbox = gtk_hbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (eyes_applet->vbox), eyes_applet->hbox, TRUE, TRUE, 0);

	eyes_applet->eyes = g_new0 (GtkWidget *, eyes_applet->num_eyes);

        for (i = 0; i < eyes_applet->num_eyes; i++) {
                eyes_applet->eyes[i] = gtk_image_new ();
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
                
		if ((eyes_applet->num_eyes != 1) && (i == 0)) {
                	gtk_misc_set_alignment (GTK_MISC (eyes_applet->eyes[i]), 1.0, 0.5);
		}
		else if ((eyes_applet->num_eyes != 1) && (i == eyes_applet->num_eyes - 1)) {
			gtk_misc_set_alignment (GTK_MISC (eyes_applet->eyes[i]), 0.0, 0.5);
		}
		else {
			gtk_misc_set_alignment (GTK_MISC (eyes_applet->eyes[i]), 0.5, 0.5);
		}
		
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

	g_free (eyes_applet->eyes);
}

static EyesApplet *
create_eyes (PanelApplet *applet)
{
	EyesApplet *eyes_applet = g_new0 (EyesApplet, 1);

        eyes_applet->applet = applet;
        eyes_applet->vbox = gtk_vbox_new (FALSE, 0);

	gtk_container_add (GTK_CONTAINER (applet), eyes_applet->vbox);

	return eyes_applet;
}

static void
destroy_cb (GtkObject *object, EyesApplet *eyes_applet)
{
	g_return_if_fail (eyes_applet);

	gtk_timeout_remove (eyes_applet->timeout_id);
	if (eyes_applet->hbox)
		destroy_eyes (eyes_applet);
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
	
	if (eyes_applet->about_dialog)
	  	gtk_widget_destroy (eyes_applet->about_dialog);

	if (eyes_applet->prop_box.pbox)
	  	gtk_widget_destroy (eyes_applet->prop_box.pbox);

	g_free (eyes_applet);
}

static void
help_cb (BonoboUIComponent *uic,
	 EyesApplet        *eyes_applet,
	 const char        *verbname)
{
	GError *error = NULL;

	egg_help_display_on_screen (
		"geyes", NULL,
		gtk_widget_get_screen (GTK_WIDGET (eyes_applet->applet)),
		&error);

	if (error) { /* FIXME: the user needs to see this */
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
	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
	
        eyes_applet = create_eyes (applet);

	panel_applet_add_preferences (applet, "/schemas/apps/geyes/prefs", NULL);

	eyes_applet->about_dialog = NULL;

        eyes_applet->timeout_id = gtk_timeout_add (
		UPDATE_TIMEOUT, (GtkFunction) timer_cb, eyes_applet);
			
	panel_applet_setup_menu_from_file (eyes_applet->applet,
                                           NULL,
				           "GNOME_GeyesApplet.xml",
                                           NULL,
				           geyes_applet_menu_verbs,
				           eyes_applet);

	if (panel_applet_get_locked_down (eyes_applet->applet)) {
		BonoboUIComponent *popup_component;

		popup_component = panel_applet_get_popup_component (eyes_applet->applet);

		bonobo_ui_component_set_prop (popup_component,
					      "/commands/Props",
					      "hidden", "1",
					      NULL);
	}

	eyes_applet->tooltips = gtk_tooltips_new ();
	g_object_ref (eyes_applet->tooltips);
	gtk_object_sink (GTK_OBJECT (eyes_applet->tooltips));
	gtk_tooltips_set_tip (eyes_applet->tooltips, GTK_WIDGET (eyes_applet->applet), _("Geyes"), NULL);

	set_atk_name_description (GTK_WIDGET (eyes_applet->applet), _("Geyes"), 
			_("The eyes look in the direction of the mouse pointer"));

	g_signal_connect (eyes_applet->applet,
			  "change_background",
			  G_CALLBACK (applet_back_change),
			  eyes_applet);
	g_signal_connect (eyes_applet->vbox,
			  "destroy",
			  G_CALLBACK (destroy_cb),
			  eyes_applet);

	gtk_widget_show_all (GTK_WIDGET (eyes_applet->applet));

	/* setup here and not in create eyes so the destroy signal is set so 
	 * that when there is an error within loading the theme
	 * we can emit this signal */
        if (properties_load (eyes_applet) == FALSE)
		return FALSE;

	setup_eyes (eyes_applet);

	return TRUE;
}

static gboolean
geyes_applet_factory (PanelApplet *applet,
		      const gchar *iid,
		      gpointer     data)
{
	gboolean retval = FALSE;

	theme_dirs_create ();

	if (!strcmp (iid, "OAFIID:GNOME_GeyesApplet"))
		retval = geyes_applet_fill (applet); 
   
	if (retval == FALSE) {
		exit (-1);
	}

	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_GeyesApplet_Factory",
			     PANEL_TYPE_APPLET,
			     "geyes",
			     "0",
			     geyes_applet_factory,
			     NULL)
