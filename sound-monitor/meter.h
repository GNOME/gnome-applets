/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef METER_H
#define METER_H


MeterData *meter_new_from_data(gchar **data, gint sections, gint x, gint y, gint horizontal);
MeterData *meter_new_from_file(gchar *file, gint sections, gint x, gint y, gint horizontal);
void meter_free(MeterData *meter);
gint meter_draw(MeterData *meter, gint value, GdkPixbuf *pb);
void meter_mode_set(MeterData *meter, PeakModeType mode, gfloat falloff);
void meter_back_set(MeterData *meter, GdkPixbuf *pb);


#endif
