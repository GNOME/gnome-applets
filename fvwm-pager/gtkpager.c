#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <fvwm/module.h>
#include <fvwm/fvwm.h>
#include <libs/fvwmlib.h>

#include <X11/X.h>
#include <gdk/gdkx.h>

#include "gtkpager.h"
#include "pager-win.h"

#include "fvwm-pager.h"

static GtkFvwmPagerClass* parent_class = NULL;

static void gtk_fvwmpager_class_init  (GtkFvwmPagerClass* class);
static void gtk_fvwmpager_init        (GtkFvwmPager*      pager);
static void gtk_fvwmpager_finalize    (GtkObject*         object);

static gint desktop_event                   (GnomeCanvasItem* i, GdkEvent* e, gpointer data);
static gint canvas_item_event               (GnomeCanvasItem* i, GdkEvent* e, gpointer data);




enum {
  SWITCH_TO_DESK,
  LAST_SIGNAL
};


static guint pager_signals[LAST_SIGNAL] = {0 };


guint
gtk_fvwmpager_get_type(void)
{
  static guint fvwmpager_type = 0;

  if (!fvwmpager_type)
    {
      GtkTypeInfo fvwmpager_info =
      {
	"GtkFvwmPager",
	sizeof (GtkFvwmPager),
	sizeof (GtkFvwmPagerClass),
	(GtkClassInitFunc)  gtk_fvwmpager_class_init,
	(GtkObjectInitFunc) gtk_fvwmpager_init,
	(GtkArgSetFunc) NULL,
	(GtkArgGetFunc) NULL,
      };

      fvwmpager_type = gtk_type_unique(gnome_canvas_get_type(), &fvwmpager_info);
    }
  return fvwmpager_type;
}

typedef void (*GtkFvwmpagerSignal1) (GtkObject*,
				     guint arg1,
				     gpointer data);


static void
gtk_fvwmpager_marshal_signal_1(GtkObject*      object,
			       GtkSignalFunc   func,
			       gpointer        func_data,
			       GtkArg*         args)
{
  GtkFvwmpagerSignal1 rfunc;

  
  rfunc = (GtkFvwmpagerSignal1) func;

  (* rfunc)(object,
	    GTK_VALUE_UINT(args[0]),
	    func_data);
}

    

static void
gtk_fvwmpager_class_init(GtkFvwmPagerClass* klass)
{
  GtkObjectClass* object_class;

  object_class = (GtkObjectClass*) klass;
  object_class->finalize = gtk_fvwmpager_finalize;

  pager_signals[SWITCH_TO_DESK] =
    gtk_signal_new("switch_desktop",
		   GTK_RUN_FIRST,
		   object_class->type,
		   GTK_SIGNAL_OFFSET(GtkFvwmPagerClass, switch_to_desk),
		   gtk_fvwmpager_marshal_signal_1,
		   GTK_TYPE_NONE, 1, GTK_TYPE_INT);
  
  gtk_object_class_add_signals(object_class, pager_signals, LAST_SIGNAL);
  klass->switch_to_desk = gtk_fvwmpager_set_current_desk;
}

static guint
xidhash(gconstpointer key)
{
  return (guint)key;
}

static gint
xidcompare(gconstpointer a, gconstpointer b)
{
  return (gint)a == (gint)b;
}


static void
gtk_fvwmpager_init(GtkFvwmPager* pager)
{
  pager->desktops            = 0;
  pager->num_of_desks        = 0;
  pager->current_desktop     = 0;
  pager->windows             = g_hash_table_new(xidhash, xidcompare);
  

}


GtkWidget*
gtk_fvwmpager_new(gint* fd, gint width, gint height)
{
  gulong pg_type = gtk_fvwmpager_get_type();
  GtkWidget* pager;

  pager = gtk_type_new(pg_type);
  GTK_FVWMPAGER(pager)->width  = width;
  GTK_FVWMPAGER(pager)->height = height;
  GTK_FVWMPAGER(pager)->fd     = fd;
  gnome_canvas_set_scroll_region(GNOME_CANVAS(pager), 0.0, 0.0, width, height);
  return pager;

}

static void
gtk_fvwmpager_finalize(GtkObject* object)
{
  GtkFvwmPager* pager;
  
  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_FVWMPAGER(object));

  pager = GTK_FVWMPAGER(object);

  if (pager->desktops)
    /* @mla@ FIXME: free all desktops */
  (*GTK_OBJECT_CLASS(parent_class)->finalize)(object);
}




