/*
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "command-line.h"

#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "preferences.h"
#include "exec.h"
#include "cmd-completion.h"
#include "history.h"

static gint file_browser_response_signal(GtkWidget *widget, gint response, gpointer mc_data);
static gint history_popup_clicked_cb(GtkWidget *widget, gpointer data);
static gint history_popup_clicked_inside_cb(GtkWidget *widget, gpointer data);
static gchar* history_auto_complete(GtkWidget *widget, GdkEventKey *event);

static int history_position = MC_HISTORY_LIST_LENGTH;
static gchar *browsed_folder = NULL;

static GtkWidget*
mc_gtk_wdget_get_great_grandparent(GtkWidget* widget)
{
    GtkWidget *parent = NULL;
    int i = 0;

    /* great-grandparent is the parent's parent's parent */
    while (widget && i < 3) {
        parent = gtk_widget_get_parent (widget);
        widget = parent;
        ++i;
    }

    return parent;
}

static gboolean
button_press_cb (GtkEntry       *entry,
		 GdkEventButton *event,
		 MCData         *mc)
{
    const gchar *str;

    gp_applet_request_focus (GP_APPLET (mc), event->time);

    if (mc->error) {
	   mc->error = FALSE;
	   str = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	   gtk_entry_set_text (entry, (gchar *) str + 3);
	}
    return FALSE;
}

static gboolean
command_key_event (GtkEntry   *entry,
		   GdkEventKey *event,
		   MCData      *mc)
{
    guint key = event->keyval;
    char *command;
    static char current_command[MC_MAX_COMMAND_LENGTH];
    char buffer[MC_MAX_COMMAND_LENGTH];
    gboolean propagate_event = TRUE;
    const gchar *str;

    if (mc->error) {
	   mc->error = FALSE;
	   str = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	   gtk_entry_set_text (entry, (gchar *) str + 3);
	   gtk_editable_set_position (GTK_EDITABLE (entry), strlen (str));
	}
    if(key == GDK_KEY_Tab
       || key == GDK_KEY_KP_Tab
       || key == GDK_KEY_ISO_Left_Tab)
        {
            if(event->state == GDK_CONTROL_MASK)
                {
                    /*
                     * Move focus to the next widget (browser) button.
                     */
                    gtk_widget_child_focus (GTK_WIDGET (mc), GTK_DIR_TAB_FORWARD);
	            propagate_event = FALSE;
                }
            else if(event->state != GDK_SHIFT_MASK)
	        {
	                    /* tab key pressed */
	            strcpy(buffer, (char *) gtk_entry_get_text(GTK_ENTRY(entry)));
	            mc_cmd_completion (mc, buffer);
	            gtk_entry_set_text(GTK_ENTRY(entry), (gchar *) buffer);

	            propagate_event = FALSE;
	        }
	}
    else if(key == GDK_KEY_Up
	    || key == GDK_KEY_KP_Up
	    || key == GDK_KEY_ISO_Move_Line_Up
	    || key == GDK_KEY_Pointer_Up)
	{
	    /* up key pressed */
	    if(history_position == MC_HISTORY_LIST_LENGTH)
		{
		    /* store current command line */
		    strcpy(current_command, (char *) gtk_entry_get_text(entry));
		}
	    if(history_position > 0 && exists_history_entry(history_position - 1))
		{
		    gtk_entry_set_text(entry, (gchar *) get_history_entry(--history_position));
		}

	    propagate_event = FALSE;
	}
    else if(key == GDK_KEY_Down
	    || key == GDK_KEY_KP_Down
	    || key == GDK_KEY_ISO_Move_Line_Down
	    || key == GDK_KEY_Pointer_Down)
	{
	    /* down key pressed */
	    if(history_position <  MC_HISTORY_LIST_LENGTH - 1)
		{
		    gtk_entry_set_text(entry, (gchar *) get_history_entry(++history_position));
		}
	    else if(history_position == MC_HISTORY_LIST_LENGTH - 1)
		{
		    gtk_entry_set_text(entry, (gchar *) current_command);
		    ++history_position;
		}

	    propagate_event = FALSE;
	}
    else if(key == GDK_KEY_Return
	    || key == GDK_KEY_KP_Enter
	    || key == GDK_KEY_ISO_Enter
	    || key == GDK_KEY_3270_Enter)
	{
	    /* enter pressed -> exec command */
	    command = (char *) malloc(sizeof(char) * MC_MAX_COMMAND_LENGTH);
	    strcpy(command, (char *) gtk_entry_get_text(entry));
	    mc_exec_command(mc, command);

	    history_position = MC_HISTORY_LIST_LENGTH;
	    free(command);

	    strcpy(current_command, "");
	    propagate_event = FALSE;
	}
    else if (mc->preferences.auto_complete_history && key >= GDK_KEY_space && key <= GDK_KEY_asciitilde )
	{
            char *completed_command;
	    gint current_position = gtk_editable_get_position(GTK_EDITABLE(entry));

	    if(current_position != 0)
		{
		    gtk_editable_delete_text( GTK_EDITABLE(entry), current_position, -1 );
		    completed_command = history_auto_complete(GTK_WIDGET (entry), event);

		    if(completed_command != NULL)
			{
			    gtk_entry_set_text(entry, completed_command);
			    gtk_editable_set_position(GTK_EDITABLE(entry), current_position + 1);
			    propagate_event = FALSE;
			}
		}
	}

    return !propagate_event;
}

