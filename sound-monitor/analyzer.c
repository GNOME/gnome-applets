/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "sound-monitor.h"
#include "analyzer.h"
#include "pixbuf_ops.h"

#include "fft.h"

#define DEBUG

/* these are 1000's of a second, the time to fall from full to 0 */
#define ANALYZER_DEFAULT_BAND_DECAY 1000
#define ANALYZER_DEFAULT_PEAK_DECAY 4000

/*
 *----------------------------------------------------------------------------
 * Analyzer
 *----------------------------------------------------------------------------
 */

static void analyzer_clear_bands(AnalyzerData *analyzer);


static AnalyzerData *analyzer_new(GdkPixbuf *pb, gint x, gint y, gint horizontal,
				  gint bands, gint rows)
{
	AnalyzerData *analyzer;

	if (!pb) return NULL;

	analyzer = g_new0(AnalyzerData, 1);

	analyzer->overlay = pb;
	analyzer->x = x;
	analyzer->y = y;
	analyzer->width = gdk_pixbuf_get_width(pb);
        analyzer->height = gdk_pixbuf_get_height(pb) / 2;

	analyzer->band_count = bands;
	analyzer->bands = g_malloc(sizeof(float) * analyzer->band_count);
	analyzer->peaks = g_malloc(sizeof(float) * analyzer->band_count);

	analyzer->rows = rows;

	analyzer->led_width = analyzer->width / analyzer->band_count;
	analyzer->led_height = analyzer->height / analyzer->rows;

	analyzer->horizontal = horizontal;

	analyzer->flip = FALSE;
	analyzer->decay_speed = ANALYZER_DEFAULT_BAND_DECAY;

	analyzer->peak_enable = FALSE;
	analyzer->peak_decay_speed = ANALYZER_DEFAULT_PEAK_DECAY;

	analyzer_clear_bands(analyzer);

	analyzer->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, analyzer->width, analyzer->height);

	return analyzer;
}

AnalyzerData *anaylzer_new_from_data(gchar **data, gint x, gint y, gint horizontal,
				     gint bands, gint rows)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_xpm_data((const gchar **)data);

	return analyzer_new(pb, x, y, horizontal, bands, rows);
}

AnalyzerData *anaylzer_new_from_file(gchar *file, gint x, gint y, gint horizontal,
				     gint bands, gint rows)
{
	GdkPixbuf *pb;

	pb = gdk_pixbuf_new_from_file(file);

	return analyzer_new(pb, x, y, horizontal, bands, rows);
}

void analyzer_free(AnalyzerData *analyzer)
{
	if (!analyzer) return;
	if (analyzer->pixbuf) gdk_pixbuf_unref(analyzer->pixbuf);
	if (analyzer->overlay) gdk_pixbuf_unref(analyzer->overlay);
	g_free(analyzer->bands);
	g_free(analyzer->peaks);
	g_free(analyzer);
}

void analyzer_set_options(AnalyzerData *analyzer, gint decay, gint peak_enable, gint peak_decay, gint flip)
{
	if (!analyzer) return;

	analyzer->decay_speed = decay;
	analyzer->peak_enable = peak_enable;
	analyzer->peak_decay_speed = peak_decay;
	analyzer->flip = flip;

	/* sanity checks */
	if (analyzer->decay_speed < 10) analyzer->decay_speed = 10;
	if (analyzer->peak_decay_speed < 10) analyzer->peak_decay_speed = 10;
}

static void analyzer_draw_layer(AnalyzerData *analyzer, GdkPixbuf *pb, float *layer, gint offset, gint fill)
{
	gint i;

	for (i = 0; i < analyzer->band_count; i++)
		{
		gint x, y, w, h;

		x = i * analyzer->led_width;
		y = analyzer->height - (layer[i] * analyzer->rows * analyzer->led_height);
		w = analyzer->led_width;
		if (fill)
			{
			h = analyzer->height - y;
			}
		else if (y == analyzer->height)
			{
			h = 0;
			}
		else
			{
			h = analyzer->led_height;
			}
		if (h) pixbuf_copy_area_alpha(analyzer->overlay, x, offset * analyzer->height + y,
					      pb, analyzer->x + x, analyzer->y + y, w, h, 255);
		}
}

