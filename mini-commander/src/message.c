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

#include <string.h>
#include <config.h>
#include <gnome.h>
#include <applet-widget.h>

#include "message.h"
#include "mini-commander_applet.h"
#include "preferences.h"

GtkWidget *labelMessage;
static int messageLocked = FALSE;

static gint hideMessage(gpointer data);
static gint showInterestingInformation(gpointer data);


void
initMessageLabel(void)
{
    labelMessage = gtk_label_new((gchar *) "");
    gtk_timeout_add(15*1000, (GtkFunction) showInterestingInformation, (gpointer) NULL);
}

void showMessage(gchar *message)
{
    messageLocked = TRUE;
    /* I don't know why, but if I don't call gtp_widget_hide then the
       label update doesn't work the way it should */

    /* gtk_widget_hide (applet); */
    gtk_label_set_text(GTK_LABEL(labelMessage), message);
    /* refresh frame; otherwise it is covered by the label;
       a bug in gtk? */
    /*    gtk_widget_hide (frame);
	  gtk_widget_show (frame); */

    /* gtk_widget_show (applet); */
    
    gtk_timeout_add(2000, (GtkFunction) hideMessage, (gpointer) message);
}

static gint
hideMessage(gpointer data)
{
    gchar *message = (char *) data;
    gchar *currentMessage;

    gtk_label_get(GTK_LABEL(labelMessage), &currentMessage);
    if(strcmp((char *) message, (char *) currentMessage) == 0)
	{
	    /* this is the message which has to be removed;
	       otherwise don't hide this message */
	    /* gtk_widget_hide (applet); */
	    gtk_label_set_text(GTK_LABEL(labelMessage), " "); 
	    /* gtk_widget_show (applet); */
	    messageLocked = FALSE;
	}

    /* stop timeout function */
    return FALSE;
}

static gint
showInterestingInformation(gpointer data)
{
    /* shows intersting information while there is no text
       in the mesage label
       currently only time is shown*/
    char message[21];
    char *timeFormat;
    gchar *currentMessage;

    time_t seconds = time((time_t *) 0);
    struct tm *tm;

    if (messageLocked == FALSE)
	{
	    if(prop.showTime || prop.showDate)
		{
		    if(prop.showTime && prop.showDate)
			timeFormat = _("%H:%M - %d. %b");
		    else if(prop.showTime && !prop.showDate)
/* 			timeFormat = _("%H:%M %Z"); */
			timeFormat = _("%H:%M");
		    else if(!prop.showTime && prop.showDate)
			timeFormat = _("%d. %b");
		    else
			timeFormat = "-";
		    
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
		    strftime(message, 20, timeFormat, tm);
		    gtk_label_get(GTK_LABEL(labelMessage), &currentMessage);
		    if(strcmp(message, currentMessage) != 0)
			{
			    gtk_label_set_text(GTK_LABEL(labelMessage), message); 
			    /* refresh frame; otherwise it is covered by the label;
			       a bug in gtk? */
			    /*			    gtk_widget_hide (frame);
			    			    gtk_widget_show (frame); */
			}
		}
	}

    /* continue timeout function */
    return TRUE;
    data = NULL;
}
