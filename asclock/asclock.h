#ifndef __ASCLOCK_H
#define __ASCLOCK_H
#include <gtk/gtk.h>
#include <applet-widget.h>

#include <time.h>
#include <sys/time.h>

typedef struct _location {
  float lon;
  float lat;
  char name[64];
} location, *plocation;

struct _asclock
{
  GtkWidget *display_area;
  GdkGC *gc;
  GdkPixmap *pixmap;
};
typedef struct _asclock asclock;


extern GdkPixmap *month_pixmap;
extern GdkPixmap *led_pixmap;
extern GdkPixmap *weekday_pixmap;
extern GdkPixmap *month_pixmap;
extern GdkPixmap *date_pixmap;
extern GdkPixmap *clock_pixmap;

/* different types of drawing functions */
void Twelve(GdkPixmap *p, GdkGC *gc , struct tm *clk);
void TwentyFour(GdkPixmap *p, GdkGC *gc , struct tm *clk);

/* dialogs & properties */
void about_dialog(AppletWidget *applet, gpointer data);
void properties_dialog(AppletWidget *applet, gpointer data);

#endif