static gint
history_popup_clicked_cb(GtkWidget *widget, gpointer data)
{
    gdk_pointer_ungrab(GDK_CURRENT_TIME);
    gdk_keyboard_ungrab(GDK_CURRENT_TIME);
    gtk_grab_remove(GTK_WIDGET(widget));
    gtk_widget_destroy(GTK_WIDGET(widget));
    widget = NULL;

    /* go on */
    return (FALSE);
}

static gint
history_popup_clicked_inside_cb(GtkWidget *widget, gpointer data)
{
    /* eat signal (prevent that popup will be destroyed) */
    return(TRUE);
}

static gboolean
history_key_press_cb (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    if (event->keyval == GDK_KEY_Escape) {
        gdk_pointer_ungrab(GDK_CURRENT_TIME);
        gdk_keyboard_ungrab(GDK_CURRENT_TIME);
        gtk_grab_remove(GTK_WIDGET(widget));
        gtk_widget_destroy(GTK_WIDGET(widget));
        widget = NULL;
    }

    return FALSE;

}

static gboolean
history_list_key_press_cb (GtkWidget   *widget,
			   GdkEventKey *event,
			   MCData      *mc)
{
    GtkTreeView *tree = g_object_get_data (G_OBJECT (mc), "tree");
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *command;

    switch (event->keyval) {
    case GDK_KEY_KP_Enter:
    case GDK_KEY_ISO_Enter:
    case GDK_KEY_3270_Enter:
    case GDK_KEY_Return:
    case GDK_KEY_space:
    case GDK_KEY_KP_Space:
        if ((event->state & GDK_CONTROL_MASK) &&
            (event->keyval == GDK_KEY_space || event->keyval == GDK_KEY_KP_Space))
             break;

        if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection (tree),
        				      &model,
        				      &iter))
            return FALSE;
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                            0, &command, -1);
        mc_exec_command (mc, command);
        g_free (command);
        gtk_widget_destroy (mc_gtk_wdget_get_great_grandparent (widget));

	return TRUE;

    default:
        break;
    }

    return FALSE;
}

static gboolean
history_list_button_press_cb (GtkWidget      *widget,
			      GdkEventButton *event,
			      MCData         *mc)
{
    GtkTreeView *tree = g_object_get_data (G_OBJECT (mc), "tree");
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *command;

    if (event->type == GDK_2BUTTON_PRESS) {
    	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection (tree),
        				      &model,
        				      &iter))
            return FALSE;
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                            0, &command, -1);
        mc_exec_command(mc, command);
        g_free (command);
        gtk_widget_destroy (mc_gtk_wdget_get_great_grandparent (widget));

	return TRUE;
    }

    return FALSE;
}

