/* generic rotation drawing for Gdk Images and Pixmaps
 *
 * These functions allow drawing of pixmaps and images onto another with
 * rotation. The function does anti-aliasing.
 * Rotation is specified in degrees as an integer from 0 - 360.
 *
 * Author: John Ellis
 *
 * The private functions were adapted from the Afterstep Clock (asclock),
 * by Beat Christen.
 * They were in rot.c. The static variables were removed and the functions
 * made more generic.
 *
 * xp and yp are the center point of rotation
 *
 * xo and yo is the position in the source image that is the center axis, it
 * must be within the image.
 */

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

/* This gets a map for use as a mask with draw_image_rotated().
 * returns NULL if no Image.
 */
char *get_map(GdkImage *im);

/* This draws a GdkImage onto another, map is the mask (use above)
 * map can be NULL for no mask.
 */
void draw_image_rotated(GdkImage *dest, GdkImage *src, char *map,
			 gint xp, gint yp, gint xo, gint yo, gint r_deg,
			 GdkWindow *window);

/* This draws a GdkPixmap onto another, mask can be NULL
 */
void draw_pixmap_rotated(GdkPixmap *dest, GdkPixmap *src, GdkBitmap *mask,
			 gint xp, gint yp, gint xo, gint yo, gint r_deg,
			 GtkWidget *widget);


