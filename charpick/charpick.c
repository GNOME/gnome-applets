/* charpick.c  This is a gnome panel applet that allow users to select
 * accented (and other) characters to be pasted into other apps.
 */

#include <config.h>
#include <applet-widget.h>
#include "charpick.h"


static GtkWidget * event_box;
static charpick_button_cb_data button_cb_data[MAX_BUTTONS];
charpick_data curr_data;

/* This stuff assumes that this program is being run in an environment
 * that uses ISO-8859-1 (latin-1) as its native character code.
 */

/* The comment for each char list has the html entity names of the chars */
/* aacute, agrave, acirc, atilde, auml. aring, aelig, ordf */
static const gchar *a_list = "áàâãäåæª";
static const gchar *cap_a_list = "ÁÀÂÃÄÅÆª";
/* ccedil, cent, copy */
static const gchar *c_list = "ç¢©";
static const gchar *cap_c_list = "Ç¢©";
/* eacute, egrave, ecirc, euml, aelig */
static const gchar *e_list = "éèêëæ";
static const gchar *cap_e_list = "ÉÈÊËÆ";
/* iacute, igrave, icirc, iuml */
static const gchar *i_list = "íìîï";
static const gchar *cap_i_list = "ÍÌÎÏ";
/* micro */
static const gchar *m_list = "µ";
/* ntilde (this is the most important line in this program.) */
static const gchar *n_list = "ñ";
static const gchar *cap_n_list = "Ñ";
/* oacute, ograve, ocirc, otilde, ouml, oslash, ordm */
static const gchar *o_list = "óòôõöøº";
static const gchar *cap_o_list = "ÓÒÔÕÖØº";
/* para */
static const gchar *p_list = "¶";
/* reg */
static const gchar *r_list = "®";
/* szlig, sect */
static const gchar *s_list = "ß§";
/* uacute, ugrave, ucirc, uuml */
static const gchar *u_list = "úùûü";
static const gchar *cap_u_list = "ÚÙÛÜ";
/* yacute, yuml Yes, there is no capital yuml in iso 8859-1.*/
static const gchar *y_list = "ýÿ";
static const gchar *cap_y_list = "Ýÿ";

/* extra characters unrelated to the latin alphabet. All of them should
   go in eventually, but some are less obvious where they should go */
/* divide frac14 frac12 frac34 */
static const gchar *slash_list = "÷¼½¾";
/* not shy macr plusmn*/
static const gchar *dash_list = "¬­¯±";
/* iquest */
static const gchar *quest_list = "¿";
/* iexcl */
static const gchar *excl_list = "¡";
/* brvbar */
static const gchar *pipe_list = "¦";
/* laquo raquo uml*/
static const gchar *quote_list = "«»¨";
/* middot times*/
static const gchar *ast_list = "·×";

