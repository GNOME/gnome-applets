#include <gnomepager.h>

extern gint         gdk_error_warnings;
GtkWidget          *applet = NULL;
GtkWidget          *popbox = NULL;
GtkWidget          *popbox_q = NULL;
GtkWidget         **blists = NULL;
GtkWidget         **flists = NULL;
gint                blists_num = 0;
GList              *tasks = NULL;
PanelOrientType     applet_orient = -1;
gint                current_desk = 0;
gint                num_desk = 0;
gchar              *desk_name[32];
GtkWidget          *desk_widget[32];
GList              *task_widgets = NULL;
GList              *task_dest = NULL;
GtkWidget          *hold_box = NULL;
GtkWidget          *main_box = NULL;
GtkWidget          *task_table = NULL;
GtkWidget          *prop_button = NULL;
GtkWidget          *arrow_button = NULL;
GdkPixmap          *p_1 = NULL, *p_2 = NULL, *p_3 = NULL;
GdkPixmap          *m_1 = NULL, *m_2 = NULL, *m_3 = NULL;

typedef struct {
	gint                pager_rows;
	gint                pager_size; /*bool*/
	gint                tasks_all; /*bool*/
	gint                minimized_tasks; /*bool*/
	gint                minimized_tasks_all; /*bool*/
	gint                minimized_tasks_only; /*bool*/
	gint                task_rows_h;
	gint                task_rows_v;
	gint                max_task_width;
	gint                max_task_vwidth;
	gint                show_tasks; /*bool*/
	gint                show_pager; /*bool*/
	gint                show_icons; /*bool*/
	gint		    show_arrow; /*bool*/
	gint                fixed_tasklist; /*bool*/
	gint                pager_w_0, pager_h_0;
	gint                pager_w_1, pager_h_1;
} Config;

/*note: no need for defaults as they will be read in*/
Config config; /*this is the actual current configuration*/
Config o_config; /*temporary configuration used for properties*/

gint                area_w = 1;
gint                area_h = 1;
gint                area_x = 0;
gint                area_y = 0;
gchar               tasks_changed = 0;

#define PAGER_W_0 31
#define PAGER_H_0 22
#define PAGER_W_1 62
#define PAGER_H_1 44

static void 
redo_interface(void)
{
  gint i;

  for (i = 0; i < 32; i++)
    desk_widget[i] = NULL;

  /*hackish way to keep these widgets around*/
  if(prop_button->parent)
	  gtk_container_remove(GTK_CONTAINER(prop_button->parent),
			       prop_button);
  if(arrow_button->parent)
	  gtk_container_remove(GTK_CONTAINER(arrow_button->parent),
			       arrow_button);
  /*kill the arrow*/
  if(GTK_BIN(arrow_button)->child)
    gtk_widget_destroy(GTK_BIN(arrow_button)->child);

  if (main_box)
    gtk_widget_destroy(main_box);
  if (popbox)
    gtk_widget_destroy(popbox);
  popbox_q = popbox = NULL;
  if (blists)
    g_free(blists);
  if (flists)
    g_free(flists);
  blists = NULL;
  blists_num = 0;
  flists = NULL;
  main_box = NULL;

  switch (applet_orient) 
    {
     case ORIENT_UP:
     case ORIENT_DOWN:
      init_applet_gui(TRUE);
      break;
     case ORIENT_LEFT:
     case ORIENT_RIGHT:
      init_applet_gui(FALSE);
      break;
    }
}



/* APPLET callbacks */
void 
cb_applet_orient_change(GtkWidget *w, PanelOrientType o, gpointer data)
{
  if (o == applet_orient) 
    return;

  applet_orient = o;
  
  redo_interface();

  gtk_widget_show(w);
}


void 
cb_applet_about(AppletWidget * widget, gpointer data)
{
  GtkWidget          *about;
  const gchar        *authors[] =
    {"The Rasterman", NULL};
  
  about = gnome_about_new
    (_("Desktop Pager Applet"), "0.1", _("Copyright (C)1998,1999 Red Hat Software, Free Software Foundation"),
     authors,
     _("Pager for a GNOME compliant Window Manager"),
     NULL);
  gtk_widget_show(about);
}

static void 
cb_check(GtkWidget *widget, gint * the_data)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(widget), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  if (GTK_TOGGLE_BUTTON(widget)->active)
    *the_data = 1;
  else
    *the_data = 0;
}

static void 
cb_check_enable(GtkWidget *tb, GtkWidget *thewid)
{
	gtk_widget_set_sensitive(thewid,
				 GTK_TOGGLE_BUTTON(tb)->active);
}

static void 
cb_adj(GtkAdjustment *adj, gint *the_data)
{
  GtkWidget *prop;
  
  prop = gtk_object_get_data(GTK_OBJECT(adj), "prop");
  if (prop)
    gnome_property_box_changed(GNOME_PROPERTY_BOX(prop));
  *the_data = (gint)(adj->value);
}

void
cb_prop_apply(GtkWidget *widget, int page, gpointer data)
{
  if (page != -1)
    return;

  config = o_config;
  
  redo_interface();
}

static void
prop_destroyed(GtkWidget *widget, GtkWidget **prop)
{
	*prop = NULL;
}

