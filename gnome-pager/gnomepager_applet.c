#include <gnomepager.h>

extern gint         gdk_error_warnings;
GtkWidget          *applet = NULL;
GtkWidget          *popbox = NULL;
GList              *tasks = NULL;
PanelOrientType     applet_orient = ORIENT_DOWN;
gint                current_desk = 0;
gint                num_desk = 0;
gchar              *desk_name[32];
GtkWidget          *desk_widget[32];
GList              *task_widgets = NULL;
GList              *task_dest = NULL;
GtkWidget          *hold_box = NULL;
GtkWidget          *main_box = NULL;
GtkWidget          *task_table = NULL;
GdkPixmap          *p_1 = NULL, *p_2 = NULL, *p_3 = NULL;
GdkPixmap          *m_1 = NULL, *m_2 = NULL, *m_3 = NULL;
gint                pager_rows = 2;
gchar               pager_size = 1;
gchar               tasks_all = 0;
gint                task_rows_h = 2;
gint                task_rows_v = 1;
gint                max_task_width = 400;
gchar               show_tasks = 1;
gchar               show_pager = 1;
gchar               show_icons = 1;

#define PAGER_W_0 31
#define PAGER_H_0 22
#define PAGER_W_1 62
#define PAGER_H_1 44

/* APPLET callbacks */
void 
cb_applet_orient_change(GtkWidget *w, PanelOrientType o, gpointer data)
{
  gint i;
  
  if (o == applet_orient) 
    return;
  applet_orient = o;
  switch (o) 
    {
     case ORIENT_UP:
     case ORIENT_DOWN:
      for (i = 0; i < 32; i++)
	desk_widget[i] = NULL;
      if (main_box)
	gtk_widget_destroy(main_box);
      main_box = NULL;
      init_applet_gui_horiz();
      break;
     case ORIENT_LEFT:
     case ORIENT_RIGHT:
      for (i = 0; i < 32; i++)
	desk_widget[i] = NULL;
      if (main_box)
	gtk_widget_destroy(main_box);
      main_box = NULL;
      init_applet_gui_vert();
      break;
    }
  w = NULL;
  data = NULL;
}


void 
cb_applet_about(AppletWidget * widget, gpointer data)
{
  GtkWidget          *about;
  const gchar        *authors[] =
    {"The Rasterman", NULL};
  
  about = gnome_about_new
    (_("Desktop Pager Applet"), "0.1", _("Copyright (C)1998 Red Hat Software"),
     authors,
     _("Pager for a GNOME compliant Window Manager"),
     NULL);
  gtk_widget_show(about);
  data = NULL;
  widget = NULL;
}

void 
cb_check_show_icons(GtkWidget *widget, gpointer data)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(widget), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  if (GTK_TOGGLE_BUTTON(widget)->active)
    show_icons = 1;
  else
    show_icons = 0;
}

void 
cb_check_pager_size(GtkWidget *widget, gpointer data)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(widget), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  if (GTK_TOGGLE_BUTTON(widget)->active)
    pager_size = 1;
  else
    pager_size = 0;
}

void 
cb_check_all_tasks(GtkWidget *widget, gpointer data)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(widget), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  if (GTK_TOGGLE_BUTTON(widget)->active)
    tasks_all = 1;
  else
    tasks_all = 0;
}

void 
cb_check_show_tasks(GtkWidget *widget, gpointer data)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(widget), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  if (GTK_TOGGLE_BUTTON(widget)->active)
    show_tasks = 1;
  else
    show_tasks = 0;
}

void 
cb_check_show_pager(GtkWidget *widget, gpointer data)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(widget), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  if (GTK_TOGGLE_BUTTON(widget)->active)
    show_pager = 1;
  else
    show_pager = 0;
}

void 
cb_adj_max_width(GtkAdjustment *adj, GtkAdjustment *adj1)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(adj), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  max_task_width = (gint)(adj->value);
}

void 
cb_adj_rows_h(GtkAdjustment *adj, GtkAdjustment *adj1)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(adj), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  task_rows_h = (gint)(adj->value);
}

void 
cb_adj_rows_v(GtkAdjustment *adj, GtkAdjustment *adj1)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(adj), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  task_rows_v = (gint)(adj->value);
}

void 
cb_adj_rows(GtkAdjustment *adj, GtkAdjustment *adj1)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(adj), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  pager_rows = (gint)(adj->value);
}

void
cb_prop_cancel(GtkWidget *widget, gpointer data)
{
  widget = NULL;
  data = NULL;
}

void
cb_prop_apply(GtkWidget *widget, gpointer data)
{
  gint i;
  
  switch (applet_orient) 
    {
     case ORIENT_UP:
     case ORIENT_DOWN:
      for (i = 0; i < 32; i++)
	desk_widget[i] = NULL;
      if (main_box)
	gtk_widget_destroy(main_box);
      main_box = NULL;
      init_applet_gui_horiz();
      break;
     case ORIENT_LEFT:
     case ORIENT_RIGHT:
      for (i = 0; i < 32; i++)
	desk_widget[i] = NULL;
      if (main_box)
	gtk_widget_destroy(main_box);
      main_box = NULL;
      init_applet_gui_vert();
      break;
    }
  gnome_config_set_int("gnome_pager/stuff/pager_rows", pager_rows);
  gnome_config_set_int("gnome_pager/stuff/pager_size", pager_size);
  gnome_config_set_int("gnome_pager/stuff/tasks_all", tasks_all);
  gnome_config_set_int("gnome_pager/stuff/task_rows_h", task_rows_h);
  gnome_config_set_int("gnome_pager/stuff/task_rows_v", task_rows_v);
  gnome_config_set_int("gnome_pager/stuff/max_task_width", max_task_width);
  gnome_config_set_int("gnome_pager/stuff/show_tasks", show_tasks);
  gnome_config_set_int("gnome_pager/stuff/show_pager", show_pager);
  gnome_config_set_int("gnome_pager/stuff/show_icons", show_icons);
  gnome_config_sync();
  widget = NULL;
  data = NULL;
}

