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
#include <applet-widget.h>
#include <string.h>

#include "preferences.h"
#include "command_line.h" /* needed for style changes */
#include "history.h"
#include "message.h"
#include "mini-commander_applet.h"

static void reset_temporary_prefs(void);

properties prop;
properties prop_tmp;

static void
checkbox_toggled_signal(GtkWidget *check_box_widget, int *data)
{
    *data = GTK_TOGGLE_BUTTON(check_box_widget)->active;
}

static void
entry_changed_signal(GtkWidget *entry_widget, char **data)
{
    free(*data);
    *data = (char *) malloc(sizeof(char) * (strlen(gtk_entry_get_text(GTK_ENTRY(entry_widget))) + 1));
    strcpy(*data, gtk_entry_get_text(GTK_ENTRY(entry_widget)));
}

static void
entry_integer_changed_signal(GtkWidget *entry_widget, int *data)
{
    *data = atoi(gtk_entry_get_text(GTK_ENTRY(entry_widget)));
}

static void
color_cmd_fg_changed_signal(GtkWidget *color_picker_widget, gpointer *data)
{
    gushort r, g, b;

    gnome_color_picker_get_i16(GNOME_COLOR_PICKER(color_picker_widget), &r, &g, &b, NULL);

    prop_tmp.cmd_line_color_fg_r = (int) r;
    prop_tmp.cmd_line_color_fg_g = (int) g; 
    prop_tmp.cmd_line_color_fg_b = (int) b;
    return;
    data = NULL;
}

static void
color_cmd_bg_changed_signal(GtkWidget *color_picker_widget, gpointer *data)
{
    gushort r, g, b;

    gnome_color_picker_get_i16(GNOME_COLOR_PICKER(color_picker_widget), &r, &g, &b, NULL);

    prop_tmp.cmd_line_color_bg_r = (int) r;
    prop_tmp.cmd_line_color_bg_g = (int) g; 
    prop_tmp.cmd_line_color_bg_b = (int) b;
    return;
    data = NULL;
}

static void
reset_temporary_prefs(void)
{
    /* reset temporary prefereces */
    prop_tmp.show_time = -1;
    prop_tmp.show_date = -1;
    prop_tmp.show_handle = -1;
    prop_tmp.show_frame = -1;
    prop_tmp.auto_complete_history = -1;
    prop_tmp.normal_size_x = -1;
    prop_tmp.normal_size_y = -1;
    prop_tmp.reduced_size_x = -1;
    prop_tmp.reduced_size_y = -1;
    prop_tmp.auto_complete_history = -1;
    prop_tmp.cmd_line_size_y = -1;
    prop_tmp.flat_layout = -1;
    prop_tmp.cmd_line_color_fg_r = -1;
    prop_tmp.cmd_line_color_fg_g = -1;
    prop_tmp.cmd_line_color_fg_b = -1;
    prop_tmp.cmd_line_color_bg_r = -1;
    prop_tmp.cmd_line_color_bg_g = -1;
    prop_tmp.cmd_line_color_bg_b = -1;
}


