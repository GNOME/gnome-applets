/*
 * Mini-Commander Applet
 * Copyright (C) 1998, 1999 Oliver Maruhn <oliver@maruhn.com>
 *
 * Author: Oliver Maruhn <oliver@maruhn.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <config.h>
#include <gnome.h>
#include <panel-applet.h>
#include <gconf/gconf.h>
#include <string.h>
#include <panel-applet-gconf.h>

#include "preferences.h"
#include "command_line.h" /* needed for style changes */
#include "history.h"
#include "message.h"
#include "mini-commander_applet.h"

static void reset_temporary_prefs(void);
static void phelp_cb (void);

static gboolean
pattern_entry_changed_signal(GtkWidget *entry_widget, GdkEventFocus *event, gpointer data)
{
    PanelApplet *applet = g_object_get_data (G_OBJECT (entry_widget), "applet");
    properties *prop = g_object_get_data (G_OBJECT (entry_widget), "prop");
    GConfValue *macro_patterns;
    GSList *pattern_list = NULL;
    gchar *new;
    gint i, current;
        
    new = gtk_editable_get_chars (GTK_EDITABLE (entry_widget), 0, -1);
    current = GPOINTER_TO_INT (data);
    if (!new)
    	return FALSE;

    if (!g_strcasecmp (new, prop->macro_pattern[current]))
    	return FALSE;
	 
    free(prop->macro_pattern[current]);
    prop->macro_pattern[current] = new;
    
    /* save the patterns */
    for(i=0; i<=MAX_NUM_MACROS-1; i++) {
        GConfValue *entry;
	    
	entry = gconf_value_new (GCONF_VALUE_STRING);
	gconf_value_set_string (entry, prop->macro_pattern[i]);
	pattern_list = g_slist_append (pattern_list, entry);	   
    
    }  
    
    macro_patterns = gconf_value_new (GCONF_VALUE_LIST);
    if (pattern_list) {
        gconf_value_set_list_type (macro_patterns, GCONF_VALUE_STRING);
        gconf_value_set_list (macro_patterns, pattern_list);
        panel_applet_gconf_set_value (applet, "macro_patterns", macro_patterns, NULL);               
    }
#if 0
    if(prop->macro_regex[current] != NULL)
	free(prop->macro_regex[current]);
#endif
    if(prop->macro_pattern[current] && prop->macro_pattern[current][0] != '\0')
    {
	prop->macro_regex[current] = malloc(sizeof(regex_t));
	/* currently we don't care about syntax errors in regex patterns */
	regcomp(prop->macro_regex[current], prop->macro_pattern[current], REG_EXTENDED);
    }
    else
    {
	prop->macro_regex[current] = NULL;
    }
    return FALSE;
}

static gboolean
command_entry_changed_signal(GtkWidget *entry_widget, GdkEventFocus *event, gpointer data)
{
    PanelApplet *applet = g_object_get_data (G_OBJECT (entry_widget), "applet");
    properties *prop = g_object_get_data (G_OBJECT (entry_widget), "prop");
    GConfValue *macro_command;
    GSList *command_list = NULL;
    gchar *new;
    gint i;
     
    new = gtk_editable_get_chars (GTK_EDITABLE (entry_widget), 0, -1);
    i = GPOINTER_TO_INT (data);
    if (!new)
    	return FALSE;
 
    if (!g_strcasecmp (new, prop->macro_command[i]))
    	return FALSE;
 
    free(prop->macro_command[i]);
    prop->macro_command[i] = new;
    
    /* save the commands */
    for(i=0; i<=MAX_NUM_MACROS-1; i++) {
        GConfValue *entry;
	    
	entry = gconf_value_new (GCONF_VALUE_STRING);
	gconf_value_set_string (entry, prop->macro_command[i]);
	command_list = g_slist_append (command_list, entry);	   
    
    }  
    
    macro_command = gconf_value_new (GCONF_VALUE_LIST);
    if (command_list) {
        gconf_value_set_list_type (macro_command, GCONF_VALUE_STRING);
        gconf_value_set_list (macro_command, command_list);
        panel_applet_gconf_set_value (applet, "macro_commands", macro_command, NULL);               
    }

    return FALSE;
}

static void
entry_integer_changed_signal(GtkWidget *entry_widget, int *data)
{
    *data = atoi(gtk_entry_get_text(GTK_ENTRY(entry_widget)));
}

