/*
 * GNOME cpuload panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Author: Tim P. Gerla
 * With code from wmload.c, v0.9.2, apparently by Ryan Land, rland@bc1.com.
 *
 */

#include <stdio.h>
#ifdef HAVE_LIBINTL
#    include <libintl.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include "applet-lib.h"
#include "panel.h"
#include "mico-parse.h"

#include "linux-proc.h"

GtkWidget *cpuload;
GdkPixmap *pixmap;
GtkWidget *disp;
GdkGC *gc;
GdkColor darkgrey;

int applet_id = -1;

#define APPLET_HEIGHT 40
#define APPLET_WIDTH  40

guchar data[APPLET_WIDTH];
guchar odata[APPLET_WIDTH];

int draw(void)
{
	int usr=0, sys=0, nice=0, free=0, i;

	GetLoad(APPLET_HEIGHT, &usr, &nice, &sys, &free );

	for( i=0; i < APPLET_WIDTH-1; i++ )
		data[i+1] = odata[i];
	data[0] = usr;
	
	/* Erase Rectangle */
	gdk_draw_rectangle( pixmap,
		disp->style->black_gc,
		TRUE, 0,0,
		disp->allocation.width,
		disp->allocation.height );
	for( i=0; i < APPLET_WIDTH; i++ )
	{
		if( data[i] )
			gdk_draw_line( pixmap,
			       gc,
			       i,APPLET_HEIGHT,
			       i,(APPLET_HEIGHT-data[i]) );
	}
	gdk_draw_pixmap(disp->window,
		disp->style->fg_gc[GTK_WIDGET_STATE(disp)],
	        pixmap,
	        0, 0,
	        0, 0,
	        disp->allocation.width,
	        disp->allocation.height);

	for( i=0; i < APPLET_WIDTH; i++ )
		odata[i] = data[i];
	return TRUE;
}

static gint cpuload_configure(GtkWidget *widget, GdkEventConfigure *event)
{
        pixmap = gdk_pixmap_new( widget->window,
                                 widget->allocation.width,
                                 widget->allocation.height,
                                 -1 );
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

static gint cpuload_expose(GtkWidget *widget, GdkEventExpose *event)
{
        gdk_draw_pixmap(widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                pixmap,
                event->area.x, event->area.y,
                event->area.x, event->area.y,
                event->area.width, event->area.height);
        return FALSE;
}

GtkWidget *cpuload_new( void )
{
	GtkWidget *frame, *box;

	g_print( "entering cpuload_new\n" );

	box = gtk_vbox_new(FALSE, FALSE);
	gtk_widget_show(box);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame), GTK_SHADOW_IN );

	disp = gtk_drawing_area_new();
	gtk_signal_connect( GTK_OBJECT(disp), "expose_event",
                (GtkSignalFunc)cpuload_expose, NULL);
        gtk_signal_connect( GTK_OBJECT(disp),"configure_event",
                (GtkSignalFunc)cpuload_configure, NULL);
        gtk_widget_set_events( disp, GDK_EXPOSURE_MASK
        				  | GDK_LEAVE_NOTIFY_MASK
					  | GDK_BUTTON_PRESS_MASK
					  | GDK_POINTER_MOTION_MASK 
                			  | GDK_POINTER_MOTION_HINT_MASK);

	gtk_box_pack_start_defaults( GTK_BOX(box), disp );
	gtk_container_add( GTK_CONTAINER(frame), box );

	gtk_widget_set_usize(disp, APPLET_WIDTH, APPLET_HEIGHT);

        gtk_timeout_add(3000, (GtkFunction)draw, NULL);
        
        gtk_widget_show_all(frame);
	return frame;
}

void create_gc(void)
{
	GdkColormap *colormap;

        gc = gdk_gc_new( disp->window );
        gdk_gc_copy( gc, disp->style->white_gc );
        
	colormap = gtk_widget_get_colormap(disp);
                
        gdk_color_parse("#606060", &darkgrey);
        gdk_color_alloc(colormap, &darkgrey);
                                
}

static gint
destroy_plug(GtkWidget *widget, gpointer data)
{
	gtk_exit(0);
	return FALSE;
}

int main(int argc, char **argv)
{
	GtkWidget *plug;
        char *result, *cfgpath, *globcfgpath, *myinvoc;
	guint32 winid;

	myinvoc = get_full_path(argv[0]);
        if(!myinvoc)
                return 1;
        panel_corba_register_arguments();

        gnome_init("cpuload_applet", NULL, argc, argv, 0, NULL);

	if (!gnome_panel_applet_init_corba())
                g_error("Could not comunicate with the panel\n");
        result = gnome_panel_applet_request_id(myinvoc, &applet_id,
                                               &cfgpath, &globcfgpath,
                                               &winid);
        g_free(myinvoc);
        if (result)
                g_error("Could not talk to the Panel: %s\n", result);

        g_free(globcfgpath);
        g_free(cfgpath);

	plug = gtk_plug_new(winid);
        cpuload = cpuload_new();
        gtk_container_add( GTK_CONTAINER(plug), cpuload );
        gtk_widget_show(plug);
	gtk_signal_connect(GTK_OBJECT(plug),"destroy",
			   GTK_SIGNAL_FUNC(destroy_plug),
			   NULL);
	
	create_gc();

        result = gnome_panel_applet_register(plug, applet_id);
        if (result)
                g_error("Could not talk to the Panel: %s\n", result);
	
	applet_corba_gtk_main("IDL:GNOME/Applet:1.0");

        return 0;
}

/*these are commands sent over corba: */
void
change_orient(int id, int orient)
{
}
                                        
int session_save(int id, const char *cfgpath, const char *globcfgpath)
{
	/*save the session here */
        return TRUE;
}
