/* GNOME Desktop Guide
 * Copyright (C) 1999 Tim Janik
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
#include "deskguide_applet.h"
#include <config.h>	/* PACKAGE */
#include "gwmdesktop.h"



/* --- prototypes --- */
static void	gp_init_gui		(void);
static void	gp_destroy_gui		(void);
static void	gp_create_desk_widgets	(void);
static gboolean	gp_desk_notifier	(gpointer	    func_data,
					 GwmhDesk	   *desk,
					 GwmhDeskInfoMask   change_mask);
static gboolean	gp_task_notifier	(gpointer	    func_data,
					 GwmhTask	   *task,
					 GwmhTaskNotifyType ntype,
					 GwmhTaskInfoMask   imask);
static void	gp_about		(void);
static void	gp_config_popup		(void);
static gpointer	gp_config_find_value	(const gchar *path);
static void	gp_load_config		(const gchar	   *privcfgpath);
static gboolean	gp_save_session		(gpointer	    func_data,
					 const gchar	   *privcfgpath,
					 const gchar	   *globcfgpath);


static GtkWidget	*gp_applet = NULL;
static GtkTooltips	*gp_tooltips = NULL;
static GtkTooltips	*gp_desktips = NULL;
static GtkWidget	*gp_container = NULL;
static GtkWidget	*gp_desk_box = NULL;
static GtkWidget	*gp_desk_widget[MAX_DESKTOPS] = { NULL, };
static guint             gp_n_desk_widgets = 0;
static GdkWindow	*gp_atom_window = NULL;
static guint		 GP_TYPE_HBOX = 0;
static guint		 GP_TYPE_VBOX = 0;
static guint		 GP_ARROW_DIR = 0;
static gchar		*DESK_GUIDE_NAME = NULL;

static ConfigItem gp_config_items[] = {
  CONFIG_SECTION (tooltip_settings,
		  "Tooltips"),
  CONFIG_BOOL (desktips,	TRUE,
	       "Enable Desktop Names"),
  CONFIG_RANGE (desktips_delay,	10,	1,	5000,
		"Desktop Name Popup Delay [ms]"),
  CONFIG_BOOL (tooltips,	TRUE,
	       "Enable Tooltips"),
  CONFIG_RANGE (tooltips_delay,	500,	1,	5000,
		"Tooltips Delay [ms]"),
  CONFIG_SECTION (layout,
		  "Layout"),
  CONFIG_BOOL (switch_arrow,	FALSE,
	       "Switch Arrow"),
  CONFIG_BOOL (current_only,	FALSE,
	       "Show only current Desktop"),
  CONFIG_BOOL (show_pager,	TRUE,
	       "Show Desktop Pager"),
  CONFIG_PAGE ("Tasks"),
  CONFIG_BOOL (all_tasks,	FALSE,
	       "List all tasks"),
  CONFIG_PAGE ("Advanced Options"),
  CONFIG_BOOL (double_buffer,	TRUE,
	       "Draw desktops double buffered"),
};
static guint  gp_n_config_items = (sizeof (gp_config_items) /
				   sizeof (gp_config_items[0]));


/* --- FIXME: bug workarounds --- */
GNOME_Panel_OrientType fixme_panel_orient = 0;
#define applet_widget_get_panel_orient(x)	(fixme_panel_orient)
static void fixme_applet_widget_get_panel_orient (gpointer               dummy,
						  GNOME_Panel_OrientType orient)
{
  fixme_panel_orient = orient;
}

