/* gstc.h - G(something) stacking cache
 * Copyright (C) 1999 Tim Janik
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __GSTC_H__
#define __GSTC_H__

#include        <gdk/gdk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* FIXME: this works around old glib versions (pre 1.2.2) */
#undef G_GNUC_FUNCTION
#undef G_GNUC_PRETTY_FUNCTION
#ifdef  __GNUC__
#define G_GNUC_FUNCTION         __FUNCTION__
#define G_GNUC_PRETTY_FUNCTION  __PRETTY_FUNCTION__
#else   /* !__GNUC__ */
#define G_GNUC_FUNCTION         ""
#define G_GNUC_PRETTY_FUNCTION  ""
#endif  /* !__GNUC__ */


#define	GSTC_PARENT(sparent)		((GstcParent*) (sparent))
#define	GSTC_PARENT_XWINDOW(sparent)	(GDK_WINDOW_XWINDOW (GSTC_PARENT (sparent)->window))

typedef struct _GstcParent GstcParent;
struct _GstcParent
{
  GdkWindow  *window;
  guint       n_children;
  gulong     *children;
  guint       ref_count;
};

GstcParent* gstc_parent_add_watch    (GdkWindow *window);
void        gstc_parent_delete_watch (GdkWindow *window);
GstcParent* gstc_parent_from_window  (GdkWindow *window);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GSTC_H__ */
