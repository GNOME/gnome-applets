/*
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "geyes-applet.h"

#include <math.h>
#include <stdlib.h>
#include <glib/gi18n-lib.h>

#include "geyes-applet-private.h"

#define UPDATE_TIMEOUT 100

G_DEFINE_TYPE (EyesApplet, eyes_applet, GP_TYPE_APPLET)

static gfloat
gtk_align_to_gfloat (GtkAlign align)
{
	switch (align) {
		case GTK_ALIGN_START:
			return 0.0;
		case GTK_ALIGN_END:
			return 1.0;
		case GTK_ALIGN_CENTER:
		case GTK_ALIGN_FILL:
			return 0.5;
		case GTK_ALIGN_BASELINE:
		default:
			return 0.0;
	}
}

/* TODO - Optimize this a bit */
static void 
calculate_pupil_xy (EyesApplet *eyes_applet,
		    gint x, gint y,
		    gint *pupil_x, gint *pupil_y, GtkWidget* widget)
{
        GtkAllocation allocation;
        double sina;
        double cosa;
        double h;
        double temp;
 	 double nx, ny;

	 gfloat xalign, yalign;
	 gint width, height;

	 gtk_widget_get_allocation (GTK_WIDGET(widget), &allocation);
	 width = allocation.width;
	 height = allocation.height;

	xalign = gtk_align_to_gfloat (gtk_widget_get_halign (widget));
	yalign = gtk_align_to_gfloat (gtk_widget_get_valign (widget));

	 nx = x - MAX(width - eyes_applet->eye_width, 0) * xalign - eyes_applet->eye_width / 2;
	 ny = y - MAX(height- eyes_applet->eye_height, 0) * yalign - eyes_applet->eye_height / 2;
  
	 h = hypot (nx, ny);
        if (h < 0.5 || fabs (h)
            < (fabs (hypot (eyes_applet->eye_height / 2, eyes_applet->eye_width / 2)) - eyes_applet->wall_thickness - eyes_applet->pupil_height)) {
                *pupil_x = nx + eyes_applet->eye_width / 2;
                *pupil_y = ny + eyes_applet->eye_height / 2;
                return;
        }
        
	 sina = nx / h; 
	 cosa = ny / h;
	
        temp = hypot ((eyes_applet->eye_width / 2) * sina, (eyes_applet->eye_height / 2) * cosa);
        temp -= hypot ((eyes_applet->pupil_width / 2) * sina, (eyes_applet->pupil_height / 2) * cosa);
        temp -= hypot ((eyes_applet->wall_thickness / 2) * sina, (eyes_applet->wall_thickness / 2) * cosa);

        *pupil_x = temp * sina + (eyes_applet->eye_width / 2);
        *pupil_y = temp * cosa + (eyes_applet->eye_height / 2);
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
        GdkDisplay *display;
        GdkDeviceManager *device_manager;
        GdkDevice *device;
        gint x, y;
        gint pupil_x, pupil_y;
        gint i;

        display = gtk_widget_get_display (GTK_WIDGET (eyes_applet));
        device_manager = gdk_display_get_device_manager (display);
        device = gdk_device_manager_get_client_pointer (device_manager);

        for (i = 0; i < eyes_applet->num_eyes; i++) {
		if (gtk_widget_get_realized (eyes_applet->eyes[i])) {
			gdk_window_get_device_position (gtk_widget_get_window (eyes_applet->eyes[i]), device, &x, &y, NULL);
			if ((x != eyes_applet->pointer_last_x[i]) || (y != eyes_applet->pointer_last_y[i])) { 

				calculate_pupil_xy (eyes_applet, x, y, &pupil_x, &pupil_y, eyes_applet->eyes[i]);
				draw_eye (eyes_applet, i, pupil_x, pupil_y);
	    	        
			        eyes_applet->pointer_last_x[i] = x;
			        eyes_applet->pointer_last_y[i] = y;
			}
		}
        }
        return TRUE;
}

static void
about_cb (GSimpleAction *action,
          GVariant      *parameter,
          gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static int
properties_load (EyesApplet *eyes_applet)
{
        gchar *theme_path = NULL;

	theme_path = g_settings_get_string (eyes_applet->settings, KEY_THEME_PATH);
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

        eyes_applet->hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start (GTK_BOX (eyes_applet->vbox), eyes_applet->hbox, TRUE, TRUE, 0);

	eyes_applet->eyes = g_new0 (GtkWidget *, eyes_applet->num_eyes);
	eyes_applet->pointer_last_x = g_new0 (gint, eyes_applet->num_eyes);
	eyes_applet->pointer_last_y = g_new0 (gint, eyes_applet->num_eyes);

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
			gtk_widget_set_halign (eyes_applet->eyes[i], GTK_ALIGN_END);
			gtk_widget_set_valign (eyes_applet->eyes[i], GTK_ALIGN_CENTER);
		}
		else if ((eyes_applet->num_eyes != 1) && (i == eyes_applet->num_eyes - 1)) {
			gtk_widget_set_halign (eyes_applet->eyes[i], GTK_ALIGN_START);
			gtk_widget_set_valign (eyes_applet->eyes[i], GTK_ALIGN_CENTER);
		}
		else {
			gtk_widget_set_halign (eyes_applet->eyes[i], GTK_ALIGN_CENTER);
			gtk_widget_set_valign (eyes_applet->eyes[i], GTK_ALIGN_CENTER);
		}

		eyes_applet->pointer_last_x[i] = G_MAXINT;
		eyes_applet->pointer_last_y[i] = G_MAXINT;
		
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
	g_free (eyes_applet->pointer_last_x);
	g_free (eyes_applet->pointer_last_y);
}

