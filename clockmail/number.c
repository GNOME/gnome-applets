/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "clockmail.h"
#include "number.h"
#include "pixbuf_ops.h"

static DigitData *digit_new(GdkPixbuf *pb)
{
	DigitData *digit;

	if (!pb) return NULL;

	digit = g_new0(DigitData, 1);

	digit->overlay = pb;
	digit->width = gdk_pixbuf_get_width(pb) / 11;
	digit->height = gdk_pixbuf_get_height(pb);

	return digit;
}

DigitData *digit_new_from_data(gchar **data)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_xpm_data((const char **)data);

	return digit_new(pb);
}

DigitData *digit_new_from_file(gchar *file)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_file(file);

	return digit_new(pb);
}

void digit_free(DigitData *digit)
{
	if (!digit) return;
	if (digit->overlay) gdk_pixbuf_unref(digit->overlay);
	g_free(digit);
}

static void digit_draw(DigitData *digit, gint n, gint x, gint y, GdkPixbuf *pb)
{
	if (!digit) return;

	if (n == -1) n = 10;
	if (n >= 0 && n <= 10)
		{
		pixbuf_copy_area_alpha(digit->overlay, digit->width * n, 0,
				       pb, x, y, digit->width, digit->height, 255);
		}
}

NumberData *number_new(DigitData *digit, gint length, gint zeros, gint centered, gint x, gint y)
{
	NumberData *number;
	number = g_new0(NumberData, 1);

	number->digits = digit;
	number->length = length;
	number->zeros = zeros;
	number->centered = centered;
	number->x = x;
	number->y = y;

	number->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, digit->width * length, digit->height);

	return number;
}

void number_free(NumberData *number)
{
	if (!number) return;
	if (number->pixbuf) gdk_pixbuf_unref(number->pixbuf);
	g_free(number);
}

void number_draw(NumberData *number, gint n, GdkPixbuf *pb)
{
	gint i, d;
	gint t = 1;
	gint x;
	gint z;
	DigitData *digit;

	if (!number) return;

	digit = number->digits;
	pixbuf_copy_area(number->pixbuf, 0, 0, pb, number->x, number->y,
			 digit->width * number->length, digit->height, FALSE);

	x = number->x + (number->length * digit->width);
	z = number->zeros;

	for (i=0; i< number->length - 1; i++) t *= 10;
	n = n % ( t*10 ) ; /* truncate to number->length */

	if (number->centered && n < t)
		{
		gint b = FALSE;
		for (i=number->length; i > 0; i--)
			{
			d = n / t;
			n -= d * t;
			t = t / 10;
			if (!(d == 0 && i>1 && !b))
				{
				if (!b)
					{
					x = number->x + (number->length * digit->width / 2) + (i * digit->width / 2);
					b = TRUE;
					}
				digit_draw(digit, d, x - (i * digit->width), number->y, pb);
				}
			}
		return;
		}

	for (i=number->length; i > 0; i--)
		{
		d = n / t;
		n -= d * t;
		t = t / 10;
		if (d == 0 && i>1 && (!z))
			{
			d = -1;
			}
		else
			{
			z = TRUE;
			}
		digit_draw(digit, d, x - (i * digit->width), number->y, pb);
		}
}

void number_back_set(NumberData *number, GdkPixbuf *pb)
{
	if (!number) return;

	pixbuf_copy_area(pb, number->x, number->y, number->pixbuf, 0, 0,
			 number->digits->width * number->length, number->digits->height, FALSE);
}


