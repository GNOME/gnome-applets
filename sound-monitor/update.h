/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef UPDATE_H
#define UPDATE_H


void update_modes(AppData *ad);
void update_fps_timeout(AppData *ad);

gint update_display(gpointer data);


#endif