void 
cb_applet_properties(AppletWidget * widget, gpointer data)
{
  static GnomeHelpMenuEntry help_entry = { NULL, "properties" };
  static GtkWidget *prop = NULL;
  GtkWidget *hbox,*hbox1,*vbox,*vbox1,*frame,*label, *spin, *check, *sh;
  GtkWidget *rb1, *rb2;
  GtkAdjustment *adj;
  
  if(prop)
    {
      gtk_widget_show(prop);
      if(prop->window) gdk_window_raise(prop->window);
      return;
    }

  help_entry.name = gnome_app_id;

  o_config = config;
  
  prop = gnome_property_box_new();
  gtk_signal_connect (GTK_OBJECT(prop), "destroy",
		      GTK_SIGNAL_FUNC(prop_destroyed),&prop);
  gtk_signal_connect (GTK_OBJECT(prop), "apply",
		      GTK_SIGNAL_FUNC(cb_prop_apply), NULL);
  gtk_signal_connect (GTK_OBJECT(prop), "help",
		      GTK_SIGNAL_FUNC(gnome_help_pbox_display),
		      &help_entry);
  gtk_window_set_title(GTK_WINDOW(prop), _("Gnome Pager Settings"));
  

  /* Pager properties page */
  vbox = gtk_vbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_container_set_border_width(GTK_CONTAINER(vbox),GNOME_PAD_SMALL);
  gnome_property_box_append_page(GNOME_PROPERTY_BOX(prop), vbox,
				 gtk_label_new (_("Pager")));
  
  sh = check = gtk_check_button_new_with_label(_("Show pager"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), config.show_pager);
  gtk_signal_connect(GTK_OBJECT(check), "toggled",
		     GTK_SIGNAL_FUNC(cb_check), &o_config.show_pager);
  gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  
  check = gtk_check_button_new_with_label(_("Use small pagers"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), config.pager_size);
  gtk_signal_connect(GTK_OBJECT(check), "toggled",
		     GTK_SIGNAL_FUNC(cb_check), &o_config.pager_size);
  gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(sh), "toggled",
		     GTK_SIGNAL_FUNC(cb_check_enable), check);
  cb_check_enable(sh,check);

  adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)config.pager_rows, 1, 
					    8, 1, 1, 1 );
  label = gtk_label_new(_("Rows of pagers"));
  spin = gtk_spin_button_new(adj, 1, 0);
  hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),spin,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		     GTK_SIGNAL_FUNC(cb_adj), &o_config.pager_rows);
  gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);

  adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)config.pager_w_0, 1, 
					    100, 1, 1, 1 );
  label = gtk_label_new(_("Width of small pagers"));
  spin = gtk_spin_button_new(adj, 1, 0);
  hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),spin,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		     GTK_SIGNAL_FUNC(cb_adj), &o_config.pager_w_0);
  gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);

  adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)config.pager_h_0, 1, 
					    100, 1, 1, 1 );
  label = gtk_label_new(_("Height of small pagers"));
  spin = gtk_spin_button_new(adj, 1, 0);
  hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),spin,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		     GTK_SIGNAL_FUNC(cb_adj), &o_config.pager_h_0);
  gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);

  adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)config.pager_w_1, 1, 
					    100, 1, 1, 1 );
  label = gtk_label_new(_("Width of large pagers"));
  spin = gtk_spin_button_new(adj, 1, 0);
  hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),spin,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		     GTK_SIGNAL_FUNC(cb_adj), &o_config.pager_w_1);
  gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);

  adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)config.pager_h_1, 1, 
					    100, 1, 1, 1 );
  label = gtk_label_new(_("Height of large pagers"));
  spin = gtk_spin_button_new(adj, 1, 0);
  hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),spin,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		     GTK_SIGNAL_FUNC(cb_adj), &o_config.pager_h_1);
  gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);

  gtk_signal_connect(GTK_OBJECT(sh), "toggled",
		     GTK_SIGNAL_FUNC(cb_check_enable), hbox);
  cb_check_enable(sh,hbox);


  /* Tasklist properties page */
  vbox = gtk_vbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_container_set_border_width(GTK_CONTAINER(vbox),GNOME_PAD_SMALL);
  gnome_property_box_append_page(GNOME_PROPERTY_BOX(prop), vbox,
				 gtk_label_new (_("Tasklist")));
  
  check = gtk_check_button_new_with_label(_("Show task list button"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check), config.show_arrow);
  gtk_signal_connect(GTK_OBJECT(check), "toggled",
		     GTK_SIGNAL_FUNC(cb_check), &o_config.show_arrow);
  gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);

  sh = check = gtk_check_button_new_with_label(_("Show task list"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), config.show_tasks);
  gtk_signal_connect(GTK_OBJECT(check), "toggled",
		     GTK_SIGNAL_FUNC(cb_check), &o_config.show_tasks);
  gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);

  check = gtk_check_button_new_with_label(_("Show button icons"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), config.show_icons);
  gtk_signal_connect(GTK_OBJECT(check), "toggled",
		     GTK_SIGNAL_FUNC(cb_check), &o_config.show_icons);
  gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
  gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(sh), "toggled",
		     GTK_SIGNAL_FUNC(cb_check_enable), check);
  cb_check_enable(sh,check);

  hbox1 = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(vbox),hbox1,FALSE,FALSE,0);
  
  /* Radio buttons for which tasks to show */
  
  frame = gtk_frame_new(_("Which tasks to show"));
  gtk_box_pack_start(GTK_BOX(hbox1),frame,FALSE,FALSE,0);
  vbox1 = gtk_vbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_container_set_border_width(GTK_CONTAINER(vbox1),GNOME_PAD_SMALL);
  gtk_container_add(GTK_CONTAINER(frame),vbox1);
  gtk_signal_connect(GTK_OBJECT(sh), "toggled",
		     GTK_SIGNAL_FUNC(cb_check_enable), frame);
  cb_check_enable(sh,frame);

  rb1 = check = gtk_radio_button_new_with_label(NULL,_("Show all tasks"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check), config.minimized_tasks);
  gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
  gtk_box_pack_start(GTK_BOX(vbox1),check,FALSE,FALSE,0);

  check = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (rb1),
			  _("Show normal tasks only"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check),
				!config.minimized_tasks && !config.minimized_tasks_only);
  gtk_box_pack_start(GTK_BOX(vbox1),check,FALSE,FALSE,0);
  
  rb2 = check = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (rb1),
			  _("Show minimized tasks only"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check), config.minimized_tasks_only);
  gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
  gtk_box_pack_start(GTK_BOX(vbox1),check,FALSE,FALSE,0);

  /* We connect these signals afterwards, so that when we set the
   * states above, we don't trigger these callbacks.
   */
  gtk_signal_connect(GTK_OBJECT(rb1), "toggled",
		     GTK_SIGNAL_FUNC(cb_check), &o_config.minimized_tasks);
  gtk_signal_connect(GTK_OBJECT(rb2), "toggled",
		     GTK_SIGNAL_FUNC(cb_check), &o_config.minimized_tasks_only);

  check = gtk_check_button_new_with_label(_("Show all tasks on all desktops"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), config.tasks_all);
  gtk_signal_connect(GTK_OBJECT(check), "toggled",
		     GTK_SIGNAL_FUNC(cb_check), &o_config.tasks_all);
  gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
  gtk_box_pack_start(GTK_BOX(vbox1),check,FALSE,FALSE,0);

  check = gtk_check_button_new_with_label(_("Show minimized tasks on all desktops"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), config.minimized_tasks_all);
  gtk_signal_connect(GTK_OBJECT(check), "toggled",
		     GTK_SIGNAL_FUNC(cb_check), &o_config.minimized_tasks_all);
  gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
  gtk_box_pack_start(GTK_BOX(vbox1),check,FALSE,FALSE,0);

  frame = gtk_frame_new(_("Geometry"));
  gtk_box_pack_start(GTK_BOX(hbox1),frame,FALSE,FALSE,0);
  vbox1 = gtk_vbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_container_set_border_width(GTK_CONTAINER(vbox1),GNOME_PAD_SMALL);
  gtk_container_add(GTK_CONTAINER(frame),vbox1);
  gtk_signal_connect(GTK_OBJECT(sh), "toggled",
		     GTK_SIGNAL_FUNC(cb_check_enable), frame);
  cb_check_enable(sh,frame);

  check = gtk_check_button_new_with_label(_("Tasklist always maximum size"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(check), config.fixed_tasklist);
  gtk_signal_connect(GTK_OBJECT(check), "toggled",
		     GTK_SIGNAL_FUNC(cb_check), &o_config.fixed_tasklist);
  gtk_object_set_data(GTK_OBJECT(check), "prop", prop);
  gtk_box_pack_start(GTK_BOX(vbox1),check,FALSE,FALSE,0);
  
  adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)config.max_task_width,
					    20, 
					    (gfloat)gdk_screen_width(), 
					    16, 16, 16 );
  label = gtk_label_new(_("Maximum width of horizontal task list"));
  spin = gtk_spin_button_new(adj, 1, 0);
  hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(vbox1),hbox,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_box_pack_end(GTK_BOX(hbox),spin,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		     GTK_SIGNAL_FUNC(cb_adj), &o_config.max_task_width);
  gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);

  adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)config.max_task_vwidth,
					    4, 
					    (gfloat)gdk_screen_width(), 
					    4, 4, 4 );
  label = gtk_label_new(_("Maximum width of vertical task list"));
  spin = gtk_spin_button_new(adj, 1, 0);
  hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(vbox1),hbox,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_box_pack_end(GTK_BOX(hbox),spin,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		     GTK_SIGNAL_FUNC(cb_adj), &o_config.max_task_vwidth);
  gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);

  adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)config.task_rows_h, 1, 
					    8, 1, 1, 1 );
  label = gtk_label_new(_("Number of rows of horizontal tasks"));
  spin = gtk_spin_button_new(adj, 1, 0);
  hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(vbox1),hbox,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_box_pack_end(GTK_BOX(hbox),spin,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		     GTK_SIGNAL_FUNC(cb_adj), &o_config.task_rows_h);
  gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);

  adj = (GtkAdjustment *)gtk_adjustment_new((gfloat)config.task_rows_v, 1, 
					    4, 1, 1, 1 );
  label = gtk_label_new(_("Number of vertical columns of tasks"));
  spin = gtk_spin_button_new(adj, 1, 0);
  hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(vbox1),hbox,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_box_pack_end(GTK_BOX(hbox),spin,FALSE,FALSE,0);
  gtk_signal_connect(GTK_OBJECT(adj), "value_changed",
		     GTK_SIGNAL_FUNC(cb_adj), &o_config.task_rows_v);
  gtk_object_set_data(GTK_OBJECT(adj), "prop", prop);

  gtk_widget_show_all(prop);
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
  
  gdk_error_warnings = 0;
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
      if(format_ret==32)
        {
	   int i;
	   *size = num_ret * sizeof(unsigned long);
	   data = g_malloc(*size);
	   for(i=0;i<num_ret;i++)
	     ((unsigned long *)data)[i] = ((unsigned long *)retval)[i];
	}
      else if(format_ret==16)
        {
	   int i;
	   *size = num_ret * sizeof(unsigned short);
	   data = g_malloc(*size);
	   for(i=0;i<num_ret;i++)
	     ((unsigned short *)data)[i] = ((unsigned short *)retval)[i];
	}
      else /*format_ret==8*/
	{
          *size = num_ret;
          data = g_malloc(num_ret);
          if (data)
	    memcpy(data, retval, num_ret);
	}
      XFree(retval);
      return data;
    }
  if (retval)
    XFree (retval);
  return NULL;
}


/* CLIENT WINDOW manipulation functions */

void
client_win_kill(Task *t)
{
  gdk_error_warnings = 0;
  XKillClient(GDK_DISPLAY(), (XID)t->win);
}

void
client_win_close(Task *t)
{
  XClientMessageEvent ev;
  Atom                a1, a2, a3, *prop;
  unsigned long       lnum, ldummy;
  int                 num, i, del, dummy;
  
  gdk_error_warnings = 0;
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
  gdk_error_warnings = 0;
  XIconifyWindow(GDK_DISPLAY(), t->win, DefaultScreen(GDK_DISPLAY()));
  XSync(GDK_DISPLAY(), False);
}

void 
client_win_show(Task *t)
{
  Atom                a1, a2, a3, *prop;
  XClientMessageEvent ev;
  unsigned long       lnum, ldummy;
  int                 num, i, foc, dummy, x, y;
  
  gdk_error_warnings = 0;
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
  if (t->iconified)
    XMapWindow(GDK_DISPLAY(), t->win);
  if (!t->focused)
    XSetInputFocus(GDK_DISPLAY(), t->win, RevertToPointerRoot, CurrentTime);
  XRaiseWindow(GDK_DISPLAY(), t->win);
  if ((!t->sticky) && (t->desktop != current_desk) && (!t->iconified))
    gnome_win_hints_set_current_workspace(t->desktop);
  if (!t->iconified)
    {
      x = ((gdk_screen_width() * area_x) + (t->x + (t->w / 2)));
      y = ((gdk_screen_height() * area_y) + (t->y + (t->h / 2)));
      desktop_set_area(x / gdk_screen_width(), y / gdk_screen_height());
    }
  /***/
  XSync(GDK_DISPLAY(), False);
}

