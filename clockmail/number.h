/* GNOME clock & mailcheck applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef NUMBER_H
#define NUMBER_H


DigitData *digit_new_from_data(gchar **data);
DigitData *digit_new_from_file(gchar *file);
void digit_free(DigitData *digit);

NumberData *number_new(DigitData *digit, gint length, gint zeros, gint centered, gint x, gint y);
void number_free(NumberData *number);
void number_draw(NumberData *number, gint n, GdkPixbuf *pb);
void number_back_set(NumberData *number, GdkPixbuf *pb);


#endif

