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

static gint fileBrowserOK_signal(GtkWidget *widget, gpointer fileSelect);
static void historySelectionMade_cb(GtkWidget *clist, gint row, gint column,
				    GdkEventButton *event, gpointer data);

GtkWidget *entryCommand;
static int historyPosition = HISTORY_DEPTH;
static char *historyCommand[HISTORY_DEPTH];
static char browsedFilename[300] = "";

static gint
commandKey_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
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

static gint
commandFocusOut_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    /*
      printf("focusOut\n");
      gtk_widget_grab_focus(GTK_WIDGET(widget)); 
    */
    return (FALSE);
}

static gint
activateCommandLine_signal(GtkWidget *widget, gpointer data)
{
    printf("focusIn\n");
    /*      gtk_widget_grab_focus(GTK_WIDGET(entryCommand)); */
    
    /* go on */
    return (FALSE);
}

static void
historySelectionMade_cb(GtkWidget *clist, gint row, gint column,
			GdkEventButton *event, gpointer data)
{
    gchar *command;

    gtk_clist_get_text(GTK_CLIST(clist), row, column, &command);
    execCommand(command);

    /* close history window */
    gtk_widget_destroy(GTK_WIDGET(clist->parent->parent));
}


gint 
showHistory_signal(GtkWidget *widget, gpointer data)
{
    GtkWidget *window;
    GtkWidget *scrolled_window;
    GtkWidget *clist;
    gchar *commandList[1];
    int i;

    window = gtk_window_new(GTK_WINDOW_DIALOG);
    /* position */
    gtk_window_position(GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    /* size */
    gtk_widget_set_usize(GTK_WIDGET(window), 200,350);
    /* border */
    gtk_container_border_width(GTK_CONTAINER(window), 3);
    /* title */
    gtk_window_set_title(GTK_WINDOW(window), (gchar *) _("Command history"));

    /* scrollbars */
    /* create scrolled window to put the GtkList widget inside */
    scrolled_window=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(window), scrolled_window);
    gtk_widget_show(scrolled_window);


    clist = gtk_clist_new(1);
    gtk_signal_connect(GTK_OBJECT(clist),
		       "select_row",
		       GTK_SIGNAL_FUNC(historySelectionMade_cb),
		       NULL);
    gtk_widget_show(window);

    /* add history items */
    for(i = 0; i < HISTORY_DEPTH; i++)
	{
	    if(historyCommand[i] != NULL)
		{
		    commandList[0] = (gchar *) malloc(sizeof(gchar) * (strlen(historyCommand[i]) + 1));
		    /* commandList[0] = (gchar *) malloc(sizeof(gchar) * (MAX_COMMAND_LENGTH + 1)); */
		    strcpy(commandList[0], historyCommand[i]);
		    gtk_clist_append(GTK_CLIST(clist), commandList);
		    free(commandList[0]);
		}
	}
    gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
    gtk_widget_show(clist);    

    
    /* FIXME: write this routine */
    /*
    gnome_dialog_run
	(GNOME_DIALOG
	 (gnome_message_box_new((gchar *) _("The history list comes later."),
				GNOME_MESSAGE_BOX_INFO,
				GNOME_STOCK_BUTTON_OK,
				NULL)
	  )
	 );
    */
    
    /* go on */
    return FALSE;  
}

static gint 
fileBrowserOK_signal(GtkWidget *widget, gpointer fileSelect)
{
    /* get selected file name */
    strcpy(browsedFilename, (char *) gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileSelect)));

    /* destroy file select dialog */
    gtk_widget_destroy(GTK_WIDGET(fileSelect));
    
    /* printf("Filename: %s\n", (char *)  browsedFilename); */

    /* execute command */
    execCommand(browsedFilename);

    /* go on */
    return FALSE;  
}

gint 
showFileBrowser_signal(GtkWidget *widget, gpointer data)
{
    /* FIXME: write this routine */
    
    GtkWidget *fileSelect;

    


    /* build file select dialog */
    fileSelect = gtk_file_selection_new((gchar *) _("Start program"));
    gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fileSelect)->ok_button),
		       "clicked",
		       GTK_SIGNAL_FUNC(fileBrowserOK_signal),
		       GTK_OBJECT(fileSelect));
    gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fileSelect)->cancel_button),
			      "clicked",
			      GTK_SIGNAL_FUNC(gtk_widget_destroy),
			      GTK_OBJECT (fileSelect));

    /* set path to last selected path */
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(fileSelect),
				    (gchar *) browsedFilename);

    /* Set as modal */
    gtk_window_set_modal(GTK_WINDOW(fileSelect),TRUE);

    gtk_window_position(GTK_WINDOW (fileSelect), GTK_WIN_POS_MOUSE);

    gtk_widget_show(fileSelect);

    /* go on */
    return FALSE;  
}

void
initCommandEntry(void)
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
