/*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <om@linuxhq.com>
 *
 * Author: Oliver Maruhn <om@linuxhq.com>
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
#include "command_line.h"
#include "message.h"

properties prop = { TRUE, TRUE, 11, 12, 13, 14, 15, 0, 0, 0, 50000, 50000, 50000 };
properties propTmp = { -1, -1, -1, -1, -1, -1, -1 -1, -1, -1, -1, -1, -1};

static void checkBoxToggled_signal(GtkWidget *checkBoxWidget, int *data)
{
    *data = GTK_TOGGLE_BUTTON(checkBoxWidget)->active;
}

static void entryChanged_signal(GtkWidget *entryWidget, char **data)
{
    free(*data);
    *data = (char *) malloc(sizeof(char) * (strlen(gtk_entry_get_text(GTK_ENTRY(entryWidget))) + 1));
    strcpy(*data, gtk_entry_get_text(GTK_ENTRY(entryWidget)));
}

static void entryIntChanged_signal(GtkWidget *entryWidget, int *data)
{
    *data = atoi(gtk_entry_get_text(GTK_ENTRY(entryWidget)));
}

static void colorCmdFgChanged_signal(GtkWidget *colorPickerWidget, gpointer *data)
{
    gushort r, g, b;

    gnome_color_picker_get_i16(GNOME_COLOR_PICKER(colorPickerWidget), &r, &g, &b, NULL);

    propTmp.cmdLineColorFgR = (int) r;
    propTmp.cmdLineColorFgG = (int) g; 
    propTmp.cmdLineColorFgB = (int) b;
}

static void colorCmdBgChanged_signal(GtkWidget *colorPickerWidget, gpointer *data)
{
    gushort r, g, b;

    gnome_color_picker_get_i16(GNOME_COLOR_PICKER(colorPickerWidget), &r, &g, &b, NULL);

    propTmp.cmdLineColorBgR = (int) r;
    propTmp.cmdLineColorBgG = (int) g; 
    propTmp.cmdLineColorBgB = (int) b;
}

static void propertiesBox_apply_signal(GnomePropertyBox *propertyBoxWidget, gint page, gpointer data)
{
    GtkStyle *style;
    int i;

    if(propTmp.showTime != -1) {
	/* checkbox has been changed */
	prop.showTime = propTmp.showTime;
    }

    if(propTmp.showTime != -1 || propTmp.showDate != -1) {
	/* checkbox has been changed */
	prop.showDate = propTmp.showDate;
    }

    if(propTmp.showTime != -1 || propTmp.showDate != -1) {
	/* checkbox has been changed */
	if(prop.showTime && prop.showDate)
	    showMessage((gchar *) _("time & date on")); 
	else if(prop.showTime && !prop.showDate)
	    showMessage((gchar *) _("time on")); 
	else if(!prop.showTime && prop.showDate)
	    showMessage((gchar *) _("date on")); 
	else
	    showMessage((gchar *) _("clock off")); 
	
	propTmp.showTime = -1;
	propTmp.showDate = -1;
    }

    if(propTmp.normalSizeX != -1)
	prop.normalSizeX = propTmp.normalSizeX;
    if(propTmp.normalSizeY != -1)
	prop.normalSizeY = propTmp.normalSizeY;
    if(propTmp.reducedSizeX != -1)
	prop.reducedSizeX = propTmp.reducedSizeX;
    if(propTmp.reducedSizeY != -1)
	prop.reducedSizeY = propTmp.reducedSizeY;
    if(propTmp.cmdLineY != -1)
	prop.cmdLineY = propTmp.cmdLineY;

    if(propTmp.normalSizeX != -1 || propTmp.normalSizeY != -1 || 
       propTmp.reducedSizeX != -1 || propTmp.reducedSizeY != -1)
	{
	    /* size was changed */
	    gtk_container_set_resize_mode(GTK_CONTAINER(applet), GTK_RESIZE_IMMEDIATE);
	    gtk_widget_set_usize(GTK_WIDGET(applet), prop.normalSizeX, prop.normalSizeY);
	    gtk_widget_show (applet);
	    gtk_widget_queue_resize(applet);
	    gtk_widget_queue_draw(applet);
	    gtk_widget_set_uposition(applet, 1, 2);
	    gtk_container_check_resize(GTK_CONTAINER(applet));
	    gtk_container_resize_children(GTK_CONTAINER(applet));
	}

    if(propTmp.cmdLineY != -1)
	gtk_widget_set_usize(GTK_WIDGET(entryCommand), -1, prop.cmdLineY);

    propTmp.normalSizeX = -1;
    propTmp.normalSizeY = -1;
    propTmp.reducedSizeX = -1;
    propTmp.reducedSizeY = -1;
    propTmp.cmdLineY = -1;

    /* colors */
    if(propTmp.cmdLineColorFgR != -1)
	{
	    prop.cmdLineColorFgR = propTmp.cmdLineColorFgR;
	    prop.cmdLineColorFgG = propTmp.cmdLineColorFgG;
	    prop.cmdLineColorFgB = propTmp.cmdLineColorFgB;
	}
    if(propTmp.cmdLineColorBgR != -1)
	{
	    prop.cmdLineColorBgR = propTmp.cmdLineColorBgR;
	    prop.cmdLineColorBgG = propTmp.cmdLineColorBgG;
	    prop.cmdLineColorBgB = propTmp.cmdLineColorBgB;
	}

    if(propTmp.cmdLineColorFgR != -1 || propTmp.cmdLineColorBgR != -1)
	{
	    /* change widget style */
	    /* style = malloc(sizeof(GtkStyle)); */
	    style = gtk_style_copy(gtk_widget_get_style(entryCommand));
	    
	    style->fg[GTK_STATE_NORMAL].red = (gushort) prop.cmdLineColorFgR;
	    style->fg[GTK_STATE_NORMAL].green = (gushort) prop.cmdLineColorFgG;
	    style->fg[GTK_STATE_NORMAL].blue = (gushort) prop.cmdLineColorFgB;
	    
	    style->base[GTK_STATE_NORMAL].red = (gushort) prop.cmdLineColorBgR;
	    style->base[GTK_STATE_NORMAL].green = (gushort) prop.cmdLineColorBgG;
	    style->base[GTK_STATE_NORMAL].blue = (gushort) prop.cmdLineColorBgB;
	    
	    gtk_widget_set_style(entryCommand, style);
	}

    propTmp.cmdLineColorFgR = -1;
    propTmp.cmdLineColorFgG = -1;
    propTmp.cmdLineColorFgB = -1;
    propTmp.cmdLineColorBgR = -1;
    propTmp.cmdLineColorBgG = -1;
    propTmp.cmdLineColorBgB = -1;

    /* prefix & command */
    for(i=0; i<=MAX_PREFIXES-1; i++)
	{
	    if (propTmp.prefix[i] != (char *) NULL)
		{
		    free(prop.prefix[i]);
		    prop.prefix[i] = propTmp.prefix[i];
		    propTmp.prefix[i] = (char *) NULL;
		}

	    if (propTmp.command[i] != (char *) NULL)
		{
		    free(prop.command[i]);
		    prop.command[i] = propTmp.command[i];
		    propTmp.command[i] = (char *) NULL;
		}
	}    


    /*    gtk_widget_queue_resize(applet); */

    /* why is this not done automatically??? */
    /*    saveSession(); */
}

