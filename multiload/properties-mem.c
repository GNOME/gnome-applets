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

#include "memload.h"
#include "properties-mem.h"

static GtkWidget *propbox=NULL;
static memload_properties temp_props;

/* extern GtkWidget *disp; */
extern memload_properties mem_props;

void
load_mem_properties (char *path, memload_properties *prop)
{
	gnome_config_push_prefix (path);
	prop->ucolor	= gnome_config_get_string ("mem/ucolor=#20b2aa");
	prop->scolor	= gnome_config_get_string ("mem/scolor=#188982");
	prop->bcolor	= gnome_config_get_string ("mem/bcolor=#ff0000");
	prop->ccolor	= gnome_config_get_string ("mem/ccolor=#00ff00");
	prop->speed	= gnome_config_get_int    ("mem/speed=2000");
	prop->height 	= gnome_config_get_int	  ("mem/height=40");
	prop->width 	= gnome_config_get_int	  ("mem/width=40");
	prop->look	= gnome_config_get_bool   ("mem/look=1");
	gnome_config_pop_prefix ();
}

void
save_mem_properties (char *path, memload_properties *prop)
{
	gnome_config_push_prefix (path);
	gnome_config_set_string ("mem/ucolor", prop->ucolor);
	gnome_config_set_string ("mem/scolor", prop->scolor);
	gnome_config_set_string ("mem/bcolor", prop->bcolor);
	gnome_config_set_string ("mem/ccolor", prop->ccolor);
	gnome_config_set_int    ("mem/speed", prop->speed);
	gnome_config_set_int    ("mem/height", prop->height);
	gnome_config_set_int    ("mem/width", prop->width);
	gnome_config_set_bool   ("mem/look", prop->look);
	gnome_config_pop_prefix ();
        gnome_config_sync ();
	gnome_config_drop_all ();
}

static void
ucolor_changed_cb (GnomeColorPicker *widget)
{
 	guint8 r,g,b;

        gnome_color_picker_get_i8
		(widget, &r, &g, &b, NULL);

	sprintf (temp_props.ucolor, "#%02x%02x%02x", r, g, b);
        gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}          

static void
scolor_changed_cb (GnomeColorPicker *widget)
{
 	guint8 r,g,b;

        gnome_color_picker_get_i8
		(widget, &r, &g, &b, NULL);
	
	sprintf (temp_props.scolor, "#%02x%02x%02x", r, g, b);
        gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}          
static void
bcolor_changed_cb (GnomeColorPicker *widget)
{
 	guint8 r,g,b;

        gnome_color_picker_get_i8
		(widget, &r, &g, &b, NULL);
	
	sprintf (temp_props.bcolor, "#%02x%02x%02x", r, g, b);
        gnome_property_box_changed (GNOME_PROPERTY_BOX(propbox));
}

