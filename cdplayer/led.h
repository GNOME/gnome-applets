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

#include <glib.h>
#include <gtk/gtk.h>

#ifndef __LED_H__
#define __LED_H__

G_BEGIN_DECLS

#define DIGIT_WIDTH 9
#define LED_HEIGHT 11
#define LED_WIDTH (DIGIT_WIDTH * 5)

void led_init(void);
void led_done(void);

void led_create_widgets(GtkWidget **time, GtkWidget **track, gpointer data);
void led_time(GtkWidget *time, int min, int sec, GtkWidget *track, int trackno);
void led_stop(GtkWidget *time, GtkWidget *track);
void led_pause(GtkWidget * time, int min, int sec, GtkWidget *track, int trackno);
void led_nodisc(GtkWidget * time, GtkWidget * track);
void led_paused(GtkWidget * time, int min, int sec, GtkWidget * track, int trackno);

G_END_DECLS

#endif
