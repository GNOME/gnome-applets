/* GNOME Tasklist Applet
 * Copyright (C) Anders Carlsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "tasklist_applet.h"

Config config;
GtkWidget *applet;
GtkWidget *area;
GList *tasklist_tasks;

/* Prototypes */
gboolean task_is_visible (TasklistTask *temp_task);
gint tasklist_get_num_rows (PanelSizeType o);
static void tasklist_cb_change_size (GtkWidget *widget,
				     PanelSizeType o, gpointer data);
TasklistTask *task_get_xy (gint x, gint y);
void task_draw (TasklistTask *temp_task);
gchar *task_get_label (TasklistTask *temp_task);
void tasklist_layout (void);
gboolean tasklist_cb_button_press (GtkWidget *widget,
				   GdkEventButton *event);
gboolean tasklist_cb_expose (GtkWidget *widget,
			     GdkEventExpose *event);
gboolean tasklist_cb_button_release (GtkWidget *widget,
				     GdkEventButton *event);
void tasklist_create_applet (void);
TasklistTask *tasklist_find_tasklist_task (GwmhTask *task);
GList *tasklist_get_visible_tasks (void);

TasklistTask *task_get_xy (gint x, gint y)
{
  GList *temp_tasks;
  TasklistTask *temp_task;

  temp_tasks = tasklist_get_visible_tasks ();

  while (temp_tasks)
    {
      temp_task = (TasklistTask *)temp_tasks->data;
      if (x > temp_task->x &&
	  x < temp_task->x + temp_task->width &&
	  y > temp_task->y &&
	  y < temp_task->y + temp_task->height)
	{
	  g_list_free (temp_tasks);
	  return temp_task;
	}

      temp_tasks = temp_tasks->next;
    }

  g_list_free (temp_tasks);
  return NULL;
}

gint tasklist_get_num_rows (PanelSizeType o)
{
  /* FIXME: Should look at applet configuration aswell */
  switch (o)
    {
    case SIZE_TINY:
      return 1;
    case SIZE_STANDARD:
      return 2;
    case SIZE_LARGE:
      return 3;
    case SIZE_HUGE:
      return 4;
    }

  /* Standard value */
  return 2;
}

static void tasklist_cb_change_size (GtkWidget *widget,
				     PanelSizeType o, gpointer data)
{

  gtk_drawing_area_size (GTK_DRAWING_AREA (area), CONFIG_WIDTH, 
			 CONFIG_ROWHEIGHT * tasklist_get_num_rows (o));
  tasklist_layout();
}

gboolean task_is_visible (TasklistTask *temp_task)
{
  GwmhDesk *desk_info;

  desk_info = gwmh_desk_get_config();

  if (GWMH_TASK_SKIP_TASKBAR (temp_task->task))
      return FALSE;
  
  if ((!config.all_tasks) && (!config.minimized_tasks))
    {  
      if (temp_task->task->desktop != desk_info->current_desktop)
	return FALSE;
    }

  if (config.minimized_tasks)
    {
      if (!(GWMH_TASK_ICONIFIED (temp_task->task)))
	 if (temp_task->task->desktop != desk_info->current_desktop)
	   return FALSE;
    }

  /* Start check whether to show normal/iconified windows */
  if (config.show_normal & (GWMH_TASK_ICONIFIED (temp_task->task)))
    return FALSE;

  if (config.show_minimized & (!(GWMH_TASK_ICONIFIED (temp_task->task))))
    return FALSE;

  return TRUE;
}

GList *tasklist_get_visible_tasks (void)
{
  GList *temp_tasks;
  GList *visible_tasks=NULL;

  temp_tasks = tasklist_tasks;
  while (temp_tasks)
    {
      if (task_is_visible((TasklistTask *)temp_tasks->data))
	{
	  visible_tasks = g_list_append (visible_tasks, temp_tasks->data);
	}
      temp_tasks = temp_tasks->next;
    }
  return visible_tasks;

}

gchar *task_get_label (TasklistTask *temp_task)
{
  gchar *str;
  gint len, label_len;

  label_len = gdk_string_width (area->style->font, temp_task->task->name);

  if (label_len>temp_task->width-CONFIG_PIXMAP-2)
    {
      len = strlen(temp_task->task->name);
      len--;
      str = g_malloc(len+4);
      strcpy (str, temp_task->task->name);
      strcat (str, "..");
      for (;len>0;len--)
	{
	  str[len]='.';
	  str[len+3]='\0';
	  
	  label_len = gdk_string_width (area->style->font, str);
	  
	  /* FIXME: pixmap */
	  if (label_len<=temp_task->width-CONFIG_PIXMAP-4)
	    break;
	}
    }
  else
    str = g_strdup(temp_task->task->name);
  
  return str;
}