void loadSession(void)
{
    int i;
    char section[100];
    char defaultPrefix[MAX_PREFIX_LENGTH];
    char defaultCommand[MAX_COMMAND_LENGTH];
    
    /* read properties */
    /* gnome_config_push_prefix(APPLET_WIDGET(applet)->globcfgpath); */
    gnome_config_push_prefix("/mini-commander/");

    prop.showTime = gnome_config_get_bool("mini_commander/show_time=true");
    prop.showDate = gnome_config_get_bool("mini_commander/show_date=false");

    /* size */
    prop.normalSizeX = gnome_config_get_int("mini_commander/normal_size_x=128");
    prop.normalSizeY = gnome_config_get_int("mini_commander/normal_size_y=48");
    prop.reducedSizeX = gnome_config_get_int("mini_commander/reduced_size_x=50");
    prop.reducedSizeY = gnome_config_get_int("mini_commander/reduced_size_y=48");
    prop.cmdLineY = gnome_config_get_int("mini_commander/cmd_line_y=24");

    /* colors */
    prop.cmdLineColorFgR = gnome_config_get_int("mini_commander/cmd_line_color_fg_r=54613");
    prop.cmdLineColorFgG = gnome_config_get_int("mini_commander/cmd_line_color_fg_g=54613");
    prop.cmdLineColorFgB = gnome_config_get_int("mini_commander/cmd_line_color_fg_b=54613");

    prop.cmdLineColorBgR = gnome_config_get_int("mini_commander/cmd_line_color_bg_r=0");
    prop.cmdLineColorBgG = gnome_config_get_int("mini_commander/cmd_line_color_bg_g=0");
    prop.cmdLineColorBgB = gnome_config_get_int("mini_commander/cmd_line_color_bg_b=0");


    for(i=0; i<=MAX_PREFIXES-1; i++)
	{
	    switch (i+1) 
		/* set default prefixes */
		{
		case 1:
		    strcpy(defaultPrefix, "http://");
		    strcpy(defaultCommand, "netscape -remote openURL\\(http://$1\\) || netscape http://$1");
		    break;
		case 2:
		    strcpy(defaultPrefix, "ftp://");
		    strcpy(defaultCommand, "netscape -remote openURL\\(ftp://$1\\) || netscape ftp://$1");
		    break;
		case 3:
		    strcpy(defaultPrefix, "www.");
		    strcpy(defaultCommand, "netscape -remote openURL\\(http://www.$1\\) || netscape http://www.$1");
		    break;
		case 4:
		    strcpy(defaultPrefix, "ftp.");
		    strcpy(defaultCommand, "netscape -remote openURL\\(ftp.://$1\\) || netscape ftp://ftp.$1");
		    break;
		case 5:
		    strcpy(defaultPrefix, "lynx:");
		    strcpy(defaultCommand, "gnome-terminal -e sh -c 'lynx $1'");
		    break;
		case 6:
		    strcpy(defaultPrefix, "term:");
		    strcpy(defaultCommand, "gnome-terminal -e sh -c '$1'");
		    break;
		case 7:
		    strcpy(defaultPrefix, "xterm:");
		    strcpy(defaultCommand, "xterm -e sh -c '$1'");
		    break;
		case 8:
		    strcpy(defaultPrefix, "nxterm:");
		    strcpy(defaultCommand, "nxterm -e sh -c '$1'");
		    break;
		case 9:
		    strcpy(defaultPrefix, "rxvt:");
		    strcpy(defaultCommand, "rxvt -e sh -c '$1'");
		    break;
		case 10:
		    strcpy(defaultPrefix, "less:");
		    strcpy(defaultCommand, "$1 | gless");
		    break;
		case 11:
		    strcpy(defaultPrefix, " t");
		    strcpy(defaultCommand, "gnome-terminal");
		    break;
		case 12:
		    strcpy(defaultPrefix, " n");
		    strcpy(defaultCommand, "netscape");
		    break;
		default:
		    strcpy(defaultPrefix, "");
		    strcpy(defaultCommand, "");		   
		}

	    sprintf(section, "mini_commander/prefix_%d=%s", i+1, defaultPrefix);
	    free(prop.prefix[i]);
	    prop.prefix[i] = (char *) gnome_config_get_string((gchar *) section);

	    sprintf(section, "mini_commander/command_%d=%s", i+1, defaultCommand);
	    free(prop.command[i]);
	    prop.command[i] = (char *) gnome_config_get_string((gchar *) section);

	    propTmp.prefix[i] = (char *) NULL;
	    propTmp.command[i] = (char *) NULL;
	}    

    gnome_config_pop_prefix();
}