static void
properties_box_apply_signal(GnomePropertyBox *property_box_widget, gint page, gpointer data)
{
    int i;

    if(prop_tmp.show_time != -1)
	/* checkbox has been changed */
	prop.show_time = prop_tmp.show_time;

    if(prop_tmp.show_date != -1)
	/* checkbox has been changed */
	prop.show_date = prop_tmp.show_date;

    if(prop_tmp.show_handle != -1)
	/* checkbox has been changed */
        prop.show_handle = prop_tmp.show_handle;

    if(prop_tmp.show_frame != -1)
	/* checkbox has been changed */
        prop.show_frame = prop_tmp.show_frame;

    if(prop_tmp.auto_complete_history != -1)
	/* checkbox has been changed */
        prop.auto_complete_history = prop_tmp.auto_complete_history;

    if(prop_tmp.show_time != -1 || prop_tmp.show_date != -1) {
	/* checkbox has been changed */
	if(prop.show_time && prop.show_date)
	    show_message((gchar *) _("time & date on")); 
	else if(prop.show_time && !prop.show_date)
	    show_message((gchar *) _("time on")); 
	else if(!prop.show_time && prop.show_date)
	    show_message((gchar *) _("date on")); 
	else
	    show_message((gchar *) _("clock off")); 
    }

    if(prop_tmp.normal_size_x != -1)
	prop.normal_size_x = prop_tmp.normal_size_x;
    if(prop_tmp.normal_size_y != -1)
	prop.normal_size_y = prop_tmp.normal_size_y;
    if(prop_tmp.reduced_size_x != -1)
	prop.reduced_size_x = prop_tmp.reduced_size_x;
    if(prop_tmp.reduced_size_y != -1)
	prop.reduced_size_y = prop_tmp.reduced_size_y;
    if(prop_tmp.cmd_line_size_y != -1)
	prop.cmd_line_size_y = prop_tmp.cmd_line_size_y;

    if(prop_tmp.normal_size_x != -1 || prop_tmp.normal_size_y != -1 || 
       prop_tmp.reduced_size_x != -1 || prop_tmp.reduced_size_y != -1)
	{
	    /* size was changed * /
	    gtk_container_set_resize_mode(GTK_CONTAINER(applet), GTK_RESIZE_IMMEDIATE);
	    gtk_widget_set_usize(GTK_WIDGET(applet), prop.normal_size_x, prop.normal_size_y);
	    gtk_widget_show (applet);
	    gtk_widget_queue_resize(applet);
	    gtk_widget_queue_draw(applet);
	    gtk_widget_set_uposition(applet, 1, 2);
	    gtk_container_check_resize(GTK_CONTAINER(applet));
	    gtk_container_resize_children(GTK_CONTAINER(applet));
	    */
            redraw_applet();
	}

    /* colors */
    if(prop_tmp.cmd_line_color_fg_r != -1)
	{
	    prop.cmd_line_color_fg_r = prop_tmp.cmd_line_color_fg_r;
	    prop.cmd_line_color_fg_g = prop_tmp.cmd_line_color_fg_g;
	    prop.cmd_line_color_fg_b = prop_tmp.cmd_line_color_fg_b;
	}
    if(prop_tmp.cmd_line_color_bg_r != -1)
	{
	    prop.cmd_line_color_bg_r = prop_tmp.cmd_line_color_bg_r;
	    prop.cmd_line_color_bg_g = prop_tmp.cmd_line_color_bg_g;
	    prop.cmd_line_color_bg_b = prop_tmp.cmd_line_color_bg_b;
	}

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

    if(prop_tmp.show_handle != -1
       || prop_tmp.show_frame != -1
       || prop_tmp.cmd_line_size_y != -1
       || prop_tmp.cmd_line_color_fg_r != -1 
       || prop_tmp.cmd_line_color_bg_r != -1)
	redraw_applet();

    reset_temporary_prefs();

    /* Why is this not done automatically??? */
    save_session();

    return;
    property_box_widget = NULL;
    page = 0;
    data = NULL;
}

