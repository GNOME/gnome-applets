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
#include <string.h>
#include <gnome.h>
#include <panel-applet.h>
#include <gdk/gdkkeysyms.h>

#include "mini-commander_applet.h"
#include "command_line.h"
#include "preferences.h"
#include "exec.h"
#include "cmd_completion.h"
#include "history.h"
#include "message.h"

static gint file_browser_ok_signal(GtkWidget *widget, gpointer file_select);
/*static gint history_item_clicked_cb(GtkWidget *widget, gpointer data);*/
static gint history_popup_clicked_cb(GtkWidget *widget, gpointer data);
static gint history_popup_clicked_inside_cb(GtkWidget *widget, gpointer data);
static void history_selection_made_cb (GtkTreeView *treeview, GtkTreePath *arg1,
                                       GtkTreeViewColumn *arg2, gpointer data);
static gchar* history_auto_complete(GtkWidget *widget, GdkEventKey *event);


static int history_position = LENGTH_HISTORY_LIST;
static char browsed_filename[300] = "";

static GtkWidget *entry_command;


static gint
command_key_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    MCData *mcdata = data;
    PanelApplet *applet = mcdata->applet;
    properties *prop = g_object_get_data (G_OBJECT (applet), "prop");
    guint key = event->keyval;
    char *command;
    static char current_command[MAX_COMMAND_LENGTH];
    char buffer[MAX_COMMAND_LENGTH];
    int propagate_event = TRUE;

    /* printf("%d,%d,%d;  ", (gint16) event->keyval, event->state, event->length); */

    if((key == GDK_Tab
       || key == GDK_KP_Tab
       || key == GDK_ISO_Left_Tab)
       && event->state != GDK_SHIFT_MASK
       && event->state != GDK_CONTROL_MASK)
	{	
	    /* tab key pressed */
	    strcpy(buffer, (char *) gtk_entry_get_text(GTK_ENTRY(widget)));
	    cmd_completion(buffer, applet);
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
		    /* store current command line */
		    strcpy(current_command, (char *) gtk_entry_get_text(GTK_ENTRY(widget)));
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
	    strcpy(command, (char *) gtk_entry_get_text(GTK_ENTRY(widget)));
	    /* printf("%s\n", command); */
	    exec_command(command, applet);

	    append_history_entry(mcdata, (char *) command, FALSE);
	    history_position = LENGTH_HISTORY_LIST;		   
	    free(command);

	    strcpy(current_command, "");
	    gtk_entry_set_text(GTK_ENTRY(widget), (gchar *) "");
	    propagate_event = FALSE;
	}
    else if(prop->auto_complete_history && key >= GDK_space && key <= GDK_asciitilde )
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
			    gtk_editable_set_position(GTK_EDITABLE(widget), current_position + 1);
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
    data = NULL;
}

#if 0
static gint
command_line_focus_out_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    fprintf(stderr, "focus_out\n");
    /*
      gtk_widget_grab_focus(GTK_WIDGET(widget)); 
    */
    return (FALSE);
    widget = NULL;
    event = NULL;
    data = NULL;
}

static gint
command_line_focus_in_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    fprintf(stderr, "focus_in\n");
    /*
      gtk_widget_grab_focus(GTK_WIDGET(widget)); 
    */
    //    gtk_widget_grab_focus(GTK_WIDGET(widget));

    return (FALSE);
    widget = NULL;
    event = NULL;
    data = NULL;
}
#endif

static gint
command_line_activate_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    /* Sometimes the text entry does not get the keyboard focus
       although it should.  This workaround fixes this problem. */
#if 1
    gtk_widget_grab_focus(GTK_WIDGET(widget));
#endif
    /* go on */
    return (FALSE);
    widget = NULL;
    event = NULL;
    data = NULL;
}