void 
cb_applet_properties(AppletWidget * widget, gpointer data)
{
  GtkWidget *prop = NULL;
  GtkWidget *table, *label, *spin, *check;
  GtkAdjustment *adj;
  
  if (!prop) 
    {
      prop = gnome_property_box_new();
      gtk_signal_connect (GTK_OBJECT(prop), "delete_event",
			  GTK_SIGNAL_FUNC(cb_prop_cancel), NULL);
      gtk_signal_connect (GTK_OBJECT(prop), "apply",
			  GTK_SIGNAL_FUNC(cb_prop_apply), NULL);
      gtk_window_set_title(GTK_WINDOW(&GNOME_PROPERTY_BOX(prop)->dialog.window),
			   _("Gnome Pager Settings"));
      table = gtk_table_new(1, 1, FALSE);
      gtk_widget_show(table);
      gnome_property_box_append_page(GNOME_PROPERTY_BOX(prop), table,
				     gtk_label_new (_("Display")));
      check = gtk_check_button_new_with_label(_("Show all tasks on all desktops"));
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(check), tasks_all);
      gtk_signal_connect(GTK_OBJECT(check), "clicked",
			 GTK_SIGNAL_FUNC(cb_check_all_tasks), NULL);
      gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
      gtk_widget_show(check);
      gtk_table_attach(GTK_TABLE(table), check, 
		       2, 4, 0, 1, GTK_FILL|GTK_EXPAND,0,0,0);
      check = gtk_check_button_new_with_label(_("Show tasks"));
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(check), show_tasks);
      gtk_signal_connect(GTK_OBJECT(check), "clicked",
			 GTK_SIGNAL_FUNC(cb_check_show_tasks), NULL);
      gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
      gtk_widget_show(check);
      gtk_table_attach(GTK_TABLE(table), check, 
		       2, 4, 1, 2, GTK_FILL|GTK_EXPAND,0,0,0);
      check = gtk_check_button_new_with_label(_("Show pager"));
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(check), show_pager);
      gtk_signal_connect(GTK_OBJECT(check), "clicked",
			 GTK_SIGNAL_FUNC(cb_check_show_pager), NULL);
      gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
      gtk_widget_show(check);
      gtk_table_attach(GTK_TABLE(table), check, 
		       2, 4, 2, 3, GTK_FILL|GTK_EXPAND,0,0,0);
      check = gtk_check_button_new_with_label(_("Use small pagers"));
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(check), pager_size);
      gtk_signal_connect(GTK_OBJECT(check), "clicked",
			 GTK_SIGNAL_FUNC(cb_check_pager_size), NULL);
      gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
      gtk_widget_show(check);
      gtk_table_attach(GTK_TABLE(table), check, 
		       2, 4, 3, 4, GTK_FILL|GTK_EXPAND,0,0,0);
      check = gtk_check_button_new_with_label(_("Show icons in tasks"));
      gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(check), show_icons);
      gtk_signal_connect(GTK_OBJECT(check), "clicked",
			 GTK_SIGNAL_FUNC(cb_check_show_icons), NULL);
      gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
      gtk_widget_show(check);
      gtk_table_attach(GTK_TABLE(table), check, 
		       2, 4, 4, 5, GTK_FILL|GTK_EXPAND,0,0,0);
      adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)max_task_width, 20, 
						(gfloat)gdk_screen_width(), 
						16, 16, 16 );
      gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			 GTK_SIGNAL_FUNC(cb_adj_max_width), adj);
      gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);
      label = gtk_label_new(_("Maximum width of horizontal task list"));
      gtk_widget_show(label);
      spin = gtk_spin_button_new(adj, 1, 0);
      gtk_widget_show(spin);
      gtk_table_attach(GTK_TABLE(table), label, 
		       0, 1, 0, 1, GTK_FILL|GTK_EXPAND,0,0,0);
      gtk_table_attach(GTK_TABLE(table), spin, 
		       1, 2, 0, 1, GTK_FILL|GTK_EXPAND,0,0,0);
      adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)task_rows_h, 1, 
						8, 1, 1, 1 );
      gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			 GTK_SIGNAL_FUNC(cb_adj_rows_h), adj);
      gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);
      label = gtk_label_new(_("Number of rows of horizontal tasks"));
      gtk_widget_show(label);
      spin = gtk_spin_button_new(adj, 1, 0);
      gtk_widget_show(spin);
      gtk_table_attach(GTK_TABLE(table), label, 
		       0, 1, 1, 2, GTK_FILL|GTK_EXPAND,0,0,0);
      gtk_table_attach(GTK_TABLE(table), spin, 
		       1, 2, 1, 2, GTK_FILL|GTK_EXPAND,0,0,0);
      adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)task_rows_v, 1, 
						4, 1, 1, 1 );
      gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			 GTK_SIGNAL_FUNC(cb_adj_rows_v), adj);
      gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);
      label = gtk_label_new(_("Number of vertical columns of tasks"));
      gtk_widget_show(label);
      spin = gtk_spin_button_new(adj, 1, 0);
      gtk_widget_show(spin);
      gtk_table_attach(GTK_TABLE(table), label, 
		       0, 1, 2, 3, GTK_FILL|GTK_EXPAND,0,0,0);
      gtk_table_attach(GTK_TABLE(table), spin, 
		       1, 2, 2, 3, GTK_FILL|GTK_EXPAND,0,0,0);
      adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)pager_rows, 1, 
						8, 1, 1, 1 );
      gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
			 GTK_SIGNAL_FUNC(cb_adj_rows), adj);
      gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);
      label = gtk_label_new(_("Number rows of pagers"));
      gtk_widget_show(label);
      spin = gtk_spin_button_new(adj, 1, 0);
      gtk_widget_show(spin);
      gtk_table_attach(GTK_TABLE(table), label, 
		       0, 1, 3, 4, GTK_FILL|GTK_EXPAND,0,0,0);
      gtk_table_attach(GTK_TABLE(table), spin, 
		       1, 2, 3, 4, GTK_FILL|GTK_EXPAND,0,0,0);
      
    }
  if (prop)
    gtk_widget_show(prop);
  data = NULL;
  widget = NULL;
}



/* UTILITY functions */

void *
util_get_atom(Window win, gchar *atom, Atom type, gint *size)
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

gchar              *
util_reduce_chars(gchar * s, int num)
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



/* CLIENT WINDOW manipulation functions */

void
client_win_kill(Task *t)
{
  XKillClient(GDK_DISPLAY(), (XID)t->win);
}

void
client_win_close(Task *t)
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
}


void 
client_win_iconify(Task *t)
{
  XIconifyWindow(GDK_DISPLAY(), t->win, DefaultScreen(GDK_DISPLAY()));
  XSync(GDK_DISPLAY(), False);
}

void 
client_win_show(Task *t)
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
  if ((!t->sticky) && (t->desktop != current_desk - 1))
    gnome_win_hints_set_current_workspace(t->desktop);
  XSync(GDK_DISPLAY(), False);
}

void 
client_win_stick(Task *t)
{
  XEvent              xev;
  Atom                a;

  a = XInternAtom(GDK_DISPLAY(), XA_WIN_STATE, False);
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = t->win;
  xev.xclient.message_type = a;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = WIN_STATE_STICKY;
  xev.xclient.data.l[1] = WIN_STATE_STICKY;
  xev.xclient.data.l[2] = CurrentTime;
  
  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False, 
	     SubstructureNotifyMask, (XEvent *) & xev);
  XSync(GDK_DISPLAY(), False);
}

void 
client_win_unstick(Task *t)
{
  XEvent              xev;
  Atom                a;

  a = XInternAtom(GDK_DISPLAY(), XA_WIN_STATE, False);
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = t->win;
  xev.xclient.message_type = a;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = WIN_STATE_STICKY;
  xev.xclient.data.l[1] = 0;
  xev.xclient.data.l[2] = CurrentTime;
  
  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False, 
	     SubstructureNotifyMask, (XEvent *) & xev);
  XSync(GDK_DISPLAY(), False);
}

void 
client_win_shade(Task *t)
{
  XEvent              xev;
  Atom                a;

  a = XInternAtom(GDK_DISPLAY(), XA_WIN_STATE, False);
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = t->win;
  xev.xclient.message_type = a;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = WIN_STATE_SHADED;
  xev.xclient.data.l[1] = WIN_STATE_SHADED;
  xev.xclient.data.l[2] = CurrentTime;
  
  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False, 
	     SubstructureNotifyMask, (XEvent *) & xev);
  XSync(GDK_DISPLAY(), False);
}