static gint
desktop_event(GnomeCanvasItem* i, GdkEvent* e, gpointer data)
{

  GdkEventButton* eb = (GdkEventButton*) e;
  GtkFvwmPager*   pager = GTK_FVWMPAGER(data);
  gint            desktop = 0;
  GList*          desks;

  if ((e->type == GDK_2BUTTON_PRESS && eb->button == 1) ||
      (e->type == GDK_BUTTON_PRESS && eb->button == 1 && eb->state & GDK_SHIFT_MASK))
    {
      
      desks = pager->desktops;
      while(desks)
	{
	  Desktop* d = (Desktop*)desks->data;
	  if (GNOME_CANVAS_ITEM(d->dt_group) == i)
	    break;
	  if (GNOME_CANVAS_ITEM(d->dt) == i)
	    break;
	  desktop++;
	  desks = g_list_next(desks);
	}
      if (!desks)
	{
	  g_log("fvwm-pager", G_LOG_LEVEL_ERROR,"Unknown desktop for event at item %p\n", i);
	  return FALSE;
	}
      gtk_signal_emit_by_name(GTK_OBJECT(pager), "switch_desktop", desktop);
      gtk_fvwmpager_set_current_desk(pager, desktop);
    }
  return FALSE;
}


static gint
canvas_item_event(GnomeCanvasItem* i, GdkEvent* e, gpointer data)
{

  GtkFvwmPager*    pager = (GtkFvwmPager*)data;

  static int       button_down = 0;
  static double    drag_start_x  = 0.0;
  static double    drag_start_y  = 0.0;
  GdkEventButton* eb = (GdkEventButton*) e;
  GdkEventMotion* em = (GdkEventMotion*) e;
  
  if (e->type == GDK_BUTTON_PRESS)
    {
      if (eb->button == 1 && ((eb->state & GDK_SHIFT_MASK) == 0))
	{
	  drag_start_x = eb->x;
	  drag_start_y = eb->y;
	  button_down = 1;
	  gnome_canvas_item_set(i, "x1", GNOME_CANVAS_WIN(i)->real_x1, NULL);
	  gnome_canvas_item_set(i, "x2", GNOME_CANVAS_WIN(i)->real_x2, NULL);
	  gnome_canvas_item_reparent(i, gnome_canvas_root(GNOME_CANVAS(pager)));
	  gnome_canvas_item_raise_to_top(i);
	}
    }
  if (e->type == GDK_BUTTON_RELEASE)
    {
      PagerWindow* window = g_hash_table_lookup(pager->windows, (gconstpointer)GNOME_CANVAS_WIN(i)->xid);
      if (eb->button == 1 && button_down)
	{
	  gint     desktop;
	  double   wx1;
	  double   wx2;
	  double   new_x;
	  double   new_y;
	  double   x_scale;
	  double   y_scale;
	  Desktop* desk;
	  char     cmd[80];

	  
	  button_down = 0;
	  desktop     = eb->x / (pager->width/pager->num_of_desks);
	  desk        = g_list_nth(pager->desktops, desktop)->data;

	  gnome_canvas_item_reparent(i, GNOME_CANVAS_GROUP(desk->dt_group));
	  window->desk = desk;
	  
	  /* redraw the	 window symbol */
	  wx1 = GNOME_CANVAS_RE(i)->x1;
	  wx2 = GNOME_CANVAS_RE(i)->x2;
	  
	  gnome_canvas_item_set(i, "wx1", wx1, NULL);
	  gnome_canvas_item_set(i, "wx2", wx2, NULL);
	  if (!( desktop == 0 && wx1 < 0))
	    if (wx1  < desktop * (pager->width / pager->num_of_desks))
	      {
		wx1 = desktop * (pager->width / pager->num_of_desks);
	      }
	  if (!(desktop >= pager->num_of_desks && wx2 > pager->width))
	    if (wx2 > (desktop + 1) * (pager->width / pager->num_of_desks))
		{
		  wx2 = (desktop + 1) * (pager->width / pager->num_of_desks);
		}
	  gnome_canvas_item_set(i, "x1", wx1, NULL);
	  gnome_canvas_item_set(i, "x2", wx2, NULL);
	  x_scale = (double)gdk_screen_width() / ((double)pager->width / (double)pager->num_of_desks);
	  y_scale = (double)gdk_screen_height() / (double)pager->height;
	  
	  new_x = (wx1 - (desktop * pager->width / pager->num_of_desks))  * x_scale;
	  new_y = GNOME_CANVAS_RE(i)->y1  * y_scale;
	  snprintf(cmd, sizeof(cmd)-1, "WindowsDesk %d", desktop);
	  if (window->flags & GTKPAGER_WINDOW_ICONIFIED)
	    {
	      XMoveWindow(GDK_DISPLAY(), window->ixid,
			  (int)new_x , (int)new_y );
	      SendInfo(pager->fd, cmd, window->ixid);
	    }
	  else
	    {
	      XMoveWindow(GDK_DISPLAY(), window->xid,
			  (int)new_x , (int)new_y + window->th);
	      SendInfo(pager->fd, cmd, window->xid);
	    }
	}
    }
  if (button_down && e->type == GDK_MOTION_NOTIFY)
    {

      double delta_x = em->x - drag_start_x;
      double delta_y = em->y - drag_start_y;
      
      drag_start_x = em->x;
      drag_start_y = em->y;
      gnome_canvas_item_move(i, delta_x, delta_y);

    }
  return FALSE;
}