void task_draw (TasklistTask *temp_task)
{
  /*  GdkGC *temp_gc;*/
  gchar *tempstr;
  gint text_height, text_width;

  if (!task_is_visible (temp_task))
      return;

  gtk_paint_box (area->style, area->window,
		 GTK_STATE_NORMAL,
		 GWMH_TASK_FOCUSED(temp_task->task) ?
				   GTK_SHADOW_IN : GTK_SHADOW_OUT,
		 NULL, area, "button",
		 temp_task->x, temp_task->y,
		 temp_task->width, temp_task->height);

  if (temp_task->task->name)
    {
      text_height = gdk_string_height(area->style->font, "1");

      tempstr = task_get_label(temp_task);
      text_width = gdk_string_width(area->style->font, tempstr);

      gdk_draw_string (area->window,
		       area->style->font, area->style->black_gc,
		       temp_task->x+CONFIG_PIXMAP/2+((temp_task->width-(text_width))/2),
		       temp_task->y+((temp_task->height-text_height)/2)+text_height, 
		       tempstr);
   
      g_free(tempstr);
    }
}

void tasklist_layout (void)
{
  int j, k, num, p;
  GList *temp_tasks;

  /* Needed for g_free () (I think) */
  GList *temp_tasks_two;

  TasklistTask *temp_task;
  int config_rows;
  int num_rows, num_cols;
  int curx, cury, curwidth, curheight;

  p = 0;
  j = 0;
  k = 0;
  num_rows = 0;
  num_cols = 0;

  config_rows = tasklist_get_num_rows (applet_widget_get_panel_size (APPLET_WIDGET (applet)));

  temp_tasks = tasklist_get_visible_tasks ();
  num = g_list_length (temp_tasks);
  temp_tasks_two = temp_tasks;

  if (num == 0)
    {
      gtk_widget_draw (area, NULL);
      return;
    }

  while (p < num)
    {
      if (num < config_rows)
	num_rows = num;
      
      j++;
      if (num_cols < j)
	num_cols = j;
      if (num_rows < k + 1)
	num_rows = k + 1;
      
      if (j >= ((num + config_rows - 1) / config_rows))
	{
	  j = 0;
	  k++;
	}
      p++;
    }

  curheight = (CONFIG_ROWHEIGHT * config_rows-4) / num_rows;
  curwidth = (CONFIG_WIDTH-4) / num_cols;

  curx = 2;
  cury = 2;

  while (temp_tasks)
    {
      temp_task = (TasklistTask *)temp_tasks->data;

      temp_task->x = curx;
      temp_task->y = cury;
      temp_task->width = curwidth;
      temp_task->height = curheight;

      curx += curwidth;
      if (curx >= CONFIG_WIDTH || curx + curwidth > CONFIG_WIDTH)
	{
	  cury += curheight;
	  curx = 2;
	}
      temp_tasks = temp_tasks->next;
    }
  g_list_free (temp_tasks_two);
  gtk_widget_draw (area, NULL);
}

gboolean tasklist_cb_button_press (GtkWidget *widget, GdkEventButton *event)
{
  TasklistTask *temp_task;

  temp_task = task_get_xy ((gint)event->x, (gint)event->y);
  if (!temp_task)
    return FALSE;

  if (event->button == 2)
    return FALSE;

  if (!config.show_winops)
    return FALSE;

  if (event->button == 3)
    {
      gtk_signal_emit_stop_by_name (GTK_OBJECT (widget),
				    "button_press_event");

      menu_popup(temp_task, event->button, event->time);
      return TRUE;
    }

  return FALSE;
}

gboolean tasklist_cb_button_release (GtkWidget *widget, GdkEventButton *event)
{
  TasklistTask *temp_task;

  temp_task = task_get_xy ((gint)event->x, (gint)event->y);

  if (!temp_task)
    return FALSE;

  if (event->button == 1)
    {
      if (GWMH_TASK_ICONIFIED (temp_task->task))
	  gwmh_task_show (temp_task->task);
      gwmh_task_show (temp_task->task);
    }

    return FALSE;
}

gboolean tasklist_cb_expose (GtkWidget *widget, GdkEventExpose *event)
{
  GList *temp_tasks;
  TasklistTask *temp_task;

  gtk_paint_box (area->style, area->window,
		 GTK_STATE_NORMAL, GTK_SHADOW_IN,
		 NULL, area, "button",
		 0, 0,
		 area->allocation.width,
		 area->allocation.height);

  temp_tasks = tasklist_get_visible_tasks();
  
  while (temp_tasks)
    {
      temp_task = (TasklistTask *)temp_tasks->data;
      task_draw (temp_task);
      temp_tasks = temp_tasks->next;
    }

  g_list_free (temp_tasks);

  return FALSE;
}