void 
client_win_stick(Task *t)
{
  XEvent              xev;
  Atom                a;

  gdk_error_warnings = 0;
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

  gdk_error_warnings = 0;
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

  gdk_error_warnings = 0;
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

  gdk_error_warnings = 0;
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
  GtkRequisition req;
  
  if (GTK_WIDGET_VISIBLE(widget))
    {
      gdk_pointer_ungrab(GDK_CURRENT_TIME);
      gtk_widget_hide(widget);
      gtk_grab_remove(widget);
    }
  else
    {
      gdk_window_get_pointer(NULL, &x, &y, NULL);
      gtk_widget_size_request(widget, &req);
      screen_width = gdk_screen_width();
      screen_height = gdk_screen_height();
      
      if ((applet_orient == ORIENT_UP) || (applet_orient == ORIENT_DOWN))
	{
	  x -= (req.width / 2);
	  if (y > screen_height / 2)
	    y -= (req.height + 2);
	  else
	    y += 2;
	}
      else
	{
	  y -= (req.height / 2);
	  if (x > screen_width / 2)
	    x -= (req.width + 2);
	  else
	    x += 2;
	}
      
      if ((x + req.width) > screen_width)
	x -= ((x + req.width) - screen_width);
      if (x < 0)
	x = 0;
      if ((y + req.height) > screen_height)
	y -= ((y + req.height) - screen_height);
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

int
showpop_cb(GtkWidget *widget, gpointer data)
{
  if(!popbox)
    {
      create_popbox();
      populate_tasks(TRUE);
    }

  custom_popbox_show(popbox);
  /*XXX: return false even though clicked is a void signal, but all the _event's
    need a return value*/
  return FALSE;
}

/* PROPERTY CHANGE info gathering callbacks */
void
cb_task_change(GtkWidget *widget, GdkEventProperty * ev, Task *t)
{
  gint i, tdesk;
  gchar tsticky;
  gchar iconified;

  tsticky = t->sticky;
  tdesk = t->desktop;
  iconified = t->iconified;
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
	  if (!config.tasks_all)
	    populate_tasks(FALSE);	  
	}
      else
	desktop_draw(t->desktop);
    }

  if (iconified != t->iconified)
      	populate_tasks(FALSE);

  set_task_info_to_button(t);
  widget = NULL;
}

void 
cb_root_prop_change(GtkWidget * widget, GdkEventProperty * ev)
{
  gint                desk, pdesk, i;
  GdkAtom             at;
  unsigned long      *da;
  gint                size;  

  gdk_error_warnings = 0;
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
      populate_tasks(FALSE);
      return;
    }
  
  at = gdk_atom_intern(XA_WIN_CLIENT_LIST, FALSE);
  if ((ev->atom == at) && (ev->state == GDK_PROPERTY_NEW_VALUE))
    {
      tasks_changed = 0;
      tasks_update();
      if (tasks_changed)
	populate_tasks(FALSE);
      tasks_changed = 0;
      return;
    }

  at = gdk_atom_intern(XA_WIN_WORKSPACE_COUNT, FALSE);
  if ((ev->atom == at) && (ev->state == GDK_PROPERTY_NEW_VALUE))
    {
      num_desk = gnome_win_hints_get_workspace_count();
      if (num_desk < 1)
	num_desk = 1;
      populate_tasks(FALSE);
      redo_interface();
      return;
    }
  
  at = gdk_atom_intern("_WIN_AREA_COUNT", FALSE);
  if ((ev->atom == at) && (ev->state == GDK_PROPERTY_NEW_VALUE))
    {
      da = util_get_atom(GDK_ROOT_WINDOW(), "_WIN_AREA_COUNT", XA_CARDINAL, &size);
      if (da)
	{
	  if (size == 2*sizeof(unsigned long))
	    {
	      area_w = da[0];
	      area_h = da[1];
	      if (area_w < 1)
		area_w = 1;
	      if (area_h < 1)
		area_h = 1;
	    }	  
	  g_free(da);
	}
      desktop_draw(current_desk);
    }
  at = gdk_atom_intern("_WIN_AREA", FALSE);
  if ((ev->atom == at) && (ev->state == GDK_PROPERTY_NEW_VALUE))
    {
      da = util_get_atom(GDK_ROOT_WINDOW(), "_WIN_AREA", XA_CARDINAL, &size);
      if (da)
	{
	  if (size == 2*sizeof(unsigned long))
	    {
	      area_x = da[0];
	      area_y = da[1];
	    }
	  g_free(da);
	}
      if (!config.tasks_all)
	populate_tasks(FALSE);
      desktop_draw(current_desk);
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
	  if (t->win == xevent->xconfigure.window ||
	      t->frame == xevent->xconfigure.window)
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
      if ((event->any.window) &&
	  (gdk_window_get_type(event->any.window) == GDK_WINDOW_FOREIGN))
        return GDK_FILTER_REMOVE;
      else
        return GDK_FILTER_CONTINUE;
      break;
    }
    return GDK_FILTER_REMOVE;
}

/* TASK manipulation functions */

void
task_get_info(Task *t)
{
  Window               ret;
  gchar               *name;
  gint                 size;
  guint                w, h, d;
  unsigned long       *val;
  Atom                 a;
  int                  rev;
  gchar               psticky;
  gchar               needredo = 0;

  gdk_error_warnings = 0;
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

  psticky = t->sticky;
  
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
  val = util_get_atom(t->win, "_WIN_STATE", XA_CARDINAL, &size);
  if (val)
    {
      if (*val & WIN_STATE_STICKY)
	t->sticky = 1;
      if (*val & WIN_STATE_SHADED)
	t->shaded = 1;
      g_free(val);
    }
  
  /* what desktop is it on ? */
  val = util_get_atom(t->win, "_WIN_WORKSPACE", XA_CARDINAL, &size);
  if (val)
    {
      t->desktop = *val;
      g_free(val);
    }
  
  /* geometry!!!!!!!!!!! */
  get_window_root_and_frame_id(t->win, &t->frame, &t->root);

  XGetGeometry(GDK_DISPLAY(), t->frame, &ret, &size, &size, &w, &h, &d, &d);
  t->w = (gint)w;
  t->h = (gint)h;
  XTranslateCoordinates(GDK_DISPLAY(), t->frame, t->root, 0, 0, 
			&(t->x), &(t->y), &ret);
  /* what area is it on ? */
  val = util_get_atom(t->win, "_WIN_AREA", XA_CARDINAL, &size);
  if (val)
    {
      gint pax, pay;
      
      pax = t->ax;
      pay = t->ay;
      t->ax = val[0];
      t->ay = val[1];
      g_free(val);
      
      if ((pax != t->ax) || (pay != t->ay))
	needredo = 1;
    }
  else
    {
      gint sw, sh;
      gint pax, pay;
      
      pax = t->ax;
      pay = t->ay;
      sw = gdk_screen_width();
      sh = gdk_screen_width();
      if (t->x > sw)
	t->ax = area_x + (t->x / sw);
      else if ((t->x + t->w) <= 0)
	t->ax = area_x + (t->x / sw) - 1;
      else
	t->ax = area_x;
      if (t->y > sh)
	t->ay = area_y + (t->y / sh);
      else if ((t->y + t->h) <= 0)
	t->ay = area_y + (t->y / sh) - 1;
      else
	t->ay = area_y;
      if (t->ax < 0)
	t->ax = 0;
      if (t->ay < 0)
	t->ay = 0;
      if (t->ax >= area_w)
	t->ax = area_w - 1;
      if (t->ay >= area_h)
	t->ay = area_h - 1;
      
      if ((pax != t->ax) || (pay != t->ay))
	needredo = 1;
    }
  if ((psticky != t->sticky) || (needredo))
    populate_tasks(FALSE);
}