/* sets the picked character as the selection when it gets a request */
static void
charpick_selection_handler(GtkWidget *widget,
			   GtkSelectionData *selection_data,
			   guint info,
			   guint time,
		           gpointer data)
{
  charpick_data *p_curr_data = data;

  gtk_selection_data_set(selection_data,
			 GDK_SELECTION_TYPE_STRING,
			 8,
			 &p_curr_data->selected_char,
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
  /* let's show no more characters than we have, or than we have space for. */
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
  gint rows = cb_data->p_curr_data->properties->rows;
  gint cols = cb_data->p_curr_data->properties->cols;
  gint button_index = cb_data->button_index;
  gint last_index = cb_data->p_curr_data->last_index;
  if ((GTK_TOGGLE_BUTTON (cb_data->p_curr_data->toggle_buttons[button_index])->active))
  {
    if ((last_index != NO_LAST_INDEX) && (last_index != button_index))
      gtk_toggle_button_set_state
        (GTK_TOGGLE_BUTTON (cb_data->p_curr_data->toggle_buttons[last_index]), 
         FALSE);    
    gtk_widget_grab_focus(cb_data->p_curr_data->event_box);
    /* set selected_char */
    cb_data->p_curr_data->selected_char = 
    cb_data->p_curr_data->charlist[button_index];
    /* set this? widget as the selection owner */
    gtk_selection_owner_set (cb_data->p_curr_data->event_box,
	  		     GDK_SELECTION_PRIMARY,
                             GDK_CURRENT_TIME);
  cb_data->p_curr_data->last_index = button_index;
  }		     
  return TRUE;
}

/* clicks on the toggle buttons with mouse buttons other than 1 go to the 
   applet */

static int
button_press_cb (GtkWidget *widget, GdkEventButton *event)
{
  if (event->button > 1)
  {
    return gtk_widget_event (curr_data.applet, (GdkEvent *)event);
  }
  return TRUE;
}

static gint
key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  charpick_data *p_curr_data = data;
  gchar inputchar = event->keyval;
  switch (inputchar)
    {
    case 'a' : p_curr_data->charlist = a_list;
               break;
    case 'A' : p_curr_data->charlist = cap_a_list;
               break;
    case 'c' : p_curr_data->charlist = c_list;
               break;
    case 'C' : p_curr_data->charlist = cap_c_list;
               break;
    case 'e' : p_curr_data->charlist = e_list;
               break;
    case 'E' : p_curr_data->charlist = cap_e_list;
               break;
    case 'i' : p_curr_data->charlist = i_list;
               break;
    case 'I' : p_curr_data->charlist = cap_i_list;
               break;
    case 'm' : p_curr_data->charlist = m_list;
               break;
    case 'n' : p_curr_data->charlist = n_list;
               break;
    case 'N' : p_curr_data->charlist = cap_n_list;
               break;
    case 'o' : p_curr_data->charlist = o_list;
               break;
    case 'O' : p_curr_data->charlist = cap_o_list;
               break;
    case 'p' : p_curr_data->charlist = p_list;
               break;
    case 'r' : p_curr_data->charlist = r_list;
               break;
    case 's' : p_curr_data->charlist = s_list;
               break;
    case 'u' : p_curr_data->charlist = u_list;
               break;
    case 'U' : p_curr_data->charlist = cap_u_list;
               break;
    case 'y' : p_curr_data->charlist = y_list;
               break;
    case 'Y' : p_curr_data->charlist = cap_y_list;
               break;
    case '/' : p_curr_data->charlist = slash_list;
               break;
    case '-' : p_curr_data->charlist = dash_list;
               break;
    case '?' : p_curr_data->charlist = quest_list;
               break;
    case '!' : p_curr_data->charlist = excl_list;
               break;
    case '|' : p_curr_data->charlist = pipe_list;
               break;
    case '\"' : p_curr_data->charlist = quote_list;
               break;
    case '*' : p_curr_data->charlist = ast_list;
               break;
    }
  if (p_curr_data->last_index != NO_LAST_INDEX)
    gtk_toggle_button_set_state
        (GTK_TOGGLE_BUTTON (p_curr_data->toggle_buttons[p_curr_data->last_index]), 
         FALSE);
  p_curr_data->last_index = NO_LAST_INDEX;
  display_charlist(p_curr_data);
  return TRUE;
}


void
build_table(charpick_data *p_curr_data)
{
  GtkWidget *event_box = p_curr_data->event_box;
  GtkWidget *table = p_curr_data->table;
  GtkWidget * *toggle_button;
  GtkWidget * *label;
  gint rows, cols, size;
  gint i;
  rows = p_curr_data->properties->rows;
  cols = p_curr_data->properties->cols;
  size = p_curr_data->properties->size;
  toggle_button = p_curr_data->toggle_buttons;
  label = p_curr_data->labels;
  /*for (i=0;i<MAX_BUTTONS;i++)*/
  if (table)
    gtk_widget_destroy(table);
  table = gtk_table_new (rows, cols, TRUE);
  p_curr_data->table = table;
  for(i=0;i<(rows*cols);i++)
  {
    label[i] = gtk_label_new("Q"); /*if we get a list of Qs, something busted*/
  }
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
                              (i / cols),
                              (i / cols + 1));
    button_cb_data[i].button_index = i;
    button_cb_data[i].p_curr_data = p_curr_data;
    gtk_button_set_relief(GTK_BUTTON(toggle_button[i]), GTK_RELIEF_NONE);
    /* connect toggle signal for button to handler */
    gtk_signal_connect (GTK_OBJECT (toggle_button[i]), "toggled",
                        (GtkSignalFunc) toggle_button_toggled_cb,
                        &button_cb_data[i]);
    gtk_signal_connect (GTK_OBJECT (toggle_button[i]), "button_press_event", 
                        (GtkSignalFunc) button_press_cb, NULL);
  }
  gtk_container_add (GTK_CONTAINER(event_box), table);
  gtk_widget_show (event_box);
  /* a fudge factor is applied to make less space wasted.
     this is needed now that we have capital letters and the new 
     2 x 4 default layout with larger cells that required.
     The right thing to do would be to make width of cells setable
     seperately from height, or to make aspect ratio setable.
   */  
  gtk_widget_set_usize(table, 
                       ((cols * size * 2) / 3), 
                       (rows * size));
  gtk_widget_show (table);
  display_charlist(p_curr_data);
}

