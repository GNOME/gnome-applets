/* charpick.c  This is a gnome panel applet that allow users to select
 * accented (and other) characters to be pasted into other apps.
 */

#include <config.h>
#include <panel-applet.h>
#include "charpick.h"


/* This stuff assumes that this program is being run in an environment
 * that uses ISO-8859-1 (latin-1) as its native character code.
 */

/* The comment for each char list has the html entity names of the chars */

/* This is the default list used when starting charpick the first time */
/* aacute, agrave, eacute, iacute, oacute, frac12, copy*/
static const gchar *def_list = "áàéíñó½©";

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
/* */
static const gchar *l_list = "£";
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
/* szlig, sect, dollar */
static const gchar *s_list = "ß§$";
/* eth, thorn */
static const gchar *t_list = "ðþ";
static const gchar *cap_t_list = "ÐÞ";
/* uacute, ugrave, ucirc, uuml */
static const gchar *u_list = "úùûü";
static const gchar *cap_u_list = "ÚÙÛÜ";
/* yacute, yuml, yen Yes, there is no capital yuml in iso 8859-1.*/
static const gchar *y_list = "ýÿ¥";
static const gchar *cap_y_list = "Ýÿ¥";

/* extra characters unrelated to the latin alphabet. All characters in 
   ISO-8859-1 should now be accounted for.*/
/* divide frac14 frac12 frac34 */
static const gchar *slash_list = "÷¼½¾";
/* not shy macr plusmn */
static const gchar *dash_list = "¬­¯±";
/* plusmn */
static const gchar *plus_list = "±";
/* cedil */
static const gchar *comma_list = "¸";
/* iquest */
static const gchar *quest_list = "¿";
/* iexcl */
static const gchar *excl_list = "¡";
/* brvbar */
static const gchar *pipe_list = "¦";
/* laquo raquo uml */
static const gchar *quote_list = "«»¨";
/* middot times */
static const gchar *ast_list = "·×";
/* curren, pound, yen, cent, dollar */
static const gchar *currency_list = "¤£¥¢$";
/* sup1 frac12 */
static const gchar *one_list = "¹½¼";
/* sup2 frac12 */
static const gchar *two_list = "²½";
/* sup3 frac34 */
static const gchar *three_list = "³¾ ";

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
  return;
}

/* untoggles the active toggle_button when we lose the selection */
static gint
selection_clear_cb (GtkWidget *widget, GdkEventSelection *event,
                    gpointer data)
{
  charpick_data *curr_data = data;
  
  gint last_index = curr_data->last_index;
  GtkWidget *toggle_button;

  toggle_button = curr_data->toggle_buttons[last_index];
  gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON(toggle_button), FALSE);
  curr_data->last_index = NO_LAST_INDEX;
  return TRUE;
}

/* this sets up the rows and columns according to the panel size and
   settings as appropriate */
static void
setup_rows_cols(charpick_data *p_curr_data, gint *rows, gint *cols)
{
  if (p_curr_data->properties->follow_panel_size)
  {
    int r,c;
    /* for vertical we only take 2/3 of the size as that is what is
       then used for the table */
    if (p_curr_data->panel_vertical)
      r = (p_curr_data->panel_size * 3) / (p_curr_data->properties->size * 2);
    else
      r = p_curr_data->panel_size / p_curr_data->properties->size;

    if (r < 1)
      r = 1;
    else if (r > p_curr_data->properties->min_cells)
      r = p_curr_data->properties->min_cells;

    if (p_curr_data->properties->min_cells % r == 0)
      c = p_curr_data->properties->min_cells / r;
    else
      c = (p_curr_data->properties->min_cells / r) + 1;
      
    if (p_curr_data->panel_vertical)
    {
      *rows = c;
      *cols = r;
    }
    else
    {
      *rows = r;
      *cols = c;
    }
  }
  else
  {
    *rows = p_curr_data->properties->rows;
    *cols = p_curr_data->properties->cols;
  }
}


/* displays a list of characters in labels on buttons*/
static void
display_charlist (charpick_data *data)
{
  guint rows;
  guint cols;
  const gchar *charlist = data->charlist;
  /* let's show no more characters than we have, or than we have space for. */
  guint numtoshow;
  guint i = 0; /* loop variable */
  gchar currstr[2];

  setup_rows_cols (data, &rows, &cols);

  numtoshow = MIN(rows * cols, strlen(charlist));

  /* reset the characters on the labels and reshow the buttons */
  for (i=0;i<numtoshow;i++)
  {
    gchar *str_utf8;
    currstr[0] = charlist[i];
    currstr[1] = '\0';
    
    str_utf8 = g_locale_to_utf8 (currstr, -1, NULL, NULL, NULL);
    if (str_utf8) {
      gtk_label_set_text(GTK_LABEL(data->labels[i]), str_utf8);
      g_free (str_utf8);
    }
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
  charpick_data *curr_data = data;
  GtkClipboard *clipboard;
  gint button_index;
  gint last_index = curr_data->last_index;
   
  button_index = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "index"));  

  if ((GTK_TOGGLE_BUTTON (curr_data->toggle_buttons[button_index])->active))
  {
    if ((last_index != NO_LAST_INDEX) && (last_index != button_index))
      gtk_toggle_button_set_state
       (GTK_TOGGLE_BUTTON (curr_data->toggle_buttons[last_index]), FALSE);   
        
    gtk_widget_grab_focus(curr_data->event_box);
    /* set selected_char */
    curr_data->selected_char = curr_data->charlist[button_index];
    /* set this? widget as the selection owner */
    gtk_selection_owner_set (curr_data->event_box,
	  		     GDK_SELECTION_PRIMARY,
                             GDK_CURRENT_TIME); 
    curr_data->last_index = button_index;
  }	
	     
  return TRUE;
}

