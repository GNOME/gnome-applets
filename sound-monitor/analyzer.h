/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef ANALYZER_H
#define ANALYZER_H

AnalyzerData *anaylzer_new_from_data(gchar **data, gint x, gint y, gint horizontal,
				     gint bands, gint rows);
AnalyzerData *anaylzer_new_from_file(gchar *file, gint x, gint y, gint horizontal,
				     gint bands, gint rows);

void analyzer_free(AnalyzerData *analyzer);
void analyzer_set_options(AnalyzerData *analyzer, gint decay, gint peak_enable, gint peak_decay, gint flip);

void analyzer_draw(AnalyzerData *analyzer, GdkPixbuf *pb);

void analyzer_update(AnalyzerData *analyzer, GdkPixbuf *pb, float *band_data, gint fps);
void analyzer_update_w_fft(AnalyzerData *analyzer, GdkPixbuf *pb, short *buffer, gint buffer_length, gint fps);

void analyzer_clear(AnalyzerData *analyzer, GdkPixbuf *pb);
void analyzer_back_set(AnalyzerData *analyzer, GdkPixbuf *pb);


#endif

