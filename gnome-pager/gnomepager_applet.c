/*
 * GNOME Desktop Pager Applet
 * (C) 1998 The Free Software Foundation
 *
 * Author: M.Watson
 *         The Rasterman
 *
 * A panel applet that switches between multiple desktops using
 * Marko Macek's XA_WIN_* hints proposal. Compatible with icewm => 0.91
 *
 */

#define GWIN_STATE_STICKY          (1<<0)
#define GWIN_STATE_RESERVED_BIT1   (1<<1)
#define GWIN_STATE_MAXIMIZED_VERT  (1<<2)
#define GWIN_STATE_MAXIMIZED_HORIZ (1<<3)
#define GWIN_STATE_HIDDEN          (1<<4)
#define GWIN_STATE_SHADED          (1<<5)
#define GWIN_STATE_HID_WORKSPACE   (1<<6)
#define GWIN_STATE_HID_TRANSIENT   (1<<7)
#define GWIN_STATE_FIXED_POSITION  (1<<8)
#define GWIN_STATE_ARRANGE_IGNORE  (1<<9)

#define GWIN_HINTS_SKIP_FOCUS             (1<<0)
#define GWIN_HINTS_SKIP_WINLIST           (1<<1)
#define GWIN_HINTS_SKIP_TASKBAR           (1<<2)
#define GWIN_HINTS_GROUP_TRANSIENT        (1<<3)

#include <X11/Xlib.h>
#include <X11/Xmd.h>
#include <X11/Xatom.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include "WinMgr.h"
#include "applet-lib.h"
#include "applet-widget.h"

typedef struct _task
  {
    GtkWidget          *widget;
    GtkWidget          *label;
    GtkWidget          *icon;
    GtkWidget          *state[2];
    GtkWidget          *menu;
    GtkWidget          *tooltip;
    GtkWidget          *_widget;
    GtkWidget          *_label;
    GtkWidget          *_icon;
    GtkWidget          *_state[2];
    GtkWidget          *_menu;
    GtkWidget          *_tooltip;
    
    GtkWidget          *dummy;
    gchar              *name;
    gint                x, y, w, h;
    GdkWindow          *gdkwin;
    Window              win;
    gchar               iconified;
    gchar               shaded;
    gchar               focused;
    gchar               sticky;
    gint                desktop;
  }
Task;

void                setup(void);
void                prop_change_cb(GtkWidget * widget, GdkEventProperty * ev);
void                switch_cb(GtkWidget * widget, gpointer data);
void                about_cb(AppletWidget * widget, gpointer data);
void                properties_dialog(AppletWidget * widget, gpointer data);
void                change_workspace(gint ws);
void                make_sticky(Window win);
void                make_shaded(Window win);
void                make_unshaded(Window win);
gint                check_workspace(gpointer data);
GList              *get_workspaces(void);
gint                get_current_workspace(void);
gint                get_current_workspace(void);
void                kill_cb(GtkWidget * widget, Task * t);
void                iconify_cb(GtkWidget * widget, Task * t);
void                show_cb(GtkWidget * widget, Task * t);
void               *AtomGet(Window win, char *atom, Atom type, int *size);
void                task_cb(GtkWidget * widget, Task * t);
gchar              *munge(gchar * s, int num);
void                client_prop_change_cb(GtkWidget * widget, GdkEvent * ev, Task * t);
void                enter_cb(GtkWidget * widget, Task * t);
void                leave_cb(GtkWidget * widget, Task * t);
void                add_task(Window win);
void                del_task(Window win);
Task               *find_task(Window win);
void                match_tasks(Window * win, guint num);
void                update_tasks(void);
GList              *get_workspaces(void);
gint                get_current_workspace(void);
void shade_cb(GtkWidget * widget, Task * t);
void showhide_cb(GtkWidget * widget, Task * t);
void nuke_cb(GtkWidget * widget, Task * t);
void showpop_cb(GtkWidget * widget, gpointer data);
void applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data);

GtkWidget          *dummy_win;
GtkWidget          *dummy_box;
GtkWidget          *applet;
GtkWidget          *tasks1;
GtkWidget          *popbox;
GtkWidget          *qme, *qme2;
GtkWidget         **boxlists;
GtkWidget         **flists;
GList              *workspace_list, *button_list, *tasks = NULL;
gint                workspace_count, current_workspace = 1;
gint                i_changed_it = 0;
gint                tasks_in_row = 16;
Atom                _XA_GWIN_WORKSPACE;
Atom                _XA_GWIN_WORKSPACE_NAMES;
Atom                _XA_GWIN_STATE;
PanelOrientType     orient;
extern gint         gdk_error_warnings;

void 
applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
  orient = o;
  switch (o) {
   case ORIENT_UP:
   case ORIENT_DOWN:
   case ORIENT_LEFT:
   case ORIENT_RIGHT:
  }
  w = NULL;
  data = NULL;
}

void 
nuke_cb(GtkWidget * widget, Task * t)
{
  XKillClient(GDK_DISPLAY(), (XID) t->win);
  widget = NULL;
}

void 
kill_cb(GtkWidget * widget, Task * t)
{
  XClientMessageEvent ev;
  Atom                a1, a2, a3, *prop;
  unsigned long       lnum, ldummy;
  int                 num, i, del, dummy;

  a1 = XInternAtom(GDK_DISPLAY(), "WM_DELETE_WINDOW", False);
  a2 = XInternAtom(GDK_DISPLAY(), "WM_PROTOCOLS", False);
  num = 0;
  prop = NULL;
  del = 0;
  if (!XGetWMProtocols(GDK_DISPLAY(), t->win, &prop, &num))
    {
      XGetWindowProperty(GDK_DISPLAY(), t->win, a2, 0, 10, False, a2, &a3, &dummy,
			 &lnum, &ldummy, (unsigned char **)&prop);
      num = (int)lnum;
    }
  if (prop)
    {
      for (i = 0; i < num; i++)
	if (prop[i] == a1)
	  del = 1;
      XFree(prop);
    }
  if (del)
    {
      ev.type = ClientMessage;
      ev.window = t->win;
      ev.message_type = a2;
      ev.format = 32;
      ev.data.l[0] = a1;
      ev.data.l[1] = CurrentTime;
      XSendEvent(GDK_DISPLAY(), t->win, False, 0, (XEvent *) & ev);
    }
  else
    XKillClient(GDK_DISPLAY(), (XID) t->win);
  XSync(GDK_DISPLAY(), False);
  widget = NULL;
}

