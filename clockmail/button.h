/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef BUTTON_H
#define BUTTON_H


ButtonData *button_new(ItemData *item, gint width, gint height,
		       void (*click_func)(gpointer), gpointer click_data,
		       void (*redraw_func)(gpointer), gpointer redraw_data);
void button_free(ButtonData *button);
gint button_draw(ButtonData *button, gint prelight, gint pressed, gint force,
		 GdkPixbuf *pb, gint enable_draw_cb);


#endif