static void
history_selection_made_cb (GtkTreeView *treeview, GtkTreePath *arg1,
                           GtkTreeViewColumn *arg2, gpointer data)
{
    PanelApplet *applet = data;
    gchar *command;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GValue val = {0,};
    GtkWidget *widget;

    widget = (GtkWidget *) treeview;
    model = gtk_tree_view_get_model (treeview);
    gtk_tree_model_get_iter ( model, &iter, arg1);
    gtk_tree_model_get_value (GTK_TREE_MODEL (model), &iter,
                              0, &val);
    command = g_strdup (g_value_get_string (&val));
    exec_command(command, applet);

    g_value_unset(&val);
    g_free(command);
    /* close history window */
    gtk_widget_destroy(GTK_WIDGET(widget->parent->parent->parent));
    return;
    data = NULL;
}

#if 0
static gint
history_item_clicked_cb(GtkWidget *widget, gpointer data)
{
    gchar *command;

    command = (gchar *) data;

    g_print("ITEM:%s\n", command);

    if (data != NULL)
	show_message((gchar *) command); 
    else
	show_message((gchar *) "[NULL]"); 

/*     command = (gchar *) data; */
/*     exec_command(command);  */

    /* go on */
    return (FALSE);
}
#endif

static gint
history_popup_clicked_cb(GtkWidget *widget, gpointer data)
{
    gdk_pointer_ungrab(GDK_CURRENT_TIME);
    gtk_grab_remove(GTK_WIDGET(widget));
    gtk_widget_destroy(GTK_WIDGET(widget));
    widget = NULL;
     
    /* go on */
    return (FALSE);
    data = NULL;
}

static gint
history_popup_clicked_inside_cb(GtkWidget *widget, gpointer data)
{
    /* eat signal (prevent that popup will be destroyed) */
    return(TRUE);
    widget = NULL;
    data = NULL;
}

