/* GNOME cpuload/memload panel applet - properties dialog
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Tim P. Gerla
 *          Martin Baulig
 *
 */

#include <stdio.h>
#include <string.h>
#include <config.h>
#include <gnome.h>

#include <assert.h>

#include "swapload.h"
#include "properties-swap.h"

static GtkWidget *propbox=NULL;
static swapload_properties temp_props;

// extern GtkWidget *disp;
extern swapload_properties swap_props;

void
load_swap_properties (char *path, swapload_properties *prop)
{
	gnome_config_push_prefix (path);
	prop->ucolor	= gnome_config_get_string ("swap/ucolor=#ff0000");
	prop->fcolor	= gnome_config_get_string ("swap/fcolor=#000000");
	prop->speed	= gnome_config_get_int    ("swap/speed=2000");
	prop->height 	= gnome_config_get_int	  ("swap/height=40");
	prop->width 	= gnome_config_get_int	  ("swap/width=40");
	prop->look	= gnome_config_get_bool   ("swap/look=1");
	gnome_config_pop_prefix ();
}

void
save_swap_properties (char *path, swapload_properties *prop)
{
	gnome_config_push_prefix (path);
	gnome_config_set_string ("swap/ucolor", prop->ucolor);
	gnome_config_set_string ("swap/fcolor", prop->fcolor);
	gnome_config_set_int    ("swap/speed", prop->speed);
	gnome_config_set_int    ("swap/height", prop->height);
	gnome_config_set_int    ("swap/width", prop->width);
	gnome_config_set_bool   ("swap/look", prop->look);
	gnome_config_pop_prefix ();
        gnome_config_sync ();
	gnome_config_drop_all ();
}

static void
color_changed_cb (GnomeColorSelector *widget, gchar **color)
{
        char *tmp;
 	int r,g,b;

	tmp = g_malloc (24);
        gnome_color_selector_get_color_int
		(widget, &r, &g, &b, 255);
	
	sprintf (tmp, "#%02x%02x%02x", r, g, b);
        *color = tmp;
        gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}          

static void
height_cb (GtkWidget *widget, GtkWidget *spin)
{
	temp_props.height =
		gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}

static void
width_cb (GtkWidget *widget, GtkWidget *spin)
{
	temp_props.width =
		gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}

static void
freq_cb (GtkWidget *widget, GtkWidget *spin)
{
	temp_props.speed = gtk_spin_button_get_value_as_float
		(GTK_SPIN_BUTTON(spin)) * 1000;
        gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}	