void
load_session(void)
{
    int i;
    char section[MAX_COMMAND_LENGTH + 100];
    char default_macro_pattern[MAX_MACRO_PATTERN_LENGTH];
    char default_macro_command[MAX_COMMAND_LENGTH];
    
    /* read properties */
    /* gnome_config_push_prefix(APPLET_WIDGET(applet)->globcfgpath); */
    gnome_config_push_prefix("/mini-commander/");

    prop.show_time = gnome_config_get_bool("mini_commander/show_time=true");
    prop.show_date = gnome_config_get_bool("mini_commander/show_date=false");
    prop.show_handle = gnome_config_get_bool("mini_commander/show_handle=false");
    prop.show_frame = gnome_config_get_bool("mini_commander/show_frame=false");

    prop.auto_complete_history = gnome_config_get_bool("mini_commander/autocomplete_history=true");

    /* size */
    prop.normal_size_x = gnome_config_get_int("mini_commander/normal_size_x=148");
    prop.normal_size_y = gnome_config_get_int("mini_commander/normal_size_y=48");
    prop.reduced_size_x = gnome_config_get_int("mini_commander/reduced_size_x=50");
    prop.reduced_size_y = gnome_config_get_int("mini_commander/reduced_size_y=48");
    prop.cmd_line_size_y = gnome_config_get_int("mini_commander/cmd_line_y=24");

    /* colors */
    prop.cmd_line_color_fg_r = gnome_config_get_int("mini_commander/cmd_line_color_fg_r=64613");
    prop.cmd_line_color_fg_g = gnome_config_get_int("mini_commander/cmd_line_color_fg_g=64613");
    prop.cmd_line_color_fg_b = gnome_config_get_int("mini_commander/cmd_line_color_fg_b=64613");

    prop.cmd_line_color_bg_r = gnome_config_get_int("mini_commander/cmd_line_color_bg_r=0");
    prop.cmd_line_color_bg_g = gnome_config_get_int("mini_commander/cmd_line_color_bg_g=0");
    prop.cmd_line_color_bg_b = gnome_config_get_int("mini_commander/cmd_line_color_bg_b=0");

    /* macros */
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

	    g_snprintf(section, sizeof(section), "mini_commander/macro_pattern_%.2u=%s", i+1, default_macro_pattern);
	    free(prop.macro_pattern[i]);
	    prop.macro_pattern[i] = (char *)gnome_config_get_string((gchar *) section);

	    g_snprintf(section, sizeof(section), "mini_commander/macro_command_%.2u=%s", i+1, default_macro_command);
	    free(prop.macro_command[i]);
	    prop.macro_command[i] = (char *)gnome_config_get_string((gchar *) section);

	    if(prop.macro_pattern[i][0] != '\0')
		{
		    prop.macro_regex[i] = malloc(sizeof(regex_t));
		    /* currently we don't care about syntax errors in regex patterns */
		    regcomp(prop.macro_regex[i], prop.macro_pattern[i], REG_EXTENDED);
		}
	    else
		{
		    prop.macro_regex[i] = NULL;
		}

	    prop_tmp.macro_pattern[i] = (char *)NULL;
	    prop_tmp.macro_command[i] = (char *)NULL;

	}    

    /* history */
    for(i = 0; i < LENGTH_HISTORY_LIST; i++)
	{
	    gchar *temp_str;
	    g_snprintf(section, sizeof(section), "mini_commander/history_%.2u=%s", i, "");
	    temp_str = gnome_config_get_string((gchar *) section);
	    if(strcmp("", temp_str) != 0)
	       append_history_entry(temp_str);
	    g_free(temp_str);
	}

    gnome_config_pop_prefix();
}


void
save_session(void)
{
    int i;
    char section[100];

    /* gnome_config_push_prefix(globcfgpath); */
    /* all instances of mini-commander use the same global settings */
    gnome_config_push_prefix("/mini-commander/"); 

    /* version */
    gnome_config_set_string("mini_commander/version", (gchar *) VERSION);

    /* clock */
    gnome_config_set_bool("mini_commander/show_time", prop.show_time);
    gnome_config_set_bool("mini_commander/show_date", prop.show_date);
    gnome_config_set_bool("mini_commander/show_handle", prop.show_handle);
    gnome_config_set_bool("mini_commander/show_frame", prop.show_frame);

    gnome_config_set_bool("mini_commander/autocomplete_history", prop.auto_complete_history);

    /* size */
    gnome_config_set_int("mini_commander/normal_size_x", prop.normal_size_x);
    gnome_config_set_int("mini_commander/normal_size_y", prop.normal_size_y);
    gnome_config_set_int("mini_commander/reduced_size_x", prop.reduced_size_x);
    gnome_config_set_int("mini_commander/reduced_size_y", prop.reduced_size_y);
    gnome_config_set_int("mini_commander/cmd_line_y", prop.cmd_line_size_y);

    /* colors */
    gnome_config_set_int("mini_commander/cmd_line_color_fg_r", prop.cmd_line_color_fg_r);
    gnome_config_set_int("mini_commander/cmd_line_color_fg_g", prop.cmd_line_color_fg_g);
    gnome_config_set_int("mini_commander/cmd_line_color_fg_b", prop.cmd_line_color_fg_b);

    gnome_config_set_int("mini_commander/cmd_line_color_bg_r", prop.cmd_line_color_bg_r);
    gnome_config_set_int("mini_commander/cmd_line_color_bg_g", prop.cmd_line_color_bg_g);
    gnome_config_set_int("mini_commander/cmd_line_color_bg_b", prop.cmd_line_color_bg_b);

    /* macros */
    for(i=0; i<=MAX_NUM_MACROS-1; i++)
	{
	    g_snprintf(section, sizeof(section), "mini_commander/macro_pattern_%.2u", i+1);
	    gnome_config_set_string((gchar *) section, (gchar *) prop.macro_pattern[i]);

	    g_snprintf(section, sizeof(section), "mini_commander/macro_command_%.2u", i+1);
	    gnome_config_set_string((gchar *) section, (gchar *) prop.macro_command[i]);

	    prop_tmp.macro_pattern[i] = (char *) NULL;
	    prop_tmp.macro_command[i] = (char *) NULL;
	}    

    /* history */
    for(i = 0; i < LENGTH_HISTORY_LIST; i++)
	{
	    g_snprintf(section, sizeof(section), "mini_commander/history_%.2u", i);
	    if(exists_history_entry(i))
		gnome_config_set_string((gchar *) section, (gchar *) get_history_entry(i));
	    else
		gnome_config_set_string((gchar *) section, (gchar *) "");
	}
  
    gnome_config_sync();
    /* you need to use the drop_all here since we're all writing to
       one file, without it, things might not work too well */
    gnome_config_drop_all();
    
    gnome_config_pop_prefix(); 
}