gint 
show_history_signal(GtkWidget *widget, gpointer data)
{
     PanelApplet *applet = data;
     properties *prop = g_object_get_data (G_OBJECT (applet), "prop");
     GtkWidget *window;
     GtkWidget *frame;
     GtkWidget *scrolled_window;
     GtkListStore *store;
     GtkTreeIter iter;
     GtkTreeModel *model;
     GtkWidget    *treeview;
     GtkCellRenderer *cell_renderer;
     GtkTreeViewColumn *column;
     /*Gtk_style *style;*/
     gchar *command_list[1];
     int i, j;

     /* count commands stored in history list */
     for(i = 0, j = 0; i < LENGTH_HISTORY_LIST; i++)
	 if(exists_history_entry(i))
	     j++;

     if(j == 0)
	 {
	     show_message((gchar *) _("history list empty"));

	     /* don't show history popup window; go on */
	     return FALSE;  	     
	 }
     
     window = gtk_window_new(GTK_WINDOW_POPUP); 
     gtk_window_set_policy(GTK_WINDOW(window), 0, 0, 1);
     /* cb */
     gtk_signal_connect_after(GTK_OBJECT(window),
			      "button_press_event",
			      GTK_SIGNAL_FUNC(history_popup_clicked_cb),
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
     /* create scrolled window to put the Gtk_list widget inside */
     scrolled_window=gtk_scrolled_window_new(NULL, NULL);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
     gtk_signal_connect(GTK_OBJECT(scrolled_window),
			"button_press_event",
			GTK_SIGNAL_FUNC(history_popup_clicked_inside_cb),
			NULL);
     gtk_container_add(GTK_CONTAINER(frame), scrolled_window);
     gtk_container_set_border_width (GTK_CONTAINER(scrolled_window), 2);
     gtk_widget_show(scrolled_window);
     

     /* the history list */
     /* style 
     style = malloc(sizeof(Gtk_style));
     style = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(applet)));

     style->fg[GTK_STATE_NORMAL].red = (gushort) prop->cmd_line_color_fg_r;
     style->fg[GTK_STATE_NORMAL].green = (gushort) prop->cmd_line_color_fg_g;
     style->fg[GTK_STATE_NORMAL].blue = (gushort) prop->cmd_line_color_fg_b;

     style->base[GTK_STATE_NORMAL].red = (gushort) prop->cmd_line_color_bg_r;
     style->base[GTK_STATE_NORMAL].green = (gushort) prop->cmd_line_color_bg_g;
     style->base[GTK_STATE_NORMAL].blue = (gushort) prop->cmd_line_color_bg_b;
     
     gtk_widget_push_style (style);
     */
     /*      gtk_widget_pop_style (); */
     
     store = gtk_list_store_new (1, G_TYPE_STRING);

     /* add history entries to list */
     for(i = 0; i < LENGTH_HISTORY_LIST; i++)
	 {
	     if(exists_history_entry(i))
		 {
		     command_list[0] = get_history_entry(i);
                     gtk_list_store_append (store, &iter);
                     gtk_list_store_set (store, &iter,0,command_list[0],-1);
		 }
	 }
     
     model = GTK_TREE_MODEL(store);
     treeview = gtk_tree_view_new_with_model (model);
     cell_renderer = gtk_cell_renderer_text_new ();
     column = gtk_tree_view_column_new_with_attributes (NULL, cell_renderer,
                                                       "text", 0, NULL);
     gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
     gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
     gtk_tree_selection_set_mode( (GtkTreeSelection *)gtk_tree_view_get_selection
                                (GTK_TREE_VIEW (treeview)),
                                 GTK_SELECTION_SINGLE);
     gtk_signal_connect(GTK_OBJECT(treeview),
                        "row_activated",
                        GTK_SIGNAL_FUNC(history_selection_made_cb),
                        applet);
     g_object_unref (G_OBJECT (model));
     gtk_container_add(GTK_CONTAINER(scrolled_window),treeview);
     gtk_widget_show (treeview); 
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
     widget = NULL;
     data = NULL;
}

static gint 
file_browser_ok_signal(GtkWidget *widget, gpointer file_select)
{
    GtkWidget *fs = file_select;
    PanelApplet *applet = g_object_get_data (G_OBJECT (fs), "applet");
    /* get selected file name */
    strcpy(browsed_filename, (char *) gtk_file_selection_get_filename(GTK_FILE_SELECTION(file_select)));

    /* destroy file select dialog */
    gtk_widget_destroy(GTK_WIDGET(file_select));
    
    /* printf("Filename: %s\n", (char *)  browsed_filename); */

    /* execute command */
    exec_command(browsed_filename, applet);

    /* go on */
    return FALSE;  
    widget = NULL;
}

gint 
show_file_browser_signal(GtkWidget *widget, gpointer data)
{
    /* FIXME: write this routine */
    PanelApplet *applet = data;
    GtkWidget *file_select;

    /* build file select dialog */
    file_select = gtk_file_selection_new((gchar *) _("Start program"));
    g_object_set_data (G_OBJECT (file_select), "applet", applet);
    gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(file_select)->ok_button),
		       "clicked",
		       GTK_SIGNAL_FUNC(file_browser_ok_signal),
		       GTK_OBJECT(file_select));
    gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(file_select)->cancel_button),
			      "clicked",
			      GTK_SIGNAL_FUNC(gtk_widget_destroy),
			      GTK_OBJECT (file_select));

    /* set path to last selected path */
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_select),
				    (gchar *) browsed_filename);

    /* Set as modal */
    gtk_window_set_modal(GTK_WINDOW(file_select),TRUE);

    gtk_window_set_position (GTK_WINDOW (file_select), GTK_WIN_POS_MOUSE);

    gtk_widget_show(file_select);

    /* go on */
    return FALSE;
    widget = NULL;
    data = NULL;
}

