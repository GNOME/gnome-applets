#include <config.h>
#include "asclock.h"
#include <X11/X.h>
#include <X11/Intrinsic.h>
#ifdef ASCLOCK_GNOME
#include <gnome.h>
#endif
#include <gdk/gdkx.h>

/* the xpm's */
#include "default_theme/clock.xpm"
#include "default_theme/month.xpm"
#include "default_theme/date.xpm"
#include "default_theme/led.xpm"
#include "default_theme/beats.xpm"
#include "default_theme/hour.xpm"
#include "default_theme/weekday.xpm"
#include "default_theme/minute.xpm"
#include "default_theme/second.xpm"

char clock_xpm_fn[MAX_PATH_LEN] = "";
char month_xpm_fn[MAX_PATH_LEN] = "";
char weekday_xpm_fn[MAX_PATH_LEN] = "";
char led_xpm_fn[MAX_PATH_LEN] = "";
char beats_xpm_fn[MAX_PATH_LEN] = "";
char date_xpm_fn[MAX_PATH_LEN] = "";
char hour_xpm_fn[MAX_PATH_LEN] = "";
char min_xpm_fn[MAX_PATH_LEN] = "";
char sec_xpm_fn[MAX_PATH_LEN] = "";
char exec_str[MAX_PATH_LEN];

GdkColormap *cmap=NULL;

GdkPixmap *clock_pixmap=NULL;
GdkBitmap *clock_mask=NULL;
GdkPixmap *month_pixmap=NULL;
GdkBitmap *month_mask=NULL;
GdkPixmap *weekday_pixmap=NULL;
GdkBitmap *weekday_mask=NULL;
GdkPixmap *led_pixmap=NULL;
GdkBitmap *led_mask=NULL;
GdkPixmap *beats_pixmap=NULL;
GdkBitmap *beats_mask=NULL;
GdkPixmap *date_pixmap=NULL;
GdkBitmap *date_mask=NULL;
GdkPixmap *hour_pixmap=NULL;
GdkBitmap *hour_mask=NULL;
GdkPixmap *min_pixmap=NULL;
GdkBitmap *min_mask=NULL;
GdkPixmap *sec_pixmap=NULL;
GdkBitmap *sec_mask=NULL;

GdkImage *map=NULL;
GdkImage *clock_img=NULL;
GdkImage *hour_img=NULL;
GdkImage *min_img=NULL;
GdkImage *sec_img=NULL;

char *hour_map=NULL;
char *min_map=NULL;
char *sec_map=NULL;

gint visual_depth=0;

#ifdef ASCLOCK_GTK
/* when invoked (via signal delete_event), terminates the application.
 */
static void close_application( GtkWidget *widget, GdkEvent *event, gpointer *data ) {
    gtk_main_quit();
    return;
}
#endif

static void fail2load(char *filename)
{
  perror(filename);
  exit(-1);

}

static void fail2display(char *pointer)
{
  fprintf(stderr, "The %s pointer's origin must completely reside inside xpm\n", pointer);
  exit(-1);
}

