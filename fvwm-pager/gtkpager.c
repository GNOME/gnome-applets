#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "gtkpager.h"
#include <gtk/gtkdrawingarea.h>

static GtkFvwmPagerClass* parent_class = NULL;

static void gtk_fvwmpager_class_init  (GtkFvwmPagerClass* class);
static void gtk_fvwmpager_init        (GtkFvwmPager*      pager);
static void gtk_fvwmpager_finalize    (GtkObject*         object);
static gint gtk_fvwmpager_graph_events(GtkWidget*         widget,
				       GdkEvent*          event,
				       GtkFvwmPager*      pager);
static gint gtk_fvwmpager_expose      (GtkWidget*         widget,
				       GdkEventExpose*    event,
				       GtkFvwmPager*      pager);
static gint gtk_fvwmpager_move        (GtkWidget*         widget,
				       GdkEvent*          event,
				       GtkFvwmPager*      pager);

static gint gtk_fvwmpager_buttonpress (GtkWidget*         widget,
				       GdkEventButton*    event,
				       GtkFvwmPager*      pager);

static gint gtk_fvwmpager_configure   (GtkWidget*         widget,
				       GdkEvent*          event,
				       GtkFvwmPager*      pager);

static void emit_desktop_switch       (GtkWidget*         widget,
				       guint              desktop_offset);
static void
gtk_fvwmpager_switch_to_desk(GtkWidget* w, guint desktop);

void
gtk_fvwmpager_destroy_window(GtkFvwmPager* pager, unsigned long xid);

void
gtk_fvwmpager_deiconify_window(GtkFvwmPager* pager, unsigned long xid);

void
gtk_fvwmpager_raise_window(GtkFvwmPager* pager, unsigned long xid);
void
gtk_fvwmpager_lower_window(GtkFvwmPager* pager, unsigned long xid);

enum {
  SWITCH_TO_DESK,
  LAST_SIGNAL
};


static guint pager_signals[LAST_SIGNAL] = {0 };

static __inline__ int max(gint i1, gint i2)
{
  return i1 > i2 ? i1 : i2;
}


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

      fvwmpager_type = gtk_type_unique(gtk_drawing_area_get_type(),
				       &fvwmpager_info);
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

  parent_class = gtk_type_class(gtk_drawing_area_get_type());

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
  klass->switch_to_desk = gtk_fvwmpager_switch_to_desk;
}

static void
gtk_fvwmpager_init(GtkFvwmPager* pager)
{
  gint old_mask;
  
  pager->desktops = 0;
  pager->xwins    = 0;
  pager->pixmap   = 0;
  pager->num_of_desks = 0;

  pager->desk_fg = 0;
  pager->desk_bg = 0;
  pager->current_desk_fg = 0;
  pager->current_desk_bg = 0;
  pager->window_bg = 0;
  pager->window_focus_bg = 0;

  pager->current_desk_gc = 0;
  pager->desk_gc = 0;
  pager->window_gc = 0;
  pager->window_focus_gc = 0;
  
  gtk_widget_set_events(GTK_WIDGET(pager),
			GDK_EXPOSURE_MASK
			| GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_POINTER_MOTION_MASK
			| GDK_POINTER_MOTION_HINT_MASK);

  gtk_signal_connect(GTK_OBJECT(pager), "expose_event",
		     (GtkSignalFunc) gtk_fvwmpager_expose, pager);
  gtk_signal_connect(GTK_OBJECT(pager), "motion_notify_event",
		     (GtkSignalFunc) gtk_fvwmpager_move, pager);
  gtk_signal_connect(GTK_OBJECT(pager), "configure_event",
		     (GtkSignalFunc) gtk_fvwmpager_configure, pager);
  gtk_signal_connect(GTK_OBJECT(pager), "button_press_event",
		     (GtkSignalFunc) gtk_fvwmpager_buttonpress, pager);
}


GtkWidget*
gtk_fvwmpager_new(void)
{
  
  return gtk_type_new(gtk_fvwmpager_get_type());
}

static void
gtk_fvwmpager_finalize(GtkObject* object)
{
  GtkFvwmPager* pager;
  
  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_FVWMPAGER(object));

  pager = GTK_FVWMPAGER(object);

  if (pager->desktops)
    /* free all desktops */
    fprintf(stderr,"gtk_fvwmpager_finalize: dekstops stilll there\n");
  (*GTK_OBJECT_CLASS(parent_class)->finalize)(object);
}


