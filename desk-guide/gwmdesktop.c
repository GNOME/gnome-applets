/* GwmDesktop - desktop widget
 * Copyright (C) 1999 Tim Janik
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * this code is loosely based on the original gnomepager_applet
 * implementation of The Rasterman (Carsten Haitzler) <raster@rasterman.com>
 */
#include "gwmdesktop.h"
#include "stripe.xbm"
#include <gtk/gtkprivate.h>

typedef struct
{
  gint  x, y;
  guint width, height;
} GrabArea;


/* --- prototypes --- */
static void	gwm_desktop_class_init	  (GwmDesktopClass    *klass);
static void	gwm_desktop_init	  (GwmDesktop	      *desktop);
static void	gwm_desktop_destroy	  (GtkObject	      *object);
static gboolean gwm_desktop_desk_notifier (gpointer            func_data,
					   GwmhDesk           *desk,
					   GwmhDeskInfoMask    change_mask);
static gboolean gwm_desktop_task_notifier (gpointer	       func_data,
					   GwmhTask           *task,
					   GwmhTaskNotifyType  ntype,
					   GwmhTaskInfoMask    imask);
static void     gwm_desktop_realize       (GtkWidget          *widget);
static void     gwm_desktop_size_allocate (GtkWidget          *widget,
					   GtkAllocation      *allocation);
static void     gwm_desktop_unrealize     (GtkWidget          *widget);
static void	gwm_desktop_size_request  (GtkWidget          *widget,
					   GtkRequisition     *requisition);
static gboolean	gwm_desktop_expose	  (GtkWidget          *widget,
					   GdkEventExpose     *event);
static void	gwm_desktop_draw	  (GtkWidget          *widget,
					   GdkRectangle	      *area);
static gboolean	gwm_desktop_button_press  (GtkWidget          *widget,
					   GdkEventButton     *event);
static gboolean gwmh_desktop_motion	  (GtkWidget          *widget,
					   GdkEventMotion     *event);


/* --- static variables --- */
static gpointer         parent_class = NULL;
static GwmDesktopClass *gwm_desktop_class = NULL;
static GQuark		quark_grab_area = 0;