static void
help_cb (GSimpleAction *action,
         GVariant      *parameter,
         gpointer       user_data)
{
  gp_applet_show_help (GP_APPLET (user_data), NULL);
}

static const GActionEntry geyes_applet_menu_actions [] = {
	{ "preferences", properties_cb, NULL, NULL, NULL },
	{ "help",        help_cb,       NULL, NULL, NULL },
	{ "about",       about_cb,      NULL, NULL, NULL },
	{ NULL }
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

static void
eyes_applet_fill (EyesApplet *eyes_applet)
{
	const char *menu_resource;
	GAction *action;

	eyes_applet->vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (eyes_applet), eyes_applet->vbox);

	eyes_applet->settings = gp_applet_settings_new (GP_APPLET (eyes_applet), GEYES_PREFS_SCHEMA);

        eyes_applet->timeout_id = g_timeout_add (
		UPDATE_TIMEOUT, (GSourceFunc) timer_cb, eyes_applet);

	menu_resource = GRESOURCE_PREFIX "/ui/geyes-applet-menu.ui";
	gp_applet_setup_menu_from_resource (GP_APPLET (eyes_applet),
	                                    menu_resource,
	                                    geyes_applet_menu_actions);

	action = gp_applet_menu_lookup_action (GP_APPLET (eyes_applet), "preferences");
	g_object_bind_property (eyes_applet, "locked-down",
	                        action, "enabled",
	                        G_BINDING_DEFAULT|G_BINDING_INVERT_BOOLEAN|G_BINDING_SYNC_CREATE);

	gtk_widget_set_tooltip_text (GTK_WIDGET (eyes_applet), _("Eyes"));

	set_atk_name_description (GTK_WIDGET (eyes_applet), _("Eyes"), 
			_("The eyes look in the direction of the mouse pointer"));

	gtk_widget_show_all (GTK_WIDGET (eyes_applet));

	/* setup here and not in create eyes so the destroy signal is set so 
	 * that when there is an error within loading the theme
	 * we can emit this signal */
	if (!properties_load (eyes_applet))
		return;

	setup_eyes (eyes_applet);
}

static void
eyes_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (eyes_applet_parent_class)->constructed (object);
  eyes_applet_fill (EYES_APPLET (object));
}

static void
eyes_applet_dispose (GObject *object)
{
  EyesApplet *self;

  self = EYES_APPLET (object);

  if (self->timeout_id != 0)
    {
      g_source_remove (self->timeout_id);
      self->timeout_id = 0;
    }

  if (self->hbox != NULL)
    {
      destroy_eyes (self);
      self->hbox = NULL;
    }

  g_clear_object (&self->eye_image);
  g_clear_object (&self->pupil_image);

  g_clear_pointer (&self->theme_dir, g_free);
  g_clear_pointer (&self->theme_name, g_free);
  g_clear_pointer (&self->eye_filename, g_free);
  g_clear_pointer (&self->pupil_filename, g_free);

  g_clear_pointer (&self->prop_box.pbox, gtk_widget_destroy);

  g_clear_object (&self->settings);

  G_OBJECT_CLASS (eyes_applet_parent_class)->dispose (object);
}

static void
eyes_applet_class_init (EyesAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = eyes_applet_constructed;
  object_class->dispose = eyes_applet_dispose;
}

static void
eyes_applet_init (EyesApplet *self)
{
  gp_applet_set_flags (GP_APPLET (self), GP_APPLET_FLAGS_EXPAND_MINOR);

  theme_dirs_create ();
}

void
eyes_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **documenters;
  const char *copyright;

  comments = _("A goofy set of eyes for the GNOME panel. They "
               "follow your mouse.");

  authors = (const char *[])
    {
      "Dave Camp <campd@oit.edu>",
      NULL
    };

  documenters = (const char *[])
    {
      "Arjan Scherpenisse <acscherp@wins.uva.nl>",
      "Telsa Gwynne <hobbit@aloss.ukuu.org.uk>",
      "Sun GNOME Documentation Team <gdocteam@sun.com>",
      NULL
    };

  copyright = "\xC2\xA9 1999 Dave Camp";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