static void
ccolor_changed_cb (GnomeColorPicker *widget)
{
 	guint8 r,g,b;

        gnome_color_picker_get_i8
		(widget, &r, &g, &b, NULL);
	
	sprintf (temp_props.ccolor, "#%02x%02x%02x", r, g, b);
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
	GtkWidget *box, *color_a, *color_b, *size, *speed;
	GtkWidget *height, *width, *freq;
	GtkObject *height_a, *width_a, *freq_a;
	        
	GtkWidget *ucolor_gcs, *scolor_gcs;
	GtkWidget *bcolor_gcs, *ccolor_gcs;
        int ur,ug,ub, sr,sg,sb, br,bg,bb, cr,cg,cb;

	sscanf (temp_props.ucolor, "#%02x%02x%02x", &ur, &ug, &ub);
        sscanf (temp_props.scolor, "#%02x%02x%02x", &sr, &sg, &sb);
	sscanf (temp_props.bcolor, "#%02x%02x%02x", &br, &bg, &bb);
	sscanf (temp_props.ccolor, "#%02x%02x%02x", &cr, &cg, &cb);
        
	box     = gtk_vbox_new (5, TRUE);
	color_a = gtk_hbox_new (5, TRUE);
	color_b = gtk_hbox_new (5, TRUE);
	size    = gtk_hbox_new (5, TRUE);
	speed   = gtk_hbox_new (5, TRUE);
	gtk_container_set_border_width (GTK_CONTAINER(box), 5);
	        
	ucolor_gcs  = gnome_color_picker_new ();
	scolor_gcs  = gnome_color_picker_new ();
	bcolor_gcs  = gnome_color_picker_new ();
	ccolor_gcs  = gnome_color_picker_new ();

        gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (ucolor_gcs), 
				   ur, ug, ub, 255);
	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (scolor_gcs), 
				   sr, sg, sb, 255);
        gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (bcolor_gcs), 
				   br, bg, bb, 255);
	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (ccolor_gcs), 
				   cr, cg, cb, 255);
                  

	label = gtk_label_new (_("Used Memory"));
	gtk_box_pack_start_defaults (GTK_BOX (color_a), label);
	gtk_box_pack_start_defaults (GTK_BOX (color_a),	ucolor_gcs);

	label = gtk_label_new (_("Shared Memory"));
	gtk_box_pack_start_defaults (GTK_BOX (color_a), label);
	gtk_box_pack_start_defaults (GTK_BOX (color_a), scolor_gcs);

	label = gtk_label_new (_("Buffered Memory"));
	gtk_box_pack_start_defaults (GTK_BOX (color_b), label);
	gtk_box_pack_start_defaults (GTK_BOX (color_b), bcolor_gcs);

	label = gtk_label_new (_("Cached Memory"));
	gtk_box_pack_start_defaults (GTK_BOX (color_b), label);
	gtk_box_pack_start_defaults (GTK_BOX (color_b),	ccolor_gcs);
	
	gtk_signal_connect (GTK_OBJECT (ucolor_gcs), "color_set",
			    GTK_SIGNAL_FUNC (ucolor_changed_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (scolor_gcs), "color_set",
			    GTK_SIGNAL_FUNC (scolor_changed_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (bcolor_gcs), "color_set",
			    GTK_SIGNAL_FUNC (bcolor_changed_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (ccolor_gcs), "color_set",
			    GTK_SIGNAL_FUNC (ccolor_changed_cb), NULL);

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
        gtk_signal_connect (GTK_OBJECT (height), "changed",
			    GTK_SIGNAL_FUNC (height_cb), height);
        gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (height),
					   GTK_UPDATE_ALWAYS);
        gtk_signal_connect (GTK_OBJECT (width_a), "value_changed",
			    GTK_SIGNAL_FUNC (width_cb), width);
        gtk_signal_connect (GTK_OBJECT (width), "changed",
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
        gtk_signal_connect (GTK_OBJECT (freq), "changed",
			    GTK_SIGNAL_FUNC (freq_cb), freq);
        gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (freq),
					   GTK_UPDATE_ALWAYS);
        
        gtk_box_pack_start_defaults (GTK_BOX (box), color_a);
        gtk_box_pack_start_defaults (GTK_BOX (box), color_b);
	gtk_box_pack_start_defaults (GTK_BOX (box), size);
	gtk_box_pack_start_defaults (GTK_BOX (box), speed);
	
	gtk_widget_show_all (box);
	return box;
}

static void
apply_cb (GtkWidget *widget, gint dummy, gpointer data)
{
	Memload *c = (Memload *) data;
	
	memcpy (&mem_props, &temp_props, sizeof (memload_properties));

	memload_setup_colors (c);
	memload_start_timer (c);

	/*FIXME: this won't make the window smaller*/
	gtk_widget_set_usize (c->disp, mem_props.width, mem_props.height);
}

static gint
destroy_cb (GtkWidget *widget, gpointer data)
{
	propbox = NULL;
	return FALSE;
}

void
mem_properties (AppletWidget *applet, gpointer data)
{
	GtkWidget *frame, *label;

	if (propbox) {
		gdk_window_raise (propbox->window);
		return;
	}

	memcpy (&temp_props, &mem_props, sizeof (memload_properties));

        propbox = gnome_property_box_new ();
	gtk_window_set_title
		(GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window),
		 _("MemLoad Settings"));
	
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
