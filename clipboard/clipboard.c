/* this is a simple clipboard applet for the gnome panel*/
#include "copy.xpm"
#include "set_selection.xpm"

#include <config.h>
#include <gnome.h>
#include <applet-widget.h>

#define CLIPBOARD_VERSION "0.02"
#define MAX_HISTORY 10 /* maximum number of items in history list
                          (once deleting old history is implemented. 
                          of course eventually, it should be configurable.) */
#define MAX_DESCRIPTION_LEN 25
typedef struct _clipboard_item clipboard_item;
/* All data associated with a particular item in the clipboard goes here 
 * When data types other than strings become supported, a data-type member
 * should be added */
struct _clipboard_item {
  GString *clipboard_data; /* when we support more kinds of selections, this
                            * will become a union or something */
  GString *description;
  guint id; 
};

GtkWidget *vbox;
GtkWidget *main_box;
GtkWidget *applet;
GtkWidget *copy_button;
GtkWidget *set_selection_button;
GtkTooltips *set_selection_tooltip;
GdkPixmap *copy_pixmap;
GdkPixmap *set_selection_pixmap;
clipboard_item clipboard;
GList     *history;
gchar     *goad_id;

static void
set_description(clipboard_item *p_clipboard)
{
  g_string_assign (p_clipboard->description, p_clipboard->clipboard_data->str);
  g_string_truncate (p_clipboard->description, MAX_DESCRIPTION_LEN); 
}

static void 
clipboard_item_assign(clipboard_item *lvalue, clipboard_item *rvalue)
{
  g_string_assign(lvalue->clipboard_data, rvalue->clipboard_data->str);
  g_string_assign(lvalue->description, rvalue->description->str);
}

static void
history_menu_cb (AppletWidget *applet, gpointer data)
{
  clipboard_item * p_history_clipboard = data;
  clipboard_item_assign(&clipboard, p_history_clipboard);
  gtk_selection_owner_set (set_selection_button,
	  		     GDK_SELECTION_PRIMARY,
                             GDK_CURRENT_TIME);
  gtk_tooltips_set_tip (set_selection_tooltip, set_selection_button, 
			clipboard.description->str, NULL);
  return;
}

static void
update_history (clipboard_item *p_clipboard_data)
{
  clipboard_item * new_clipboard;
  new_clipboard = g_malloc(sizeof(clipboard_item));
  new_clipboard->description = g_string_new(NULL);
  new_clipboard->clipboard_data = g_string_new(NULL);
  clipboard_item_assign(new_clipboard, p_clipboard_data);
  history=g_list_append(history, &new_clipboard);
  new_clipboard->id = g_list_length(history);
  applet_widget_register_callback (APPLET_WIDGET(applet),
				   g_strdup_printf("%d", new_clipboard->id),
				   new_clipboard->description->str,
				   history_menu_cb,
				   new_clipboard);
  /*
  if (g_list_length(history) > MAX_HISTORY)
  {
    g_list_remove(g_list_first(history));
  }
  */
}

static gint
copy_clicked_cb(GtkWidget *widget, gpointer data)
{
  static GdkAtom targets_atom = GDK_NONE;
  targets_atom = gdk_atom_intern ("COMPOUND_TEXT", FALSE);
  gtk_selection_convert (widget, GDK_SELECTION_PRIMARY, targets_atom,
                         GDK_CURRENT_TIME);
  return TRUE;
}

/* Signal handler called when the selections owner returns the data */
static void
selection_received (GtkWidget *widget, GtkSelectionData *selection_data, 
                    gpointer data)
{
  if (selection_data->length > 0) /* this tests if we got the selection */ 
  {
    g_message(selection_data->data);
    g_string_assign (clipboard.clipboard_data, selection_data->data);
    set_description (&clipboard);
    gtk_tooltips_set_tip (set_selection_tooltip, set_selection_button, 
			  clipboard.description->str, NULL);
    update_history (&clipboard);
  }
  return;
}

static gint
set_selection_clicked_cb(GtkWidget *widget, gpointer data)
{
  gtk_selection_owner_set (widget,
	  		     GDK_SELECTION_PRIMARY,
                             GDK_CURRENT_TIME);
  return TRUE;
}

/* sets the clipboard contents as the selection when it gets a request */
static void
clipboard_selection_handler(GtkWidget *widget,
			   GtkSelectionData *selection_data,
			   guint info,
			   guint time,
		           gpointer data)
{
  gtk_selection_data_set(selection_data,
			 GDK_SELECTION_TYPE_STRING,
			 8,
			 clipboard.clipboard_data->str,
			 clipboard.clipboard_data->len);
  return;
}

static GtkWidget *
button_new_with_xpm(GtkWidget *parent, gchar **xpm)
{
  GtkWidget *button;
  GtkWidget *pixmapwid;
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  GtkStyle *style;
  /* load the pixmaps */
  style = gtk_widget_get_style(parent);
  pixmap = gdk_pixmap_create_from_xpm_d (parent->window, &mask,
                                               &style->bg[GTK_STATE_NORMAL],
                                               xpm);
  pixmapwid = gtk_pixmap_new (pixmap, mask);
  button = gtk_button_new();
  gtk_container_add (GTK_CONTAINER (button), pixmapwid);
  gtk_widget_show (pixmapwid);
  return button;
}

