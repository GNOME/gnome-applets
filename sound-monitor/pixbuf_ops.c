/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "sound-monitor.h"
#include "pixbuf_ops.h"

#include "math.h"

/*
 * Copies src to dest, optionally applying alpha
 * but takes into account the source's alpha channel,
 * no source alpha results in a simple copy.
 */
static void _pixbuf_copy_area_alpha(GdkPixbuf *src, gint sx, gint sy,
				    GdkPixbuf *dest, gint dx, gint dy,
				    gint w, gint h, gint apply_alpha,
				    gint alpha_modifier, gint alpha_too)
{
	gint s_alpha;
	gint d_alpha;
	gint sw, sh, srs;
	gint dw, dh, drs;
	guchar *s_pix;
        guchar *d_pix;
	guchar *sp;
        guchar *dp;
	guchar r, g, b, a;
	gint i, j;

	if (!src || !dest) return;

	sw = gdk_pixbuf_get_width(src);
	sh = gdk_pixbuf_get_height(src);

	if (sx < 0 || sx + w > sw) return;
	if (sy < 0 || sy + h > sh) return;

	dw = gdk_pixbuf_get_width(dest);
	dh = gdk_pixbuf_get_height(dest);

	if (dx < 0 || dx + w > dw) return;
	if (dy < 0 || dy + h > dh) return;

	s_alpha = gdk_pixbuf_get_has_alpha(src);
	d_alpha = gdk_pixbuf_get_has_alpha(dest);
	srs = gdk_pixbuf_get_rowstride(src);
	drs = gdk_pixbuf_get_rowstride(dest);
	s_pix = gdk_pixbuf_get_pixels(src);
	d_pix = gdk_pixbuf_get_pixels(dest);

	if (s_alpha && apply_alpha)
		{
	        for (i = 0; i < h; i++)
			{
			sp = s_pix + (sy + i) * srs + sx * 4;
			dp = d_pix + (dy + i) * drs + (dx * (d_alpha ? 4 : 3));
			for (j = 0; j < w; j++)
				{
				r = *(sp++);
				g = *(sp++);
				b = *(sp++);
				a = (*(sp++) * alpha_modifier) >> 8;
				*dp = (r * a + *dp * (256-a)) >> 8;
				dp++;
				*dp = (g * a + *dp * (256-a)) >> 8;
				dp++;
				*dp = (b * a + *dp * (256-a)) >> 8;
				dp++;
				if (d_alpha) dp++;
				}
			}
		}
	else if (alpha_too && s_alpha && d_alpha)
		{
		/* I wonder if using memcopy would be faster? */
	        for (i = 0; i < h; i++)
			{
			sp = s_pix + (sy + i) * srs + (sx * 4);
			dp = d_pix + (dy + i) * drs + (dx * 4);
			for (j = 0; j < w; j++)
				{
				*(dp++) = *(sp++);	/* r */
				*(dp++) = *(sp++);	/* g */
				*(dp++) = *(sp++);	/* b */
				*(dp++) = *(sp++);	/* a */
				}
			}
		}
	else
		{
		/* I wonder if using memcopy would be faster? */
	        for (i = 0; i < h; i++)
			{
			sp = s_pix + (sy + i) * srs + (sx * (s_alpha ? 4 : 3));
			dp = d_pix + (dy + i) * drs + (dx * (d_alpha ? 4 : 3));
			for (j = 0; j < w; j++)
				{
				*(dp++) = *(sp++);	/* r */
				*(dp++) = *(sp++);	/* g */
				*(dp++) = *(sp++);	/* b */
				if (s_alpha) sp++;	/* a (?) */
				if (d_alpha) dp++;	/* a (?) */
				}
			}
		}
}

/*
 * Copies src to dest, ignoring alpha
 */
void pixbuf_copy_area(GdkPixbuf *src, gint sx, gint sy,
		      GdkPixbuf *dest, gint dx, gint dy,
		      gint w, gint h, gint alpha_too)
{
	_pixbuf_copy_area_alpha(src, sx, sy, dest, dx, dy, w, h, FALSE, 0, alpha_too);
}

/*
 * Copies src to dest, applying alpha
 * no source alpha results in a simple copy.
 */
void pixbuf_copy_area_alpha(GdkPixbuf *src, gint sx, gint sy,
			    GdkPixbuf *dest, gint dx, gint dy,
			    gint w, gint h, gint alpha_modifier)
{
	_pixbuf_copy_area_alpha(src, sx, sy, dest, dx, dy, w, h, TRUE, alpha_modifier, FALSE);
}


static void buf_pixel_get(guchar *buf, gint x, gint y, guint8 *r, guint8 *g, guint8 *b, guint8 *a,
			  gint has_alpha, gint rowstride)
{
	guchar *p;

	p = buf + (y * rowstride) + (x * (has_alpha ? 4 : 3));

	*r = *(p++);
	*g = *(p++);
	*b = *(p++);
	*a = has_alpha ? *p : 255;
}

static void buf_pixel_set(guchar *buf, gint x, gint y, guint8 r, guint8 g, guint8 b, guint8 a,
			  gint has_alpha, gint rowstride)
{
	guchar *p;

	p = buf + (y * rowstride) + (x * (has_alpha ? 4 : 3));

	*p = (r * a + *p * (256 - a)) >> 8;
	p++;
	*p = (g * a + *p * (256 - a)) >> 8;
	p++;
	*p = (b * a + *p * (256 - a)) >> 8;
}

/*
 * Fills region of pixbuf at x,y over w,h
 * with colors red (r), green (g), blue (b)
 * applying alpha (a), use a=255 for solid.
 */