static void
gtk_fvwmpager_draw(GtkFvwmPager* pager)
{

  gint             desk_width = 0;
  gint             desk_height = 0;
  GtkWidget*       widget = GTK_WIDGET(pager);
  GdkRectangle     rect;
  int idx;
  double           screen_width;
  double           screen_height;
  double           scale_x;
  double           scale_y;
  
  screen_width  = (double) gdk_screen_width();
  screen_height = (double) gdk_screen_height();

  if (!pager->desk_fg)
    {
      pager->desk_fg = malloc(sizeof(GdkColor));
      gdk_color_parse("blue", pager->desk_fg);
      gdk_color_alloc(gtk_widget_get_colormap(widget), pager->desk_fg);
    }

  if (!pager->desk_bg)
    {
      pager->desk_bg = malloc(sizeof(GdkColor));
      gdk_color_parse("red", pager->desk_bg);
      gdk_color_alloc(gtk_widget_get_colormap(widget), pager->desk_bg);
    }

  if (!pager->current_desk_fg)
    {
      pager->current_desk_fg = malloc(sizeof(GdkColor));
      gdk_color_parse("yellow", pager->current_desk_fg);
      gdk_color_alloc(gtk_widget_get_colormap(widget), pager->current_desk_fg);
    }
  if (!pager->current_desk_bg)
    {
      pager->current_desk_bg = malloc(sizeof(GdkColor));
      gdk_color_parse("green", pager->current_desk_bg);
      gdk_color_alloc(gtk_widget_get_colormap(widget), pager->current_desk_bg);
    }

  if (!pager->window_bg)
    {
      pager->window_bg = malloc(sizeof(GdkColor));
      gdk_color_parse("white", pager->window_bg);
      gdk_color_alloc(gtk_widget_get_colormap(widget), pager->window_bg);
    }

  if (!pager->window_focus_bg)
    {
      pager->window_focus_bg = malloc(sizeof(GdkColor));
      gdk_color_parse("PeachPuff", pager->window_focus_bg);
      gdk_color_alloc(gtk_widget_get_colormap(widget), pager->window_focus_bg);
    }

  if (!pager->pixmap)
    {
      pager->pixmap = gdk_pixmap_new(GTK_WIDGET(pager)->window,
				     GTK_WIDGET(pager)->allocation.width,
				     GTK_WIDGET(pager)->allocation.height,
				     -1);
    }
  if (pager->num_of_desks)
    {

      
      if (!pager->current_desk_gc)
	{
	  pager->current_desk_gc = gdk_gc_new(GTK_WIDGET(pager)->window);
	  gdk_gc_copy(pager->current_desk_gc, widget->style->bg_gc[GTK_WIDGET_STATE(widget)]);
	  gdk_gc_set_foreground(pager->current_desk_gc, pager->current_desk_fg);
	}
      
      if (!pager->desk_gc)
	{
	  pager->desk_gc = gdk_gc_new(GTK_WIDGET(pager)->window);
	  gdk_gc_copy(pager->desk_gc, widget->style->bg_gc[GTK_WIDGET_STATE(widget)]);
	  gdk_gc_set_foreground(pager->desk_gc, pager->desk_fg);
	}

      if (!pager->window_gc)
	{
	  pager->window_gc = gdk_gc_new(GTK_WIDGET(pager)->window);
	  gdk_gc_copy(pager->window_gc, widget->style->bg_gc[GTK_WIDGET_STATE(widget)]);
	  gdk_gc_set_background(pager->window_gc, pager->window_bg);
	}

      if (!pager->window_focus_gc)
	{
	  pager->window_focus_gc = gdk_gc_new(GTK_WIDGET(pager)->window);
	  gdk_gc_copy(pager->window_gc, widget->style->bg_gc[GTK_WIDGET_STATE(widget)]);
	  gdk_gc_set_foreground(pager->window_focus_gc, pager->window_focus_bg);
	}
      
      desk_width  = widget->allocation.width / (pager->num_of_desks);
      desk_height = widget->allocation.height;

      gdk_draw_rectangle(pager->pixmap,
			 pager->desk_gc,
			 1,
			 0,
			 0,
			 widget->allocation.width,
			 widget->allocation.height);
      
      scale_x = widget->allocation.width / screen_width / pager->num_of_desks;
      scale_y = widget->allocation.height / screen_height;
      
      for (idx = 0; idx < pager->num_of_desks; idx++)
	{
	  gdk_draw_rectangle(pager->pixmap,
			     (idx == pager->current_desktop_idx) ?
			     pager->current_desk_gc : pager->desk_gc,
			     1,
			     desk_width  * idx,
			     0,
			     desk_width,
			     desk_height);
	  gdk_draw_line(pager->pixmap,
			widget->style->black_gc,
			desk_width * (idx),
			desk_height,
			desk_width * (idx),
			0);
	  
	}
      for (idx = 0; idx < pager->num_of_wins; idx++)
	{
	  int x;
	  int y;
	  int w;
	  int h;
	  struct Xwin* win;
	  GList* ptr;

	  ptr = g_list_nth(pager->xwins, idx);
	  win = (struct Xwin*)ptr->data;
	  if (win->flags & GTKPAGER_WINDOW_ICONIFIED)
	    {
	      x = (int)(win->icon_x * scale_x);
	      y = (int)(win->icon_y * scale_y);
	      w = max(1,(int)(win->icon_w * scale_x));
	      h = max(1,(int)(win->icon_h * scale_y));
	    }
	  else
	    {
	      x = (int)(win->x * scale_x);
	      y = (int)(win->y * scale_y);
	      w = max(1,(int)(win->w * scale_x));
	      h = max(1,(int)(win->h * scale_y));
	    }	      
	  fprintf(stderr," Drawing Window at (%d,%d) with (%d,%d)\n",
		  x, y, w, h);
	  x += win->desktop_idx * desk_width;
	  gdk_draw_rectangle(pager->pixmap,
			     pager->window_gc,
			     1,
			     x,
			     y,
			     w,
			     h);
	  gdk_draw_rectangle(pager->pixmap,
			     widget->style->black_gc,
			     0,
			     x,
			     y,
			     w,
			     h);
	}
      rect.x = 0;
      rect.y = 0;
      rect.width = widget->allocation.width;
      rect.height= widget->allocation.height;
      gtk_widget_draw(GTK_WIDGET(pager), &rect);
    }
}