void 
client_win_unshade(Task *t)
{
  XEvent              xev;
  Atom                a;

  a = XInternAtom(GDK_DISPLAY(), XA_WIN_STATE, False);
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = t->win;
  xev.xclient.message_type = a;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = WIN_STATE_SHADED;
  xev.xclient.data.l[1] = 0;
  xev.xclient.data.l[2] = CurrentTime;
  
  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False, 
	     SubstructureNotifyMask, (XEvent *) & xev);
  XSync(GDK_DISPLAY(), False);
}


/* CUSTOM WIDGET STUFF */

void 
custom_popbox_show(GtkWidget * widget)
{
  gint x, y;
  gint screen_width;
  gint screen_height;
  GdkCursor *cursor;
  
  if (GTK_WIDGET_VISIBLE(widget))
    {
      gdk_pointer_ungrab(GDK_CURRENT_TIME);
      gtk_widget_hide(widget);
      gtk_grab_remove(widget);
    }
  else
    {
      gdk_window_get_pointer(NULL, &x, &y, NULL);
      gtk_widget_size_request(widget, &widget->requisition);
      screen_width = gdk_screen_width();
      screen_height = gdk_screen_height();
      
      if ((applet_orient == ORIENT_UP) || (applet_orient == ORIENT_DOWN))
	{
	  x -= (widget->requisition.width / 2);
	  if (y > screen_height / 2)
	    y -= (widget->requisition.height + 2);
	  else
	    y += 2;
	}
      else
	{
	  y -= (widget->requisition.height / 2);
	  if (x > screen_width / 2)
	    x -= (widget->requisition.width + 2);
	  else
	    x += 2;
	}
      
      if ((x + widget->requisition.width) > screen_width)
	x -= ((x + widget->requisition.width) - screen_width);
      if (x < 0)
	x = 0;
      if ((y + widget->requisition.height) > screen_height)
	y -= ((y + widget->requisition.height) - screen_height);
      if (y < 0)
	y = 0;
      
      gtk_widget_set_uposition(widget, x, y);
      gtk_widget_show(widget);
      while(gtk_events_pending())
	gtk_main_iteration();
      cursor = gdk_cursor_new(GDK_ARROW);
      gdk_pointer_grab (widget->window, TRUE,
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
			GDK_POINTER_MOTION_MASK,
			NULL, cursor, GDK_CURRENT_TIME);
      gtk_grab_add(widget);
      gdk_cursor_destroy(cursor);
    }
  widget = NULL;
}

/* PROPERTY CHANGE info gathering callbacks */
void
cb_task_change(GtkWidget *widget, GdkEventProperty * ev, Task *t)
{
  gint i, tdesk;
  gchar tsticky;
  
  tsticky = t->sticky;
  tdesk = t->desktop;
  task_get_info(t);

  if (tsticky)
    {
      for (i = 0; i < num_desk; i++)
	desktop_draw(i);
    }
  else
    {
      if (t->sticky)
	{
	  for (i = 0; i < num_desk; i++)
	    desktop_draw(i);
	}
      else if (tdesk != t->desktop)
	{
	  desktop_draw(t->desktop);
	  desktop_draw(tdesk);
	}
      else
	desktop_draw(t->desktop);
    }
  set_task_info_to_button(t);
  widget = NULL;
}

void 
cb_root_prop_change(GtkWidget * widget, GdkEventProperty * ev)
{
  gint                desk, pdesk, i;
  GdkAtom             at;

  at = gdk_atom_intern(XA_WIN_WORKSPACE, FALSE);
  if ((ev->atom == at) && (ev->state == GDK_PROPERTY_NEW_VALUE))
    {
      desk = gnome_win_hints_get_current_workspace();

      if (desk == current_desk)
	return;
      pdesk = current_desk;
      current_desk = desk;
      desktop_draw(pdesk);
      desktop_draw(current_desk);
      populate_tasks();
      return;
    }
  
  at = gdk_atom_intern(XA_WIN_CLIENT_LIST, FALSE);
  if ((ev->atom == at) && (ev->state == GDK_PROPERTY_NEW_VALUE))
    {
      tasks_update();
      populate_tasks();
      return;
    }

  at = gdk_atom_intern(XA_WIN_WORKSPACE_COUNT, FALSE);
  if ((ev->atom == at) && (ev->state == GDK_PROPERTY_NEW_VALUE))
    {
      num_desk = gnome_win_hints_get_workspace_count();
      populate_tasks();
      switch (applet_orient) 
	{
	 case ORIENT_UP:
	 case ORIENT_DOWN:
	  for (i = 0; i < 32; i++)
	    desk_widget[i] = NULL;
	  if (main_box)
	    gtk_widget_destroy(main_box);
	  main_box = NULL;
	  init_applet_gui_horiz();
	  break;
	 case ORIENT_LEFT:
	 case ORIENT_RIGHT:
	  for (i = 0; i < 32; i++)
	    desk_widget[i] = NULL;
	  if (main_box)
	    gtk_widget_destroy(main_box);
	  main_box = NULL;
	  init_applet_gui_vert();
	  break;
	}
      return;
    }

  at = gdk_atom_intern(XA_WIN_WORKSPACE_NAMES, FALSE);
  if ((ev->atom == at) && (ev->state == GDK_PROPERTY_NEW_VALUE))
    {
      get_desktop_names();
      return;
    }
  widget = NULL;
}

/* FILTER hack to stop gtk pulling out the rug from under me */
GdkFilterReturn
cb_filter_intercept(GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
  XEvent *xevent;
  Task *t;
  GList *ptr;
  
  xevent = (XEvent *)gdk_xevent;
  switch (xevent->type)
    {
     case DestroyNotify:
      gdk_window_destroy_notify(((GdkEventAny *)event)->window);
      return GDK_FILTER_REMOVE;
      break;
     case FocusIn:
     case FocusOut:
      ptr = tasks;
      while (ptr)
	{
	  t = (Task *)(ptr->data);
	  if (t->win == xevent->xfocus.window)
	    {
	      cb_task_change(NULL, (GdkEventProperty *)event, t);
	      return GDK_FILTER_CONTINUE;
	    }
	  ptr = ptr->next;
	}
      break;
     case ConfigureNotify:
      ptr = tasks;
      while (ptr)
	{
	  t = (Task *)(ptr->data);
	  if (t->win == xevent->xconfigure.window)
	    {
	      cb_task_change(NULL, (GdkEventProperty *)event, t);
	      return GDK_FILTER_CONTINUE;
	    }
	  ptr = ptr->next;
	}
      break;
     case PropertyNotify:
      ptr = tasks;
      while (ptr)
	{
	  t = (Task *)(ptr->data);
	  if (t->win == xevent->xproperty.window)
	    {
	      cb_task_change(NULL, (GdkEventProperty *)event, t);
	      return GDK_FILTER_CONTINUE;
	    }
	  ptr = ptr->next;
	}
      break;
     default:
      return GDK_FILTER_REMOVE;
    }
  return GDK_FILTER_REMOVE;
}