static void
color_cmd_fg_changed_signal(GtkWidget *color_picker_widget,
                            guint r, guint b, guint g, guint a, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    properties *prop;
    gushort red, green, blue;
    
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    gnome_color_picker_get_i16 (GNOME_COLOR_PICKER (color_picker_widget),
    				&red,
    				&green,
    				&blue,
    				NULL);
    
    prop->cmd_line_color_fg_r = (int) red;
    prop->cmd_line_color_fg_g = (int) green; 
    prop->cmd_line_color_fg_b = (int) blue;
   
    command_entry_update_color (mcdata->entry, prop);
    
    panel_applet_gconf_set_int (applet, "cmd_line_color_fg_r", prop->cmd_line_color_fg_r,
    			        NULL);
    panel_applet_gconf_set_int (applet, "cmd_line_color_fg_g", prop->cmd_line_color_fg_g,
    			        NULL);
    panel_applet_gconf_set_int (applet, "cmd_line_color_fg_b", prop->cmd_line_color_fg_b,
    			        NULL);

    return;
    data = NULL;
}

static void
color_cmd_bg_changed_signal(GtkWidget *color_picker_widget, 
			    guint r, guint b, guint g, guint a, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    properties *prop;
    gushort red, green, blue;
    
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    gnome_color_picker_get_i16 (GNOME_COLOR_PICKER (color_picker_widget),
    				&red,
    				&green,
    				&blue,
    				NULL);
    
    prop->cmd_line_color_bg_r = (int) red;
    prop->cmd_line_color_bg_g = (int) green;
    prop->cmd_line_color_bg_b = (int) blue;
   
    command_entry_update_color (mcdata->entry, prop);

    panel_applet_gconf_set_int (applet, "cmd_line_color_bg_r", prop->cmd_line_color_bg_r,
    			        NULL);
    panel_applet_gconf_set_int (applet, "cmd_line_color_bg_g", prop->cmd_line_color_bg_g,
    			        NULL);
    panel_applet_gconf_set_int (applet, "cmd_line_color_bg_b", prop->cmd_line_color_bg_b,
    			        NULL);
    return;
    data = NULL;
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

    /* Set label-for relation */
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);	

    /* Check if gail is loaded */
    if (GTK_IS_ACCESSIBLE (atk_widget) == FALSE)
        return;
    
    /* Set labelled-by relation */
    relation_set = atk_object_ref_relation_set (atk_widget);
    targets[0] = atk_label;
    relation = atk_relation_new (targets, 1, ATK_RELATION_LABELLED_BY);
    atk_relation_set_add (relation_set, relation);
    g_object_unref (G_OBJECT (relation)); 
}

static void
properties_box_apply_signal(GnomePropertyBox *property_box_widget, gint page, gpointer data)
{
    int i;
#if 0
    /* macros */
    for(i = 0; i <= MAX_NUM_MACROS - 1; ++i)
	{
	    if(prop_tmp.macro_pattern[i] != (char *)NULL)
		{
		    free(prop.macro_pattern[i]);
		    prop.macro_pattern[i] = prop_tmp.macro_pattern[i];
		    prop_tmp.macro_pattern[i] = (char *)NULL;	    

		    if(prop.macro_regex[i] != NULL)
			free(prop.macro_regex[i]);
		    prop.macro_regex[i] = malloc(sizeof(regex_t));
		    /* currently we don't care about syntax errors in regex patterns */
		    regcomp(prop.macro_regex[i], prop.macro_pattern[i], REG_EXTENDED);
		}    
	    if(prop_tmp.macro_command[i] != (char *)NULL)
		{
		    free(prop.macro_command[i]);
		    prop.macro_command[i] = prop_tmp.macro_command[i];
		    prop_tmp.macro_command[i] = (char *)NULL;
		}
	}    

#endif
}

