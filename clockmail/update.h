/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef UPDATE_H
#define UPDATE_H

void update_time_and_date(AppData *ad, gint force);
void update_mail_status(AppData *ad, gint force);

void update_all(gpointer data);
gint update_display(gpointer data);


#endif
