/* charpick.c  This is a gnome panel applet that allow users to select
 * accented (and other) characters to be pasted into other apps.
 */

#include <config.h>
#include <applet-widget.h>
#define NO_LAST_INDEX -1

typedef struct _charpick_persistant_properties charpick_persistant_properties;
/* This is the data type for user definable properties of the charpick applet.
 * It should include anything we might want to save when (and if)
 * we save the session. TODO: add font, height, and width here.
 */
struct _charpick_persistant_properties {
  const gchar * default_charlist;
  gint    rows;
  gint    cols;
  gint    height;
  gint    width;
};

typedef struct _charpick_data charpick_data;
/* this type has basically all data for this program */
struct _charpick_data {
  const gchar * charlist;
  gchar selected_char;
  gint last_index;
  GtkWidget * *toggle_buttons;
  GtkWidget * *labels;
  GtkWidget *event_box;
  GtkWidget *applet;
  charpick_persistant_properties * properties;
};

typedef struct _charpick_button_cb_data charpick_button_cb_data;
/* This is the data type for the button callback function. */

struct _charpick_button_cb_data {
  gint button_index;
  charpick_data * curr_data;
};

static GtkWidget * event_box;

/* This stuff assumes that this program is being run in an environment
 * that uses ISO-8859-1 (latin-1) as its native character code.
 */

/* The comment for each char list has the html entity names of the chars */
/* agrave, aacute, acirc, atilde, auml. aring, aelig, ordf */
static const gchar *a_list = "àáâãäåæª";
/* ccedil, cent, copy */
static const gchar *c_list = "ç¢©";
/* egrave, eacute, ecirc, euml, aelig */
static const gchar *e_list = "èéêëæ";
/* igrave, iacute, icirc, iuml */
static const gchar *i_list = "ìíîï";
/* micro */
static const gchar *m_list = "µ";
/* ntilde (this is the most important line in this program.) */
static const gchar *n_list = "ñ";
/* ograve, oacute, ocirc, otilde, ouml, oslash, ordm */
static const gchar *o_list = "òóôõöøº";
/* para */
static const gchar *p_list = "¶";
/* reg */
static const gchar *r_list = "®";
/* szlig, sect */
static const gchar *s_list = "ß§";
/* ugrave, uacute, ucirc, uuml */
static const gchar *u_list = "ùúûü";
/* yacute, yuml */
static const gchar *y_list = "ýÿ";



/* sets the picked character as the selection when it gets a request */
static void
charpick_selection_handler(GtkWidget *widget,
			   GtkSelectionData *selection_data,
			   guint info,
			   guint time,
		           gpointer data)
{
  charpick_data *curr_data = data;

#ifdef DEBUG
  g_print("Selection Retrieved\n");
#endif

  gtk_selection_data_set(selection_data,
			 GDK_SELECTION_TYPE_STRING,
			 8,
			 &curr_data->selected_char,
			 1);
}

/* displays a list of characters in labels on buttons*/
static void
display_charlist (charpick_data *data)
{
  gint rows = data->properties->rows;
  gint cols = data->properties->cols;
  const gchar *charlist = data->charlist;
  gchar * teststr;
  /* let's show no more characters than we have, or that we have space for. */
  gint numtoshow = MIN(rows * cols, strlen(charlist));
  gint i = 0; /* loop variable */
  gchar currstr[2];

  /* reset the characters on the labels and reshow the buttons */
  for (i=0;i<numtoshow;i++)
  {
    currstr[0] = charlist[i];
    currstr[1] = '\0';
    gtk_label_set_text(GTK_LABEL(data->labels[i]), currstr);
    gtk_widget_show_all(data->toggle_buttons[i]);
  }
  /* extra buttons? hide em */
  if ((rows * cols) > strlen(charlist))
    for (i=numtoshow;i<(rows*cols);i++)
      gtk_widget_hide(data->toggle_buttons[i]);
  
}

static gint
toggle_button_toggled_cb(GtkWidget *widget, gpointer data)
{
  charpick_button_cb_data *cb_data = data;
  gint i;
  gint rows = cb_data->curr_data->properties->rows;
  gint cols = cb_data->curr_data->properties->cols;
  gint button_index = cb_data->button_index;
  gint last_index = cb_data->curr_data->last_index;
  if ((GTK_TOGGLE_BUTTON (cb_data->curr_data->toggle_buttons[button_index])->active))
  {
    if ((last_index != NO_LAST_INDEX) && (last_index != button_index))
      gtk_toggle_button_set_state
        (GTK_TOGGLE_BUTTON (cb_data->curr_data->toggle_buttons[last_index]), 
         FALSE);    
    gtk_widget_grab_focus(event_box);
    /* set selected_char */
    cb_data->curr_data->selected_char = 
    cb_data->curr_data->charlist[button_index];
    /* set this? widget as the selection owner */
    gtk_selection_owner_set (event_box,
	  		     GDK_SELECTION_PRIMARY,
                             GDK_CURRENT_TIME);
  cb_data->curr_data->last_index = button_index;
  }		     
  return TRUE;
}

static gint
key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  charpick_data *curr_data = data;
  gchar inputchar = event->keyval;
  switch (inputchar)
    {
    case 'a' : curr_data->charlist = a_list;
               break;
    case 'c' : curr_data->charlist = c_list;
               break;
    case 'e' : curr_data->charlist = e_list;
               break;
    case 'i' : curr_data->charlist = i_list;
               break;
    case 'm' : curr_data->charlist = m_list;
               break;
    case 'n' : curr_data->charlist = n_list;
               break;
    case 'o' : curr_data->charlist = o_list;
               break;
    case 'p' : curr_data->charlist = p_list;
               break;
    case 'r' : curr_data->charlist = r_list;
               break;
    case 's' : curr_data->charlist = s_list;
               break;
    case 'u' : curr_data->charlist = u_list;
               break;
    case 'y' : curr_data->charlist = y_list;
               break;


    }
  if (curr_data->last_index != NO_LAST_INDEX)
    gtk_toggle_button_set_state
        (GTK_TOGGLE_BUTTON (curr_data->toggle_buttons[curr_data->last_index]), 
         FALSE);
  curr_data->last_index = NO_LAST_INDEX;
  display_charlist(curr_data);
  return TRUE;
}