/* FIXME: port to gconf */
properties *
load_session(MCData *mcdata)
{
    PanelApplet *applet = mcdata->applet;
    GConfValue *history;
    GConfValue *macro_patterns, *macro_commands;
    GSList *pattern_list = NULL, *command_list = NULL;
    properties *prop;
    GError *error = NULL;
    int i;
    char section[MAX_COMMAND_LENGTH + 100];
    char default_macro_pattern[MAX_MACRO_PATTERN_LENGTH];
    char default_macro_command[MAX_COMMAND_LENGTH];
    
    prop = g_new0 (properties, 1);
    
    prop->show_time = FALSE;
    prop->show_date = FALSE;
    prop->show_default_theme = panel_applet_gconf_get_bool (applet, 
                                      			    "show_default_theme", 
                                      		  	    &error);
    if (error) {
	g_print ("%s \n", error->message);
	g_error_free (error);
	error = NULL;
    }
    prop->show_handle = panel_applet_gconf_get_bool (applet, "show_handle", NULL);
    prop->show_frame = panel_applet_gconf_get_bool(applet, "show_frame", NULL);

    prop->auto_complete_history = panel_applet_gconf_get_bool(applet, 
    							      "autocomplete_history",
    							      NULL);

    /* size */
    prop->normal_size_x = panel_applet_gconf_get_int(applet, "normal_size_x", NULL);
    prop->normal_size_x = CLAMP (prop->normal_size_x, 50, 200);
    prop->normal_size_y = panel_applet_gconf_get_int(applet, "normal_size_y", NULL);
    prop->normal_size_y = CLAMP (prop->normal_size_y, 5, 200);
    prop->reduced_size_x = 50;
    prop->reduced_size_y = 48;
    prop->cmd_line_size_y = 24;

    /* colors */
    prop->cmd_line_color_fg_r = panel_applet_gconf_get_int(applet, "cmd_line_color_fg_r",
    							   NULL);
    prop->cmd_line_color_fg_g = panel_applet_gconf_get_int(applet, "cmd_line_color_fg_g",
    							   NULL);
    prop->cmd_line_color_fg_b = panel_applet_gconf_get_int(applet, "cmd_line_color_fg_b",
    							   NULL);

    prop->cmd_line_color_bg_r = panel_applet_gconf_get_int(applet, "cmd_line_color_bg_r",
    							   NULL);
    prop->cmd_line_color_bg_g = panel_applet_gconf_get_int(applet, "cmd_line_color_bg_g",
    							   NULL);
    prop->cmd_line_color_bg_b = panel_applet_gconf_get_int(applet, "cmd_line_color_bg_b",
    							   NULL);
    macro_patterns = panel_applet_gconf_get_value (applet, "macro_patterns", NULL);
    macro_commands = panel_applet_gconf_get_value (applet, "macro_commands", NULL);
    
    if (macro_patterns &&  macro_commands) {
    	GSList *list1 = NULL, *list2 = NULL;
    	gint i = 0;
    	
        list1 = gconf_value_get_list (macro_patterns);
        list2 = gconf_value_get_list (macro_commands);
        while (list1 && list2) {
            GConfValue *value1 = list1->data;
            GConfValue *value2 = list2->data;
            const gchar *pattern = NULL, *command = NULL;
            
            pattern = gconf_value_get_string (value1);
            command = gconf_value_get_string (value2);
            
            free(prop->macro_pattern[i]);
	    prop->macro_pattern[i] = (char *)pattern;
            free(prop->macro_command[i]);
	    prop->macro_command[i] = (char *)command; 
	    
	    if(prop->macro_pattern[i][0] != '\0')
		{
		    prop->macro_regex[i] = malloc(sizeof(regex_t));
		    /* currently we don't care about syntax errors in regex patterns */
		    regcomp(prop->macro_regex[i], prop->macro_pattern[i], REG_EXTENDED);
		}
	    else
		{
		    prop->macro_regex[i] = NULL;
		}
		
            list1 = g_slist_next (list1);
            list2 = g_slist_next (list2);
            i++;
        }
    }
    else {    
    /* defaults */
    	for(i=0; i<=MAX_NUM_MACROS-1; i++)
	{
	    switch (i + 1) 
		/* set default macros */
		{
		case 1:
		    strcpy(default_macro_pattern, "^(http://.*)$");
		    strcpy(default_macro_command, "gnome-moz-remote \\1");
		    break;
		case 2:
		    strcpy(default_macro_pattern, "^(ftp://.*)");
		    strcpy(default_macro_command, "gnome-moz-remote \\1");
		    break;
		case 3:
		    strcpy(default_macro_pattern, "^(www\\..*)$");
		    strcpy(default_macro_command, "gnome-moz-remote http://\\1");
		    break;
		case 4:
		    strcpy(default_macro_pattern, "^(ftp\\..*)$");
		    strcpy(default_macro_command, "gnome-moz-remote ftp://\\1");
		    break;
		case 5:
		    strcpy(default_macro_pattern, "^lynx: *(.*)$");
		    strcpy(default_macro_command, "gnome-terminal -e \"sh -c 'lynx \\1'\"");
		    break;
		case 6:
		    strcpy(default_macro_pattern, "^term: *(.*)$");
		    strcpy(default_macro_command, "gnome-terminal -e \"sh -c '\\1'\"");
		    break;
		case 7:
		    strcpy(default_macro_pattern, "^xterm: *(.*)$");
		    strcpy(default_macro_command, "xterm -e sh -c '\\1'");
		    break;
		case 8:
		    strcpy(default_macro_pattern, "^nxterm: *(.*)$");
		    strcpy(default_macro_command, "nxterm -e sh -c '\\1'");
		    break;
		case 9:
		    strcpy(default_macro_pattern, "^rxvt: *(.*)$");
		    strcpy(default_macro_command, "rxvt -e sh -c '\\1'");
		    break;
		case 10:
		    strcpy(default_macro_pattern, "^less: *(.*)$");
		    strcpy(default_macro_command, "\\1 | gless");
		    break;
		case 11:
		    strcpy(default_macro_pattern, "^av: *(.*)$");
		    strcpy(default_macro_command, "set altavista search by Chad Powell; gnome-moz-remote --newwin http://www.altavista.net/cgi-bin/query?pg=q\\&kl=XX\\&q=$(echo '\\1'|sed -e ': p;s/+/%2B/;t p;: s;s/\\ /+/;t s;: q;s/\\\"/%22/;t q')");
		    break;
		case 12:
		    strcpy(default_macro_pattern, "^yahoo: *(.*)$");
		    strcpy(default_macro_command, "set yahoo search by Chad Powell; gnome-moz-remote --newwin http://ink.yahoo.com/bin/query?p=$(echo '\\1'|sed -e ': p;s/+/%2B/;t p;: s;s/\\ /+/;t s;: q;s/\\\"/%22/;t q')");
		    break;
		case 13:
		    strcpy(default_macro_pattern, "^fm: *(.*)$");
		    strcpy(default_macro_command, "set freshmeat search by Chad Powell; gnome-moz-remote --newwin http://core.freshmeat.net/search.php3?query=$(echo '\\1'|tr \" \" +)");
		    break;
		case 14:
		    strcpy(default_macro_pattern, "^dictionary: *(.*)$");
		    strcpy(default_macro_command, "set dictionary search by Chad Powell; gnome-moz-remote --newwin http://www.dictionary.com/cgi-bin/dict.pl?term=\\1");
		    break;
		case 15:
		    strcpy(default_macro_pattern, "^t$");
		    strcpy(default_macro_command, "gnome-terminal");
		    break;
		case 16:
		    strcpy(default_macro_pattern, "^nx$");
		    strcpy(default_macro_command, "nxterm");
		    break;
		case 17:
		    strcpy(default_macro_pattern, "^n$");
		    strcpy(default_macro_command, "netscape");
		    break;
		default:
		    strcpy(default_macro_pattern, "");
		    strcpy(default_macro_command, "");		   
		} 

		free(prop->macro_pattern[i]);
	        prop->macro_pattern[i] = g_strdup (default_macro_pattern);
            	free(prop->macro_command[i]);
	    	prop->macro_command[i] = g_strdup (default_macro_command); 
	    
	    	if(prop->macro_pattern[i][0] != '\0')
		{
		    prop->macro_regex[i] = malloc(sizeof(regex_t));
		    /* currently we don't care about syntax errors in regex patterns */
		    regcomp(prop->macro_regex[i], prop->macro_pattern[i], REG_EXTENDED);
		}
	   	else
		{
		    prop->macro_regex[i] = NULL;
		}
	}
    }
    	
    /* save the commands and patterns */
    for(i=0; i<=MAX_NUM_MACROS-1; i++) {
        GConfValue *entry1, *entry2;
	    
	entry1 = gconf_value_new (GCONF_VALUE_STRING);
	gconf_value_set_string (entry1, prop->macro_pattern[i]);
	entry2 = gconf_value_new (GCONF_VALUE_STRING);
	gconf_value_set_string (entry2, prop->macro_command[i]);
	pattern_list = g_slist_append (pattern_list, entry1);
	command_list = g_slist_append (command_list, entry2);	   
    
    }  
    
    macro_patterns = gconf_value_new (GCONF_VALUE_LIST);
    macro_commands = gconf_value_new (GCONF_VALUE_LIST);
    if (pattern_list && command_list) {
        gconf_value_set_list_type (macro_patterns, GCONF_VALUE_STRING);
        gconf_value_set_list (macro_patterns, pattern_list);
        panel_applet_gconf_set_value (applet, "macro_patterns", macro_patterns, NULL);
        gconf_value_set_list_type (macro_commands, GCONF_VALUE_STRING);
        gconf_value_set_list (macro_commands, command_list);
        panel_applet_gconf_set_value (applet, "macro_commands", macro_commands, NULL);        
    }

    /* history */
    history = panel_applet_gconf_get_value (applet, "history", NULL);
    if (history) {
        GSList *list = NULL;
        list = gconf_value_get_list (history);
        
        while (list) {
            GConfValue *value = list->data;
            const gchar *entry = NULL;
            
            entry = gconf_value_get_string (value);
            if (entry) {
                append_history_entry(mcdata, (char *)entry, TRUE);
            }
            list = g_slist_next (list);
        }
    }
    
    return prop;
}