/* clicks on the toggle buttons with mouse buttons other than 1 go to the 
   applet */

static int
button_press_cb (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  charpick_data *curr_data = data;
g_print ("button press \n");  
  if (event->button > 1)
  {
    return gtk_widget_event (curr_data->applet, (GdkEvent *)event);
  }

  return FALSE;
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
    case 'l' : p_curr_data->charlist = l_list;
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
    case 't' : p_curr_data->charlist = t_list;
               break;
    case 'T' : p_curr_data->charlist = cap_t_list;
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
    case '+' : p_curr_data->charlist = plus_list;
               break;
    case ',' : p_curr_data->charlist = comma_list;
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
    case '$' : p_curr_data->charlist = currency_list;
               break;
    case '£' : p_curr_data->charlist = currency_list;
               break;
    case '¤' : p_curr_data->charlist = currency_list;
               break;
    case '1' : p_curr_data->charlist = one_list;
               break;
    case '2' : p_curr_data->charlist = two_list;
               break;
    case '3' : p_curr_data->charlist = three_list;
               break;
    case ' ' : p_curr_data->charlist = 
	         p_curr_data->properties->default_charlist;
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

/* creates table of buttons, sets up their callbacks, and packs the table in
   the event box */

void
build_table(charpick_data *p_curr_data)
{
  GtkWidget *table = p_curr_data->table;
  GtkWidget * *toggle_button;
  GtkWidget * *label;
  gint rows, cols, size;
  gint i;
  size = p_curr_data->properties->size;
g_print ("build table \n");
  setup_rows_cols (p_curr_data, &rows, &cols);

  toggle_button = p_curr_data->toggle_buttons;
  label = p_curr_data->labels;
  /*for (i=0;i<MAX_BUTTONS_WITH_BUFFER;i++)*/
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
    gtk_button_set_relief(GTK_BUTTON(toggle_button[i]), GTK_RELIEF_NONE);
    /* connect toggle signal for button to handler */
    g_object_set_data (G_OBJECT (toggle_button[i]), "index", GINT_TO_POINTER (i));
    gtk_signal_connect (GTK_OBJECT (toggle_button[i]), "toggled",
                        (GtkSignalFunc) toggle_button_toggled_cb,
                        p_curr_data);
    gtk_signal_connect (GTK_OBJECT (toggle_button[i]), "button_press_event", 
                        (GtkSignalFunc) button_press_cb, p_curr_data);
  }
  
  gtk_container_add (GTK_CONTAINER(p_curr_data->event_box), table);
  gtk_widget_show (p_curr_data->event_box);
  
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

static void applet_change_pixel_size(PanelApplet *applet, gint size, gpointer data)
{
  charpick_data *curr_data = data;
  curr_data->panel_size = size;

  build_table (curr_data);
  return;
}

static void applet_change_orient(PanelApplet *applet, PanelAppletOrient o, gpointer data)
{
  charpick_data *curr_data = data;
  if (o == PANEL_APPLET_ORIENT_UP ||
      o == PANEL_APPLET_ORIENT_DOWN)
    curr_data->panel_vertical = FALSE;
  else
    curr_data->panel_vertical = TRUE;
  build_table (curr_data);
  return;
}


static void
about (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  static const char *authors[] = { "Alexandre Muñiz <munizao@xprt.net", NULL };
  static GtkWidget *about_box = NULL;

  if (about_box != NULL)
  {
	  gdk_window_show(about_box->window);
	  gdk_window_raise(about_box->window);
	  return;
  }
  about_box = gnome_about_new (_("Character Picker"),
			       CHARPICK_VERSION,
			       _("Copyright (C) 1998"),
			       _("Gnome Panel applet for selecting strange "
			         "characters that are not on my keyboard. "
				 "Released under GNU General Public Licence."),
			       authors,
			       NULL,
			       NULL,
			       NULL);

  gtk_signal_connect(GTK_OBJECT(about_box), "destroy",
		     GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_box);
  gtk_widget_show(about_box);
  return;
}


static void
help_cb (BonoboUIComponent *uic, gpointer user_data, const gchar *verbname)
{
#ifdef FIXME
        GnomeHelpMenuEntry help_entry = { "charpick_applet",
                                          "index.html" };
        gnome_help_display(NULL, &help_entry);
#endif
}


static const BonoboUIVerb charpick_applet_menu_verbs [] = {
        BONOBO_UI_VERB ("Props", property_show),
        BONOBO_UI_VERB ("Help", help_cb),
        BONOBO_UI_VERB ("About", about),

        BONOBO_UI_VERB_END
};

static const char charpick_applet_menu_xml [] =
	"<popup name=\"button3\">\n"
	"   <menuitem name=\"Item 1\" verb=\"Props\" _label=\"Properties\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-properties\"/>\n"
	"   <menuitem name=\"Item 2\" verb=\"Help\" _label=\"Help\"\n"
	"             pixtype=\"stock\" pixname=\"gtk-help\"/>\n"
	"   <menuitem name=\"Item 3\" verb=\"About\" _label=\"About\"\n"
	"             pixtype=\"stock\" pixname=\"gnome-stock-about\"/>\n"
	"</popup>\n";


static gboolean
charpicker_applet_fill (PanelApplet *applet)
{
  charpick_data *curr_data;
  GtkWidget *event_box = NULL;
  GtkWidget *table = NULL;
  
  panel_applet_add_preferences (applet, "/schemas/apps/charpick/prefs", NULL);
   
  curr_data = g_new0 (charpick_data, 1);
  curr_data->properties = g_new0 (charpick_persistant_properties, 1);
  curr_data->charlist = def_list;
  curr_data->selected_char = ' ';
  curr_data->last_index = NO_LAST_INDEX;
  curr_data->toggle_buttons = g_new0 (GtkWidget *, MAX_BUTTONS_WITH_BUFFER);
  curr_data->labels = g_new0 (GtkWidget *, MAX_BUTTONS_WITH_BUFFER);
  curr_data->table = table;
  curr_data->event_box = event_box;
  curr_data->applet = GTK_WIDGET (applet);
  
/* FIXME: hook up to gconf */
  property_load(curr_data);
  if (!curr_data->properties->default_charlist)
    curr_data->properties->default_charlist = g_strdup (def_list);

  /* Create the event_box (needed to catch keypress and focus change events) */

  event_box = gtk_event_box_new ();
  curr_data->event_box = event_box; 
  GTK_WIDGET_SET_FLAGS (event_box, GTK_CAN_FOCUS);

  /* Create table */
  curr_data->frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (curr_data->frame), event_box);
  build_table (curr_data);
  
  gtk_frame_set_shadow_type (GTK_FRAME(curr_data->frame), GTK_SHADOW_IN);

  /* Event signals */
  gtk_signal_connect (GTK_OBJECT (event_box), "key_press_event",
		      (GtkSignalFunc) key_press_event, curr_data);
  /*
  gtk_signal_connect (GTK_OBJECT (applet), "focus_in_event",
		      (GtkSignalFunc) focus_in_event, NULL);

  gtk_signal_connect (GTK_OBJECT (applet), "focus_out_event",
		      (GtkSignalFunc) focus_out_event, NULL);
  */

  gtk_widget_set_events (curr_data->event_box, GDK_FOCUS_CHANGE_MASK|GDK_KEY_PRESS_MASK);

  /* selection handling for selected character */
  gtk_selection_add_target (curr_data->event_box, 
			    GDK_SELECTION_PRIMARY,
                            GDK_SELECTION_TYPE_STRING,
			    0);
  gtk_signal_connect (GTK_OBJECT (curr_data->event_box), "selection_get",
		      GTK_SIGNAL_FUNC (charpick_selection_handler),
		      curr_data);
  gtk_signal_connect (GTK_OBJECT (curr_data->event_box), "selection_clear_event",
		      GTK_SIGNAL_FUNC (selection_clear_cb),
		      curr_data);
 

  gtk_container_add (GTK_CONTAINER (applet), curr_data->frame);
  
  /* session save signal */ 
  g_signal_connect (G_OBJECT (applet), "change_orient",
		    G_CALLBACK (applet_change_orient), curr_data);

  g_signal_connect (G_OBJECT (applet), "change_size",
		    G_CALLBACK (applet_change_pixel_size), curr_data);
  
  gtk_widget_show_all (GTK_WIDGET (applet));
  
  panel_applet_setup_menu (PANEL_APPLET (applet),
			   charpick_applet_menu_xml,
			   charpick_applet_menu_verbs,
			   curr_data);

  return TRUE;
}

static gboolean
charpicker_applet_factory (PanelApplet *applet,
			   const gchar          *iid,
			   gpointer              data)
{
	gboolean retval = FALSE;
    
	if (!strcmp (iid, "OAFIID:GNOME_CharpickerApplet"))
		retval = charpicker_applet_fill (applet); 
    
	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_CharpickerApplet_Factory",
			     "Inserts characters",
			     "0",
			     charpicker_applet_factory,
			     NULL)

