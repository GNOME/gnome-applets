/*
 * GNOME diskusage panel applet
 * (C) 1997 The Free Software Foundation
 * 
 * Author: Bruno Widmann
 * 	   Modified from code taken from 
 * 	   cpumemusage & cpuload applets.
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
#include <applet-widget.h>

#include "diskusage.h"
#include "properties.h"
#include "mountlist.h"

/*#define DU_DEBUG*/


diskusage_properties props;

static gint update_values ();

DiskusageInfo   summary_info;

static GtkWidget *diskusage;
static GtkWidget *my_frame;  /* the frame wherer hbox or vbox reside   */
static GtkWidget *my_applet;
GtkWidget *disp;
GdkPixmap *pixmap;
GdkGC *gc;
GdkColor ucolor, fcolor, tcolor, bcolor;

int timer_index=-1;

int first_time=1;

void diskusage_resize ()
{
	gtk_widget_set_usize(disp, props.width, props.height);
}

/*
 * for horizontal panel
 */
int draw_h(void)
{
	int i, l;

	GdkFont* my_font;
	char *text;
	double ratio;		/* % of space used */
	int pie_width;		/* width+height of piechart */
	int pie_spacing;	/* space between piechart and border */
	int sel_fs;		/* # of selected filesystem */
	gchar avail_buf1[100];  /* used for mountpoint text */
	gchar avail_buf2[100];  /* used for free-space text */
	
	my_font = gdk_font_load      ("fixed");

	
	sel_fs = summary_info.selected_filesystem;
	
	pie_width = disp->allocation.height - DU_PIE_GAP;
	pie_spacing = DU_PIE_GAP / 2;



	/* Mountpoint text */
	text = summary_info.filesystems[sel_fs].mount_dir;

	strcpy(avail_buf1, "MP: ");
	strcat(avail_buf1, text);


	/* Free Space text */		        
	sprintf (avail_buf2,"av: %u\0", summary_info.filesystems[sel_fs].sizeinfo[2]);



	gdk_gc_set_foreground( gc, &bcolor );
	
	/* Erase Rectangle */
	gdk_draw_rectangle( pixmap,
		gc,
		TRUE, 0,0,
		disp->allocation.width,
		disp->allocation.height );


	gdk_gc_set_foreground( gc, &tcolor );
	
	/* draw text strings */
	gdk_draw_string(pixmap, my_font, gc,
			DU_MOUNTPOINT_X + pie_width + DU_PIE_GAP, 
			DU_MOUNTPOINT_Y,
			avail_buf1);
	
	gdk_draw_string(pixmap, my_font, gc,
			DU_FREESPACE_X + pie_width + DU_PIE_GAP, 
			DU_FREESPACE_Y,
			avail_buf2);
	
	/* Draw % usage Pie */
	gdk_gc_set_foreground( gc, &ucolor );
	
	
	ratio = ((double) summary_info.filesystems[sel_fs].sizeinfo[1]
		 / (double) summary_info.filesystems[sel_fs].sizeinfo[0]);

	/* 
	 * ratio      0..1   used space
	 * (1-ratio)  0..1   free space
	 */

	gdk_draw_arc (pixmap,
			gc,
			1,		/* filled = true */
			pie_spacing, pie_spacing, pie_width, pie_width,
			90 * 64,
			- (360 * ratio) * 64);

	gdk_gc_set_foreground( gc, &fcolor );
	gdk_draw_arc (pixmap,
			gc,
			1,		/* filled = true */
			pie_spacing, pie_spacing, pie_width, pie_width,
			(90 + 360 * (1 - ratio)) * 64,
			- (360 * (1 -ratio)) * 64);

	
	
	gdk_draw_pixmap(disp->window,
		disp->style->fg_gc[GTK_WIDGET_STATE(disp)],
	        pixmap,
	        0, 0,
	        0, 0,
	        disp->allocation.width,
	        disp->allocation.height);


	gdk_font_unref(my_font);

	return TRUE;
}

/*
 * for vertical panel
 */