static void
phelp_cb (void)
{
    GError *error = NULL;
    gnome_help_display ("command-line", "command-line-prefs-0", &error);

    if (error) {
        g_warning ("help error: %s\n", error->message);
        g_error_free (error);
        error = NULL;
    }
}

static void
check_time_toggled (GtkToggleButton *button, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    properties *prop;
    gboolean toggled;
    
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    
    toggled = gtk_toggle_button_get_active (button);
    
    if (toggled == prop->show_time) 
        return;
        
    prop->show_time = toggled;
    redraw_applet (mcdata);
    
}

static void
check_date_toggled (GtkToggleButton *button, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    properties *prop;
    gboolean toggled;
    
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    
    toggled = gtk_toggle_button_get_active (button);
    
    if (toggled == prop->show_date) 
        return;
        
    prop->show_date = toggled;
    redraw_applet (mcdata);
    
}

static void
size_changed_cb (GtkSpinButton *spin, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    properties *prop = mcdata->prop;
    
    prop->normal_size_x = gtk_spin_button_get_value (spin);
    redraw_applet (mcdata);
    panel_applet_gconf_set_int (applet, "normal_size_x", prop->normal_size_x, NULL);
    
}

static void
check_handle_toggled (GtkToggleButton *button, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    properties *prop;
    gboolean toggled;
    
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    
    toggled = gtk_toggle_button_get_active (button);
    
    if (toggled == prop->show_handle) 
        return;
        
    prop->show_handle = toggled;
    redraw_applet (mcdata);
    panel_applet_gconf_set_bool (applet, "show_handle", prop->show_handle, NULL);
    
}