gint
save_session_signal(GtkWidget *widget, const char *privcfgpath, const char *globcfgpath)
{       
#ifdef HAVE_PANEL_PIXEL_SIZE
    if(!prop.flat_layout)
	show_message((gchar *) _("saving prefs...")); 	    
#else
    show_message((gchar *) _("saving prefs...")); 	    
#endif

    save_session();

    /* make sure you return FALSE, otherwise your applet might not
       work compeltely, there are very few circumstances where you
       want to return TRUE. This behaves similiar to GTK events, in
       that if you return FALSE it means that you haven't done
       everything yourself, meaning you want the panel to save your
       other state such as the panel you are on, position,
       parameter, etc ... */
    return FALSE;
    widget = NULL;
    privcfgpath = NULL;
    globcfgpath = NULL;
}

static void
phelp_cb (GtkWidget *w, gint tab, gpointer data)
{
        GnomeHelpMenuEntry help_entry = { "mini-commander_applet",
                                          "index.html#MINI-COMMANDER-PREFS" };
        gnome_help_display(NULL, &help_entry);
}


void
properties_box(AppletWidget *applet, gpointer data)
{
    static GnomeHelpMenuEntry help_entry = { NULL,  "properties" };
    static GtkWidget *properties_box = NULL;
    GtkWidget *vbox, *vbox1, *frame;
    GtkWidget *hbox;
    GtkWidget *table;
    GtkWidget *check_time, *check_date, *check_handle, *check_frame, *check_auto_complete_history;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *color_picker;
    GtkWidget *scrolled_window;
    char text_label[50], buffer[50];
    int i;


    if (properties_box != NULL)
    {
        gdk_window_show(GTK_WIDGET(properties_box)->window);
	gdk_window_raise(GTK_WIDGET(properties_box)->window);
	return;
    }
    help_entry.name = gnome_app_id;

    reset_temporary_prefs();

    properties_box = gnome_property_box_new();
    
    gtk_window_set_title(GTK_WINDOW(properties_box), _("Mini-Commander Properties"));
    gtk_signal_connect(GTK_OBJECT(properties_box),
		       "destroy",
		       gtk_widget_destroyed,
		       &properties_box);
    
    /* time & date */
    vbox = gtk_vbox_new(FALSE, GNOME_PAD_BIG);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);

    frame = gtk_frame_new(_("Clock"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(frame), vbox1);

    /* show time check box */
    check_time = gtk_check_button_new_with_label (_("Show time"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_time), prop.show_time);
    gtk_signal_connect(GTK_OBJECT(check_time),
		       "toggled",
		       GTK_SIGNAL_FUNC(checkbox_toggled_signal),
		       &prop_tmp.show_time);
    gtk_signal_connect_object(GTK_OBJECT(check_time), "toggled",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(properties_box));  
    gtk_box_pack_start(GTK_BOX(vbox1), check_time, FALSE, TRUE, 0);

    /* show date check box */
    check_date = gtk_check_button_new_with_label (_("Show date"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_date), prop.show_date);
    gtk_signal_connect(GTK_OBJECT(check_date),
		       "toggled",
		       GTK_SIGNAL_FUNC(checkbox_toggled_signal),
		       &prop_tmp.show_date);
    gtk_signal_connect_object(GTK_OBJECT(check_date), "toggled",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(properties_box));
    gtk_box_pack_start(GTK_BOX(vbox1), check_date, FALSE, TRUE, 0);


    /* appearance frame */
    frame = gtk_frame_new(_("Appearance"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(frame), vbox1);

    /* show handle check box */
    check_handle = gtk_check_button_new_with_label (_("Show handle"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_handle), prop.show_handle);
    gtk_signal_connect(GTK_OBJECT(check_handle),
                       "toggled",
                       GTK_SIGNAL_FUNC(checkbox_toggled_signal),
                       &prop_tmp.show_handle);
    gtk_signal_connect_object(GTK_OBJECT(check_handle), "toggled",
                              GTK_SIGNAL_FUNC(gnome_property_box_changed),
                              GTK_OBJECT(properties_box));
    gtk_box_pack_start(GTK_BOX(vbox1), check_handle, FALSE, TRUE, 0);

    /* show frame check box */
    check_frame = gtk_check_button_new_with_label (_("Show frame"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_frame), prop.show_frame);
    gtk_signal_connect(GTK_OBJECT(check_frame),
                       "toggled",
                       GTK_SIGNAL_FUNC(checkbox_toggled_signal),
                       &prop_tmp.show_frame);
    gtk_signal_connect_object(GTK_OBJECT(check_frame), "toggled",
                              GTK_SIGNAL_FUNC(gnome_property_box_changed),
                              GTK_OBJECT(properties_box));
    gtk_box_pack_start(GTK_BOX(vbox1), check_frame, FALSE, TRUE, 0);



    /* auto complete frame */
    frame = gtk_frame_new(_("Autocompletion"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_set_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(frame), vbox1);

    /* show history autocomplete */
    check_auto_complete_history = gtk_check_button_new_with_label (_("Enable history based autocompletion"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_auto_complete_history), prop.auto_complete_history);
    gtk_signal_connect(GTK_OBJECT(check_auto_complete_history),
                       "toggled",
                       GTK_SIGNAL_FUNC(checkbox_toggled_signal),
                       &prop_tmp.auto_complete_history);
    gtk_signal_connect_object(GTK_OBJECT(check_auto_complete_history), "toggled",
                              GTK_SIGNAL_FUNC(gnome_property_box_changed),
                              GTK_OBJECT(properties_box));
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
    label = gtk_label_new(_("Applet width:"));
    gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), 
		     label,
		     0, 1,
		     0, 1,
		     GTK_FILL, 0,
		     0, 0);
    
    entry = gtk_entry_new_with_max_length(4);
    g_snprintf(buffer, sizeof(buffer), "%d", prop.normal_size_x);
    gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);
    gtk_widget_set_usize(entry, 50, -1);
    gtk_signal_connect(GTK_OBJECT(entry),
		       "changed",
		       GTK_SIGNAL_FUNC(entry_integer_changed_signal),
		       &prop_tmp.normal_size_x);
    gtk_signal_connect_object(GTK_OBJECT(entry),
			      "changed",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(properties_box));      
    gtk_table_attach(GTK_TABLE(table), 
		     entry,
		     1, 2,
		     0, 1,
		     GTK_FILL, 0,
		     0, 0);

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
    g_snprintf(buffer, sizeof(buffer), "%d", prop.normal_size_y);
    gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);
    gtk_widget_set_usize(entry, 50, -1);
    gtk_signal_connect(GTK_OBJECT(entry),
		       "changed",
		       GTK_SIGNAL_FUNC(entry_integer_changed_signal),
		       &prop_tmp.normal_size_y);
    gtk_signal_connect_object(GTK_OBJECT(entry),
			      "changed",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(properties_box));      
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
    g_snprintf(buffer, sizeof(buffer), "%d", prop.cmd_line_size_y);
    gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);
    gtk_widget_set_usize(entry, 50, -1);
    gtk_signal_connect(GTK_OBJECT(entry),
		       "changed",
		       GTK_SIGNAL_FUNC(entry_integer_changed_signal),
		       &prop_tmp.cmd_line_size_y);
    gtk_signal_connect_object(GTK_OBJECT(entry),
			      "changed",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(properties_box));      
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

    /* Color */
    frame = gtk_frame_new(_("Colors"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(2, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
    gtk_table_set_col_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
    gtk_container_set_border_width(GTK_CONTAINER(table), GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(frame), table);

    /* fg */
    label = gtk_label_new(_("Command line foreground:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), 
		     label,
		     0, 1,
		     0, 1,
		     GTK_FILL, 0,
		     0, 0);

    color_picker = gnome_color_picker_new();
    gnome_color_picker_set_i16(GNOME_COLOR_PICKER(color_picker),
			       prop.cmd_line_color_fg_r, 
			       prop.cmd_line_color_fg_g, 
			       prop.cmd_line_color_fg_b, 
			       0);
    gtk_signal_connect(GTK_OBJECT(color_picker),
		       "color_set",
		       GTK_SIGNAL_FUNC(color_cmd_fg_changed_signal),
		       (gpointer *) NULL);
    gtk_signal_connect_object(GTK_OBJECT(color_picker),
			      "color_set",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(properties_box));      
    /*
      gtk_box_pack_start(GTK_BOX(hbox), color_picker, FALSE, TRUE, 0);
    */
    gtk_table_attach(GTK_TABLE(table), 
		     color_picker,
		     1, 2,
		     0, 1,
		     0, 0,
		     0, 0);
    

    /* bg */
    label = gtk_label_new(_("Command line background:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach(GTK_TABLE(table), 
		     label,
		     0, 1,
		     1, 2,
		     GTK_FILL, 0,
		     0, 0);
    color_picker = gnome_color_picker_new();
    gnome_color_picker_set_i16(GNOME_COLOR_PICKER(color_picker),
			       prop.cmd_line_color_bg_r, 
			       prop.cmd_line_color_bg_g, 
			       prop.cmd_line_color_bg_b, 
			       0);
    gtk_signal_connect(GTK_OBJECT(color_picker),
		       "color_set",
		       GTK_SIGNAL_FUNC(color_cmd_bg_changed_signal),
		       (gpointer *) NULL);
    gtk_signal_connect_object(GTK_OBJECT(color_picker),
			      "color_set",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(properties_box));      
    gtk_table_attach(GTK_TABLE(table), 
		     color_picker,
		     1, 2,
		     1, 2,
		     0, 0,
		     0, 0);
    


    gnome_property_box_append_page(GNOME_PROPERTY_BOX(properties_box),
				   vbox,
				   gtk_label_new(_("General")));



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

    for(i=0; i < MAX_NUM_MACROS; i++)
	{
	    hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
  	    gtk_box_pack_start(GTK_BOX(vbox1), hbox, TRUE, TRUE, 0);   
	    
	    /* prefix */    
	    g_snprintf(text_label, sizeof(text_label), _("Regex %.2d:"), i+1);
	    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(text_label), FALSE, TRUE, 0);
	    
	    entry = gtk_entry_new_with_max_length(MAX_MACRO_PATTERN_LENGTH);
	    gtk_widget_set_usize(entry, 75, -1);
	    if (prop.macro_pattern[i] != (gchar *) NULL)
		gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) prop.macro_pattern[i]);
	    gtk_signal_connect(GTK_OBJECT(entry),
			       "changed",
			       GTK_SIGNAL_FUNC(entry_changed_signal),
			       &prop_tmp.macro_pattern[i]);
	    gtk_signal_connect_object(GTK_OBJECT(entry),
				      "changed",
				      GTK_SIGNAL_FUNC(gnome_property_box_changed),
				      GTK_OBJECT(properties_box));      
	    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);
	    
	    /* command */
	    g_snprintf(text_label, sizeof(text_label), _("   Macro %.2d:"), i+1);
	    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(text_label), FALSE, TRUE, 0);
	    
	    entry = gtk_entry_new_with_max_length(MAX_COMMAND_LENGTH);
	    if (prop.macro_command[i] != (gchar *) NULL)
		gtk_entry_set_text(GTK_ENTRY(entry), prop.macro_command[i]);
	    gtk_signal_connect(GTK_OBJECT(entry),
			       "changed",
			       GTK_SIGNAL_FUNC(entry_changed_signal),
			       &prop_tmp.macro_command[i]);
	    gtk_signal_connect_object(GTK_OBJECT(entry),
				      "changed",
				      GTK_SIGNAL_FUNC(gnome_property_box_changed),
				      GTK_OBJECT(properties_box));  
	    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
	}

    gnome_property_box_append_page(GNOME_PROPERTY_BOX(properties_box),
				   frame,
				   gtk_label_new((gchar *) _("Macros")));

    gtk_signal_connect(GTK_OBJECT(properties_box), 
		       "apply",
		       GTK_SIGNAL_FUNC(properties_box_apply_signal),
		       NULL);

    gtk_signal_connect(GTK_OBJECT(properties_box),
		       "help",
		       GTK_SIGNAL_FUNC(phelp_cb),
		       &help_entry);
    
    gtk_widget_show_all(properties_box);
    return;
    applet = NULL;
    data = NULL;
}