int
mc_show_history (GtkWidget *widget,
		 MCData    *mc)
{
     GtkWidget *window;
     GtkWidget *frame;
     GtkWidget *scrolled_window;
     GdkWindow *gdk_window;
     GtkListStore *store;
     GtkTreeIter iter;
     GtkTreeModel *model;
     GtkWidget    *treeview;
     GtkCellRenderer *cell_renderer;
     GtkTreeViewColumn *column;
     GtkRequisition  req;
     gchar *command_list[1];
     int i, j;
     gint x, y, width, height, screen_width, screen_height;

     /* count commands stored in history list */
     for(i = 0, j = 0; i < MC_HISTORY_LIST_LENGTH; i++)
	 if(exists_history_entry(i))
	     j++;

     window = gtk_window_new(GTK_WINDOW_POPUP);
     gtk_window_set_screen (GTK_WINDOW (window),
			    gtk_widget_get_screen (GTK_WIDGET (mc)));
     gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
     gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_COMBO);
     /* cb */
     g_signal_connect_after(G_OBJECT(window),
			      "button_press_event",
			      G_CALLBACK(history_popup_clicked_cb),
			      NULL);
     g_signal_connect_after (G_OBJECT (window), "key_press_event",
     		       G_CALLBACK (history_key_press_cb), NULL);

     /* size */
     gtk_widget_set_size_request(GTK_WIDGET(window), 200, 350);


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
     g_signal_connect(G_OBJECT(scrolled_window),
			"button_press_event",
			G_CALLBACK(history_popup_clicked_inside_cb),
			NULL);
     gtk_container_add(GTK_CONTAINER(frame), scrolled_window);
     gtk_container_set_border_width (GTK_CONTAINER(scrolled_window), 2);
     gtk_widget_show(scrolled_window);

     store = gtk_list_store_new (1, G_TYPE_STRING);

     /* add history entries to list */
     if (j == 0) {
          gtk_list_store_append (store, &iter);
          gtk_list_store_set (store, &iter,0, _("No items in history"), -1);
     }
     else {
          for(i = 0; i < MC_HISTORY_LIST_LENGTH; i++)
	      {
     	     if(exists_history_entry(i))
	     	 {
     		      command_list[0] = get_history_entry(i);
                      gtk_list_store_prepend (store, &iter);
                      gtk_list_store_set (store, &iter,0,command_list[0],-1);
		 }
	      }
     }
     model = GTK_TREE_MODEL(store);
     treeview = gtk_tree_view_new_with_model (model);
     g_object_set_data (G_OBJECT (mc), "tree", treeview);
     cell_renderer = gtk_cell_renderer_text_new ();
     column = gtk_tree_view_column_new_with_attributes (NULL, cell_renderer,
                                                       "text", 0, NULL);
     gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
     gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
     if (j == 0) {
          gtk_tree_selection_set_mode( (GtkTreeSelection *)gtk_tree_view_get_selection
                                (GTK_TREE_VIEW (treeview)),
                                 GTK_SELECTION_NONE);
     }
     else {
          gtk_tree_selection_set_mode( (GtkTreeSelection *)gtk_tree_view_get_selection
                                (GTK_TREE_VIEW (treeview)),
                                 GTK_SELECTION_SINGLE);
          g_signal_connect (G_OBJECT (treeview), "button_press_event",
     		       G_CALLBACK (history_list_button_press_cb), mc);
          g_signal_connect (G_OBJECT (treeview), "key_press_event",
     		       G_CALLBACK (history_list_key_press_cb), mc);
     }

     g_object_unref (G_OBJECT (model));
     gtk_container_add(GTK_CONTAINER(scrolled_window),treeview);
     gtk_widget_show (treeview);

     gtk_widget_get_preferred_size (window, NULL, &req);
     gdk_window = gtk_widget_get_window (GTK_WIDGET (mc));
     gdk_window_get_origin (gdk_window, &x, &y);
     gdk_window_get_geometry (gdk_window, NULL, NULL,
                              &width, &height);

     switch (gp_applet_get_position (GP_APPLET (mc))) {
     case GTK_POS_TOP:
        y += height;
     	break;
     case GTK_POS_BOTTOM:
        y -= req.height;
     	break;
     case GTK_POS_RIGHT:
     	x -= req.width;
	break;
     case GTK_POS_LEFT:
     	x += width;
	break;

     default:
        g_assert_not_reached ();
        break;
     }

     screen_width = gdk_screen_width ();
     screen_height = gdk_screen_height ();
     x = CLAMP (x - 2, 0, MAX (0, screen_width - req.width));
     y = CLAMP (y - 2, 0, MAX (0, screen_height - req.height));
     gtk_window_move (GTK_WINDOW (window), x, y);
     gtk_widget_show(window);

     /* grab focus */
     gdk_window = gtk_widget_get_window (window);
     gdk_pointer_grab (gdk_window,
		       TRUE,
		       GDK_BUTTON_PRESS_MASK
		       | GDK_BUTTON_RELEASE_MASK
		       | GDK_ENTER_NOTIFY_MASK
		       | GDK_LEAVE_NOTIFY_MASK
		       | GDK_POINTER_MOTION_MASK,
		       NULL,
		       NULL,
		       GDK_CURRENT_TIME);
     gdk_keyboard_grab (gdk_window, TRUE, GDK_CURRENT_TIME);
     gtk_grab_add(window);
     gtk_widget_grab_focus (treeview);

     return FALSE;
}

static gint
file_browser_response_signal(GtkWidget *widget, gint response, gpointer mc_data)
{
    MCData *mc = mc_data;
    gchar *filename;

    if (response == GTK_RESPONSE_OK) {

        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(mc->file_select));

        if (filename != NULL) {
            if (browsed_folder)
                g_free (browsed_folder);

            browsed_folder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(mc->file_select));

            mc_exec_command(mc, filename);
            g_free(filename);
       }
    }

    /* destroy file select dialog */
    gtk_widget_destroy (mc->file_select);
    mc->file_select = NULL;

     /* go on */
    return FALSE;
}