gint
task_add(Window win)
{
  Task               *t;
  int                 size;
  gchar              *str;
  unsigned long      *val;
  GnomeUIInfo         uinfo[5] = 
    {
      GNOMEUIINFO_END,
      GNOMEUIINFO_END,
      GNOMEUIINFO_END,
      GNOMEUIINFO_END,
      GNOMEUIINFO_END
    };

  gdk_error_warnings = 0;
  /* has this task asked to be skipped by the task list ? */
  val = util_get_atom(win, "_WIN_HINTS", XA_CARDINAL, &size);
  if (val)
    {
      if ((*val) & WIN_HINTS_SKIP_TASKBAR)
	{
	  g_free(val);
	  return 0;
	}
      g_free(val);
    }

  /* create task struct */
  t = g_malloc(sizeof(Task));

  /* add this client to the list of clients */
  tasks = g_list_append(tasks, t);

  /* initialize members */
  t->win = win;
  t->x = 0;
  t->y = 0;
  t->w = -1;
  t->h = -1;
  t->ax = 0;
  t->ay = 0;
  t->name = NULL;
  t->iconified = 0;
  t->shaded = 0;
  t->focused = 0;
  t->sticky = 0;
  t->desktop = 0;
  /*t->dummy = NULL;*/
  t->gdkwin = NULL;
  t->frame_gdkwin = NULL;
  
  get_window_root_and_frame_id(t->win, &t->frame, &t->root);

  /* create dummy GTK widget to get events from */
  t->gdkwin = gdk_window_lookup(win);
  if (t->gdkwin)
    gdk_window_ref(t->gdkwin);
  else
    t->gdkwin = gdk_window_foreign_new(win);
/*  t->dummy = gtk_window_new(GTK_WINDOW_POPUP);*/
  /* realize that damn widget */
/*  gtk_widget_realize(t->dummy);*/
  gdk_window_add_filter(t->gdkwin, cb_filter_intercept, NULL/*t->dummy*/);  
  /* fake events form win producing signals on dummy widget */
/*  gdk_window_set_user_data(t->gdkwin, t->dummy);*/

  /* DO it all again for the frame window */
  t->frame_gdkwin = gdk_window_lookup(t->frame);
  if (t->frame_gdkwin)
    gdk_window_ref(t->frame_gdkwin);
  else
    t->frame_gdkwin = gdk_window_foreign_new(t->frame);
  gdk_window_add_filter(t->frame_gdkwin, cb_filter_intercept, NULL/*t->dummy*/);  

  /* make sure we get the events */
  if(!t->gdkwin || gdk_window_get_type(t->gdkwin) == GDK_WINDOW_FOREIGN)
    {
      XSelectInput(GDK_DISPLAY(), win, PropertyChangeMask | FocusChangeMask |
		   StructureNotifyMask);
    }
  else
    {
      /*if the window is ours don't make it select out some events only!*/
      XWindowAttributes attr;
      XGetWindowAttributes(GDK_DISPLAY(), win, &attr);
      XSelectInput(GDK_DISPLAY(), win,
		   attr.your_event_mask | PropertyChangeMask |
	           FocusChangeMask | StructureNotifyMask);
    }

  /* make sure we get the events */
  XSelectInput(GDK_DISPLAY(), t->frame, StructureNotifyMask);

  /* get info about task */
  task_get_info(t);
  return 1;
}


void 
task_delete(Task *t)
{
  guint               i, tdesk;
  gchar               tstick;
  
  g_return_if_fail(t!=NULL);

  gdk_error_warnings = 0;

  tasks = g_list_remove(tasks, t);
  tstick = t->sticky;
  tdesk = t->desktop;
  if (t->name)
    g_free(t->name);
  if (t->gdkwin)
    gdk_window_unref(t->gdkwin);
  if (t->frame_gdkwin)
    gdk_window_unref(t->frame_gdkwin);
  g_free(t);
  if (!tstick)
    desktop_draw(tdesk);
  else
    {
      for (i = 0; i < (guint)num_desk; i++)
        desktop_draw(i);
    }
}

Task *
task_find(Window win)
{
  Task               *t;
  GList              *ptr;
  
  for(ptr = tasks;ptr!=NULL; ptr=g_list_next(ptr))
    {
      t = ptr->data;
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
		  break;
		}
	    }
	  if (!there)
	    {
	      tasks_changed = 1;
	      /*XXX:although it will work in this case, removing stuff from
		a list while we are going through it is nuts and asking for
	        trouble*/
	      task_delete(t);
	    }
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
	      break;
	    }
	}
      if (!there)
	{
	  if (task_add(win[i]))
	    tasks_changed = 1;
	}
    }
}

void 
tasks_update(void)
{
  Window              *list;
  gint                 num, size;

  gdk_error_warnings = 0;
  list = util_get_atom(GDK_ROOT_WINDOW(), "_WIN_CLIENT_LIST", 
		       XA_CARDINAL, &size);
  
  if ((size > 0) && (list))
    {
      num = size / sizeof(Window);
      tasks_match(list, num);
      g_free(list);
    }
  else
    {
      tasks_match(NULL, 0);
    }
}

