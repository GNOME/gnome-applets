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

#include "command_line.h"
#include "preferences.h"
#include "exec.h"
#include "cmd_completion.h"
#include "history.h"
#include "message.h"

static gint fileBrowserOK_signal(GtkWidget *widget, gpointer fileSelect);
static gint historyItemClicked_cb(GtkWidget *widget, gpointer data);
static gint historyPopupClicked_cb(GtkWidget *widget, gpointer data);
static gint historyPopupClickedInside_cb(GtkWidget *widget, gpointer data);
static void historySelectionMade_cb(GtkWidget *clist, gint row, gint column,
				    GdkEventButton *event, gpointer data);
static gchar* historyAutoComplete(GtkWidget *widget, GdkEventKey *event);


GtkWidget *entryCommand;
static int historyPosition = HISTORY_DEPTH;
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

/* no longer needed */
static void
historySelectionMade_cb(GtkWidget *clist, gint row, gint column,
			GdkEventButton *event, gpointer data)
{
    gchar *command;

    gtk_clist_get_text(GTK_CLIST(clist), row, column, &command);
    execCommand(command);

    /* close history window */
    gtk_widget_destroy(GTK_WIDGET(clist->parent->parent->parent));
}

static gint
historyItemClicked_cb(GtkWidget *widget, gpointer data)
{
    gchar *command;

    command = (gchar *) data;

    g_print("ITEM:%s\n", command);

    if (data != NULL)
	showMessage((gchar *) command); 
    else
	showMessage((gchar *) "[NULL]"); 

/*     command = (gchar *) data; */
/*     execCommand(command);  */

    /* go on */
    return (FALSE);
}

static gint
historyPopupClicked_cb(GtkWidget *widget, gpointer data)
{
    gdk_pointer_ungrab(GDK_CURRENT_TIME);
    gtk_grab_remove(GTK_WIDGET(widget));
    gtk_widget_destroy(GTK_WIDGET(widget));
    widget = NULL;
     
    /* go on */
    return (FALSE);
}

static gint
historyPopupClickedInside_cb(GtkWidget *widget, gpointer data)
{
    /* eat signal (prevent that popup will be destroyed) */
    return(TRUE);
}

gint 
showHistory_signal(GtkWidget *widget, gpointer data)
{
     GtkWidget *window;
     GtkWidget *frame;
     GtkWidget *scrolled_window;
     GtkWidget *clist;
     GtkStyle *style;
     GdkColor color;
     gchar *commandList[1];
     int i, j;

     /* count commands stored in history list */
     for(i = 0, j = 0; i < HISTORY_DEPTH; i++)
	 if(existsHistoryEntry(i))
	     j++;

     if(j == 0)
	 {
	     showMessage((gchar *) _("history list empty"));

	     /* don't show history popup window; go on */
	     return FALSE;  	     
	 }
     
     window = gtk_window_new(GTK_WINDOW_POPUP); 
     gtk_window_set_policy(GTK_WINDOW(window), 0, 0, 1);
     /* cb */
     gtk_signal_connect_after(GTK_OBJECT(window),
			      "button_press_event",
			      GTK_SIGNAL_FUNC(historyPopupClicked_cb),
			      NULL);
     /* position */
     gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
     /* size */
     gtk_widget_set_usize(GTK_WIDGET(window), 200, 350);
     /* title */
     gtk_window_set_title(GTK_WINDOW(window), (gchar *) _("Command history"));
     gtk_widget_show(window);

     /* frame */
     frame = gtk_frame_new(NULL);
     gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
     gtk_widget_show(frame);
     gtk_container_add(GTK_CONTAINER(window), frame);
     
     /* scrollbars */
     /* create scrolled window to put the GtkList widget inside */
     scrolled_window=gtk_scrolled_window_new(NULL, NULL);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
     gtk_signal_connect(GTK_OBJECT(scrolled_window),
			"button_press_event",
			GTK_SIGNAL_FUNC(historyPopupClickedInside_cb),
			NULL);
     gtk_container_add(GTK_CONTAINER(frame), scrolled_window);
     gtk_container_set_border_width (GTK_CONTAINER(scrolled_window), 2);
     gtk_widget_show(scrolled_window);
     

     /* the history list */
     /* style 
     style = malloc(sizeof(GtkStyle));
     style = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(applet)));

     style->fg[GTK_STATE_NORMAL].red = (gushort) prop.cmdLineColorFgR;
     style->fg[GTK_STATE_NORMAL].green = (gushort) prop.cmdLineColorFgG;
     style->fg[GTK_STATE_NORMAL].blue = (gushort) prop.cmdLineColorFgB;

     style->base[GTK_STATE_NORMAL].red = (gushort) prop.cmdLineColorBgR;
     style->base[GTK_STATE_NORMAL].green = (gushort) prop.cmdLineColorBgG;
     style->base[GTK_STATE_NORMAL].blue = (gushort) prop.cmdLineColorBgB;
     
     gtk_widget_push_style (style);
     */
     clist = gtk_clist_new(1);
     /*      gtk_widget_pop_style (); */
     gtk_signal_connect(GTK_OBJECT(clist),
			"select_row",
			GTK_SIGNAL_FUNC(historySelectionMade_cb),
			NULL);
     
     
     /* add history entries to list */
     for(i = 0; i < HISTORY_DEPTH; i++)
	 {
	     if(existsHistoryEntry(i))
		 {
		     commandList[0] = getHistoryEntry(i);
		     gtk_clist_append(GTK_CLIST(clist), commandList);
		 }
	 }
     gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
     gtk_widget_show(clist);    
     
     /* grab focus */
     gdk_pointer_grab (window->window,
		       TRUE,
		       GDK_BUTTON_PRESS_MASK
		       | GDK_BUTTON_RELEASE_MASK
		       | GDK_ENTER_NOTIFY_MASK
		       | GDK_LEAVE_NOTIFY_MASK 
		       | GDK_POINTER_MOTION_MASK,
		       NULL,
		       NULL,
		       GDK_CURRENT_TIME); 
     gtk_grab_add(window);
     
     
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

    gtk_window_set_position (GTK_WINDOW (fileSelect), GTK_WIN_POS_MOUSE);

    gtk_widget_show(fileSelect);

    /* go on */
    return FALSE;  
}

void
initCommandEntry(void)
{
    if(entryCommand)
    	gtk_widget_destroy(GTK_WIDGET(entryCommand));    
    
    /* create the widget we are going to put on the applet */
    entryCommand = gtk_entry_new_with_max_length((guint16) MAX_COMMAND_LENGTH); 
    
    /*	gtk_signal_connect(GTK_OBJECT(entryCommand), "activate",
	GTK_SIGNAL_FUNC(commandEntered_cb),
	entryCommand); */
    gtk_signal_connect(GTK_OBJECT(entryCommand), "key_press_event",
		       GTK_SIGNAL_FUNC(commandKey_event),
		       NULL);
    gtk_signal_connect(GTK_OBJECT(entryCommand), "focus_out_event",
		       GTK_SIGNAL_FUNC(commandFocusOut_event),
		       NULL);
    
    command_entry_update_color();
    command_entry_update_size();
}


void
command_entry_update_color(void)
{
    GtkStyle *style;
    GdkColor color;
    
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


void
command_entry_update_size(void)
{
    gtk_widget_set_usize(GTK_WIDGET(entryCommand), -1, prop.cmdLineY);
}


/* Thanks to Halfline <halfline@hawaii.rr.com> for his initial version
   of historyAutoComplete */
gchar *
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