void load_pixmaps(GtkWidget *window, GtkStyle *style)
{
  guint32 l;
    if(clock_pixmap) gdk_pixmap_unref(clock_pixmap);
    if(clock_mask) gdk_pixmap_unref(clock_mask);
    clock_pixmap=NULL;
    if(clock_xpm_fn[0])
      clock_pixmap = gdk_pixmap_create_from_xpm( window->window, &clock_mask,
                                         &style->bg[GTK_STATE_NORMAL],
                                         clock_xpm_fn );
    if( !clock_pixmap)
      clock_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &clock_mask,
                                         &style->bg[GTK_STATE_NORMAL], clock_xpm);

    if(!clock_pixmap)
      fail2load(clock_xpm_fn);

    if(analog_visible) {
      if(clock_img) gdk_image_destroy(clock_img);
      clock_img = gdk_image_get(clock_pixmap, 0, 0,
				((GdkWindowPrivate *)clock_pixmap)->width, 
				((GdkWindowPrivate *)clock_pixmap)->height);
      
    }

    if(month_visible) {
      if(month_pixmap) gdk_pixmap_unref(month_pixmap);
      if(month_mask) gdk_pixmap_unref(month_mask);
      month_pixmap = NULL;
      if(month_xpm_fn[0])
        month_pixmap = gdk_pixmap_create_from_xpm( window->window, &month_mask,
		  				   &style->bg[GTK_STATE_NORMAL],
						   month_xpm_fn );

      if ( !month_pixmap)
        month_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &month_mask,
                                         &style->bg[GTK_STATE_NORMAL], month_xpm);
      if(!month_pixmap) fail2load(month_xpm_fn);

      month_elem_width = ((GdkWindowPrivate *)month_pixmap)->width;
      month_elem_height= ((GdkWindowPrivate *)month_pixmap)->height / 12;
    }

    if(week_visible) {
      if(weekday_pixmap) gdk_pixmap_unref(weekday_pixmap);
      if(weekday_mask) gdk_pixmap_unref(weekday_mask);
      weekday_pixmap = NULL;
      if(weekday_xpm_fn[0])
        weekday_pixmap = gdk_pixmap_create_from_xpm( window->window, &weekday_mask,
  						     &style->bg[GTK_STATE_NORMAL],
						     weekday_xpm_fn );
      if (!weekday_pixmap)
        weekday_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &weekday_mask,
                                         &style->bg[GTK_STATE_NORMAL], weekday_xpm);

      if(!weekday_pixmap) fail2load(weekday_xpm_fn);

      week_elem_width = ((GdkWindowPrivate *)weekday_pixmap)->width;
      week_elem_height= ((GdkWindowPrivate *)weekday_pixmap)->height/7;

    }

    if(led_visible) {
      if(led_pixmap) gdk_pixmap_unref(led_pixmap);
      if(led_mask) gdk_pixmap_unref(led_mask);
      led_pixmap = NULL;
      if(led_xpm_fn[0])
        led_pixmap = gdk_pixmap_create_from_xpm( window->window, &led_mask,
					         &style->bg[GTK_STATE_NORMAL],
					         led_xpm_fn );
      if(!led_pixmap)
	led_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &led_mask,
					&style->bg[GTK_STATE_NORMAL], led_xpm);

      if(!led_pixmap) fail2load(led_xpm_fn);
      led_elem_width = ((GdkWindowPrivate *)led_pixmap)->width /15;
      led_elem_height= ((GdkWindowPrivate *)led_pixmap)->height;
    }

    if(beats_visible) {
      if(beats_pixmap) gdk_pixmap_unref(beats_pixmap);
      if(beats_mask) gdk_pixmap_unref(beats_mask);
      beats_pixmap = NULL;
      if(beats_xpm_fn[0])
        beats_pixmap = gdk_pixmap_create_from_xpm( window->window, &beats_mask,
                                                 &style->bg[GTK_STATE_NORMAL],
                                                 beats_xpm_fn );
      if(!beats_pixmap)
        beats_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &beats_mask,
                                        &style->bg[GTK_STATE_NORMAL], beats_xpm);

      if(!beats_pixmap) fail2load(beats_xpm_fn);
      beats_elem_width = ((GdkWindowPrivate *)beats_pixmap)->width /12;
      beats_elem_height= ((GdkWindowPrivate *)beats_pixmap)->height;
    }

    if(day_visible) {
      if(date_pixmap) gdk_pixmap_unref(date_pixmap);
      if(date_mask) gdk_pixmap_unref(date_mask);
      date_pixmap = NULL;
      if(date_xpm_fn[0])
        date_pixmap = gdk_pixmap_create_from_xpm( window->window, &date_mask,
						  &style->bg[GTK_STATE_NORMAL],
						  date_xpm_fn );
      if(!date_pixmap)
	date_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &date_mask,
						&style->bg[GTK_STATE_NORMAL], date_xpm);
      if(!date_pixmap) fail2load(date_xpm_fn);
      day_elem_width =((GdkWindowPrivate *)date_pixmap)->width/10;
      day_elem_height=((GdkWindowPrivate *)date_pixmap)->height;
    }

    if(hour_visible) {
      int x, y, w, h;
      if(hour_pixmap) gdk_pixmap_unref(hour_pixmap);
      if(hour_mask) gdk_pixmap_unref(hour_mask);
      hour_pixmap = NULL;
      if(hour_xpm_fn[0])
        hour_pixmap = gdk_pixmap_create_from_xpm( window->window, &hour_mask,
						  &style->bg[GTK_STATE_NORMAL],
						  hour_xpm_fn );
      if(!hour_pixmap)
	hour_pixmap =  gdk_pixmap_create_from_xpm_d(window->window, &hour_mask,
						&style->bg[GTK_STATE_NORMAL], hour_xpm);

      if(!hour_pixmap) fail2load(hour_xpm_fn);

      w = ((GdkWindowPrivate *)hour_pixmap)->width;
      h = ((GdkWindowPrivate *)hour_pixmap)->height;

      if(hour_img)
	      gdk_image_destroy(hour_img);
      hour_img = gdk_image_get(hour_mask, 0, 0, w, h);
      if(hour_map)
	      g_free(hour_map);
      hour_map = g_malloc(2*w*h*sizeof(char));
      for(y=0;y<h;y++) {
	for(x=0;x<w;x++) {
	  l = gdk_image_get_pixel(hour_img, x, y);
	  hour_map[y*w+x]= (l!=0);
	}
      }
      gdk_image_destroy(hour_img);
      hour_img = gdk_image_get(hour_pixmap, 0, 0, w, h);

      if((hour_rot_x<0) || (hour_rot_y < 0) || (hour_rot_x>=w) || (hour_rot_y>=h))
	 fail2display("hour");

    }

    if(min_visible) {
      int x, y, w, h;
      if(min_pixmap) gdk_pixmap_unref(min_pixmap);
      if(min_mask) gdk_pixmap_unref(min_mask);
      min_pixmap = NULL;
      if(min_xpm_fn[0])
        min_pixmap = gdk_pixmap_create_from_xpm(  window->window, &min_mask,
						  &style->bg[GTK_STATE_NORMAL],
						  min_xpm_fn );
      if(!min_pixmap)
	min_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &min_mask,
						&style->bg[GTK_STATE_NORMAL], minute_xpm);
    
      if(!min_pixmap) fail2load(min_xpm_fn);

      w = ((GdkWindowPrivate *)min_pixmap)->width;
      h = ((GdkWindowPrivate *)min_pixmap)->height;

      if(min_img)
	      gdk_image_destroy(min_img);
      min_img = gdk_image_get(min_mask, 0, 0, w, h);
      if(min_map)
	      g_free(min_map);
      min_map = g_malloc(w*h*sizeof(char));
      for(y=0;y<h;y++) {
	for(x=0;x<w;x++) {
	  l = gdk_image_get_pixel(min_img, x, y);
	  min_map[y*w+x]= (l!=0);
	}
      }
      gdk_image_destroy(min_img);

      min_img = gdk_image_get(min_pixmap, 0, 0, w, h);

      if((min_rot_x<0) || (min_rot_y < 0) || (min_rot_x>=w) || (min_rot_y>=h))
	 fail2display("minute");

    }

    if(sec_visible) {
      int x, y, w, h;
      if(sec_pixmap) gdk_pixmap_unref(sec_pixmap);
      if(sec_mask) gdk_pixmap_unref(sec_mask);
      sec_pixmap = NULL;
      if(sec_xpm_fn[0])
        sec_pixmap = gdk_pixmap_create_from_xpm( window->window, &sec_mask,
						 &style->bg[GTK_STATE_NORMAL],
						 sec_xpm_fn );
      if(!sec_pixmap)
	sec_pixmap = gdk_pixmap_create_from_xpm_d( window->window, &sec_mask,
                                                &style->bg[GTK_STATE_NORMAL],
						second_xpm);

      if(!sec_pixmap) fail2load(sec_xpm_fn);

      w = ((GdkWindowPrivate *)sec_pixmap)->width;
      h = ((GdkWindowPrivate *)sec_pixmap)->height;
     
      if(sec_img)
	      gdk_image_destroy(sec_img); 
      sec_img = gdk_image_get(sec_mask, 0, 0, w, h);
      if(sec_map)
	      g_free(sec_map);
      sec_map = g_malloc(w*h*sizeof(char));
      for(y=0;y<h;y++) {
	for(x=0;x<w;x++) {
	  l = gdk_image_get_pixel(sec_img, x, y);
	  sec_map[y*w+x]= (l!=0);
	}
      }
      gdk_image_destroy(sec_img);


      sec_img = gdk_image_get(sec_pixmap, 0, 0, w, h);

      if((sec_rot_x<0) || (sec_rot_y < 0) || (sec_rot_x>=w) || (sec_rot_y>=h))
	fail2display("second");
      
    }
}

