/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <math.h>

#include "clockmail.h"
#include "pixbuf_ops.h"


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
 *---------------------------------------------------------------------------
 * The rotation code was borrowed from the afterstep clock (asclock), rot.c
 * and it has been adapted a bit.
 * Update 3/11/2000: Updated to work with GdkPixbuf buffers, and includes
 * source alpha channel support. Starting to look nothing like original code.
 * Only the pixel x, y location lines have remained the same.
 * Update 5/15/2000: Remove all division.. now use bitshifts, optimizations,
 * the with_clipping version no longer resembles the original at all.
 *
 * Still slower than it should be, IMHO.
 *---------------------------------------------------------------------------
 */

#define PI 3.14159

/*
 * Copies src to dest rotated, applying source alpha,
 * rotation is in degrees.
 * this version only works well if the source's center offset pixel is not
 * transparent (alpha value = 0) and is a basic shape.
 */
void pixbuf_copy_rotate_alpha(GdkPixbuf *src, gint offset_x, gint offset_y,
			      GdkPixbuf *dest, gint center_x, gint center_y,
			      double theta)
{
	guchar *s_buf;
	gint s_w, s_h;
	gint s_alpha, s_rs;
	gint s_step;

	guchar *d_buf;
	gint d_w, d_h;
	gint d_alpha, d_rs;

	int done, miss, missed=0;
	int goup;
	int goleft;

	int x, y;
	double xoff, yoff;

	s_buf = gdk_pixbuf_get_pixels(src);
	s_alpha = gdk_pixbuf_get_has_alpha(src);
	s_rs = gdk_pixbuf_get_rowstride(src);
	s_w = gdk_pixbuf_get_width(src);
	s_h = gdk_pixbuf_get_height(src);
	s_step = s_alpha ? 4 : 3;

	d_buf = gdk_pixbuf_get_pixels(dest);
	d_alpha = gdk_pixbuf_get_has_alpha(dest);
	d_rs = gdk_pixbuf_get_rowstride(dest);
	d_w = gdk_pixbuf_get_width(dest);
	d_h = gdk_pixbuf_get_height(dest);

	theta = - theta * PI / 180.0;
  
	xoff = center_x;
	yoff = center_y;

	done = FALSE;
	miss = FALSE;

	goup=TRUE;
	goleft=TRUE;
	x = center_x;
	y = center_y;
	while(!done) 
		{
		double ex, ey;
		int oldx, oldy;

		/* transform into old coordinate system */
		ex = offset_x-0.25 + ((x-xoff) * cos(theta) - (y-yoff) * sin(theta));
		ey = offset_y-0.25 + ((x-xoff) * sin(theta) + (y-yoff) * cos(theta));
		oldx = floor(ex);
		oldy = floor(ey);

		if(oldx >= -1 && oldx < s_w && oldy >= -1 && oldy < s_h)
			{
			if(x >= 0 && y >= 0 && x < d_w && y < d_h)
				{
				guchar tr, tg, tb, ta;
				guchar ag, bg, cg, dg;
				guchar *pp;

				/* calculate the strength of the according pixels */
				dg = (guchar)((ex - oldx) * (ey - oldy) * 255.0);
				cg = (guchar)((oldx + 1 - ex) * (ey - oldy) * 255.0);
				bg = (guchar)((ex - oldx) * (oldy + 1 - ey) * 255.0);
				ag = (guchar)((oldx + 1 - ex) * (oldy + 1 - ey) * 255.0);
	  
				tr = 0;
				tg = 0;
				tb = 0;
				ta = 0;

				pp = s_buf + (oldy * s_rs) + (oldx * s_step);

				if(oldx >= 0 && oldy >= 0)
					{
					guchar *pw;
					guchar a;

					pw = pp;
					a = s_alpha ? *(pw + 3) : 255;
					tr += (ag * *pw * a >> 16);
					pw++;
					tg += (ag * *pw * a >> 16);
					pw++;
					tb += (ag * *pw * a >> 16);
					ta += (ag * a >> 8);
					}
				if(oldx + 1 < s_w && oldy >= 0)
					{
					guchar *pw;
					guchar a;

					pw = pp + s_step;
					a = s_alpha ? *(pw + 3) : 255;
					tr += (bg * *pw * a >> 16);
					pw++;
					tg += (bg * *pw * a >> 16);
					pw++;
					tb += (bg * *pw * a >> 16);
					ta += (bg * a >> 8);
					}
				if(oldx >= 0 && oldy + 1 < s_h)
					{
					guchar *pw;
					guchar a;

					pw = pp + s_rs;
					a = s_alpha ? *(pw + 3) : 255;
					tr += (cg * *pw * a >> 16);
					pw++;
					tg += (cg * *pw * a >> 16);
					pw++;
					tb += (cg * *pw * a >> 16);
					ta += (cg * a >> 8);
					}
				if(oldx + 1 < s_w && oldy + 1 < s_h)
					{
					guchar *pw;
					guchar a;

					pw = pp + s_rs + s_step;
					a = s_alpha ? *(pw + 3) : 255;
					tr += (dg * *pw * a >> 16);
					pw++;
					tg += (dg * *pw * a >> 16);
					pw++;
					tb += (dg * *pw * a >> 16);
					ta += (dg * a >> 8);
					}
				buf_pixel_set(d_buf, x, y, tr, tg, tb, ta, d_alpha, d_rs);
				}
			missed = 0;
			miss = FALSE;
			}
		else
			{
			miss = TRUE;
			missed++;
			if(missed >= 15)
				{
				if(goup)
					{
					goup=FALSE;
					goleft=FALSE;
					x = center_x;
					y = center_y;
					}
				else
					{
					done = TRUE;
					}
				}
			}
		if(miss && (missed == 1))
			{
			goleft = !goleft;
			if(goup)
				y--;
			else
				y++;

			}
		else
			{
			if(goleft)
				x--;
			else
				x++;
			}
		}
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