void
gtk_fvwmpager_display_desks(GtkFvwmPager* pager, GList* desktops)
{
  GList*          desks;
  double          desk_width;
  double          startx = 0.0;
  gint            deskidx = 0;
  
  desks = pager->desktops;
  while (desks)
    {
      Desktop* d = desks->data;
      if (d->dt)
	gtk_object_destroy(GTK_OBJECT(d->dt));
      if (d->dt_group)
	gtk_object_destroy(GTK_OBJECT(d->dt_group));
      desks = g_list_remove_link(desks, desks);
      g_free(d);
    }
  pager->num_of_desks = g_list_length(desktops);

  
  desk_width = pager->width / pager->num_of_desks;

  desks = pager->desktops = desktops;
  while(desks)
    {
      Desktop* d;
      d = (Desktop*)desks->data;

      d->dt_group = gnome_canvas_item_new(gnome_canvas_root(GNOME_CANVAS(pager)),
					  gnome_canvas_group_get_type(),
					  "x",    (double)0.0,
					  "y",    (double)0.0,
					  NULL);
      d->dt = gnome_canvas_item_new(gnome_canvas_root(GNOME_CANVAS(pager)),
				    gnome_canvas_rect_get_type(),
				    "x1",           startx,
				    "y1",           0.0,
				    "x2",           startx + desk_width,
				    "y2",           100.0,
				    "fill_color",   pager_props.inactive_desk_color,
				    "outline_color","black",
				    "width_pixels", 1,
				    NULL);

      gnome_canvas_item_lower_to_bottom(d->dt);
      gtk_signal_connect(GTK_OBJECT(d->dt), "event", GTK_SIGNAL_FUNC(desktop_event), pager);
      gtk_signal_connect(GTK_OBJECT(d->dt_group), "event", GTK_SIGNAL_FUNC(desktop_event), pager);
      d->idx = deskidx++;
      d->x1  = startx;
      d->x2  = startx + desk_width;
      d->y1  = 0;
      d->y2  = pager->height;
      d->x_scale = gdk_screen_width() / (double)(pager->width / pager->num_of_desks);
      d->y_scale = gdk_screen_height() / (double)pager->height;
      startx += desk_width;
      desks = g_list_next(desks);
    }
}

void
gtk_fvwmpager_set_current_desk(GtkFvwmPager* pager, int desktop)
{
  if (desktop > pager->num_of_desks)
    {
      g_log("fvwm-pager", G_LOG_LEVEL_ERROR, "desktop requested: %d, max = %d\n",desktop, pager->num_of_desks);
      return;
    }
  if (!pager->current_desktop || pager->current_desktop->idx != desktop)
    {
      
      GnomeCanvasItem* i;
      if (pager->current_desktop && pager->current_desktop->dt)
	{
	  i = GNOME_CANVAS_ITEM(pager->current_desktop->dt);
	  gnome_canvas_item_set(i, "fill_color", pager_props.inactive_desk_color, NULL);
	}
      pager->current_desktop     = (Desktop*)g_list_nth(pager->desktops, desktop)->data;
      i = GNOME_CANVAS_ITEM(pager->current_desktop->dt);
      gnome_canvas_item_set(i, "fill_color", pager_props.active_desk_color, NULL);
    }
}

