/* GNOME diskusage panel applet - properties dialog
 *
 * (C) 1997 The Free Software Foundation
 *
 * this was cut & past'ed nearly 1:1 from the cpuload applet
 *
 * Author: Tim P. Gerla
 *
 */

#include <stdio.h>
#include <string.h>
#include <config.h>
#include <gnome.h>

#include "properties.h"
#include "diskusage.h"

/*#define DU_DEBUG*/

GtkWidget *propbox=NULL;
static diskusage_properties temp_props;

extern GtkWidget *disp;
extern diskusage_properties props;
extern DiskusageInfo   summary_info;

void setup_colors(void);
void start_timer( void );

void load_properties( char *path, diskusage_properties *prop )
{
	gnome_config_push_prefix (path);
	prop->ucolor	= gnome_config_get_string ("disk/ucolor=#cf5f5f");
	prop->fcolor	= gnome_config_get_string ("disk/fcolor=#008f00");
	prop->tcolor	= gnome_config_get_string ("disk/tcolor=#bbbbbb");
	prop->bcolor	= gnome_config_get_string ("disk/bcolor=#000000");
	prop->speed	= gnome_config_get_int    ("disk/speed=2000");
	prop->height 	= gnome_config_get_int	  ("disk/height=40");
	prop->width 	= gnome_config_get_int	  ("disk/width=120");
	prop->look	= gnome_config_get_bool   ("disk/look=1");
	gnome_config_pop_prefix ();
}



void save_properties( char *path, diskusage_properties *prop )
{
	gnome_config_push_prefix (path);
	gnome_config_set_string( "disk/ucolor", prop->ucolor );
	gnome_config_set_string( "disk/fcolor", prop->fcolor );
	gnome_config_set_string( "disk/tcolor", prop->tcolor );
	gnome_config_set_string( "disk/bcolor", prop->bcolor );
	gnome_config_set_int   ( "disk/speed", prop->speed );
	gnome_config_set_int   ( "disk/height", prop->height );
	gnome_config_set_int   ( "disk/width", prop->width );
	gnome_config_set_bool  ( "disk/look", prop->look );
	gnome_config_pop_prefix ();
        gnome_config_sync();
	gnome_config_drop_all();
}

void color_changed_cb( GnomeColorSelector *widget, gchar **color )
{
        char *tmp;
 	int r,g,b;

	/* FIXME ugh, mem leak..anyone have a better way of doing this? */        
	tmp = malloc(24);
        if( !tmp )
        {
	        g_warning(_("Can't allocate memory for color\n"));
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
	temp_props.height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}
void width_cb( GtkWidget *widget, GtkWidget *spin )
{
	temp_props.width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

void freq_cb( GtkWidget *widget, GtkWidget *spin )
{
	temp_props.speed = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spin))*1000;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}	

