/* properties.c -- properties dialog box and session management for character
 * picker applet. Much of this is adapted from modemlights/properties.c
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "charpick.h"

static void
update_default_list_cb (GtkEditable *editable, gpointer data)
{
  charpick_data *curr_data = data;
  PanelApplet *applet = PANEL_APPLET (curr_data->applet);
  gchar *text;
  
  text = gtk_editable_get_chars (editable, 0, -1);
  if (!text)
  	return;
  	
  if (g_utf8_strlen (text, -1) == 0) {
    gchar *old_text = g_convert(curr_data->default_charlist, -1, "UTF-8", 
  		                "ISO-8859-1", NULL, NULL, NULL);
    gint pos;
    gtk_editable_insert_text (editable, old_text, -1, &pos);
    g_free (old_text);
    return;
  }
  
  if (curr_data->default_charlist)
    g_free (curr_data->default_charlist	);
  curr_data->default_charlist = g_convert (text, -1, "ISO-8859-1", "UTF-8", 
  					   NULL, NULL, NULL);
  
  panel_applet_gconf_set_string (applet, "default_list", text, NULL);
  g_free (text);
  
}

static void default_chars_frame_create(charpick_data *curr_data)
{
  GtkWidget *propwindow = curr_data->propwindow;
  GtkWidget *tab_label;
  GtkWidget *frame;
  GtkWidget *default_list_hbox;
  GtkWidget *default_list_label;
  GtkWidget *default_list_entry;
  GtkWidget *explain_label;
  gchar *text_utf8;

  /* init widgets */
  frame = gtk_vbox_new(FALSE, 5);
  default_list_hbox = gtk_hbox_new(FALSE, 5);
  default_list_label = gtk_label_new(_("Default character list:"));
  default_list_entry = gtk_entry_new_with_max_length (MAX_BUTTONS);
  text_utf8 = g_convert (curr_data->default_charlist, -1, "UTF-8", 
  			 "ISO-8859-1", NULL, NULL, NULL);
  gtk_entry_set_text(GTK_ENTRY(default_list_entry), 
		     text_utf8);
  g_free (text_utf8);

  explain_label = gtk_label_new(_("These characters will appear when the panel"
                                  " is started. To return to this list, hit"
                                  " <space> while the applet has focus."));
  gtk_label_set_line_wrap(GTK_LABEL(explain_label), TRUE);
  /* pack the main vbox */
  gtk_box_pack_start (GTK_BOX(frame), default_list_hbox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(frame), explain_label, FALSE, FALSE, 5);
  /* default_list hbox */
  gtk_box_pack_start(GTK_BOX(default_list_hbox), default_list_label, FALSE, FALSE, 5);
  gtk_box_pack_start( GTK_BOX(default_list_hbox), default_list_entry, 
		      FALSE, FALSE, 5);

  g_signal_connect (G_OBJECT(default_list_entry), "changed", 
		    G_CALLBACK (update_default_list_cb), curr_data);

  gtk_widget_show_all(frame);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (propwindow)->vbox), frame, TRUE, TRUE, 0);
  
  return;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
#ifdef FIXME
	GnomeHelpMenuEntry help_entry = { "charpick_applet",
                                          "index.html#CHARPICKAPPLET-PREFS" };
	gnome_help_display(NULL, &help_entry);
#endif
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
  charpick_data *curr_data = data;
  
  gtk_widget_destroy (curr_data->propwindow);
  curr_data->propwindow = NULL;
  
}

void
property_show(BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
  charpick_data *curr_data = data;
  
  /*if the properties dialog is already open, just raise it.*/
  if(curr_data->propwindow)
  {
    gtk_window_present(GTK_WINDOW (curr_data->propwindow->window));
    return;
  }
  curr_data->propwindow = gnome_property_box_new();
  curr_data->propwindow = gtk_dialog_new_with_buttons (_("Character Palette Properties"), 
  					    NULL,
					    GTK_DIALOG_DESTROY_WITH_PARENT,
					    GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					    NULL);
  /*size_frame_create();*/
  default_chars_frame_create(curr_data);
  g_signal_connect (G_OBJECT (curr_data->propwindow), "response",
  		    G_CALLBACK (response_cb), curr_data);
  		    
  gtk_widget_show_all(curr_data->propwindow);

  return;
}





