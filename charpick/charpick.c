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
static const gchar *def_list = "áàéíñóœ©";
static gunichar def_code[] = {225, 224, 233, 237, 241, 243, 189, 169, 1579, -1};

/* aacute, agrave, acirc, atilde, auml. aring, aelig, ordf */
static const gchar *a_list = "áàâãäåæª";
static gunichar a_code[] = {225, 224, 226, 227, 228, 229, 230, 170, -1};
static const gchar *cap_a_list = "ÁÀÂÃÄÅÆª";
static gunichar cap_a_code[] = {192, 193, 194, 195, 196, 197, 198, 170, -1}; 
/* ccedil, cent, copy */
static const gchar *c_list = "ç¢©";
static gunichar c_code[] = {231, 162, 169, -1};
static const gchar *cap_c_list = "Ç¢©";
static gunichar cap_c_code[] = {199, 162, 169, -1};
/* eacute, egrave, ecirc, euml, aelig */
static const gchar *e_list = "éèêëæ";
static gunichar e_code[] = {232, 233, 234, 235, 230, -1};
static const gchar *cap_e_list = "ÉÈÊËÆ";
static gunichar cap_e_code[] = {200, 201, 202, 203, 198, -1};
/* iacute, igrave, icirc, iuml */
static const gchar *i_list = "íìîï";
static gunichar i_code[] = {236, 237, 238, 239, -1};
static const gchar *cap_i_list = "ÍÌÎÏ";
static gunichar cap_i_code[] = {204, 205, 206, 207, -1};
/* */
static const gchar *l_list = "£";
static gunichar l_code[] = {163, -1};
/* micro */
static const gchar *m_list = "µ";
static gunichar m_code[] = {181, -1};
/* ntilde (this is the most important line in this program.) */
static const gchar *n_list = "ñ";
static gunichar n_code[] = {241, -1};
static const gchar *cap_n_list = "Ñ";
static gunichar cap_n_code[] = {209, -1};
/* oacute, ograve, ocirc, otilde, ouml, oslash, ordm */
static const gchar *o_list = "óòôõöøº";
static gunichar o_code[] = {242, 243, 244, 245, 246, 248, 176, -1};
static const gchar *cap_o_list = "ÓÒÔÕÖØº";
static gunichar cap_o_code[] = {210, 211, 212, 213, 214, 216, 176, -1};
/* para */
static const gchar *p_list = "¶";
static gunichar p_code[] = {182, -1};
/* reg */
static const gchar *r_list = "®";
static gunichar r_code[] = {174, -1};
/* szlig, sect, dollar */
static const gchar *s_list = "ß§$";
static gunichar s_code[] = {223, 167, 36, -1};
/* eth, thorn */
static const gchar *t_list = "ðþ";
static gunichar t_code[] = {240, 254, -1};
static const gchar *cap_t_list = "ÐÞ";
static gunichar cap_t_code[] = {208, 222, -1};
/* uacute, ugrave, ucirc, uuml */
static const gchar *u_list = "úùûü";
static gunichar u_code[] = {249, 250, 251, 252, -1};
static const gchar *cap_u_list = "ÚÙÛÜ";
static gunichar cap_u_code[] = {217, 218, 219, 220, -1};
/* yacute, yuml, yen Yes, there is no capital yuml in iso 8859-1.*/
static const gchar *y_list = "ýÿ¥";
static gunichar y_code[] = {253, 255, 165, -1};
static const gchar *cap_y_list = "Ýÿ¥";
static gunichar cap_y_code[] = {221, 255, 165, -1};

/* extra characters unrelated to the latin alphabet. All characters in 
   ISO-8859-1 should now be accounted for.*/