static void
check_frame_toggled (GtkToggleButton *button, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    properties *prop;
    gboolean toggled;
    
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    
    toggled = gtk_toggle_button_get_active (button);
    
    if (toggled == prop->show_frame) 
        return;
        
    prop->show_frame = toggled;
    redraw_applet (mcdata);
    panel_applet_gconf_set_bool (applet, "show_frame", prop->show_frame, NULL);
    
}

static void
check_theme_toggled (GtkToggleButton *button, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    GtkWidget *fg_color_picker;
    GtkWidget *bg_color_picker;
    properties *prop;
    gboolean toggled;
    
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    
    toggled = gtk_toggle_button_get_active (button);
    
    if (toggled == prop->show_default_theme) 
        return;
        
    prop->show_default_theme = toggled;
    fg_color_picker = g_object_get_data ( G_OBJECT (applet), "fg_color_picker");
    bg_color_picker = g_object_get_data ( G_OBJECT (applet), "bg_color_picker");
    gtk_widget_set_sensitive( GTK_WIDGET (fg_color_picker), !(prop->show_default_theme));
    gtk_widget_set_sensitive( GTK_WIDGET (bg_color_picker), !(prop->show_default_theme));
    redraw_applet (mcdata);
    panel_applet_gconf_set_bool (applet, "show_default_theme",
                                 prop->show_default_theme,
                                 NULL);

}

static void
autocomplete_toggled (GtkToggleButton *button, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    properties *prop;
    gboolean toggled;
    
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    
    toggled = gtk_toggle_button_get_active (button);
    
    if (toggled == prop->auto_complete_history) 
        return;
        
    prop->auto_complete_history = toggled;
    panel_applet_gconf_set_bool (applet, "autocomplete_history", 
    			         prop->auto_complete_history, 
    				 NULL);
}

static void
response_cb (GtkDialog *dialog, gint it, gpointer data)
{
    MCData *mcdata = data;

    if(it == GTK_RESPONSE_HELP){
        phelp_cb ();
        return;
    }
    
    gtk_widget_destroy (GTK_WIDGET (dialog));
    mcdata->properties_box = NULL;

}