/* --- functions --- */
GtkType
gwm_desktop_get_type (void)
{
  static GtkType desktop_type = 0;
  
  if (!desktop_type)
    {
      GtkTypeInfo desktop_info =
      {
	"GwmDesktop",
	sizeof (GwmDesktop),
	sizeof (GwmDesktopClass),
	(GtkClassInitFunc) gwm_desktop_class_init,
	(GtkObjectInitFunc) gwm_desktop_init,
        /* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
	(GtkClassInitFunc) NULL,
      };
      
      desktop_type = gtk_type_unique (GTK_TYPE_DRAWING_AREA, &desktop_info);
    }
  
  return desktop_type;
}

static void
gwm_desktop_class_init (GwmDesktopClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = GTK_OBJECT_CLASS (class);
  widget_class = GTK_WIDGET_CLASS (class);

  gwm_desktop_class = class;
  parent_class = gtk_type_class (GTK_TYPE_DRAWING_AREA);

  quark_grab_area = g_quark_from_static_string ("gwm_grab-area");

  object_class->destroy = gwm_desktop_destroy;

  widget_class->realize = gwm_desktop_realize;
  widget_class->size_allocate = gwm_desktop_size_allocate;
  widget_class->unrealize = gwm_desktop_unrealize;
  widget_class->size_request = gwm_desktop_size_request;
  widget_class->expose_event = gwm_desktop_expose;
  widget_class->draw = gwm_desktop_draw;
  widget_class->button_press_event = gwm_desktop_button_press;
  widget_class->motion_notify_event = gwmh_desktop_motion;

  class->orientation = GTK_ORIENTATION_HORIZONTAL;
  class->area_size = 22;
  class->double_buffer = TRUE;
}

static void
gwm_desktop_init (GwmDesktop *desktop)
{
  GtkWidget *widget = GTK_WIDGET (desktop);
  GwmhDesk *desk = gwmh_desk_get_config ();

  GTK_WIDGET_UNSET_FLAGS (desktop, GTK_NO_WINDOW);

  desktop->index = 0;
  desktop->harea = 0;
  desktop->varea = 0;
  desktop->last_desktop = desk->current_desktop;
  desktop->task_list = NULL;
  
  desktop->bitmap = NULL;
  desktop->pixmap = NULL;
  desktop->tooltips = NULL;
  
  gtk_widget_set_events (widget,
			 GDK_BUTTON_PRESS_MASK |
			 GDK_BUTTON_RELEASE_MASK |
			 GDK_BUTTON1_MOTION_MASK |
			 GDK_EXPOSURE_MASK |
			 GDK_ENTER_NOTIFY_MASK |
			 GDK_LEAVE_NOTIFY_MASK);

  gwmh_desk_notifier_add (gwm_desktop_desk_notifier, desktop);
  gwmh_task_notifier_add (gwm_desktop_task_notifier, desktop);
}

static void
gwm_desktop_destroy (GtkObject *object)
{
  GwmDesktop *desktop;

  g_return_if_fail (object != NULL);

  desktop = GWM_DESKTOP (object);

  gwmh_desk_notifier_remove_func (gwm_desktop_desk_notifier, desktop);
  gwmh_task_notifier_remove_func (gwm_desktop_task_notifier, desktop);

  desktop->index = 0;
  desktop->harea = 0;
  desktop->varea = 0;
  desktop->last_desktop = 0;
  g_list_free (desktop->task_list);
  desktop->task_list = NULL;
  desktop->grab_task = NULL;
  
  if (desktop->tooltips)
    {
      gtk_object_unref (GTK_OBJECT (desktop->tooltips));
      desktop->tooltips = NULL;
    }

  GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gwm_desktop_realize (GtkWidget *widget)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  GwmDesktopClass *class = GWM_DESKTOP_GET_CLASS (desktop);

  GTK_WIDGET_CLASS (parent_class)->realize (widget);

  desktop->bitmap = gdk_bitmap_create_from_data (widget->window,
						 stripe_bits,
						 stripe_width,
						 stripe_height);
  if (class->double_buffer)
    desktop->pixmap = gdk_pixmap_new (widget->window,
				      widget->allocation.width,
				      widget->allocation.height,
				      -1);
}

static void
gwm_desktop_size_allocate (GtkWidget     *widget,
			   GtkAllocation *allocation)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);

  GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);

  if (GTK_WIDGET_REALIZED (desktop) && desktop->pixmap)
    {
      gdk_pixmap_unref (desktop->pixmap);
      desktop->pixmap = gdk_pixmap_new (widget->window,
					widget->allocation.width,
					widget->allocation.height,
					-1);
      gtk_widget_queue_draw (widget);
    }
}

static void
gwm_desktop_unrealize (GtkWidget *widget)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);

  if (desktop->pixmap)
    {
      gdk_pixmap_unref (desktop->pixmap);
      desktop->pixmap = NULL;
    }
  gdk_pixmap_unref (desktop->bitmap);
  desktop->bitmap = NULL;

  GTK_WIDGET_CLASS (parent_class)->unrealize (widget);
}

static void
gwm_desktop_size_request (GtkWidget      *widget,
			  GtkRequisition *requisition)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  GwmDesktopClass *class = GWM_DESKTOP_GET_CLASS (desktop);
  GwmhDesk *desk = gwmh_desk_get_config ();
  gint xthick = widget->style->klass->xthickness;
  gint ythick = widget->style->klass->ythickness;
  gfloat ratio;
  gint area_width, area_height;

  ratio = ((gfloat) gdk_screen_width ()) / ((gfloat) gdk_screen_height ());

  /* frame */
  requisition->width = 2 * xthick;
  requisition->height = 2 * ythick;
  /* areas */
  if (class->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      area_height = class->area_size;
      area_width = ratio * class->area_size + 0.5;
    }
  else
    {
      area_width = class->area_size;
      area_height = class->area_size / ratio + 0.5;
    }
  requisition->width += area_width * desk->n_hareas;
  requisition->height += area_height * desk->n_vareas;
}

GtkWidget*
gwm_desktop_new (guint        index,
		 GtkTooltips *tooltips)
{
  GtkWidget *desktop;

  if (tooltips)
    g_return_val_if_fail (GTK_IS_TOOLTIPS (tooltips), NULL);

  desktop = gtk_widget_new (GWM_TYPE_DESKTOP, NULL);

  if (tooltips)
    {
      gtk_object_ref (GTK_OBJECT (tooltips));
      GWM_DESKTOP (desktop)->tooltips = tooltips;
    }

  gwm_desktop_set_index (GWM_DESKTOP (desktop), index);

  return desktop;
}

void
gwm_desktop_set_index (GwmDesktop *desktop,
		       guint       index)
{
  GtkWidget *widget = GTK_WIDGET (desktop);
  GwmhDesk *desk = gwmh_desk_get_config ();
  GList *node;

  g_return_if_fail (GWM_IS_DESKTOP (desktop));
  g_return_if_fail (index < desk->n_desktops);

  desktop->index = index;
  gwmh_desk_guess_desktop_area (desktop->index,
				&desktop->harea,
				&desktop->varea);
  desktop->last_desktop = desk->current_desktop;

  if (desktop->tooltips)
    gtk_tooltips_set_tip (desktop->tooltips,
			  widget,
			  desk->desktop_names[index],
			  NULL);
  
  
  g_list_free (desktop->task_list);
  desktop->task_list = NULL;
  desktop->grab_task = NULL;
  for (node = desk->client_list; node; node = node->next)
    {
      GwmhTask *task = node->data;
      
      if (task->desktop == desktop->index)
	desktop->task_list = g_list_prepend (desktop->task_list, task);
    }

  gtk_widget_queue_resize (widget);
}

static gboolean
gwm_desktop_desk_notifier (gpointer         func_data,
			   GwmhDesk        *desk,
			   GwmhDeskInfoMask change_mask)
{
  GwmDesktop *desktop = GWM_DESKTOP (func_data);
  GtkWidget *widget = GTK_WIDGET (desktop);
  
  if (desktop->last_desktop != desk->current_desktop)
    {
      if (desktop->last_desktop == desktop->index)
	gtk_widget_queue_draw (widget);
      desktop->last_desktop = desk->current_desktop;
      if (desktop->last_desktop == desktop->index)
	gtk_widget_queue_draw (widget);
    }

  if (change_mask & (GWMH_DESK_INFO_N_DESKTOPS |
		     GWMH_DESK_INFO_N_AREAS |
		     GWMH_DESK_INFO_CLIENT_LIST))
    gwm_desktop_set_index (desktop, MIN (desktop->index, desk->n_desktops - 1));
  else
    {
      if ((change_mask & GWMH_DESK_INFO_DESKTOP_NAMES) &&
	  desktop->tooltips)
	gtk_tooltips_set_tip (desktop->tooltips,
			      widget,
			      desk->desktop_names[desktop->index],
			      NULL);
      
      if ((change_mask & GWMH_DESK_INFO_CURRENT_AREA) &&
	  desk->current_desktop == desktop->index)
	{
	  desktop->harea = desk->current_harea;
	  desktop->varea = desk->current_varea;
	  gtk_widget_queue_draw (widget);
	}
    }

  return TRUE;
}

static gboolean
gwm_desktop_task_notifier (gpointer           func_data,
			   GwmhTask          *task,
			   GwmhTaskNotifyType ntype,
			   GwmhTaskInfoMask   imask)
{
  GwmDesktop *desktop = GWM_DESKTOP (func_data);
  GtkWidget *widget = GTK_WIDGET (desktop);

  if (ntype == GWMH_NOTIFY_NEW)
    {
      if (task->desktop == desktop->index)
	{
	  desktop->task_list = g_list_append (desktop->task_list, task);
	  desktop->grab_task = NULL;
	  gtk_widget_queue_draw (widget);
	}
    }
  else if (ntype == GWMH_NOTIFY_INFO_CHANGED && imask)
    {
      if (task->desktop == desktop->index)
	{
	  if (imask & GWMH_TASK_INFO_DESKTOP)
	    {
	      desktop->task_list = g_list_append (desktop->task_list, task);
	      desktop->grab_task = NULL;
	    }
	  gtk_widget_queue_draw (widget);
	}
      else if ((imask & GWMH_TASK_INFO_DESKTOP) &&
	       task->last_desktop == desktop->index)
	{
	  desktop->task_list = g_list_remove (desktop->task_list, task);
	  desktop->grab_task = NULL;
	  gtk_widget_queue_draw (widget);
	}
    }
  else if (ntype == GWMH_NOTIFY_DESTROY)
    {
      if (task->desktop == desktop->index)
	{
	  desktop->task_list = g_list_remove (desktop->task_list, task);
	  desktop->grab_task = NULL;
	  gtk_widget_queue_draw (widget);
	}
    }

  return TRUE;
}

static gboolean
gwm_desktop_button_press (GtkWidget      *widget,
			  GdkEventButton *event)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  GwmhDesk *desk = gwmh_desk_get_config ();
  gboolean handled = FALSE;

  desktop->grab_task = NULL;
  if (event->button == 1)
    {
      GList *node;
      gint xthick = widget->style->klass->xthickness;
      gint ythick = widget->style->klass->ythickness;
      gint width = widget->allocation.width;
      gint height = widget->allocation.height;
      gint x = event->x - xthick;
      gint y = event->y - xthick;

      for (node = g_list_last (desktop->task_list); node; node = node->prev)
	{
	  GwmhTask *task = node->data;
	  GrabArea *grab_area = gwmh_task_get_qdata (task, quark_grab_area);

	  if (grab_area &&
	      (event->x >= grab_area->x &&
	       event->x < grab_area->x + grab_area->width) &&
	      (event->y >= grab_area->y &&
	       event->y < grab_area->y + grab_area->height))
	    {
	      gwmh_task_show (task);
	      if (task->desktop == desk->current_desktop)
		{
		  desktop->grab_task = task;
		  desktop->x_comp = desktop->x_spixels * (event->x - grab_area->x);
		  desktop->y_comp = desktop->y_spixels * (event->y - grab_area->y);
		}
	      break;
	    }
	}

      if (x > xthick && x < width - 2 * xthick &&
	  y > ythick && y < height - 2 * ythick)
	{
	  x -= xthick;
	  y -= ythick;
	  width -= xthick * 2;
	  height -= ythick * 2;
	  x = x / (width / desk->n_hareas);
	  y = y / (height / desk->n_vareas);
	  gwmh_desk_set_current_area (desktop->index, x, y);
	}
#if 0  /* this is confusing */
      else
	gwmh_desk_set_current_desktop (desktop->index);
#endif
      
      handled = TRUE;
    }
  
  return handled;
}

static gboolean
gwmh_desktop_motion (GtkWidget      *widget,
		     GdkEventMotion *event)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  GwmhTask *task = desktop->grab_task;

  if (task && !GWMH_TASK_UPDATE_QUEUED (task))
    {
      gdk_window_move (task->gdkwindow,
		       (event->x - desktop->x_origin) *
		       desktop->x_spixels - desktop->x_comp +
		       task->win_x - task->frame_x,
		       (event->y - desktop->y_origin) *
		       desktop->y_spixels - desktop->y_comp +
		       task->win_y - task->frame_y);
      gdk_flush ();
    }

  return TRUE;
}