/* --- main ---*/
gint 
main (gint   argc,
      gchar *argv[])
{
  /* Initialize the i18n stuff */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);
  
  applet_widget_init ("deskguide_applet",
		      VERSION,
		      argc, argv,
		      NULL, 0, NULL);

  DESK_GUIDE_NAME = TRANSL ("GNOME Desktop Guide (Pager)");
  
  /* setup applet widget
   */
  gp_tooltips = gtk_tooltips_new ();
  gp_desktips = gtk_tooltips_new ();
  gp_applet = applet_widget_new ("deskguide_applet");
  if (!gp_applet)
    g_error ("Unable to create applet widget");
  gtk_widget_ref (gp_applet);
  
  /* main container
   */
  gp_container = gtk_widget_new (GTK_TYPE_ALIGNMENT,
				 "signal::destroy", gtk_widget_destroyed, &gp_container,
				 "child", gtk_type_new (GTK_TYPE_WIDGET),
				 NULL);
  applet_widget_add (APPLET_WIDGET (gp_applet), gp_container);
  
  /* bail out for non GNOME window managers
   */
  if (!gwmh_init ())
    {
      GtkWidget *dialog;
      gchar *error_msg =
	TRANSL ( "You are not running a GNOME "
		 "Compliant Window Manager.\n"
		 "GNOME support by the window\n"
		 "manager is a requirement for "
		 "the Desk Guide to work properly." );
      
      dialog = gnome_error_dialog (TRANSL ("Desk Guide Error"));
      gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox),
			  gtk_widget_new (GTK_TYPE_LABEL,
					  "visible", TRUE,
					  "label", DESK_GUIDE_NAME,
					  NULL),
			  TRUE, TRUE, 5);
      gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox),
			  gtk_widget_new (GTK_TYPE_LABEL,
					  "visible", TRUE,
					  "label", error_msg,
					  NULL),
			  TRUE, TRUE, 5);
      gnome_dialog_run (GNOME_DIALOG (dialog));
      
      // gtk_exit (1);
    }
  
  /* notifiers and callbacks
   */
  gwmh_desk_notifier_add (gp_desk_notifier, NULL);
  gwmh_task_notifier_add (gp_task_notifier, NULL);
  gtk_widget_set (gp_applet,
		  "signal::change-orient", fixme_applet_widget_get_panel_orient, NULL,
		  "signal::change-orient", gp_destroy_gui, NULL,
		  "signal::change-orient", gp_init_gui, NULL,
		  "object_signal::save-session", gp_save_session, NULL,
		  "signal::destroy", gtk_main_quit, NULL,
		  NULL);
  applet_widget_register_stock_callback (APPLET_WIDGET (gp_applet),
					 "about",
					 GNOME_STOCK_MENU_ABOUT,
					 TRANSL ("About..."),
					 (AppletCallbackFunc) gp_about,
					 NULL);
  applet_widget_register_stock_callback (APPLET_WIDGET (gp_applet),
					 "properties",
					 GNOME_STOCK_MENU_PROP,
					 TRANSL ("Properties..."),
					 (AppletCallbackFunc) gp_config_popup,
					 NULL);
  
  /* load configuration
   */
  gp_load_config (APPLET_WIDGET (gp_applet)->privcfgpath);
  
  /* advertise to a WM that we exist and are here
   */
  gp_atom_window = gwmh_root_put_atom_window ("_GNOME_PAGER_ACTIVE",
					      GDK_WINDOW_TEMP,
					      GDK_INPUT_OUTPUT,
					      0);
  
  /* and away into the main loop
   */
  gtk_main ();
  
  /* shutdown
   */
  gwmh_desk_notifier_remove_func (gp_desk_notifier, NULL);
  gwmh_task_notifier_remove_func (gp_task_notifier, NULL);
  gdk_window_unref (gp_atom_window);
  gtk_widget_destroy (gp_applet);
  gtk_widget_unref (gp_applet);
  
  return 0;
}

static void
gp_load_config (const gchar *privcfgpath)
{
  static guint loaded = FALSE;
  guint i;
  gchar *section = "general";
  
  if (loaded)
    return;
  loaded = TRUE;
  
  gnome_config_push_prefix (privcfgpath);
  
  for (i = 0; i < gp_n_config_items; i++)
    {
      ConfigItem *item = gp_config_items + i;
      
      if (!item->path)					/* page */
	continue;
      else if (item->min == -2 && item->max == -2)	/* section */
	section = item->path;
      else  if (item->min == -1 && item->max == -1)	/* boolean */
	{
	  gchar *path = g_strconcat (section, "/",
				     item->path, "=",
				     GPOINTER_TO_INT (item->value)
				     ? "true" :
				     "false",
				     NULL);
	  
	  item->value = GINT_TO_POINTER (gnome_config_get_bool (path));
	  g_free (path);
	}
      else						/* integer range */
	{
	  gchar *path = g_strdup_printf ("%s/%s=%d",
					 section,
					 item->path,
					 GPOINTER_TO_INT (item->value));
	  
	  item->value = GINT_TO_POINTER (gnome_config_get_int (path));
	  g_free (path);
	}
    }
  
  gnome_config_pop_prefix ();
}

static gpointer
gp_config_find_value (const gchar *path)
{
  guint i;
  
  for (i = 0; i < gp_n_config_items; i++)
    {
      ConfigItem *item = gp_config_items + i;
      
      if (path == item->path)
	return item->value;
    }
  
  g_warning (G_GNUC_PRETTY_FUNCTION "(): unable to find config value for <%s>", path);
  
  return NULL;
}

