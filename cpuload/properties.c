/* GNOME cpuload panel applet - properties dialog
 * (C) 1997 The Free Software Foundation
 *
 * Author: Tim P. Gerla
 *
 */

#include <stdio.h>
#include <string.h>
#include <config.h>
#include <gnome.h>

#include "properties.h"

GtkWidget *propbox=NULL;
static cpuload_properties temp_props;

extern GtkWidget *disp;
extern cpuload_properties props;

static const int max_rgb_str_len = 7;
static const int max_rgb_str_size = 8;

void load_properties( const char *path, cpuload_properties *prop )
{
	gnome_config_push_prefix (path);
	prop->ucolor	= gnome_config_get_string ("cpu/ucolor=#20b2aa");
	if (prop->ucolor && (strlen(prop->ucolor) < max_rgb_str_len))
	  prop->ucolor = g_realloc(prop->ucolor, max_rgb_str_size);

	prop->scolor	= gnome_config_get_string ("cpu/scolor=#188982");
	if (prop->scolor && (strlen(prop->scolor) < max_rgb_str_len))
	  prop->scolor = g_realloc(prop->scolor, max_rgb_str_size);

	prop->speed	= gnome_config_get_int    ("cpu/speed=2000");
	prop->height 	= gnome_config_get_int	  ("cpu/height=40");
	prop->width 	= gnome_config_get_int	  ("cpu/width=40");
	prop->look	= gnome_config_get_bool   ("cpu/look=1");
	gnome_config_pop_prefix ();
}

void save_properties( const char *path, cpuload_properties *prop )
{
	gnome_config_push_prefix (path);
	gnome_config_set_string( "cpu/ucolor", prop->ucolor );
	gnome_config_set_string( "cpu/scolor", prop->scolor );
	gnome_config_set_int   ( "cpu/speed", prop->speed );
	gnome_config_set_int   ( "cpu/height", prop->height );
	gnome_config_set_int   ( "cpu/width", prop->width );
	gnome_config_set_bool  ( "cpu/look", prop->look );
	gnome_config_pop_prefix ();
        gnome_config_sync();
	gnome_config_drop_all();
}

static void 
ucolor_changed_cb( GnomeColorPicker *widget)
{
 	guint8 r,g,b;

	/* FIXME ugh, mem leak..anyone have a better way of doing this? */
        /* FIXED! Greg */

        gnome_color_picker_get_i8 (widget, &r, &g, &b, NULL);
	
	g_snprintf( temp_props.ucolor, max_rgb_str_size, 
		    "#%02x%02x%02x", r, g, b );
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}          

