/* GNOME pppload panel applet - properties dialog
 * (C) 1997 The Free Software Foundation
 *
 * Author: Tim P. Gerla
 *
 */

#include <stdio.h>
#include <string.h>
#include <config.h>
#include <gnome.h>
#include <applet-widget.h>

#include "properties.h"

void gcolor_changed_cb( GnomeColorPicker *widget );
void bcolor_changed_cb( GnomeColorPicker *widget );
void height_cb( GtkWidget *widget, GtkWidget *spin );
void width_cb( GtkWidget *widget, GtkWidget *spin );
void freq_cb( GtkWidget *widget, GtkWidget *spin );
GtkWidget *create_general_frame(void);
void device_cb( GtkWidget *widget, GtkWidget *value );
void line_cb( GtkWidget *widget, GtkWidget *spin );
GtkWidget *create_device_frame(void);
void apply_cb( GtkWidget *widget, void *data );
void close_cb( GtkWidget *widget, void *data );

GtkWidget *propbox;
extern netload_properties props;
static netload_properties new_props;

void setup_colors(void);
void start_timer( void );

void load_properties(char *path, netload_properties *prop )
{
	gnome_config_push_prefix(path);
	prop->gcolor	= gnome_config_get_string ("net/gcolor=#f7f426");
	prop->bcolor	= gnome_config_get_string ("net/bcolor=#e88a40");
	prop->speed	= gnome_config_get_int    ("net/speed=2000");
	prop->height 	= gnome_config_get_int	  ("net/height=40");
	prop->width 	= gnome_config_get_int	  ("net/width=40");
	prop->look	= gnome_config_get_bool   ("net/look=1");
	prop->device	= gnome_config_get_string ("net/device=ppp0");
	prop->line_spacing = gnome_config_get_int ("net/line_spacing=2");
	gnome_config_pop_prefix();
}

void save_properties(char *path, netload_properties *prop )
{
	gnome_config_push_prefix(path);
	gnome_config_set_string( "net/gcolor", prop->gcolor );
	gnome_config_set_string( "net/bcolor", prop->bcolor );
	gnome_config_set_int   ( "net/speed", prop->speed );
	gnome_config_set_int   ( "net/height", prop->height );
	gnome_config_set_int   ( "net/width", prop->width );
	gnome_config_set_bool  ( "net/look", prop->look );
	gnome_config_set_string( "net/device", prop->device );
	gnome_config_set_int   ( "net/line_spacing", prop->line_spacing );
	gnome_config_sync();
	gnome_config_pop_prefix();
}

void gcolor_changed_cb( GnomeColorPicker *widget )
{
        gchar tmp[24];
 	guint8 r,g,b;

	gnome_color_picker_get_i8(widget, &r, &g, &b, NULL);

	sprintf( tmp, "#%02x%02x%02x", r, g, b );
	strcpy(new_props.gcolor, tmp);
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}          

void bcolor_changed_cb( GnomeColorPicker *widget )
{
        gchar tmp[24];
 	guint8 r,g,b;

	gnome_color_picker_get_i8(widget, &r, &g, &b, NULL);

	sprintf( tmp, "#%02x%02x%02x", r, g, b );
	strcpy(new_props.bcolor, tmp);
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}          

void height_cb( GtkWidget *widget, GtkWidget *spin )
{
	new_props.height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

void width_cb( GtkWidget *widget, GtkWidget *spin )
{
	new_props.width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

void freq_cb( GtkWidget *widget, GtkWidget *spin )
{
	new_props.speed = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spin))*1000;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}	

GtkWidget *create_general_frame(void)
{
	GtkWidget *label;
	GtkWidget *box, *color, *size, *speed;
	GtkWidget *height, *width, *freq;
	GtkObject *height_a, *width_a, *freq_a;

	GtkWidget *gcolor_gcs, *bcolor_gcs;
        int ur,ug,ub, sr,sg,sb;

	sscanf( props.gcolor, "#%02x%02x%02x", &ur,&ug,&ub );
        sscanf( props.bcolor, "#%02x%02x%02x", &sr,&sg,&sb );
        
	box = gtk_vbox_new( 5, TRUE );
	color=gtk_hbox_new( 5, TRUE );
	size =gtk_hbox_new( 5, TRUE );
	speed=gtk_hbox_new( 5, TRUE );
	gtk_container_set_border_width( GTK_CONTAINER(box), 5 );
	        
	gcolor_gcs = gnome_color_picker_new();
	bcolor_gcs = gnome_color_picker_new();

	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (gcolor_gcs),
				   ur, ug, ub, 255);
	gnome_color_picker_set_i8 (GNOME_COLOR_PICKER (bcolor_gcs),
				   sr, sg, sb, 255);

	label = gtk_label_new(_("Network Traffic"));
	gtk_box_pack_start_defaults( GTK_BOX(color), label );
	gtk_box_pack_start_defaults( GTK_BOX(color), gcolor_gcs );

	label = gtk_label_new(_("Traffic bars"));
	gtk_box_pack_start_defaults( GTK_BOX(color), label );
	gtk_box_pack_start_defaults( GTK_BOX(color), bcolor_gcs );

	label = gtk_label_new(_("Applet Height"));
	height_a = gtk_adjustment_new( props.height, 0.5, 128, 1, 8, 8 );
	height  = gtk_spin_button_new( GTK_ADJUSTMENT(height_a), 1, 0 );
	gtk_box_pack_start_defaults( GTK_BOX(size), label );
	gtk_box_pack_start_defaults( GTK_BOX(size), height );
	
	label = gtk_label_new(_("Width"));
	width_a = gtk_adjustment_new( props.width, 0.5, 128, 1, 8, 8 );
	width  = gtk_spin_button_new( GTK_ADJUSTMENT(width_a), 1, 0 );
	gtk_box_pack_start_defaults( GTK_BOX(size), label );
	gtk_box_pack_start_defaults( GTK_BOX(size), width );

        gtk_signal_connect( GTK_OBJECT(height_a),"value_changed",
		GTK_SIGNAL_FUNC(height_cb), height );
        gtk_signal_connect( GTK_OBJECT(height),"changed",
		GTK_SIGNAL_FUNC(height_cb), height );
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(height),
        	GTK_UPDATE_ALWAYS );
        gtk_signal_connect( GTK_OBJECT(width_a),"value_changed",
       		GTK_SIGNAL_FUNC(width_cb), width );
        gtk_signal_connect( GTK_OBJECT(width),"changed",
       		GTK_SIGNAL_FUNC(width_cb), width );
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(width),
        	GTK_UPDATE_ALWAYS );
	gtk_signal_connect (GTK_OBJECT (gcolor_gcs), "color_set",
			    GTK_SIGNAL_FUNC (gcolor_changed_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (bcolor_gcs), "color_set",
			    GTK_SIGNAL_FUNC (bcolor_changed_cb), NULL);

	label = gtk_label_new(_("Update Frequency"));

	freq_a = gtk_adjustment_new( (float)props.speed/1000, 1, 60, 0.1, 5, 5 );
	freq  = gtk_spin_button_new( GTK_ADJUSTMENT(freq_a), 1, 1 );

	gtk_box_pack_start( GTK_BOX(speed), label,TRUE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(speed), freq, TRUE, TRUE, 0 );

        gtk_signal_connect( GTK_OBJECT(freq_a),"value_changed",
		GTK_SIGNAL_FUNC(freq_cb), freq );
        gtk_signal_connect( GTK_OBJECT(freq),"changed",
		GTK_SIGNAL_FUNC(freq_cb), freq );

        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(freq),
        	GTK_UPDATE_ALWAYS );

        gtk_box_pack_start_defaults( GTK_BOX(box), color );
	gtk_box_pack_start_defaults( GTK_BOX(box), size );
	gtk_box_pack_start_defaults( GTK_BOX(box), speed );

	gtk_widget_show_all(box);

	return box;
}

