/* Various menu functions */
#include <gtk/gtk.h>
#include <gnome.h>
#include <applet-widget.h>
#include "tasklist_applet.h"


extern Config config;

void menu_cb_position (GtkMenu *menu, gint *x, gint *y, gpointer user_data);

extern GtkWidget *area;
extern GtkWidget *applet;

/* Don't know if this works always */
TasklistTask *curtask;

void menu_popup (TasklistTask *temp_task, guint button, guint32 activate_time)
{
  GtkWidget *menu;
 
  menu = menu_get_popup_menu(temp_task);

  gtk_menu_popup(GTK_MENU(menu), NULL, NULL, 
		 menu_cb_position, temp_task,
		 button, activate_time);
}

/* Callback for stick/unstick */
gboolean menu_cb_stick_unstick (GtkWidget *widget, TasklistTask *temp_task)
{
  if (GWMH_TASK_STICKY(temp_task->task))
    gwmh_task_unset_gstate_flags (temp_task->task, WIN_STATE_STICKY);
  else
    gwmh_task_set_gstate_flags (temp_task->task, WIN_STATE_STICKY);
  return TRUE;
}

/* Callback for shade/unshade */
gboolean menu_cb_shade_unshade (GtkWidget *widget, TasklistTask *temp_task)
{
  if (GWMH_TASK_SHADED(temp_task->task))
    gwmh_task_unset_gstate_flags (temp_task->task, WIN_STATE_SHADED);
  else
    gwmh_task_set_gstate_flags (temp_task->task, WIN_STATE_SHADED);
  return TRUE;
}

/* Callback for to desktop */
gboolean menu_cb_to_desktop (GtkWidget *widget, int i)
{
  /* Don't know why I have to do this twice but it works */
  gwmh_task_set_desktop (curtask->task, i);
  gwmh_task_set_desktop (curtask->task, i);
  tasklist_layout();
  gtk_widget_draw(area, NULL);
  return TRUE;
}

/* Callback for restore/minimize */
gboolean menu_cb_restore_minimize(GtkWidget *widget, TasklistTask *temp_task)
{
  if (GWMH_TASK_MINIMIZED(temp_task->task))
      gwmh_task_show (temp_task->task);
  else
    gwmh_task_iconify (temp_task->task);
  return TRUE;
}

/* Callback for close */
gboolean menu_cb_close (GtkWidget *widget, TasklistTask *temp_task)
{
  gwmh_task_close (temp_task->task);
  return TRUE;
}

/* Callback for kill */
gboolean menu_cb_kill (GtkWidget *widget, TasklistTask *temp_task)
{
  GtkWidget *msg_box=NULL;
  int retval;

  if (!config.confirm_kill)
    {
      gwmh_task_kill(temp_task->task);
      return TRUE;
    }

  msg_box = gnome_message_box_new("Warning! Unsaved changes will be lost!\nProceed?",
				  GNOME_MESSAGE_BOX_QUESTION,
				  GNOME_STOCK_BUTTON_YES,
				  GNOME_STOCK_BUTTON_NO,
				  NULL);
  gtk_widget_show(msg_box);
  retval = gnome_dialog_run(GNOME_DIALOG(msg_box));

  if (retval)
    return TRUE;

  gwmh_task_kill(temp_task->task);

  return TRUE;
}

/* Create a popup menu */
GtkWidget *menu_get_popup_menu(TasklistTask *temp_task)
{
  gint i, curworkspace;
  GtkWidget *menu;
  gchar *wsname;
  GtkWidget *menuitem;
  GtkWidget *desktop;
  GwmhDesk *desk_info;

  curtask = temp_task;
  menu = gtk_menu_new();
  gtk_widget_show(menu);
  
  menuitem = gtk_menu_item_new_with_label(GWMH_TASK_ICONIFIED(temp_task->task)
					  ?"Restore":"Minimize");
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate", 
		     GTK_SIGNAL_FUNC(menu_cb_restore_minimize), temp_task);
  gtk_widget_show(menuitem);
  gtk_menu_append (GTK_MENU(menu), menuitem);

  menuitem = gtk_menu_item_new_with_label(GWMH_TASK_SHADED(temp_task->task)
					  ?"Unshade":"Shade");
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
		     GTK_SIGNAL_FUNC(menu_cb_shade_unshade), temp_task);
  gtk_widget_show(menuitem);
  gtk_menu_append(GTK_MENU(menu), menuitem);
  menuitem = gtk_menu_item_new_with_label(GWMH_TASK_STICKY(temp_task->task)
					  ?"Unstick":"Stick");
  gtk_signal_connect (GTK_OBJECT(menuitem), "activate",
		      GTK_SIGNAL_FUNC(menu_cb_stick_unstick), temp_task);
  gtk_widget_show(menuitem);
  gtk_menu_append(GTK_MENU(menu), menuitem);

  menuitem = gtk_menu_item_new_with_label("To Desktop");
  gtk_widget_show(menuitem);
  gtk_menu_append (GTK_MENU(menu), menuitem);
  
  if (!GWMH_TASK_STICKY(temp_task->task))
    {
      desktop = gtk_menu_new();
      gtk_widget_show(desktop);
      gtk_menu_item_set_submenu(GTK_MENU_ITEM (menuitem), desktop);

      desk_info = gwmh_desk_get_config ();

      curworkspace = desk_info->current_desktop;

      for (i=0;i<(desk_info->n_desktops);i++)
	{
	  wsname = g_strdup(desk_info->desktop_names[i]);
	  menuitem = gtk_menu_item_new_with_label(wsname);
	  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
			     GTK_SIGNAL_FUNC (menu_cb_to_desktop), i);
	  if (i==curworkspace)
	    gtk_widget_set_sensitive(menuitem, FALSE);
	  gtk_widget_show(menuitem);
	  gtk_menu_append(GTK_MENU(desktop), menuitem);
	  g_free(wsname);
	}
    }
  else
      gtk_widget_set_sensitive(menuitem, FALSE);
    
  menuitem = gtk_menu_item_new_with_label("Close");
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
		     GTK_SIGNAL_FUNC(menu_cb_close), temp_task);
  gtk_widget_show(menuitem);
  gtk_menu_append (GTK_MENU(menu), menuitem);
  menuitem = gtk_menu_item_new();
  gtk_widget_show(menuitem);
  gtk_menu_append (GTK_MENU(menu), menuitem);
 
  menuitem = gtk_menu_item_new_with_label("Kill");
  gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
		     GTK_SIGNAL_FUNC(menu_cb_kill), temp_task);

  gtk_widget_show(menuitem);
  gtk_menu_append (GTK_MENU(menu), menuitem);

  gtk_widget_show (menu);

  return menu;
}

void menu_cb_position (GtkMenu *menu, gint *x, gint *y, gpointer user_data)
{
  GtkRequisition mreq;
  gint wx, wy;

  TasklistTask *temp_task;
  temp_task = (TasklistTask *)user_data;
  gtk_widget_get_child_requisition(GTK_WIDGET(menu), &mreq);
  gdk_window_get_origin(area->window, &wx, &wy);

  *x = wx+temp_task->x;
  *y = wy-mreq.height+temp_task->y;
}