void 
get_desktop_names(void)
{
  GList *gl, *p;
  gint i;
  
  gdk_error_warnings = 0;
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

/* sesion save signal handler*/
static gint
cb_applet_save_session(GtkWidget *w,
		       const char *privcfgpath,
		       const char *globcfgpath)
{
  /*XXX: if any of this applies to pager stuff in general, push
    globcfgpath, and use a descriptive section name such as gnome_pager*/
  gnome_config_push_prefix(privcfgpath);
  gnome_config_set_int("stuff/pager_rows", config.pager_rows);
  gnome_config_set_int("stuff/pager_size", config.pager_size);
  gnome_config_set_int("stuff/tasks_all", config.tasks_all);
  gnome_config_set_int("stuff/minimized_tasks", config.minimized_tasks);
  gnome_config_set_int("stuff/minimized_tasks_all", config.minimized_tasks_all);
  gnome_config_set_int("stuff/minimized_tasks_only", config.minimized_tasks_only);
  gnome_config_set_int("stuff/task_rows_h", config.task_rows_h);
  gnome_config_set_int("stuff/task_rows_v", config.task_rows_v);
  gnome_config_set_int("stuff/max_task_width", config.max_task_width);
  gnome_config_set_int("stuff/max_task_vwidth", config.max_task_vwidth);
  gnome_config_set_int("stuff/show_tasks", config.show_tasks);
  gnome_config_set_int("stuff/show_pager", config.show_pager);
  gnome_config_set_int("stuff/show_icons", config.show_icons);
  gnome_config_set_int("stuff/show_arrow", config.show_arrow);
  gnome_config_set_int("stuff/fixed_tasklist", config.fixed_tasklist);
  gnome_config_set_int("stuff/pager_w_0", config.pager_w_0);
  gnome_config_set_int("stuff/pager_h_0", config.pager_h_0);
  gnome_config_set_int("stuff/pager_w_1", config.pager_w_1);
  gnome_config_set_int("stuff/pager_h_1", config.pager_h_1);
  gnome_config_pop_prefix();
  gnome_config_sync();
  gnome_config_drop_all();
  return FALSE;
}

/* select on events for the root window properies */
void
select_root_properties(void)
{
  GtkWidget *dummy_win;
  
  gdk_error_warnings = 0;
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
  GtkWidget *dlg;
  gint i, size;
  unsigned long *da;
  
  /* Initialize the i18n stuff */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  for (i = 0; i < 32; i++)
    {
      desk_name[i] = NULL;
      desk_widget[i] = NULL;
    }
  
  applet_widget_init("gnomepager_applet", VERSION, argc, argv, NULL, 
		     0, NULL);
  gdk_error_warnings = 0;
  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());
  gnome_win_hints_init();
  if (!gnome_win_hints_wm_exists())
    {
      GtkWidget *d, *l;
      
      d = gnome_error_dialog(_("Gnome Pager Error"));
      l = gtk_label_new(_("You are not running a GNOME Compliant Window Manager.\n"
			  "Please run a GNOME Compliant Window Manager.\n"
			  "For Example:\n"
			  "Enlightenment (DR-0.15)\n"
			  "Then start this applet again.\n"));
      gtk_widget_show(l);
      gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(d)->vbox), l, TRUE, TRUE, 5);
      gnome_dialog_run(GNOME_DIALOG(d));
      
      gtk_exit(1);
    }

  /*FIXME: remove this later!!!!!, in favour of the WELL BEHAVED
    session saving/loading, leave it in for now so that people that
    have some saved config get it*/
  config.pager_rows = gnome_config_get_int("gnome_pager/stuff/pager_rows=2");
  config.pager_size = gnome_config_get_int("gnome_pager/stuff/pager_size=0");
  config.tasks_all = gnome_config_get_int("gnome_pager/stuff/tasks_all=0");
  config.task_rows_h = gnome_config_get_int("gnome_pager/stuff/task_rows_h=2");
  config.task_rows_v = gnome_config_get_int("gnome_pager/stuff/task_rows_v=1");
  config.max_task_width = gnome_config_get_int("gnome_pager/stuff/max_task_width=400");
  config.max_task_vwidth = gnome_config_get_int("gnome_pager/stuff/max_task_vwidth=48");
  config.show_tasks = gnome_config_get_int("gnome_pager/stuff/show_tasks=1");
  config.show_pager = gnome_config_get_int("gnome_pager/stuff/show_pager=1");
  config.show_icons = gnome_config_get_int("gnome_pager/stuff/show_icons=1");
  config.show_arrow = gnome_config_get_int("gnome_pager/stuff/show_arrow=1");
  config.pager_w_0 = gnome_config_get_int("gnome_pager/stuff/pager_w_0=31");
  config.pager_h_0 = gnome_config_get_int("gnome_pager/stuff/pager_h_0=22");
  config.pager_w_1 = gnome_config_get_int("gnome_pager/stuff/pager_w_1=62");
  config.pager_h_1 = gnome_config_get_int("gnome_pager/stuff/pager_h_1=44");

  /*make sure these are not done next time*/
  gnome_config_clean_file("gnome_pager");

  /*need to create the applet widget before we can get config data, so
    that we know where to get them from*/
  applet = applet_widget_new("gnomepager_applet");
  if (!applet)
    {
      g_error("Can't create applet!\n");
      exit(1);
    }
  gtk_signal_connect(GTK_OBJECT(applet), "change_orient",
		     GTK_SIGNAL_FUNC(cb_applet_orient_change),
		     NULL);
  gtk_signal_connect(GTK_OBJECT(applet), "save_session",
		     GTK_SIGNAL_FUNC(cb_applet_save_session),
		     NULL);

  if (!hold_box)
    {
      hold_box = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);

      main_box = gtk_hbox_new(0,0);
      gtk_container_add(GTK_CONTAINER(hold_box), main_box);

      /*XXX: a hackish way to bind events on these buttons so that
	we will recieve the right click and such events over them to
	display menus and allow dragging over them */
      prop_button = gtk_button_new_with_label(_("?"));
      gtk_widget_ref(prop_button);
      gtk_widget_show(prop_button);
      gtk_box_pack_start_defaults(GTK_BOX(main_box),prop_button);
      arrow_button = gtk_button_new();
      gtk_widget_ref(arrow_button);
      gtk_widget_show(arrow_button);
      gtk_box_pack_start_defaults(GTK_BOX(main_box),arrow_button);

      gtk_signal_connect(GTK_OBJECT(prop_button), "clicked",
			 GTK_SIGNAL_FUNC(cb_applet_properties), NULL);
      gtk_signal_connect(GTK_OBJECT(arrow_button), "clicked",
			 GTK_SIGNAL_FUNC(showpop_cb), NULL);

      applet_widget_add(APPLET_WIDGET(applet), hold_box);
      gtk_widget_show(hold_box);
    }

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

  /*this really loads the correct data*/
  gnome_config_push_prefix(APPLET_WIDGET(applet)->privcfgpath);
  config.pager_rows = gnome_config_get_int("stuff/pager_rows=2");
  config.pager_size = gnome_config_get_int("stuff/pager_size=1");
  config.tasks_all = gnome_config_get_int("stuff/tasks_all=0");
  config.minimized_tasks = gnome_config_get_int("stuff/minimized_tasks=1");
  config.minimized_tasks_all = gnome_config_get_int("stuff/minimized_tasks_all=0");
  config.minimized_tasks_only = gnome_config_get_int("stuff/minimized_tasks_only=0");
  config.task_rows_h = gnome_config_get_int("stuff/task_rows_h=2");
  config.task_rows_v = gnome_config_get_int("stuff/task_rows_v=1");
  config.max_task_width = gnome_config_get_int("stuff/max_task_width=400");
  config.max_task_vwidth = gnome_config_get_int("stuff/max_task_vwidth=48");
  config.show_tasks = gnome_config_get_int("stuff/show_tasks=1");
  config.show_pager = gnome_config_get_int("stuff/show_pager=1");
  config.show_icons = gnome_config_get_int("stuff/show_icons=1");
  config.show_arrow = gnome_config_get_int("stuff/show_arrow=1");
  config.fixed_tasklist = gnome_config_get_int("stuff/fixed_tasklist=0");

  gdk_error_warnings = 0;  
  get_desktop_names();
  current_desk = gnome_win_hints_get_current_workspace();
  num_desk = gnome_win_hints_get_workspace_count();
  if (num_desk < 1)
    num_desk = 1;
  gdk_error_warnings = 0;
  da = util_get_atom(GDK_ROOT_WINDOW(), "_WIN_AREA_COUNT", XA_CARDINAL, &size);
  if (da)
    {
      if (size == 2*sizeof(unsigned long))
	{
	  area_w = da[0];
	  area_h = da[1];
	  if (area_w < 1)
	    area_w = 1;
	  if (area_h < 1)
	    area_h = 1;
	}
      g_free(da);
    }
  gdk_error_warnings = 0;
  da = util_get_atom(GDK_ROOT_WINDOW(), "_WIN_AREA", XA_CARDINAL, &size);
  if (da)
    {
      if (size == 2*sizeof(unsigned long))
	{
	  area_x = da[0];
	  area_y = da[1];
	}
      g_free(da);
    }
  gdk_error_warnings = 0;
  tasks_update();
  select_root_properties();

  gdk_imlib_data_to_pixmap(icon1_xpm, &p_1, &m_1);
  gdk_imlib_data_to_pixmap(icon2_xpm, &p_2, &m_2);
  gdk_imlib_data_to_pixmap(icon3_xpm, &p_3, &m_3);

  /* advertise to a WM that we exist and are here */
    {
      GdkAtom atom;
      GdkWindow *win;
      GdkWindowAttr attr;
      Window ww;
      
      atom = gdk_atom_intern("_GNOME_PAGER_ACTIVE", FALSE);
      attr.window_type = GDK_WINDOW_TEMP;
      attr.wclass = GDK_INPUT_OUTPUT;
      attr.x = -99;
      attr.y = -99;
      attr.width = 88;
      attr.height = 88;
      attr.event_mask = 0;

      win = gdk_window_new(NULL, &attr, 0);
      ww = GDK_WINDOW_XWINDOW(win);
      gdk_property_change(GDK_ROOT_PARENT(), atom, (GdkAtom)XA_WINDOW, 32, 
			  GDK_PROP_MODE_REPLACE, (guchar *)(&ww), 1);
      gdk_property_change(win, atom, (GdkAtom)XA_WINDOW, 32, 
			  GDK_PROP_MODE_REPLACE, (guchar *)(&ww), 1);
    }
  applet_widget_gtk_main();
  return 0;
}

void 
desktop_draw(gint i)
{
  if ((i >= 0) && (i < 32) && (desk_widget[i]))
    desktop_cb_redraw(desk_widget[i], NULL);
}

gboolean
desktop_cb_button_down(GtkWidget * widget, GdkEventButton *event)
{
  gint desk, selm, button;
  gint x, y, w, h;
  
  button = event->button;  
  desk = GPOINTER_TO_INT (gtk_object_get_data(GTK_OBJECT(widget), "desktop"));
  
  if (button == 1) {
    gnome_win_hints_set_current_workspace(desk);
    
    w = widget->allocation.width - 4;
    h = widget->allocation.height - 4;
    x = (((gint)event->x * area_w) - 2) / w;
    y = (((gint)event->y * area_h) - 2) / h;
    if (x >= area_w)
      x = area_w - 1;
    if (x < 0)
      x = 0;
    if (y >= area_h)
      y = area_h - 1;
    if (y < 0)
      y = 0;
    desktop_set_area(x, y);

    return TRUE;
  }

  return FALSE;
}

gboolean
desktop_cb_button_up(GtkWidget * widget, GdkEventButton *event)
{
  gint desk, selm, button;

  if (event->button == 1) {
    button = event->button;  
    desk = GPOINTER_TO_INT (gtk_object_get_data(GTK_OBJECT(widget), "desktop"));
  }

  return TRUE;
}

void
desktop_set_area(int ax, int ay)
{
  XEvent xev;
  
  gdk_error_warnings = 0;
  xev.type = ClientMessage;
  xev.xclient.type = ClientMessage;
  xev.xclient.window = GDK_ROOT_WINDOW();
  xev.xclient.message_type = XInternAtom(GDK_DISPLAY(), "_WIN_AREA", False);
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = ax;
  xev.xclient.data.l[1] = ay;

  XSendEvent(GDK_DISPLAY(), GDK_ROOT_WINDOW(), False,
	               SubstructureNotifyMask, (XEvent*) &xev);
}


