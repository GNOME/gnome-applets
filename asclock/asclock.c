#include "asclock.h"
#include "clk.xpm"
gchar clock_xpm[]  = "asclock/clk.xpm";
#include "month.xpm"
gchar month_xpm[]  = "asclock/month.xpm";
#include "weekday.xpm"
gchar weekday_xpm[]= "asclock/weekday.xpm";
#include "led.xpm"
gchar led_xpm[]    = "asclock/led.xpm";
#include "date.xpm"
gchar date_xpm[]   = "asclock/date.xpm";

GdkPixmap *clock_pixmap;
GdkBitmap *clock_mask;
GdkPixmap *month_pixmap;
GdkBitmap *month_mask;
GdkPixmap *weekday_pixmap;
GdkBitmap *weekday_mask;
GdkPixmap *led_pixmap;
GdkBitmap *led_mask;
GdkPixmap *date_pixmap;
GdkBitmap *date_mask;

/* prototypes */
static gint update_clock(gpointer data);
static int mytime(void);



static void
load_pixmaps(GtkWidget *window, GtkStyle *style)
{
    char *tmp, *fname;;

    fname = gnome_unconditional_pixmap_file(clock_xpm);
    clock_pixmap = gdk_pixmap_create_from_xpm( window->window, &clock_mask,
                                         &style->bg[GTK_STATE_NORMAL],
                                         fname );
    if ( !clock_pixmap)
      clock_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &clock_mask,
                                         &style->bg[GTK_STATE_NORMAL], clock_xpm_data);

    month_pixmap = gdk_pixmap_create_from_xpm( window->window, &month_mask,
                                         &style->bg[GTK_STATE_NORMAL],
                                         month_xpm );
    if ( !month_pixmap)
      month_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &month_mask,
                                         &style->bg[GTK_STATE_NORMAL], month_xpm_data);

    weekday_pixmap = gdk_pixmap_create_from_xpm( window->window, &weekday_mask,
                                         &style->bg[GTK_STATE_NORMAL],
                                         weekday_xpm );
    if ( !weekday_pixmap)
      weekday_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &weekday_mask,
                                         &style->bg[GTK_STATE_NORMAL], weekday_xpm_data);

    led_pixmap = gdk_pixmap_create_from_xpm( window->window, &led_mask,
                                         &style->bg[GTK_STATE_NORMAL],
                                         led_xpm );
    if ( !led_pixmap)
      led_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &led_mask,
                                         &style->bg[GTK_STATE_NORMAL], led_xpm_data);

    date_pixmap = gdk_pixmap_create_from_xpm( window->window, &date_mask,
                                         &style->bg[GTK_STATE_NORMAL],
                                         date_xpm );
    if ( !date_pixmap)
      date_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &date_mask,
                                         &style->bg[GTK_STATE_NORMAL], date_xpm_data);

}

static gint update_clock(gpointer data)
{
  time_t unix_now;
  asclock *my;
  struct tm *now;
  GdkRectangle r;
  
  my = (asclock *)data;
  unix_now = mytime();
  now = localtime(&unix_now);

  TwentyFour(my->pixmap, 
             my->display_area->style->fg_gc[GTK_WIDGET_STATE(my->display_area)],
             now);

  r.x = 0;
  r.y = 0;
  r.width = 64;
  r.height = 64;

  gdk_window_set_back_pixmap(my->display_area->window,my->pixmap,FALSE);
  gdk_window_clear(my->display_area->window);
  gtk_widget_draw(my->display_area, &r);

  return TRUE;
}

static int mytime(void)
{
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  return tv.tv_sec;
}

int main( int argc, char *argv[] )
{
    /* GtkWidget is the storage type for widgets */
    GtkWidget *box;
    GtkWidget *applet;
    struct tm *now;
    asclock my;
    time_t unix_now;

    /* create the main window, and attach delete_event signal to terminating
       the application */
    applet_widget_init("asclock_applet", VERSION, argc, argv,
				NULL, 0, NULL);

    applet = applet_widget_new("asclock_applet");

    if (!applet)
      g_error("Can't create asclock applet");

    my.display_area = gtk_drawing_area_new();
    gtk_drawing_area_size(GTK_DRAWING_AREA(my.display_area),64,64);
    gtk_widget_show(my.display_area);

    gtk_widget_set_usize(my.display_area, 64, 64); 

    applet_widget_add(APPLET_WIDGET(applet), my.display_area);

    gtk_widget_realize(my.display_area);

    /* now for the pixmap from gdk */
    load_pixmaps(my.display_area,
		 my.display_area->style);

    my.pixmap = gdk_pixmap_new(my.display_area->window, 64, 64, -1);

    gtk_widget_show(applet);

    /* callback for updating the time */
    gtk_timeout_add(100, update_clock, &my);

    update_clock(&my);

    applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					  "hello",
					  GNOME_STOCK_MENU_ABOUT,
					  _("About"),
					  about_dialog,
					  NULL);

    applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					  "properties",
					  GNOME_STOCK_MENU_PROP,
					  _("Properties..."),
					  properties_dialog,
					  NULL);


    applet_widget_gtk_main ();
          
    return 0;
}