/* TASK manipulation functions */

void
task_get_info(Task *t)
{
  GnomeWinState       *win_state;
  Window               w2, ret, root, *wl;
  gchar               *name;
  gint                 size;
  guint                w, h, d;
  CARD32              *val;
  Atom                 a;
  int                  rev;

  /* is this window focused */
  t->focused = 0;
  XGetInputFocus(GDK_DISPLAY(), &ret, &rev);
  if (t->win == ret)
    t->focused = 1;

  /* get this windows title */
  name = NULL;
  if (t->name)
    g_free(t->name);
  t->name = NULL;
  name = util_get_atom(t->win, "WM_NAME", XA_STRING, &size);
  if (name)
    {
      t->name = g_malloc(size + 1);
      memcpy(t->name, name, size);
      t->name[size] = 0;
      g_free(name);
    }

  /* get the state of this window */
  t->iconified = 0;
  t->shaded = 0;
  t->sticky = 0;
  t->desktop = 0;

  /* iconified ? */
  a = XInternAtom(GDK_DISPLAY(), "WM_STATE", False);
  val = util_get_atom(t->win, "WM_STATE", a, &size);
  if (val)
    {
      if (val[0] == IconicState)
	t->iconified = 1;
      g_free(val);
    }
  
  /* sticky or shaded ? */
  win_state = util_get_atom(t->win, "_WIN_STATE", XA_CARDINAL, &size);
  if (win_state)
    {
      if (*win_state & WIN_STATE_STICKY)
	t->sticky = 1;
      if (*win_state & WIN_STATE_SHADED)
	t->shaded = 1;
      g_free(win_state);
    }
  
  /* what desktop is it on ? */
  val = util_get_atom(t->win, "_WIN_WORKSPACE", XA_CARDINAL, &size);
  if (val)
    {
      t->desktop = *val;
      g_free(val);
    }
  
  /* geometry!!!!!!!!!!! */
  XGetGeometry(GDK_DISPLAY(), t->win, &ret, &size, &size, &w, &h, &d, &d);
  t->w = (gint)w;
  t->h = (gint)h;
  wl = NULL;
  w2 = t->win;
  while (XQueryTree(GDK_DISPLAY(), w2, &root, &ret, &wl, &size))
    {
      if ((wl) && (size > 0))
	XFree(wl);
      w2 = ret;
      if (ret == root)
	break;
      val = util_get_atom(ret, "ENLIGHTENMENT_DESKTOP", XA_CARDINAL, &size);
      if (val)
	{
	  break;
	  g_free(val);
	}
    }
  /* Wheee w2 is our actualy "root" window even on virtual root windows */
  XTranslateCoordinates(GDK_DISPLAY(), t->win, w2, 0, 0, 
			&(t->x), &(t->y), &ret);
}

void 
task_add(Window win)
{
  Task               *t;
  int                 size;
  gchar              *str;
  CARD32             *val;
  GnomeWinHints      *win_hints;
  GnomeUIInfo         uinfo[5] = 
    {
      GNOMEUIINFO_END,
      GNOMEUIINFO_END,
      GNOMEUIINFO_END,
      GNOMEUIINFO_END,
      GNOMEUIINFO_END
    };

  /* has this task asked to be skipped by the task list ? */
  win_hints = util_get_atom(win, "_WIN_HINTS", XA_CARDINAL, &size);
  if (win_hints)
    {
      if ((*win_hints) & WIN_HINTS_SKIP_TASKBAR)
	{
	  g_free(win_hints);
	  return;
	}
      g_free(win_hints);
    }

  /* create task struct */
  t = g_malloc(sizeof(Task));

  /* initialize members */
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
  t->dummy = NULL;
  t->gdkwin = NULL;
  
  /* create dummy GTK widget to get events from */
  t->gdkwin = gdk_window_lookup(win);
  if (t->gdkwin)
    gdk_window_ref(t->gdkwin);
  else
    t->gdkwin = gdk_window_foreign_new(win);
/*  t->dummy = gtk_window_new(GTK_WINDOW_POPUP);*/
  /* realize that damn widget */
/*  gtk_widget_realize(t->dummy);*/
  gdk_window_add_filter(t->gdkwin, cb_filter_intercept, t->dummy);  
  /* fake events form win producing signals on dummy widget */
/*  gdk_window_set_user_data(t->gdkwin, t->dummy);*/
  /* conntect to "faked" signals */
/*  gtk_signal_connect(GTK_OBJECT(t->dummy), "property_notify_event",
		     GTK_SIGNAL_FUNC(cb_task_change), t);
  gtk_signal_connect(GTK_OBJECT(t->dummy), "focus_in_event",
		     GTK_SIGNAL_FUNC(cb_task_change), t);
  gtk_signal_connect(GTK_OBJECT(t->dummy), "focus_out_event",
		     GTK_SIGNAL_FUNC(cb_task_change), t);
  gtk_signal_connect(GTK_OBJECT(t->dummy), "configure_event",
		     GTK_SIGNAL_FUNC(cb_task_change), t);
*/
  /* make sure we get the events */
  XSelectInput(GDK_DISPLAY(), win, PropertyChangeMask | FocusChangeMask |
	       StructureNotifyMask);
  /* add this client to the list of clients */
  tasks = g_list_append(tasks, t);

  /* get info about task */
  task_get_info(t);
}


void 
task_delete(Window win)
{
  guint               i, n, tdesk;
  Task               *t;
  gchar               tstick;

  n = g_list_length(tasks);
  for (i = 0; i < n; i++)
    {
      t = (g_list_nth(tasks, i))->data;
      if (win == t->win)
	{
	  tstick = t->sticky;
	  tdesk = t->desktop;
	  if (t->name)
	    g_free(t->name);
	  if (t->gdkwin)
	    gdk_window_unref(t->gdkwin);
/*	  if (t->dummy)
	    gtk_widget_destroy(t->dummy);*/
	  tasks = g_list_remove(tasks, t);
	  g_free(t);
	  if (!tstick)
	    desktop_draw(tdesk);
	  else
	    {
	      for (i = 0; i < (guint)num_desk; i++)
		desktop_draw(i);
	    }
	  return;
	}
    }
}

Task *
task_find(Window win)
{
  guint               n, i;
  Task               *t;

  n = g_list_length(tasks);
  for (i = 0; i < n; i++)
    {
      t = (g_list_nth(tasks, i))->data;
      if (win == t->win)
	return t;
    }
  return NULL;
}

void 
tasks_match(Window * win, guint num)
{
  guint               i, j, there;
  GList              *p1;
  Task               *t;

  p1 = tasks;
  while (p1)
    {
      t = p1->data;
      p1 = p1->next;
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
	    task_delete(t->win);
	}
    }
  for (i = 0; i < num; i++)
    {
      there = 0;
      p1 = tasks;
      while (p1)
	{
	  t = p1->data;
	  p1 = p1->next;
	  if (t->win == win[i])
	    {
	      there = 1;
	      p1 = NULL;	      
	    }
	}
      if (!there)
	task_add(win[i]);
    }
}