static gint
gtk_fvwmpager_expose(GtkWidget* widget, GdkEventExpose* event, GtkFvwmPager* 
		     pager)
{
  if (pager->pixmap) {
    gdk_draw_pixmap(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
		    pager->pixmap,
		    /*
		      event->area.x, event->area.y,
		      event->area.x, event->area.y,
		      event->area.width, event->area.height);
		    */
		    0, 0,
		    0, 0,
		    widget->allocation.width,
		    widget->allocation.height);
  }
  return TRUE;
}
      
static gint
gtk_fvwmpager_configure(GtkWidget* widget, GdkEvent* event, GtkFvwmPager* 
		     pager)
{
  GList* ptr;
  int idx;
  gint desk_width = 0;
  gint desk_height = 0;
  
  if (pager->num_of_desks)
    {
      desk_width  = widget->allocation.width / (pager->num_of_desks);
      desk_height = widget->allocation.height;
    }
  if (pager->pixmap) {
    gdk_pixmap_unref(pager->pixmap);
  }
  pager->pixmap = gdk_pixmap_new(GTK_WIDGET(pager)->window,
				 GTK_WIDGET(pager)->allocation.width,
				 GTK_WIDGET(pager)->allocation.height,
				 -1);
  gdk_draw_rectangle(pager->pixmap,
		     widget->style->white_gc,
		     TRUE,
		     0, 0,
		     widget->allocation.width,
		     widget->allocation.height);
  
  gtk_fvwmpager_draw(pager);
  return TRUE;
}

static gint
gtk_fvwmpager_move(GtkWidget* widget, GdkEvent* event, GtkFvwmPager*
		   pager)
{
  return FALSE;
}


static void
gtk_fvwmpager_switch_to_desk(GtkWidget* w, guint desktop)
{
  fprintf(stderr,"GnomePager: gtk_switch_to_desk %d called\n", desktop);
}
  
void
gtk_fvwmpager_add_desk(GtkFvwmPager* pager, char* name)
{
  struct Desk* desktop;
  
  desktop = malloc(sizeof (struct Desk));
  desktop->title = strdup(name);
  desktop->fg = 0;
  desktop->bg = 0;
  pager->desktops = g_list_append(pager->desktops, desktop);
  pager->num_of_desks++;
}

void
gtk_fvwmpager_set_current_desk(GtkFvwmPager* pager, int desktop)
{
  
  if (pager->current_desktop_idx != desktop)
    {
      pager->current_desktop_idx = desktop;
      pager->current_desktop     = (struct Desk*)g_list_nth(pager->desktops, desktop);
      gtk_fvwmpager_draw(pager);
    }
  
}



void
gtk_fvwmpager_add_window(GtkFvwmPager* pager, struct Xwin* win)
{

  pager->xwins = g_list_append(pager->xwins, win);
  pager->num_of_wins++;
  gtk_fvwmpager_draw(pager);

}