static gint update_clock(gpointer data)
{
  time_t unix_now;
  asclock *my;
  struct tm *now;
  GdkRectangle r;

  my = (asclock *)data;

  unix_now = time(NULL);

  if(unix_now != my->actual_time)
    {
      my->actual_time = unix_now;

      now = localtime(&unix_now);

      my->beats = floor(( ( unix_now + 60*60) % (24*60*60) ) / 86.4);

      gdk_draw_pixmap(my->pixmap, my->white_gc, clock_pixmap,
                        0, 0,
                        0, 0,
                        my->width, my->height);

      if(beats_visible) swatch_beats(my, my->pixmap, my->white_gc, now, my->beats);

      if(my->showampm)
	Twelve(my, my->pixmap, my->white_gc, now);
      else
	TwentyFour(my, my->pixmap, my->white_gc, now);

      if(analog_visible) analog(my, my->pixmap, my->white_gc, now);
      
      if(itdocks==FALSE) 
	{
	  
	  r.x = 0;
	  r.y = 0;
	  r.width = my->width;
	  r.height = my->height;
	  
	  gtk_widget_draw(my->p, &r);
	}
    }

  if(itdocks==TRUE)
      XCopyArea(GDK_DISPLAY(),
		GDK_WINDOW_XWINDOW(my->pixmap), 
		my->iconwin,
		((GdkGCPrivate *)(my->white_gc))->xgc,
		0, 0, my->width, my->height, 0, 0);

  return 1;
}

