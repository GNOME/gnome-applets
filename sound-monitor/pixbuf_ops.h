/* GNOME sound-monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#ifndef PIXBUF_OPS_H
#define PIXBUF_OPS_H


void pixbuf_copy_area(GdkPixbuf *src, gint sx, gint sy,
		      GdkPixbuf *dest, gint dx, gint dy,
		      gint w, gint h, gint alpha_too);

void pixbuf_copy_area_alpha(GdkPixbuf *src, gint sx, gint sy,
			    GdkPixbuf *dest, gint dx, gint dy,
			    gint w, gint h, gint alpha_modifier);

void pixbuf_copy_rotate_alpha(GdkPixbuf *src, gint offset_x, gint offset_y,
			      GdkPixbuf *dest, gint center_x, gint center_y,
			      double theta);

void pixbuf_draw_rect_fill(GdkPixbuf *pb,
			   gint x, gint y, gint w, gint h,
			   gint r, gint g, gint b, gint a);

void pixbuf_copy_point(GdkPixbuf *src, gint sx, gint sy,
		       GdkPixbuf *dest, gint dx, gint dy, gint use_alpha);

void pixbuf_copy_point_at_alpha(GdkPixbuf *src, gint sx, gint sy,
				GdkPixbuf *dest, gint dx, gint dy, gint alpha);

void pixbuf_copy_line(GdkPixbuf *src, gint sx1, gint sy1, gint sx2, gint sy2,
		      GdkPixbuf *dest, gint dx, gint dy, gint use_alpha);


#endif