void 
showpop_cb(GtkWidget * widget, gpointer data)
{
  gint x, y;
  gint screen_width;
  gint screen_height;
  GdkCursor *cursor;
  
  if (GTK_WIDGET_VISIBLE(popbox))
    {
      gdk_pointer_ungrab (GDK_CURRENT_TIME);
      gtk_widget_hide(popbox);
      gtk_grab_remove (popbox);
    }
  else
    {
      gdk_window_get_pointer (NULL, &x, &y, NULL);
      gtk_widget_size_request (qme, &qme->requisition);
      screen_width = gdk_screen_width ();
      screen_height = gdk_screen_height ();
      
      if ((orient == ORIENT_UP) || (orient == ORIENT_DOWN))
	{
	  x -= (qme->requisition.width / 2);
	  if (y > screen_height / 2)
	    y -= (qme->requisition.height + 2);
	  else
	    y += 2;
	}
      else
	{
	  y -= (qme->requisition.height / 2);
	  if (x > screen_width / 2)
	    x -= (qme->requisition.width + 2);
	  else
	    x += 2;
	}
      
      if ((x + qme->requisition.width) > screen_width)
	x -= ((x + qme->requisition.width) - screen_width);
      if (x < 0)
	x = 0;
      if ((y + qme->requisition.height) > screen_height)
	y -= ((y + qme->requisition.height) - screen_height);
      if (y < 0)
	y = 0;
      
      gtk_widget_set_uposition(popbox, x, y);
      gtk_widget_show(popbox);
      while(gtk_events_pending())
	gtk_main_iteration();
      cursor = gdk_cursor_new (GDK_ARROW);
      gdk_pointer_grab (popbox->window, TRUE,
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
			GDK_POINTER_MOTION_MASK,
			NULL, cursor, GDK_CURRENT_TIME);
      gtk_grab_add (popbox);
      gdk_cursor_destroy (cursor);
    }
  widget = NULL;
  data = NULL;
}

void 
iconify_cb(GtkWidget * widget, Task * t)
{
  XIconifyWindow(GDK_DISPLAY(), t->win, DefaultScreen(GDK_DISPLAY()));
  XSync(GDK_DISPLAY(), False);
  widget = NULL;
}

void 
show_cb(GtkWidget * widget, Task * t)
{
  Atom                a1, a2, a3, *prop;
  XClientMessageEvent ev;
  unsigned long       lnum, ldummy;
  int                 num, i, foc, dummy;

  a1 = XInternAtom(GDK_DISPLAY(), "WM_TAKE_FOCUS", False);
  a2 = XInternAtom(GDK_DISPLAY(), "WM_PROTOCOLS", False);
  num = 0;
  prop = NULL;
  foc = 0;
  if (!XGetWMProtocols(GDK_DISPLAY(), t->win, &prop, &num))
    {
      XGetWindowProperty(GDK_DISPLAY(), t->win, a2, 0, 10, False, a2, &a3, &dummy,
			 &lnum, &ldummy, (unsigned char **)&prop);
      num = (int)lnum;
    }
  if (prop)
    {
      for (i = 0; i < num; i++)
	if (prop[i] == a1)
	  foc = 1;
      XFree(prop);
    }
  if (foc)
    {
      ev.type = ClientMessage;
      ev.window = t->win;
      ev.message_type = a2;
      ev.format = 32;
      ev.data.l[0] = a1;
      ev.data.l[1] = CurrentTime;
      XSendEvent(GDK_DISPLAY(), t->win, False, 0, (XEvent *) & ev);
    }

  XSetInputFocus(GDK_DISPLAY(), t->win, RevertToPointerRoot, CurrentTime);

  XRaiseWindow(GDK_DISPLAY(), t->win);
  XMapWindow(GDK_DISPLAY(), t->win);
  if ((!t->sticky) && (t->desktop != current_workspace - 1))
    {
      ev.type = ClientMessage;
      ev.window = GDK_ROOT_WINDOW();
      ev.message_type = _XA_GWIN_WORKSPACE;
      ev.format = 32;
      ev.data.l[0] = t->desktop;
      ev.data.l[1] = CurrentTime;
      
      XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
		 SubstructureNotifyMask, (XEvent *) &ev);
    }
  XSync(GDK_DISPLAY(), False);
  widget = NULL;
}

void               *
AtomGet(Window win, char *atom, Atom type, int *size)
{
  unsigned char      *retval;
  Atom                to_get, type_ret;
  unsigned long       bytes_after, num_ret;
  int                 format_ret;
  long                length;
  void               *data;

  to_get = XInternAtom(GDK_DISPLAY(), atom, False);
  retval = NULL;
  length = 0x7fffffff;
  XGetWindowProperty(GDK_DISPLAY(), win, to_get, 0,
		     length,
		     False, type,
		     &type_ret,
		     &format_ret,
		     &num_ret,
		     &bytes_after,
		     &retval);
  if ((retval) && (num_ret > 0) && (format_ret > 0))
    {
      data = g_malloc(num_ret * (format_ret >> 3));
      if (data)
	memcpy(data, retval, num_ret * (format_ret >> 3));
      XFree(retval);
      *size = num_ret * (format_ret >> 3);
      return data;
    }
  return NULL;
}