void saveSession(void)
{
    int i;
    char section[100];

    /* gnome_config_push_prefix(globcfgpath); */
    gnome_config_push_prefix("/mini-commander/"); 

    /* version */
    gnome_config_set_string("mini_commander/version", (gchar *) VERSION);

    /* clock */
    gnome_config_set_bool("mini_commander/show_time", prop.showTime);
    gnome_config_set_bool("mini_commander/show_date", prop.showDate);

    /* size */
    gnome_config_set_int("mini_commander/normal_size_x", prop.normalSizeX);
    gnome_config_set_int("mini_commander/normal_size_y", prop.normalSizeY);
    gnome_config_set_int("mini_commander/reduced_size_x", prop.reducedSizeX);
    gnome_config_set_int("mini_commander/reduced_size_y", prop.reducedSizeY);
    gnome_config_set_int("mini_commander/cmd_line_y", prop.cmdLineY);

    /* colors */
    gnome_config_set_int("mini_commander/cmd_line_color_fg_r", prop.cmdLineColorFgR);
    gnome_config_set_int("mini_commander/cmd_line_color_fg_g", prop.cmdLineColorFgG);
    gnome_config_set_int("mini_commander/cmd_line_color_fg_b", prop.cmdLineColorFgB);

    gnome_config_set_int("mini_commander/cmd_line_color_bg_r", prop.cmdLineColorBgR);
    gnome_config_set_int("mini_commander/cmd_line_color_bg_g", prop.cmdLineColorBgG);
    gnome_config_set_int("mini_commander/cmd_line_color_bg_b", prop.cmdLineColorBgB);

    /* prefix & command */
    for(i=0; i<=MAX_PREFIXES-1; i++)
	{
	    sprintf(section, "mini_commander/prefix_%d", i+1);
	    gnome_config_set_string((gchar *) section, (gchar *) prop.prefix[i]);

	    sprintf(section, "mini_commander/command_%d", i+1);
	    gnome_config_set_string((gchar *) section, (gchar *) prop.command[i]);

	    propTmp.prefix[i] = (char *) NULL;
	    propTmp.command[i] = (char *) NULL;
	}    

  
    gnome_config_sync();
    /* you need to use the drop_all here since we're all writing to
       one file, without it, things might not work too well */
    gnome_config_drop_all();
    
    /* make sure you return FALSE, otherwise your applet might not
       work compeltely, there are very few circumstances where you
       want to return TRUE. This behaves similiar to GTK events, in
       that if you return FALSE it means that you haven't done
       everything yourself, meaning you want the panel to save your
       other state such as the panel you are on, position,
       parameter, etc ... */

    gnome_config_pop_prefix(); 
}

