/*
 * GNOME diskusage panel applet
 * (C) 1997 The Free Software Foundation
 *
 * Shows a pie with the used and free space
 * for the selected filesystem.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *                                
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *           
 * Authors: 	Bruno Widmann
 *		Martin Baulig - libgtop support
 *		Dave Finton - mountlist menu
 * 
 * With code from cpumemusage & cpuload applets.
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
#include <applet-widget.h>

#include "diskusage.h"
#include "properties.h"


/* Many systems reserve some space on each filesystem that only the superuser
 * can use. Set this to true if you want count this space towards the among of
 * free space. This would be the opposite of what 'df' does. */
#undef ADD_RESERVED_SPACE

diskusage_properties props;

static gint update_values (void);

DiskusageInfo   summary_info;
	
glibtop_fsusage fsu;
glibtop_mountlist mountlist;
glibtop_mountentry *mount_list;

static GtkWidget *diskusage;
static GtkWidget *my_applet;
GtkWidget *disp;
GdkPixmap *pixmap;
GdkGC *gc;
GdkColor ucolor, fcolor, tcolor, bcolor;

void update_mount_list_menu_items (void);
void diskusage_resize (void);
void diskusage_read (void);
int draw_h(void);
int draw_v(void);
void draw(void);
void setup_colors(void);
void create_gc(void);
void start_timer(void);
void change_filesystem_cb (AppletWidget *applet, gpointer data);
void add_mount_list_menu_items (void);
#if 0
static void browse_cb (AppletWidget *widget, gpointer data);
#endif
int diskusage_get_best_size_v (void);
int diskusage_get_best_size_h (void);

GtkWidget *diskusage_widget(void);

GString **mpoints;
GString **menuitem;
guint num_mpoints;

int timer_index=-1;

int diskusage_get_best_size_v ()
{
	GdkFont* my_font;
	int string_height;
	int du_pie_gap;
	int du_mountpoint_x;
	int du_freespace_x;
	int total_height;
	int pie_width;		/* width+height of piechart */


	my_font = gdk_font_load      ("fixed");

	if (summary_info.pixel_size <= PIXEL_SIZE_TINY) {
		du_pie_gap = du_mountpoint_x = du_freespace_x = 0;
	} else if (summary_info.pixel_size <= PIXEL_SIZE_SMALL) {
		du_pie_gap = du_mountpoint_x = du_freespace_x = 1;
	} else {
		du_pie_gap = DU_PIE_GAP;
		du_mountpoint_x = DU_MOUNTPOINT_X;
		du_freespace_x = DU_FREESPACE_X;
	}
	
	pie_width = summary_info.pixel_size - du_pie_gap;

	string_height = gdk_string_height (my_font, "MP: ");

	total_height = DU_FREESPACE2_Y_VERT + pie_width + du_pie_gap +
		string_height * 4;

	return total_height;
}

int diskusage_get_best_size_h ()
{
	GdkFont* my_font;
	int string_width;
	int cur_width;
	int du_pie_gap;
	int total_width;
	int pie_width;		/* width+height of piechart */
	gchar avail_buf[100];	/* used for mountpoint text */
	char *text;
	int i;

	glibtop_mountlist my_mountlist;
	glibtop_mountentry *my_mount_list;


	my_font = gdk_font_load      ("fixed");

	if (summary_info.pixel_size <= PIXEL_SIZE_TINY) {
		du_pie_gap = 0;
	} else if (summary_info.pixel_size <= PIXEL_SIZE_SMALL) {
		du_pie_gap = 1;
	} else {
		du_pie_gap = DU_PIE_GAP;
	}
	
	pie_width = summary_info.pixel_size - du_pie_gap;

	string_width = 0;


	memset (&my_mountlist, 0, sizeof (glibtop_mountlist));
	my_mount_list = glibtop_get_mountlist (&my_mountlist, 1);
	
	for (i = 0; i < mountlist.number; i++) {
	    text = my_mount_list [i].mountdir;
	    strcpy(avail_buf, "MP: ");
	    strcat(avail_buf, text);

	    cur_width = gdk_string_width (my_font, avail_buf);

	    if (cur_width > string_width)
		string_width = cur_width;
	}

	glibtop_free (my_mount_list);

	cur_width = gdk_string_width (my_font, "av: 9999.999 MB");
	if (cur_width > string_width)
	    string_width = cur_width;

	total_width = pie_width + du_pie_gap + DU_FREESPACE_HOR_X * 2 +
		string_width + du_pie_gap;

	return total_width;
}


