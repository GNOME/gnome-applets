/* charpick.c  This is a gnome panel applet that allow users to select
 * accented (and other) characters to be pasted into other apps.
 */

#include <config.h>
#include <panel-applet.h>
#include <egg-screen-help.h>
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
  
  if (curr_data->last_toggle_button)
    gtk_toggle_button_set_state (curr_data->last_toggle_button, FALSE);

  curr_data->last_toggle_button = NULL;
  return TRUE;
}

static gint
toggle_button_toggled_cb(GtkToggleButton *button, gpointer data)
{
  charpick_data *curr_data = data;
  GtkClipboard *clipboard;
  gint button_index;
  gint last_index = curr_data->last_index;
  gboolean toggled;
   
  button_index = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "index")); 
  toggled = gtk_toggle_button_get_active (button); 

  if (toggled)
  { 
    if (curr_data->last_toggle_button && (button != curr_data->last_toggle_button))
    	gtk_toggle_button_set_active (curr_data->last_toggle_button,
    				      FALSE);
    				      
    curr_data->last_toggle_button = button; 
    gtk_widget_grab_focus(curr_data->applet);
    /* set selected_char */
    curr_data->selected_char = curr_data->charlist[button_index];
    /* set this? widget as the selection owner */
    gtk_selection_owner_set (curr_data->applet,
	  		     GDK_SELECTION_PRIMARY,
                             GDK_CURRENT_TIME); 
    gtk_selection_owner_set (curr_data->applet,
	  		     GDK_SELECTION_CLIPBOARD,
                             GDK_CURRENT_TIME); 
    curr_data->last_index = button_index;
  }	
	     
  return TRUE;
}

/* This is a hack around the fact that gtk+ doesn't
 * propogate button presses on button2/3.
 */
static gboolean
button_press_hack (GtkWidget      *widget,
		   GdkEventButton *event,
		   GtkWidget      *applet)
{
    if (event->button == 3 || event->button == 2) {
	gtk_propagate_event (applet, (GdkEvent *) event);

	return TRUE;
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
    case 'd' : p_curr_data->charlist = 
               p_curr_data->default_charlist;
               break;
    default :
    		return FALSE;
    }

  p_curr_data->last_index = NO_LAST_INDEX;
  p_curr_data->last_toggle_button = NULL;
  build_table(p_curr_data);
  return TRUE;
}

/* creates table of buttons, sets up their callbacks, and packs the table in
   the event box */