static gboolean
gp_save_session (gpointer     func_data,
		 const gchar *privcfgpath,
		 const gchar *globcfgpath)
{
  guint i;
  gchar *section = "general";
  
  gnome_config_push_prefix (privcfgpath);
  
  for (i = 0; i < gp_n_config_items; i++)
    {
      ConfigItem *item = gp_config_items + i;
      gchar *path = g_strconcat (section, "/", item->path, NULL);
      
      if (!item->path)					/* page */
	continue;
      else if (item->min == -2 && item->max == -2)	/* section */
	section = item->path;
      else if (item->min == -1 && item->max == -1)	/* boolean */
	gnome_config_set_bool (path, GPOINTER_TO_INT (item->value));
      else						/* integer range */
	gnome_config_set_int (path, GPOINTER_TO_INT (item->value));
      g_free (path);
    }
  
  gnome_config_pop_prefix ();
  
  gnome_config_sync ();
  gnome_config_drop_all ();
  
  return FALSE;
}

static void
gp_config_reset_tmp_values (void)
{
  guint i;
  
  for (i = 0; i < gp_n_config_items; i++)
    {
      ConfigItem *item = gp_config_items + i;
      
      if (!item->path)					/* page */
	continue;
      else if (item->min == -2 && item->max == -2)	/* section */
	continue;
      else if (item->min == -1 && item->max == -1)	/* boolean */
	item->tmp_value = item->value;
      else						/* integer range */
	item->tmp_value = item->value;
    }
}

static void
gp_config_apply_tmp_values (void)
{
  guint i;
  
  for (i = 0; i < gp_n_config_items; i++)
    {
      ConfigItem *item = gp_config_items + i;
      
      if (!item->path)					/* page */
	continue;
      else if (item->min == -2 && item->max == -2)	/* section */
	continue;
      else if (item->min == -1 && item->max == -1)	/* boolean */
	item->value = item->tmp_value;
      else						/* integer range */
	item->value = item->tmp_value;
    }
}

static gboolean
gp_desk_notifier (gpointer	   func_data,
		  GwmhDesk	  *desk,
		  GwmhDeskInfoMask change_mask)
{
  guint i;
  
  /* keep number of desktop widgets in sync with desk */
  for (i = desk->n_desktops; i < gp_n_desk_widgets; i++)
    gtk_widget_destroy (gp_desk_widget[i]);
  gp_create_desk_widgets ();

  if (gp_n_desk_widgets && BOOL_CONFIG (current_only))
    gwm_desktop_set_index (GWM_DESKTOP (gp_desk_widget[0]),
			   desk->current_desktop);
  
  return TRUE;
}

static gboolean
gp_task_notifier (gpointer	     func_data,
		  GwmhTask	    *task,
		  GwmhTaskNotifyType ntype,
		  GwmhTaskInfoMask   imask)
{
  return TRUE;
}

static void 
gp_destroy_gui (void)
{
  if (gp_container)
    {
      gtk_widget_hide (gp_container);
      if (GTK_BIN (gp_container)->child)
	gtk_widget_destroy (GTK_BIN (gp_container)->child);
      else
	g_warning (G_GNUC_PRETTY_FUNCTION "(): missing container child");
    }
  else
    g_warning (G_GNUC_PRETTY_FUNCTION "(): missing container");
}

static void
gp_create_desk_widgets (void)
{
  if (N_DESKTOPS > MAX_DESKTOPS)
    g_error ("MAX_DESKTOPS limit reached, adjust source code");

  if (!gp_desk_box)
    return;

  gp_n_desk_widgets = 0;
  if (BOOL_CONFIG (show_pager))
    {
      guint i, max = BOOL_CONFIG (current_only) ? 1 : N_DESKTOPS;
      
      for (i = 0; i < max; i++)
	if (!gp_desk_widget[i])
	  {
	    gp_desk_widget[i] = gwm_desktop_new (max > 1 ? i : CURRENT_DESKTOP,
						 gp_desktips);
	    gtk_widget_set (gp_desk_widget[i],
			    "parent", gp_desk_box,
			    "visible", TRUE,
			    "signal::destroy", gtk_widget_destroyed, &gp_desk_widget[i],
			    NULL);
	  }
      gp_n_desk_widgets = i;
    }
}