void 
task_cb(GtkWidget * widget, Task * t)
{
  static guint32      last = 0;
  guint32             current;
  static gchar        was_double = 0;
  guint32             dub_click_time = 300;

  if (GTK_WIDGET_VISIBLE(popbox))
    showpop_cb(NULL, NULL);    
  
  current = gdk_time_get();
  if ((current - last > 0) && (current - last < dub_click_time) &&
      (!was_double))
    {
      was_double = 1;
      iconify_cb(widget, t);
    }
  else
    {
      show_cb(widget, t);
      was_double = 0;
    }
  last = current;
  widget = NULL;
}

gchar              *
munge(gchar * s, int num)
{
  int                 l;
  gchar              *ss;

  l = strlen(s);
  if (l > num)
    {
      ss = g_malloc(num + 4);
      ss[0] = 0;
      strncat(ss, s, num);
      strcat(ss, "...");
      return ss;
    }
  else
    return strdup(s);
}

void 
client_prop_change_cb(GtkWidget * widget, GdkEvent * ev, Task * t)
{
  gchar              *str;
  int                 size, rev, pstick, pdesk;
  Window              win, ret;
  CARD32             *val;
  Atom                a;

  win = t->win;
  XGetInputFocus(GDK_DISPLAY(), &ret, &rev);
  t->focused = 0;
  if (win == ret)
    t->focused = 1;
  if (t->name)
    g_free(t->name);
  t->name = NULL;

  str = NULL;
  str = AtomGet(win, "WM_NAME", XA_STRING, &size);
  if (str)
    {
      t->name = g_malloc(size + 1);
      memcpy(t->name, str, size);
      t->name[size] = 0;
      g_free(str);
    }
  str = NULL;
  if (t->name)
    {
      str = munge(t->name, tasks_in_row);
      gtk_label_set(GTK_LABEL(t->label), str);
      g_free(str);
      str = munge(t->name, 16);
      gtk_label_set(GTK_LABEL(t->_label), str);
      g_free(str);
    }
  else
    {
      gtk_label_set(GTK_LABEL(t->label), _("??"));
      gtk_label_set(GTK_LABEL(t->_label), _("??"));
    }
  if (t->name)
    {
      gtk_tooltips_set_tip(GTK_TOOLTIPS(t->tooltip),
			   t->widget, t->name, t->name);
      gtk_tooltips_set_tip(GTK_TOOLTIPS(t->_tooltip),
			   t->_widget, t->name, t->name);
    }
  else
    {
      gtk_tooltips_set_tip(GTK_TOOLTIPS(t->tooltip),
			   t->widget, "??", "??");
      gtk_tooltips_set_tip(GTK_TOOLTIPS(t->_tooltip),
			   t->_widget, "??", "??");
    }

  
  pstick = t->sticky;
  pdesk = t->desktop;
  t->iconified = 0;
  t->shaded = 0;
  t->sticky = 0;
  t->desktop = 0;

  val = AtomGet(win, "WIN_STATE", XA_CARDINAL, &size);
  if (val)
    {
      if (*val & GWIN_STATE_STICKY)
	t->sticky = 1;
      if (*val & GWIN_STATE_SHADED)
	t->shaded = 1;
      g_free(val);
    }
  a = XInternAtom(GDK_DISPLAY(), "WM_STATE", False);
  val = AtomGet(win, "WM_STATE", a, &size);
  if (val)
    {
      if (val[0] == IconicState)
	t->iconified = 1;
      g_free(val);
    }
  val = AtomGet(win, "WIN_WORKSPACE", XA_CARDINAL, &size);
  if (val)
    {
      t->desktop = *val;
      g_free(val);
    }
  gtk_widget_hide(t->state[0]);
  gtk_widget_hide(t->_state[0]);
  gtk_widget_hide(t->state[1]);
  gtk_widget_hide(t->_state[1]);
  if (t->iconified)
    {
      gtk_widget_show(t->state[0]);
      gtk_widget_show(t->_state[0]);
    }
  else
    {
      gtk_widget_show(t->state[1]);
      gtk_widget_show(t->_state[1]);
    }
  if (t->focused)
    {
      gtk_widget_set_state(t->widget, GTK_STATE_ACTIVE);
    }
  else
    {
      if (ev == (void *)1)
	{
	  gtk_widget_set_state(t->widget, GTK_STATE_PRELIGHT);
	  gtk_widget_set_state(t->_widget, GTK_STATE_PRELIGHT);
	}
      else
	{
	  gtk_widget_set_state(t->widget, GTK_STATE_NORMAL);
	  gtk_widget_set_state(t->_widget, GTK_STATE_NORMAL);
	}
    }
  if ((t->sticky != pstick) || (t->desktop != pdesk))
    update_tasks();
  ev = NULL;
  widget = NULL;
}

void 
enter_cb(GtkWidget * widget, Task * t)
{
  client_prop_change_cb(widget, (void *)1, t);
}

void 
leave_cb(GtkWidget * widget, Task * t)
{
  client_prop_change_cb(widget, NULL, t);
}

void
shade_cb(GtkWidget * widget, Task * t)
{
  if (t->shaded)
    make_unshaded(t->win);
  else
    make_shaded(t->win);
  widget = NULL;
}

void
showhide_cb(GtkWidget * widget, Task * t)
{
  if (t->iconified)
    show_cb(NULL, t);
  else
    iconify_cb(NULL, t);
  widget = NULL;
}