GtkWidget *
init_command_entry(MCData *mcdata)
{
    PanelApplet *applet = mcdata->applet;
    properties *prop = mcdata->prop;
    
    if(entry_command)
    	gtk_widget_destroy(GTK_WIDGET(entry_command));    
    
    /* create the widget we are going to put on the applet */
    entry_command = gtk_entry_new_with_max_length((guint16) MAX_COMMAND_LENGTH); 
    set_atk_name_description (entry_command, _("Command line"), 
        _("Type a command here and Gnome will execute it for you"));
        
    /* in case we get destroyed elsewhere */
    gtk_signal_connect(GTK_OBJECT(entry_command),"destroy",
		       GTK_SIGNAL_FUNC(gtk_widget_destroyed),
		       &entry_command);
    
    gtk_signal_connect(GTK_OBJECT(entry_command), "key_press_event",
		       GTK_SIGNAL_FUNC(command_key_event),
		       mcdata);

#if 0
    gtk_signal_connect(GTK_OBJECT(entry_command), "focus_out_event",
		       GTK_SIGNAL_FUNC(command_line_focus_out_cb),
		       NULL);
    gtk_signal_connect(GTK_OBJECT(entry_command), "focus_in_event",
		       GTK_SIGNAL_FUNC(command_line_focus_in_cb),
		       NULL);
#endif
#if 1
    gtk_signal_connect(GTK_OBJECT(entry_command), "button_press_event",
		       GTK_SIGNAL_FUNC(command_line_activate_cb),
		       mcdata);
#endif   
    if (prop->show_default_theme != TRUE)
        command_entry_update_color(entry_command, prop); 
    command_entry_update_size(entry_command, prop);
    
    return entry_command;
}


void
command_entry_update_color(GtkWidget *entry_command, properties *prop)
{
    GdkColor fg, bg;

    fg.red = prop->cmd_line_color_fg_r;
    fg.green = prop->cmd_line_color_fg_g;
    fg.blue = prop->cmd_line_color_fg_b;
    bg.red = prop->cmd_line_color_bg_r;
    bg.green = prop->cmd_line_color_bg_g;
    bg.blue = prop->cmd_line_color_bg_b;
    gtk_widget_modify_text (entry_command, GTK_STATE_NORMAL, &fg);
    gtk_widget_modify_text (entry_command, GTK_STATE_PRELIGHT, &fg);
    gtk_widget_modify_base (entry_command, GTK_STATE_NORMAL, &bg);
    gtk_widget_modify_base (entry_command, GTK_STATE_PRELIGHT, &bg);
   
}


void
command_entry_update_size(GtkWidget *entry_command,properties *prop)
{
    int size_y = -1;

    if(prop->flat_layout)  {
	if(prop->show_handle && !prop->show_frame)
	    size_y = prop->normal_size_x - 17 - 10;
	else if(!prop->show_handle && !prop->show_frame)
	    size_y = prop->normal_size_x - 17;
	if(prop->show_handle && prop->show_frame)
	    size_y = prop->normal_size_x - 17 - 10 - 10;
	else if(!prop->show_handle && prop->show_frame)
	    size_y = prop->normal_size_x - 17 - 10;
    }
    gtk_widget_set_usize(GTK_WIDGET(entry_command), size_y, prop->cmd_line_size_y);
}


/* Thanks to Halfline <halfline@hawaii.rr.com> for his initial version
   of history_auto_complete */
gchar *
history_auto_complete(GtkWidget *widget, GdkEventKey *event)
{
    gchar current_command[MAX_COMMAND_LENGTH];
    gchar* completed_command;
    int i;

    g_snprintf(current_command, sizeof(current_command), "%s%s", 
	       gtk_entry_get_text(GTK_ENTRY(widget)), event->string); 
    for(i = LENGTH_HISTORY_LIST - 1; i >= 0; i--) 
  	{
	    if(!exists_history_entry(i))
		break;
  	    completed_command = get_history_entry(i); 
  	    if(!strncmp(completed_command, current_command, strlen(current_command))) 
		return completed_command; 
  	} 
    
    return NULL;
}