void device_cb( GtkWidget *widget, GtkWidget *value )
{
	new_props.device = gtk_entry_get_text (GTK_ENTRY(value));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}	

void line_cb( GtkWidget *widget, GtkWidget *spin )
{
	new_props.line_spacing = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

GtkWidget *create_device_frame(void)
{
	GtkWidget *box, *label, *device_box, *device, *line_box, *line;
	GtkObject *line_a;

	box = gtk_vbox_new( 5, TRUE );

	device_box = gtk_hbox_new(5, FALSE);
	label = gtk_label_new(_("Device name (like ppp0 or eth0)"));
	gtk_box_pack_start_defaults( GTK_BOX(device_box), label);
	device = gtk_entry_new_with_max_length(5);
	gtk_entry_set_text(GTK_ENTRY(device), props.device);
	gtk_signal_connect( GTK_OBJECT(device), "changed", GTK_SIGNAL_FUNC(device_cb), device);
	gtk_box_pack_start_defaults( GTK_BOX(device_box), device);

	line_box = gtk_hbox_new(5, TRUE);
	label = gtk_label_new(_("Vertical spacing of bars (in kilobytes)"));
	gtk_box_pack_start_defaults( GTK_BOX(line_box), label);
	line_a = gtk_adjustment_new( props.line_spacing, 1, 1024, 1, 10, 10 );
	line = gtk_spin_button_new( GTK_ADJUSTMENT(line_a), 1, 0 );
	gtk_box_pack_start_defaults( GTK_BOX(line_box), line);

        gtk_signal_connect( GTK_OBJECT(line_a),"value_changed",
       		GTK_SIGNAL_FUNC(line_cb), line );
        gtk_signal_connect( GTK_OBJECT(line),"changed",
       		GTK_SIGNAL_FUNC(line_cb), line );
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(line),
        	GTK_UPDATE_ALWAYS );
		
	gtk_box_pack_start_defaults( GTK_BOX(box), device_box);
	gtk_box_pack_start_defaults( GTK_BOX(box), line_box);
	gtk_widget_show_all(box);

	return box;
}

void apply_cb( GtkWidget *widget, void *data )
{
	memcpy(&props, &new_props, sizeof(netload_properties));
	props.device=strdup(new_props.device);
        setup_colors();
	start_timer();
	
	/* FIXME: Need to resize. */
}

void close_cb( GtkWidget *widget, void *data )
{
	propbox = NULL;
}

void properties(AppletWidget *applet, gpointer data)
{
	GtkWidget *frame, *label;

	if( propbox )
		return;

	memcpy(&new_props, &props, sizeof(netload_properties));
		
        if (!(propbox = gnome_property_box_new())){
		fprintf(stderr,_("properties.c: gnome_property_box_new() failed.\n"));
		return;
	}

	gtk_window_set_title( 
		GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window), 
		_("Network Load Settings"));
	
	frame = create_general_frame();
	label = gtk_label_new(_("General"));
        gtk_widget_show(frame);
	gnome_property_box_append_page( GNOME_PROPERTY_BOX(propbox),
		frame, label );

	frame = create_device_frame();
	label = gtk_label_new(_("Device"));
        gtk_widget_show(frame);
	gnome_property_box_append_page( GNOME_PROPERTY_BOX(propbox),
		frame, label );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"apply", GTK_SIGNAL_FUNC(apply_cb), NULL );

	gtk_signal_connect( GTK_OBJECT(propbox),
		"destroy", GTK_SIGNAL_FUNC(close_cb), NULL );

	gtk_widget_show_all(propbox);
}