/* divide frac14 frac12 frac34 */
static const gchar *slash_list = "÷ŒœŸ";
static gunichar slash_code[] = {247, 188, 189, 190, -1};
/* not shy macr plusmn */
static const gchar *dash_list = "¬­¯±";
static gunichar dash_code[] = {172, 173, 175, 177, -1};
/* plusmn */
static const gchar *plus_list = "±";
static gunichar plus_code[] = {177, -1};
/* cedil */
static const gchar *comma_list = "ž";
static gunichar comma_code[] = {184, -1};
/* iquest */
static const gchar *quest_list = "¿";
static gunichar quest_code[] = {191, -1};
/* iexcl */
static const gchar *excl_list = "¡";
static gunichar excl_code[] = {161, -1};
/* brvbar */
static const gchar *pipe_list = "Š";
static gunichar pipe_code[] = {124, -1};
/* laquo raquo uml */
static const gchar *quote_list = "«»š";
static gunichar quote_code[] = {171, 187, 168, -1};
/* middot times */
static const gchar *ast_list = "·×";
static gunichar ast_code[] = {183, 215, -1};
/* curren, pound, yen, cent, dollar */
static const gchar *currency_list = "€£¥¢$";
static gunichar currency_code[] = {164, 163, 165, 162, 36, 8364, -1}; 
/* sup1 frac12 */
static const gchar *one_list = "¹œŒ";
static gunichar one_code[] = {185, 189, 188, -1};
/* sup2 frac12 */
static const gchar *two_list = "²œ";
static gunichar two_code[] = {178, 189, -1};
/* sup3 frac34 */
static const gchar *three_list = "³Ÿ ";
static gunichar three_code[] = {179, 190, -1};