static void 
scolor_changed_cb( GnomeColorPicker *widget)
{
 	guint8 r,g,b;

	/* FIXME ugh, mem leak..anyone have a better way of doing this? */
        /* FIXED! Greg */

        gnome_color_picker_get_i8 (widget, &r, &g, &b, NULL);
	
	g_snprintf( temp_props.scolor, max_rgb_str_size,
		    "#%02x%02x%02x", r, g, b );
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

static void
height_cb( GtkWidget *widget, GtkWidget *spin )
{
	temp_props.height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

static void
width_cb( GtkWidget *widget, GtkWidget *spin )
{
	temp_props.width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

static void
freq_cb( GtkWidget *widget, GtkWidget *spin )
{
	temp_props.speed = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spin))*1000;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}	

static GtkWidget *
create_frame(void)
{
	GtkWidget *label;
	GtkWidget *box, *color, *size, *speed;
	GtkWidget *height, *width, *freq;
	GtkObject *height_a, *width_a, *freq_a;
	        
	GtkWidget *ucolor_gcs, *scolor_gcs;
        int ur,ug,ub, sr,sg,sb;

	sscanf( temp_props.ucolor, "#%02x%02x%02x", &ur,&ug,&ub );
        sscanf( temp_props.scolor, "#%02x%02x%02x", &sr,&sg,&sb );
        
	box = gtk_vbox_new( 5, TRUE );
	color=gtk_hbox_new( 5, TRUE );
	size =gtk_hbox_new( 5, TRUE );
	speed=gtk_hbox_new( 5, TRUE );
	gtk_container_set_border_width( GTK_CONTAINER(box), 5 );
	        
	
	ucolor_gcs  = gnome_color_picker_new ();
	scolor_gcs  = gnome_color_picker_new ();

        gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (ucolor_gcs), 
				   ur, ug, ub, 255 );
	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (scolor_gcs), 
				   sr, sg, sb, 255 );
                  

	label = gtk_label_new(_("User Load"));
	gtk_box_pack_start_defaults (GTK_BOX(color), label);
	gtk_box_pack_start_defaults (GTK_BOX(color), ucolor_gcs); 
	gtk_signal_connect (GTK_OBJECT (ucolor_gcs), "color_set",
			    GTK_SIGNAL_FUNC (ucolor_changed_cb), NULL);

	label = gtk_label_new(_("System Load"));
	gtk_box_pack_start_defaults (GTK_BOX(color), label );
	gtk_box_pack_start_defaults (GTK_BOX(color), scolor_gcs);
	gtk_signal_connect (GTK_OBJECT (scolor_gcs), "color_set",
			    GTK_SIGNAL_FUNC (scolor_changed_cb), NULL);

	label = gtk_label_new(_("Applet Height"));
	height_a = gtk_adjustment_new( temp_props.height, 0.5, 128, 1, 8, 8 );
	height  = gtk_spin_button_new( GTK_ADJUSTMENT(height_a), 1, 0 );
	gtk_box_pack_start_defaults( GTK_BOX(size), label );
	gtk_box_pack_start_defaults( GTK_BOX(size), height );
	
	label = gtk_label_new(_("Width"));
	width_a = gtk_adjustment_new( temp_props.width, 0.5, 128, 1, 8, 8 );
	width  = gtk_spin_button_new( GTK_ADJUSTMENT(width_a), 1, 0 );
	gtk_box_pack_start_defaults( GTK_BOX(size), label );
	gtk_box_pack_start_defaults( GTK_BOX(size), width );

        gtk_signal_connect( GTK_OBJECT(height_a),"value_changed",
		GTK_SIGNAL_FUNC(height_cb), height );
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(height),
        	GTK_UPDATE_ALWAYS );
        gtk_signal_connect( GTK_OBJECT(width_a),"value_changed",
       		GTK_SIGNAL_FUNC(width_cb), width );
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(width),
        	GTK_UPDATE_ALWAYS );

	label = gtk_label_new(_("Update Frequency"));
	g_print( "%d %d\n", temp_props.speed, temp_props.speed/1000 );
	freq_a = gtk_adjustment_new( (float)temp_props.speed/1000, 0.1, 60, 0.1, 5, 5 );
	freq  = gtk_spin_button_new( GTK_ADJUSTMENT(freq_a), 0.1, 1 );
	gtk_box_pack_start( GTK_BOX(speed), label,TRUE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(speed), freq, TRUE, TRUE, 0 );
	
        gtk_signal_connect( GTK_OBJECT(freq_a),"value_changed",
		GTK_SIGNAL_FUNC(freq_cb), freq );
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(freq),
        	GTK_UPDATE_ALWAYS );
        
        gtk_box_pack_start_defaults( GTK_BOX(box), color );
	gtk_box_pack_start_defaults( GTK_BOX(box), size );
	gtk_box_pack_start_defaults( GTK_BOX(box), speed );
	
	gtk_widget_show_all(box);
	return box;
}

static void
apply_cb( GtkWidget *widget, void *data )
{
	memcpy( &props, &temp_props, sizeof(cpuload_properties) );

        setup_colors();
	start_timer();

	/*FIXME: this won't make the window smaller*/
	gtk_widget_set_usize( disp, props.width, props.height );
}

static gint
destroy_cb( GtkWidget *widget, void *data )
{
	propbox = NULL;
	return FALSE;
}

void
properties(AppletWidget *applet, gpointer data)
{
        static GnomeHelpMenuEntry help_entry = { "cpuload_applet",
						 "properties" };
	GtkWidget *frame, *label;

	if( propbox ) {
		gdk_window_raise(propbox->window);
		return;
	}

	memcpy(&temp_props,&props,sizeof(cpuload_properties));

        propbox = gnome_property_box_new();
	gtk_window_set_title( 
		GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window), 
		_("CPULoad Settings"));
	
	frame = create_frame();
	label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
	gtk_notebook_append_page(
		GTK_NOTEBOOK(GNOME_PROPERTY_BOX(propbox)->notebook),
		frame, label );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"apply", GTK_SIGNAL_FUNC(apply_cb), NULL );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"destroy", GTK_SIGNAL_FUNC(destroy_cb), NULL );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"help", GTK_SIGNAL_FUNC(gnome_help_pbox_display),
			    &help_entry );

	gtk_widget_show_all(propbox);
}