void 
tasks_update(void)
{
  Window             *list;
  gint                 num, size;

  list = util_get_atom(GDK_ROOT_WINDOW(), "_WIN_CLIENT_LIST", 
		       XA_CARDINAL, &size);
  if ((size > 0) && (list))
    {
      num = size / sizeof(CARD32);
      tasks_match(list, num);
      g_free(list);
    }
}

void 
get_desktop_names(void)
{
  GList *gl, *p;
  gint i;
  
  for (i = 0; i < 32; i++)
    {
      if (desk_name[i])
	g_free(desk_name[i]);
      desk_name[i] = NULL;
    }
  gl = gnome_win_hints_get_workspace_names();  
  p = gl;
  i = 0;
  while ((p)  && (i < 32))
    {
      desk_name[i++] = p->data;
      p = p->next;
    }
  if (gl)
    g_list_free(gl);
}

/* select on events for the root window properies */
void
select_root_properties(void)
{
  GtkWidget *dummy_win;
  
  dummy_win = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_widget_realize(dummy_win);
  gdk_window_set_user_data(GDK_ROOT_PARENT(), dummy_win);
  gdk_window_ref(GDK_ROOT_PARENT());
  gdk_xid_table_insert(&(GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT())), 
		       GDK_ROOT_PARENT());
  gtk_signal_connect(GTK_OBJECT(dummy_win), "property_notify_event",
		     GTK_SIGNAL_FUNC(cb_root_prop_change), NULL);
  XSelectInput(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(GDK_ROOT_PARENT()), 
	       PropertyChangeMask);
}

/* MAIN MAIN MAIN MAIN */
int 
main(int argc, char *argv[])
{
  gint i;
  
  for (i = 0; i < 32; i++)
    {
      desk_name[i] = NULL;
      desk_widget[i] = NULL;
    }
  
  /*  panel_corba_register_arguments();*/
  applet_widget_init("gnomepager_applet","0.1", argc, argv, NULL, 
		     0, NULL, TRUE, FALSE, NULL, NULL);
  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());
  gnome_win_hints_init();
  if (!gnome_win_hints_wm_exists())
    {
      GtkWidget *d, *l;
      
      d = gnome_dialog_new(_("Gnome Pager Error"),
			   GNOME_STOCK_BUTTON_OK,
			   NULL);
      l = gtk_label_new(_("You are not running a GNOME Compliant Window Manager.\n"
			  "Please run a GNOME Compliant Window Manager.\n"
			  "For Example:\n"
			  "Enlightenment (DR-0.15)\n"
			  "Then start this applet again.\n"));
      gtk_widget_show(l);
      gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(d)->vbox), l, TRUE, TRUE, 5);
      gtk_window_set_modal(GTK_WINDOW(d),TRUE);
      gtk_window_position(GTK_WINDOW(d), GTK_WIN_POS_CENTER);
      gtk_window_set_modal(GTK_WINDOW(d),TRUE);
      gnome_dialog_run(GNOME_DIALOG(d));
      gtk_widget_destroy(d);
      
      exit(1);
    }
  
  pager_rows = gnome_config_get_int("gnome_pager/stuff/pager_rows=2");
  pager_size = gnome_config_get_int("gnome_pager/stuff/pager_size=1");
  tasks_all = gnome_config_get_int("gnome_pager/stuff/tasks_all=0");
  task_rows_h = gnome_config_get_int("gnome_pager/stuff/task_rows_h=2");
  task_rows_v = gnome_config_get_int("gnome_pager/stuff/task_rows_v=1");
  max_task_width = gnome_config_get_int("gnome_pager/stuff/max_task_width=400");
  show_tasks = gnome_config_get_int("gnome_pager/stuff/show_tasks=1");
  show_pager = gnome_config_get_int("gnome_pager/stuff/show_pager=1");
  show_icons = gnome_config_get_int("gnome_pager/stuff/show_icons=1");
  
  gdk_error_warnings = 0;  
  get_desktop_names();
  current_desk = gnome_win_hints_get_current_workspace();
  num_desk = gnome_win_hints_get_workspace_count();
  tasks_update();
  select_root_properties();

  gdk_imlib_data_to_pixmap(icon1_xpm, &p_1, &m_1);
  gdk_imlib_data_to_pixmap(icon2_xpm, &p_2, &m_2);
  gdk_imlib_data_to_pixmap(icon3_xpm, &p_3, &m_3);
  applet = applet_widget_new("gnomepager_applet");
  if (!applet)
    {
      g_error("Can't create applet!\n");
      exit(1);
    }
  gtk_signal_connect(GTK_OBJECT(applet), "change_orient",
		     GTK_SIGNAL_FUNC(cb_applet_orient_change),
		     NULL);
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"about",
					GNOME_STOCK_MENU_ABOUT,
					_("About..."),
					cb_applet_about,
					NULL);
  applet_widget_register_stock_callback(APPLET_WIDGET(applet),
					"properties",
					GNOME_STOCK_MENU_PROP,
					_("Properties..."),
					cb_applet_properties,
					NULL);
  
  gtk_widget_realize(applet);
  init_applet_gui();
  gtk_widget_show(applet);
  
  applet_widget_gtk_main();
  return 0;
}

void
init_applet_gui(void)
{
  gtk_window_set_policy(GTK_WINDOW(applet), 1, 1, 1);

  if (!hold_box)
    {
      hold_box = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
      gtk_widget_show(hold_box);
      applet_widget_add(APPLET_WIDGET(applet), hold_box);
    }
}

void 
desktop_draw(gint i)
{
  if ((i >= 0) && (i < 32) && (desk_widget[i]))
    desktop_cb_redraw(desk_widget[i], NULL);
}

void
desktop_cb_button_down(GtkWidget * widget, GdkEventButton *event)
{
  gint desk, selm, button;
  
  button = event->button;  
  desk = (gint)gtk_object_get_data(GTK_OBJECT(widget), "desktop");
  
  if (button == 1)
    gnome_win_hints_set_current_workspace(desk);
}

void
desktop_cb_button_up(GtkWidget * widget, GdkEventButton *event)
{
  gint desk, selm, button;
  
  button = event->button;  
  desk = (gint)gtk_object_get_data(GTK_OBJECT(widget), "desktop");
}

void
desktop_cb_redraw(GtkWidget *widget, gpointer data)
{
  GtkStyle *s;
  GdkWindow *win;
  gint sel, sw, sh, w, h, desk, x, y, ww, hh;
  Task *t;
  GList *p;

  if (!widget)
    return;
  if (!widget->window)
    return;
  if (!widget->style)
    return;
  if (!(show_pager))
    return;
  desk = (gint)gtk_object_get_data(GTK_OBJECT(widget), "desktop");
  sel = (gint)gtk_object_get_data(GTK_OBJECT(widget), "select");
  w = widget->allocation.width - 4;
  h = widget->allocation.height - 4;
  s = widget->style;
  win = widget->window;
  sw = gdk_screen_width();
  sh = gdk_screen_height();
  
  if (desk == current_desk)
    gtk_draw_box(s, win, GTK_STATE_ACTIVE, GTK_SHADOW_IN, 0, 0, -1, -1);
  else
    gtk_draw_box(s, win, GTK_STATE_ACTIVE, GTK_SHADOW_OUT, 0, 0, -1, -1);
  
  p = tasks;
  while (p)
    {
      t = (Task *)p->data;
      if (((t->desktop == desk) || (t->sticky)) && (!(t->iconified)))
	{
	  x = 2 + (t->x * w) / sw;
	  ww = (t->w * w) / sw;
	  y = 2 +(t->y * h) / sh;
	  hh = (t->h * h) / sh;
	  if (t->focused)
	    gtk_draw_box(s, win, GTK_STATE_PRELIGHT, GTK_SHADOW_OUT, 
			 x, y, ww, hh);
	  else
	    gtk_draw_box(s, win, GTK_STATE_NORMAL, GTK_SHADOW_ETCHED_IN, 
			 x, y, ww, hh);
	    
	}
      p = p->next;
    }
}