static inline gboolean
gp_widget_button_popup_task_editor (GtkWidget      *widget,
				    GdkEventButton *event,
				    gpointer        data)
{
  guint button = GPOINTER_TO_UINT (data);


  if (event->button == button)
    {
      g_message ("Task Editor unimplemented yet!");
      gdk_beep ();

      return TRUE;
    }
  else
    return FALSE;
}

static inline gboolean
gp_widget_button_popup_config (GtkWidget      *widget,
			       GdkEventButton *event,
			       gpointer        data)
{
  guint button = GPOINTER_TO_UINT (data);


  if (event->button == button)
    {
      gp_config_popup ();

      return TRUE;
    }
  else
    return FALSE;
}

static inline gboolean
gp_widget_ignore_button (GtkWidget      *widget,
			 GdkEventButton *event,
			 gpointer        data)
{
  guint button = GPOINTER_TO_UINT (data);
  
  if (event->button == button)
    gtk_signal_emit_stop_by_name (GTK_OBJECT (widget),
				  event->type == GDK_BUTTON_PRESS
				  ? "button_press_event"
				  : "button_release_event");
  return FALSE;
}

static void 
gp_init_gui (void)
{
  GtkWidget *button, *abox;
  GwmDesktopClass *class;
  gboolean arrow_at_end = FALSE;
  
  class = gtk_type_class (GWM_TYPE_DESKTOP);
  class->double_buffer = BOOL_CONFIG (double_buffer);
  gtk_widget_set_usize (gp_container, 0, 0);
  switch (applet_widget_get_panel_orient (APPLET_WIDGET (gp_applet)))
    {
    case ORIENT_UP:
      GP_TYPE_HBOX = GTK_TYPE_HBOX;
      GP_TYPE_VBOX = GTK_TYPE_VBOX;
      GP_ARROW_DIR = GTK_ARROW_UP;
      gtk_widget_set (gp_container,
		      NULL);
      class->orientation = GTK_ORIENTATION_HORIZONTAL;
      arrow_at_end = FALSE;
      break;
    case ORIENT_DOWN:
      GP_TYPE_HBOX = GTK_TYPE_HBOX;
      GP_TYPE_VBOX = GTK_TYPE_VBOX;
      GP_ARROW_DIR = GTK_ARROW_DOWN;
      gtk_widget_set (gp_container,
		      NULL);
      class->orientation = GTK_ORIENTATION_HORIZONTAL;
      arrow_at_end = TRUE;
      break;
    case ORIENT_LEFT:
      GP_TYPE_HBOX = GTK_TYPE_VBOX;
      GP_TYPE_VBOX = GTK_TYPE_HBOX;
      GP_ARROW_DIR = GTK_ARROW_LEFT;
      gtk_widget_set (gp_container,
		      NULL);
      class->orientation = GTK_ORIENTATION_VERTICAL;
      arrow_at_end = FALSE;
      break;
    case ORIENT_RIGHT:
      GP_TYPE_HBOX = GTK_TYPE_VBOX;
      GP_TYPE_VBOX = GTK_TYPE_HBOX;
      GP_ARROW_DIR = GTK_ARROW_RIGHT;
      gtk_widget_set (gp_container,
		      NULL);
      class->orientation = GTK_ORIENTATION_VERTICAL;
      arrow_at_end = TRUE;
      break;
    }
  
  /* configure tooltips
   */
  (BOOL_CONFIG (tooltips)
   ? gtk_tooltips_enable
   : gtk_tooltips_disable) (gp_tooltips);
  gtk_tooltips_set_delay (gp_tooltips, RANGE_CONFIG (tooltips_delay));
  (BOOL_CONFIG (desktips)
   ? gtk_tooltips_enable
   : gtk_tooltips_disable) (gp_desktips);
  gtk_tooltips_set_delay (gp_desktips, RANGE_CONFIG (desktips_delay));
  
  /* main container
   */
  gp_desk_box = gtk_widget_new (GP_TYPE_HBOX,
				"visible", TRUE,
				"spacing", 0,
				"signal::destroy", gtk_widget_destroyed, &gp_desk_box,
				"parent", gp_container,
				NULL);
  
  
  /* arrow button
   */
  abox = gtk_widget_new (GP_TYPE_VBOX,
			 "visible", TRUE,
			 "spacing", 0,
			 NULL);
  (BOOL_CONFIG (switch_arrow)
   ? gtk_box_pack_end
   : gtk_box_pack_start) (GTK_BOX (gp_desk_box), abox, FALSE, TRUE, 0);
  button = gtk_widget_new (GTK_TYPE_BUTTON,
			   "visible", TRUE,
			   "child", gtk_widget_new (GTK_TYPE_ARROW,
						    "arrow_type", GP_ARROW_DIR,
						    "visible", TRUE,
						    NULL),
			   "signal::button_press_event", gp_widget_button_popup_task_editor, GUINT_TO_POINTER (1),
			   "signal::button_press_event", gp_widget_ignore_button, GUINT_TO_POINTER (2),
			   "signal::button_release_event", gp_widget_ignore_button, GUINT_TO_POINTER (2),
			   "signal::button_press_event", gp_widget_ignore_button, GUINT_TO_POINTER (3),
			   "signal::button_release_event", gp_widget_ignore_button, GUINT_TO_POINTER (3),
			   NULL);
  gtk_tooltips_set_tip (gp_tooltips,
			button,
			DESK_GUIDE_NAME,
			NULL);
  (arrow_at_end
   ? gtk_box_pack_end
   : gtk_box_pack_start) (GTK_BOX (abox), button, FALSE, TRUE, 0);
  button = gtk_widget_new (GTK_TYPE_BUTTON,
			   "visible", TRUE,
			   "label", "?",
			   "signal::button_press_event", gp_widget_button_popup_config, GUINT_TO_POINTER (1),
			   "signal::button_press_event", gp_widget_ignore_button, GUINT_TO_POINTER (2),
			   "signal::button_release_event", gp_widget_ignore_button, GUINT_TO_POINTER (2),
			   "signal::button_press_event", gp_widget_ignore_button, GUINT_TO_POINTER (3),
			   "signal::button_release_event", gp_widget_ignore_button, GUINT_TO_POINTER (3),
			   NULL);
  gtk_tooltips_set_tip (gp_tooltips,
			button,
			DESK_GUIDE_NAME,
			NULL);
  (!arrow_at_end
   ? gtk_box_pack_end
   : gtk_box_pack_start) (GTK_BOX (abox), button, TRUE, TRUE, 0);
  
  /* desktop pagers
   */
  gp_create_desk_widgets ();
  
  gtk_widget_show (gp_container);
  gtk_widget_show (gp_applet);
}

