/* properties.c -- properties dialog box and session management for character
 * picker applet. Much of this is adapted from modemlights/properties.c
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "charpick.h"
#include <egg-screen-help.h>

static gboolean
update_default_list_cb (GtkWidget *editable, GdkEventFocus *event, gpointer data)
{
  charpick_data *curr_data = data;
  PanelApplet *applet = PANEL_APPLET (curr_data->applet);
  gchar *text;
  
  text = gtk_editable_get_chars (GTK_EDITABLE (editable), 0, -1);
  if (!text)
  	return FALSE;
  g_strstrip (text);	
  if (g_utf8_strlen (text, -1) == 0){
  	g_free (text);
  	return FALSE;
  }
  
  if (g_ascii_strcasecmp (curr_data->charlist, curr_data->default_charlist) == 0) {
  	g_free (curr_data->charlist);
  	curr_data->charlist = g_strdup (text);
  	build_table (curr_data);
  }	
  if (curr_data->default_charlist)
    g_free (curr_data->default_charlist);
  curr_data->default_charlist = g_strdup (text);
  
  panel_applet_gconf_set_string (applet, "default_list", text, NULL);

  g_free (text);
  
  return FALSE;
}

static void
set_atk_relation (GtkWidget *label, GtkWidget *widget)
{
  AtkObject *atk_widget;
  AtkObject *atk_label;
  AtkRelationSet *relation_set;
  AtkRelation *relation;
  AtkObject *targets[1];

  atk_widget = gtk_widget_get_accessible (widget);
  atk_label = gtk_widget_get_accessible (label);
  
  /* set label-for relation */
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);	

  /* return if gail is not loaded */
  if (GTK_IS_ACCESSIBLE (atk_widget) == FALSE)
    return;

  /* set label-by relation */
  relation_set = atk_object_ref_relation_set (atk_widget);
  targets[0] = atk_label;
  relation = atk_relation_new (targets, 1, ATK_RELATION_LABELLED_BY);
  atk_relation_set_add (relation_set, relation);
  g_object_unref (G_OBJECT (relation)); 
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
 
  /* init widgets */
  frame = gtk_vbox_new(FALSE, 5);
  default_list_hbox = gtk_hbox_new(FALSE, 5);
  default_list_label = gtk_label_new_with_mnemonic(_("_Default character list:"));
  default_list_entry = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY(default_list_entry), MAX_BUTTONS);
  gtk_entry_set_text(GTK_ENTRY(default_list_entry), 
		     curr_data->default_charlist);
  set_atk_relation (default_list_label, default_list_entry);
  set_atk_name_description (default_list_entry, _("Default list of characters"), 
              _("Set the default character list here"));

  explain_label = gtk_label_new(_("These characters will appear when the panel"
                                  " is started. To return to this list, hit"
                                  " <d> while the applet has focus."));
  gtk_label_set_line_wrap(GTK_LABEL(explain_label), TRUE);
  /* pack the main vbox */
  gtk_box_pack_start (GTK_BOX(frame), default_list_hbox, FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX(frame), explain_label, FALSE, FALSE, 5);
  /* default_list hbox */
  gtk_box_pack_start(GTK_BOX(default_list_hbox), default_list_label, FALSE, FALSE, 5);
  gtk_box_pack_start( GTK_BOX(default_list_hbox), default_list_entry, 
		      FALSE, FALSE, 5);

  g_signal_connect (G_OBJECT (default_list_entry), "focus_out_event",
  		            G_CALLBACK (update_default_list_cb), curr_data);

  gtk_widget_show_all(frame);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (propwindow)->vbox), frame, TRUE, TRUE, 0);
  
  return;
}

static void
phelp_cb (GtkDialog *dialog, gint tab, gpointer data)
{
  GError *error = NULL;

  egg_help_display_on_screen (
		"char-palette", "charpick-prefs",
		gtk_window_get_screen (GTK_WINDOW (dialog)),
		&error);

  if (error) { /* FIXME: the user needs to see this */
    g_warning ("help error: %s\n", error->message);
    g_error_free (error);
    error = NULL;
  }
}

static void
response_cb (GtkDialog *dialog, gint id, gpointer data)
{
  charpick_data *curr_data = data;

  if(id == GTK_RESPONSE_HELP){
    phelp_cb (dialog,id,data);
    return;
  }
  
  gtk_widget_destroy (curr_data->propwindow);
  curr_data->propwindow = NULL;
  
}

void
show_preferences_dialog (BonoboUIComponent *uic,
			 charpick_data     *curr_data,
			 const gchar       *verbname)
{
  if (curr_data->propwindow) {
    gtk_window_set_screen (GTK_WINDOW (curr_data->propwindow),
			   gtk_widget_get_screen (curr_data->applet));
    gtk_window_present (GTK_WINDOW (curr_data->propwindow));
    return;
  }

  curr_data->propwindow = gtk_dialog_new_with_buttons (_("Character Palette Preferences"), 
  					    NULL,
					    GTK_DIALOG_DESTROY_WITH_PARENT,
					    GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					    GTK_STOCK_HELP, GTK_RESPONSE_HELP,
					    NULL);
  gtk_window_set_screen (GTK_WINDOW (curr_data->propwindow),
			 gtk_widget_get_screen (curr_data->applet));
  gtk_dialog_set_default_response (GTK_DIALOG (curr_data->propwindow), GTK_RESPONSE_CLOSE);
  /*size_frame_create();*/
  default_chars_frame_create(curr_data);
  g_signal_connect (G_OBJECT (curr_data->propwindow), "response",
  		    G_CALLBACK (response_cb), curr_data);
  		    
  gtk_widget_show_all (curr_data->propwindow);
}