GtkWidget *create_frame(void)
{
	GtkWidget *label;
	GtkWidget *box, *color, *size, *speed;
	GtkWidget *color1, *color2;
	GtkWidget *height, *width, *freq;
	GtkObject *height_a, *width_a, *freq_a;

	GnomeColorSelector *ucolor_gcs, *fcolor_gcs, *tcolor_gcs, *bcolor_gcs;
        int ur,ug,ub, fr,fg,fb, tr,tg,tb, br, bg, bb;

#ifdef DU_DEBUG
	printf (" entering create_frame\n");
#endif
	        
	sscanf( temp_props.ucolor, "#%02x%02x%02x", &ur,&ug,&ub );
        sscanf( temp_props.fcolor, "#%02x%02x%02x", &fr,&fg,&fb );
        sscanf( temp_props.tcolor, "#%02x%02x%02x", &tr,&tg,&tb );
        sscanf( temp_props.bcolor, "#%02x%02x%02x", &br,&bg,&bb );
        
#ifdef DU_DEBUG
	printf (" gtkvboxnew \n");
#endif
	box = gtk_vbox_new( 5, TRUE );
	color=gtk_vbox_new( 5, TRUE );
	size =gtk_hbox_new( 5, TRUE );
	speed=gtk_hbox_new( 5, TRUE );
	gtk_container_border_width( GTK_CONTAINER(box), 5 );
	        
#ifdef DU_DEBUG
	printf (" colorselctor_new \n");
#endif
	color1=gtk_hbox_new( 5, TRUE );
	color2=gtk_hbox_new( 5, TRUE );
	
	ucolor_gcs  = gnome_color_selector_new( (SetColorFunc)color_changed_cb,
		&temp_props.ucolor );
	fcolor_gcs = gnome_color_selector_new( (SetColorFunc)color_changed_cb,
		&temp_props.fcolor );
	tcolor_gcs = gnome_color_selector_new( (SetColorFunc)color_changed_cb,
		&temp_props.tcolor );
	bcolor_gcs = gnome_color_selector_new( (SetColorFunc)color_changed_cb,
		&temp_props.bcolor );

        gnome_color_selector_set_color_int( ucolor_gcs, ur, ug, ub, 255 );
	gnome_color_selector_set_color_int( fcolor_gcs, fr, fg, fb, 255 );
	gnome_color_selector_set_color_int( tcolor_gcs, tr, tg, tb, 255 );
	gnome_color_selector_set_color_int( bcolor_gcs, br, bg, bb, 255 );
                  
#ifdef DU_DEBUG
	printf (" label_new \n");
#endif

	label = gtk_label_new(_("Used Diskspace"));
	gtk_box_pack_start_defaults( GTK_BOX(color1), label );
	gtk_box_pack_start_defaults( GTK_BOX(color1), 
		gnome_color_selector_get_button(ucolor_gcs) );

	label = gtk_label_new(_("Free Diskspace"));
	gtk_box_pack_start_defaults( GTK_BOX(color2), label );
	gtk_box_pack_start_defaults( GTK_BOX(color2), 
		gnome_color_selector_get_button(fcolor_gcs) );

	label = gtk_label_new(_("Textcolor"));
	gtk_box_pack_start_defaults( GTK_BOX(color1), label );
	gtk_box_pack_start_defaults( GTK_BOX(color1), 
		gnome_color_selector_get_button(tcolor_gcs) );

	label = gtk_label_new(_("Backgroundcolor"));
	gtk_box_pack_start_defaults( GTK_BOX(color2), label );
	gtk_box_pack_start_defaults( GTK_BOX(color2), 
		gnome_color_selector_get_button(bcolor_gcs) );


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
#ifdef DU_DEBUG
	g_print( "%d %d\n", temp_props.speed, temp_props.speed/1000 );
#endif
	freq_a = gtk_adjustment_new( (float)temp_props.speed/1000, 0.1, 60, 0.1, 5, 5 );
	freq  = gtk_spin_button_new( GTK_ADJUSTMENT(freq_a), 0.1, 1 );
	gtk_box_pack_start( GTK_BOX(speed), label,TRUE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(speed), freq, TRUE, TRUE, 0 );

#ifdef DU_DEBUG
	printf (" signal_connect\n");
#endif
	
        gtk_signal_connect( GTK_OBJECT(freq_a),"value_changed",
		GTK_SIGNAL_FUNC(freq_cb), freq );
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(freq),
        	GTK_UPDATE_ALWAYS );
        
	gtk_box_pack_start_defaults( GTK_BOX(color), color1 );
        gtk_box_pack_start_defaults( GTK_BOX(color), color2 );
        
        gtk_box_pack_start_defaults( GTK_BOX(box), color );
	gtk_box_pack_start_defaults( GTK_BOX(box), size );
	gtk_box_pack_start_defaults( GTK_BOX(box), speed );
	
	gtk_widget_show_all(box);
	return box;
}

void apply_cb( GtkWidget *widget, void *data )
{
	memcpy( &props, &temp_props, sizeof(diskusage_properties) );

        setup_colors();
	start_timer();
	diskusage_resize();

}

gint destroy_cb( GtkWidget *widget, void *data )
{
	propbox = NULL;
	return FALSE;
}

void properties(AppletWidget *applet, gpointer data)
{
	GtkWidget *frame, *label;

#ifdef DU_DEBUG
	printf ("entering diskusage properties( \n");
#endif

	if( propbox ) {
		gdk_window_raise(propbox->window);
		return;
	}
#ifdef DU_DEBUG
	printf ("  memcpy \n");
#endif

	memcpy(&temp_props,&props,sizeof(diskusage_properties));

#ifdef DU_DEBUG
	printf ("  memcpy ...\n");
#endif

        propbox = gnome_property_box_new();
	gtk_window_set_title( 
		GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window), 
		_("Diskusage Settings"));

#ifdef DU_DEBUG
	printf ("  frame \n");
#endif
	
	frame = create_frame();
#ifdef DU_DEBUG
	printf ("  label_new\n");
#endif
	label = gtk_label_new(_("General"));
#ifdef DU_DEBUG
	printf ("  widget_show\n");
#endif
        gtk_widget_show(frame);

#ifdef DU_DEBUG
	printf ("  property_box_append_page\n");
#endif
	
	gnome_property_box_append_page( GNOME_PROPERTY_BOX(propbox),
		frame, label );

#ifdef DU_DEBUG
	printf ("  gtk_signal_connect \n");
#endif
        gtk_signal_connect( GTK_OBJECT(propbox),
		"apply", GTK_SIGNAL_FUNC(apply_cb), NULL );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"destroy", GTK_SIGNAL_FUNC(destroy_cb), NULL );

	gtk_widget_show_all(propbox);
}