void diskusage_resize ()
{
	if ((summary_info.orient == ORIENT_LEFT) ||
	    (summary_info.orient == ORIENT_RIGHT))
		if (props.best_size)
			gtk_widget_set_usize (my_applet, summary_info.pixel_size,
					      diskusage_get_best_size_v ());
		else
			gtk_widget_set_usize (my_applet, summary_info.pixel_size,
					      props.size);
	else
		if (props.best_size)
			gtk_widget_set_usize (my_applet,
					      diskusage_get_best_size_h (),
					      summary_info.pixel_size);
		else
			gtk_widget_set_usize (my_applet, props.size,
					      summary_info.pixel_size);
}

/* Get list of currently mounted filesystems. */
void diskusage_read ()
{
	unsigned int i;

	if (mount_list != NULL)
		glibtop_free (mount_list);
	mount_list = glibtop_get_mountlist (&mountlist, 0);
	
	assert (mount_list != NULL);

	summary_info.n_filesystems = 0;
	for (i=0; i<mountlist.number; i++) {
		glibtop_get_fsusage (&fsu, mount_list [i].mountdir);
		if (fsu.blocks > 0)
			summary_info.n_filesystems++;
	}
}

/*
 * for horizontal panel
 */
int draw_h(void)
{

	GdkFont* my_font;
	char *text;
	unsigned free_space;
	int du_pie_gap;
	int du_mountpoint_y;
	int du_freespace_y;
	int string_height;
	int vert_spacing;
	double ratio;		/* % of space used */
	int pie_width;		/* width+height of piechart */
	int pie_spacing;	/* space between piechart and border */
	int sel_fs;		/* # of selected filesystem */
	gchar avail_buf1[100];  /* used for mountpoint text */
	gchar avail_buf2[100];  /* used for free-space text */
	
	my_font = gdk_font_load      ("fixed");

	
	sel_fs = summary_info.selected_filesystem;

	if (summary_info.pixel_size <= PIXEL_SIZE_TINY) {
		du_pie_gap = du_mountpoint_y = du_freespace_y = 0;
	} else if (summary_info.pixel_size <= PIXEL_SIZE_SMALL) {
		du_pie_gap = du_mountpoint_y = du_freespace_y = 1;
	} else {
		du_pie_gap = DU_PIE_GAP;
		du_mountpoint_y = DU_MOUNTPOINT_Y;
		du_freespace_y = DU_FREESPACE_Y;
	}
	
	pie_width = disp->allocation.height - du_pie_gap;
	pie_spacing = du_pie_gap / 2;

	string_height = gdk_string_height (my_font, "MP: ");

	vert_spacing = disp->allocation.height -
		(string_height * 2 + du_pie_gap);
	if (vert_spacing < 0) vert_spacing = 0;
	vert_spacing /= 3;

	/* Mountpoint text */
	text = mount_list [sel_fs].mountdir;

	strcpy(avail_buf1, "MP: ");
	strcat(avail_buf1, text);


	/* Free Space text */		        
	glibtop_get_fsusage (&fsu, mount_list [sel_fs].mountdir);

	fsu.blocks /= 2;
	fsu.bfree /= 2;
	fsu.bavail /= 2;

#ifdef ADD_RESERVED_SPACE
	free_space = fsu.bfree; /* Free blocks available to superuser. */
#else
	free_space = fsu.bavail; /* Free blocks available to non-superuser. */
#endif

	if (free_space > 1048471142) /* 9999 MB */
	    g_snprintf (avail_buf2,sizeof(avail_buf2),"av: %.3f GB",
			(float)free_space / 1073741824.0);
	else if (free_space > 10238976) /* 9999 kB */
	    g_snprintf (avail_buf2,sizeof(avail_buf2),"av: %.3f MB",
			(float)free_space / 1048576.0);
	else if (free_space > 9999) /* 9999 Bytes */
	    g_snprintf (avail_buf2,sizeof(avail_buf2),"av: %.3f kB",
			free_space / 1024.0);
	else
	    g_snprintf (avail_buf2,sizeof(avail_buf2),"av: %u",
			free_space);

	/* g_snprintf (avail_buf2, sizeof(avail_buf2), "av: %u", free_space); */


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
			DU_MOUNTPOINT_HOR_X + pie_width + du_pie_gap, 
			string_height + vert_spacing,
			avail_buf1);
	
	gdk_draw_string(pixmap, my_font, gc,
			DU_FREESPACE_HOR_X + pie_width + du_pie_gap, 
			2 * (string_height + vert_spacing),
			avail_buf2);
	
	/* Draw % usage Pie */
	gdk_gc_set_foreground( gc, &ucolor );
	
	/* fsu.blocks 	Total blocks */	
	ratio = ((double) (fsu.blocks - free_space) / (double) fsu.blocks);

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

	GdkFont* my_font;
	char *text;
	unsigned free_space;
	double ratio;		/* % of space used */
	int du_pie_gap;
	int du_mountpoint_x;
	int du_freespace_x;
	int vert_spacing;	/* vertical spacing */
	int pie_width;		/* width+height of piechart */
	int pie_spacing;	/* space between piechart and border */
	int sel_fs;		/* # of selected filesystem */
	gchar avail_buf1[100];  /* used for mountpoint text */
	gchar avail_buf2[100];  /* used for free-space text */
	
	my_font = gdk_font_load      ("fixed");

	
	sel_fs = summary_info.selected_filesystem;

	if (summary_info.pixel_size <= PIXEL_SIZE_TINY) {
		du_pie_gap = du_mountpoint_x = du_freespace_x = 0;
	} else if (summary_info.pixel_size <= PIXEL_SIZE_SMALL) {
		du_pie_gap = du_mountpoint_x = du_freespace_x = 1;
	} else {
		du_pie_gap = DU_PIE_GAP;
		du_mountpoint_x = DU_MOUNTPOINT_X;
		du_freespace_x = DU_FREESPACE_X;
	}
	
	pie_width = disp->allocation.width - du_pie_gap;
	pie_spacing = du_pie_gap / 2;

	vert_spacing = disp->allocation.height -
		(DU_FREESPACE2_Y_VERT + pie_width + du_pie_gap);
	if (vert_spacing < 0) vert_spacing = 0;
	vert_spacing /= 2;

	/* Mountpoint text, part 1*/

	strcpy(avail_buf1, "MP: ");


	/* Free Space text, part1*/		        
	g_snprintf (avail_buf2,sizeof(avail_buf2),"av: ");


	
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
			du_mountpoint_x, 
			DU_MOUNTPOINT_Y_VERT + pie_width + du_pie_gap +
			vert_spacing,
			avail_buf1);
	
	gdk_draw_string(pixmap, my_font, gc,
			du_freespace_x, 
			DU_FREESPACE_Y_VERT + pie_width + du_pie_gap +
			vert_spacing,
			avail_buf2);
	
	/* Mountpoint text, part 2*/
	text = mount_list [sel_fs].mountdir;

	strcpy(avail_buf1, text);

	/* Free Space text, part2*/		        
	glibtop_get_fsusage (&fsu, mount_list [sel_fs].mountdir);

	fsu.blocks /= 2;
	fsu.bfree /= 2;
	fsu.bavail /= 2;

