#ifndef __GTK_PAGER_H__
#define __GTK_PAGER_H__

#include <gtk/gtk.h>
#include <gtk/gtkdrawingarea.h>

#ifdef __cpluplus
extern "C" {
#endif

#define GTKPAGER_WINDOW_ICONIFIED  0x00000001

#define GTK_FVWMPAGER(obj)         (GTK_CHECK_CAST ((obj), gtk_fvwmpager_get_type (), GtkFvwmPager))
#define GTK_FVWMPAGER_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), gtk_fvwmpager_get_type (), GtkFvwmPagerClass))
#define GTK_IS_FVWMPAGER(obj)      (GTK_CHECK_TYPE ((obj), gtk_fvwmpager_get_type ()))

typedef struct _GtkFvwmPager       GtkFvwmPager;
typedef struct _GtkFvwmPagerClass  GtkFvwmPagerClass;


struct Desk
{
  char* title;
  gint x;
  gint y;
  gint w;
  gint h;
  GdkColor* fg;
  GdkColor* bg;
};

struct Xwin
{
  char* title;
  guint xid;
  guint flags;

  gint x;
  gint icon_x;
  
  gint y;
  gint icon_y;
  
  gint w;
  gint icon_w;
  
  gint h;
  gint icon_h;
  
  gint desktop_idx;
  GdkColor* fg;
  GdkColor* bg;
  GdkColor* focus_fg;
  GdkColor* focus_bg;
  struct Desk* desk;
};


struct _GtkFvwmPager
{
  GtkDrawingArea drawingarea;

  gint         num_of_desks;
  gint         num_of_wins;
  GdkPixmap*   pixmap;
  GList*       desktops;
  GList*       xwins;
  struct Desk* current_desktop;
  int          current_desktop_idx;
  GdkColor*    desk_fg;
  GdkColor*    desk_bg;
  GdkColor*    current_desk_fg;
  GdkColor*    current_desk_bg;
  GdkColor*    window_bg;
  GdkColor*    window_focus_bg;

  GdkGC*       current_desk_gc;
  GdkGC*       desk_gc;
  GdkGC*       window_gc;
  GdkGC*       window_focus_gc;
};


struct _GtkFvwmPagerClass
{
  GtkDrawingAreaClass parent_class;

  void (* switch_to_desk) (GtkWidget*, guint desktop_offset);
  
};


guint         gtk_fvwmpager_get_type         (void);
GtkWidget*    gtk_fvwmpager_new              (void);
void          gtk_fvwmpager_add_desk         (GtkFvwmPager*, char*);
void          gtk_fvwmpager_set_current_desk (GtkFvwmPager*, int);
void          gtk_fvwmpager_conf_window      (GtkFvwmPager*, struct Xwin*);



#endif
