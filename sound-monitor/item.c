/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "sound-monitor.h"
#include "item.h"
#include "pixbuf_ops.h"

static ItemData *item_new(GdkPixbuf *pb, gint sections, gint x, gint y)
{
	ItemData *item;

	if (!pb) return NULL;

	item = g_new0(ItemData, 1);

	item->overlay = pb;
	item->width = gdk_pixbuf_get_width(item->overlay);
	item->height = gdk_pixbuf_get_height(item->overlay) / sections;
	item->sections = sections;
	item->x = x;
	item->y = y;

	item->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, item->width, gdk_pixbuf_get_height(item->overlay));
	item->old_val = -1;

	return item;
}

ItemData *item_new_from_data(gchar **data, gint sections, gint x, gint y)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_xpm_data((const char **)data);

	return item_new(pb, sections, x, y);
}

ItemData *item_new_from_file(gchar *file, gint sections, gint x, gint y)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_file(file);

	return item_new(pb, sections, x, y);
}

void item_free(ItemData *item)
{
	if (!item) return;
	if (item->pixbuf) gdk_pixbuf_unref(item->pixbuf);
	if (item->overlay) gdk_pixbuf_unref(item->overlay);
	g_free(item);
}

gint item_draw(ItemData *item, gint section, GdkPixbuf *pb)
{
	if (!item || section >= item->sections) return FALSE;

	if (item->old_val == section) return FALSE;

	item->old_val = section;

	pixbuf_copy_area(item->pixbuf, 0, item->height * section,
			 pb, item->x, item->y,
			 item->width, item->height, FALSE);

	return TRUE;
}

gint item_draw_by_percent(ItemData *item, gint percent, GdkPixbuf *pb)
{
	gint section;
	if (!item) return FALSE;

	section = (float)item->sections * percent / 100;
	if (section == 0 && percent > 0) section = 1;
	if (section > item->sections - 1) section = item->sections - 1;

	return item_draw(item, section, pb);
}

void item_back_set(ItemData *item, GdkPixbuf *pb)
{
	gint i;

	if (!item) return;

	for (i = 0; i < item->sections; i++)
		{
		pixbuf_copy_area(pb, item->x, item->y, item->pixbuf,
				 0, i * item->height, item->width, item->height, FALSE);
		}

	pixbuf_copy_area_alpha(item->overlay, 0, 0,
			       item->pixbuf, 0, 0, item->width, gdk_pixbuf_get_height(item->overlay), 255);
}