static gboolean
gwm_desktop_expose (GtkWidget      *widget,
		    GdkEventExpose *event)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);

  if (!desktop->pixmap ||
      GTK_WIDGET_REDRAW_PENDING (widget) ||
      GTK_WIDGET_RESIZE_NEEDED (widget))
    gtk_widget_queue_draw_area (widget,
				event->area.x, event->area.y,
				event->area.width, event->area.height);
  else
    gdk_draw_pixmap (widget->window,
		     widget->style->fg_gc[GTK_STATE_NORMAL],
		     desktop->pixmap,
		     0, 0, 0, 0,
		     -1, -1);

  return TRUE;
}

static void
gwm_desktop_draw (GtkWidget    *widget,
		  GdkRectangle *area)
{
  GwmDesktop *desktop = GWM_DESKTOP (widget);
  GwmhDesk *desk = gwmh_desk_get_config ();
  GdkWindow *window = desktop->pixmap ? desktop->pixmap : widget->window;
  GtkStyle *style = widget->style;
  guint index = desktop->index;
  gfloat swidth = gdk_screen_width ();
  gfloat sheight = gdk_screen_height ();
  gint xthick = widget->style->klass->xthickness;
  gint ythick = widget->style->klass->ythickness;
  gint width = widget->allocation.width;
  gint height = widget->allocation.height;
  gfloat area_width = ((gfloat) (width - 2 * xthick)) / ((gfloat) desk->n_hareas);
  gfloat area_height = ((gfloat) (height - 2 * ythick)) / ((gfloat) desk->n_vareas);
  gint x, y;
  gboolean current = desk->current_desktop == index;
  GList *node;

#if 0
  g_print ("REDRAW DESKTOP %d\n", index);
#endif
  
  desktop->x_spixels = swidth / area_width;
  desktop->y_spixels = sheight / area_height;

  gdk_flush ();

  /* draw border */
  gtk_draw_box (style,
		window,
		GTK_STATE_ACTIVE,
		current ? GTK_SHADOW_IN : GTK_SHADOW_OUT,
		0, 0,
		-1, -1);

  /* create gc for current area and draw its background */
  if (desk->n_hareas * desk->n_vareas > 1)
    {
      GdkGC *gc = (current ? style->bg_gc : style->base_gc)[GTK_STATE_SELECTED];
      
      gdk_gc_set_stipple (gc, desktop->bitmap);
      gdk_gc_set_clip_mask (gc, NULL);
      gdk_gc_set_fill (gc, GDK_STIPPLED);
      gdk_gc_set_ts_origin (gc, 0, 0);
      x = xthick + desktop->harea * area_width;
      y = ythick + desktop->varea * area_height;
      gdk_draw_rectangle (window, gc, TRUE,
			  x, y,
			  area_width,
			  area_height);
      gdk_gc_set_fill (gc, GDK_SOLID);
      desktop->x_origin = x - xthick / 2;
      desktop->y_origin = y - ythick / 2;
    }
  
  /* draw grid */
  for (x = 1; x < desk->n_hareas; x++)
    gtk_draw_vline (style, window,
		    GTK_WIDGET_STATE (widget),
		    ythick,
		    ythick + area_height * desk->n_vareas - ythick / 2,
		    xthick + x * area_width - xthick / 2);
  for (y = 1; y < desk->n_vareas; y++)
    gtk_draw_hline (style, window,
		    GTK_WIDGET_STATE (widget),
		    xthick,
		    xthick + area_width * desk->n_hareas - xthick / 2,
		    ythick + y * area_height - ythick / 2);

  /* draw tasks */
  desktop->task_list = gwmh_task_list_stack_sort (desktop->task_list);
  for (node = desktop->task_list; node; node = node->next)
    {
      GwmhTask *task = node->data;
      GrabArea *grab_area = g_new (GrabArea, 1);
      gint task_x, task_y;
      gint x2, y2;

      /* offset to area */
      x = xthick + task->harea * area_width;
      y = ythick + task->varea * area_height;
      x2 = x;
      y2 = y;
      /* offset within area */
      gwmh_task_get_frame_area_pos (task, &task_x, &task_y);
      x += task_x * area_width / swidth;
      y += task_y * area_height / sheight;
      x2 += (task_x + task->frame_width - 1) * area_width / swidth;
      y2 += (task_y + task->frame_height - 1) * area_height / sheight;
      grab_area->x = x;
      grab_area->y = y;
      grab_area->width = x2 - x + 1;
      grab_area->height = y2 - y + 1;
      gwmh_task_set_qdata_full (task, quark_grab_area, grab_area, g_free);

#if 0
      g_print ("draw task %p %d, %d, %d, %d\n",
	       task,
	       grab_area->x,
	       grab_area->y,
	       grab_area->width,
	       grab_area->height);
#endif

      gtk_draw_box (style, window,
		    GWMH_TASK_FOCUSED (task) ? GTK_STATE_PRELIGHT : GTK_STATE_NORMAL,
		    GWMH_TASK_FOCUSED (task) ? GTK_SHADOW_OUT : GTK_SHADOW_ETCHED_IN,
		    grab_area->x,
		    grab_area->y,
		    grab_area->width,
		    grab_area->height);
    }

  if (window == desktop->pixmap)
    gdk_draw_pixmap (widget->window,
		     widget->style->fg_gc[GTK_STATE_NORMAL],
		     desktop->pixmap,
		     0, 0, 0, 0,
		     -1, -1);
  gdk_flush ();
}
