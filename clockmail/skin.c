/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "clockmail.h"
#include "skin.h"
#include "item.h"
#include "button.h"
#include "number.h"
#include "clock.h"
#include "theme.h"
#include "def_theme.h"
#include "pixbuf_ops.h"
#include "update.h"

SkinData *skin_new(void)
{
	SkinData *s;
	s = g_new0(SkinData, 1);
	return s;
}

void skin_free(SkinData *s)
{
	if (!s) return;

	if (s->pixbuf) gdk_pixbuf_unref(s->pixbuf);
	if (s->overlay) gdk_pixbuf_unref(s->overlay);

	number_free(s->hour);
	number_free(s->min);
	number_free(s->sec);
	number_free(s->month);
	number_free(s->day);
	number_free(s->year);
	number_free(s->mail_count);
	number_free(s->messages);

	digit_free(s->dig_small);
	digit_free(s->dig_large);

	item_free(s->mail);
	item_free(s->month_txt);
	item_free(s->week_txt);
	item_free(s->mail_amount);

	item_free(s->button_pix);
	button_free(s->button);

	clock_free(s->clock);

	g_free(s);
}

void skin_flush(AppData *ad)
{
	GdkPixmap *pixmap;

	if (!ad->display->window) return;

	/* FIXME: memory/time consuming, should keep a pixmap around for this */

	gdk_pixbuf_render_pixmap_and_mask(ad->skin->pixbuf, &pixmap, NULL, 128);

	gdk_window_set_back_pixmap(ad->display->window, pixmap, FALSE);
	gdk_window_clear(ad->display->window);

	gdk_pixmap_unref(pixmap);
}

void applet_skin_redraw_all(AppData *ad)
{
	update_time_and_date(ad, TRUE);
	update_mail_status(ad, TRUE);
}

static gint applet_skin_backing_sync_idle(gpointer data)
{
	AppData *ad = data;

	applet_skin_backing_sync(ad);

	return FALSE;
}

void applet_skin_backing_sync(AppData *ad)
{
	guchar *rgb = NULL;
	int w, h, rs;

	if (!ad->skin) return;

	applet_widget_get_rgb_bg(APPLET_WIDGET(ad->applet),
				 &rgb, &w, &h, &rs);

	if (rgb == NULL) return;

	if (w != gdk_pixbuf_get_width(ad->skin->pixbuf) || h != gdk_pixbuf_get_height(ad->skin->pixbuf))
		{
		/* out of sync, this happens in startup, when changing
		 * panel size, switching panels, etc. So fill the back with the first color, may be
		 * ugly on pixmap backs, but better than showing unzeroed memory till the next sync.
		 */

		guchar *p = rgb;
		int r, g, b;
		r = *p; p++;
		g = *p; p++;
		b = *p;
		pixbuf_draw_rect_fill(ad->skin->pixbuf,
				      0, 0, ad->skin->width, ad->skin->height,
				      r, g, b, 255);

		/* bah, newer panels don't seem to send the do_draw signal once the applet size is synced.
		   probably because it thinks we already got the correct size here. (wrong!)
		   so, add an idle callback to do this all over again *sigh*
		 */
		g_idle_add(applet_skin_backing_sync_idle, ad);
		}
	else
		{
		GdkPixbuf *pb;

		pb = gdk_pixbuf_new_from_data(rgb, GDK_COLORSPACE_RGB, FALSE, 8,
					      w, h, rs, NULL, NULL);
		pixbuf_copy_area(pb, 0, 0, ad->skin->pixbuf, 0, 0, w, h, FALSE);

		gdk_pixbuf_unref(pb);
		}

	g_free(rgb);

	pixbuf_copy_area_alpha(ad->skin->overlay, 0, 0,
			       ad->skin->pixbuf, 0, 0, ad->skin->width, ad->skin->height, 255);

	item_back_set(ad->skin->mail, ad->skin->pixbuf);
	item_back_set(ad->skin->month_txt, ad->skin->pixbuf);
	item_back_set(ad->skin->week_txt, ad->skin->pixbuf);
	item_back_set(ad->skin->mail_amount, ad->skin->pixbuf);
	item_back_set(ad->skin->button_pix, ad->skin->pixbuf);

	number_back_set(ad->skin->hour, ad->skin->pixbuf);
	number_back_set(ad->skin->min, ad->skin->pixbuf);
	number_back_set(ad->skin->sec, ad->skin->pixbuf);
	number_back_set(ad->skin->month, ad->skin->pixbuf);
	number_back_set(ad->skin->day, ad->skin->pixbuf);
	number_back_set(ad->skin->year, ad->skin->pixbuf);
	number_back_set(ad->skin->mail_count, ad->skin->pixbuf);
	number_back_set(ad->skin->messages, ad->skin->pixbuf);

	clock_back_set(ad->skin->clock, ad->skin->pixbuf);

	button_draw(ad->skin->button, FALSE, FALSE, TRUE, ad->skin->pixbuf, FALSE);
	applet_skin_redraw_all(ad);

	skin_flush(ad);
}

void applet_skin_window_sync(AppData *ad)
{
	if (!ad->skin) return;

	gtk_drawing_area_size(GTK_DRAWING_AREA(ad->display), ad->skin->width, ad->skin->height);
	gtk_widget_set_usize(ad->display, ad->skin->width, ad->skin->height);
	gtk_widget_set_usize(ad->applet, ad->skin->width, ad->skin->height);

	if (ad->skin->mail)
		ad->mail_sections = ad->skin->mail->sections;
	else
		ad->mail_sections = 1;

	applet_skin_backing_sync(ad);
}

static SkinData *skin_load_best_match(gchar *path, gint vertical, gint size, AppData *ad)
{
	SkinData *s = NULL;

	s = skin_load(path, vertical, size, ad);
	if (s) return s;

	/* tiny falls back to small */
	if (size == SIZEHINT_TINY)
		{
		s = skin_load(path, vertical, SIZEHINT_SMALL, ad);
		if (s) return s;
		}

	/* small falls back to tiny */
	if (size == SIZEHINT_SMALL)
		{
		s = skin_load(path, vertical, SIZEHINT_TINY, ad);
		if (s) return s;
		}

	/* huge falls back to large */
	if (size == SIZEHINT_HUGE)
		{
		s = skin_load(path, vertical, SIZEHINT_LARGE, ad);
		if (s) return s;
		}

	if (size == SIZEHINT_STANDARD) return NULL;

	/* failed so far, try standard */
	return skin_load(path, vertical, SIZEHINT_STANDARD, ad);
}

gint skin_set(gchar *path, AppData *ad)
{
	SkinData *new_s = NULL;
	SkinData *old_s = ad->skin;

	if (!path)
		{
		new_s = skin_load_default(ad);
		}
	else
		{
		new_s = skin_load_best_match(path,
					     (ad->orient == ORIENT_LEFT || ad->orient == ORIENT_RIGHT),
					     ad->sizehint, ad);
		if (!new_s)
			{
			new_s = skin_load(path,
					  !(ad->orient == ORIENT_LEFT || ad->orient == ORIENT_RIGHT),
					  SIZEHINT_STANDARD, ad);
			}
		}

	if (!new_s) return FALSE;

	ad->skin = new_s;

	applet_skin_window_sync(ad);
	ad->active = NULL;

	skin_free(old_s);

	return TRUE;
}