GtkWidget *
make_desktop_pane(gint desktop, gint width, gint height)
{
  GtkWidget *area, *tip;
  gchar *s;
  
  area = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(area), width, height);
  gtk_object_set_data(GTK_OBJECT(area), "desktop", (gpointer)desktop);
  gtk_object_set_data(GTK_OBJECT(area), "select", (gpointer)0);
  gtk_widget_set_events(area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			GDK_EXPOSURE_MASK | GDK_ENTER_NOTIFY_MASK |
			GDK_LEAVE_NOTIFY_MASK);
  gtk_signal_connect(GTK_OBJECT(area), "button_press_event",
		     GTK_SIGNAL_FUNC(desktop_cb_button_down), NULL);
  gtk_signal_connect(GTK_OBJECT(area), "button_release_event",
		     GTK_SIGNAL_FUNC(desktop_cb_button_up), NULL);
  gtk_signal_connect(GTK_OBJECT(area), "expose_event",
		     GTK_SIGNAL_FUNC(desktop_cb_redraw), NULL);
  s = desk_name[desktop];
  if (s) 
    {
/*      tip = (GtkWidget *)gtk_tooltips_new();
      gtk_tooltips_set_tip(GTK_TOOLTIPS(tip), area, s, NULL);
      gtk_tooltips_enable(GTK_TOOLTIPS(tip));*/
    }
  return area;
}

void
init_applet_gui_horiz(void)
{
  GtkWidget *hbox, *frame, *button, *arrow, *table;
  GtkWidget *desk, *align;
  gint i, j, k;
  
  if (main_box)
    return;
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(hbox);
  main_box = hbox;
  gtk_container_add(GTK_CONTAINER(hold_box), hbox);

  align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
  if (show_pager)
    gtk_widget_show(align);
  gtk_box_pack_start(GTK_BOX(hbox), align, FALSE, FALSE, 0);
  
  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  gtk_widget_show(frame);
  gtk_container_add(GTK_CONTAINER(align), frame);

  table = gtk_table_new(1, 1, TRUE);
  gtk_widget_show(table);
  gtk_container_add(GTK_CONTAINER(frame), table);
  
  j = 0;
  k = 0;
  for (i = 0; i < num_desk; i++)
    {
      if (pager_size)
	desk = make_desktop_pane(i, 31, 22);
      else
	desk = make_desktop_pane(i, 62, 44);
      desk_widget[i] = desk;
      gtk_widget_show(desk);
      gtk_table_attach_defaults(GTK_TABLE(table), desk,
				j, j + 1, k, k + 1);
      j++;
      if (j >= num_desk / pager_rows)
	{
	  j = 0;
	  k ++;
	}
    }
  
  if (applet_orient == ORIENT_UP)
    arrow = gtk_arrow_new(GTK_ARROW_UP, GTK_SHADOW_OUT);
  else
    arrow = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_OUT);
  gtk_widget_show(arrow);
  
  button = gtk_button_new();
  gtk_widget_show(button);
  gtk_container_add(GTK_CONTAINER(button), arrow);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
/*  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     GTK_SIGNAL_FUNC(showpop_cb), NULL);*/  

  frame = gtk_frame_new(NULL);
  gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 0);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  if (show_tasks)
    gtk_widget_show(frame);

  table = gtk_table_new(1, 1, TRUE);
  gtk_widget_show(table);
  gtk_container_add(GTK_CONTAINER(frame), table);
  task_table = table;
  emtpy_task_widgets();
  populate_tasks();
}

void
init_applet_gui_vert(void)
{
  GtkWidget *vbox, *frame, *button, *arrow, *table;
  GtkWidget *desk, *align;
  gint i, j, k;
  
  if (main_box)
    return;
  vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(vbox);
  main_box = vbox;
  gtk_container_add(GTK_CONTAINER(hold_box), vbox);

  align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
  if (show_pager)
    gtk_widget_show(align);
  gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 0);
  
  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  gtk_widget_show(frame);
  gtk_container_add(GTK_CONTAINER(align), frame);

  table = gtk_table_new(1, 1, TRUE);
  gtk_widget_show(table);
  gtk_container_add(GTK_CONTAINER(frame), table);
  
  j = 0;
  k = 0;
  for (i = 0; i < num_desk; i++)
    {
      if (pager_size)
	desk = make_desktop_pane(i, PAGER_W_0, PAGER_H_0);
      else
	desk = make_desktop_pane(i, PAGER_W_1, PAGER_W_1);
      desk_widget[i] = desk;
      gtk_widget_show(desk);
      gtk_table_attach_defaults(GTK_TABLE(table), desk,
				j, j + 1, k, k + 1);
      j++;
      if (j >= pager_rows)
	{
	  j = 0;
	  k ++;
	}
    }
  
  if (applet_orient == ORIENT_LEFT)
    arrow = gtk_arrow_new(GTK_ARROW_LEFT, GTK_SHADOW_OUT);
  else
    arrow = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
  gtk_widget_show(arrow);
  
  button = gtk_button_new();
  gtk_widget_show(button);
  gtk_container_add(GTK_CONTAINER(button), arrow);
  gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
/*  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     GTK_SIGNAL_FUNC(showpop_cb), NULL);*/  

  frame = gtk_frame_new(NULL);
  gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  if (show_tasks)
    gtk_widget_show(frame);

  table = gtk_table_new(1, 1, TRUE);
  gtk_widget_show(table);
  gtk_container_add(GTK_CONTAINER(frame), table);
  task_table = table;
  emtpy_task_widgets();
  populate_tasks();
}

void
task_cb_button_enter(GtkWidget * widget, GdkEventCrossing *event)
{
  Task *t;
  
  t = (Task *)gtk_object_get_data(GTK_OBJECT(widget), "task");
  set_task_info_to_button(t);
}

void
task_cb_button_leave(GtkWidget * widget, GdkEventCrossing *event)
{
  Task *t;
  
  t = (Task *)gtk_object_get_data(GTK_OBJECT(widget), "task");
  set_task_info_to_button(t);
}

void
task_cb_button_down(GtkWidget * widget, GdkEventButton *event)
{
  gint button;
  Task *t;
  
  button = event->button;  
  t = (Task *)gtk_object_get_data(GTK_OBJECT(widget), "task");
  set_task_info_to_button(t);
}