void
get_window_root_and_frame_id(Window w, Window *ret_frame, Window *ret_root)
{
  Window               w3, w2, ret, root, *wl;
  gint                 size;
  unsigned long       *val;

  w3 = w2 = w;

  while (XQueryTree(GDK_DISPLAY(), w2, &root, &ret, &wl, &size))
    {
      if ((wl) && (size > 0))
	XFree(wl);
      w3 = w2;
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

  *ret_root = w2;
  *ret_frame = w3;

  return;
}



GList  *get_task_stacking(gint desk)
{
  GList *tasks_on_desk=NULL;
  GList *root_ids=NULL;
  GList *retval=NULL;
  GList *p, *p2, *p3;
  Task *t;
  Window dummy1, dummy2, root, *wl;
  gint size, count;

  /* Find the tasks that are sticky or on the specified desk 
     which are not iconified. */

  for (p = tasks; p != NULL; p = p->next) {
    t = (Task *)p->data;
    
    if (((t->desktop == desk) || (t->sticky)) && (!(t->iconified))) {
      tasks_on_desk = g_list_prepend(tasks_on_desk, (gpointer) t);
    }
  }
  tasks_on_desk = g_list_reverse(tasks_on_desk);

  /* Find the IDs of all root or virtual root windows they 
     may be on. */

  for (p = tasks_on_desk; p != NULL; p = p->next) {
    t = (Task *)p->data;

    if (NULL == g_list_find(root_ids, (gpointer) t->root)) {
      root_ids = g_list_prepend(root_ids, (gpointer) t->root);
    }
  }

  /* XQueryTree each virtual root window, and for each window ID we
     get back that matches a frame window, add the corresponding task
     structure to retval. If we have tasks claiming to be on the same
     desk which are on different "root" windows, the stacking order
     between windows on the different roots will be aribtrary, but
     that is a WM bug (or temporary state) anyway. */
  
  for (p = root_ids; p != NULL; p = p->next) {
    root = (Window) p->data;
    
    wl = NULL;
    size = 0;
    if (XQueryTree(GDK_DISPLAY(), root, &dummy1, &dummy2, &wl, &size))
      {
	for (count = 0; count < size; count++) {
	  Window w = wl[count];
	  
	  for (p2 = tasks_on_desk; p2 != NULL; p2 = p2->next) {
	    t = (Task *)p2->data;
	    
	    if (t->frame == wl[count]) {
	      tasks_on_desk=g_list_remove_link(tasks_on_desk, p2);
	      
	      if (retval)
		      retval->prev = p2;
	      p2->next = retval;
	      p2->prev = NULL;
	      retval = p2;
	      break;
	    }
	  }
	}
	if ((count > 0) && (wl))
	  XFree(wl);
      }
  }
  retval = g_list_reverse(retval);

  g_list_free(tasks_on_desk);
  g_list_free(root_ids);

  return retval;
}


#include "stripe.xbm"

int
actual_redraw(gpointer data)
{
  GtkWidget *widget = data;
  guint current_timeout;
  GtkStyle *s;
  GdkWindow *win;
  gint sel, sw, sh, w, h, desk, x, y, ww, hh;
  Task *t;
  GList *p, *stacking;
  GdkBitmap *bm;
  GdkGC *gc;

  if (!widget)
    return TRUE;
  if (!widget->window)
    return TRUE;
  if (!widget->style)
    return TRUE;
  if (!(config.show_pager))
    return TRUE;
  desk = GPOINTER_TO_INT (gtk_object_get_data(GTK_OBJECT(widget), "desktop"));
  /* FIXME: sel is currently always zero */
  sel = GPOINTER_TO_INT (gtk_object_get_data(GTK_OBJECT(widget), "select"));

  if (config.pager_size)
    {
      if (widget->allocation.width != (area_w * config.pager_w_0))
	gtk_widget_set_usize(widget, (area_w * config.pager_w_0), 
			 widget->allocation.height);
      if (widget->allocation.height != (area_h * config.pager_h_0))
	gtk_widget_set_usize(widget, widget->allocation.width, 
			 (area_h * config.pager_h_0));
    }
  else
    {
      if (widget->allocation.width != (area_w * config.pager_w_1))
	gtk_widget_set_usize(widget, (area_w * config.pager_w_1), 
			 widget->allocation.height);
      if (widget->allocation.height != (area_h * config.pager_h_1))
	gtk_widget_set_usize(widget, widget->allocation.width, 
			 (area_h * config.pager_h_1));
    }
  w = widget->allocation.width - 4;
  h = widget->allocation.height - 4;
  s = widget->style;
  win = widget->window;
  sw = gdk_screen_width();
  sh = gdk_screen_height();

  gc = NULL;
  if (desk == current_desk)
    {
      gc = s->bg_gc[GTK_STATE_SELECTED];
      gtk_draw_box(s, win,GTK_STATE_ACTIVE, GTK_SHADOW_IN, 0, 0, -1, -1);
    }
  else
    {
      gc = s->base_gc[GTK_STATE_SELECTED];
      gtk_draw_box(s, win,GTK_STATE_ACTIVE, GTK_SHADOW_OUT, 0, 0, -1, -1);
    }
  
  bm = gdk_bitmap_create_from_data(win,stripe_bits, stripe_width, stripe_height);
  gdk_gc_set_clip_mask(gc, NULL);
  gdk_gc_set_fill(gc, GDK_STIPPLED);
  gdk_gc_set_stipple(gc, bm);
  gdk_gc_set_ts_origin(gc, 0, 0);

  x = 2 + ((area_x * w) / area_w);
  y = 2 + ((area_y * h) / area_h);
  gdk_draw_rectangle(win,gc, TRUE, 
		     x, y, w / area_w, h / area_h); 

  gdk_gc_set_stipple(gc, NULL);
  gdk_gc_set_fill(gc, GDK_SOLID);
  gdk_pixmap_unref(bm);
  
  for (x = 1; x < area_w; x++)
    gtk_draw_vline(s, win,GTK_STATE_NORMAL, 2, 1 + h, 1 + ((x * w) / area_w));
  for (y = 1; y < area_h; y++)
    gtk_draw_hline(s, win,GTK_STATE_NORMAL, 2, 1 + w, 1 + ((y * h) / area_h));
  stacking = get_task_stacking(desk);
  p = stacking;
  while (p)
    {
      t = (Task *)p->data;
      if (((t->desktop == desk) || (t->sticky)) && (!(t->iconified)))
	{
	  x = 2 + ((t->x + (area_x * gdk_screen_width())) * w) / (sw * area_w);
	  ww = (t->w * w) / (sw * area_w);
	  y = 2 +((t->y + (area_y * gdk_screen_height()))* h) / (sh * area_h);
	  hh = (t->h * h) / (sh * area_h);
	  if (t->focused)
	    gtk_draw_box(s, win,GTK_STATE_PRELIGHT, GTK_SHADOW_OUT, 
			 x, y, ww, hh);
	  else
	    gtk_draw_box(s, win,GTK_STATE_NORMAL, GTK_SHADOW_ETCHED_IN, 
			 x, y, ww, hh);
	    
	}
      p = p->next;
    }
  g_list_free(stacking);
  
  current_timeout =
    GPOINTER_TO_INT (gtk_object_get_data(GTK_OBJECT(widget), "timeout"));
  gtk_object_set_data(GTK_OBJECT(widget), "timeout", 0);
  return FALSE;
}

int
desktop_cb_redraw(GtkWidget *widget, gpointer data)
{
  guint current_timeout;
  
  current_timeout = 
    GPOINTER_TO_INT (gtk_object_get_data(GTK_OBJECT(widget), "timeout"));
  if (current_timeout)
    gtk_timeout_remove(current_timeout);
  current_timeout = gtk_timeout_add(200, 
				    actual_redraw, 
				    widget);
  gtk_object_set_data(GTK_OBJECT(widget), "timeout", GINT_TO_POINTER(current_timeout));
  return FALSE;
}

void
cb_desk_destroy(GtkWidget *widget, gpointer data)
{
  gint current_timeout;
  
  current_timeout =
    GPOINTER_TO_INT (gtk_object_get_data(GTK_OBJECT(widget), "timeout"));
  if (current_timeout)
    gtk_timeout_remove(current_timeout);
}

GtkWidget *
make_desktop_pane(gint desktop, gint width, gint height)
{
  GtkWidget *area, *tip;
  gchar *s;
  
  area = gtk_drawing_area_new();
  
  if ((applet_orient == ORIENT_UP) || (applet_orient == ORIENT_DOWN))
    width = (width * area_w) / area_h;
  else
    height = (height * area_h) / area_w;
  gtk_drawing_area_size(GTK_DRAWING_AREA(area), width, height);
  gtk_widget_set_usize(area, width, height);
  gtk_object_set_data(GTK_OBJECT(area), "desktop", GINT_TO_POINTER (desktop));
  gtk_object_set_data(GTK_OBJECT(area), "select", GINT_TO_POINTER (0));
  gtk_widget_set_events(area, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			GDK_EXPOSURE_MASK | GDK_ENTER_NOTIFY_MASK |
			GDK_LEAVE_NOTIFY_MASK);
  gtk_signal_connect(GTK_OBJECT(area), "button_press_event",
		     GTK_SIGNAL_FUNC(desktop_cb_button_down), NULL);
  gtk_signal_connect(GTK_OBJECT(area), "button_release_event",
		     GTK_SIGNAL_FUNC(desktop_cb_button_up), NULL);
  gtk_signal_connect(GTK_OBJECT(area), "expose_event",
		     GTK_SIGNAL_FUNC(desktop_cb_redraw), NULL);
  gtk_signal_connect(GTK_OBJECT(area), "destroy",
		     GTK_SIGNAL_FUNC(cb_desk_destroy), NULL);
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
create_popbox(void)
{
  GtkWidget *widg, *widg2, *hb, *hbox;
  gint i;
  
  popbox = gtk_window_new(GTK_WINDOW_POPUP);
  gtk_window_set_policy(GTK_WINDOW(popbox), 0, 0, 1);
  gtk_widget_set_events(popbox, GDK_BUTTON_PRESS_MASK | 
			GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK |
			GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK);
  gtk_signal_connect(GTK_OBJECT(popbox), "button_press_event",
		     GTK_SIGNAL_FUNC(showpop_cb), NULL);
  gtk_signal_connect(GTK_OBJECT(popbox), "button_release_event",
		     GTK_SIGNAL_FUNC(showpop_cb), NULL);

  popbox_q = hb = gtk_frame_new(NULL);
  gtk_widget_show(hb);
  gtk_container_add(GTK_CONTAINER(popbox), hb);
  gtk_frame_set_shadow_type(GTK_FRAME(hb), GTK_SHADOW_OUT);

  hbox = gtk_hbox_new(FALSE, 1);
  gtk_widget_show(hbox);
  gtk_container_add(GTK_CONTAINER(hb), hbox);

  if (blists)
    g_free(blists);
  if (flists)
    g_free(flists);
  blists = NULL;
  blists_num = 0;
  flists = NULL;
  blists = g_malloc(sizeof(GtkWidget *) * (num_desk + 1));
  flists = g_malloc(sizeof(GtkWidget *) * (num_desk + 1));
  blists_num = num_desk + 1;
  for (i = 0; i < num_desk + 1; i++)
    {
      blists[i] = NULL;
      flists[i] = NULL;
    }
  for(i = 0; i < num_desk + 1; i++)
    {
      if (i == 0)
	  widg = gtk_frame_new(_("Sticky"));
      else
	{
	  widg = gtk_frame_new(desk_name[i - 1]);
	}
      flists[i] = widg;
      gtk_widget_show(widg);
      gtk_frame_set_label_align(GTK_FRAME(widg), 0.5, 0.5);
      gtk_box_pack_start(GTK_BOX(hbox), widg, FALSE, FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER(widg), 4);
      widg2 = gtk_alignment_new(0.5, 0.0, 0.0, 0.0);
      gtk_widget_show(widg2);
      gtk_container_add(GTK_CONTAINER(widg), widg2);
      blists[i] = gtk_vbox_new(FALSE, 0);
      gtk_widget_show(blists[i]);
      gtk_container_add(GTK_CONTAINER(widg2), blists[i]);
    }
}

void
init_applet_gui(int horizontal)
{
  GtkWidget *frame, *arrow, *table;
  GtkWidget *desk, *align, *box;
  gint i, j, k;
  
  if (main_box)
    return;
  if (horizontal)
    main_box = gtk_hbox_new(FALSE, 0);
  else
    main_box = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(main_box);
  gtk_container_add(GTK_CONTAINER(hold_box), main_box);

  align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
  if (config.show_pager)
    gtk_widget_show(align);
  gtk_box_pack_start(GTK_BOX(main_box), align, FALSE, FALSE, 0);
  
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
      if (config.pager_size)
	desk = make_desktop_pane(i, config.pager_w_0, config.pager_h_0);
      else
	desk = make_desktop_pane(i, config.pager_w_1, config.pager_h_1);
      desk_widget[i] = desk;
      gtk_widget_show(desk);
      gtk_table_attach_defaults(GTK_TABLE(table), desk,
				j, j + 1, k, k + 1);
      j++;
      if (horizontal?
	  (j >= num_desk / config.pager_rows):
	  (j >= config.pager_rows))
	{
	  j = 0;
	  k ++;
	}
    }

  if (config.show_arrow) 
    {
      if(horizontal)
        box = gtk_vbox_new(FALSE, 0);
      else
        box = gtk_hbox_new(FALSE, 0);
      gtk_widget_show(box);

      switch(applet_orient)
        {
	 case ORIENT_UP:
	  arrow = gtk_arrow_new(GTK_ARROW_UP, GTK_SHADOW_OUT);
	  gtk_box_pack_start(GTK_BOX(box), arrow_button, FALSE, FALSE, 0);	
	  gtk_box_pack_start(GTK_BOX(box), prop_button, TRUE, TRUE, 0);
	  break;
	 case ORIENT_DOWN:
	  arrow = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_OUT);
	  gtk_box_pack_start(GTK_BOX(box), prop_button, TRUE, TRUE, 0);
	  gtk_box_pack_start(GTK_BOX(box), arrow_button, FALSE, FALSE, 0);
	  break;
	 case ORIENT_LEFT:
	  arrow = gtk_arrow_new(GTK_ARROW_LEFT, GTK_SHADOW_OUT);
	  gtk_box_pack_start(GTK_BOX(box), arrow_button, FALSE, FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(box), prop_button, TRUE, TRUE, 0);
	  break;
	 case ORIENT_RIGHT:
	  arrow = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_OUT);
	  gtk_box_pack_start(GTK_BOX(box), prop_button, TRUE, TRUE, 0);
	  gtk_box_pack_start(GTK_BOX(box), arrow_button, FALSE, FALSE, 0);
	  break;
	 default:
	  arrow = box = NULL; /*shut up warning*/
	  g_assert_not_reached();
	  break;
	}

     gtk_widget_show(arrow);
     gtk_container_add(GTK_CONTAINER(arrow_button), arrow);

     gtk_box_pack_start(GTK_BOX(main_box), box, FALSE, FALSE, 0);
    }
  
  frame = gtk_frame_new(NULL);
  gtk_box_pack_start(GTK_BOX(main_box), frame, FALSE, FALSE, 0);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  if (config.show_tasks)
    gtk_widget_show(frame);

  table = gtk_table_new(1, 1, TRUE);
  gtk_widget_show(table);
  if(config.fixed_tasklist)
    {
      if (horizontal)
        box = gtk_hbox_new(0,0);
      else
        box = gtk_vbox_new(0,0);
      gtk_widget_show(box);
      if (horizontal)
	gtk_widget_set_usize(box,config.max_task_width,-1);
      else
	gtk_widget_set_usize(box,-1,config.max_task_width);
      gtk_container_add(GTK_CONTAINER(frame), box);
      gtk_box_pack_start(GTK_BOX(box),table,FALSE,FALSE,0);
    }
  else
    gtk_container_add(GTK_CONTAINER(frame), table);
  task_table = table;
  
  emtpy_task_widgets();
  populate_tasks(FALSE);
}

