/*
 * GNOME Desktop Pager Applet
 * (C) 1998 Red Hat Software
 *
 * Author: The Rasterman
 *
 * Conforms to GNOME WM Hints API.
 *
 */

#include <X11/Xmd.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <config.h>
/* #include <string.h> */
#include <gnome.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include "applet-widget.h"

typedef struct _task
{
  gchar              *name;
  gint                x, y, w, h;
  gint                ax, ay;
  Window              win;
  Window              frame; /* Window manager's frame window */
  Window              root;  /* Root or pseudo-root */
  gchar               iconified;
  gchar               shaded;
  gchar               focused;
  gchar               sticky;
  gint                desktop;
  GdkWindow          *gdkwin;
  GtkWidget          *dummy;
  GdkWindow          *frame_gdkwin;
}
Task;

int main(int argc, char **argv);

void cb_applet_orient_change(GtkWidget *w, PanelOrientType o, gpointer data);
void cb_applet_about(AppletWidget * widget, gpointer data);
void cb_check_show_icons(GtkWidget *widget, gpointer data);
void cb_check_fixed_tasklist(GtkWidget *widget, gpointer data);
void cb_check_show_arrow(GtkWidget *widget, gpointer data);
void cb_check_pager_size(GtkWidget *widget, gpointer data);
void cb_check_all_tasks(GtkWidget *widget, gpointer data);
void cb_check_show_tasks(GtkWidget *widget, gpointer data);
void cb_check_show_pager(GtkWidget *widget, gpointer data);
void cb_adj_max_width(GtkAdjustment *adj, GtkAdjustment *adj1);
void cb_adj_max_vwidth(GtkAdjustment *adj, GtkAdjustment *adj1);
void cb_adj_rows_h(GtkAdjustment *adj, GtkAdjustment *adj1);
void cb_adj_rows_v(GtkAdjustment *adj, GtkAdjustment *adj1);
void cb_adj_rows(GtkAdjustment *adj, GtkAdjustment *adj1);
void cb_prop_apply(GtkWidget *widget, gpointer data);
void cb_applet_properties(AppletWidget * widget, gpointer data);

void *util_get_atom(Window win, gchar *atom, Atom type, gint *size);
gchar *util_reduce_chars(gchar * s, int num);

void client_win_kill(Task *t);
void client_win_close(Task *t);
void client_win_iconify(Task *t);
void client_win_show(Task *t);
void client_win_stick(Task *t);
void client_win_unstick(Task *t);
void client_win_shade(Task *t);
void client_win_unshade(Task *t);

void custom_popbox_show(GtkWidget * widget);

void            cb_task_change(GtkWidget *widget, GdkEventProperty * ev, Task *t);
void            cb_root_prop_change(GtkWidget * widget, GdkEventProperty * ev);
GdkFilterReturn cb_filter_intercept(GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data);

void task_get_info(Task *t);
gint task_add(Window win);
void task_delete(Window win);
Task *task_find(Window win);
void tasks_match(Window * win, guint num);
void tasks_update(void);
void get_desktop_names(void);

void select_root_properties(void);

void init_applet_gui(void);
void init_applet_gui_horiz(void);
void init_applet_gui_vert(void);

void desktop_draw(gint i);
int actual_redraw(gpointer data);
gboolean desktop_cb_button_down(GtkWidget * widget, GdkEventButton *event);
gboolean desktop_cb_button_up(GtkWidget * widget, GdkEventButton *event);
int desktop_cb_redraw(GtkWidget *widget, gpointer data);
void cb_desk_destroy(GtkWidget *widget, gpointer data);
GtkWidget *make_desktop_pane(gint desktop, gint width, gint height);

gboolean task_cb_button_enter(GtkWidget * widget, GdkEventCrossing *event);
gboolean task_cb_button_leave(GtkWidget * widget, GdkEventCrossing *event);
gboolean task_cb_button_down(GtkWidget * widget, GdkEventButton *event);
gboolean task_cb_button_up(GtkWidget * widget, GdkEventButton *event);

void cb_showhide(GtkWidget * widget, Task *t);
void cb_shade(GtkWidget * widget, Task *t);
void cb_kill(GtkWidget * widget, Task *t);
void cb_nuke(GtkWidget * widget, Task *t);
void emtpy_task_widgets(void);
void desktroy_task_widgets(void);
GtkWidget *find_task_widget(Task *t);
void set_task_info_to_button(Task *t);
void populate_tasks(void);

void desktop_set_area(int ax, int ay);
void create_popbox(void);
int showpop_cb(GtkWidget *widget, gpointer data);

void get_window_root_and_frame_id(Window w, Window *ret_frame, 
				      Window *ret_root);
GList *get_task_stacking(GList *p, gint desk);



#include "icon1.xpm"
#include "icon2.xpm"
#include "icon3.xpm"
#ifdef ANIMATION      
#include "f1.xpm"
#include "f2.xpm"
#include "f3.xpm"
#include "f4.xpm"
#endif