void 
add_task(Window win)
{
  Task               *t;
  int                 size;
  gchar              *str;
  GtkWidget          *hb;
  CARD32             *val;
  GnomeUIInfo         uinfo[5] = 
    {
      GNOMEUIINFO_END,
      GNOMEUIINFO_END,
      GNOMEUIINFO_END,
      GNOMEUIINFO_END,
      GNOMEUIINFO_END
    };

  val = AtomGet(win, "WIN_HINTS", XA_CARDINAL, &size);
  if (val)
    {
      if ((*val) & GWIN_HINTS_SKIP_TASKBAR)
	{
	  g_free(val);
	  return;
	}
    }

  t = g_malloc(sizeof(Task));

  t->win = win;
  t->x = 0;
  t->y = 0;
  t->w = -1;
  t->h = -1;
  t->name = NULL;
  t->iconified = 0;
  t->shaded = 0;
  t->focused = 0;
  t->sticky = 0;
  t->desktop = 0;

  uinfo[0].type            = GNOME_APP_UI_ITEM;
  uinfo[0].label           = N_("Show / Hide");
  uinfo[0].hint            = NULL;
  uinfo[0].moreinfo        = showhide_cb;
  uinfo[0].user_data       = t;
  uinfo[0].unused_data     = NULL;
  uinfo[0].pixmap_type     = GNOME_APP_PIXMAP_STOCK;
  uinfo[0].pixmap_info     = GNOME_STOCK_MENU_OPEN;
  uinfo[0].accelerator_key = 0;
  uinfo[0].ac_mods         = (GdkModifierType) 0;
  uinfo[0].widget          = NULL;

  uinfo[1].type            = GNOME_APP_UI_ITEM;
  uinfo[1].label           = N_("Shade / Unshade");
  uinfo[1].hint            = NULL;
  uinfo[1].moreinfo        = shade_cb;
  uinfo[1].user_data       = t;
  uinfo[1].unused_data     = NULL;
  uinfo[1].pixmap_type     = GNOME_APP_PIXMAP_STOCK;
  uinfo[1].pixmap_info     = GNOME_STOCK_MENU_OPEN;
  uinfo[1].accelerator_key = 0;
  uinfo[1].ac_mods         = (GdkModifierType) 0;
  uinfo[1].widget          = NULL;

  uinfo[2].type            = GNOME_APP_UI_ITEM;
  uinfo[2].label           = N_("Close");
  uinfo[2].hint            = NULL;
  uinfo[2].moreinfo        = kill_cb;
  uinfo[2].user_data       = t;
  uinfo[2].unused_data     = NULL;
  uinfo[2].pixmap_type     = GNOME_APP_PIXMAP_STOCK;
  uinfo[2].pixmap_info     = GNOME_STOCK_MENU_OPEN;
  uinfo[2].accelerator_key = 0;
  uinfo[2].ac_mods         = (GdkModifierType) 0;
  uinfo[2].widget          = NULL;

  uinfo[3].type            = GNOME_APP_UI_ITEM;
  uinfo[3].label           = N_("Nuke");
  uinfo[3].hint            = NULL;
  uinfo[3].moreinfo        = nuke_cb;
  uinfo[3].user_data       = t;
  uinfo[3].unused_data     = NULL;
  uinfo[3].pixmap_type     = GNOME_APP_PIXMAP_STOCK;
  uinfo[3].pixmap_info     = GNOME_STOCK_MENU_OPEN;
  uinfo[3].accelerator_key = 0;
  uinfo[3].ac_mods         = (GdkModifierType) 0;
  uinfo[3].widget          = NULL;

  val = AtomGet(win, "WIN_STATE", XA_CARDINAL, &size);
  if (val)
    {
      if ((*val) & GWIN_STATE_STICKY)
	t->sticky = 1;
      if ((*val) & GWIN_STATE_SHADED)
	t->shaded = 1;
      g_free(val);
    }
  val = AtomGet(win, "WM_STATE", XA_CARDINAL, &size);
  if (val)
    {
      if (val[0] == IconicState)
	t->iconified = 1;
      g_free(val);
    }
  val = AtomGet(win, "WIN_WORKSPACE", XA_CARDINAL, &size);
  if (val)
    {
      t->desktop = *val;
      g_free(val);
    }

  str = NULL;
  str = AtomGet(win, "WM_NAME", XA_STRING, &size);
  if (str)
    {
      t->name = g_malloc(size + 1);
      memcpy(t->name, str, size);
      t->name[size] = 0;
      g_free(str);
    }
  str = NULL;
  t->widget = gtk_button_new();
  t->_widget = gtk_button_new();
  if (t->name)
    {
      str = munge(t->name, tasks_in_row);
      t->label = gtk_label_new(str);
      g_free(str);
      str = munge(t->name, 16);
      t->_label = gtk_label_new(str);
      g_free(str);
    }
  else
    {
      t->label = gtk_label_new(_("??"));
      t->_label = gtk_label_new(_("??"));
    }
  hb = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hb);
  gtk_container_add(GTK_CONTAINER(t->widget), hb);

  t->icon = gnome_stock_pixmap_widget_new(applet,
					  GNOME_STOCK_MENU_TRASH);
  t->state[1] = gnome_stock_pixmap_widget_new(applet,
					      GNOME_STOCK_MENU_BOOK_OPEN);
  t->state[0] = gnome_stock_pixmap_widget_new(applet,
					      GNOME_STOCK_MENU_BOOK_RED);
  t->tooltip = (GtkWidget *) gtk_tooltips_new();
  gtk_tooltips_set_tip(GTK_TOOLTIPS(t->tooltip),
		       t->widget, t->name, t->name);
  t->menu = gnome_popup_menu_new(uinfo);
  gnome_popup_menu_attach(t->menu, t->widget, t);

  gtk_box_pack_start(GTK_BOX(hb), t->icon, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hb), t->label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hb), t->state[0], FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hb), t->state[1], FALSE, FALSE, 0);
  
  hb = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hb);
  gtk_container_add(GTK_CONTAINER(t->_widget), hb);

  t->_icon = gnome_stock_pixmap_widget_new(applet,
					  GNOME_STOCK_MENU_TRASH);
  t->_state[1] = gnome_stock_pixmap_widget_new(applet,
					      GNOME_STOCK_MENU_BOOK_OPEN);
  t->_state[0] = gnome_stock_pixmap_widget_new(applet,
					      GNOME_STOCK_MENU_BOOK_RED);
  t->_tooltip = (GtkWidget *) gtk_tooltips_new();
  gtk_tooltips_set_tip(GTK_TOOLTIPS(t->_tooltip),
		       t->_widget, t->name, t->name);
  t->_menu = gnome_popup_menu_new(uinfo);
  gnome_popup_menu_attach(t->_menu, t->_widget, t);

  gtk_box_pack_start(GTK_BOX(hb), t->_icon, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hb), t->_label, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hb), t->_state[0], FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hb), t->_state[1], FALSE, FALSE, 0);

  gtk_widget_show(t->widget);
  gtk_widget_show(t->label);
  gtk_widget_show(t->icon);
  gtk_widget_show(t->state[1]);
  gtk_signal_connect(GTK_OBJECT(t->widget), "clicked",
		     GTK_SIGNAL_FUNC(task_cb), t);
  gtk_signal_connect(GTK_OBJECT(t->widget), "enter",
		     GTK_SIGNAL_FUNC(enter_cb), t);
  gtk_signal_connect(GTK_OBJECT(t->widget), "leave",
		     GTK_SIGNAL_FUNC(leave_cb), t);
  gtk_widget_show(t->widget);

  gtk_widget_show(t->_widget);
  gtk_widget_show(t->_label);
  gtk_widget_show(t->_icon);
  gtk_widget_show(t->_state[1]);
  gtk_signal_connect(GTK_OBJECT(t->_widget), "clicked",
		     GTK_SIGNAL_FUNC(task_cb), t);
  gtk_signal_connect(GTK_OBJECT(t->_widget), "enter",
		     GTK_SIGNAL_FUNC(enter_cb), t);
  gtk_signal_connect(GTK_OBJECT(t->_widget), "leave",
		     GTK_SIGNAL_FUNC(leave_cb), t);
  gtk_widget_show(t->_widget);

  t->gdkwin = gdk_window_foreign_new(win);
  t->dummy = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_widget_realize(t->dummy);
  gdk_window_set_user_data(t->gdkwin, t->dummy);
  gtk_signal_connect(GTK_OBJECT(t->dummy), "property_notify_event",
		     GTK_SIGNAL_FUNC(client_prop_change_cb), t);
  gtk_signal_connect(GTK_OBJECT(t->dummy), "focus_in_event",
		     GTK_SIGNAL_FUNC(client_prop_change_cb), t);
  gtk_signal_connect(GTK_OBJECT(t->dummy), "focus_out_event",
		     GTK_SIGNAL_FUNC(client_prop_change_cb), t);
  XSelectInput(GDK_DISPLAY(), win, PropertyChangeMask | FocusChangeMask);

  gtk_widget_ref(t->widget);
  gtk_object_sink(GTK_OBJECT(t->widget));
  gtk_widget_ref(t->_widget);
  gtk_object_sink(GTK_OBJECT(t->_widget));

  tasks = g_list_append(tasks, t);
  client_prop_change_cb(NULL, NULL, t);
}

