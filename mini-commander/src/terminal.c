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

#include <string.h>
#include <config.h>
#include <gnome.h>
#include <applet-widget.h>
#include <gdk/gdkkeysyms.h>
#include <zvt/zvtterm.h>

#include "command_line.h"
#include "preferences.h"
#include "exec.h"
#include "cmd_completion.h"
#include "history.h"
#include "message.h"

#include "terminal.h"

GtkWidget *terminal_zvt = NULL;
#if 0
static int history_position = LENGTH_HISTORY_LIST;
static char browsed_filename[300] = "";

static gchar * history_auto_complete(GtkWidget *widget, GdkEventKey *event);

static gint
command_key_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    guint key = event->keyval;
    char *command, *temp_command;
    static char current_command[MAX_COMMAND_LENGTH];
    char buffer[MAX_COMMAND_LENGTH];
    int propagate_event = TRUE;

    /* printf("%d,%d,%d;  ", (gint16) event->keyval, event->state, event->length); */

    if(key == GDK_Tab
       || key == GDK_KP_Tab
       || key == GDK_ISO_Left_Tab)
	{
	    /* tab key pressed */
	    temp_command = gtk_entry_get_text(GTK_ENTRY(widget));
	    if (strlen(temp_command) > sizeof(buffer))
		    return;
	    strcpy(buffer, temp_command);
	    cmd_completion(buffer);
	    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) buffer);

	    propagate_event = FALSE;
	}
    else if(key == GDK_Up
	    || key == GDK_KP_Up
	    || key == GDK_ISO_Move_Line_Up
	    || key == GDK_Pointer_Up)
	{
	    /* up key pressed */
	    if(history_position == LENGTH_HISTORY_LIST)
		{	    
		    temp_command = gtk_entry_get_text(GTK_ENTRY(widget));
		    if (strlen(temp_command) > sizeof(buffer))
			    return;
		    /* store current command line */
		    strcpy(current_command, temp_command);
		}
	    if(history_position > 0 && exists_history_entry(history_position - 1))
		{
		    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) get_history_entry(--history_position));
		}
	    else
		show_message((gchar *) _("end of history list"));

	    propagate_event = FALSE;
	}
    else if(key == GDK_Down
	    || key == GDK_KP_Down
	    || key == GDK_ISO_Move_Line_Down
	    || key == GDK_Pointer_Down)
	{
	    /* down key pressed */
	    if(history_position <  LENGTH_HISTORY_LIST - 1)
		{
		    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) get_history_entry(++history_position));
		}
	    else if(history_position == LENGTH_HISTORY_LIST - 1)
		{	    
		    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) current_command);
		    ++history_position;
		}
	    else
		show_message((gchar *) _("end of history list"));

	    propagate_event = FALSE;
	}
    else if(key == GDK_Return
	    || key == GDK_KP_Enter
	    || key == GDK_ISO_Enter
	    || key == GDK_3270_Enter)
	{
	    /* enter pressed -> exec command */
	    show_message((gchar *) _("starting...")); 
	    command = (char *) malloc(sizeof(char) * MAX_COMMAND_LENGTH);
	    temp_command = gtk_entry_get_text(GTK_ENTRY(widget));
	    if (strlen(temp_command) > sizeof(command))
		    return;
	    strcpy(command, temp_command);
	    /* printf("%s\n", command); */
	    exec_command(command);

	    append_history_entry((char *) command);
	    history_position = LENGTH_HISTORY_LIST;		   
	    free(command);

	    strcpy(current_command, "");
	    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) "");
	    propagate_event = FALSE;
	}
    else if(prop.auto_complete_history && key >= GDK_space && key <= GDK_asciitilde )
	{
            char *completed_command;
	    gint current_position = gtk_editable_get_position(GTK_EDITABLE(widget));
	    
	    if(current_position != 0)
		{
		    gtk_editable_delete_text( GTK_EDITABLE(widget), current_position, -1 );
		    completed_command = history_auto_complete(widget, event);
		    
		    if(completed_command != NULL)
			{
			    gtk_entry_set_text(GTK_ENTRY(widget), completed_command);
			    gtk_editable_set_position(GTK_EDITABLE(widget), current_position );
			    propagate_event = FALSE;
			    show_message((gchar *) _("autocompleted"));
			}
		}	    
	}
    

    if(propagate_event == FALSE)
	{
	    /* I have to do this to stop gtk from propagating this event;
	       error in gtk? */
	    event->keyval = GDK_Right;
	    event->state = 0;
	    event->length = 0;  
	}
    return (propagate_event == FALSE);
}