gboolean
task_cb_button_enter(GtkWidget * widget, GdkEventCrossing *event)
{
  Task *t;
  
  t = (Task *)gtk_object_get_data(GTK_OBJECT(widget), "task");
  set_task_info_to_button(t);

  return TRUE;
}

gboolean
task_cb_button_leave(GtkWidget * widget, GdkEventCrossing *event)
{
  Task *t;
  
  t = (Task *)gtk_object_get_data(GTK_OBJECT(widget), "task");
  set_task_info_to_button(t);

  return TRUE;
}

gboolean
task_cb_button_down(GtkWidget * widget, GdkEventButton *event)
{
  gint button;
  Task *t;
  
  button = event->button;
  if ((button == 1) || (button == 2))
    {
      t = (Task *)gtk_object_get_data(GTK_OBJECT(widget), "task");
      set_task_info_to_button(t);
      return TRUE;
    }
  return FALSE;
}

gboolean
task_cb_button_up(GtkWidget * widget, GdkEventButton *event)
{
  gint button;
  Task *t;
  
  button = event->button;  
  t = (Task *)gtk_object_get_data(GTK_OBJECT(widget), "task");
  set_task_info_to_button(t);
  if (button == 1)
    {
      client_win_show(t);
      return TRUE;
    }
  else if (button == 2)
    {
      if (t->iconified)
	client_win_show(t);
      else
	client_win_iconify(t);
      return TRUE;
    }
  return FALSE;
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
cb_stick(GtkWidget * widget, Task *t)
{
  if (t->sticky)
    client_win_unstick(t);
  else
    client_win_stick(t);
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
      gtk_widget_destroy((GtkWidget *)(p->data));
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
  gint num, mw;
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
      if (config.show_icons)
	{
	  if (icon1)
	    gtk_widget_hide(icon1);
	  if (icon2)
	    gtk_widget_hide(icon2);
	  if (icon3)
	    gtk_widget_show(icon3);
	}
      gtk_widget_set_state(button, GTK_STATE_NORMAL);
    }
  else
    {
      if (t->focused)
	{
#ifdef ANIMATION      
	  gnome_animator_start(GNOME_ANIMATOR(icon2));
#endif
	  if (config.show_icons)
	    {
	      if (icon3)
		gtk_widget_hide(icon3);
	      if (icon1)
		gtk_widget_hide(icon1);
	      if (icon2)
		gtk_widget_show(icon2);
	    }
	  gtk_widget_set_state(button, GTK_STATE_ACTIVE);
	}
      else
	{
#ifdef ANIMATION      
	  gnome_animator_stop(GNOME_ANIMATOR(icon2));
#endif	  
	  if (config.show_icons)
	    {
	      if (icon3)
		gtk_widget_hide(icon3);
	      if (icon2)
		gtk_widget_hide(icon2);
	      if (icon1)
		gtk_widget_show(icon1);
	    }
	  gtk_widget_set_state(button, GTK_STATE_NORMAL);
	}
    }
  num = 0;
  p = tasks;
  while (p)
    {
      Task *t = p->data;
      if ((config.tasks_all || 
	  (t->sticky) || 
	  (config.minimized_tasks_all && t->iconified) ||
	  (
	   (t->desktop == current_desk)
	  && (t->ax == area_x)
	  && (t->ay == area_y)
	   )
	  ) && ((config.minimized_tasks_only && t->iconified) ||
	      	(!config.minimized_tasks_only)
	  ) && (!t->iconified || config.minimized_tasks || config.minimized_tasks_only))
	num++;
      p = p->next;
    }

  if ((label) && (t->name))
    gtk_label_set_text (GTK_LABEL(label), t->name);
  else if (label)
    gtk_label_set_text (GTK_LABEL(label), "???");
  gtk_widget_size_request(button, &req);
  if ((applet_orient == ORIENT_UP) || (applet_orient == ORIENT_DOWN))
    {
       if (num < config.task_rows_h) 
	 num = config.task_rows_h;
      mw = config.max_task_width / ((num + config.task_rows_h - 1) / config.task_rows_h);
    }
  else
    {
      mw = config.max_task_vwidth;
    }
  
    /*if the size is too much and we can reduce it, do it*/
    /*FIXME: this  won't work with i18n (with multibyte characters)
      properly*/
    if(req.width > mw && label && (!t->name || *t->name)) {
	    int len;
	    char *str;
	    int button_width;

	    gtk_label_set_text (GTK_LABEL(label), "");
	    gtk_widget_size_request(button, &req);

	    if (t->name)
		    len = strlen(t->name);
	    else
		    len = 3; /*strlen("???");*/
	    
	    len--;
	    str = g_malloc(len+4);
	    if(t->name)
		    strcpy(str,t->name);
	    else
		    strcpy(str,"???");
	    strcat(str,"..");
	    for(;len>0;len--) {
		    int label_len;
		    str[len]='.';
		    str[len+3]='\0';

		    label_len = gdk_string_width
			    (GTK_WIDGET (label)->style->font, str);
		    if(label_len+req.width<=mw)
			    break;
	    }
	    if(len<=0)
		    gtk_label_set_text (GTK_LABEL(label), "");
	    else
		    gtk_label_set_text (GTK_LABEL(label), str);
	    g_free(str);
    }
}