void 
del_task(Window win)
{
  guint               n, i;
  Task               *t;

  n = g_list_length(tasks);
  for (i = 0; i < n; i++)
    {
      t = (g_list_nth(tasks, i))->data;
      if (win == t->win)
	{
	  if (t->name)
	    g_free(t->name);
	  if (t->widget)
	    {
	      gtk_widget_unref(t->widget);
	      gtk_widget_destroy(t->widget);
	      gtk_widget_unref(t->_widget);
	      gtk_widget_destroy(t->_widget);
	    }
	  g_free(t);
	  tasks = g_list_remove(tasks, t);
	  return;
	}
    }
}

Task               *
find_task(Window win)
{
  guint               n, i;
  Task               *t;

  n = g_list_length(tasks);
  for (i = 0; i < n; i++)
    {
      t = (g_list_nth(tasks, i))->data;
      if (win == t->win)
	{
	  return t;
	}
    }
  return NULL;
}

void 
match_tasks(Window * win, guint num)
{
  guint               n, i, j, there;
  Task               *t;

  n = g_list_length(tasks);
  for (i = 0; i < n; i++)
    {
      t = (g_list_nth(tasks, i))->data;
      if (t)
	{
	  there = 0;
	  for (j = 0; j < num; j++)
	    {
	      if (win[j] == t->win)
		{
		  there = 1;
		  j = num;
		}
	    }
	  if (!there)
	    {
	      del_task(t->win);
	      n--;
	    }
	}
    }
  for (i = 0; i < num; i++)
    {
      there = 0;
      for (j = 0; j < n; j++)
	{
	  t = (g_list_nth(tasks, j))->data;
	  if (t->win == win[i])
	    {
	      there = 1;
	      j = n;
	    }
	}
      if (!there)
	{
	  add_task(win[i]);
	}
    }
}

