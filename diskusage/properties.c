/* 
 * GNOME diskusage panel applet - properties dialog
 *
 * (C) 1997 The Free Software Foundation
 *
 */

#include <stdio.h>
#include <string.h>
#include <config.h>
#include <gnome.h>

#include "properties.h"
#include "diskusage.h"


GtkWidget *propbox=NULL;
static diskusage_properties temp_props;

static const unsigned int max_rgb_str_len = 7;
static const unsigned int max_rgb_str_size = 8;

extern GtkWidget *disp;
extern diskusage_properties props;
extern DiskusageInfo   summary_info;
	
extern void diskusage_resize(void);

typedef struct	_RadioButtonCbData		RadioButtonCbData;

struct _RadioButtonCbData
{
	GtkWidget *button, *size;
	diskusage_properties *props;
};

void setup_colors(void);
void start_timer( void );
void ucolor_set_cb(GnomeColorPicker *cp);
void fcolor_set_cb(GnomeColorPicker *cp);
void tcolor_set_cb(GnomeColorPicker *cp);
void bcolor_set_cb(GnomeColorPicker *cp);
void size_cb( GtkWidget *widget, GtkWidget *spin );
void freq_cb( GtkWidget *widget, GtkWidget *spin );
void best_size_cb( GtkWidget *widget, RadioButtonCbData *cb_data );
void apply_cb( GtkWidget *widget, int page_num, AppletWidget *applet );
gint destroy_cb( GtkWidget *widget, void *data );
GtkWidget *create_frame(diskusage_properties *props);

void load_properties( const char *path, diskusage_properties *prop )
{
	gnome_config_push_prefix (path);
	prop->startfs   = gnome_config_get_int    ("disk/startfs=0");
	prop->ucolor	= gnome_config_get_string ("disk/ucolor=#cf5f5f");
        if (prop->ucolor && (strlen(prop->ucolor) < max_rgb_str_len))
          prop->ucolor = g_realloc(prop->ucolor, max_rgb_str_size);

	prop->fcolor	= gnome_config_get_string ("disk/fcolor=#008f00");
        if (prop->fcolor && (strlen(prop->fcolor) < max_rgb_str_len))
          prop->fcolor = g_realloc(prop->fcolor, max_rgb_str_size);

	prop->tcolor	= gnome_config_get_string ("disk/tcolor=#bbbbbb");
        if (prop->tcolor && (strlen(prop->tcolor) < max_rgb_str_len))
          prop->tcolor = g_realloc(prop->tcolor, max_rgb_str_size);

	prop->bcolor	= gnome_config_get_string ("disk/bcolor=#000000");
        if (prop->bcolor && (strlen(prop->bcolor) < max_rgb_str_len))
          prop->bcolor = g_realloc(prop->bcolor, max_rgb_str_size);

	prop->speed	= gnome_config_get_int    ("disk/speed=2000");
	prop->size 	= gnome_config_get_int	  ("disk/size=120");
	prop->look	= gnome_config_get_bool   ("disk/look=1");
	prop->best_size	= gnome_config_get_bool   ("disk/best_size=1");
	gnome_config_pop_prefix ();
}



void save_properties( const char *path, diskusage_properties *prop )
{
	gnome_config_push_prefix (path);
	gnome_config_set_int   ( "disk/startfs", prop->startfs );
	gnome_config_set_string( "disk/ucolor", prop->ucolor );
	gnome_config_set_string( "disk/fcolor", prop->fcolor );
	gnome_config_set_string( "disk/tcolor", prop->tcolor );
	gnome_config_set_string( "disk/bcolor", prop->bcolor );
	gnome_config_set_int   ( "disk/speed", prop->speed );
	gnome_config_set_int   ( "disk/size", prop->size );
	gnome_config_set_bool  ( "disk/look", prop->look );
	gnome_config_set_bool  ( "disk/best_size", prop->best_size );
	gnome_config_pop_prefix ();
        gnome_config_sync();
	gnome_config_drop_all();
}