static void 
gp_about (void)
{
  static GtkWidget *dialog = NULL;
  
  if (!dialog)
    {
      const char *authors[] = {
	"Tim Janik",
	NULL
      };
      
      dialog = gnome_about_new ("Desk Guide",
				"0.3",
				"Copyright (C) 1999 Tim Janik",
				authors,
				DESK_GUIDE_NAME,
				NULL);
      gtk_widget_set (dialog,
		      "signal::destroy", gtk_widget_destroyed, &dialog,
		      NULL);
      gnome_dialog_close_hides (GNOME_DIALOG (dialog), TRUE);
      gtk_quit_add_destroy (1, GTK_OBJECT (dialog));
    }
  
  gtk_widget_show (dialog);
  gdk_window_raise (dialog->window);
}

static void
gp_config_toggled (GtkToggleButton *button,
		   ConfigItem      *item)
{
  item->tmp_value = GINT_TO_POINTER (button->active);
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gtk_widget_get_toplevel (GTK_WIDGET (button))));
}

static inline GtkWidget*
gp_config_add_boolean (GtkWidget  *vbox,
		       ConfigItem *item)
{
  GtkWidget *widget;
  
  widget = gtk_widget_new (GTK_TYPE_CHECK_BUTTON,
			   "visible", TRUE,
			   "label", TRANSL (item->name),
			   "active", GPOINTER_TO_INT (item->value),
			   "signal::toggled", gp_config_toggled, item,
			   NULL);
  gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, TRUE, 0);
  gtk_widget_set (GTK_BIN (widget)->child,
		  "xalign", 0.0,
		  NULL);
  
  return widget;
}

static void
gp_config_value_changed (GtkAdjustment *adjustment,
			 ConfigItem      *item)
{
  item->tmp_value = GINT_TO_POINTER ((gint) adjustment->value);
  gnome_property_box_changed (GNOME_PROPERTY_BOX (gtk_widget_get_toplevel (gtk_object_get_user_data (GTK_OBJECT (adjustment)))));
}