void 
update_tasks(void)
{
  Window             *list;
  gint                left, top, i, num, size, tpl, maxh, num2, nn, chr;
  gint                rows = 2;
  Task               *t;
  gchar              *str;

  list = AtomGet(GDK_ROOT_WINDOW(), "WIN_CLIENT_LIST", XA_CARDINAL, &size);
  if ((size > 0) && (list))
    {
      num = size / sizeof(CARD32);
      match_tasks(list, num);
      g_free(list);
    }
  num = 0;
  num2 = g_list_length(tasks);
  for (i = 0; i < num2; i++)
    {
      t = (g_list_nth(tasks, i))->data;
      if ((t->desktop == (current_workspace - 1)) || (t->sticky))
	{
	  num++;
	}
    }
  top = 0;
  left = 0;
  maxh = 0;
  tpl = num / rows;
  if (tpl * rows < num)
    tpl++;

  if (tpl > 0)
    chr = 64 / ( tpl * 2);
  else
    chr = 16;
  if (chr < 2)
    chr = 2;
  tasks_in_row = chr;
  
  for (i = 0; i < num2; i++)
    {
      t = (g_list_nth(tasks, i))->data;
      if (t->name)
	{
	  str = munge(t->name, chr);
	  gtk_label_set(GTK_LABEL(t->label), str);
	  g_free(str);
	}
      else
	{
	  gtk_label_set(GTK_LABEL(t->label), _("??"));
	}
    }
  
  for (nn = 0; nn < workspace_count + 1; nn++)
    {
      size = 0;
      if (nn == 0)
	{
	  for (i = 0; i < num2; i++)
	    {
	      t = (g_list_nth(tasks, i))->data;
	      if (t->sticky)
		size++;
	    }
	}
      else
	{
	  for (i = 0; i < num2; i++)
	    {
	      t = (g_list_nth(tasks, i))->data;
	      if (t->desktop == nn - 1)
		size++;
	    }
	}
      if (size > 0)
	gtk_widget_show(flists[nn]);
      else
	gtk_widget_hide(flists[nn]);
    }
  for (i = 0; i < num2; i++)
    {
      t = (g_list_nth(tasks, i))->data;
      if (t->sticky)
	nn = 0;
      else
	nn = t->desktop + 1;
      if (t->_widget->parent != boxlists[nn])
	{
	  if (t->_widget->parent)
	    {
	      gtk_container_remove(GTK_CONTAINER(t->_widget->parent), 
				   t->_widget);
	    }
	  gtk_box_pack_start(GTK_BOX(boxlists[nn]), t->_widget,
			     FALSE, FALSE, 0);
	}
      
      if ((t->desktop == (current_workspace - 1)) || (t->sticky))
	{
	  if (t->widget->parent)
	    {
	      gtk_container_remove(GTK_CONTAINER(t->widget->parent), t->widget);
	    }
	  gtk_table_attach_defaults(GTK_TABLE(tasks1), t->widget,
					left, left + 1, top, top + 1);
	  left++;
	  if (left > maxh)
	    maxh = left;
	  if (left >= tpl)
	    {
	      left = 0;
	      top++;
	    }
	}
      else
	{
	  if (t->widget->parent)
	    {
	      gtk_container_remove(GTK_CONTAINER(t->widget->parent), t->widget);
	    }
	}
    }
  gtk_table_resize(GTK_TABLE(tasks1), top, maxh);
}

int 
main(int argc, char *argv[])
{

  panel_corba_register_arguments();

  applet_widget_init_defaults("wmpager_applet", NULL, argc, argv, 0, NULL, argv[0]);

  /* Get the Atom for making a window sticky, so that the detached pager appears on all screens */
  _XA_GWIN_STATE = XInternAtom(GDK_DISPLAY(), XA_GWIN_STATE, False);

  if (((_XA_GWIN_WORKSPACE = XInternAtom(GDK_DISPLAY(), XA_GWIN_WORKSPACE, True)) == None) ||
      (_XA_GWIN_WORKSPACE_NAMES = XInternAtom(GDK_DISPLAY(), XA_GWIN_WORKSPACE_NAMES, True)) == None)
    {
      /* FIXME: This should check to make sure icewm (or equiv) is running.
       * Not sure if it is working though... */
      g_error(_("This applet requires you to run a window manager with the XA_ extensions.\n"));
      exit(1);
    }

  applet = applet_widget_new();
  if (!applet)
    g_error("Can't create applet!\n");

  gtk_widget_realize(applet);

  workspace_list = get_workspaces();
  gdk_error_warnings = 0;
  setup();

  current_workspace = get_current_workspace();
  gtk_toggle_button_set_state
    (GTK_TOGGLE_BUTTON(g_list_nth(button_list, current_workspace)->data), 1);

  gtk_widget_show(applet);
  /*
   * gtk_signal_connect(GTK_OBJECT(applet),"destroy",
   * GTK_SIGNAL_FUNC(destroy_applet),
   * NULL);
   * gtk_signal_connect(GTK_OBJECT(applet),"session_save",
   * GTK_SIGNAL_FUNC(applet_session_save),
   * NULL);
   */
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"about",
					GNOME_STOCK_MENU_ABOUT,
					_("About..."),
					about_cb,
					NULL);

  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"properties",
					GNOME_STOCK_MENU_PROP,
					_("Properties..."),
					properties_dialog,
					NULL);

  gtk_signal_connect(GTK_OBJECT(applet), "change_orient",
		     GTK_SIGNAL_FUNC(applet_change_orient),
		     NULL);
  
  applet_widget_gtk_main();

  return 0;
}

GList              *
get_workspaces(void)
{
  GList              *tmp_list;
  XTextProperty       tp;
  char              **list;
  int                 count, i;

  XGetTextProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(), &tp,
		   _XA_GWIN_WORKSPACE_NAMES);
  XTextPropertyToStringList(&tp, &list, &count);
  tmp_list = g_list_alloc();
  for (i = 0; i < count; i++)
    tmp_list = g_list_append(tmp_list, g_strdup(list[i]));

  return tmp_list;
}

