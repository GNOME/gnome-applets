/* implements session controls for webcontrol */

#include "session.h"
#include "properties.h"
#include "webcontrol.h"

extern WebControl WC;
extern webcontrol_properties props;

/* fills up the props.browsers list - assumes it is empty */
static void
wc_load_browsers (void)
{
	void *i;
	GSList *list;
	gchar *key, *value, *temp;
	browser *b;

	list = props.browsers = NULL;

	i = gnome_config_init_iterator_sections ("/webcontrol_applet");

	gnome_config_iterator_next (i, &key, &value);

	while (i && key)
	{
		if (WC_IS_BROWSER (key))
		{
			b = g_malloc (sizeof (browser));
			temp = g_malloc (strlen (key) + 21);
			g_snprintf (temp, strlen (key) + 21,
				    "/webcontrol_applet/%s/", key);
			gnome_config_push_prefix (temp);
			
			b->name = g_strdup (WC_BROWSER_NAME (key));
			b->command = gnome_config_get_string ("command");
			b->newwin = gnome_config_get_string ("newwin=");
			if (!b->newwin 
			    || b->newwin[0] == '\020'         /* why?!! */
			    || !strcmp (b->newwin, ""))
			{
				g_free (b->newwin);
				b->newwin = NULL;
			}

			b->no_newwin = gnome_config_get_string ("no_newwin=");
			if (!b->no_newwin 
			    || b->no_newwin[0] == '\020'   
			    || !strcmp (b->no_newwin, ""))
			{
				g_free (b->no_newwin);
				b->no_newwin = NULL;
			}
			
			list = g_slist_append (list, (gpointer) b);

			gnome_config_pop_prefix ();
			g_free (temp);
		}			

		gnome_config_iterator_next (i, &key, &value);		
	}

	props.browsers = list;

	g_free (key);
	g_free (value);
	
	return;
}


void
wc_load_session (void)
{
	GSList *ptr;
	gchar *temp;

	wc_load_browsers ();

	gnome_config_push_prefix ("/webcontrol_applet/Properties/");
	
	props.newwindow = gnome_config_get_bool ("newwindow=false");
	props.show_check = gnome_config_get_bool ("show_check=false");
	props.show_clear = gnome_config_get_bool ("show_clear=false");
	props.clear_top = gnome_config_get_bool ("clear_top=true");
	props.show_go = gnome_config_get_bool ("show_go=true");
	props.use_mime = gnome_config_get_bool ("use_mime=true");

	props.width = gnome_config_get_int ("width=200");
	props.hist_len = gnome_config_get_int ("hist_len=10");

	temp = gnome_config_get_string ("curr_browser=Mozilla");

	for (ptr = props.browsers; ptr; ptr = ptr->next)
	{
		if (!strcmp (temp, B_LIST_NAME (ptr)))
		{
			props.curr_browser = ptr;
			break;
		}
	}	       
	
	/* if there is no current browser, use MIME handler instead */
	if (!ptr)
	{
		props.use_mime = TRUE;
	}

	gnome_config_pop_prefix ();

	return;
}
	
	
/* this has to be a separate function since we can't put anything into 
 * a non-existing widget at startup */
void
wc_load_history (GtkCombo *combo)
{
	GtkWidget *litem;
	gchar *key, *value;
	void *i;
	gint count = 0;
	
	i = gnome_config_init_iterator ("/webcontrol_applet/URLHistory/");

	gnome_config_iterator_next (i, &key, &value);

	while (key && i && ++count <= props.hist_len)
	{
		litem = gtk_list_item_new_with_label (value);
		gtk_widget_show (litem);

		gtk_container_add (GTK_CONTAINER (GTK_COMBO (combo)->list), litem);

		gnome_config_iterator_next (i, &key, &value);	
	}
	
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (combo)->entry), "");
	
	return;
}

	
gint
wc_save_session (GtkWidget *w, gpointer data)
{
	GList *ptr;
	GSList *b;
	GtkWidget *w_temp;
	gchar *s_temp;
	gchar h_item[5];      /* nobody's got more than 10,000 history items :) */
	gint hist_count = 0;
	gchar *b_key = NULL;
	gint b_key_size;
	
	gnome_config_push_prefix ("/webcontrol_applet/Properties/");
	
	gnome_config_set_bool ("newwindow", props.newwindow);
	gnome_config_set_bool ("show_check", props.show_check);
	gnome_config_set_bool ("show_clear", props.show_clear);
	gnome_config_set_bool ("clear_top", props.clear_top);
	gnome_config_set_bool ("show_go", props.show_go);
	gnome_config_set_bool ("use_mime", props.use_mime);

	gnome_config_set_int ("width", props.width);
	gnome_config_set_int ("hist_len", props.hist_len);

	if (props.curr_browser)
		gnome_config_set_string ("curr_browser", 
					 B_LIST_NAME (props.curr_browser));

	gnome_config_pop_prefix ();

	gnome_config_push_prefix ("/webcontrol_applet/URLHistory/");
	
	/* put each history item into the saved history */
	gnome_config_clean_section ("/webcontrol_applet/URLHistory/");
	for (ptr = GTK_LIST (GTK_COMBO (WC.input)->list)->children; 
	     ptr; ptr = ptr->next) 
	{
		w_temp = ptr->data;
		if (!w_temp)
			continue;		
      
		s_temp = GTK_LABEL (GTK_BIN (w_temp)->child)->label;		
		
		g_snprintf (h_item, 5, "%d", hist_count++);
		
		gnome_config_set_string (h_item, s_temp);		
	}       

	/* save browser configurations - move this eventually */
	for (b = props.browsers; b; b = b->next)
	{                                   
		b_key_size = strlen (B_LIST_NAME(b)) +  30;
		b_key = (gchar *) g_malloc (b_key_size);
		g_snprintf (b_key, b_key_size, "/webcontrol_applet/Browser: %s/", 
			    B_LIST_NAME (b));

		gnome_config_push_prefix (b_key);
		
		gnome_config_set_string ("command", B_LIST_COMMAND (b));
		gnome_config_set_string ("newwin", B_LIST_NEWWIN (b));
		gnome_config_set_string ("no_newwin", B_LIST_NO_NEWWIN (b));

		gnome_config_pop_prefix ();
		
		g_free (b_key);
	}

	gnome_config_sync ();

	/* you need to use the drop_all here since we're all writing to
	   one file, without it, things might not work too well */
	gnome_config_drop_all (); 
		
	/* make sure you return FALSE, otherwise your applet might not
	   work compeltely, there are very few circumstances where you
	   want to return TRUE. This behaves similiar to GTK events, in
	   that if you return FALSE it means that you haven't done
	   everything yourself, meaning you want the panel to save your
	   other state such as the panel you are on, position,
	   parameter, etc ... */
	return FALSE;
}