/* Thanks to Halfline <halfline@hawaii.rr.com> for his initial version
   of history_auto_complete */
static gchar *
history_auto_complete(GtkWidget *widget, GdkEventKey *event)
{
    gchar current_command[MAX_COMMAND_LENGTH];
    gchar* completed_command;
    int i;

    sprintf(current_command, "%s%s", gtk_entry_get_text(GTK_ENTRY(widget)), event->string); 
    for(i = LENGTH_HISTORY_LIST - 1; i >= 0; i--) 
  	{
	    if(!exists_history_entry(i))
		break;
  	    completed_command = get_history_entry(i); 
  	    if(!g_strncasecmp(completed_command, current_command, strlen( current_command))) 
		return completed_command; 
  	} 
    
    return NULL;
}
#endif


static gint
term_key_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    guint key = event->keyval;
    char buffer[MAX_COMMAND_LENGTH];
    int propagate_event = TRUE;
    char *temp_command;
    
    /* printf("%d,%d,%d;  ", (gint16) event->keyval, event->state, event->length); */

    if(key == GDK_Tab
       || key == GDK_KP_Tab
       || key == GDK_ISO_Left_Tab)
	{
	    /* tab key pressed */
	    temp_command = gtk_entry_get_text(GTK_ENTRY(widget));
	    if (strlen(temp_command) > sizeof(buffer))
		    return FALSE;
	    strcpy(buffer, temp_command);
	    cmd_completion(buffer);
	    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) buffer);

	    propagate_event = FALSE;
	}
    else
	{
	    /* zvt_term_feed(ZVT_TERM(terminal_zvt), "Ready.", sizeof("Ready.")); */
	    buffer[0] = key;
	    buffer[1] = '\0';
/* 	    zvt_term_feed(ZVT_TERM(terminal_zvt), buffer, sizeof(buffer)); */
	    zvt_term_feed(ZVT_TERM(terminal_zvt), event->string, sizeof(char));
	}

    return(TRUE);
    data = NULL;
}



static gint
term_focus_in_cb(GtkWidget *widget, gpointer data)
{
    printf("focus_in\n");
    gtk_widget_grab_focus(GTK_WIDGET(terminal_zvt));
    
    /* go on */
    return (FALSE);
    widget = NULL;
    data = NULL;
}


void
terminal_init(void)
{
    gushort red[18], green[18], blue[18];
    int i;

    if(terminal_zvt)
    	gtk_widget_destroy(GTK_WIDGET(terminal_zvt));    
    
    /* create the widget we are going to put on the applet */
    terminal_zvt = zvt_term_new_with_size(16,1);
    /* in case we get destroyed elsewhere */
    gtk_signal_connect(GTK_OBJECT(terminal_zvt),"destroy",
		       GTK_SIGNAL_FUNC(gtk_widget_destroyed),
		       &terminal_zvt);
    zvt_term_set_blink (ZVT_TERM(terminal_zvt), TRUE);   
    zvt_term_feed(ZVT_TERM(terminal_zvt), "Ready.", sizeof("Ready."));

    gtk_signal_connect(GTK_OBJECT(terminal_zvt), "key_press_event",
		       GTK_SIGNAL_FUNC(term_key_cb),
		       NULL);

    gtk_signal_connect(GTK_OBJECT(terminal_zvt), "enter_notify_event",
		       GTK_SIGNAL_FUNC(term_focus_in_cb),
		       NULL);

    /* set colors */

    for(i = 0; i < 18; i++)
	{
	    red[i] = 0xffff;
	    green[i] = 0xffff;
	    blue[i] = 0xffff;
	}

    /*
    red[16] = (gushort) prop.cmd_line_color_fg_r;
    green[16] = (gushort) prop.cmd_line_color_fg_g;
    blue[16] = (gushort) prop.cmd_line_color_fg_b;
    
    red[17] = (gushort) prop.cmd_line_color_bg_r;
    green[17] = (gushort) prop.cmd_line_color_bg_g;
    blue[17] = (gushort) prop.cmd_line_color_bg_b;*/

    /* zvt_term_set_color_scheme(ZVT_TERM(terminal_zvt), red, green, blue); */

    /*     command_entry_update_size(); */
}
