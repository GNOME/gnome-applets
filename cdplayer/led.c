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

#include "images/led.xpm"

static void led_draw_track(GtkWidget *track, int trackno);

static GdkPixbuf *led_pixbuf = NULL;

void
led_init()
{
	if(!led_pixbuf) led_pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)led_xpm);
	g_object_ref(G_OBJECT(led_pixbuf));
}

void
led_done()
{
	g_object_unref(G_OBJECT(led_pixbuf));
}

void
led_create_widgets(GtkWidget **time, GtkWidget **track, gpointer data)
{
	GdkPixbuf *pixbuf;

    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, LED_WIDTH, LED_HEIGHT + 2);
    gdk_pixbuf_fill(pixbuf, 0);
	*time = gtk_image_new_from_pixbuf(pixbuf);
	gtk_widget_set_size_request(*time, LED_WIDTH, LED_HEIGHT + 2);
	g_object_ref(G_OBJECT(*time));
	gtk_widget_show(*time);
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, DIGIT_WIDTH * 2 + 2, LED_HEIGHT + 2);
    gdk_pixbuf_fill(pixbuf, 0);
	*track = gtk_image_new_from_pixbuf(pixbuf);
	gtk_widget_set_size_request(*track, DIGIT_WIDTH * 2 + 2, LED_HEIGHT + 2);
	g_object_ref(G_OBJECT(*track));
	gtk_widget_show(*track);
}

void
led_stop(GtkWidget *time, GtkWidget *track)
{
	GdkPixbuf *pixbuf;

	if(!GTK_WIDGET_REALIZED(time)) return;

	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(time));
    gdk_pixbuf_fill(pixbuf, 0);

    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   10 * DIGIT_WIDTH + 7, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   1, 1);
    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   10 * DIGIT_WIDTH + 7, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   DIGIT_WIDTH + 1, 1);

    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   10 * DIGIT_WIDTH, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   2 * DIGIT_WIDTH + 2, 1);

    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   10 * DIGIT_WIDTH + 7, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   3 * DIGIT_WIDTH - 1, 1);

    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   10 * DIGIT_WIDTH + 7, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   4 * DIGIT_WIDTH - 1, 1);
    gtk_widget_queue_draw(time);
	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(track));
    gdk_pixbuf_fill(pixbuf, 0);
    gtk_widget_queue_draw(track);
}

static void
led_draw_track(GtkWidget *track, int trackno)
{
	GdkPixbuf *pixbuf;
	
	if(!GTK_WIDGET_REALIZED(track)) return;

	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(track));
    gdk_pixbuf_fill(pixbuf, 0);

    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   (trackno / 10) * DIGIT_WIDTH, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   1, 1);
    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   (trackno % 10) * DIGIT_WIDTH, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   DIGIT_WIDTH + 1, 1);
    gtk_widget_queue_draw(track);
}

void
led_time(GtkWidget * time, int min, int sec, GtkWidget * track, int trackno)
{
	GdkPixbuf *pixbuf;
	
	if(!GTK_WIDGET_REALIZED(time)) return;

	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(time));
    gdk_pixbuf_fill(pixbuf, 0);

    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   (min / 10) * DIGIT_WIDTH, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   1, 1);
    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   (min % 10) * DIGIT_WIDTH, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   DIGIT_WIDTH + 1, 1);

    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   10 * DIGIT_WIDTH, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   2 * DIGIT_WIDTH + 2, 1);

    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   (sec / 10) * DIGIT_WIDTH, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   3 * DIGIT_WIDTH - 1, 1);

    gdk_pixbuf_copy_area ((const GdkPixbuf *)led_pixbuf,
						   (sec % 10) * DIGIT_WIDTH, 0,
						   DIGIT_WIDTH, LED_HEIGHT,
						   pixbuf,
						   4 * DIGIT_WIDTH - 1, 1);
    gtk_widget_queue_draw(time);
	led_draw_track(track, trackno);
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
		GdkPixbuf *pixbuf;
		
		if(!GTK_WIDGET_REALIZED(time)) return;
		pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(time));
        gdk_pixbuf_fill(pixbuf, 0);
        gtk_widget_queue_draw(time);
		visible = 1;
	}
}

void
led_nodisc(GtkWidget * time, GtkWidget * track)
{
	GdkPixbuf *pixbuf;
	
	if(!GTK_WIDGET_REALIZED(time)) return;

	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(time));
    gdk_pixbuf_fill(pixbuf, 0);
    gtk_image_set_from_pixbuf(GTK_IMAGE(time), pixbuf);
	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(track));
    gdk_pixbuf_fill(pixbuf, 0);
    gtk_image_set_from_pixbuf(GTK_IMAGE(track), pixbuf);
}
