#ifndef ASCLOCK_H
#define ASCLOCK_H
#include <config.h>
#include <sys/time.h>
#include <ctype.h>
#include <gtk/gtk.h>
#include <gdk/gdkprivate.h>
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef ASCLOCK_GNOME
#include <applet-widget.h>
#endif

#include "version.h"

#define VISIBLE 1
#define UNDEFINED 99999

#define MAX_PATH_LEN 512
#define INT_TYPE 0

typedef struct _asclock asclock;
typedef struct _symbol symbol;
#ifdef ASCLOCK_GNOME
typedef struct _location {
   float lon;
   float lat;
   char name[64];
 } location, *plocation;
#endif
struct _asclock
{
  int width;
  int height;
  int beats;
  time_t actual_time;
  GtkWidget *p;
  GtkWidget *fixed;
  GdkPixmap *pixmap;
  GtkStyle *style;
  GdkGC *white_gc;
  GdkGC *black_gc;
  GtkWidget *window;
  Window iconwin;
  int showampm;
  int selected_showampm;
  int itblinks;
  int selected_itblinks;
#ifdef ASCLOCK_GNOME
  char theme_filename[MAX_PATH_LEN];
  char selected_theme_filename[MAX_PATH_LEN];
  char timezone[MAX_PATH_LEN];
  char selected_timezone[MAX_PATH_LEN];
  GtkWidget *pwin;
  GtkWidget *pic;
#endif
};

struct _symbol
{
  char *name;
  int *addr;
};
extern symbol config_symbols[];
extern char *themes_directories[];

extern gint visual_depth;
GdkVisual *visual;
GdkColormap *cmap;

/* the filenames */
extern char clock_xpm_fn[MAX_PATH_LEN];
extern char month_xpm_fn[MAX_PATH_LEN];
extern char weekday_xpm_fn[MAX_PATH_LEN];
extern char led_xpm_fn[MAX_PATH_LEN];
extern char beats_xpm_fn[MAX_PATH_LEN];
extern char date_xpm_fn[MAX_PATH_LEN];
extern char hour_xpm_fn[MAX_PATH_LEN];
extern char min_xpm_fn[MAX_PATH_LEN];
extern char sec_xpm_fn[MAX_PATH_LEN];

extern char exec_str[MAX_PATH_LEN];

extern GdkPixmap *month_pixmap;
extern GdkPixmap *led_pixmap;
extern GdkPixmap *beats_pixmap;
extern GdkPixmap *weekday_pixmap;
extern GdkPixmap *month_pixmap;
extern GdkPixmap *date_pixmap;
extern GdkPixmap *clock_pixmap;
extern GdkPixmap *hour_pixmap;
extern GdkPixmap *min_pixmap;
extern GdkPixmap *sec_pixmap;

extern GdkImage *img;
extern GdkImage *clock_img;
extern GdkImage *hour_img;
extern GdkImage *min_img;
extern GdkImage *sec_img;

extern int itdocks;

extern int led_visible;
extern int led_elem_width;
extern int led_elem_height;
extern int led_12h_hour1_x;
extern int led_12h_hour2_x;
extern int led_12h_colon_x;
extern int led_12h_min1_x;
extern int led_12h_min2_x;
extern int led_ampm_x;
extern int led_ampm_y;
extern int led_ampm_width;
extern int led_12h_y;
extern int led_24h_hour1_x;
extern int led_24h_hour2_x;
extern int led_24h_colon_x;
extern int led_24h_min1_x;
extern int led_24h_min2_x;
extern int led_24h_y;

extern int week_visible;
extern int week_elem_width;
extern int week_elem_height;
extern int week_x;
extern int week_y;

extern int day_visible;
extern int day_elem_width;
extern int day_elem_height;
extern int day_x;
extern int day1_x;
extern int day2_x;
extern int day_y;

extern int month_visible;
extern int month_elem_width;
extern int month_elem_height;
extern int month_x;
extern int month_y;

extern int analog_visible;
extern int hour_visible ;
extern int hour_center_x ;
extern int hour_center_y ;
extern int hour_rot_x ;
extern int hour_rot_y ;

extern int min_visible ;
extern int min_center_x ;
extern int min_center_y ;
extern int min_rot_x ;
extern int min_rot_y ;

extern int sec_visible ;
extern int sec_center_x ;
extern int sec_center_y ;
extern int sec_rot_x ;
extern int sec_rot_y ;

extern int beats_visible ;
extern int beats1_x;
extern int beats2_x;
extern int beats3_x;
extern int beats_y;
extern int beats_elem_width;
extern int beats_elem_height;
extern int beats_at_x;
extern int beats_at_width;

extern char *hour_map;
extern char *min_map;
extern char *sec_map;

/********** asclock.c ****************/
void load_pixmaps(GtkWidget *window, GtkStyle *style);
void set_clock_pixmap(void);

/************ draw.c ******************/
void analog(asclock *my, GdkPixmap *p, GdkGC *gc, struct tm *clk);
void swatch_beats(asclock *my, GdkPixmap *p, GdkGC *gc, struct tm *clk, int beats);
void Twelve(asclock *my, GdkPixmap *p, GdkGC *gc , struct tm *clk);
void TwentyFour(asclock *my, GdkPixmap *p, GdkGC *gc , struct tm *clk);

/********** config.c ****************/
void init_symbols(void);
void config(void);
void parseArgs(asclock *my, int argc, char **argv);
int loadTheme(char *themesdir);
void postconfig(void);

/********** rot.c *******************/
void rotate(GdkImage *img, char *map, int center_x, int center_y, int rot_x, int rot_y, double alpha);

/********** dialogs.c ***************/
#ifdef ASCLOCK_GNOME
void about_dialog(AppletWidget *applet, gpointer data);
void properties_dialog(AppletWidget *applet, gpointer data);

/********** timezone.c **************/
void enum_timezones(asclock *my_asclock, GtkWidget *clist );

/********** gnome_config.c **********/
void set_gnome_config(asclock *my, char *configpath);
void get_gnome_config(asclock *my, char *configpath);
#endif

/********* parser.c ****************/

int read_init(FILE *f);
int read_type(int *type);
int read_token(char *str, int max);
int read_assign(void);
int read_int(int *ret);
int read_semicolon(void);

#endif /* ASCLOCK_H */