int draw_v(void)
{
	int i, l;

	GdkFont* my_font;
	char *text;
	double ratio;		/* % of space used */
	int pie_width;		/* width+height of piechart */
	int pie_spacing;	/* space between piechart and border */
	int sel_fs;		/* # of selected filesystem */
	gchar avail_buf1[100];  /* used for mountpoint text */
	gchar avail_buf2[100];  /* used for free-space text */
	
	my_font = gdk_font_load      ("fixed");

	
	sel_fs = summary_info.selected_filesystem;
	
	pie_width = disp->allocation.width - DU_PIE_GAP;
	pie_spacing = DU_PIE_GAP / 2;



	/* Mountpoint text, part 1*/

	strcpy(avail_buf1, "MP: ");


	/* Free Space text, part1*/		        
	sprintf (avail_buf2,"av: \0", summary_info.filesystems[sel_fs].sizeinfo[2]);


	
	gdk_gc_set_foreground( gc, &bcolor );

	/* Erase Rectangle */
	gdk_draw_rectangle( pixmap,
		gc,
		TRUE, 0,0,
		disp->allocation.width,
		disp->allocation.height );


	gdk_gc_set_foreground( gc, &tcolor );
	
	/* draw text strings */
	gdk_draw_string(pixmap, my_font, gc,
			DU_MOUNTPOINT_X, 
			DU_MOUNTPOINT_Y_VERT + pie_width + DU_PIE_GAP,
			avail_buf1);
	
	gdk_draw_string(pixmap, my_font, gc,
			DU_FREESPACE_X, 
			DU_FREESPACE_Y_VERT + pie_width + DU_PIE_GAP,
			avail_buf2);
	
	/* Mountpoint text, part 2*/
	text = summary_info.filesystems[sel_fs].mount_dir;

	strcpy(avail_buf1, text);


	/* Free Space text, part2*/		        
	sprintf (avail_buf2,"%u\0", summary_info.filesystems[sel_fs].sizeinfo[2]);

	/* draw text strings 2nd part*/
	gdk_draw_string(pixmap, my_font, gc,
			DU_MOUNTPOINT_X, 
			DU_MOUNTPOINT2_Y_VERT + pie_width + DU_PIE_GAP,
			avail_buf1);
	
	gdk_draw_string(pixmap, my_font, gc,
			DU_FREESPACE_X, 
			DU_FREESPACE2_Y_VERT + pie_width + DU_PIE_GAP,
			avail_buf2);
	
	/* Draw % usage Pie */
	gdk_gc_set_foreground( gc, &ucolor );
	
	
	ratio = ((double) summary_info.filesystems[sel_fs].sizeinfo[1]
		 / (double) summary_info.filesystems[sel_fs].sizeinfo[0]);

	/* 
	 * ratio      0..1   used space
	 * (1-ratio)  0..1   free space
	 */

	gdk_draw_arc (pixmap,
			gc,
			1,		/* filled = true */
			pie_spacing, pie_spacing, pie_width, pie_width,
			90 * 64,
			- (360 * ratio) * 64);

	gdk_gc_set_foreground( gc, &fcolor );
	gdk_draw_arc (pixmap,
			gc,
			1,		/* filled = true */
			pie_spacing, pie_spacing, pie_width, pie_width,
			(90 + 360 * (1 - ratio)) * 64,
			- (360 * (1 -ratio)) * 64);

	
	
	gdk_draw_pixmap(disp->window,
		disp->style->fg_gc[GTK_WIDGET_STATE(disp)],
	        pixmap,
	        0, 0,
	        0, 0,
	        disp->allocation.width,
	        disp->allocation.height);

	gdk_font_unref(my_font);
	return TRUE;
}



int draw(void)
{

	if (summary_info.orient == ORIENT_LEFT || summary_info.orient == ORIENT_RIGHT)
		draw_v();
	else
		draw_h();

}


void setup_colors(void)
{
	GdkColormap *colormap;

	colormap = gtk_widget_get_colormap(disp);
                
        gdk_color_parse(props.ucolor, &ucolor);
        gdk_color_alloc(colormap, &ucolor);

        gdk_color_parse(props.fcolor, &fcolor);
        gdk_color_alloc(colormap, &fcolor);
        
	gdk_color_parse(props.tcolor, &tcolor);
        gdk_color_alloc(colormap, &tcolor);
	
	gdk_color_parse(props.bcolor, &bcolor);
        gdk_color_alloc(colormap, &bcolor);
}
	        
void create_gc(void)
{
        gc = gdk_gc_new( disp->window );
        gdk_gc_copy( gc, disp->style->white_gc );
}



void start_timer( void )
{
#ifdef DU_DEBUG
	printf ("restarting timer with %u \n", props.speed);
#endif
	if( timer_index != -1 )
		gtk_timeout_remove(timer_index);

	timer_index = gtk_timeout_add(props.speed, (GtkFunction)update_values, NULL);
}





static void
applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{

	guint tmp;

#ifdef DU_DEBUG
	printf ("entering applet_change_orient  \n");
#endif
	summary_info.orient = o;

	/*
	 * swap width <-> height of applet, when width is > then height
	 * and vertical panel, or when it is < on horizontal panel
	 */

	if (o == ORIENT_LEFT || o == ORIENT_RIGHT) {
		if (props.width > props.height) {
			tmp = props.width;
			props.width = props.height;
			props.height = tmp;
		}
	} else {
		if (props.width < props.height) {
			tmp = props.width;
			props.width = props.height;
			props.height = tmp;
		}
	}


	diskusage_resize();

#ifdef DU_DEBUG
	printf ("leaving applet_change_orient  \n");
#endif
}


