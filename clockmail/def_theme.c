/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "clockmail.h"
#include "def_theme.h"
#include "item.h"
#include "button.h"
#include "number.h"
#include "skin.h"
#include "update.h"

#include "backgrnd.xpm"
#include "digmed.xpm"
#include "digsml.xpm"
#include "mailpics.xpm"
#include "button.xpm"

#include "backgrnd_tiny.xpm"
#include "button_tiny.xpm"
#include "backgrnd_tiny_v.xpm"
#include "button_tiny_v.xpm"

SkinData *skin_load_default(AppData *ad)
{
	SkinData *s;

	s = skin_new();

	if ((ad->sizehint == SIZEHINT_TINY || ad->sizehint == SIZEHINT_SMALL) &&
	    (ad->orient == ORIENT_UP || ad->orient == ORIENT_DOWN))
		{
		/* this is the tiny size theme for a horizonatal panel*/

		s->overlay = gdk_pixbuf_new_from_xpm_data((const char **)backgrnd_tiny_xpm);

		s->dig_large = digit_new_from_data((gchar **)digmed_xpm);
		s->dig_small = digit_new_from_data((gchar **)digsml_xpm);

		s->hour = number_new(s->dig_large, 2, FALSE, FALSE, 3, 4);
		s->min = number_new(s->dig_large, 2, TRUE, FALSE, 26, 4);

		s->messages = number_new(s->dig_small, 3, FALSE, FALSE, 72, 8);

		s->mail = item_new_from_data((gchar **)mailpics_xpm, 10, 49, 5);

		s->button_pix = item_new_from_data((gchar **)button_tiny_xpm, 3, 0, 0);
		}
	else if ((ad->sizehint == SIZEHINT_TINY || ad->sizehint == SIZEHINT_SMALL) &&
		 (ad->orient == ORIENT_LEFT || ad->orient == ORIENT_RIGHT))
		{
		/* this is the tiny size theme for a vertical panel*/

		s->overlay = gdk_pixbuf_new_from_xpm_data((const char **)backgrnd_tiny_v_xpm);

		s->dig_large = digit_new_from_data((gchar **)digmed_xpm);
		s->dig_small = digit_new_from_data((gchar **)digsml_xpm);

		s->hour = number_new(s->dig_large, 2, FALSE, FALSE, 2, 4);
		s->min = number_new(s->dig_large, 2, TRUE, FALSE, 2, 26);

		s->messages = number_new(s->dig_small, 3, FALSE, FALSE, 2, 59);

		s->mail = item_new_from_data((gchar **)mailpics_xpm, 10, 1, 45);

		s->button_pix = item_new_from_data((gchar **)button_tiny_v_xpm, 3, 0, 0);
		}
	else
		{
		/* this is the standard size theme */
		s->overlay = gdk_pixbuf_new_from_xpm_data((const char **)backgrnd_xpm);

		s->dig_large = digit_new_from_data((gchar **)digmed_xpm);
		s->dig_small = digit_new_from_data((gchar **)digsml_xpm);

		s->hour = number_new(s->dig_large, 2, FALSE, FALSE, 3, 5);
		s->min = number_new(s->dig_large, 2, TRUE, FALSE, 26, 5);

		s->messages = number_new(s->dig_small, 3, FALSE, FALSE, 26, 32);

		s->mail = item_new_from_data((gchar **)mailpics_xpm, 10, 3, 29);

		s->button_pix = item_new_from_data((gchar **)button_xpm, 3, 0, 0);
		}

	s->width = gdk_pixbuf_get_width(s->overlay);
	s->height = gdk_pixbuf_get_height(s->overlay);

	s->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, s->width, s->height);

	s->button = button_new(s->button_pix, s->width, s->height, launch_mail_reader, ad, update_all, ad);

	return s;
}