static asclock *my = NULL;

void set_clock_pixmap()
{
    GdkPixmap *pixmap;
    GdkWindowPrivate *clk = (GdkWindowPrivate *) clock_pixmap;
    my->width=clk->width;
    my->height=clk->height;

    gdk_pixmap_unref(my->pixmap);

    pixmap = gdk_pixmap_new(my->window->window, my->width, my->height, -1);
    my->pixmap = pixmap;

    gdk_draw_pixmap(pixmap, my->white_gc,
                clock_pixmap, 0, 0, 0, 0, my->width, my->height);

    my->actual_time = 0;
    update_clock((gpointer)my);

    gtk_pixmap_set(GTK_PIXMAP(my->p), pixmap, clock_mask);
    gtk_widget_set_usize(my->fixed, my->width, my->height);

#ifdef ASCLOCK_GNOME
    {
      GdkWindowPrivate *sock_win = (GdkWindowPrivate *) GTK_PLUG(my->window)->socket_window;
      Window root_return;
      Window parent_return;
      Window *children_return;
      unsigned int nchildren_return;

      /* find the evil parent window */
      XQueryTree(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(sock_win),
        &root_return, &parent_return, &children_return, &nchildren_return);
      XFree(children_return);

      /* Set the mask for our parent, too */
      XShapeCombineMask(GDK_DISPLAY(), parent_return,
        ShapeBounding, 0, 0, ((GdkWindowPrivate *)clock_mask)->xwindow,
         ShapeSet);
    }
#endif
    gtk_widget_shape_combine_mask(my->window, clock_mask, 0, 0);
}

