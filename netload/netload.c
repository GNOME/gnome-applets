/* GNOME network load panel applet
 * (C) 1997,1998 The Free Software Foundation
 *
 * Author: Stephen Norris
 * With code extensively stolen from: Tim P. Gerla
 * With code from wmload.c, v0.9.2, apparently by Ryan Land, rland@bc1.com.
 *
 */

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <config.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include "applet-lib.h"

#include "linux-proc.h"
#include "properties.h"

void start_timer( void );

GtkWidget *netload;
GdkPixmap *pixmap;
GtkWidget *disp;
GdkGC *gc;
GdkColor gcolor, bcolor;
netload_properties props;
int timer_index=-1;

/*
 * Use a circular buffer for storing the values.
 */

int draw(void)
{
	static gint width = 0;
	static unsigned long int	*data = NULL;
	static unsigned long int	old_bytes, max_delta;	/* Remember what the biggest value was. */
	static int	max_delta_pos = -1;	/* Remember where the maximum was. */
	unsigned long int	bytes;
	static int 	init = 0;
	static int	front = 0;	/* Front of the buffer.*/
	int	i, p;
	static float	scale_factor = 0;
	int height = disp->allocation.height;

	if (width != disp->allocation.width || data == NULL){
		/* Resize horizontally. */
		width = disp->allocation.width;
		if (data)
			free(data);
		data = (unsigned long int *)calloc(width, sizeof(unsigned long int));
	}
	
	bytes = GetTraffic(1, props.device);

	if (! init){
		init = 1;
		old_bytes = GetTraffic(1, props.device);
	}

	/* The MAX is in case the stats get reset or wrap. It'll get one wrong sample. */
	data[front] = MAX(0, ((signed int)(bytes - old_bytes))/ ((float)props.speed/1000));

	if (front == max_delta_pos){
		/* The maximum has scrolled off. Rescale. */
		max_delta = data[0];
		max_delta_pos = 0;
		for (i = 0; i < width; i ++){
			if (data[i] > max_delta){
				max_delta = data[i];
				scale_factor = (float)height / (float)max_delta;
				max_delta_pos = i;
			}
		}
	}

	if (data[front] > max_delta){
			max_delta = data[front];
			max_delta_pos = front;
			scale_factor = (float)height / (float)max_delta;
	}

	/* Erase Rectangle */
	gdk_draw_rectangle( pixmap,
		disp->style->black_gc,
		TRUE, 0,0,
		disp->allocation.width,
		disp->allocation.height );

	p = width;
	gdk_gc_set_foreground( gc, &gcolor );
	for( i = front; i >= 0; i-- ){
		if (data[i] ){
			gdk_draw_line( pixmap,
			       gc,
			       p, height,
			       p, (int)(height - (data[i] * scale_factor)) );
		}
		p --;
	}

	for (i = width - 1; i > front; i --){
		if (data[i] ){
			gdk_draw_line( pixmap,
			       gc,
			       p, height,
			       p, (int)(height - (data[i] * scale_factor)) );
		}
		p --;
	}

	front++;
	if (front >= width)
		front = 0;

	old_bytes = bytes;

	/* Draw on horizontal lines if appropriate. */

	gdk_gc_set_foreground( gc, &bcolor );
	for (i = props.line_spacing; i < max_delta; i+=props.line_spacing){
		gdk_draw_line( pixmap,
		       gc,
		       0, height - (int)(i * scale_factor),
		       width, height - (int)(i * scale_factor) );
	}
		
	gdk_draw_pixmap(disp->window,
		disp->style->fg_gc[GTK_WIDGET_STATE(disp)],
	        pixmap,
	        0, 0,
	        0, 0,
	        disp->allocation.width,
	        disp->allocation.height);

#if 0
	g_print("max_delta = %ld scale_factor = %f\n", max_delta, scale_factor);
#endif
	
	return TRUE;
}

static gint netload_configure(GtkWidget *widget, GdkEventConfigure *event)
{
        pixmap = gdk_pixmap_new( widget->window,
                                 widget->allocation.width,
                                 widget->allocation.height,
                                 gtk_widget_get_visual(disp)->depth );
        gdk_draw_rectangle( pixmap,
                            widget->style->black_gc,
                            TRUE, 0,0,
                            widget->allocation.width,
                            widget->allocation.height );
        gdk_draw_pixmap(widget->window,
                disp->style->fg_gc[GTK_WIDGET_STATE(widget)],
                pixmap,
                0, 0,
                0, 0,
                disp->allocation.width,
                disp->allocation.height);
	return TRUE;
} 

static gint netload_expose(GtkWidget *widget, GdkEventExpose *event)
{
        gdk_draw_pixmap(widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                pixmap,
                event->area.x, event->area.y,
                event->area.x, event->area.y,
                event->area.width, event->area.height);
        return FALSE;
}