void
properties_box (BonoboUIComponent *uic,
		MCData            *data,
		const char        *verbname)
{
   #if 0 /* FIXME */
    static GnomeHelpMenuEntry help_entry = { NULL,  "properties" };
   #endif
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    properties *prop;
    GtkWidget *notebook;
    GtkWidget *vbox, *vbox1, *frame;
    GtkWidget *hbox;
    GtkWidget *table;
    GtkWidget *check_time, *check_date, *check_handle, *check_frame, 
              *check_auto_complete_history, *check_theme;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *spin;
    GtkWidget *color_picker;
    GtkWidget *scrolled_window;
    char text_label[50], buffer[50];
    int i;

    if (mcdata->properties_box) {
#ifdef HAVE_GTK_MULTIHEAD
	gtk_window_set_screen (GTK_WINDOW (mcdata->properties_box),
			       gtk_widget_get_screen (GTK_WIDGET (mcdata->applet)));
#endif
        gtk_window_present (GTK_WINDOW (mcdata->properties_box));
	return;
    }
    prop = g_object_get_data (G_OBJECT (applet), "prop");
    mcdata->properties_box = gtk_dialog_new_with_buttons (_("Command Line Preferences"), 
    						  NULL,
						  GTK_DIALOG_DESTROY_WITH_PARENT,
						  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
						  GTK_STOCK_HELP, GTK_RESPONSE_HELP,
						  NULL);
    gtk_dialog_set_default_response (GTK_DIALOG (mcdata->properties_box), GTK_RESPONSE_CLOSE);
    gtk_window_set_default_size (GTK_WINDOW (mcdata->properties_box), 400, 300);
#ifdef HAVE_GTK_MULTIHEAD
    gtk_window_set_screen (GTK_WINDOW (mcdata->properties_box),
			   gtk_widget_get_screen (GTK_WIDGET (mcdata->applet)));
#endif
    
    notebook = gtk_notebook_new ();
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG (mcdata->properties_box)->vbox), 
    			notebook, TRUE, TRUE, 0);
    
    /* time & date */
    vbox = gtk_vbox_new(FALSE, GNOME_PAD_BIG);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