void tasklist_create_applet(void)
{
  GtkWidget *hbox;

  applet = applet_widget_new("tasklist_applet");

  config_load (APPLET_WIDGET (applet)->privcfgpath);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (hbox);
  applet_widget_add (APPLET_WIDGET (applet), hbox);

  area = gtk_drawing_area_new();
  gtk_widget_show (area);
  gtk_box_pack_start_defaults (GTK_BOX (hbox), area);

  gtk_drawing_area_size (GTK_DRAWING_AREA (area), CONFIG_WIDTH, 
			 CONFIG_ROWHEIGHT * tasklist_get_num_rows (applet_widget_get_panel_size(APPLET_WIDGET (applet))));
  gtk_widget_set_events (area, GDK_EXPOSURE_MASK | GDK_BUTTON_RELEASE_MASK |
			 GDK_BUTTON_PRESS_MASK);
  gtk_signal_connect (GTK_OBJECT (area), "expose_event",
		      GTK_SIGNAL_FUNC (tasklist_cb_expose), NULL);
  gtk_signal_connect (GTK_OBJECT (area), "button_press_event",
		      GTK_SIGNAL_FUNC (tasklist_cb_button_press), NULL);
  gtk_signal_connect (GTK_OBJECT (area), "button_release_event",
		      GTK_SIGNAL_FUNC (tasklist_cb_button_release), NULL);

  gtk_widget_show (applet);
}

TasklistTask *tasklist_find_tasklist_task(GwmhTask *task)
{
  GList *temp_tasks;
  TasklistTask *temp_task;

  temp_tasks = tasklist_tasks;

  while (temp_tasks)
    {
      temp_task = (TasklistTask *)temp_tasks->data;
      if (temp_task->task == task)
	return temp_task;
      temp_tasks = temp_tasks->next;
    }
  return NULL;
}

static gboolean tasklist_task_notifier (gpointer func_data, GwmhTask *task,
					GwmhTaskNotifyType ntype,
					GwmhTaskInfoMask imask)
{
  TasklistTask *temp_task;
  
  if (ntype == GWMH_NOTIFY_INFO_CHANGED)
    {
      if (imask & GWMH_TASK_INFO_FOCUSED)
	task_draw (tasklist_find_tasklist_task (task));
      if (imask & GWMH_TASK_INFO_MISC)
	task_draw (tasklist_find_tasklist_task (task));
    }

  if (ntype == GWMH_NOTIFY_NEW)
    {
      temp_task = g_malloc(sizeof(TasklistTask));
      temp_task->task = task;
      tasklist_tasks = g_list_append(tasklist_tasks, temp_task);
      tasklist_layout();
    }
  
  if (ntype == GWMH_NOTIFY_DESTROY)
    {
      tasklist_tasks = g_list_remove(tasklist_tasks, tasklist_find_tasklist_task(task));
      tasklist_layout();
    }

  return TRUE;
}

static void tasklist_cb_properties (void)
{
  properties_show();
}

static void tasklist_cb_about (void)
{
  GtkWidget *dialog;

  const char *authors[] =
  {
    "Anders Carlsson (anders.carlsson@tordata.se)",
    NULL
  };

  dialog = gnome_about_new ("Gnome Tasklist",
			    "0.1",
			    "Copyright (C) 1999 Anders Carlsson",
			    authors,
			    "A tasklist for the GNOME desktop environment",
			    NULL);
  gtk_widget_show (dialog);
  gdk_window_raise (dialog->window);

}

static gboolean tasklist_desk_notifier (gpointer func_data, GwmhDesk *desk,
					GwmhDeskInfoMask change_mask)
{
  tasklist_layout();

  return TRUE;
}

static gboolean tasklist_save_session (gpointer func_data, 
				       const gchar *privcfgpath,
				       const gchar *globcfgpath)
{
  config_save (privcfgpath);

  return FALSE;
}

gint main (gint argc, gchar *argv[])
{
  /* FIXME: i18n and config.h*/
  applet_widget_init ("tasklist_applet",
		      "1.0",
		      argc, argv,
		      NULL, 0, NULL);
  
  /* FIXME: check for gnome wm */
  gwmh_init();

  gwmh_desk_notifier_add (tasklist_desk_notifier, NULL);
  gwmh_task_notifier_add (tasklist_task_notifier, NULL);

  tasklist_create_applet();

  gtk_signal_connect (GTK_OBJECT (applet), "save-session", 
		      (AppletCallbackFunc)tasklist_save_session, NULL);
  gtk_signal_connect (GTK_OBJECT (applet), "change_size", 
		      tasklist_cb_change_size, NULL);

  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 "About...",
					 (AppletCallbackFunc) tasklist_cb_about,
					 NULL);

  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
					 "properties",
					 GNOME_STOCK_MENU_PROP,
					 "Properties...",
					 (AppletCallbackFunc) tasklist_cb_properties,
					 NULL);

  applet_widget_gtk_main();
  return 0;
}

#include "gwmh.c"
#include "gstc.c"