void 
setup(void)
{
  GtkWidget          *frame;
  GtkWidget          *hb, *vb;
  GtkWidget          *button;
  GtkWidget          *label;
  GtkWidget          *hbox;
  GtkWidget          *vbox;
  GtkWidget          *more;
  GtkWidget          *widg;
  GSList             *rblist = NULL;
  gchar               s[1024];

  gint                num_ws;	/* the number of workspaces */
  gint                even;	/* even number of workspaces? */
  gint                i;	/* counter for loops */

  workspace_list = g_list_first(workspace_list);
  button_list = g_list_alloc();
  workspace_count = num_ws = g_list_length(workspace_list) - 1;
  even = (((num_ws / 2) * 2) < num_ws) ? 0 : 1;

  gtk_window_set_policy(GTK_WINDOW(applet), 1, 1, 1);

  hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  gtk_container_add(GTK_CONTAINER(applet), hbox);

  frame = gtk_frame_new(NULL);
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  gtk_widget_show(frame);

  vbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

  tasks1 = gtk_table_new(2, 2, TRUE);
  gtk_widget_show(tasks1);
  gtk_box_pack_start(GTK_BOX(vbox), tasks1, FALSE, FALSE, 0);

  widg = gtk_frame_new(_("More"));
  gtk_widget_show(widg);
  gtk_box_pack_start(GTK_BOX(vbox), widg, FALSE, FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(widg), 4);

  more = gtk_button_new();
  gtk_widget_show(more);
  gtk_container_add(GTK_CONTAINER(widg), more);
  gtk_container_border_width(GTK_CONTAINER(more), 4);
  gtk_signal_connect(GTK_OBJECT(more), "clicked",
		     GTK_SIGNAL_FUNC(showpop_cb), NULL);

  widg = gtk_arrow_new(GTK_ARROW_UP, GTK_SHADOW_IN);
  gtk_widget_show(widg);
  gtk_container_add(GTK_CONTAINER(more), widg);

  vb = gtk_vbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER(frame), vb);
  gtk_widget_show(vb);

  hb = gtk_hbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vb), hb, TRUE, TRUE, 0);
  gtk_widget_show(hb);

  button = NULL;
  for (i = 1; i < ((num_ws + !even) / 2) + 1; i++)
    {
      workspace_list = g_list_next(workspace_list);
      button = gtk_radio_button_new_with_label(rblist, (gchar *) (workspace_list->data));
      gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button), 0);
      rblist = g_slist_append(rblist, button);
      button_list = g_list_append(button_list, button);
      gtk_signal_connect(GTK_OBJECT(button), "clicked",
			 GTK_SIGNAL_FUNC(switch_cb), (int *)i);
      gtk_box_pack_start(GTK_BOX(hb), button, TRUE, TRUE, 0);
      gtk_widget_show(button);
    }

  hb = gtk_hbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vb), hb, TRUE, TRUE, 0);
  gtk_widget_show(hb);

  for (i = ((num_ws + !even) / 2) + 1; i < num_ws + 1; i++)
    {
      workspace_list = g_list_next(workspace_list);
      button = gtk_radio_button_new_with_label(rblist, (gchar *) (workspace_list->data));
      gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button), 0);
      rblist = g_slist_append(rblist, button);
      button_list = g_list_append(button_list, button);
      gtk_signal_connect(GTK_OBJECT(button), "toggled",
			 GTK_SIGNAL_FUNC(switch_cb), (int *)i);
      gtk_box_pack_start(GTK_BOX(hb), button, TRUE, TRUE, 0);
      gtk_widget_show(button);
    }

  if (!even)
    {
      label = gtk_label_new("");
      gtk_box_pack_start(GTK_BOX(hb), label, TRUE, TRUE, 0);
      button_list = g_list_append(button_list, button);
      gtk_signal_connect(GTK_OBJECT(button), "toggled",
			 GTK_SIGNAL_FUNC(switch_cb), (int *)i);
      gtk_widget_show(label);
    }

  dummy_win = gtk_window_new(GTK_WINDOW_POPUP);
  dummy_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(dummy_box);
  gtk_container_add(GTK_CONTAINER(dummy_win), dummy_box);
  gtk_widget_realize(dummy_win);
  gdk_window_set_user_data(GDK_ROOT_PARENT(), dummy_win);
  gdk_window_ref(GDK_ROOT_PARENT());
  gdk_xid_table_insert(&gdk_root_window, GDK_ROOT_PARENT());
  gtk_signal_connect(GTK_OBJECT(dummy_win), "property_notify_event",
		     GTK_SIGNAL_FUNC(prop_change_cb), NULL);
  XSelectInput(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()), PropertyChangeMask);
  
  popbox = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_window_set_policy(GTK_WINDOW(popbox), 0, 0, 1);
  gtk_widget_set_events(popbox, GDK_BUTTON_PRESS_MASK | 
			GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK |
			GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK);
  gtk_signal_connect(GTK_OBJECT(popbox), "button_press_event",
		     GTK_SIGNAL_FUNC(showpop_cb), NULL);
  gtk_signal_connect(GTK_OBJECT(popbox), "button_release_event",
		     GTK_SIGNAL_FUNC(showpop_cb), NULL);

  qme = hb = gtk_frame_new(NULL);
  gtk_widget_show(hb);
  gtk_container_add(GTK_CONTAINER(popbox), hb);
  gtk_frame_set_shadow_type(GTK_FRAME(hb), GTK_SHADOW_OUT);

  qme2 = hbox = gtk_hbox_new(FALSE, 1);
  gtk_widget_show(hbox);
  gtk_container_add(GTK_CONTAINER(hb), hbox);

  boxlists = g_malloc(sizeof(GtkWidget *) * (num_ws + 1));
  flists = g_malloc(sizeof(GtkWidget *) * (num_ws + 1));
  
  for(i = 0; i < num_ws + 1; i++)
    {
      if (i == 0)
	{
	  widg = gtk_frame_new(_("Sticky"));
	  workspace_list = g_list_first(workspace_list);
	}
      else
	{
	  workspace_list = g_list_next(workspace_list);
	  g_snprintf(s, sizeof(s), "%s", (gchar *) (workspace_list->data));
	  widg = gtk_frame_new(s);
	}
      flists[i] = widg;
      gtk_widget_show(widg);
      gtk_frame_set_label_align(GTK_FRAME(widg), 0.5, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox), widg, FALSE, FALSE, 0);
      gtk_container_border_width(GTK_CONTAINER(widg), 4);
      more = gtk_alignment_new(0.5, 0.0, 0.0, 0.0);
      gtk_widget_show(more);
      gtk_container_add(GTK_CONTAINER(widg), more);
      boxlists[i] = gtk_vbox_new(FALSE, 0);
      gtk_widget_show(boxlists[i]);
      gtk_container_add(GTK_CONTAINER(more), boxlists[i]);
    }
  
  
  /* This is where the applet_widget_add belongs, but it seems to screw up the buttons and the handlebox. */
  /*
   * applet_widget_add(APPLET_WIDGET(applet), handle);
   */
  update_tasks();
  return;
}