void
gtk_fvwmpager_display_window(GtkFvwmPager* pager, PagerWindow* old_window, gint xid)
{
  PagerWindow* new_window;
  double wx1;
  double wx2;
  double wy1;
  double wy2;
  double x1;
  double x2;
  gint   winx;
  gint   winy;
  gint   winh;
  gint   winw;
  
  new_window = g_hash_table_lookup(pager->windows, (gconstpointer)xid);

  if (old_window && old_window->desk != new_window->desk)
    {
      if (old_window->desk)
	{
	  gnome_canvas_item_reparent(old_window->rect, GNOME_CANVAS_GROUP(new_window->desk->dt_group));
	}
      else
	{
	  g_log("fvwm-pager", G_LOG_LEVEL_ERROR, "old window without desktop\n");
	  return;
	}
    }
  
  if (!new_window->desk)
    {
      g_log("fvwm-pager", G_LOG_LEVEL_ERROR, "new window without desktop????\n");
      return;
    }
  
  if (new_window->flags & GTKPAGER_WINDOW_ICONIFIED)
    {
      winx = new_window->ix;
      winy = new_window->iy;
      winh = new_window->ih;
      winw = new_window->iw;
    }
  else
    {
      winx = new_window->x;
      winy = new_window->y;
      winh = new_window->h;
      winw = new_window->w;
    }
  
  x1 = wx1 = (winx / new_window->desk->x_scale) + new_window->desk->x1;
  x2 = wx2 = ((winx + winw) / new_window->desk->x_scale) + new_window->desk->x1;
  wy1 = winy / new_window->desk->y_scale;
  wy2 = (winy + winh) / new_window->desk->y_scale;

  if (!(new_window->desk->idx == 0 && wx1 < 0))
    if (wx1  < new_window->desk->idx * (pager->width / pager->num_of_desks))
      {
	x1 = new_window->desk->idx * (pager->width / pager->num_of_desks);
      }

  if (!(new_window->desk->idx >= pager->num_of_desks && wx2 > pager->width))
    if (wx2 > (new_window->desk->idx + 1) * (pager->width / pager->num_of_desks))
      {
	x2 = (new_window->desk->idx + 1) * (pager->width / pager->num_of_desks);
      }
  
  if (new_window->rect)
    {
      gnome_canvas_item_set(GNOME_CANVAS_ITEM(new_window->rect),
			    "wx1",      wx1,
			    "wx2",      wx2,
			    "wy1",      wy1,
			    "wy2",      wy2,
			    "x1",       x1,
			    "x2",       x2,
			    NULL);
    }
  else
    {
        new_window->rect = gnome_canvas_item_new(GNOME_CANVAS_GROUP(new_window->desk->dt_group),
						 gnome_canvas_win_get_type(),
						 "xid",           xid,
						 "wx1",           wx1,
						 "wx2",           wx2,
						 "wy1",           wy1,
						 "wy2",           wy2,
						 "x1",            x1,
						 "x2",            x2,
						 "fill_color",    pager_props.inactive_win_color,
						 "outline_color", "black",
						 "width_pixels",  1,
						 NULL);
	gtk_signal_connect(GTK_OBJECT(new_window->rect), "event",
			   GTK_SIGNAL_FUNC(canvas_item_event), pager);
    }
}


void
gtk_fvwmpager_destroy_window(GtkFvwmPager* pager, gint xid)
{

  PagerWindow* window = g_hash_table_lookup(pager->windows, (gconstpointer) xid);

  gtk_object_destroy(GTK_OBJECT(window->rect));
  g_hash_table_remove(pager->windows, (gconstpointer) xid);
  g_free(window);
}



void
gtk_fvwmpager_raise_window(GtkFvwmPager* pager, gint xid)
{

  PagerWindow* window = g_hash_table_lookup(pager->windows, (gconstpointer)xid);

  if (!window)
    return;
  gnome_canvas_item_raise_to_top(GNOME_CANVAS_ITEM(window->rect));
}


void
gtk_fvwmpager_lower_window(GtkFvwmPager* pager, gint xid)
{
  PagerWindow* window = g_hash_table_lookup(pager->windows, (gconstpointer)xid);

  if (!window)
    return;

  gnome_canvas_item_lower_to_bottom(GNOME_CANVAS_ITEM(window->rect));
}
      

  
void
gtk_fvwmpager_label_desk(GtkFvwmPager* pager, int idx, char* label)
{
#if 0
  GList* elem = g_list_nth(pager->desktops, idx);
  struct Desk* desk = elem->data;

  free(desk->title);
  desk->title = strdup(label);
#endif
}

  