/* sets the picked character as the selection when it gets a request */
static void
charpick_selection_handler(GtkWidget *widget,
			   GtkSelectionData *selection_data,
			   guint info,
			   guint time,
		           gpointer data)
{
  charpick_data *p_curr_data = data;
  gint num;
  gchar tmp[7];
  num = g_unichar_to_utf8 (p_curr_data->selected_unichar, tmp);
  tmp[num] = '\0';
  
  gtk_selection_data_set_text (selection_data, tmp, -1);

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

static gchar *
get_utf_string (gunichar *codes)
{
	gchar *string = NULL, tmp[7];
	gint i = 0;
	while (codes[i] != -1) {
		gint num;
		num = g_unichar_to_utf8 (codes[i], tmp);
		tmp[num] = 0;
		if (string) 
			string = g_strconcat (string, tmp, NULL);
		else
			string = g_strdup (tmp);
		i++;
	}
	return string;
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
    gunichar unichar;
    if (curr_data->last_toggle_button && (button != curr_data->last_toggle_button))
    	gtk_toggle_button_set_active (curr_data->last_toggle_button,
    				      FALSE);
    				      
    curr_data->last_toggle_button = button; 
    gtk_widget_grab_focus(curr_data->applet);
    unichar = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "unichar"));
    curr_data->selected_unichar = unichar;
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
  gint *code = NULL;
  gchar inputchar = event->keyval;

  switch (inputchar)
    {
    case 'a' : code = a_code;
               break;
    case 'A' : code = cap_a_code;
               break;
    case 'c' : code = c_code;
               break;
    case 'C' : code = cap_c_code;
               break;
    case 'e' : code = e_code;
               break;
    case 'E' : code = cap_e_code;
               break;
    case 'i' : code = i_code;
               break;
    case 'I' : code =  cap_i_code;
               break;
    case 'l' : code = l_code;
               break;
    case 'm' : code = m_code;
               break;
    case 'n' : code = n_code;
               break;
    case 'N' : code = cap_n_code;
               break;
    case 'o' : code = o_code;
               break;
    case 'O' : code = cap_o_code;
               break;
    case 'p' : code = p_code;
               break;
    case 'r' : code = r_code;
               break;
    case 's' : code = s_code;
               break;
    case 't' : code =  t_code;
               break;
    case 'T' : code = cap_t_code;
               break;
    case 'u' : code = u_code;
               break;
    case 'U' : code = cap_u_code;
               break;
    case 'y' : code = y_code;
               break;
    case 'Y' : code = cap_y_code;
               break;
    case '/' : code = slash_code;
               break;
    case '-' : code = dash_code;
               break;
    case '+' : code = plus_code;
               break;
    case ',' : code = comma_code;
               break;
    case '?' : code = quest_code;
               break;
    case '!' : code = excl_code;
               break;
    case '|' : code = pipe_code;
               break;
    case '\"' : code = quote_code;
               break;
    case '*' : code = ast_code;
               break;
    case '$' : code = currency_code;
               break;
    case '£' : code = currency_code;
               break;
    case '€' : code = currency_code;
               break;
    case '1' : code = one_code;
               break;
    case '2' : code = two_code;
               break;
    case '3' : code = three_code;
               break;
    case 'd' : code = NULL;
               break;
    default :
    		return FALSE;
    }
  if (p_curr_data->charlist)
    g_free (p_curr_data->charlist);
  if (code)
    p_curr_data->charlist = get_utf_string (code);
  else
    p_curr_data->charlist = g_strdup (p_curr_data->default_charlist);
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
  gint i = 0, len;
  gchar *charlist;
  
  if (p_curr_data->box)
    gtk_widget_destroy(p_curr_data->box);
  if (p_curr_data->panel_vertical == TRUE)
    box = gtk_vbox_new (TRUE, 0);
  else
    box = gtk_hbox_new (TRUE, 0);
  gtk_widget_show (box);
  p_curr_data->box = box;
  
  len = g_utf8_strlen (p_curr_data->charlist, -1);
  charlist = g_strdup (p_curr_data->charlist);
  for (i = 0; i < len; i++) {
    gchar label[7];
    gint num;
    g_utf8_strncpy (label, charlist, 1);
    charlist = g_utf8_next_char (charlist);
   
    toggle_button = gtk_toggle_button_new_with_label (label);
    gtk_box_pack_start (GTK_BOX (box), toggle_button, TRUE, TRUE, 0);
    gtk_button_set_relief(GTK_BUTTON(toggle_button), GTK_RELIEF_NONE);
    g_object_set_data (G_OBJECT (toggle_button), "unichar", 
				GINT_TO_POINTER(g_utf8_get_char (label)));
    gtk_signal_connect (GTK_OBJECT (toggle_button), "toggled",
                        (GtkSignalFunc) toggle_button_toggled_cb,
                        p_curr_data);
    gtk_signal_connect (GTK_OBJECT (toggle_button), "button_press_event", 
                        (GtkSignalFunc) button_press_hack, p_curr_data->applet);
  }
  
  gtk_container_add (GTK_CONTAINER(p_curr_data->applet), box);
  gtk_widget_show_all (p_curr_data->box);

  if (p_curr_data->panel_vertical == TRUE)
     gtk_widget_set_size_request (box, p_curr_data->panel_size,  -1);
  else
     gtk_widget_set_size_request (box, -1, p_curr_data->panel_size+5);

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
  GdkAtom utf8_atom;
  
  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/charpick.png");
  
  panel_applet_add_preferences (applet, "/schemas/apps/charpick/prefs", NULL);
   
  curr_data = g_new0 (charpick_data, 1);
  curr_data->last_index = NO_LAST_INDEX;
  curr_data->applet = GTK_WIDGET (applet);

  curr_data->default_charlist =  panel_applet_gconf_get_string (applet, 
  							 "default_list", NULL);
  if (!curr_data->default_charlist)
      curr_data->default_charlist = get_utf_string (def_code);    
  curr_data->charlist = g_strdup (curr_data->default_charlist);
 
  curr_data->panel_size = panel_applet_get_size (applet);
  
  build_table (curr_data);
    
  g_signal_connect (G_OBJECT (curr_data->applet), "key_press_event",
		             G_CALLBACK (key_press_event), curr_data);

  utf8_atom = gdk_atom_intern ("UTF8_STRING", FALSE);
  gtk_selection_add_target (curr_data->applet, 
			    GDK_SELECTION_PRIMARY,
                            utf8_atom,
			    0);
  gtk_selection_add_target (curr_data->applet, 
			    GDK_SELECTION_CLIPBOARD,
                            utf8_atom,
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