#ifdef ASCLOCK_GNOME
static gint save_session_cb(GtkWidget *widget, gchar *privcfgpath,
                            gchar *globcfgpath, gpointer data)
{
        asclock *my = (asclock *) data;
        set_gnome_config(my, privcfgpath);
	return FALSE;
}

static void
help_cb (AppletWidget *applet, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "asclock_applet", "index.html" };
	gnome_help_display (NULL, &help_entry);
}
#endif

int main( int argc, char *argv[] )
{
    /* GtkWidget is the storage type for widgets */
    GdkWindowPrivate *clk;
    GdkColor colour;

    bindtextdomain(PACKAGE, GNOMELOCALEDIR);
    textdomain(PACKAGE);

    my = g_new0 (asclock, 1);
    config();
    
    /* create the main window, and attach delete_event signal to terminating
       the application */
#ifdef ASCLOCK_GNOME

   applet_widget_init("asclock_applet", ASCLOCK_VERSION, argc, argv,
                                 NULL, 0, NULL);
   my->window = applet_widget_new("asclock_applet");

#else
#ifdef ASCLOCK_GTK
    parseArgs(my, argc, argv);
    gtk_init( &argc, &argv );
    my->window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title (GTK_WINDOW (my->window), ASCLOCK_VERSION);

    gtk_signal_connect( GTK_OBJECT (my->window), "delete_event",
                        GTK_SIGNAL_FUNC (close_application), NULL );

#else
#error use either ASCLOCK_GNOME or ASCLOCK_GTK to buid
#endif
#endif

    /* now for the pixmap from gdk */
    my->style = gtk_widget_get_style( my->window );
    gtk_widget_realize(my->window);

    my->white_gc = gdk_gc_new(my->window->window);
    /* Get the system colour map and allocate the colour red */
    cmap = gtk_widget_get_colormap (my->window);
    colour.red = 0x00;
    colour.green = 0x00;
    colour.blue = 0x00;
    if (!gdk_color_alloc(cmap, &colour)) {
      g_error("couldn't allocate colour");
    }

    my->black_gc = gdk_gc_new(my->window->window);
    gdk_gc_set_foreground(my->black_gc, &colour);

    colour.red = 0xffff;
    colour.green = 0xffff;
    colour.blue = 0xffff;
    if (!gdk_color_alloc(cmap, &colour)) {
      g_error("couldn't allocate colour");
    }

/*    my->white_gc = gdk_gc_new(my->window->window); */
    gdk_gc_set_foreground(my->white_gc, &colour);

#ifdef ASCLOCK_GNOME
   get_gnome_config(my, APPLET_WIDGET(my->window)->privcfgpath);
   /* set the timezone */
   if(strlen(my->timezone)>0)
   { 
#ifdef HAVE_SETENV
     setenv("TZ", my->timezone, TRUE);
#else
#ifdef HAVE_PUTENV
     char line[MAX_PATH_LEN];
     g_snprintf(line, MAX_PATH_LEN, "TZ=%s", my->timezone);
     putenv(line);
#else
#error neither setenv nor putenv defined
#endif
#endif
     tzset();
   }
   /* set the theme */
   if(strlen(my->theme_filename)>0)
     if(!loadTheme(my->theme_filename))
       config();
#endif

    /* get all pixmaps and store size of the clock pixmap for further usage */
    load_pixmaps(my->window, my->style);
    postconfig();

    clk = (GdkWindowPrivate *) clock_pixmap;
    my->width=clk->width;
    my->height=clk->height;

    my->pixmap = gdk_pixmap_new(my->window->window, my->width, my->height, -1);

    gdk_draw_pixmap(my->pixmap, my->white_gc,
                clock_pixmap, 0, 0, 0, 0, my->width, my->height);

    my->p = gtk_pixmap_new(my->pixmap, clock_mask);
    gtk_widget_show(my->p);

    my->fixed = gtk_fixed_new();
    gtk_fixed_put(GTK_FIXED(my->fixed), my->p, 0, 0);

    gtk_widget_set_usize(my->fixed, my->width, my->height);
#ifdef ASCLOCK_GNOME
    applet_widget_add(APPLET_WIDGET(my->window), my->fixed);
#else
#ifdef ASCLOCK_GTK
    gtk_container_add(GTK_CONTAINER(my->window), my->fixed);
#else
#error use either ASCLOCK_GNOME or ASCLOCK_GTK to build
#endif
#endif

    gtk_widget_show(my->fixed);
     

    /* callback for updating the time */
    gtk_timeout_add(100, update_clock, my);

    /* set the mask */
#ifdef ASCLOCK_GNOME
    {
      GdkWindowPrivate *sock_win = (GdkWindowPrivate *) GTK_PLUG(my->window)->socket_window;
      Window root_return;
      Window parent_return;
      Window *children_return;
      unsigned int nchildren_return;

      /* find the evil parent window */
      XQueryTree(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(sock_win),
	&root_return, &parent_return, &children_return, &nchildren_return);
      XFree(children_return);

      /* Set the mask for our parent, too */
      XShapeCombineMask(GDK_DISPLAY(), parent_return,
	ShapeBounding, 0, 0, ((GdkWindowPrivate *)clock_mask)->xwindow,
	 ShapeSet);
    }
#endif
    gtk_widget_shape_combine_mask(my->window, clock_mask, 0, 0);

    /* show the window */
    gtk_container_border_width (GTK_CONTAINER (my->window), 0);
    gtk_widget_show(my->window);

    /* fetch information about our display */
    visual = gdk_window_get_visual(my->window->window);
    visual_depth = gdk_visual_get_best_depth();

    if(analog_visible && !((visual_depth>=15) || (visual_depth==8)))
      {
	fprintf(stderr, "analog clocks are not supported yet on displays with a visual depth of %d bits\n", visual_depth);
	exit(-1);
      }

    cmap = gdk_window_get_colormap(my->window->window);
#ifdef ASCLOCK_GNOME
    applet_widget_register_stock_callback (APPLET_WIDGET (my->window),
                                           "properties",
                                           GNOME_STOCK_MENU_PROP,
                                           _("Properties..."),
                                           (AppletCallbackFunc)properties_dialog,
                                           my);
    applet_widget_register_stock_callback (APPLET_WIDGET (my->window),
					   "help",
					   GNOME_STOCK_PIXMAP_HELP,
					   _("Help"), help_cb, NULL);

    applet_widget_register_stock_callback (APPLET_WIDGET (my->window),
                                           "about",
                                           GNOME_STOCK_MENU_ABOUT,
                                           _("About..."),
                                           (AppletCallbackFunc)about_dialog,
                                           NULL);

    gtk_signal_connect(GTK_OBJECT(my->window),"save_session",
                       GTK_SIGNAL_FUNC(save_session_cb),
                       my);

    applet_widget_gtk_main ();
#else
#ifdef ASCLOCK_GTK 
  
    if(itdocks) 
      {
	Display *dpy = ((GdkWindowPrivate *)my->window->window)->xdisplay;
	Window win = ((GdkWindowPrivate *)my->window->window)->xwindow;
	XWMHints mywmhints;
	Pixel back_pix, fore_pix;
	
	back_pix = WhitePixel(dpy, DefaultScreen(dpy));
	fore_pix = BlackPixel(dpy, DefaultScreen(dpy));
	
	my->iconwin = XCreateSimpleWindow(dpy, win, 0, 0, 
					 my->width, my->height,
					 0, fore_pix, back_pix);

	XShapeCombineMask(dpy, my->iconwin, ShapeBounding, 0, 0, 
			  ((GdkWindowPrivate *)clock_mask)->xwindow, ShapeSet);

	mywmhints.icon_window = my->iconwin;
	mywmhints.flags = StateHint | IconWindowHint | WindowGroupHint;
	mywmhints.initial_state = WithdrawnState;
	mywmhints.window_group = win;
	
	XSetWMHints(dpy, win, &mywmhints);

	XSetCommand(dpy, win, argv, argc);

      }

    gtk_main ();
#else
#error use either ASCLOCK_GNOME or ASCLOCK_GTK to buid
#endif
#endif

    return 0;
}



