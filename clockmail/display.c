/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "clockmail.h"
#include "display.h"
#include "button.h"

/*
 *-----------------------------------
 * motion/button events for buttons
 *-----------------------------------
 */

static void display_motion(GtkWidget *w, GdkEventMotion *event, gpointer data)
{
	AppData *ad = data;
	ButtonData *button;
	gint x = (float)event->x;
	gint y = (float)event->y;

	button = ad->skin->button;
	if (!button) return;

	if (x >= button->x && x < button->x + button->width &&
				y >= button->y && y < button->y + button->height)
		{
		if (ad->active == button)
			button_draw(button, FALSE, TRUE, FALSE, ad->skin->pixbuf, TRUE);
		else
			button_draw(button, TRUE, FALSE, FALSE, ad->skin->pixbuf, TRUE);
		}
	else
		{
		button_draw(button, FALSE, FALSE, FALSE, ad->skin->pixbuf, TRUE);
		}
	return;
}

static void display_pressed(GtkWidget *w, GdkEventButton *event, gpointer data)
{
	AppData *ad = data;
	ButtonData *button;
	gint x = (float)event->x;
	gint y = (float)event->y;

	button = ad->skin->button;
	if (!button) return;

	if (x >= button->x && x < button->x + button->width &&
				y >= button->y && y < button->y + button->height)
		{
		ad->active = button;
		button_draw(button, FALSE, TRUE, FALSE, ad->skin->pixbuf, TRUE);
		}
	return;
}

static void display_released(GtkWidget *w, GdkEventButton *event, gpointer data)
{
	AppData *ad = data;
	ButtonData *button;
	gint x = (float)event->x;
	gint y = (float)event->y;


	if (!ad->active) return;
	button = ad->active;
	ad->active = NULL;

	if (button->pushed && x >= button->x && x < button->x + button->width &&
				y >= button->y && y < button->y + button->height)
		{
		button_draw(button, FALSE, FALSE, FALSE, ad->skin->pixbuf, TRUE);
		if (button->click_func)
			button->click_func(button->click_data);
		}
	return;
}

static void display_leave(GtkWidget *w, GdkEventCrossing *event, gpointer data)
{
	AppData *ad = data;
	ButtonData *button;

	button = ad->skin->button;
	if (!button) return;

	button_draw(button, FALSE, FALSE, FALSE, ad->skin->pixbuf, TRUE);
	return;
}

void display_events_init(AppData *ad)
{
	gtk_widget_set_events(ad->display, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK |
			      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_signal_connect(GTK_OBJECT(ad->display),"motion_notify_event",(GtkSignalFunc) display_motion, ad);
	gtk_signal_connect(GTK_OBJECT(ad->display),"button_press_event",(GtkSignalFunc) display_pressed, ad);
	gtk_signal_connect(GTK_OBJECT(ad->display),"button_release_event",(GtkSignalFunc) display_released, ad);
	gtk_signal_connect(GTK_OBJECT(ad->display),"leave_notify_event",(GtkSignalFunc) display_leave, ad);
}