static void
about (AppletWidget *applet, gpointer data)
{
  static const char *authors[] = {"Alexandre Muñiz <munizao@xprt.net>",
				  NULL };
  static GtkWidget *about_box = NULL;

  if (about_box != NULL)
  {
  	gdk_window_show(about_box->window);
	gdk_window_raise(about_box->window);
	return;
  }
  about_box = gnome_about_new (_("Clipboard Applet"),
			       CLIPBOARD_VERSION,
			       _("Copyright (C) 1999"),
			       authors,
			       _("Gnome panel applet for copying and "
			         "retrieving selections with a history list. "
				 "Released under GNU General Public Licence."),
			       NULL);
  gtk_signal_connect( GTK_OBJECT(about_box), "destroy",
		      GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_box );
  gtk_widget_show( about_box );
  return;
}

static void
relayout (gboolean vertical)
{
	gtk_widget_ref (copy_button);
	gtk_widget_ref (set_selection_button);

	gtk_container_remove (GTK_CONTAINER (main_box), copy_button);
	gtk_container_remove (GTK_CONTAINER (main_box), set_selection_button);
	gtk_widget_destroy (main_box);

	if (vertical)
		main_box = gtk_vbox_new(TRUE, 0);
	else
		main_box = gtk_hbox_new(TRUE, 0);
	gtk_widget_show (main_box);
	gtk_box_pack_start(GTK_BOX (vbox), main_box, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX (main_box), copy_button, TRUE, TRUE, 0);

	gtk_box_pack_start(GTK_BOX (main_box), set_selection_button, TRUE, TRUE, 0);

	gtk_widget_unref (copy_button);
	gtk_widget_unref (set_selection_button);
}

/*this is when the panel orientation changes*/
static void
applet_change_orient (GtkWidget *w, PanelOrientType o, gpointer data)
{
	int size = applet_widget_get_panel_pixel_size (APPLET_WIDGET (applet));
	switch(o) {
	case ORIENT_UP: 
	case ORIENT_DOWN:
		if(size<48)
			relayout (FALSE);
		else
			relayout (TRUE);
		break;
	case ORIENT_LEFT:
	case ORIENT_RIGHT:
		if(size<48)
			relayout (TRUE);
		else
			relayout (FALSE);
		break;
	}
	return;
}

static void
applet_change_pixel_size (GtkWidget *w, int size, gpointer data)
{
	PanelOrientType o = applet_widget_get_panel_orient (APPLET_WIDGET (applet));
	switch(o) {
	case ORIENT_UP: 
	case ORIENT_DOWN:
		if(size<48)
			relayout (FALSE);
		else
			relayout (TRUE);
		break;
	case ORIENT_LEFT:
	case ORIENT_RIGHT:
		if(size<48)
			relayout (TRUE);
		else
			relayout (FALSE);
		break;
	}
	return;
}

int
main(int argc, char **argv)
{
  /* initialize the clipboard */
  clipboard.clipboard_data = g_string_new(NULL);
  clipboard.description = g_string_new(NULL);  

  /* Initialize the i18n stuff */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  /* intialize, this will basically set up the applet, corba and
     call gnome_init */
   applet_widget_init("clipboard_applet", VERSION, argc, argv, NULL, 0, NULL);

   /* create tooltip */
  set_selection_tooltip = gtk_tooltips_new ();

  /* create a new applet_widget */
  applet = applet_widget_new("clipboard_applet");
  /* in the rare case that the communication with the panel
     failed, error out */
  if (!applet)
    g_error("Can't create applet!\n");
  
  /* create the vbox */
  vbox = gtk_vbox_new(TRUE, 0);
  main_box = gtk_vbox_new(TRUE, 0);
  gtk_box_pack_start(GTK_BOX (vbox), main_box, TRUE, TRUE, 0);
  /* create, attach signal to, and pack the buttons */

  /* realize the applet so that we may get the xpms */
  gtk_widget_realize (applet);

  copy_button = button_new_with_xpm(applet, copy_xpm);
  gtk_signal_connect (GTK_OBJECT (copy_button), "clicked",
                      GTK_SIGNAL_FUNC (copy_clicked_cb), NULL);
  gtk_signal_connect (GTK_OBJECT(copy_button), "selection_received",
                      GTK_SIGNAL_FUNC (selection_received), NULL);
  gtk_box_pack_start(GTK_BOX (main_box), copy_button, TRUE, TRUE, 0);

  set_selection_button = button_new_with_xpm(applet, set_selection_xpm);
  gtk_signal_connect (GTK_OBJECT (set_selection_button), "clicked",
                      GTK_SIGNAL_FUNC (set_selection_clicked_cb), NULL);
  gtk_selection_add_target (set_selection_button, 
			    GDK_SELECTION_PRIMARY,
                            GDK_SELECTION_TYPE_STRING,
			    0);
  gtk_signal_connect (GTK_OBJECT (set_selection_button), "selection_get",
		      GTK_SIGNAL_FUNC (clipboard_selection_handler),
		      NULL);

  gtk_box_pack_start(GTK_BOX (main_box), set_selection_button, TRUE, TRUE, 0);
  gtk_widget_show_all(vbox);

  gtk_signal_connect(GTK_OBJECT(applet),"change_pixel_size",
		     GTK_SIGNAL_FUNC(applet_change_pixel_size),
		     NULL);
  gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
		     GTK_SIGNAL_FUNC(applet_change_orient),
		     NULL);

  /* connect up the about box */
  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
				         "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."),
					 about,
					 NULL);

  /* add the vbox to the applet_widget */
  applet_widget_add (APPLET_WIDGET (applet), vbox);
  gtk_widget_show (applet);

  /* special corba main loop */
  applet_widget_gtk_main ();

  return 0;
}