void
build_table(charpick_data *p_curr_data)
{
  GtkWidget *box;
  GtkWidget *toggle_button;
  gint size;
  gint i, num;
  
  if (p_curr_data->box)
    gtk_widget_destroy(p_curr_data->box);
  if (p_curr_data->panel_vertical == TRUE)
    box = gtk_vbox_new (TRUE, 0);
  else
    box = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (box);
  p_curr_data->box = box;
  num = strlen (p_curr_data->charlist);

  for (i=0; i < num; i++) {
    gchar *str_utf8;
    gchar currstr[2];
    currstr[0] = p_curr_data->charlist[i];
    currstr[1] = '\0';
 
    str_utf8 = g_convert (currstr, -1, "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
    toggle_button = gtk_toggle_button_new_with_label (str_utf8);
    g_free (str_utf8);
    gtk_box_pack_start (GTK_BOX (box), toggle_button, TRUE, TRUE, 0);
    gtk_button_set_relief(GTK_BUTTON(toggle_button), GTK_RELIEF_NONE);
    g_object_set_data (G_OBJECT (toggle_button), "index", GINT_TO_POINTER (i));
    gtk_signal_connect (GTK_OBJECT (toggle_button), "toggled",
                        (GtkSignalFunc) toggle_button_toggled_cb,
                        p_curr_data);
    gtk_signal_connect (GTK_OBJECT (toggle_button), "button_press_event", 
                        (GtkSignalFunc) button_press_hack, p_curr_data->applet);
  }

  gtk_container_add (GTK_CONTAINER(p_curr_data->frame), box);
  gtk_widget_show_all (p_curr_data->frame);
  
  /* a fudge factor is applied to make less space wasted.
     this is needed now that we have capital letters and the new 
     2 x 4 default layout with larger cells that required.
     The right thing to do would be to make width of cells setable
     seperately from height, or to make aspect ratio setable.
   */  
#if 0
  gtk_widget_set_usize(table, 
                       ((cols * size * 2) / 3), 
                       (rows * size));

#endif

  p_curr_data->last_index = NO_LAST_INDEX;
  p_curr_data->last_toggle_button = NULL;
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
about (BonoboUIComponent *uic,
       charpick_data     *curr_data,
       const char        *verb)
{
  static GtkWidget *about_box = NULL;
  GdkPixbuf   	   *pixbuf;
  GError      	   *error     = NULL;
  gchar            *file;
   
  const char *authors[] = {
	  /* If your charset supports it, please use U00F1 to replace the "n"
	   * in "Muniz". */
	  _("Alexandre Muniz <munizao@xprt.net>"),
	  NULL
  };

  const gchar *documenters[] = {
	  NULL
  };

  const gchar *translator_credits = _("translator_credits");

  if (about_box) {
	gtk_window_set_screen (GTK_WINDOW (about_box),
			       gtk_widget_get_screen (curr_data->applet));
	gtk_window_present (GTK_WINDOW (about_box));
	return;
  }
  
  file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "charpick.png", FALSE, NULL);
  pixbuf = gdk_pixbuf_new_from_file (file, &error);
  g_free (file);
   
  if (error) {
  	  g_warning (G_STRLOC ": cannot open %s: %s", file, error->message);
	  g_error_free (error);
  }
  
  about_box = gnome_about_new (_("Character Palette"),
			       CHARPICK_VERSION,
			       _("Copyright (C) 1998"),
			       _("Gnome Panel applet for selecting strange "
			         "characters that are not on my keyboard. "
				 "Released under GNU General Public Licence."),
			       authors,
			       documenters,
			       strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
			       pixbuf);
  
  if (pixbuf) 
  	gdk_pixbuf_unref (pixbuf);
   
  gtk_window_set_screen (GTK_WINDOW (about_box),
			 gtk_widget_get_screen (curr_data->applet));
  gtk_window_set_wmclass (GTK_WINDOW (about_box), "character palette", "Character Palette");
  gnome_window_icon_set_from_file (GTK_WINDOW (about_box), GNOME_ICONDIR"/charpick.png");

  gtk_signal_connect(GTK_OBJECT(about_box), "destroy",
		     GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about_box);
  gtk_widget_show(about_box);
  return;
}


static void
help_cb (BonoboUIComponent *uic,
	 charpick_data     *curr_data,
	 const char        *verb)
{
  GError *error = NULL;

  egg_help_display_on_screen (
		"char-palette", NULL,
		gtk_widget_get_screen (curr_data->applet),
		&error);

  if (error) { /* FIXME: the user needs to see this */
    g_warning ("help error: %s\n", error->message);
    g_error_free (error);
    error = NULL;
  }
}

static void
applet_destroy (GtkWidget *widget, gpointer data)
{
  charpick_data *curr_data = data;
  GtkTooltips *tooltips;
  GtkWidget *applet = curr_data->applet;

  g_return_if_fail (curr_data);
  
  if (curr_data->default_charlist)
      g_free (curr_data->default_charlist);
  curr_data->default_charlist = NULL;
  
  tooltips = g_object_get_data (G_OBJECT (applet), "tooltips");
  if (tooltips) {
      g_object_unref (tooltips);
      g_object_set_data (G_OBJECT (applet), "tooltips", NULL);
  }
  
  if (curr_data->propwindow)
    gtk_widget_destroy (curr_data->propwindow);

  g_free (curr_data);
  
}

static const BonoboUIVerb charpick_applet_menu_verbs [] = {
        BONOBO_UI_UNSAFE_VERB ("Preferences", show_preferences_dialog),
        BONOBO_UI_UNSAFE_VERB ("Help",        help_cb),
        BONOBO_UI_UNSAFE_VERB ("About",       about),

        BONOBO_UI_VERB_END
};

void
set_atk_name_description (GtkWidget *widget, const gchar *name,
                          const gchar *description)
{
  AtkObject *aobj;
  aobj = gtk_widget_get_accessible (widget);
  /* return if gail is not loaded */
  if (GTK_IS_ACCESSIBLE (aobj) == FALSE)
     return;
  atk_object_set_name (aobj, name);
  atk_object_set_description (aobj, description);
}

static void
make_applet_accessible (GtkWidget *applet)
{
  GtkTooltips *tooltips;
  tooltips = gtk_tooltips_new ();
  g_object_ref (tooltips);
  gtk_object_sink (GTK_OBJECT (tooltips));
  g_object_set_data (G_OBJECT (applet), "tooltips", tooltips); 
  gtk_tooltips_set_tip (tooltips, applet, 
                      _("Insert special characters"), NULL);
  set_atk_name_description (applet, _("Character Palette"), NULL);
}

static gboolean
charpicker_applet_fill (PanelApplet *applet)
{
  charpick_data *curr_data;
  gchar *default_charlist_utf8;
  
  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/charpick.png");
  
  panel_applet_add_preferences (applet, "/schemas/apps/charpick/prefs", NULL);
   
  curr_data = g_new0 (charpick_data, 1);
  curr_data->selected_char = ' ';
  curr_data->last_index = NO_LAST_INDEX;
  curr_data->applet = GTK_WIDGET (applet);
  
  default_charlist_utf8 = panel_applet_gconf_get_string (applet, 
  							 "default_list", NULL);
  if (!default_charlist_utf8)
    curr_data->default_charlist = g_strdup (def_list);
  else {
    curr_data->default_charlist = g_convert (default_charlist_utf8, -1, 
    					     "ISO-8859-1", "UTF-8", 
  					      NULL, NULL, NULL);
    g_free (default_charlist_utf8);
  }
    
  curr_data->charlist = curr_data->default_charlist;

  /* Create table */
  curr_data->frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (curr_data->applet), curr_data->frame);
  build_table (curr_data);
  
  gtk_frame_set_shadow_type (GTK_FRAME(curr_data->frame), GTK_SHADOW_IN);

  
  gtk_signal_connect (GTK_OBJECT (curr_data->applet), "key_press_event",
		      (GtkSignalFunc) key_press_event, curr_data);

  gtk_selection_add_target (curr_data->applet, 
			    GDK_SELECTION_PRIMARY,
                            GDK_SELECTION_TYPE_STRING,
			    0);
  gtk_selection_add_target (curr_data->applet, 
			    GDK_SELECTION_CLIPBOARD,
                            GDK_SELECTION_TYPE_STRING,
			    0);
  gtk_signal_connect (GTK_OBJECT (curr_data->applet), "selection_get",
		      GTK_SIGNAL_FUNC (charpick_selection_handler),
		      curr_data);
  gtk_signal_connect (GTK_OBJECT (curr_data->applet), "selection_clear_event",
		      GTK_SIGNAL_FUNC (selection_clear_cb),
		      curr_data);
 
  make_applet_accessible (GTK_WIDGET (applet));
  
  /* session save signal */ 
  g_signal_connect (G_OBJECT (applet), "change_orient",
		    G_CALLBACK (applet_change_orient), curr_data);

  g_signal_connect (G_OBJECT (applet), "change_size",
		    G_CALLBACK (applet_change_pixel_size), curr_data);
		    
  g_signal_connect (G_OBJECT (applet), "destroy",
  		    G_CALLBACK (applet_destroy), curr_data);
  
  gtk_widget_show_all (GTK_WIDGET (applet));
  
  panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
                                     NULL,
			             "GNOME_CharpickerApplet.xml",
                                     NULL,
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
			     PANEL_TYPE_APPLET,
			     "char-palette",
			     "0",
			     charpicker_applet_factory,
			     NULL)