void pixbuf_draw_rect_fill(GdkPixbuf *pb,
			   gint x, gint y, gint w, gint h,
			   gint r, gint g, gint b, gint a)
{
	gint p_alpha;
	gint pw, ph, prs;
	guchar *p_pix;
	guchar *pp;
	gint i, j;

	if (!pb) return;

	pw = gdk_pixbuf_get_width(pb);
	ph = gdk_pixbuf_get_height(pb);

	if (x < 0 || x + w > pw) return;
	if (y < 0 || y + h > ph) return;

	p_alpha = gdk_pixbuf_get_has_alpha(pb);
	prs = gdk_pixbuf_get_rowstride(pb);
	p_pix = gdk_pixbuf_get_pixels(pb);

        for (i = 0; i < h; i++)
		{
		pp = p_pix + (y + i) * prs + (x * (p_alpha ? 4 : 3));
		for (j = 0; j < w; j++)
			{
			*pp = (r * a + *pp * (256-a)) >> 8;
			pp++;
			*pp = (g * a + *pp * (256-a)) >> 8;
			pp++;
			*pp = (b * a + *pp * (256-a)) >> 8;
			pp++;
			if (p_alpha) pp++;
			}
		}
}

void pixbuf_copy_point(GdkPixbuf *src, gint sx, gint sy,
		       GdkPixbuf *dest, gint dx, gint dy, gint use_alpha)
{
	guint8 r, g, b, a;

	if (sx < 0 || sx >= gdk_pixbuf_get_width(src)) return;
	if (sy < 0 || sy >= gdk_pixbuf_get_height(src)) return;

	if (dx < 0 || dx >= gdk_pixbuf_get_width(dest)) return;
	if (dy < 0 || dy >= gdk_pixbuf_get_height(dest)) return;

	buf_pixel_get(gdk_pixbuf_get_pixels(src), sx, sy, &r, &g, &b, &a, 
		      gdk_pixbuf_get_has_alpha(src), gdk_pixbuf_get_rowstride(src));

	buf_pixel_set(gdk_pixbuf_get_pixels(dest), dx, dy, r, g, b, use_alpha ? a : 255,
		      gdk_pixbuf_get_has_alpha(dest), gdk_pixbuf_get_rowstride(dest));
}

void pixbuf_copy_point_at_alpha(GdkPixbuf *src, gint sx, gint sy,
				GdkPixbuf *dest, gint dx, gint dy, gint alpha)
{
	guint8 r, g, b, a;

	if (sx < 0 || sx >= gdk_pixbuf_get_width(src)) return;
	if (sy < 0 || sy >= gdk_pixbuf_get_height(src)) return;

	if (dx < 0 || dx >= gdk_pixbuf_get_width(dest)) return;
	if (dy < 0 || dy >= gdk_pixbuf_get_height(dest)) return;

	buf_pixel_get(gdk_pixbuf_get_pixels(src), sx, sy, &r, &g, &b, &a, 
		      gdk_pixbuf_get_has_alpha(src), gdk_pixbuf_get_rowstride(src));

	buf_pixel_set(gdk_pixbuf_get_pixels(dest), dx, dy, r, g, b, alpha * a >> 8,
		      gdk_pixbuf_get_has_alpha(dest), gdk_pixbuf_get_rowstride(dest));
}

void pixbuf_copy_line(GdkPixbuf *src, gint sx1, gint sy1, gint sx2, gint sy2,
		      GdkPixbuf *dest, gint dx, gint dy, gint use_alpha)
{
	guint8 r, g, b, a;
	guchar *s_pix, *d_pix;
	gint s_alpha, d_alpha;
	gint srs, drs;

	gint xd, yd;

	gfloat xstep, ystep;

	gfloat i, j;
	gint n, nt;

	if (sx1 == sx2 && sy1 == sy2);

	if (sx1 < 0 || sx1 >= gdk_pixbuf_get_width(src)) return;
	if (sy1 < 0 || sy1 >= gdk_pixbuf_get_height(src)) return;
	if (sx2 < 0 || sx2 >= gdk_pixbuf_get_width(src)) return;
	if (sy2 < 0 || sy2 >= gdk_pixbuf_get_height(src)) return;

	if (dx < 0 || dx >= gdk_pixbuf_get_width(dest)) return;
	if (dy < 0 || dy >= gdk_pixbuf_get_height(dest)) return;

	xd = sx2 - sx1;
	yd = sy2 - sy1;

	if (dx + xd < 0 || dx + xd >= gdk_pixbuf_get_width(dest)) return;
	if (dy + yd < 0 || dy + yd >= gdk_pixbuf_get_height(dest)) return;

	s_pix = gdk_pixbuf_get_pixels(src);
	srs = gdk_pixbuf_get_rowstride(src);
	s_alpha = gdk_pixbuf_get_has_alpha(src);

	d_pix = gdk_pixbuf_get_pixels(dest);
	drs = gdk_pixbuf_get_rowstride(dest);
	d_alpha = gdk_pixbuf_get_has_alpha(dest);

	nt = sqrt(xd * xd + yd * yd);
	xstep = (float)xd / nt;
	ystep = (float)yd / nt;

	i = j = 0.0;
	for(n = 0; n < nt; n++)
		{
		buf_pixel_get(s_pix, sx1 + (gint)j, sy1 + (gint)i, &r, &g, &b, &a, s_alpha, srs);
		buf_pixel_set(d_pix, dx + (gint)j, dy + (gint)i, r, g, b, use_alpha ? a : 255, d_alpha, drs);
		i += ystep;
		j += xstep;
		}
}