static GtkWidget *
create_frame (void)
{
	GtkWidget *label;
	GtkWidget *box, *color, *size, *speed;
	GtkWidget *height, *width, *freq;
	GtkObject *height_a, *width_a, *freq_a;
	        
	GnomeColorSelector *ucolor_gcs, *fcolor_gcs;
        int ur,ug,ub, sr,sg,sb;

	sscanf (temp_props.ucolor, "#%02x%02x%02x", &ur, &ug, &ub);
        sscanf (temp_props.fcolor, "#%02x%02x%02x", &sr, &sg, &sb);
        
	box   = gtk_vbox_new (5, TRUE);
	color = gtk_hbox_new (5, TRUE);
	size  = gtk_hbox_new (5, TRUE);
	speed = gtk_hbox_new (5, TRUE);
	gtk_container_border_width (GTK_CONTAINER(box), 5);
	        
	
	ucolor_gcs  = gnome_color_selector_new
		((SetColorFunc)color_changed_cb, &temp_props.ucolor);
	fcolor_gcs = gnome_color_selector_new
		((SetColorFunc)color_changed_cb, &temp_props.fcolor );

        gnome_color_selector_set_color_int (ucolor_gcs, ur, ug, ub, 255);
	gnome_color_selector_set_color_int (fcolor_gcs, sr, sg, sb, 255);
                  

	label = gtk_label_new (_("Used Swap"));
	gtk_box_pack_start_defaults
		(GTK_BOX (color), label);
	gtk_box_pack_start_defaults
		(GTK_BOX (color),
		 gnome_color_selector_get_button (ucolor_gcs));

	label = gtk_label_new (_("Free Swap"));
	gtk_box_pack_start_defaults
		(GTK_BOX(color), label);
	gtk_box_pack_start_defaults
		(GTK_BOX(color),
		 gnome_color_selector_get_button (fcolor_gcs));

	label = gtk_label_new (_("Applet Height"));
	height_a = gtk_adjustment_new
		(temp_props.height, 0.5, 128, 1, 8, 8);
	height  = gtk_spin_button_new
		(GTK_ADJUSTMENT (height_a), 1, 0);
	gtk_box_pack_start_defaults (GTK_BOX (size), label);
	gtk_box_pack_start_defaults (GTK_BOX (size), height);
	
	label = gtk_label_new (_("Width"));
	width_a = gtk_adjustment_new
		(temp_props.width, 0.5, 128, 1, 8, 8);
	width  = gtk_spin_button_new (GTK_ADJUSTMENT (width_a), 1, 0);
	gtk_box_pack_start_defaults (GTK_BOX (size), label);
	gtk_box_pack_start_defaults (GTK_BOX (size), width);

        gtk_signal_connect (GTK_OBJECT (height_a), "value_changed",
			    GTK_SIGNAL_FUNC (height_cb), height);
        gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (height),
					   GTK_UPDATE_ALWAYS);
        gtk_signal_connect (GTK_OBJECT (width_a), "value_changed",
			    GTK_SIGNAL_FUNC (width_cb), width);
        gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (width),
					   GTK_UPDATE_ALWAYS);

	label = gtk_label_new (_("Update Frequency"));
	g_print ("%d %d\n", temp_props.speed, temp_props.speed/1000);
	freq_a = gtk_adjustment_new
		((float)temp_props.speed/1000, 0.1, 60, 0.1, 5, 5);
	freq  = gtk_spin_button_new (GTK_ADJUSTMENT (freq_a), 0.1, 1);
	gtk_box_pack_start (GTK_BOX (speed), label, TRUE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (speed), freq,  TRUE, TRUE, 0);
	
        gtk_signal_connect (GTK_OBJECT (freq_a), "value_changed",
			    GTK_SIGNAL_FUNC (freq_cb), freq);
        gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (freq),
					   GTK_UPDATE_ALWAYS);
        
        gtk_box_pack_start_defaults (GTK_BOX (box), color);
	gtk_box_pack_start_defaults (GTK_BOX (box), size);
	gtk_box_pack_start_defaults (GTK_BOX (box), speed);
	
	gtk_widget_show_all (box);
	return box;
}

static void
apply_cb (GtkWidget *widget, gint dummy, gpointer data)
{
	Swapload *c = (Swapload *) data;
	
	memcpy (&swap_props, &temp_props, sizeof (swapload_properties));

	swapload_setup_colors (c);
	swapload_start_timer (c);

	/*FIXME: this won't make the window smaller*/
	gtk_widget_set_usize (c->disp, swap_props.width, swap_props.height);
}

static gint
destroy_cb (GtkWidget *widget, gpointer data)
{
	propbox = NULL;
	return FALSE;
}

void
swap_properties (AppletWidget *applet, gpointer data)
{
	GtkWidget *frame, *label;

	if (propbox) {
		gdk_window_raise (propbox->window);
		return;
	}

	memcpy (&temp_props, &swap_props, sizeof (swapload_properties));

        propbox = gnome_property_box_new ();
	gtk_window_set_title
		(GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window),
		 _("SwapLoad Settings"));
	
	frame = create_frame ();
	label = gtk_label_new (_("General"));
        gtk_widget_show (frame);
	gtk_notebook_append_page
		(GTK_NOTEBOOK(GNOME_PROPERTY_BOX(propbox)->notebook),
		 frame, label);

        gtk_signal_connect (GTK_OBJECT (propbox), "apply",
			    GTK_SIGNAL_FUNC (apply_cb), data);

        gtk_signal_connect (GTK_OBJECT (propbox), "destroy",
			    GTK_SIGNAL_FUNC (destroy_cb), data);

	gtk_widget_show_all (propbox);
}
