/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "sound-monitor.h"
#include "scope.h"
#include "pixbuf_ops.h"

static void scope_clear(ScopeData *item);

/*
 *----------------------------------------------------------------------------
 * Scope
 *----------------------------------------------------------------------------
 */

static ScopeData *scope_new(GdkPixbuf *pb, gint x, gint y, gint horizontal)
{
	ScopeData *scope;

	if (!pb) return NULL;

	scope = g_new0(ScopeData, 1);

	scope->overlay = pb;
	scope->width = gdk_pixbuf_get_width(pb);
	scope->height = gdk_pixbuf_get_height(pb) / 2;

	scope->x = x;
	scope->y = y;
	scope->horizontal = horizontal;

	if (scope->horizontal)
		scope->pts_len = scope->height;
	else
		scope->pts_len = scope->width;

	scope->pts = g_malloc(sizeof(gint) * (scope->pts_len + 4));

	scope_clear(scope);

	scope->type = ScopeType_Points;

	scope->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, scope->width, gdk_pixbuf_get_height(scope->overlay));

	return scope;
}

ScopeData *scope_new_from_data(gchar **data, gint x, gint y, gint horizontal)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_xpm_data((const gchar **)data);

	return scope_new(pb, x, y, horizontal);
}

ScopeData *scope_new_from_file(gchar *file, gint x, gint y, gint horizontal)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_file(file);

	return scope_new(pb, x, y, horizontal);
}

void scope_free(ScopeData *scope)
{
	if (!scope) return;
	if (scope->pixbuf) gdk_pixbuf_unref(scope->pixbuf);
	if (scope->overlay) gdk_pixbuf_unref(scope->overlay);
	g_free(scope->pts);
	g_free(scope);
}

void scope_scale_set(ScopeData *scope, gint scale)
{
	if (!scope) return;

	scope->scale = scale;
}

void scope_type_set(ScopeData *scope, ScopeType type)
{
	if (!scope) return;

	scope->type = type;
}

static void scope_clear(ScopeData *scope)
{
	gint i, t;

	if (scope->horizontal)
		t = scope->height;
	else
		t = scope->width;

	for (i = 0; i < t; i++) scope->pts[i] = 0;
}

static void scope_fill_data(ScopeData *scope, short *buffer, gint buffer_length)
{
	gint i, p;
	gint scale;
	gint len, step;
	gfloat fact;

	if (buffer_length / 2 >= scope->scale * scope->pts_len)
		{
		scale = scope->scale;
		}
	else
		{
		scale = buffer_length / 2 / scope->pts_len;
		}

	step = scale * 2;
	len = scope->pts_len * step;

	if (scope->horizontal)
		{
		fact = (gfloat)65536 * scale / scope->width;
		}
	else
		{
		fact = (gfloat)65536 * scale / scope->height;
		}

	i = p = 0;
	while(i < len)
		{
		gint k;
		scope->pts[p] = 0;
		for (k = 0; k < step; k += 2)
			{
			scope->pts[p] += (buffer[i + k] + buffer[i + k + 1]) / 2;
			}
		scope->pts[p] = (gfloat)scope->pts[p] / fact;
		p++;
		i += step;
		}
}

void scope_draw(ScopeData *scope, GdkPixbuf *pb, gint flat, short *buffer, gint buffer_length)
{
	gint i;

	if (!scope) return;

	if (flat)
		{
		scope_clear(scope);
		}
	else
		{
		scope_fill_data(scope, buffer, buffer_length);
		}

	pixbuf_copy_area(scope->pixbuf, 0, 0, pb, scope->x, scope->y, scope->width, scope->height, FALSE);

	if (scope->type == ScopeType_Lines)
		{
		if (scope->horizontal)
			{
			for (i = 0; i < scope->height - 1; i++)
				pixbuf_copy_line(scope->pixbuf, (scope->width / 2) - scope->pts[i], scope->height + i,
						 (scope->width / 2) - scope->pts[i+1], scope->height + i+1,
						 pb, scope->x + (scope->width / 2) - scope->pts[i], scope->y + i, FALSE);
			}
		else
			{
			for (i = 0; i < scope->width - 1; i++)
				pixbuf_copy_line(scope->pixbuf, i, scope->height + (scope->height / 2) - scope->pts[i],
						 i+1, scope->height + (scope->height / 2) - scope->pts[i+1],
						 pb, scope->x + i, scope->y + (scope->height / 2) - scope->pts[i], FALSE);
			}
		}
	else
		{
		if (scope->horizontal)
			{
			for (i = 0; i < scope->height; i++)
				pixbuf_copy_point(scope->pixbuf, (scope->width / 2) + scope->pts[i], scope->height + i,
						  pb, scope->x + (scope->width / 2) + scope->pts[i], scope->y + i, FALSE);
			}
		else
			{
			for (i = 0; i < scope->width; i++)
				pixbuf_copy_point(scope->pixbuf, i, scope->height + (scope->height / 2) - scope->pts[i],
						  pb, scope->x + i, scope->y + (scope->height / 2) - scope->pts[i], FALSE);
			}
		}
}

void scope_back_set(ScopeData *scope, GdkPixbuf *pb)
{
	gint i;

	if (!scope) return;

	for (i = 0; i < 2; i++)
		{
		pixbuf_copy_area(pb, scope->x, scope->y, scope->pixbuf,
				 0, i * scope->height, scope->width, scope->height, FALSE);
		}

	pixbuf_copy_area_alpha(scope->overlay, 0, 0,
                               scope->pixbuf, 0, 0, scope->width, gdk_pixbuf_get_height(scope->overlay), 255);
}



