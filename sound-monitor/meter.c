/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */


#include "sound-monitor.h"
#include "meter.h"
#include "pixbuf_ops.h"

static MeterData *meter_new(GdkPixbuf *pb, gint sections, gint x, gint y, gint horizontal)
{
	MeterData *meter;

	if (!pb) return NULL;

	meter = g_new0(MeterData, 1);

	meter->overlay = pb;
	meter->sections = sections;
	meter->horizontal = horizontal;
	if (horizontal)
		{
		meter->width = gdk_pixbuf_get_width(pb) / 3;
		meter->height = gdk_pixbuf_get_height(pb);
		meter->section_height = meter->width / sections;
		}
	else
		{
		meter->width = gdk_pixbuf_get_width(pb);
		meter->height = gdk_pixbuf_get_height(pb) / 3;
		meter->section_height = meter->height / sections;
		}
	meter->x = x;
	meter->y = y;

	meter->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, meter->width, gdk_pixbuf_get_height(meter->overlay));

	meter->peak = 100.0;
	meter->old_peak = (float)sections;
	meter->value = 0;
	meter->old_val = 0;
	meter->peak_fall = 1.0;

	return meter;
}

MeterData *meter_new_from_data(gchar **data, gint sections, gint x, gint y, gint horizontal)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_xpm_data((const gchar **)data);

	return meter_new(pb, sections, x, y, horizontal);
}

MeterData *meter_new_from_file(gchar *file, gint sections, gint x, gint y, gint horizontal)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_file(file);

	return meter_new(pb, sections, x, y, horizontal);
}

void meter_free(MeterData *meter)
{
	if (!meter) return;
	if (meter->pixbuf) gdk_pixbuf_unref(meter->pixbuf);
	if (meter->overlay) gdk_pixbuf_unref(meter->overlay);
	g_free(meter);
}

gint meter_draw(MeterData *meter, gint value, GdkPixbuf *pb)
{
	gint new_val;
	gfloat new_peak;

	gint old_val;
	gfloat old_peak;

	if (!meter) return FALSE;

	old_val = meter->value;
	old_peak = meter->peak;

	meter->value = value;

	if (meter->mode == PeakMode_Off)
		{
		meter->peak = 0.0;
		}
	else if (meter->mode == PeakMode_Active)
		{
		if ((float)meter->value >= meter->peak)
			meter->peak = (float)meter->value;
		else
			{
			meter->peak -= meter->peak_fall;
			if (meter->peak < (float)meter->value) meter->peak = (float)meter->value;
			}
		}
	else /* assume now that (meter->mode == PeakMode_Smooth) */
		{
		if ((float)meter->value > meter->peak)
			{
			meter->peak += meter->peak_fall;
			if (meter->peak > (float)meter->value) meter->peak = (float)meter->value;
			}
		else if ((float)meter->value < meter->peak)
			{
			meter->peak -= meter->peak_fall;
			if (meter->peak < (float)meter->value) meter->peak = (float)meter->value;
			}
		}

	new_val = (float)meter->value / 100.0 * meter->sections;
	if (new_val > meter->sections) new_val = meter->sections;

	if (meter->peak > 0.0)
		{
		new_peak = meter->peak / 100.0 * (float)meter->sections;
		if (new_peak > (float)meter->sections) new_peak = (float)meter->sections;
		}
	else
		{
		new_peak = 0.0;
		}

	/* now to the drawing */

	if (new_val != meter->old_val || new_peak != meter->old_peak)
		{
		pixbuf_copy_area(meter->pixbuf, 0, 0,
				 pb, meter->x, meter->y, meter->width, meter->height, FALSE);

		if (meter->horizontal)
			{
			pixbuf_copy_area(meter->pixbuf, meter->width, 0,
					 pb, meter->x, meter->y, meter->section_height * new_val, meter->height, FALSE);

			if (new_peak > 0.0)
				pixbuf_copy_area_alpha(meter->overlay, (meter->width * 2) + meter->section_height * (gint)new_peak, 0,
						       pb, meter->x + (meter->section_height * (gint)new_peak), meter->y,
						       meter->section_height, meter->height, 255);
			}
		else
			{
			pixbuf_copy_area(meter->pixbuf, 0, (meter->height * 2) - (meter->section_height * new_val),
					 pb, meter->x, meter->y + ((meter->sections - new_val) * meter->section_height),
					 meter->width, meter->section_height * new_val, FALSE);

			if (new_peak > 0)
				pixbuf_copy_area_alpha(meter->overlay, 0, (meter->height * 3) - (meter->section_height * (gint)new_peak),
						       pb, meter->x, meter->y + ((meter->sections - (gint)new_peak) * meter->section_height),
						       meter->width, meter->section_height, 255);
			}

		meter->old_val = new_val;
		meter->old_peak = new_peak;
		return TRUE;
		}

	return FALSE;
}

void meter_mode_set(MeterData *meter, PeakModeType mode, gfloat falloff)
{
	if (!meter) return;
	meter->mode = mode;
	meter->peak_fall = falloff;
}

void meter_back_set(MeterData *meter, GdkPixbuf *pb)
{
	gint i;

	if (!meter) return;

	for (i = 0; i < 2; i++)
		{
		pixbuf_copy_area(pb, meter->x, meter->y, meter->pixbuf,
				 0, i * meter->height, meter->width, meter->height, FALSE);
		}

	pixbuf_copy_area_alpha(meter->overlay, 0, 0,
			       meter->pixbuf, 0, 0, meter->width, gdk_pixbuf_get_height(meter->overlay), 255);
}


