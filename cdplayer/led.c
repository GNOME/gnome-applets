/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "cdplayer.h"
#include "led.h"

GtkRequisition track_size, time_size;
gboolean check_for_theme = FALSE, large_font = FALSE;

static void led_draw_track(GtkWidget *track, int trackno);

/* FIXME: The functions names has to be changed to something other than led. As we don't use led.xpm file anymore */
void
led_create_widgets(GtkWidget **time, GtkWidget **track, gpointer data)
{
    GtkWidget *w;

	track_size.width = track_size.height = time_size.width = time_size.height = -1;

    *time = gtk_label_new("--:--");
	GTK_WIDGET_UNSET_FLAGS(*time, GTK_CAN_DEFAULT);

	gtk_widget_set_size_request(*time, LED_WIDTH, -1); 
	gtk_label_set_justify(GTK_LABEL (*time), GTK_JUSTIFY_CENTER);

    gtk_widget_show(*time);
    gtk_widget_ref(*time);


    *track = gtk_label_new("");
    GTK_WIDGET_UNSET_FLAGS(*track, GTK_CAN_DEFAULT);

    gtk_widget_set_size_request(*track, LED_WIDTH - 25, -1);  
    gtk_label_set_justify(GTK_LABEL (*track), GTK_JUSTIFY_CENTER);

    gtk_widget_show(*track);
    gtk_widget_ref(*track); 

}

/* FIXME: this function is same as led_nodisc(), one of them can be removed */
void
led_stop(GtkWidget *time, GtkWidget *track)
{
	if(!GTK_WIDGET_REALIZED(time)) return;

    gtk_label_set_text(GTK_LABEL(time), "--:--");
    gtk_label_set_text(GTK_LABEL(track), "");

	if (large_font) {
		gtk_widget_set_size_request(time, time_size.width, time_size.height);
		gtk_widget_set_size_request(track, track_size.width, track_size.height);
	}

}

static void
led_draw_track(GtkWidget *track, int trackno)
{
    gchar *track_number;
	
	if(!GTK_WIDGET_REALIZED(track)) return;

    track_number = g_strdup_printf("%d%d",trackno / 10, trackno % 10);
    gtk_label_set_text(GTK_LABEL(track), track_number);

	if ((!check_for_theme) && (large_font)) {
	/* FIXME: this is a hack to get the size for bigger fonts */
		gtk_widget_set_size_request(track, -1, -1);
		gtk_widget_size_request(track, &track_size);
		check_for_theme = TRUE;
	}

	if (large_font) 
		gtk_widget_set_size_request(track, track_size.width, track_size.height);

    g_free(track_number);
}

void
led_time(GtkWidget * time, int min, int sec, GtkWidget * track, int trackno)
{
    gchar *display_time;

	if(!GTK_WIDGET_REALIZED(time)) return;

    display_time = g_strdup_printf("%d%d : %d%d", min / 10, min % 10, sec / 10, sec % 10);
    gtk_label_set_text(GTK_LABEL(time), display_time);

	/* check for large font only once */
	if (!check_for_theme) {
		gtk_widget_set_size_request(time, -1, -1);
		gtk_widget_size_request(time, &time_size);
		if (time_size.width > LED_WIDTH) 
			large_font = TRUE;
		else
			gtk_widget_set_size_request(time, LED_WIDTH, -1);
	}

	if (large_font) 
		gtk_widget_set_size_request(time, time_size.width, time_size.height);

    led_draw_track(track, trackno);

    g_free(display_time);
}

void
led_paused(GtkWidget *time, int min, int sec, GtkWidget *track, int trackno)
{
	static int visible = 1;

	if(!GTK_WIDGET_REALIZED(time)) return;

	if (visible == 1) {
		led_time(time, min, sec, track, trackno);
		visible = 0;
	} else {
		if(!GTK_WIDGET_REALIZED(time)) return;
        gtk_label_set_text(GTK_LABEL(time),"");
		if (large_font)
			gtk_widget_set_size_request(time, time_size.width, time_size.height);
		visible = 1;
	}
}

void
led_nodisc(GtkWidget * time, GtkWidget * track)
{
	if(!GTK_WIDGET_REALIZED(time)) return;

    gtk_label_set_text(GTK_LABEL(time), "--:--");
    gtk_label_set_text(GTK_LABEL(track), "");
	
	if (large_font) {
		gtk_widget_set_size_request(time, time_size.width, time_size.height);
		gtk_widget_set_size_request(track, track_size.width, track_size.height);
	}

}
