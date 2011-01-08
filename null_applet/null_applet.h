/* -*- mode: C; c-basic-offset: 4 -*-
 * Null Applet - The Applet Deprecation Applet
 * Copyright (c) 2004, Davyd Madeley
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:
 *   Rodrigo Moya <rodrigo@gnome.org>
 */

#ifndef __NULL_APPLET_H
#define __NULL_APPLET_H

#include <panel-applet.h>

G_BEGIN_DECLS

#define TYPE_NULL_APPLET           (null_applet_get_type ())
#define NULL_APPLET(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_NULL_APPLET, NullApplet))
#define NULL_APPLET_CLASS(obj)     (G_TYPE_CHECK_CLASS_CAST    ((obj), TYPE_NULL_APPLET, NullAppletClass))
#define IS_NULL_APPLET(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_NULL_APPLET))
#define IS_NULL_APPLET_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE    ((obj), TYPE_NULL_APPLET))
#define NULL_APPLET_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS  ((obj), TYPE_NULL_APPLET, NullAppletClass))

typedef struct _NullApplet      NullApplet;
typedef struct _NullAppletClass NullAppletClass;

struct _NullApplet {
  PanelApplet parent;
};

struct _NullAppletClass {
  PanelAppletClass parent_class;
};

G_END_DECLS

#endif
