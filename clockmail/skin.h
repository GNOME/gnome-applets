/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef SKIN_H
#define SKIN_H


SkinData *skin_new(void);
void skin_free(SkinData *s);

void skin_flush(AppData *ad);
void applet_skin_redraw_all(AppData *ad);

void applet_skin_backing_sync(AppData *ad);
void applet_skin_window_sync(AppData *ad);

gint skin_set(gchar *path, AppData *ad);


#endif