gint saveSession_signal(GtkWidget *widget, const char *privcfgpath, const char *globcfgpath)
{       
    showMessage((gchar *) _("saving prefs...")); 	    
    saveSession();

    return FALSE;
}

void propertiesBox(AppletWidget *applet, gpointer data)
{
    GtkWidget *propertiesBox;
    GtkWidget *vbox, *vbox1, *frame;
    GtkWidget *hbox, *hbox1;
    GtkWidget *table;
    GtkWidget *checkTime, *checkDate;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *colorPicker;
    char textLabel[50], buffer[50];
    int i, j;
    
    propertiesBox = gnome_property_box_new();
    
    gtk_window_set_title(GTK_WINDOW(propertiesBox), _("Mini-Commander Properties"));
    
    /* time & date */
    vbox = gtk_vbox_new(FALSE, GNOME_PAD_BIG);
    gtk_container_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);

    frame = gtk_frame_new(_("Clock"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(frame), vbox1);

    /* show time check box */
    checkTime = gtk_check_button_new_with_label (_("Show time"));
    gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(checkTime), prop.showTime);
    gtk_signal_connect(GTK_OBJECT(checkTime),
		       "toggled",
		       GTK_SIGNAL_FUNC(checkBoxToggled_signal),
		       &propTmp.showTime);
    gtk_signal_connect_object(GTK_OBJECT(checkTime), "toggled",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(propertiesBox));  
    gtk_box_pack_start(GTK_BOX(vbox1), checkTime, FALSE, TRUE, 0);

    /* show date check box */

    checkDate = gtk_check_button_new_with_label (_("Show date"));
    gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(checkDate), prop.showDate);
    gtk_signal_connect(GTK_OBJECT(checkDate),
		       "toggled",
		       GTK_SIGNAL_FUNC(checkBoxToggled_signal),
		       &propTmp.showDate);
    gtk_signal_connect_object(GTK_OBJECT(checkDate), "toggled",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(propertiesBox));
    gtk_box_pack_start(GTK_BOX(vbox1), checkDate, FALSE, TRUE, 0);

    /* Size */
    frame = gtk_frame_new(_("Size"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(5, 4, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), GNOME_PAD_SMALL); 
    /*    gtk_table_set_row_spacing(GTK_TABLE(table), 1, GNOME_PAD_SMALL); 
	  gtk_table_set_row_spacing(GTK_TABLE(table), 2, 20);  */ /* no effect? */
    gtk_table_set_col_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
    gtk_container_border_width(GTK_CONTAINER(table), GNOME_PAD_SMALL);
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
    sprintf(buffer, "%d", prop.normalSizeX);
    gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);
    gtk_widget_set_usize(entry, 50, -1);
    gtk_signal_connect(GTK_OBJECT(entry),
		       "changed",
		       GTK_SIGNAL_FUNC(entryIntChanged_signal),
		       &propTmp.normalSizeX);
    gtk_signal_connect_object(GTK_OBJECT(entry),
			      "changed",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(propertiesBox));      
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
    sprintf(buffer, "%d", prop.normalSizeY);
    gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);
    gtk_widget_set_usize(entry, 50, -1);
    gtk_signal_connect(GTK_OBJECT(entry),
		       "changed",
		       GTK_SIGNAL_FUNC(entryIntChanged_signal),
		       &propTmp.normalSizeY);
    gtk_signal_connect_object(GTK_OBJECT(entry),
			      "changed",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(propertiesBox));      
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
    sprintf(buffer, "%d", prop.cmdLineY);
    gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);
    gtk_widget_set_usize(entry, 50, -1);
    gtk_signal_connect(GTK_OBJECT(entry),
		       "changed",
		       GTK_SIGNAL_FUNC(entryIntChanged_signal),
		       &propTmp.cmdLineY);
    gtk_signal_connect_object(GTK_OBJECT(entry),
			      "changed",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(propertiesBox));      
    gtk_table_attach(GTK_TABLE(table), 
		     entry,
		     1, 2,
		     2, 3,
		     GTK_FILL, 0,
		     0, 0);
    
    /* hint */
    label = gtk_label_new((gchar *) _("\nSometimes the applet has to be moved on the panel\nto make a change of the size visible."));
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_table_attach(GTK_TABLE(table), 
		     label,
		     0, 5,
		     3, 4,
		     GTK_FILL, 0,
		     0, 0);

    /* Color */
    frame = gtk_frame_new(_("Colors"));
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(2, 2, FALSE);
    gtk_table_set_row_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
    gtk_table_set_col_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
    gtk_container_border_width(GTK_CONTAINER(table), GNOME_PAD_SMALL);
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

    colorPicker = gnome_color_picker_new();
    gnome_color_picker_set_i16(GNOME_COLOR_PICKER(colorPicker),
			       prop.cmdLineColorFgR, 
			       prop.cmdLineColorFgG, 
			       prop.cmdLineColorFgB, 
			       0);
    gtk_signal_connect(GTK_OBJECT(colorPicker),
		       "color_set",
		       GTK_SIGNAL_FUNC(colorCmdFgChanged_signal),
		       (gpointer *) NULL);
    gtk_signal_connect_object(GTK_OBJECT(colorPicker),
			      "color_set",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(propertiesBox));      
    //    gtk_box_pack_start(GTK_BOX(hbox), colorPicker, FALSE, TRUE, 0); 
    gtk_table_attach(GTK_TABLE(table), 
		     colorPicker,
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
    colorPicker = gnome_color_picker_new();
    gnome_color_picker_set_i16(GNOME_COLOR_PICKER(colorPicker),
			       prop.cmdLineColorBgR, 
			       prop.cmdLineColorBgG, 
			       prop.cmdLineColorBgB, 
			       0);
    gtk_signal_connect(GTK_OBJECT(colorPicker),
		       "color_set",
		       GTK_SIGNAL_FUNC(colorCmdBgChanged_signal),
		       (gpointer *) NULL);
    gtk_signal_connect_object(GTK_OBJECT(colorPicker),
			      "color_set",
			      GTK_SIGNAL_FUNC(gnome_property_box_changed),
			      GTK_OBJECT(propertiesBox));      
    gtk_table_attach(GTK_TABLE(table), 
		     colorPicker,
		     1, 2,
		     1, 2,
		     0, 0,
		     0, 0);
    


    gnome_property_box_append_page(GNOME_PROPERTY_BOX(propertiesBox),
				   vbox,
				   gtk_label_new(_("General")));



    /*Prefixes */
    for(j=0; j<3; j++)
	{
	    vbox = gtk_vbox_new(FALSE, GNOME_PAD_BIG);
	    gtk_container_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);
	    
	    frame = gtk_frame_new(_("Special Prefixes"));
	    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
	    
	    vbox1 = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	    gtk_container_border_width(GTK_CONTAINER(vbox1), GNOME_PAD_SMALL);
	    gtk_container_add(GTK_CONTAINER(frame), vbox1);
	    
	    /*    gtk_box_pack_start(GTK_BOX(vbox1),
		  gtk_label_new((gchar *) _("HGjshd ajhs djah dkjha sjhd akjshda jshdajks dhjkashd ahjshd ajkshd ajkshd jkashdjkas hdjkas hdjkashd jhasd jkashdjkh.")), 
		  FALSE, 
		  FALSE, 
		  0); */
	    
	    for(i=(MAX_PREFIXES/3)*j; i<=(MAX_PREFIXES/3)*(j+1)-1; i++)
		{
		    hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
		    gtk_box_pack_start(GTK_BOX(vbox1), hbox, TRUE, TRUE, 0);
		    
		    /* prefix */    
		    sprintf(textLabel, _("Prefix %.2d:"), i+1);
		    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(textLabel), FALSE, TRUE, 0);
		    
		    entry = gtk_entry_new_with_max_length(MAX_PREFIX_LENGTH);
		    gtk_widget_set_usize(entry, 75, -1);
		    if (prop.prefix[i] != (gchar *) NULL)
			gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) prop.prefix[i]);
		    gtk_signal_connect(GTK_OBJECT(entry),
				       "changed",
				       GTK_SIGNAL_FUNC(entryChanged_signal),
				       &propTmp.prefix[i]);
		    gtk_signal_connect_object(GTK_OBJECT(entry),
					      "changed",
					      GTK_SIGNAL_FUNC(gnome_property_box_changed),
					      GTK_OBJECT(propertiesBox));      
		    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, TRUE, 0);
		    
		    /* command */
		    sprintf(textLabel, _("   Command %.2d:"), i+1);
		    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(textLabel), FALSE, TRUE, 0);
		    
		    entry = gtk_entry_new_with_max_length(MAX_COMMAND_LENGTH);
		    if (prop.command[i] != (gchar *) NULL)
			gtk_entry_set_text(GTK_ENTRY(entry), prop.command[i]);
		    gtk_signal_connect(GTK_OBJECT(entry),
				       "changed",
				       GTK_SIGNAL_FUNC(entryChanged_signal),
				       &propTmp.command[i]);
		    gtk_signal_connect_object(GTK_OBJECT(entry),
					      "changed",
					      GTK_SIGNAL_FUNC(gnome_property_box_changed),
					      GTK_OBJECT(propertiesBox));  
		    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
		}
	    
	    sprintf(textLabel, _("Prefixes %d"), j+1);
	    gnome_property_box_append_page(GNOME_PROPERTY_BOX(propertiesBox),
					   vbox,
					   gtk_label_new((gchar *) textLabel));
	}

    gtk_signal_connect(GTK_OBJECT(propertiesBox), 
		       "apply",
		       GTK_SIGNAL_FUNC(propertiesBox_apply_signal),
		       NULL);
    
    gtk_widget_show_all(propertiesBox);
}