static GList*
glist_splice(GList* list1, GList* list2)
{
  list1->next->prev = g_list_last(list2);
  g_list_last(list2)->next = list1->next;
  list1->next = list2;
  list2->prev = list1;
  return list1;
}
  
  
static void
copy_relevant_xwin(struct Xwin* old_xwin, struct Xwin* new_xwin)
{
  old_xwin->xid     = new_xwin->xid;
  old_xwin->flags   = new_xwin->flags;
  old_xwin->x       = new_xwin->x;
  old_xwin->y       = new_xwin->y;

  old_xwin->h       = new_xwin->h;
  old_xwin->w       = new_xwin->w;

  old_xwin->icon_x  = new_xwin->icon_x;
  old_xwin->icon_y  = new_xwin->icon_y;
  old_xwin->icon_w  = new_xwin->icon_w;
  old_xwin->icon_h  = new_xwin->icon_h;
  
}  
  

static void
gtk_fvwmpager_reconf_window(GtkFvwmPager* pager, GList* old,
			    struct Xwin* new_xwin)
{

  struct Xwin* old_xwin = old->data;
  
  if (new_xwin->desktop_idx == -1)
    {
      new_xwin->desktop_idx = old_xwin->desktop_idx;
    }
  copy_relevant_xwin(old_xwin, new_xwin);

  gtk_fvwmpager_draw(pager);
}


void
gtk_fvwmpager_conf_window(GtkFvwmPager* pager, struct Xwin* win)
{
  GList* elem = pager->xwins;
  struct Xwin* old_win;
  
  while(elem)
    {
      old_win = elem->data;
      if (old_win->xid == win->xid)
	{
	  gtk_fvwmpager_reconf_window(pager, elem, win);
	  return;
	}
      elem = g_list_next(elem);
    }
  gtk_fvwmpager_add_window(pager, win);
  
}

void
gtk_fvwmpager_destroy_window(GtkFvwmPager* pager, unsigned long xid)
{
  GList* elem = pager->xwins;
  struct Xwin* old_win;

  while (elem)
    {
      old_win = elem->data;
      if (old_win->xid == xid)
	{
	  pager->xwins = g_list_remove_link(pager->xwins, elem);
	  free(elem->data);
	  pager->num_of_wins--;
	  gtk_fvwmpager_draw(pager);
	  return;
	}
      elem = g_list_next(elem);
    }
}


void
gtk_fvwmpager_deiconify_window(GtkFvwmPager* pager, unsigned long xid)
{
  GList* elem = pager->xwins;
  struct Xwin* win;
  
  while(elem)
    {
      win = elem->data;
      if (win->xid == xid)
	{
	  win->flags   &= ~GTKPAGER_WINDOW_ICONIFIED;
	  gtk_fvwmpager_draw(pager);
	  return;
	}
      elem = g_list_next(elem);
    }
}
	 

void
gtk_fvwmpager_raise_window(GtkFvwmPager* pager, unsigned long xid)
{
  GList* elem = pager->xwins;
  struct Xwin*  xwin;

  while (elem)
    {
      xwin = elem->data;
      if (xwin->xid == xid)
	{
	  pager->xwins = g_list_remove_link(pager->xwins, elem);
	  xwin = elem->data;
	  g_list_free_1(elem);
	  pager->xwins = g_list_append(pager->xwins, xwin);
	  gtk_fvwmpager_draw(pager);
	  return;
	}
      elem = g_list_next(elem);
    }
}

void
gtk_fvwmpager_lower_window(GtkFvwmPager* pager, unsigned long xid)
{
  GList* elem = pager->xwins;
  struct Xwin*  xwin;

  while (elem)
    {
      xwin = elem->data;
      if (xwin->xid == xid)
	{
	  pager->xwins = g_list_remove_link(pager->xwins, elem);
	  xwin = elem->data;
	  g_list_free_1(elem);
	  pager->xwins = g_list_prepend(pager->xwins, xwin);
	  gtk_fvwmpager_draw(pager);
	  return;
	}
      elem = g_list_next(elem);
    }
}
      

static gint
gtk_fvwmpager_buttonpress(GtkWidget* widget, GdkEventButton* event, GtkFvwmPager* pager)
{
  int desk_width =  (widget->allocation.width / pager->num_of_desks);
  int desk_number = (int)event->x / desk_width;
  
#if 0
  fprintf(stderr,"GnomePager: event->x      = %g\n", event->x);
  fprintf(stderr,"            event->y      = %g\n", event->y);
  fprintf(stderr,"            event->button = %d\n", event->button);
  fprintf(stderr,"            dekwidth      = %d\n", desk_width);
  fprintf(stderr,"            desktop       = %d\n", desk_number);
#endif
  if (event->button == 1)
    {
      gtk_signal_emit_by_name(GTK_OBJECT(pager), "switch_desktop",
			      desk_number);
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}
  
void
gtk_fvwmpager_label_desk(GtkFvwmPager* pager, int idx, char* label)
{
  GList* elem = g_list_nth(pager->desktops, idx);
  struct Desk* desk = elem->data;

  free(desk->title);
  desk->title = strdup(label);
}

  
