/* GNOME cpuload panel applet - properties dialog
 * (C) 1997 The Free Software Foundation
 *
 * Author: Tim P. Gerla
 *
 */

#include <stdio.h>
#ifdef HAVE_LIBINTL
#    include <libintl.h>
#endif
#include <gnome.h>

#include "properties.h"

GtkWidget *propbox;
cpuload_properties props;

void setup_colors(void);
void start_timer( void );

void load_properties( cpuload_properties *prop )
{
	prop->ucolor	= gnome_config_get_string ("/cpuload_applet/all/ucolor=#20b2aa");
	prop->scolor	= gnome_config_get_string ("/cpuload_applet/all/scolor=#188982");
	prop->speed	= gnome_config_get_int    ("/cpuload_applet/all/speed=2000");
	prop->height 	= gnome_config_get_int	  ("/cpuload_applet/all/height=40");
	prop->width 	= gnome_config_get_int	  ("/cpuload_applet/all/width=40");
	prop->look	= gnome_config_get_bool   ("/cpuload_applet/all/look=1");
}

void save_properties( cpuload_properties *prop )
{
	gnome_config_set_string( "/cpuload_applet/all/ucolor", prop->ucolor );
	gnome_config_set_string( "/cpuload_applet/all/scolor", prop->scolor );
	gnome_config_set_int   ( "/cpuload_applet/all/speed", prop->speed );
	gnome_config_set_int   ( "/cpuload_applet/all/height", prop->height );
	gnome_config_set_int   ( "/cpuload_applet/all/width", prop->width );
	gnome_config_set_bool  ( "/cpuload_applet/all/look", prop->look );
        gnome_config_sync();
}

void color_changed_cb( GnomeColorSelector *widget, gchar **color )
{
        char *tmp;
 	int r,g,b;

	/* FIXME ugh, mem leak..anyone have a better way of doing this? */        
	tmp = malloc(24);
        if( !tmp )
        {
        	g_warning( "Can't allocate memory for color\n" );
                return;
        }
        gnome_color_selector_get_color_int(
        	widget, &r, &g, &b, 255 );
	
	sprintf( tmp, "#%02x%02x%02x", r, g, b );
        *color = tmp;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}          

void height_cb( GtkWidget *widget, GtkWidget *spin )
{
	props.height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}
void width_cb( GtkWidget *widget, GtkWidget *spin )
{
	props.width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}
void freq_cb( GtkWidget *widget, GtkWidget *spin )
{
	props.speed = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spin))*1000;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}	

GtkWidget *create_frame(void)
{
	GtkWidget *label;
	GtkWidget *box, *color, *size, *speed;
	GtkWidget *height, *width, *freq;
	GtkObject *height_a, *width_a, *freq_a;
	        
	GnomeColorSelector *ucolor_gcs, *scolor_gcs;
        int ur,ug,ub, sr,sg,sb;

	sscanf( props.ucolor, "#%02x%02x%02x", &ur,&ug,&ub );
        sscanf( props.scolor, "#%02x%02x%02x", &sr,&sg,&sb );
        
	box = gtk_vbox_new( 5, TRUE );
	color=gtk_hbox_new( 5, TRUE );
	size =gtk_hbox_new( 5, TRUE );
	speed=gtk_hbox_new( 5, TRUE );
	gtk_container_border_width( GTK_CONTAINER(box), 5 );
	        
	
	ucolor_gcs  = gnome_color_selector_new( (SetColorFunc)color_changed_cb,
		&props.ucolor );
	scolor_gcs = gnome_color_selector_new( (SetColorFunc)color_changed_cb,
		&props.scolor );

        gnome_color_selector_set_color_int( ucolor_gcs, ur, ug, ub, 255 );
	gnome_color_selector_set_color_int( scolor_gcs, sr, sg, sb, 255 );
                  

	label = gtk_label_new("User Load");
	gtk_box_pack_start_defaults( GTK_BOX(color), label );
	gtk_box_pack_start_defaults( GTK_BOX(color), 
		gnome_color_selector_get_button(ucolor_gcs) );

	label = gtk_label_new("System Load");
	gtk_box_pack_start_defaults( GTK_BOX(color), label );
	gtk_box_pack_start_defaults( GTK_BOX(color), 
		gnome_color_selector_get_button(scolor_gcs) );

	label = gtk_label_new("Applet Height");
	height_a = gtk_adjustment_new( props.height, 0.5, 128, 1, 8, 8 );
	height  = gtk_spin_button_new( GTK_ADJUSTMENT(height_a), 1, 0 );
	gtk_box_pack_start_defaults( GTK_BOX(size), label );
	gtk_box_pack_start_defaults( GTK_BOX(size), height );
	
	label = gtk_label_new("Width");
	width_a = gtk_adjustment_new( props.width, 0.5, 128, 1, 8, 8 );
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

	label = gtk_label_new("Update Frequency");
	g_print( "%d %d\n", props.speed, props.speed/1000 );
	freq_a = gtk_adjustment_new( (float)props.speed/1000, 0.1, 60, 0.1, 5, 5 );
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

void apply_cb( GtkWidget *widget, void *data )
{
	save_properties(&props);
        setup_colors();
	start_timer();
}

void properties(int id, gpointer data)
{
	GtkWidget *frame, *label;

	load_properties(&props);
	
        propbox = gnome_property_box_new();
	gtk_window_set_title( 
		GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window), 
		"CPULoad Settings" );
	
	frame = create_frame();
	label = gtk_label_new("General");
        gtk_widget_show(frame);
	gnome_property_box_append_page( GNOME_PROPERTY_BOX(propbox),
		frame, label );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"apply", GTK_SIGNAL_FUNC(apply_cb), NULL );

	gtk_widget_show_all(propbox);
}
