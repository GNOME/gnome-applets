/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef CLOCK_H
#define CLOCK_H


HandData *hand_new_from_data(gchar **data, gint xo, gint yo);
HandData *hand_new_from_file(gchar *file, gint xo, gint yo);
void hand_free(HandData *hd);
void hand_draw(HandData *hd, gint x, gint y, gint r_deg, GdkPixbuf *pb);

ClockData *clock_new_from_data(HandData *h, HandData *m, HandData *s,
			       gchar **data, gint x, gint y, gint width, gint height);
ClockData *clock_new_from_file(HandData *h, HandData *m, HandData *s,
			       gchar *file, gint x, gint y);
void clock_free(ClockData *cd);
void clock_draw(ClockData *cd, gint h, gint m, gint s, GdkPixbuf *pb);
void clock_back_set(ClockData *cd, GdkPixbuf *pb);


#endif