static gint applet_save_session(GtkWidget *widget, char *privcfgpath, 
                                char *globcfgpath, gpointer data)
{
  /*charpick_persistant_properties *properties = data;*/
  property_save(privcfgpath, curr_data.properties);
  return FALSE;
}

static void
about (AppletWidget *applet, gpointer data)
{
  static const char *authors[] = { "Alexandre Muñiz (munizao@cyberhighway.net)", NULL };
  GtkWidget *about_box;

  about_box = gnome_about_new (_("Character Picker"),
			       _("0.03"),
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
  GtkWidget *applet = NULL;
  GtkWidget *event_box = NULL;
  GtkWidget *table = NULL;
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
    2,
    2,
    17
  };
  /*
  charpick_data curr_data =
  {
    default_properties.default_charlist,
    ' ',
    NO_LAST_INDEX,
    toggle_button,
    label,
    table,
    event_box,
    applet,
    &default_properties
  };
  */
  curr_data.charlist = default_properties.default_charlist;
  curr_data.selected_char = ' ';
  curr_data.last_index = NO_LAST_INDEX;
  curr_data.toggle_buttons = toggle_button;
  curr_data.labels = label;
  curr_data.table = table;
  curr_data.event_box = event_box;
  curr_data.applet = applet;
  curr_data.properties = &default_properties;
  rows = default_properties.rows;
  cols = default_properties.cols;


  /* Initialize the i18n stuff */
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);
  /* initialize applet */
  applet_widget_init("charpick_applet", VERSION, argc, argv,
			      NULL, 0, NULL);

  /* create a new applet_widget*/
  applet = applet_widget_new("charpick_applet");
  curr_data.applet = applet; 
  /* in the rare case that the communication with the panel
     failed, error out */
  if (!applet)
    g_error("Can't create applet!\n");
  property_load(APPLET_WIDGET(applet)->privcfgpath, &default_properties);
  /* Create the event_box (needed to catch keypress and focus change events) */

  event_box = gtk_event_box_new ();
  curr_data.event_box = event_box; 
  GTK_WIDGET_SET_FLAGS (event_box, GTK_CAN_FOCUS);

  /* Create table */
  build_table(&curr_data);

  /*gtk_widget_grab_focus(event_box);*/

  /* Event signals */
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
  /* session save signal */ 
  gtk_signal_connect(GTK_OBJECT(applet),"save_session",
		     GTK_SIGNAL_FUNC(applet_save_session), 
                     &default_properties);

  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
				         "about",
					 GNOME_STOCK_MENU_ABOUT,
					 _("About..."),
					 about,
					 NULL);

  applet_widget_register_stock_callback (APPLET_WIDGET (applet),
				         "properties",
					 GNOME_STOCK_MENU_PROP,
					 _("Properties..."),
					 property_show,
					 &curr_data);

  applet_widget_add (APPLET_WIDGET (applet), event_box);

  gtk_widget_show (event_box);
  /*  gtk_widget_set_usize(table, 
                       default_properties.width, 
                       default_properties.height);*/
  /*  gtk_widget_show (table);*/
  /*display_charlist(&curr_data);*/
  gtk_widget_show (applet);
  applet_widget_gtk_main ();

  return 0;
}