void 
about_cb(AppletWidget * widget, gpointer data)
{
  GtkWidget          *about;
  const gchar        *authors[] =
  {"M.Watson", "The Rasterman", NULL};

  about = gnome_about_new
    (_("Desktop Pager Applet"), "0.1", _("Copyright (C)1998 M.Watson"),
     authors,
     _("Pager for icewm & Enlightenment (or other GNOME hints compliant) window manager"),
     NULL);
  gtk_widget_show(about);
  data = NULL;
  widget = NULL;
}

void 
properties_dialog(AppletWidget * widget, gpointer data)
{
  data = NULL;
  widget = NULL;
}

void 
prop_change_cb(GtkWidget * widget, GdkEventProperty * ev)
{
  gint                tmp_ws;
  GdkAtom             at;

  at = gdk_atom_intern(XA_GWIN_WORKSPACE, FALSE);
  if ((ev->atom == at) && (ev->state == GDK_PROPERTY_NEW_VALUE))
    {
      tmp_ws = get_current_workspace();

      if (tmp_ws != current_workspace)
	{
	  i_changed_it += 2;
	  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(g_list_nth(button_list, tmp_ws)->data), 1);
	  current_workspace = tmp_ws;
	  update_tasks();
	}
      return;
    }
  at = gdk_atom_intern("WIN_CLIENT_LIST", FALSE);
  if ((ev->atom == at) && (ev->state == GDK_PROPERTY_NEW_VALUE))
    {
      update_tasks();
      return;
    }
  widget = NULL;
}

void 
switch_cb(GtkWidget * widget, gpointer data)
{
  gint                button_num;
  gint                tmp_ws;

  tmp_ws = get_current_workspace();
  button_num = (int)data;
  if ((GTK_TOGGLE_BUTTON(widget)->active) && (i_changed_it == 0))
    {
      change_workspace(button_num);
    }
  if (i_changed_it)
    i_changed_it--;
  data = NULL;
  widget = NULL;
}

gint 
get_current_workspace(void)
{
  Atom                type;
  int                 format;
  unsigned long       nitems;
  unsigned long       bytes_after;
  unsigned char      *prop;
  CARD32              wws = 0;

  if ((XGetWindowProperty(GDK_DISPLAY(), GDK_ROOT_WINDOW(),
			  _XA_GWIN_WORKSPACE,
			  0, 1, False, XA_CARDINAL,
			  &type, &format, &nitems,
			  &bytes_after, &prop)) == 1)
    {
      g_error(_("Failed to retrieve workspace property."));
      exit(1);
    }

  wws = *(CARD32 *) prop;

  return wws + 1;
}

void 
change_workspace(gint ws)
{
  XEvent              xev;

  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = GDK_ROOT_WINDOW();
  xev.xclient.message_type = _XA_GWIN_WORKSPACE;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = ws - 1;
  xev.xclient.data.l[1] = CurrentTime;

  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
	     SubstructureNotifyMask, (XEvent *) & xev);
  current_workspace = ws;
  update_tasks();
  XSync(GDK_DISPLAY(), False);
}

gint 
check_workspace(gpointer data)
{
  gint                tmp_ws;

  tmp_ws = get_current_workspace();
  if (tmp_ws != current_workspace)
    {
      i_changed_it++;
      gtk_toggle_button_set_state
	(GTK_TOGGLE_BUTTON(g_list_nth(button_list, tmp_ws)->data), 1);
      current_workspace = tmp_ws;
      update_tasks();
    }
  return 1;
  data = NULL;
}

void 
make_sticky(Window win)
{
  XEvent              xev;

  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = win;
  xev.xclient.message_type = _XA_GWIN_STATE;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = WinStateAllWorkspaces;
  xev.xclient.data.l[1] = WinStateAllWorkspaces;
  xev.xclient.data.l[2] = CurrentTime;
  
  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False, 
	     SubstructureNotifyMask, (XEvent *) & xev);
  XSync(GDK_DISPLAY(), False);
}

void 
make_shaded(Window win)
{
  XEvent              xev;

  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = win;
  xev.xclient.message_type = _XA_GWIN_STATE;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = WinStateRollup;
  xev.xclient.data.l[1] = WinStateRollup;
  xev.xclient.data.l[2] = CurrentTime;
  
  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False, 
	     SubstructureNotifyMask, (XEvent *) & xev);
  XSync(GDK_DISPLAY(), False);
}

void 
make_unshaded(Window win)
{
  XEvent              xev;

  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = win;
  xev.xclient.message_type = _XA_GWIN_STATE;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = WinStateRollup;
  xev.xclient.data.l[1] = 0;
  xev.xclient.data.l[2] = CurrentTime;
  
  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False, 
	     SubstructureNotifyMask, (XEvent *) & xev);
  XSync(GDK_DISPLAY(), False);
}