static inline GtkWidget*
gp_config_add_range (GtkWidget  *vbox,
		     ConfigItem *item)
{
  GtkObject *adjustment;
  GtkWidget *hbox, *label, *spinner;
  
  adjustment = gtk_adjustment_new (GPOINTER_TO_UINT (item->tmp_value),
				   item->min,
				   item->max,
				   1, 1,
				   (item->max - item->min) / 10);
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "homogeneous", TRUE,
			 "visible", TRUE,
			 "spacing", GNOME_PAD_SMALL,
			 "border_width", GNOME_PAD_SMALL,
			 NULL);
  label = gtk_widget_new (GTK_TYPE_LABEL,
			  "visible", TRUE,
			  "xalign", 0.0,
			  "label", TRANSL (item->name),
			  "parent", hbox,
			  NULL);
  // gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
  spinner = gtk_spin_button_new (GTK_ADJUSTMENT (adjustment), 0, 0);
  gtk_widget_show (spinner);
  gtk_object_set_user_data (GTK_OBJECT (adjustment), spinner);
  gtk_box_pack_end (GTK_BOX (hbox), spinner, FALSE, TRUE, 0);
  gtk_signal_connect (adjustment,
		      "value_changed",
		      GTK_SIGNAL_FUNC (gp_config_value_changed),
		      item);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
  
  return hbox;
}

static inline GtkWidget*
gp_config_add_section (GtkWidget   *parent,
		       const gchar *section)
{
  GtkWidget *box;
  
  if (section)
    box = gtk_widget_new (GTK_TYPE_VBOX,
			  "visible", TRUE,
			  "parent",
			  gtk_widget_new (GTK_TYPE_FRAME,
					  "visible", TRUE,
					  "label", section,
					  "parent", parent,
					  NULL),
			  NULL);
  else
    box = gtk_widget_new (GTK_TYPE_VBOX,
			  "visible", TRUE,
			  "parent", parent,
			  NULL);
  return box;
}

static GSList*
gp_config_create_page (GSList		*item_slist,
		       GnomePropertyBox *pbox)
{
  gchar *page_name;
  ConfigItem *item;
  GtkWidget *page, *vbox = NULL;
  
  g_return_val_if_fail (item_slist != NULL, NULL);
  
  item = item_slist->data;
  if (!item->path)
    {
      GSList *node = item_slist;
      
      item_slist = node->next;
      g_slist_free_1 (node);
      
      page_name = TRANSL (item->name);
    }
  else
    page_name = TRANSL ("Global");
  
  page = gtk_widget_new (GTK_TYPE_VBOX,
			 "visible", TRUE,
			 "border_width", GNOME_PAD_SMALL,
			 "spacing", GNOME_PAD_SMALL,
			 NULL);
  
  while (item_slist)
    {
      GSList *node = item_slist;
      ConfigItem *item = node->data;
      
      if (!item->path)						/* page */
	break;
      item_slist = node->next;
      g_slist_free_1 (node);
      
      if (item->min == -2 && item->max == -2)			/* section */
	vbox = gp_config_add_section (page, TRANSL (item->name));
      else if (item->min == -1 && item->max == -1)		/* boolean */
	{
	  if (!vbox)
	    vbox = gp_config_add_section (page, NULL);
	  gp_config_add_boolean (vbox, item);
	}
      else							/* integer range */
	{
	  if (!vbox)
	    vbox = gp_config_add_section (page, NULL);
	  gp_config_add_range (vbox, item);
	}
    }
  
  gnome_property_box_append_page (pbox, page, gtk_label_new (TRANSL (page_name)));
  
  return item_slist;
}

static void 
gp_config_popup (void)
{
  static GtkWidget *dialog = NULL;
  
  g_return_if_fail (gp_n_config_items > 0);
  
  if (!dialog)
    {
      GSList *slist = NULL;
      guint i;
      
      for (i = 0; i < gp_n_config_items; i++)
	slist = g_slist_prepend (slist, gp_config_items + i);
      slist = g_slist_reverse (slist);
      
      dialog = gnome_property_box_new ();
      gtk_widget_set (dialog,
		      "title", TRANSL ("Desk Guide Settings"),
		      "signal::apply", gp_destroy_gui, NULL,
		      "signal::apply", gp_config_apply_tmp_values, NULL,
		      "signal::apply", gp_init_gui, NULL,
		      "signal::destroy", gp_config_reset_tmp_values, NULL,
		      NULL);
      
      while (slist)
	slist = gp_config_create_page (slist, GNOME_PROPERTY_BOX (dialog));
      
      gtk_widget_set (dialog,
		      "signal::destroy", gtk_widget_destroyed, &dialog,
		      NULL);
      gtk_quit_add_destroy (1, GTK_OBJECT (dialog));
    }
  
  gtk_widget_show (dialog);
  gdk_window_raise (dialog->window);
}