#ifdef ADD_RESERVED_SPACE
	free_space = fsu.bfree; /* Free blocks available to superuser. */
#else
	free_space = fsu.bavail; /* Free blocks available to non-superuser. */
#endif
	if (summary_info.pixel_size <= PIXEL_SIZE_STANDARD) {
		if (free_space > 1048471142) /* 9999 MB */
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%uG",
				    free_space >> 30);
		else if (free_space > 10238976) /* 9999 kB */
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%uM",
				    free_space >> 20);
		else if (free_space > 9999) /* 9999 Bytes */
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%uk",
				    free_space >> 10);
		else
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%u",
				    free_space);

	} else if (summary_info.pixel_size <= PIXEL_SIZE_LARGE) {
		if (free_space > 1048471142) /* 9999 MB */
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%.1f GB",
				    (float)free_space / 1073741824.0);
		else if (free_space > 10238976) /* 9999 kB */
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%.1f MB",
				    (float)free_space / 1048576.0);
		else if (free_space > 9999) /* 9999 Bytes */
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%.1f kB",
				    free_space / 1024.0);
		else
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%u",
				    free_space);
	} else {
		if (free_space > 1048471142) /* 9999 MB */
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%.3f GB",
				    (float)free_space / 1073741824.0);
		else if (free_space > 10238976) /* 9999 kB */
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%.3f MB",
				    (float)free_space / 1048576.0);
		else if (free_space > 9999) /* 9999 Bytes */
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%.3f kB",
				    free_space / 1024.0);
		else
			g_snprintf (avail_buf2,sizeof(avail_buf2),"%u",
				    free_space);
	}

	/* draw text strings 2nd part*/
	gdk_draw_string(pixmap, my_font, gc,
			du_mountpoint_x, 
			DU_MOUNTPOINT2_Y_VERT + pie_width + du_pie_gap +
			vert_spacing,
			avail_buf1);
	
	gdk_draw_string(pixmap, my_font, gc,
			du_freespace_x, 
			DU_FREESPACE2_Y_VERT + pie_width + du_pie_gap +
			vert_spacing,
			avail_buf2);
	
	/* Draw % usage Pie */
	gdk_gc_set_foreground( gc, &ucolor );
	
	
	/* fsu.blocks 	Total blocks */	
	ratio = ((double) (fsu.blocks - free_space) / (double) fsu.blocks);

	/* 
	 * ratio      0..1   used space
	 * (1-ratio)  0..1   free space
	 */

	gdk_draw_arc (pixmap,
			gc,
			1,		/* filled = true */
			pie_spacing, pie_spacing + (vert_spacing / 2),
		        pie_width, pie_width,
			90 * 64,
			- (360 * ratio) * 64);

	gdk_gc_set_foreground( gc, &fcolor );
	gdk_draw_arc (pixmap,
			gc,
			1,		/* filled = true */
			pie_spacing, pie_spacing + (vert_spacing / 2),
		        pie_width, pie_width,
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



void draw(void)
{
	if (summary_info.orient == ORIENT_LEFT ||
	    summary_info.orient == ORIENT_RIGHT)
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
	if( timer_index != -1 )
		gtk_timeout_remove(timer_index);

	timer_index = gtk_timeout_add(props.speed, (GtkFunction)update_values, NULL);
}





static void
applet_change_orient(AppletWidget *w, PanelOrientType o, gpointer data)
{
	summary_info.orient = applet_widget_get_panel_orient (w);

	diskusage_resize();
}

static void
applet_change_pixel_size(AppletWidget *w, int size, gpointer data)
{
	summary_info.pixel_size = applet_widget_get_panel_pixel_size (w);

	diskusage_resize();
}



/*
 * read new filesystem-info, and call draw
 */
static gint update_values (void)
{


	diskusage_read ();

	/* if selected filesystem was last, and that got unmounted
	 * meanwhile
	 */
	if (summary_info.selected_filesystem >= mountlist.number)
		summary_info.selected_filesystem = 0;

	update_mount_list_menu_items ();
	
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
	event = NULL;
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

/* Left click on the applet switches to next filesystem */
static gint diskusage_clicked_cb(GtkWidget * widget, GdkEventButton * e, 
				gpointer data) {
	unsigned int n;

	if (e->button != 1) {
		/* Ignore buttons 2 and 3 */
		return FALSE; 
	}


	n = 0;

	do {
		summary_info.selected_filesystem++;
		
		if (summary_info.selected_filesystem >= mountlist.number)
			summary_info.selected_filesystem = 0;

		glibtop_get_fsusage (&fsu, 
				mount_list [summary_info.selected_filesystem].mountdir);
		n++;
	/* skip over filesystems with 0 blocks (like /proc) */
	} while ((fsu.blocks == 0) & (n < mountlist.number));

	props.startfs = summary_info.selected_filesystem;
	
	
	/* call draw when user left-clicks applet, to avoid delay */
	update_values();
	
	return TRUE; 
	widget = NULL;
	data = NULL;
}

void change_filesystem_cb (AppletWidget *applet, gpointer data) {

  gchar *my_mpoint = (gchar *)data;

  guint n_mpoints = mountlist.number;
  guint lim1;

  for (lim1 = 0; lim1 < n_mpoints; lim1++)
    if (!strcmp (my_mpoint, mount_list [lim1].mountdir))
      break;

  summary_info.selected_filesystem = lim1;
  props.startfs = summary_info.selected_filesystem;

  update_values ();
  return;
  applet = NULL;
}

void add_mount_list_menu_items (void) {

  guint n_mpoints = mountlist.number;
  guint lim1;

  gchar digit1 = '0', digit2 = '0';

  mpoints = g_new0 (GString *, n_mpoints);
  menuitem = g_new0 (GString *, n_mpoints);

  applet_widget_register_callback_dir (APPLET_WIDGET (my_applet),
				       "filesystem",
				       _("File Systems"));

  for (lim1 = 0; lim1 < n_mpoints; lim1++) {
    
    mpoints [lim1] = g_string_new (mount_list [lim1].mountdir);

    menuitem [lim1] = g_string_new ("filesystem/fsitem");
    g_string_append_c (menuitem [lim1], digit1);
    g_string_append_c (menuitem [lim1], digit2);

    if (digit2 == '9') {
      digit1++;
      digit2 = '0';
    }
    else
      digit2++;
    
    /* don't register entrys with total blocks=0, like /proc */
    glibtop_get_fsusage (&fsu, mount_list [lim1].mountdir);
    if (fsu.blocks == 0)
	    continue;

    applet_widget_register_callback (APPLET_WIDGET (my_applet),
				     menuitem [lim1]->str,
				     mpoints [lim1]->str,
				     change_filesystem_cb,
				     mpoints [lim1]->str);

  }

  num_mpoints = n_mpoints;

}

void update_mount_list_menu_items () {

  guint n_mpoints = mountlist.number;
  guint lim1;
  int retval = TRUE;

  gchar digit1 = '0', digit2 = '0';

  if (num_mpoints != n_mpoints)
    retval = FALSE;

  for (lim1 = 0; (lim1 < n_mpoints) && retval; lim1++)
    if (strcmp (mpoints [lim1]->str, mount_list [lim1].mountdir)) {
      retval = FALSE;
      break;
    }

  if (!retval) {

    printf (_("File System Changed!\n"));

    for (lim1 = 0; lim1 < num_mpoints; lim1++) {

      /* This causes a sigsegv if the menu is actually open... dunno
	 what to do about it :) */
    
      /* don't unregister entrys with total blocks=0, like /proc */
      glibtop_get_fsusage (&fsu, mount_list [lim1].mountdir);
      if (fsu.blocks > 0)
      	applet_widget_unregister_callback (APPLET_WIDGET (my_applet), 
					 menuitem [lim1]->str);

      g_string_free (mpoints [lim1], TRUE);
      g_string_free (menuitem [lim1], TRUE);

    }

    g_free (mpoints);
    g_free (menuitem);

    num_mpoints = n_mpoints;

    mpoints = g_new0 (GString *, n_mpoints);
    menuitem = g_new0 (GString *, n_mpoints);

    for (lim1 = 0; lim1 < num_mpoints; lim1++) {

      mpoints [lim1] = g_string_new (mount_list [lim1].mountdir);

      menuitem [lim1] = g_string_new ("filesystem/fsitem");
      g_string_append_c (menuitem [lim1], digit1);
      g_string_append_c (menuitem [lim1], digit2);

      if (digit2 == '9') {
	digit1++;
	digit2 = '0';
      }
      else
	digit2++;
   

      /* don't register entrys with total blocks=0, like /proc */
      glibtop_get_fsusage (&fsu, mount_list [lim1].mountdir);
      if (fsu.blocks == 0)
	    continue;

      applet_widget_register_callback (APPLET_WIDGET (my_applet),
				       menuitem [lim1]->str,
				       mpoints [lim1]->str,
				       change_filesystem_cb,
				       mpoints [lim1]->str);

    }

  }

}

#if 0

static void browse_cb (AppletWidget *widget, gpointer data)
{
        const char *buf[2];

        
	buf[0] = mount_list [summary_info.selected_filesystem].mountdir;
        buf[1] = NULL;

/*      I assume the first (commented out) call is mor correct, but:
 *      use  "IDL:GNOME/FileManagerWindow:1.0"  ?
 *      how do we correctly construct this string? In the event version != 1.0 ?
 */

/*      goad_server_activate_with_repo_id(NULL, "IDL:GNOME/FileManagerWindow:1.0",
                        GOAD_ACTIVATE_REMOTE | GOAD_ACTIVATE_ASYNC, buf);
*/

        goad_server_activate_with_id(NULL, "gmc_filemanager_window",
		0, buf);
	return;
	widget = NULL;
	data = NULL;
}

#endif

GtkWidget *diskusage_widget(void)
{
	
	GtkWidget *frame, *box;
	GtkWidget *event_box;


	summary_info.selected_filesystem = 0;
	
	diskusage_read ();

	/* We save the information about the selected filesystem. */

	summary_info.selected_filesystem = props.startfs;

	if (summary_info.selected_filesystem >= mountlist.number)
		summary_info.selected_filesystem = 0;

	props.startfs = summary_info.selected_filesystem;
	
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
	
	summary_info.orient = applet_widget_get_panel_orient (APPLET_WIDGET (my_applet));
	summary_info.pixel_size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (my_applet));

	diskusage_resize();

	start_timer();
        
        gtk_widget_show_all(frame);
	

	return frame;
	
}


static gint applet_save_session(GtkWidget *widget, char *privcfgpath, char *globcfgpath, gpointer data)
{
	save_properties(privcfgpath,&props);

	gnome_config_sync();
	/* you need to use the drop_all here since we're all writing to
	   one file, without it, things might not work too well */
	gnome_config_drop_all ();

	return FALSE;
}

int main(int argc, char **argv)
{
	GtkWidget *applet;
	
	/* Initialize the i18n stuff */
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

        applet_widget_init("diskusage_applet", VERSION, argc, argv,
				    NULL, 0, NULL);

	applet = applet_widget_new("diskusage_applet");
	if (!applet)
		g_error("Can't create applet!\n");

	my_applet = applet;
        
	load_properties(APPLET_WIDGET(applet)->privcfgpath, &props);

        diskusage = diskusage_widget();

	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   NULL);

	gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
			   GTK_SIGNAL_FUNC(applet_change_pixel_size),
			   NULL);

        applet_widget_add( APPLET_WIDGET(applet), diskusage );


        gtk_widget_show(applet);
	
	create_gc();
	setup_colors();
       	
	add_mount_list_menu_items ();
        
	gtk_signal_connect(GTK_OBJECT(applet),"save_session",
                           GTK_SIGNAL_FUNC(applet_save_session),
                           NULL);
       	
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					      "properties",
					      GNOME_STOCK_MENU_PROP,
					      _("Properties..."),
					      properties,
					      NULL);

#if 0
	applet_widget_register_stock_callback(APPLET_WIDGET(applet),
                                              "browse",
                                              GNOME_STOCK_MENU_OPEN,
                                              _("Browse..."),
                                              browse_cb,
                                              NULL);
#endif

	applet_widget_gtk_main();


        return 0;
}