void ucolor_set_cb(GnomeColorPicker *cp)
{
 	guint8 r,g,b;
	gnome_color_picker_get_i8(cp, 
			&r,
			&g, 
			&b,
			NULL);
	g_snprintf( temp_props.ucolor, max_rgb_str_size, 
		    "#%02x%02x%02x", r, g, b );
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}
void fcolor_set_cb(GnomeColorPicker *cp)
{
 	guint8 r,g,b;
	gnome_color_picker_get_i8(cp, 
			&r,
			&g, 
			&b,
			NULL);
	g_snprintf( temp_props.fcolor, max_rgb_str_size, 
		 "#%02x%02x%02x", r, g, b );
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}
void tcolor_set_cb(GnomeColorPicker *cp)
{
 	guint8 r,g,b;
	gnome_color_picker_get_i8(cp, 
			&r,
			&g, 
			&b,
			NULL);
	g_snprintf( temp_props.tcolor, max_rgb_str_size,
		    "#%02x%02x%02x", r, g, b );
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}
void bcolor_set_cb(GnomeColorPicker *cp)
{
 	guint8 r,g,b;
	gnome_color_picker_get_i8(cp, 
			&r,
			&g, 
			&b,
			NULL);
	g_snprintf( temp_props.bcolor, max_rgb_str_size,
		    "#%02x%02x%02x", r, g, b );
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

void size_cb( GtkWidget *widget, GtkWidget *spin )
{
	temp_props.size = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

void freq_cb( GtkWidget *widget, GtkWidget *spin )
{
	temp_props.speed = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(spin))*1000;
        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
        return;
        widget = NULL;
}	

void best_size_cb( GtkWidget *widget, RadioButtonCbData *cb_data )
{
	cb_data->props->best_size = GTK_TOGGLE_BUTTON (cb_data->button)->active;

	if (cb_data->props->best_size)
		gtk_widget_set_sensitive (cb_data->size, FALSE);
	else
		gtk_widget_set_sensitive (cb_data->size, TRUE);

        gnome_property_box_changed(GNOME_PROPERTY_BOX(propbox));
}

GtkWidget *create_frame(diskusage_properties *props)
{
	GtkWidget *label;
	GtkWidget *best_size_button, *best_size;
	GtkWidget *box, *color, *size, *speed;
	GtkWidget *color1, *color2;
	GtkWidget *applet_size, *freq;
	GtkObject *applet_size_a, *freq_a;
	GtkWidget *ucolor_gcp, *fcolor_gcp, *tcolor_gcp, *bcolor_gcp;
        int ur,ug,ub, fr,fg,fb, tr,tg,tb, br, bg, bb;
	RadioButtonCbData *cb_data;

	        
	sscanf( temp_props.ucolor, "#%02x%02x%02x", &ur,&ug,&ub );
        sscanf( temp_props.fcolor, "#%02x%02x%02x", &fr,&fg,&fb );
        sscanf( temp_props.tcolor, "#%02x%02x%02x", &tr,&tg,&tb );
        sscanf( temp_props.bcolor, "#%02x%02x%02x", &br,&bg,&bb );
        
	box = gtk_vbox_new( 5, TRUE );
	color=gtk_vbox_new( 5, TRUE );
	size =gtk_hbox_new( 5, TRUE );
	speed=gtk_hbox_new( 5, TRUE );
	best_size =gtk_hbox_new( 5, TRUE );
	gtk_container_set_border_width( GTK_CONTAINER(box), 5 );
	        
	color1=gtk_hbox_new( 5, TRUE );
	color2=gtk_hbox_new( 5, TRUE );

	ucolor_gcp = gnome_color_picker_new();
	fcolor_gcp = gnome_color_picker_new();
	tcolor_gcp = gnome_color_picker_new();
	bcolor_gcp = gnome_color_picker_new();
	gnome_color_picker_set_i8(GNOME_COLOR_PICKER (ucolor_gcp), 
			ur, ug, ub, 255);
	gnome_color_picker_set_i8(GNOME_COLOR_PICKER (fcolor_gcp), 
			fr, fg, fb, 255);
	gnome_color_picker_set_i8(GNOME_COLOR_PICKER (tcolor_gcp), 
			tr, tg, tb, 255);
	gnome_color_picker_set_i8(GNOME_COLOR_PICKER (bcolor_gcp), 
			br, bg, bb, 255);
	gtk_signal_connect(GTK_OBJECT(ucolor_gcp), "color_set",
			GTK_SIGNAL_FUNC(ucolor_set_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(fcolor_gcp), "color_set",
			GTK_SIGNAL_FUNC(fcolor_set_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(tcolor_gcp), "color_set",
			GTK_SIGNAL_FUNC(tcolor_set_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(bcolor_gcp), "color_set",
			GTK_SIGNAL_FUNC(bcolor_set_cb), NULL);


	label = gtk_label_new(_("Used Diskspace"));
	gtk_box_pack_start_defaults( GTK_BOX(color1), label );
	gtk_box_pack_start_defaults( GTK_BOX(color1), 
		 ucolor_gcp );

	label = gtk_label_new(_("Free Diskspace"));
	gtk_box_pack_start_defaults( GTK_BOX(color2), label );
	gtk_box_pack_start_defaults( GTK_BOX(color2), 
		 fcolor_gcp );

	label = gtk_label_new(_("Textcolor"));
	gtk_box_pack_start_defaults( GTK_BOX(color1), label );
	gtk_box_pack_start_defaults( GTK_BOX(color1), 
		 tcolor_gcp );


	label = gtk_label_new(_("Backgroundcolor"));
	gtk_box_pack_start_defaults( GTK_BOX(color2), label );
	gtk_box_pack_start_defaults( GTK_BOX(color2), 
		 bcolor_gcp );



	label = gtk_label_new(_("Applet Size"));
	applet_size_a = gtk_adjustment_new( temp_props.size, 0.5, 256, 1, 8, 8 );
	applet_size  = gtk_spin_button_new( GTK_ADJUSTMENT(applet_size_a), 1, 0 );
	gtk_box_pack_start_defaults( GTK_BOX(size), label );
	gtk_box_pack_start_defaults( GTK_BOX(size), applet_size );


	best_size_button = gtk_check_button_new_with_label(_("Automatically pick best applet size"));

	cb_data = g_new0 (RadioButtonCbData, 1);
	cb_data->button = best_size_button;
	cb_data->size = size;
	cb_data->props = props;

	gtk_signal_connect (GTK_OBJECT (best_size_button), "toggled", best_size_cb, cb_data);

	if (props->best_size) {
		gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (best_size_button), TRUE);
		gtk_widget_set_sensitive (size, FALSE);
	}

	gtk_box_pack_start_defaults( GTK_BOX(best_size), best_size_button );

        gtk_signal_connect( GTK_OBJECT(applet_size_a),"value_changed",
		GTK_SIGNAL_FUNC(size_cb), applet_size );
        gtk_signal_connect( GTK_OBJECT(applet_size_a),"changed",
		GTK_SIGNAL_FUNC(size_cb), applet_size );
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(applet_size),
        	GTK_UPDATE_ALWAYS );


	label = gtk_label_new(_("Update Frequency"));
	freq_a = gtk_adjustment_new( (float)temp_props.speed/1000, 0.1, 60, 0.1, 5, 5 );
	freq  = gtk_spin_button_new( GTK_ADJUSTMENT(freq_a), 0.1, 1 );
	gtk_box_pack_start( GTK_BOX(speed), label,TRUE, FALSE, 0 );
	gtk_box_pack_start( GTK_BOX(speed), freq, TRUE, TRUE, 0 );

	
        gtk_signal_connect( GTK_OBJECT(freq_a),"value_changed",
		GTK_SIGNAL_FUNC(freq_cb), freq );
        gtk_signal_connect( GTK_OBJECT(freq),"changed",
		GTK_SIGNAL_FUNC(freq_cb), freq );
        gtk_spin_button_set_update_policy( GTK_SPIN_BUTTON(freq),
        	GTK_UPDATE_ALWAYS );

	gtk_box_pack_start_defaults( GTK_BOX(color), color1 );
        gtk_box_pack_start_defaults( GTK_BOX(color), color2 );
        
        gtk_box_pack_start_defaults( GTK_BOX(box), color );
	gtk_box_pack_start_defaults( GTK_BOX(box), size );
	gtk_box_pack_start_defaults( GTK_BOX(box), speed );
	gtk_box_pack_start_defaults( GTK_BOX(box), best_size );
	
	gtk_widget_show_all(box);
	return box;
}

void apply_cb( GtkWidget *widget, gint page_num, AppletWidget *applet )
{
	memcpy( &props, &temp_props, sizeof(diskusage_properties) );

        setup_colors();
	start_timer();
	diskusage_resize();
	applet_widget_sync_config(applet);
}

gint destroy_cb( GtkWidget *widget, void *data )
{
	propbox = NULL;
	return FALSE;
        widget = NULL;
	data = NULL;
}

void properties(AppletWidget *applet, gpointer data)
{
        static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
	GtkWidget *frame, *label;

	help_entry.name = gnome_app_id;


	if( propbox ) {
		gdk_window_raise(propbox->window);
		return;
	}

	memcpy(&temp_props,&props,sizeof(diskusage_properties));


        propbox = gnome_property_box_new();
	gtk_window_set_title( 
		GTK_WINDOW(&GNOME_PROPERTY_BOX(propbox)->dialog.window), 
		_("Diskusage Settings"));

	
	frame = create_frame(&temp_props);
	label = gtk_label_new(_("General"));
        gtk_widget_show(frame);

	
	gnome_property_box_append_page( GNOME_PROPERTY_BOX(propbox),
		frame, label );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"apply", GTK_SIGNAL_FUNC(apply_cb), applet );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"destroy", GTK_SIGNAL_FUNC(destroy_cb), NULL );

        gtk_signal_connect( GTK_OBJECT(propbox),
		"help", GTK_SIGNAL_FUNC(gnome_help_pbox_display),
			    &help_entry);

	gtk_widget_show_all(propbox);
        return;
        applet = NULL;
        data = NULL;	
}
