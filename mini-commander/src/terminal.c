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
static int historyPosition = HISTORY_DEPTH;
static char browsedFilename[300] = "";

static gchar * historyAutoComplete(GtkWidget *widget, GdkEventKey *event);

static gint
commandKey_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    guint key = event->keyval;
    char *command;
    static char currentCommand[MAX_COMMAND_LENGTH];
    char buffer[MAX_COMMAND_LENGTH];
    int propagateEvent = TRUE;

    /* printf("%d,%d,%d;  ", (gint16) event->keyval, event->state, event->length); */

    if(key == GDK_Tab
       || key == GDK_KP_Tab
       || key == GDK_ISO_Left_Tab)
	{
	    /* tab key pressed */
	    strcpy(buffer, (char *) gtk_entry_get_text(GTK_ENTRY(widget)));
	    cmdCompletion(buffer);
	    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) buffer);

	    propagateEvent = FALSE;
	}
    else if(key == GDK_Up
	    || key == GDK_KP_Up
	    || key == GDK_ISO_Move_Line_Up
	    || key == GDK_Pointer_Up)
	{
	    /* up key pressed */
	    if(historyPosition == HISTORY_DEPTH)
		{	    
		    /* store current command line */
		    strcpy(currentCommand, (char *) gtk_entry_get_text(GTK_ENTRY(widget)));
		}
	    if(historyPosition > 0 && existsHistoryEntry(historyPosition - 1))
		{
		    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) getHistoryEntry(--historyPosition));
		}
	    else
		showMessage((gchar *) _("end of history list"));

	    propagateEvent = FALSE;
	}
    else if(key == GDK_Down
	    || key == GDK_KP_Down
	    || key == GDK_ISO_Move_Line_Down
	    || key == GDK_Pointer_Down)
	{
	    /* down key pressed */
	    if(historyPosition <  HISTORY_DEPTH - 1)
		{
		    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) getHistoryEntry(++historyPosition));
		}
	    else if(historyPosition == HISTORY_DEPTH - 1)
		{	    
		    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) currentCommand);
		    ++historyPosition;
		}
	    else
		showMessage((gchar *) _("end of history list"));

	    propagateEvent = FALSE;
	}
    else if(key == GDK_Return
	    || key == GDK_KP_Enter
	    || key == GDK_ISO_Enter
	    || key == GDK_3270_Enter)
	{
	    /* enter pressed -> exec command */
	    showMessage((gchar *) _("starting...")); 
	    command = (char *) malloc(sizeof(char) * MAX_COMMAND_LENGTH);
	    strcpy(command, (char *) gtk_entry_get_text(GTK_ENTRY(widget)));
	    /* printf("%s\n", command); */
	    execCommand(command);

	    appendHistoryEntry((char *) command);
	    historyPosition = HISTORY_DEPTH;		   
	    free(command);

	    strcpy(currentCommand, "");
	    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) "");
	    propagateEvent = FALSE;
	}
    else if(prop.autoCompleteHistory && key >= GDK_space && key <= GDK_asciitilde )
	{
            char *completedCommand;
	    gint currentPosition = gtk_editable_get_position(GTK_EDITABLE(widget));
	    
	    if(currentPosition != 0)
		{
		    gtk_editable_delete_text( GTK_EDITABLE(widget), currentPosition, -1 );
		    completedCommand = historyAutoComplete(widget, event);
		    
		    if(completedCommand != NULL)
			{
			    gtk_entry_set_text(GTK_ENTRY(widget), completedCommand);
			    gtk_editable_set_position(GTK_EDITABLE(widget), currentPosition );
			    propagateEvent = FALSE;
			    showMessage((gchar *) _("autocompleted"));
			}
		}	    
	}
    

    if(propagateEvent == FALSE)
	{
	    /* I have to do this to stop gtk from propagating this event;
	       error in gtk? */
	    event->keyval = GDK_Right;
	    event->state = 0;
	    event->length = 0;  
	}
    return (propagateEvent == FALSE);
}


/* Thanks to Halfline <halfline@hawaii.rr.com> for his initial version
   of historyAutoComplete */
static gchar *
historyAutoComplete(GtkWidget *widget, GdkEventKey *event)
{
    gchar currentCommand[MAX_COMMAND_LENGTH];
    gchar* completedCommand;
    int i;

    sprintf(currentCommand, "%s%s", gtk_entry_get_text(GTK_ENTRY(widget)), event->string); 
    for(i = HISTORY_DEPTH - 1; i >= 0; i--) 
  	{
	    if(!existsHistoryEntry(i))
		break;
  	    completedCommand = getHistoryEntry(i); 
  	    if(!g_strncasecmp(completedCommand, currentCommand, strlen( currentCommand))) 
		return completedCommand; 
  	} 
    
    return NULL;
}
#endif


static gint
term_key_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    guint key = event->keyval;
    char buffer[MAX_COMMAND_LENGTH];
    int propagateEvent = TRUE;

    /* printf("%d,%d,%d;  ", (gint16) event->keyval, event->state, event->length); */

    if(key == GDK_Tab
       || key == GDK_KP_Tab
       || key == GDK_ISO_Left_Tab)
	{
	    /* tab key pressed */
	    strcpy(buffer, (char *) gtk_entry_get_text(GTK_ENTRY(widget)));
	    cmdCompletion(buffer);
	    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) buffer);

	    propagateEvent = FALSE;
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
}



static gint
term_focus_in_cb(GtkWidget *widget, gpointer data)
{
    printf("focusIn\n");
    gtk_widget_grab_focus(GTK_WIDGET(terminal_zvt));
    
    /* go on */
    return (FALSE);
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
    red[16] = (gushort) prop.cmdLineColorFgR;
    green[16] = (gushort) prop.cmdLineColorFgG;
    blue[16] = (gushort) prop.cmdLineColorFgB;
    
    red[17] = (gushort) prop.cmdLineColorBgR;
    green[17] = (gushort) prop.cmdLineColorBgG;
    blue[17] = (gushort) prop.cmdLineColorBgB;*/

    /* zvt_term_set_color_scheme(ZVT_TERM(terminal_zvt), red, green, blue); */

    /*     command_entry_update_size(); */
}