GtkWidget *netload_new( void )
{
	GtkWidget *frame, *box;

	box = gtk_vbox_new(FALSE, FALSE);
	gtk_widget_show(box);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame), props.look?GTK_SHADOW_OUT:GTK_SHADOW_IN );

	disp = gtk_drawing_area_new();
	gtk_signal_connect( GTK_OBJECT(disp), "expose_event",
                (GtkSignalFunc)netload_expose, NULL);
        gtk_signal_connect( GTK_OBJECT(disp),"configure_event",
                (GtkSignalFunc)netload_configure, NULL);
        gtk_widget_set_events( disp, GDK_EXPOSURE_MASK );

	gtk_box_pack_start_defaults( GTK_BOX(box), disp );
	gtk_container_add( GTK_CONTAINER(frame), box );

	gtk_widget_set_usize(disp, props.width, props.height);

	start_timer();
        
        gtk_widget_show_all(frame);
	return frame;
}

void start_timer( void )
{
	if( timer_index != -1 )
		gtk_timeout_remove(timer_index);

	timer_index = gtk_timeout_add(props.speed, (GtkFunction)draw, NULL);
}

void setup_colors(void)
{
	GdkColormap *colormap;

	colormap = gtk_widget_get_colormap(disp);
                
        gdk_color_parse(props.gcolor, &gcolor);
        gdk_color_alloc(colormap, &gcolor);

        gdk_color_parse(props.bcolor, &bcolor);
        gdk_color_alloc(colormap, &bcolor);
}
	        
void create_gc(void)
{
        gc = gdk_gc_new( disp->window );
        gdk_gc_copy( gc, disp->style->white_gc );
}

static gint applet_session_save(GtkWidget *widget, char *cfgpath, char *globcfgpath, gpointer data)
{
	save_properties(cfgpath,&props);
	return FALSE;
}

void
error_close_cb(GtkWidget *widget, void *data)
{
	applet_widget_remove_from_panel(APPLET_WIDGET(widget));
}

/*
 * An error occured.
 */
void
error_dialog(char *message, int fatal)
{
	static GtkWidget	*error = NULL;
	GtkWidget	*less, *label;

	if (error){
		return;
	}

	error = gnome_dialog_new(_("Netload Error"), GNOME_STOCK_BUTTON_CLOSE, NULL);
	gnome_dialog_set_close(GNOME_DIALOG(error), TRUE);
	gnome_dialog_close_hides(GNOME_DIALOG(error), TRUE);

	less = gnome_less_new();
	label = gtk_label_new(_("An error occured in the Netload Applet:"));

	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(error)->vbox), label,
		FALSE, FALSE, GNOME_PAD);
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(error)->vbox), less,
		TRUE, TRUE, GNOME_PAD);

	gnome_less_fixed_font(GNOME_LESS(less));
	gnome_dialog_set_modal(GNOME_DIALOG(error));

	if (fatal)
		gtk_signal_connect( GTK_OBJECT(error),
			"clicked", GTK_SIGNAL_FUNC(error_close_cb), NULL );

	gtk_widget_show(less);
	gtk_widget_show(label);
	gtk_widget_show(error);
	gnome_less_show_string(GNOME_LESS(less), message);
}

static void
about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	gchar *authors[] = {
		"Stephen Norris (srn@fn.com.au)",
	  NULL
	  };

	about = gnome_about_new (_("The GNOME Network Load Applet"), "0.0.3",
			"(C) 1998 Stephen Norris",
			authors,
			_("This applet is released under the terms and conditions of the GNU Public Licence."
			"This applet shows the load on a network device. "
			"It requires the /proc/net/ip_acct interface to be present and "
			"set up correctly for the device."),
			NULL);
	gtk_widget_show (about);

	return;
}

int main(int argc, char **argv)
{
	GtkWidget *applet;

	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	applet_widget_init_defaults("netload_applet", NULL, argc, argv, 0,
				    NULL,argv[0]);

	applet = applet_widget_new();
	if (!applet)
		g_error(_("Can't create applet!\n"));

	load_properties(APPLET_WIDGET(applet)->cfgpath, &props);
	
	netload = netload_new();
        applet_widget_add( APPLET_WIDGET(applet), netload );
        gtk_widget_show(applet);
	
	create_gc();
	setup_colors();
 
	gtk_signal_connect(GTK_OBJECT(applet),"session_save",
                           GTK_SIGNAL_FUNC(applet_session_save),
                           NULL);
	
       	applet_widget_register_callback(APPLET_WIDGET(applet),
					     "about",
                                             _("About..."),
                                             about_cb,
                                             NULL);

       	applet_widget_register_callback(APPLET_WIDGET(applet),
					     "properties",
                                             _("Properties..."),
                                             properties,
                                             NULL);

	applet_widget_gtk_main();
        return 0;
}
