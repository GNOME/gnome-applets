#include <gtk/gtk.h>
#include "led.h"
#include "images/led.xpm"

static GdkPixmap *led_pixmap;

void
led_init(GtkWidget * w)
{
	GdkColor black;

	gdk_color_black(gtk_widget_get_colormap(w), &black);
	led_pixmap = gdk_pixmap_create_from_xpm_d(w->window, NULL,
						  &black, led);
}

void
led_done()
{
	gdk_pixmap_unref(led_pixmap);
}

void
led_create_widget(GtkWidget * window, GtkWidget ** time, GtkWidget ** track)
{
	GtkWidget *w;
	GdkPixmap *pix;
	GdkGC *gc;
	
	gc = gdk_gc_new(window->window);

	pix = gdk_pixmap_new(window->window,
			     LED_WIDTH, LED_HEIGHT + 2, -1);
	gdk_draw_rectangle(pix,gc,TRUE,0,0,-1,-1);
	*time = gtk_pixmap_new(pix,NULL);

	pix = gdk_pixmap_new(window->window,
			     DIGIT_WIDTH * 2 + 2, LED_HEIGHT + 2, -1);
	gdk_draw_rectangle(pix,gc,TRUE,0,0,-1,-1);
	*track = gtk_pixmap_new(pix,NULL);
	
	gdk_gc_destroy(gc);
	return;
	w = NULL;
}

void
led_stop(GtkWidget * time, GtkWidget * track)
{
	GdkPixmap *p;
	GtkStyle *style;
	int retval;
	
	if(!GTK_WIDGET_REALIZED(time)) return;

	style = gtk_widget_get_style(time);
	gtk_pixmap_get(GTK_PIXMAP(time), &p, NULL);
	gdk_draw_rectangle(p, style->black_gc, 1, 0, 0, -1, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH + 7,
			0, 1, 1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH + 7,
			0, DIGIT_WIDTH + 1, 1, DIGIT_WIDTH, -1);
	/* : */
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH,
			0, 2 * DIGIT_WIDTH + 2, 1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH + 7,
			0, 3 * DIGIT_WIDTH - 1, 1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH + 7,
			0, 4 * DIGIT_WIDTH - 1, 1, DIGIT_WIDTH, -1);
	gtk_pixmap_get(GTK_PIXMAP(track), &p, NULL);
	gdk_draw_rectangle(p, style->black_gc, 1, 0, 0, -1, -1);
	gtk_signal_emit_by_name(GTK_OBJECT(time), "draw", &retval);
	gtk_signal_emit_by_name(GTK_OBJECT(track), "draw", &retval);

}

static void
led_draw_track(GtkStyle * style, GtkWidget * track, int trackno)
{
	GdkPixmap *p;

	if(!GTK_WIDGET_REALIZED(track)) return;

	gtk_pixmap_get(GTK_PIXMAP(track), &p, NULL);
	gdk_draw_rectangle(p, style->black_gc, 1, 0, 0, -1, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (trackno / 10) * DIGIT_WIDTH,
			0, 1, 1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (trackno % 10) * DIGIT_WIDTH,
			0, DIGIT_WIDTH + 1, 1, DIGIT_WIDTH, -1);
}

void
led_time(GtkWidget * time, int min, int sec, GtkWidget * track, int trackno)
{
	GdkPixmap *p;
	GtkStyle *style;
	int retval;

	if(!GTK_WIDGET_REALIZED(time)) return;

	style = gtk_widget_get_style(time);
	gtk_pixmap_get(GTK_PIXMAP(time), &p, NULL);
	gdk_draw_rectangle(p, style->black_gc, 1, 0, 0, -1, -1);
	/* minute */
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (min / 10) * DIGIT_WIDTH,
			0, 1, 1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (min % 10) * DIGIT_WIDTH,
			0, DIGIT_WIDTH + 1, 1, DIGIT_WIDTH, -1);
	/* : */
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, 10 * DIGIT_WIDTH,
			0, 2 * DIGIT_WIDTH + 2, 1, DIGIT_WIDTH, -1);
	/* second */
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (sec / 10) * DIGIT_WIDTH,
			0, 3 * DIGIT_WIDTH - 1, 1, DIGIT_WIDTH, -1);
	gdk_draw_pixmap(p, style->white_gc, led_pixmap, (sec % 10) * DIGIT_WIDTH,
			0, 4 * DIGIT_WIDTH - 1, 1, DIGIT_WIDTH, -1);

	led_draw_track(style, track, trackno);
	gtk_signal_emit_by_name(GTK_OBJECT(time), "draw", &retval);
	gtk_signal_emit_by_name(GTK_OBJECT(track), "draw", &retval);
}

void
led_paused(GtkWidget * time, int min, int sec, GtkWidget * track, int trackno)
{
	static int visible = 1;

	if(!GTK_WIDGET_REALIZED(time)) return;

	if (visible == 1) {
		led_time(time, min, sec, track, trackno);
		visible = 0;
	} else {
		GdkPixmap *p;
		GtkStyle *style;
		int retval;

		style = gtk_widget_get_style(time);
		gtk_pixmap_get(GTK_PIXMAP(time), &p, NULL);
		gdk_draw_rectangle(p, style->black_gc, 1, 0, 0, -1, -1);
		led_draw_track(style, track, trackno);
		visible = 1;
		gtk_signal_emit_by_name(GTK_OBJECT(time), "draw", &retval);
		gtk_signal_emit_by_name(GTK_OBJECT(track), "draw", &retval);
	}
}

void
led_nodisc(GtkWidget * time, GtkWidget * track)
{
	GdkPixmap *p;
	GtkStyle *style;
	int retval;

	if(!GTK_WIDGET_REALIZED(time)) return;

	style = gtk_widget_get_style(time);
	gtk_pixmap_get(GTK_PIXMAP(time), &p, NULL);
	gdk_draw_rectangle(p, style->black_gc, 1, 0, 0, -1, -1);
	gtk_pixmap_get(GTK_PIXMAP(track), &p, NULL);
	gdk_draw_rectangle(p, style->black_gc, 1, 0, 0, -1, -1);
	gtk_signal_emit_by_name(GTK_OBJECT(time), "draw", &retval);
	gtk_signal_emit_by_name(GTK_OBJECT(track), "draw", &retval);
}