static void
about (AppletWidget *applet, gpointer data)
{
  static const char *authors[] = { "Alexandre Muñiz (munizao@cyberhighway.net)", NULL };
  GtkWidget *about_box;

  about_box = gnome_about_new (_("Character Picker"),
			       _("0.02"),
			       _("Copyright (C) 1998"),
			       authors,
			       _("Gnome Panel applet for selecting strange "
			         "characters that are not on my keyboard. "
				 "Released under GNU General Public Licence."),
			       NULL);

  gtk_window_set_modal (GTK_WINDOW(about_box),TRUE);
  gnome_dialog_run (GNOME_DIALOG (about_box));
}

int
main (int argc, char *argv[])
{
  GtkWidget *applet;
  /*  GtkWidget *event_box; */
  GtkWidget *table;
  GtkWidget *toggle_button[9];
  GtkWidget *label[9];
  gint rows, cols;
  int      i;
  /* initialize properties. when sm is added, these will be loaded
   * rather than simply copied from the defaults.
   */
  charpick_persistant_properties default_properties =
  {
    a_list,
    3,
    3,
    51,
    51
  };

  charpick_data curr_data =
  {
    default_properties.default_charlist,
    ' ',
    NO_LAST_INDEX,
    toggle_button,
    label,
    event_box,
    applet,
    &default_properties
  };
  charpick_button_cb_data button_cb_data[9];
  rows = default_properties.rows;
  cols = default_properties.cols;


  /* Initialize the i18n stuff */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);
  /* initialize applet */
  applet_widget_init("charpick_applet", VERSION, argc, argv,
			      NULL, 0, NULL);

  /* create a new applet_widget */
  applet = applet_widget_new("charpick_applet");
  /* in the rare case that the communication with the panel
     failed, error out */
  if (!applet)
    g_error("Can't create applet!\n");


  /* Create the event_box (needed to catch keypress and focus change events) */

  event_box = gtk_event_box_new ();

  /* Create table */
  table = gtk_table_new (rows, cols, TRUE);
  for(i=0;i<(rows*cols);i++)
    label[i] = gtk_label_new("Q");
  /* buttons */
  for(i=0;i<(rows*cols);i++)
  {
    toggle_button[i] = gtk_toggle_button_new();
    /* pack label in button */
    gtk_container_add(GTK_CONTAINER(toggle_button[i]), label[i]);
    /* pack button in table */
    gtk_table_attach_defaults(GTK_TABLE(table),
                              toggle_button[i],
                              (i % cols),
                              (i % cols + 1),
                              (i / rows),
                              (i / rows + 1));
    button_cb_data[i].button_index = i;
    button_cb_data[i].curr_data = &curr_data;
    gtk_button_set_relief(GTK_BUTTON(toggle_button[i]), GTK_RELIEF_NONE);
    /* connect toggle signal for button to handler */
    gtk_signal_connect (GTK_OBJECT (toggle_button[i]), "toggled",
                        (GtkSignalFunc) toggle_button_toggled_cb,
                        &button_cb_data[i]);
  }			    

  GTK_WIDGET_SET_FLAGS (event_box, GTK_CAN_FOCUS);
  gtk_widget_grab_focus(event_box);
  /* Signals used to draw backing pixmap to the window*/
  /* 
  gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",
		      (GtkSignalFunc) expose_event, NULL);
  
  gtk_signal_connect (GTK_OBJECT(drawing_area),"configure_event",
        	      (GtkSignalFunc) configure_event, &curr_properties);
  */
  /* Event signals */
  /*
  gtk_signal_connect (GTK_OBJECT (drawing_area), "button_release_event",
		      (GtkSignalFunc) button_release_event, &curr_properties);
  */
  gtk_signal_connect (GTK_OBJECT (event_box), "key_press_event",
		      (GtkSignalFunc) key_press_event, &curr_data);
  /*
  gtk_signal_connect (GTK_OBJECT (applet), "focus_in_event",
		      (GtkSignalFunc) focus_in_event, NULL);

  gtk_signal_connect (GTK_OBJECT (applet), "focus_out_event",
		      (GtkSignalFunc) focus_out_event, NULL);
  */
  gtk_widget_set_events (event_box, /*GDK_EXPOSURE_MASK
			 | */ GDK_FOCUS_CHANGE_MASK
			 | GDK_KEY_PRESS_MASK);
  /* selection handler for selected character */
  gtk_selection_add_target (event_box, 
			    GDK_SELECTION_PRIMARY,
                            GDK_SELECTION_TYPE_STRING,
			    0);
  gtk_signal_connect (GTK_OBJECT (event_box), "selection_get",
		      GTK_SIGNAL_FUNC (charpick_selection_handler),
		      &curr_data);

  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
				         "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."),
					 about,
					 NULL);

  gtk_container_add (GTK_CONTAINER(event_box), table);
  applet_widget_add (APPLET_WIDGET (applet), event_box);

  gtk_widget_show (event_box);
  gtk_widget_set_usize(table, 
                       default_properties.width, 
                       default_properties.height);
  gtk_widget_show (table);
  display_charlist(&curr_data);
  gtk_widget_show (applet);
  applet_widget_gtk_main ();

  return 0;
}