static gint
popup_button_pressed (GtkWidget *widget, GdkEventButton *event, Task *t)
{
	GtkWidget *popup;
	if (event->button != 3)
		return FALSE;
	
	popup = gtk_object_get_data(GTK_OBJECT(widget),"popup");
	if(!popup) {
		GtkWidget *item;
		popup = gtk_menu_new();

		item = gtk_menu_item_new_with_label(_("Show / Hide"));
		gtk_widget_show(item);
		gtk_menu_append(GTK_MENU(popup),item);
		gtk_signal_connect(GTK_OBJECT(item),"activate",
				   GTK_SIGNAL_FUNC(cb_showhide),t);

		item = gtk_menu_item_new_with_label(_("Shade / Unshade"));
		gtk_widget_show(item);
		gtk_menu_append(GTK_MENU(popup),item);
		gtk_signal_connect(GTK_OBJECT(item),"activate",
				   GTK_SIGNAL_FUNC(cb_shade),t);

		item = gtk_menu_item_new_with_label(_("Stick / Unstick"));
		gtk_widget_show(item);
		gtk_menu_append(GTK_MENU(popup),item);
		gtk_signal_connect(GTK_OBJECT(item),"activate",
				   GTK_SIGNAL_FUNC(cb_stick),t);

		item = gtk_menu_item_new_with_label(_("Close"));
		gtk_widget_show(item);
		gtk_menu_append(GTK_MENU(popup),item);
		gtk_signal_connect(GTK_OBJECT(item),"activate",
				   GTK_SIGNAL_FUNC(cb_kill),t);

		item = gtk_menu_item_new_with_label(_("Nuke"));
		gtk_widget_show(item);
		gtk_menu_append(GTK_MENU(popup),item);
		gtk_signal_connect(GTK_OBJECT(item),"activate",
				   GTK_SIGNAL_FUNC(cb_nuke),t);

		gtk_signal_connect_object(GTK_OBJECT(widget),"destroy",
					  GTK_SIGNAL_FUNC(gtk_widget_destroy),
					  GTK_OBJECT(popup));

		gtk_object_set_data(GTK_OBJECT(widget),"popup",popup);
	}
	gnome_popup_menu_do_popup (popup, NULL, NULL, event, NULL);

	return TRUE;
}


void
populate_tasks(int just_popbox)
{
   Task            *t;
   GList           *p;
   GtkWidget       *hbox, *button, *icon1, *icon2, *icon3, *label, *menu, *tip;
#ifdef ANIMATION
   GtkWidget       *f1, *f2, *f3, *f4;
#endif  
   gint             n_rows, n_cols, num, i, j, k;

  if (!task_table)
    return;

  if (!just_popbox && popbox && !GTK_WIDGET_VISIBLE(popbox))
    {
      gtk_widget_destroy(popbox);
      popbox_q = popbox = NULL;
      g_list_free(task_dest);
      task_dest = NULL;
    }

  if (just_popbox)
    {
      g_list_foreach(task_dest,(GFunc)gtk_widget_destroy,NULL);
      g_list_free(task_dest);
      task_dest = NULL;
    }
  else
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
      t = p->data;
      if (((config.tasks_all) || 
	  (t->sticky) || 
	  (config.minimized_tasks_all && t->iconified) ||
	  (
	   (t->desktop == current_desk)
	  && (t->ax == area_x)
	  && (t->ay == area_y)
	   )
	  ) && ((config.minimized_tasks_only && t->iconified) ||
	                     (!config.minimized_tasks_only)
	  ) && (!t->iconified || config.minimized_tasks || config.minimized_tasks_only))
	num++;
      if (popbox)
        {
	  button = gtk_button_new_with_label(t->name ? t->name : "");
	  gtk_widget_show(button);
	  gtk_signal_connect(GTK_OBJECT(button), "button_press_event",
			     GTK_SIGNAL_FUNC(task_cb_button_down), t);
	  gtk_signal_connect(GTK_OBJECT(button), "button_release_event",
			     GTK_SIGNAL_FUNC(task_cb_button_up), t);
	  gtk_signal_connect(GTK_OBJECT(button), "enter",
			     GTK_SIGNAL_FUNC(task_cb_button_enter), t);
	  gtk_signal_connect(GTK_OBJECT(button), "leave",
			     GTK_SIGNAL_FUNC(task_cb_button_leave), t);
	  if ((blists_num > 0) && (blists) && (t->sticky) && (blists[0]))
	    gtk_box_pack_start(GTK_BOX(blists[0]), button, TRUE, TRUE, 0);
	  else if ((blists_num > ((t->desktop % num_desk) + 1)) && (blists) && 
		   (t->desktop >= 0) && (t->desktop < num_desk) && 
		   (num_desk > 0) && (blists[(t->desktop % num_desk) + 1]))
	    gtk_box_pack_start(GTK_BOX(blists[(t->desktop % num_desk) + 1]), 
				     button, TRUE, TRUE, 0);
	  gtk_object_set_data(GTK_OBJECT(button), "task", t);
	  task_dest = g_list_append(task_dest, button);
	}
      
      p = p->next;
    }
  if (popbox_q)
    gtk_widget_queue_resize(popbox_q);

  if(just_popbox)
    return;

  p = tasks;
  while (p)
    {
      t = (Task *)p->data;
      

      if ( (((t->desktop == current_desk)
	     && (t->ax == area_x)
	     && (t->ay == area_y)
	    ) ||
	    (config.tasks_all) ||
	    (t->sticky) ||
	    (config.minimized_tasks_all && t->iconified)
	   ) && 
	      ((config.minimized_tasks_only && t->iconified) || 
	       (!config.minimized_tasks_only)
	   ) && (!t->iconified || config.minimized_tasks || config.minimized_tasks_only
	   ) && !just_popbox)
	{

	  hbox = gtk_hbox_new(0, FALSE);
	  gtk_widget_show(hbox);
	  
	  label = gtk_label_new(t->name?t->name:"???");
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

	  if (config.show_icons)
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
	  
	  gtk_signal_connect (GTK_OBJECT (button), "button_press_event",
			      (GtkSignalFunc) popup_button_pressed,
			      t);

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
		if (num < config.task_rows_h) 
		  num = config.task_rows_h;
	      j++;
	      if (n_cols < j)
		n_cols = j;
	      if (n_rows < k + 1)
		n_rows = k + 1;
	      if (j >= ((num + config.task_rows_h - 1) / config.task_rows_h))
		{
		  j = 0;
		  k++;
		}
	    }
	  else
	    {
	       if (num < config.task_rows_v) 
		 num = config.task_rows_v;
	      k++;
	      if (n_cols < j + 1)
		n_cols = j + 1;
	      if (n_rows < k)
		n_rows = k;
	      if (k >= ((num + config.task_rows_v - 1) / config.task_rows_v))
		{
		  k = 0;
		  j++;
		}
	    }
	}
      p = p->next;
    }
  gtk_table_resize(GTK_TABLE(task_table), n_rows, n_cols);
  if(config.fixed_tasklist && task_table->parent)
	  gtk_widget_queue_resize(task_table->parent);
}