/*
 * read new filesystem-info, and call draw
 */
static gint update_values ()
{

#ifdef DU_DEBUG
	printf ("update\n");
#endif

	diskusage_read (&summary_info);
	draw();
	
	return TRUE;
}

static gint diskusage_configure(GtkWidget *widget, GdkEventConfigure *event)
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

static gint diskusage_expose(GtkWidget *widget, GdkEventExpose *event)
{
        gdk_draw_pixmap(widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                pixmap,
                event->area.x, event->area.y,
                event->area.x, event->area.y,
                event->area.width, event->area.height);
        return FALSE;
}

static gint diskusage_clicked_cb(GtkWidget * widget, GdkEventButton * e, 
				gpointer data) {

	if (e->button != 1) {
		/* Ignore buttons 2 and 3 */
		return FALSE; 
	}

	
	summary_info.selected_filesystem++;
	
	if (summary_info.selected_filesystem >= summary_info.n_filesystems)
		summary_info.selected_filesystem = 0;


	
	
	/* call draw when user left-clicks applet, to avoid delay */
	update_values();
	
	return TRUE; 
}

GtkWidget *diskusage_widget(void)
{
	int i;
	
	GtkWidget *frame, *box;
	GtkWidget *event_box;

#ifdef DU_DEBUG
	printf ("entering diskusage_widget  \n");
#endif
	

	/*
	 * default to horizontal panel, as long as we don't know for sure
	 */
	/*summary_info.orient = ORIENT_UP;*/

	summary_info.selected_filesystem = 0;

	/*
	diskusage_read (&summary_info);
	summary_info.previous_n_filesystems = summary_info.n_filesystems; 
	*/

	box = gtk_vbox_new(FALSE, FALSE);
	gtk_widget_show(box);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame), props.look?GTK_SHADOW_OUT:GTK_SHADOW_IN );

	disp = gtk_drawing_area_new();
	gtk_signal_connect( GTK_OBJECT(disp), "expose_event",
                (GtkSignalFunc)diskusage_expose, NULL);
        gtk_signal_connect( GTK_OBJECT(disp),"configure_event",
                (GtkSignalFunc)diskusage_configure, NULL);
        gtk_widget_set_events( disp, GDK_EXPOSURE_MASK );

	gtk_box_pack_start_defaults( GTK_BOX(box), disp );

	event_box = gtk_event_box_new();
	gtk_widget_show(event_box);
	gtk_widget_set_events(event_box, GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(event_box), "button_press_event",
			   GTK_SIGNAL_FUNC(diskusage_clicked_cb), NULL);

	gtk_container_add( GTK_CONTAINER(event_box), box );
	gtk_container_add( GTK_CONTAINER(frame), event_box);
	
	diskusage_resize();

	start_timer();
        
        gtk_widget_show_all(frame);
	

	return frame;
	

}

static void
about_cb (AppletWidget *widget, gpointer data)
{
	GtkWidget *about;
	gchar *authors[] = {
		"Bruno Widmann (bwidmann@tks.fh-sbg.ac.at)",
		"Martin Baulig (martin@home-of-linux.org)",
	  NULL
	  };

	about = gnome_about_new ( "The GNOME Disk Usage Applet", "0.1.0",
			"(C) 1998",
			authors,
			"This applet is released under "
			"the terms and conditions of the "
			"GNU Public Licence."
			"\nShows a pie with the used and free space "
			"for the selected filesystem.  ",
			NULL);
	gtk_widget_show (about);

	return;
}

static gint applet_session_save(GtkWidget *widget, char *cfgpath, char *globcfgpath, gpointer data)
{
	save_properties(cfgpath,&props);
	return FALSE;
}

int main(int argc, char **argv)
{
	GtkWidget *applet;
	
	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

        applet_widget_init_defaults("diskusage_applet", NULL, argc, argv, 0,
				    NULL,argv[0]);

	applet = applet_widget_new();
	if (!applet)
		g_error("Can't create applet!\n");

	my_applet = applet;
        
	load_properties(APPLET_WIDGET(applet)->cfgpath, &props);

        diskusage = diskusage_widget();

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   NULL);

        applet_widget_add( APPLET_WIDGET(applet), diskusage );


        gtk_widget_show(applet);
	
	create_gc();
	setup_colors();
       	
	applet_widget_register_callback(APPLET_WIDGET(applet),
					     "about",
                                             _("About..."),
                                             about_cb,
                                             NULL);
        
	gtk_signal_connect(GTK_OBJECT(applet),"session_save",
                           GTK_SIGNAL_FUNC(applet_session_save),
                           NULL);
       	
	applet_widget_register_callback(APPLET_WIDGET(applet),
					"properties",
                                        _("Properties..."),
                                        properties,
                                        NULL);


	
	applet_widget_gtk_main();


        return 0;
}
