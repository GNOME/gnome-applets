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

#include <string.h>
#include <config.h>
#include <gnome.h>
#include <applet-widget.h>
#include <gdk/gdkkeysyms.h>

#include "command_line.h"
#include "preferences.h"
#include "exec.h"
#include "cmd_completion.h"
#include "message.h"

GtkWidget *entryCommand;
static int historyPosition = HISTORY_DEPTH;
static char *historyCommand[HISTORY_DEPTH];

static gint commandKey_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    guint key = event->keyval;
    char *command;
    static char currentCommand[MAX_COMMAND_LENGTH];
    char buffer[MAX_COMMAND_LENGTH];
    int propagateEvent = TRUE;
    int pos;

    /* printf("%d,%d,%d;  ", (gint16) event->keyval, event->state, event->length); */

    if(key == GDK_Tab)
	{
	    /* tab key pressed */
	    strcpy(buffer, (char *) gtk_entry_get_text(GTK_ENTRY(widget)));
	    cmdCompletion(buffer);
	    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) buffer);

	    propagateEvent = FALSE;
	}
    else if(key == GDK_Up)
	{
	    /* up key pressed */
	    if(historyPosition == HISTORY_DEPTH)
		{	    
		    /* store current command line */
		    strcpy(currentCommand, (char *) gtk_entry_get_text(GTK_ENTRY(widget)));
		}
	    if(historyPosition > 0 && historyCommand[historyPosition - 1] != NULL)
		{
		    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) historyCommand[--historyPosition]);
		}
	    event->keyval = (gchar) -175;
	    propagateEvent = FALSE;
	}
    else if(key == GDK_Down)
	{
	    /* down key pressed */
	    if(historyPosition <  HISTORY_DEPTH - 1)
		{
		    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) historyCommand[++historyPosition]);
		}
	    else if(historyPosition == HISTORY_DEPTH - 1)
		{	    
		    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) currentCommand);
		    ++historyPosition;
		}
	    propagateEvent = FALSE;
	}
    else if(key == GDK_Return)
	{
	    /* enter pressed -> exec command */
	    showMessage((gchar *) _("starting...")); 
	    command = (char *) malloc(sizeof(char) * MAX_COMMAND_LENGTH);
	    strcpy(command, (char *) gtk_entry_get_text(GTK_ENTRY(widget)));
	    /* printf("%s\n", command); */
	    execCommand(command);

	    if(historyCommand[HISTORY_DEPTH - 1] == NULL
	       || strcmp(command, historyCommand[HISTORY_DEPTH - 1]) != 0)
	       {
		   /* this command is no dupe -> update history */
		   free(historyCommand[0]);
		   for(pos = 0; pos < HISTORY_DEPTH - 1; pos++)
		       {
			   historyCommand[pos] = historyCommand[pos+1];
			   /* printf("%s\n", historyCommand[pos]); */
		       }
		   historyCommand[HISTORY_DEPTH - 1] = command;   

	       }
	    historyPosition = HISTORY_DEPTH;		   

	    strcpy(currentCommand, "");
	    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) "");
	    propagateEvent = FALSE;
	}
    

    if(propagateEvent == FALSE)
	{
	    /* I have to do this to stop gtk from propagating this event;
	       error in gtk? */
	    event->keyval = (gchar) -173;
	    event->state = 0;
	    event->length = 0;  
	}
    return (propagateEvent == FALSE);
}

static gint commandFocusOut_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    /*
      printf("focusOut\n");
      gtk_widget_grab_focus(GTK_WIDGET(widget)); 
    */
    return (FALSE);
}

static gint activateCommandLine_signal(GtkWidget *widget, gpointer data)
{
      printf("focusIn\n");
      /*      gtk_widget_grab_focus(GTK_WIDGET(entryCommand)); */

      /* go on */
      return (FALSE);
}

void initCommandEntry(void)
{
        GtkStyle *style;
        GdkColor color;

	/* change style of command line to grey text on black ground */
	style = malloc(sizeof(GtkStyle));
	style = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(applet)));
	/* color = style->fg[GTK_STATE_NORMAL]; */
	/* style->text[GTK_STATE_NORMAL] = style->bg[GTK_STATE_NORMAL]; */ /* doesn't work; gtk? */
	/* style->fg[GTK_STATE_NORMAL] = style->bg[GTK_STATE_NORMAL];
	   style->base[GTK_STATE_NORMAL] = color; */

	style->fg[GTK_STATE_NORMAL].red = (gushort) prop.cmdLineColorFgR;
	style->fg[GTK_STATE_NORMAL].green = (gushort) prop.cmdLineColorFgG;
	style->fg[GTK_STATE_NORMAL].blue = (gushort) prop.cmdLineColorFgB;

	style->base[GTK_STATE_NORMAL].red = (gushort) prop.cmdLineColorBgR;
	style->base[GTK_STATE_NORMAL].green = (gushort) prop.cmdLineColorBgG;
	style->base[GTK_STATE_NORMAL].blue = (gushort) prop.cmdLineColorBgB;

        /* create the widget we are going to put on the applet */
	gtk_widget_push_style (style);
        entryCommand = gtk_entry_new_with_max_length((guint16) MAX_COMMAND_LENGTH); 
	gtk_widget_pop_style ();
	gtk_widget_set_usize(GTK_WIDGET(entryCommand), -1, prop.cmdLineY);


	/*	gtk_widget_set_style(GTK_WIDGET(entryCommand), style); */

	/*	gtk_signal_connect(GTK_OBJECT(entryCommand), "activate",
			   GTK_SIGNAL_FUNC(commandEntered_cb),
			   entryCommand); */
	gtk_signal_connect(GTK_OBJECT(entryCommand), "key_press_event",
			   GTK_SIGNAL_FUNC(commandKey_event),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(entryCommand), "focus_out_event",
			   GTK_SIGNAL_FUNC(commandFocusOut_event),
			   NULL);

}