void
task_cb_button_up(GtkWidget * widget, GdkEventButton *event)
{
  gint button;
  Task *t;
  
  button = event->button;  
  t = (Task *)gtk_object_get_data(GTK_OBJECT(widget), "task");
  set_task_info_to_button(t);
  if (button == 1)
    client_win_show(t);
  else if (button == 2)
    {
      if (t->iconified)
	client_win_show(t);
      else
	client_win_iconify(t);
    }
}

void
cb_showhide(GtkWidget * widget, Task *t)
{
   if (t->iconified)
     client_win_show(t);
   else
     client_win_iconify(t);
}

void
cb_shade(GtkWidget * widget, Task *t)
{
   if (t->shaded)
     client_win_unshade(t);
   else
     client_win_shade(t);
}

void
cb_kill(GtkWidget * widget, Task *t)
{
   client_win_close(t);
}

void
cb_nuke(GtkWidget * widget, Task *t)
{
   client_win_kill(t);
}

void
emtpy_task_widgets(void)
{
  if (task_widgets)
    g_list_free(task_widgets);
  task_widgets = NULL;
  if (task_dest)
    g_list_free(task_dest);
  task_dest = NULL;
}

void
desktroy_task_widgets(void)
{
  GList        *p;
  
  p = task_widgets;
  while (p)
    {
#ifdef ANIMATION      
      GtkWidget    *icon2;
      
      icon2 = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(p->data), "icon2");
      gnome_animator_stop(GNOME_ANIMATOR(icon2));
#endif     
      gtk_widget_destroy(GTK_WIDGET(p->data));
      p = p->next;
    }
  p = task_dest;
  while (p)
    {
      printf("nuking %p\n", p->data);
      gtk_widget_destroy((GtkWidget *)(p->data));
      printf("nuked %p\n", p->data);
      p = p->next;
    }
  emtpy_task_widgets();
}

GtkWidget *
find_task_widget(Task *t)
{
  GList           *p;
  GtkWidget       *button;
  Task            *tt;
  
  p = task_widgets;
  while(p)
    {
      button = (GtkWidget *)(p->data);
      tt = (Task *)gtk_object_get_data(GTK_OBJECT(button), "task");
      if (tt == t)
	return button;
      p = p->next;
    }
  return NULL;
}

void
set_task_info_to_button(Task *t)
{
  GtkWidget *button, *icon1, *icon2, *icon3, *label;
  GtkRequisition  req;
  gint num, len, mw;
  gchar *str;
  GList *p;

  if (!t)
    return;
  button = find_task_widget(t);
  if (!button)
    return;
  
  icon1 = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(button), "icon1");
  icon2 = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(button), "icon2");
  icon3 = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(button), "icon3");
  label = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(button), "label");
  if (t->iconified)
    {
      if (show_icons)
	{
	  gtk_widget_hide(icon1);
	  gtk_widget_hide(icon2);
	  gtk_widget_show(icon3);
	}
    }
  else
    {
      if (t->focused)
	{
#ifdef ANIMATION      
	  gnome_animator_start(GNOME_ANIMATOR(icon2));
#endif
	  if (show_icons)
	    {
	      gtk_widget_hide(icon3);
	      gtk_widget_hide(icon1);
	      gtk_widget_show(icon2);
	    }
	  gtk_widget_set_state(button, GTK_STATE_ACTIVE);
	}
      else
	{
#ifdef ANIMATION      
	  gnome_animator_stop(GNOME_ANIMATOR(icon2));
#endif	  
	  if (show_icons)
	    {
	      gtk_widget_hide(icon3);
	      gtk_widget_hide(icon2);
	      gtk_widget_show(icon1);
	    }
	  gtk_widget_set_state(button, GTK_STATE_NORMAL);
	}
    }
  num = 0;
  p = tasks;
  while (p)
    {
      if ((tasks_all) || 
	  (((Task *)(p->data))->sticky) || 
	  (((Task *)(p->data))->desktop == current_desk))
	num++;
      p = p->next;
    }
  if (t->name)
    gtk_label_set(GTK_LABEL(label), t->name);
  else
    gtk_label_set(GTK_LABEL(label), "???");
  gtk_widget_size_request(button, &req);
  if ((applet_orient == ORIENT_UP) || (applet_orient == ORIENT_DOWN))
    {
       if (num < task_rows_h) 
	 num = task_rows_h;
      mw = max_task_width / ((num + task_rows_h - 1) / task_rows_h);
      if (t->name)
	len = strlen(t->name);
      else
	len = strlen("???");
      while ((req.width > mw) && (len > 0))
	{
	  len--;
	  if (t->name)
	    str = util_reduce_chars(t->name, len);
	  else
	    str = util_reduce_chars("???", len);
	  gtk_label_set(GTK_LABEL(label), str);
	  g_free(str);
	  gtk_widget_size_request(button, &req);
	}
      if (len <=0)
	gtk_label_set(GTK_LABEL(label), "");
    }
  else
    {
      if (pager_size)
	mw = pager_rows * PAGER_W_0;
      else
	mw = pager_rows * PAGER_W_1;		
      if (t->name)
	len = strlen(t->name);
      else
	len = strlen("???");
      while ((req.width > mw) && (len > 0))
	{
	  len--;
	  if (t->name)
	    str = util_reduce_chars(t->name, len);
	  else
	    str = util_reduce_chars("???", len);
	  gtk_label_set(GTK_LABEL(label), str);
	  g_free(str);
	  gtk_widget_size_request(button, &req);
	}
      if (len <=0)
	gtk_label_set(GTK_LABEL(label), "");
    }
}