#if 0
    frame = gtk_frame_new(_("Clock"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(frame), vbox1);

    /* show time check box */
    check_time = gtk_check_button_new_with_label (_("Show time"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_time), prop->show_time);
    g_signal_connect (G_OBJECT (check_time), "toggled",
    		      G_CALLBACK (check_time_toggled), mcdata);
    gtk_box_pack_start(GTK_BOX(vbox1), check_time, FALSE, TRUE, 0);

    /* show date check box */
    check_date = gtk_check_button_new_with_label (_("Show date"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_date), prop->show_date);
    g_signal_connect (G_OBJECT (check_date), "toggled",
    		      G_CALLBACK (check_date_toggled), mcdata);
    gtk_box_pack_start(GTK_BOX(vbox1), check_date, FALSE, TRUE, 0);
#endif

    /* appearance frame */
    frame = gtk_frame_new(_("Appearance"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(frame), vbox1);

    /* show handle check box */
    check_handle = gtk_check_button_new_with_mnemonic (_("Show han_dle"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_handle), prop->show_handle);
    g_signal_connect (G_OBJECT (check_handle), "toggled",
    		      G_CALLBACK (check_handle_toggled), mcdata);
    gtk_box_pack_start(GTK_BOX(vbox1), check_handle, FALSE, TRUE, 0);

    /* show frame check box */
    check_frame = gtk_check_button_new_with_mnemonic (_("Show fram_e"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_frame), prop->show_frame);
    g_signal_connect (G_OBJECT (check_frame), "toggled",
    		      G_CALLBACK (check_frame_toggled), mcdata);
    gtk_box_pack_start(GTK_BOX(vbox1), check_frame, FALSE, TRUE, 0);



    /* auto complete frame */
    frame = gtk_frame_new(_("Autocompletion"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(frame), vbox1);

    /* show history autocomplete */
    check_auto_complete_history = gtk_check_button_new_with_mnemonic (_("Enable _history based autocompletion"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_auto_complete_history), prop->auto_complete_history);
    g_signal_connect (G_OBJECT (check_auto_complete_history), "toggled",
    		      G_CALLBACK (autocomplete_toggled), mcdata);
    gtk_box_pack_start(GTK_BOX(vbox1), check_auto_complete_history, FALSE, TRUE, 0);

    /* Size */
    frame = gtk_frame_new(_("Size"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(5, 4, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), GNOME_PAD_SMALL); 
    /*    gtk_table_set_row_spacing(GTK_TABLE(table), 1, GNOME_PAD_SMALL); 
	  gtk_table_set_row_spacing(GTK_TABLE(table), 2, 20);  */ /* no effect? */
    gtk_table_set_col_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
    gtk_container_set_border_width(GTK_CONTAINER(table), GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(frame), table);

    /* applet width */    
    label = gtk_label_new_with_mnemonic(_("Applet _width:"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), 
		     label,
		     0, 1,
		     0, 1,
		     GTK_FILL, 0,
		     0, 0);
    
    spin = gtk_spin_button_new_with_range (50, 400, 10);
    gtk_table_attach(GTK_TABLE(table), 
		     spin,
		     1, 2,
		     0, 1,
		     GTK_FILL, 0,
		     0, 0);
    set_atk_relation (label, spin),
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), prop->normal_size_x);
    g_signal_connect (G_OBJECT (spin), "value_changed",
    		      G_CALLBACK (size_changed_cb), mcdata); 
/*
    entry = gtk_entry_new_with_max_length(4);
    g_snprintf(buffer, sizeof(buffer), "%d", prop->normal_size_x);
    gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);
    gtk_widget_set_usize(entry, 50, -1);
    gtk_table_attach(GTK_TABLE(table), 
		     entry,
		     1, 2,
		     0, 1,
		     GTK_FILL, 0,
		     0, 0);
*/
#if 0
    /* applet height */    
    label = gtk_label_new(_("Applet height:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), 
		     label,
		     0, 1,
		     1, 2,
		     GTK_FILL, 0,
		     0, 0);

    entry = gtk_entry_new_with_max_length(4);
    set_atk_relation (label, entry);
    g_snprintf(buffer, sizeof(buffer), "%d", prop->normal_size_y);
    gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);
    gtk_widget_set_usize(entry, 50, -1);
    gtk_table_attach(GTK_TABLE(table), 
		     entry,
		     1, 2,
		     1, 2,
		     GTK_FILL, 0,
		     0, 0);

    /* cmd line height */    
    label = gtk_label_new(_("Command line height:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), 
		     label,
		     0, 1,
		     2, 3,
		     GTK_FILL, 0,
		     0, 0);

    entry = gtk_entry_new_with_max_length(4);
    set_atk_relation (label, entry);
    g_snprintf(buffer, sizeof(buffer), "%d", prop->cmd_line_size_y);
    gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);
    gtk_widget_set_usize(entry, 50, -1);
    gtk_table_attach(GTK_TABLE(table), 
		     entry,
		     1, 2,
		     2, 3,
		     GTK_FILL, 0,
		     0, 0);
    
    /* hint */
    /*
    label = gtk_label_new((gchar *) _("\n_sometimes the applet has to be moved on the panel\nto make a change of the size visible."));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(table), 
		     label,
		     0, 5,
		     3, 4,
		     GTK_FILL, 0,
		     0, 0);
    */
#endif
    /* Color */
    frame = gtk_frame_new(_("Colors"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(2, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
    gtk_table_set_col_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
    gtk_container_set_border_width(GTK_CONTAINER(table), GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(frame), table);

    /* default bg and fg theme */
    check_theme = gtk_check_button_new_with_mnemonic(_("_Use default theme colors"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_theme), prop->show_default_theme);
    g_signal_connect (G_OBJECT (check_theme), "toggled",
    		      G_CALLBACK (check_theme_toggled), mcdata); 
    gtk_table_attach(GTK_TABLE(table),
             check_theme,
             0, 1,
             0, 1,
             0, 0,
             0, 0);

    /* fg */
    label = gtk_label_new_with_mnemonic(_("Command line _foreground:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5); 
    gtk_table_attach(GTK_TABLE(table), 
		     label,
		     0, 1,
		     1, 2,
		     GTK_FILL, 0,
		     0, 0);

    color_picker = gnome_color_picker_new();
    g_object_set_data ( G_OBJECT (applet), "fg_color_picker", color_picker);
    gnome_color_picker_set_i16(GNOME_COLOR_PICKER(color_picker),
			       prop->cmd_line_color_fg_r, 
			       prop->cmd_line_color_fg_g, 
			       prop->cmd_line_color_fg_b, 
			       0);
    gtk_widget_set_sensitive ( GTK_WIDGET (color_picker), !(prop->show_default_theme));
    set_atk_relation(label, color_picker);
    gtk_signal_connect(GTK_OBJECT(color_picker),
		       "color_set",
		       GTK_SIGNAL_FUNC(color_cmd_fg_changed_signal),
		       mcdata);
    /*
      gtk_box_pack_start(GTK_BOX(hbox), color_picker, FALSE, TRUE, 0);
    */
    gtk_table_attach(GTK_TABLE(table), 
		     color_picker,
		     1, 2,
		     1, 2,
		     0, 0,
		     0, 0);
    

    /* bg */
    label = gtk_label_new_with_mnemonic(_("Command line _background:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), 
		     label,
		     0, 1,
		     2, 3,
		     GTK_FILL, 0,
		     0, 0);
    color_picker = gnome_color_picker_new();
    g_object_set_data ( G_OBJECT (applet), "bg_color_picker", color_picker);
    gnome_color_picker_set_i16(GNOME_COLOR_PICKER(color_picker),
			       prop->cmd_line_color_bg_r, 
			       prop->cmd_line_color_bg_g, 
			       prop->cmd_line_color_bg_b, 
			       0);
    gtk_widget_set_sensitive ( GTK_WIDGET (color_picker), !(prop->show_default_theme));
    set_atk_relation(label, color_picker);
    gtk_signal_connect(GTK_OBJECT(color_picker),
		       "color_set",
		       GTK_SIGNAL_FUNC(color_cmd_bg_changed_signal),
		       mcdata);
    gtk_table_attach(GTK_TABLE(table), 
		     color_picker,
		     1, 2,
		     2, 3,
		     0, 0,
		     0, 0);
    


    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox,
    			      gtk_label_new_with_mnemonic (_("_General")));

    /* Macros */   
    frame = gtk_frame_new((gchar *) NULL);

    gtk_container_set_border_width(GTK_CONTAINER(frame), GNOME_PAD_SMALL); 
    
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				   GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_AUTOMATIC);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), GNOME_PAD_SMALL); 
    gtk_container_add(GTK_CONTAINER(frame), scrolled_window); 

    vbox1 = gtk_vbox_new(TRUE, GNOME_PAD_SMALL);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
    /* gtk_container_add(GTK_CONTAINER(scrolled_window), vbox1); does not work */
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), vbox1);
    gtk_container_set_focus_hadjustment (GTK_CONTAINER (vbox1),
        gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (scrolled_window)));
    gtk_container_set_focus_vadjustment (GTK_CONTAINER (vbox1),
        gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (scrolled_window)));

    for(i=0; i < MAX_NUM_MACROS; i++)
	{
	    hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
  	    gtk_box_pack_start(GTK_BOX(vbox1), hbox, TRUE, TRUE, 0);   
	    
	    /* prefix */    
	    g_snprintf(text_label, sizeof(text_label), _("Regex _%.2d:"), i+1);
	    label = gtk_label_new_with_mnemonic(text_label);
	    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	    
	    entry = gtk_entry_new_with_max_length(MAX_MACRO_PATTERN_LENGTH);
	    gtk_widget_set_usize(entry, 75, -1);
	    if (prop->macro_pattern[i] != (gchar *) NULL)
		gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) prop->macro_pattern[i]);
	    set_atk_relation(label, entry);
	    g_object_set_data (G_OBJECT (entry), "prop", prop);
	    g_object_set_data (G_OBJECT (entry), "applet", applet);
	    #if 1
	    gtk_signal_connect(GTK_OBJECT(entry),
			       "focus_out_event",
			       GTK_SIGNAL_FUNC(pattern_entry_changed_signal),
			       GINT_TO_POINTER (i));
	    #endif
	    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);
	    
	    /* command */
	    g_snprintf(text_label, sizeof(text_label), _("   Macro _%.2d:"), i+1);
	    label = gtk_label_new_with_mnemonic(text_label);
	    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	    
	    entry = gtk_entry_new_with_max_length(MAX_COMMAND_LENGTH);
	    if (prop->macro_command[i] != (gchar *) NULL)
		gtk_entry_set_text(GTK_ENTRY(entry), prop->macro_command[i]);
	    set_atk_relation(label, entry);
	    g_object_set_data (G_OBJECT (entry), "prop", prop);
	    g_object_set_data (G_OBJECT (entry), "applet", applet);
	    #if 1
	    gtk_signal_connect(GTK_OBJECT(entry),
			       "focus_out_event",
			       GTK_SIGNAL_FUNC(command_entry_changed_signal),
			       GINT_TO_POINTER (i));
	    #endif
	    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
	}

    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame,
    			      gtk_label_new_with_mnemonic (_("_Macros")));
    
    gtk_widget_show_all(mcdata->properties_box);
    
    g_signal_connect (G_OBJECT (mcdata->properties_box), "response",
    		      G_CALLBACK (response_cb), mcdata);
    return;
}

