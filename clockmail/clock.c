/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "clockmail.h"
#include "clock.h"
#include "pixbuf_ops.h"

static HandData *hand_new(GdkPixbuf *pb, gint xo, gint yo)
{
	HandData *hd;

	if (!pb) return NULL;

	hd = g_new0(HandData, 1);

	hd->pixbuf = pb;
	hd->xo = xo;
	hd->yo = yo;

	return hd;
}

HandData *hand_new_from_data(gchar **data, gint xo, gint yo)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_xpm_data((const char **)data);

	return hand_new(pb, xo, yo);
}

HandData *hand_new_from_file(gchar *file, gint xo, gint yo)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_file(file);

	return hand_new(pb, xo, yo);
}

void hand_free(HandData *hd)
{
	if (!hd) return;

	if (hd->pixbuf) gdk_pixbuf_unref(hd->pixbuf);
	g_free(hd);
}

void hand_draw(HandData *hd, gint x, gint y, gint r_deg, GdkPixbuf *pb)
{
	if (!hd) return;

	pixbuf_copy_rotate_alpha(hd->pixbuf, hd->xo, hd->yo, pb, x, y, (double) r_deg);
}



static ClockData *clock_new(HandData *h, HandData *m, HandData *s,
			    GdkPixbuf *pb, gint x, gint y, gint width, gint height)
{
	ClockData *cd;

	cd = g_new0(ClockData, 1);

	cd->hour = h;
	cd->minute = m;
	cd->second = s;

	cd->overlay = pb;
	if (cd->overlay)
		{
		cd->width = gdk_pixbuf_get_width(pb);
		cd->height = gdk_pixbuf_get_height(pb);
		}
	else
		{
		cd->width = width;
		cd->height = height;
		}
	cd->x = x;
	cd->y = y;
	cd->cx = cd->width / 2;
	cd->cy = cd->height / 2;

	cd->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, cd->width, cd->height);

	return cd;
}

ClockData *clock_new_from_data(HandData *h, HandData *m, HandData *s,
			       gchar **data, gint x, gint y, gint width, gint height)
{
	GdkPixbuf *pb;

	if (data)
		pb = gdk_pixbuf_new_from_xpm_data((const char **)data);
	else
		pb = NULL;

	return clock_new(h, m, s, pb, x, y, width, height);
}

ClockData *clock_new_from_file(HandData *h, HandData *m, HandData *s,
			       gchar *file, gint x, gint y)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_file(file);
	if (!pb) return NULL;

	return clock_new(h, m, s, pb, x, y, 0, 0);
}

void clock_free(ClockData *cd)
{
	if (!cd) return;

	hand_free(cd->hour);
	hand_free(cd->minute);
	hand_free(cd->second);
	if (cd->pixbuf) gdk_pixbuf_unref(cd->pixbuf);
	if (cd->overlay) gdk_pixbuf_unref(cd->overlay);
	g_free(cd);
}

void clock_draw(ClockData *cd, gint h, gint m, gint s, GdkPixbuf *pb)
{
	if (!cd) return;

	pixbuf_copy_area(cd->pixbuf, 0, 0, pb, cd->x, cd->y, cd->width, cd->height, FALSE);

	hand_draw(cd->hour, cd->x + cd->cx, cd->y + cd->cy, h * 30 + ((float)m * 0.5), pb);
	hand_draw(cd->minute, cd->x + cd->cx, cd->y + cd->cy, m * 6 + ((float)s * 0.1), pb);
	hand_draw(cd->second, cd->x + cd->cx, cd->y + cd->cy, s * 6, pb);
}

void clock_back_set(ClockData *cd, GdkPixbuf *pb)
{
	if (!cd) return;

	pixbuf_copy_area(pb, cd->x, cd->y, cd->pixbuf, 0, 0, cd->width, cd->height, FALSE);

	if (cd->overlay)
		{
		pixbuf_copy_area_alpha(cd->pixbuf, 0, 0, cd->overlay, 0, 0, cd->width, cd->height, 255);
		}
}