void analyzer_draw(AnalyzerData *analyzer, GdkPixbuf *pb)
{
	if (!analyzer) return;

	/* FIXME! flip and horizontal not implemented yet! */

	pixbuf_copy_area(analyzer->pixbuf, 0, 0, pb, analyzer->x, analyzer->y, analyzer->width, analyzer->height, FALSE);

	analyzer_draw_layer(analyzer, pb, analyzer->bands, 0, TRUE);
	if (analyzer->peak_enable) analyzer_draw_layer(analyzer, pb, analyzer->peaks, 1, FALSE);
}

/*
 * if band_data is NULL, decay anyway since it might be during silence and we want
 * the leds to eventually fall to zero
 */
void analyzer_update(AnalyzerData *analyzer, GdkPixbuf *pb, float *band_data, gint fps)
{
	gint i;
	float band_decay;
	float peak_decay;

	if (!analyzer) return;

	band_decay = (float)fps / analyzer->decay_speed;
	peak_decay = (float)fps / analyzer->peak_decay_speed;

	for (i = 0; i < analyzer->band_count; i++)
		{
		if (band_data && band_data[i] >= analyzer->bands[i] - band_decay)
			{
			analyzer->bands[i] = band_data[i];
			}
		else
			{
			analyzer->bands[i] -= band_decay;
			}

		if (band_data && band_data[i] >= analyzer->peaks[i] - peak_decay)
			{
			analyzer->peaks[i] = band_data[i];
			}
		else
			{
			analyzer->peaks[i] -= peak_decay;
			}

		/* stay >= 0 */
		if (analyzer->bands[i] < 0.0) analyzer->bands[i] = 0.0;
		if (analyzer->peaks[i] < 0.0) analyzer->peaks[i] = 0.0;
		}

	analyzer_draw(analyzer, pb);
}

static void fft_to_percentage(short *dataT, long array_length, float *response, int bands)
{
	gint c;
	gfloat i;
	gfloat min = 200.0;
	gfloat max = 12000.0;
	gfloat inc;

	fft(dataT, array_length, response, bands);

	/* now make each band be a percentage value (0.0 to 1.0) */

	c = 0;
	inc = (max - min) / bands;

	for (i = min; i < max; i += inc)
		{
#ifdef DEBUG
		printf("[%d %f] ", c, (response[c] + response[c + 1]) / 2.0);
#endif

		response[c] = (response[c] - 10.0) / 10.0;	/* left */
		c++;
		response[c] = (response[c] - 10.0) / 10.0;	/* right */
		c++;
		}
}

void analyzer_update_w_fft(AnalyzerData *analyzer, GdkPixbuf *pb, short *buffer, gint buffer_length, gint fps)
{
	float *band_data;
	float *fft_data;
	gint i;

	if (!analyzer) return;

	fft_data = g_malloc(sizeof(float) * analyzer->band_count * 2);
	band_data = g_malloc(sizeof(float) * analyzer->band_count);

	fft_to_percentage(buffer, (long)buffer_length, fft_data, analyzer->band_count);

	for (i = 0; i < analyzer->band_count; i++)
		{
		band_data[i] = (fft_data[i * 2] + fft_data[i * 2 + 1]) / 2.0;
#ifdef DEBUG
		printf("[%d %f] ", i, band_data[i]);
#endif
		}
#ifdef DEBUG
		printf("\n");
#endif
	
	analyzer_update(analyzer, pb, band_data, fps);

	g_free(fft_data);
	g_free(band_data);
}

static void analyzer_clear_bands(AnalyzerData *analyzer)
{
	gint i;

	for (i = 0; i < analyzer->band_count; i++)
		{
		analyzer->bands[i] = 0.0;
		analyzer->peaks[i] = 0.0;
		}
}

void analyzer_clear(AnalyzerData *analyzer, GdkPixbuf *pb)
{
	if (!analyzer) return;

	analyzer_clear_bands(analyzer);

	analyzer_draw(analyzer, pb);
}

void analyzer_back_set(AnalyzerData *analyzer, GdkPixbuf *pb)
{
	if (!analyzer) return;

	pixbuf_copy_area(pb, analyzer->x, analyzer->y,
			 analyzer->pixbuf, 0, 0, analyzer->width, analyzer->height, FALSE);
}


