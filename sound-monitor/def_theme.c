/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "sound-monitor.h"
#include "def_theme.h"
#include "skin.h"
#include "item.h"
#include "meter.h"

#include "def_theme/back_h.xpm"
#include "def_theme/led_h.xpm"
#include "def_theme/status_h.xpm"
#include "def_theme/back_v.xpm"
#include "def_theme/led_v.xpm"
#include "def_theme/status_v.xpm"

static SkinData *default_skin_h()
{
	SkinData *s;

	s = skin_new();

	s->overlay = gdk_pixbuf_new_from_xpm_data((const gchar **)back_h_xpm);

	s->width = gdk_pixbuf_get_width(s->overlay);
	s->height = gdk_pixbuf_get_height(s->overlay);

	s->meter_left = meter_new_from_data((gchar **)led_h_xpm, 14, 1, 1, FALSE);
	s->meter_right = meter_new_from_data((gchar **)led_h_xpm, 14, 10, 1, FALSE);

	s->status = item_new_from_data((gchar **)status_h_xpm, 4, 1, 45);

	s->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, s->width, s->height);

	return s;
}

static SkinData *default_skin_v()
{
	SkinData *s;

	s = skin_new();

	s->overlay = gdk_pixbuf_new_from_xpm_data((const gchar **)back_v_xpm);

	s->width = gdk_pixbuf_get_width(s->overlay);
	s->height = gdk_pixbuf_get_height(s->overlay);

	s->meter_left = meter_new_from_data((gchar **)led_v_xpm, 14, 5, 1, TRUE);
	s->meter_right = meter_new_from_data((gchar **)led_v_xpm, 14, 5, 10, TRUE);

	s->status = item_new_from_data((gchar **)status_v_xpm, 4, 1, 1);

	s->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, s->width, s->height);

	return s;
}

SkinData *skin_load_default(AppData *ad)
{
	if (ad->orient == ORIENT_UP || ad->orient == ORIENT_DOWN)
		{
		if (ad->sizehint == SIZEHINT_TINY || ad->sizehint == SIZEHINT_SMALL)
			return default_skin_v();
		else
			return default_skin_h();
		}
	else
		{
		if (ad->sizehint == SIZEHINT_TINY || ad->sizehint == SIZEHINT_SMALL)
			return default_skin_h();
		else
			return default_skin_v();
		}
}


