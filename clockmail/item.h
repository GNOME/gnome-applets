/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef ITEM_H
#define ITEM_H


ItemData *item_new_from_data(gchar **data, gint sections, gint x, gint y);
ItemData *item_new_from_file(gchar *file, gint sections, gint x, gint y);
void item_free(ItemData *item);
void item_draw(ItemData *item, gint section, GdkPixbuf *pb);
void item_back_set(ItemData *item, GdkPixbuf *pb);


#endif