void
populate_tasks(void)
{
   Task            *t;
   GList           *p;
   GtkWidget       *hbox, *button, *icon1, *icon2, *icon3, *label, *menu, *tip;
#ifdef ANIMATION
   GtkWidget       *f1, *f2, *f3, *f4;
#endif  
   gint             n_rows, n_cols, num, i, j, k, mw, len;
   gchar           *str;
   GnomeUIInfo     uinfo[5] = {GNOMEUIINFO_END, GNOMEUIINFO_END, 
      GNOMEUIINFO_END, GNOMEUIINFO_END};

  if (!task_table)
    return;
  desktroy_task_widgets();
  if (!tasks)
    return;
  j = 0;
  k = 0;
  n_rows = 0;
  n_cols = 0;
  num = 0;
  p = tasks;
  while (p)
    {
      if ((tasks_all) || 
	  (((Task *)(p->data))->sticky) || 
	  (((Task *)(p->data))->desktop == current_desk))
	num++;
      p = p->next;
    }
  p = tasks;
  while (p)
    {
      t = (Task *)p->data;
      if (((!(tasks_all)) && (t->desktop == current_desk)) || (tasks_all) ||
	  (t->sticky))
	{
	  hbox = gtk_hbox_new(0, FALSE);
	  gtk_widget_show(hbox);
	  
	  label = gtk_label_new(t->name);
	  gtk_widget_show(label);

	  icon1 = icon2 = icon3 = NULL;
	  icon1 = gtk_pixmap_new(p_1, m_1);
	  icon3 = gtk_pixmap_new(p_3, m_3);
#ifdef ANIMATION
	  f1 = gnome_pixmap_new_from_xpm_d(f1_xpm);
	  f2 = gnome_pixmap_new_from_xpm_d(f2_xpm);
	  f3 = gnome_pixmap_new_from_xpm_d(f3_xpm);
	  f4 = gnome_pixmap_new_from_xpm_d(f4_xpm);
	  icon2 = gnome_animator_new_with_size(16, 16);
	  gnome_animator_append_frame_from_gnome_pixmap(GNOME_ANIMATOR(icon2),
							GNOME_PIXMAP(f1), 
							0, 0, 100);
	  gnome_animator_append_frame_from_gnome_pixmap(GNOME_ANIMATOR(icon2),
							GNOME_PIXMAP(f2), 
							0, 0, 100);
	  gnome_animator_append_frame_from_gnome_pixmap(GNOME_ANIMATOR(icon2),
							GNOME_PIXMAP(f3), 
							0, 0, 100);
	  gnome_animator_append_frame_from_gnome_pixmap(GNOME_ANIMATOR(icon2),
							GNOME_PIXMAP(f4), 
							0, 0, 100);
	  gnome_animator_set_loop_type(GNOME_ANIMATOR(icon2),
				       GNOME_ANIMATOR_LOOP_RESTART);
	  gnome_animator_start(GNOME_ANIMATOR(icon2));
#else	  
	  icon2 = gtk_pixmap_new(p_2, m_2);
#endif	  

	  if (show_icons)
	    gtk_widget_show(icon1);
	  
	  gtk_box_pack_start(GTK_BOX(hbox), icon1, FALSE, FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(hbox), icon2, FALSE, FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(hbox), icon3, FALSE, FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 1);

	  button = gtk_button_new();
	  gtk_widget_show(button);
	  gtk_signal_connect(GTK_OBJECT(button), "button_press_event",
			     GTK_SIGNAL_FUNC(task_cb_button_down), t);
	  gtk_signal_connect(GTK_OBJECT(button), "button_release_event",
			     GTK_SIGNAL_FUNC(task_cb_button_up), t);
	  gtk_signal_connect(GTK_OBJECT(button), "enter",
			     GTK_SIGNAL_FUNC(task_cb_button_enter), t);
	  gtk_signal_connect(GTK_OBJECT(button), "leave",
			     GTK_SIGNAL_FUNC(task_cb_button_leave), t);
/*	  
	  tip = (GtkWidget *)gtk_tooltips_new();
	  gtk_tooltips_set_tip(GTK_TOOLTIPS(tip), button, t->name, NULL);
	  gtk_tooltips_enable(GTK_TOOLTIPS(tip));
*/	  
	   uinfo[0].type            = GNOME_APP_UI_ITEM;
	   uinfo[0].label           = N_("Show / Hide");
	   uinfo[0].hint            = NULL;
	   uinfo[0].moreinfo        = cb_showhide;
	   uinfo[0].user_data       = t;
	   uinfo[0].unused_data     = NULL;
	   uinfo[0].pixmap_type     = GNOME_APP_PIXMAP_NONE;
	   uinfo[0].pixmap_info     = GNOME_STOCK_MENU_OPEN;
	   uinfo[0].accelerator_key = 0;
	   uinfo[0].ac_mods         = (GdkModifierType) 0;
	   uinfo[0].widget          = NULL;
	   
	   uinfo[1].type            = GNOME_APP_UI_ITEM;
	   uinfo[1].label           = N_("Shade / Unshade");
	   uinfo[1].hint            = NULL;
	   uinfo[1].moreinfo        = cb_shade;
	   uinfo[1].user_data       = t;
	   uinfo[1].unused_data     = NULL;
	   uinfo[1].pixmap_type     = GNOME_APP_PIXMAP_NONE;
	   uinfo[1].pixmap_info     = GNOME_STOCK_MENU_OPEN;
	   uinfo[1].accelerator_key = 0;
	   uinfo[1].ac_mods         = (GdkModifierType) 0;
	   uinfo[1].widget          = NULL;
	   
	   uinfo[2].type            = GNOME_APP_UI_ITEM;
	   uinfo[2].label           = N_("Close");
	   uinfo[2].hint            = NULL;
	   uinfo[2].moreinfo        = cb_kill;
	   uinfo[2].user_data       = t;
	   uinfo[2].unused_data     = NULL;
	   uinfo[2].pixmap_type     = GNOME_APP_PIXMAP_NONE;
	   uinfo[2].pixmap_info     = GNOME_STOCK_MENU_OPEN;
	   uinfo[2].accelerator_key = 0;
	   uinfo[2].ac_mods         = (GdkModifierType) 0;
	   uinfo[2].widget          = NULL;
	   
	   uinfo[3].type            = GNOME_APP_UI_ITEM;
	   uinfo[3].label           = N_("Nuke");
	   uinfo[3].hint            = NULL;
	   uinfo[3].moreinfo        = cb_nuke;
	   uinfo[3].user_data       = t;
	   uinfo[3].unused_data     = NULL;
	   uinfo[3].pixmap_type     = GNOME_APP_PIXMAP_NONE/*GNOME_APP_PIXMAP_STOCK*/;
	   uinfo[3].pixmap_info     = GNOME_STOCK_MENU_OPEN;
	   uinfo[3].accelerator_key = 0;
	   uinfo[3].ac_mods         = (GdkModifierType) 0;
	   uinfo[3].widget          = NULL;

/*	  menu = gnome_popup_menu_new(uinfo);
	  gnome_popup_menu_attach(menu, button, t);*/
/*	  task_dest = g_list_append(task_dest, menu);*/

	  gtk_object_set_data(GTK_OBJECT(button), "icon1", icon1);
	  gtk_object_set_data(GTK_OBJECT(button), "icon2", icon2);
	  gtk_object_set_data(GTK_OBJECT(button), "icon3", icon3);
	  gtk_object_set_data(GTK_OBJECT(button), "label", label);
	  gtk_object_set_data(GTK_OBJECT(button), "task", t);
	  
	  gtk_container_add(GTK_CONTAINER(button), hbox);
	  
	  gtk_table_attach_defaults(GTK_TABLE(task_table), button,
				    j, j + 1, k, k + 1);
	  gtk_widget_realize(label);
	  task_widgets = g_list_append(task_widgets, button);
	  set_task_info_to_button(t);
	  if ((applet_orient == ORIENT_UP) || (applet_orient == ORIENT_DOWN))
	     {	    
		if (num < task_rows_h) 
		  num = task_rows_h;
	      j++;
	      if (n_cols < j)
		n_cols = j;
	      if (n_rows < k + 1)
		n_rows = k + 1;
	      if (j >= ((num + task_rows_h - 1)/ task_rows_h))
		{
		  j = 0;
		  k++;
		}
	    }
	  else
	    {
	       if (num < task_rows_v) 
		 num = task_rows_v;
	      k++;
	      if (n_cols < j + 1)
		n_cols = j + 1;
	      if (n_rows < k)
		n_rows = k;
	      if (k >= ((num + task_rows_v - 1) / task_rows_v))
		{
		  k = 0;
		  j++;
		}
	    }
	}
      p = p->next;
    }
  gtk_table_resize(GTK_TABLE(task_table), n_rows, n_cols);
}

/*  

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
*/

/*
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

  gint                num_ws;
  gint                even;
  gint                i;

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
  
  update_tasks();
  return;
}
*/