int
mc_show_file_browser (GtkWidget *widget,
		      MCData    *mc)
{
    if (mc->file_select && gtk_widget_get_visible (mc->file_select)) {
        gtk_window_present (GTK_WINDOW (mc->file_select));
        return TRUE;
    }

    /* build file select dialog */
    mc->file_select = gtk_file_chooser_dialog_new((gchar *) _("Start program"),
						  NULL,
						  GTK_FILE_CHOOSER_ACTION_OPEN,
						  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						  GTK_STOCK_EXECUTE, GTK_RESPONSE_OK,
						  NULL);

    g_signal_connect(G_OBJECT(mc->file_select),
		     "response",
		     G_CALLBACK(file_browser_response_signal),
		     mc);

    /* set path to last selected path */
    if (browsed_folder)
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(mc->file_select),
			          (gchar *) browsed_folder);

    /* Set as modal */
    gtk_window_set_modal(GTK_WINDOW(mc->file_select),TRUE);

    gtk_window_set_screen (GTK_WINDOW (mc->file_select),
			   gtk_widget_get_screen (GTK_WIDGET (mc)));
    gtk_window_set_position (GTK_WINDOW (mc->file_select), GTK_WIN_POS_CENTER);

    gtk_widget_show(mc->file_select);

    return FALSE;
}

void
mc_create_command_entry (MCData *mc)
{
    mc->entry = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (mc->entry), MC_MAX_COMMAND_LENGTH);

    gtk_style_context_add_provider (gtk_widget_get_style_context (mc->entry),
				    GTK_STYLE_PROVIDER (mc->provider),
				    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_signal_connect (mc->entry, "key_press_event",
		      G_CALLBACK (command_key_event), mc);

    g_signal_connect (mc->entry, "button_press_event",
		      G_CALLBACK (button_press_cb), mc);

    if (!mc->preferences.show_default_theme)
    {
	    gtk_widget_set_name (mc->entry, "minicommander-applet-entry");
	    mc_command_update_entry_color (mc);
    }
    else
	    gtk_widget_set_name (mc->entry,
			    "minicommander-applet-entry-default");

    mc_command_update_entry_size (mc);

    mc_set_atk_name_description (mc->entry,
                                 _("Command line"),
                                 _("Type a command here and Gnome will execute it for you"));
}

void
mc_command_update_entry_color (MCData *mc)
{
    GdkRGBA fg_color;
    GdkRGBA bg_color;
    gchar *css_string;

    gdk_rgba_parse (&fg_color, mc->preferences.cmd_line_color_fg);
    gdk_rgba_parse (&bg_color, mc->preferences.cmd_line_color_bg);

    css_string = g_strdup_printf ("#minicommander-applet-entry {\n"
                                  "\tcolor: %s;\n"
                                  "\tcaret-color: %s;\n"
                                  "\tbackground-color: %s;\n"
                                  "}\n",
                                  gdk_rgba_to_string (&fg_color),
                                  gdk_rgba_to_string (&fg_color),
                                  gdk_rgba_to_string (&bg_color));

    gtk_css_provider_load_from_data (mc->provider, css_string, -1, NULL);
    g_free (css_string);
}

void
mc_command_update_entry_size (MCData *mc)
{
    int size_x = -1;

    size_x = mc->preferences.normal_size_x - 17;
    if (mc->orient == GTK_ORIENTATION_VERTICAL) {
      size_x = MIN(size_x, mc->preferences.panel_size_x - 17);
      gtk_widget_set_size_request (GTK_WIDGET (mc->entry), size_x, -1);
    } else {
      gtk_widget_set_size_request (GTK_WIDGET (mc->entry), size_x, mc->preferences.normal_size_y+2);
    }
}


/* Thanks to Halfline <halfline@hawaii.rr.com> for his initial version
   of history_auto_complete */
gchar *
history_auto_complete(GtkWidget *widget, GdkEventKey *event)
{
    gchar current_command[MC_MAX_COMMAND_LENGTH];
    gchar* completed_command;
    int i;

    g_snprintf(current_command, sizeof(current_command), "%s%s",
	       gtk_entry_get_text(GTK_ENTRY(widget)), event->string);
    for(i = MC_HISTORY_LIST_LENGTH - 1; i >= 0; i--)
  	{
	    if(!exists_history_entry(i))
		break;
  	    completed_command = get_history_entry(i);
  	    if(!strncmp(completed_command, current_command, strlen(current_command)))
		return completed_command;
  	}

    return NULL;
}
