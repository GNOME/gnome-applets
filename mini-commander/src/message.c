/*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <oliver@maruhn.com>
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

#include "message.h"
#include "command_line.h"
#include "mini-commander_applet.h"
#include "preferences.h"

static gint hide_message(gpointer data);
static gint show_interesting_information(gpointer data);

void
init_message_label(MCData *mcdata)
{
#if 0
    PanelApplet *applet = mcdata->applet;
    gint timeout;
    label_message = gtk_label_new((gchar *) "");
    mcdata->label_timeout = gtk_timeout_add(15*1000, 
    					    (GtkFunction) show_interesting_information, 
    					    applet);
#endif
}

void show_message(gchar *message)
{
#if 0
    if(0/*!prop.flat_layout*/)
	{
	    gtk_label_set_text(GTK_LABEL(label_message), message);   
	    message_locked = TRUE;
	}
    else
	{
	    /* FIXME: cleanup needed */
	    GtkWidget *frame;
	    gint x, y, yy;
	    
	    if(message_window != NULL)
		gtk_widget_destroy(message_window);
	    
	    message_window = gtk_window_new(GTK_WINDOW_POPUP); 
	    /* gtk_window_set_policy(GTK_WINDOW(message_window), 0, 0, 1); */
	    gtk_widget_set_usize(GTK_WIDGET(message_window), 20 /*prop.normal_size_x*/, 20);
	    gdk_window_get_origin (applet->window, &x, &y);
	    y -= 20 + 6;
	    if(y < 0)
		{
		    /* panel is at the top of the screen */
		    gdk_window_get_size(applet->window, NULL, &yy);
		    y += (20 + 6) + yy + 6;
		}
	    gtk_widget_set_uposition(message_window, x, y);
	    gtk_widget_show(message_window);
	    
	    /* frame */
	    frame = gtk_frame_new(NULL);
	    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);
	    gtk_widget_show(frame);
	    gtk_container_add(GTK_CONTAINER(message_window), frame);	
	    /* label */
	    window_message_label = gtk_label_new((gchar *) message);
	    gtk_widget_show(window_message_label);
	    gtk_container_add(GTK_CONTAINER(frame), window_message_label);
	}

    gtk_timeout_add(2000, (GtkFunction) hide_message, (gpointer) message);
#endif
}

static gint
hide_message(gpointer data)
{
#if 0
    gchar *message = (char *) data;
    gchar *current_message;

    if(0/*!prop.flat_layout*/)
	{
	    gtk_label_get(GTK_LABEL(label_message), &current_message);
	    if(message && strcmp((char *) message, (char *) current_message) == 0)
		{
		    /* this is the message which has to be removed;
		       otherwise don't hide this message */
		    /* gtk_widget_hide (applet); */
		    gtk_label_set_text(GTK_LABEL(label_message), " "); 
		    /* gtk_widget_show (applet); */
		    message_locked = FALSE;
		}
	}

    if(message_window != NULL)
	{
	    gtk_label_get(GTK_LABEL(window_message_label), &current_message);
	    if(message && strcmp((char *) message, (char *) current_message) == 0)
		{
		    gtk_widget_destroy(message_window);
		    message_window = NULL;
		}
	}

    /* stop timeout function */
    return FALSE;
#endif
}

static gint
show_interesting_information(gpointer data)
{
#if 0
    PanelApplet *applet = data;
    properties *prop = g_object_get_data (G_OBJECT (applet), "prop");
    /* shows intersting information while there is no text
       in the mesage label
       currently only time is shown*/
    char message[21];
    char *time_format;
    gchar *current_message;
    time_t seconds = time((time_t *) 0);
    struct tm *tm;

    if(!prop->flat_layout && message_locked == FALSE)
	if(prop->show_time || prop->show_date)
	    {
		if(prop->show_time && prop->show_date)
		    time_format = _("%H:%M - %d. %b");
		else if(prop->show_time && !prop->show_date)
		    time_format = _("%H:%M");
		else if(!prop->show_time && prop->show_date)
		    time_format = _("%d. %b");
		else
		    time_format = "-";
		
		/* sprintf(message, "%s", ctime(&seconds)); */
		
		/* Reset stored timezone information.  If the
		   local timezone has been changed (for example
		   because the system is running on a laptop and
		   the user is traveling) the time display gets
		   updated to reflect the new time zone.  I wonder
		   if it is OK to call tzset() four times per
		   minute. */
		/* 		    strcpy(tzname, "\0\0"); */
		/* 		    tzname[0] = '\0'; */
		/* 		    tzname[1] = '\0'; */
		/* 		    unsetenv("TZ"); */
		/*  		    tzset();  */
		tm = localtime(&seconds);
		strftime(message, 20, time_format, tm);
		gtk_label_get(GTK_LABEL(label_message), &current_message);
		if(message && strcmp(message, current_message) != 0)
		    {
			gtk_label_set_text(GTK_LABEL(label_message), message); 
			/* refresh frame; otherwise it is covered by the label;
			   a bug in gtk? */
			/*			    gtk_widget_hide (frame);
			    			    gtk_widget_show (frame); */
		    }
	    }

    /* continue timeout function */
    return TRUE;
    data = NULL;
#endif
}

