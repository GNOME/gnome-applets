/*
 * Copyright 2002 Sun Microsystems Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef __ACCESSIBLE_DRAWING_AREA_FACTORY_H__
#define __ACCESSIBLE_DRAWING_AREA_FACTORY_H__

#include <atk/atkobjectfactory.h>
#include "customdrawingarea.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ACCESSIBLE_TYPE_DRAWING_AREA_FACTORY             (accessible_drawing_area_factory_get_type())
#define ACCESSIBLE_DRAWING_AREA_FACTORY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ACCESSIBLE_TYPE_DRAWING_AREA_FACTORY, AccessibleDrawingAreaFactory))
#define ACCESSIBLE_DRAWING_AREA_FACTORY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), ACCESSIBLE_TYPE_DRAWING_AREA_FACTORY, AccessibleDrawingAreaFactoryClass))
#define ACCESSIBLE_IS_DRAWING_AREA_FACTORY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ACCESSIBLE_TYPE_DRAWING_AREA_FACTORY))
#define ACCESSIBLE_IS_DRAWING_AREA_FACTORY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), ACCESSIBLE_TYPE_DRAWING_AREA_FACTORY))
#define ACCESSIBLE_DRAWING_AREA_FACTORY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), ACCESSIBLE_TYPE_DRAWING_AREA_FACTORY, AccessibleDrawingAreaFactoryClass))

typedef struct _AccessibleDrawingAreaFactory       AccessibleDrawingAreaFactory;
typedef struct _AccessibleDrawingAreaFactoryClass  AccessibleDrawingAreaFactoryClass;

struct _AccessibleDrawingAreaFactory
{
	AtkObjectFactory parent;
};

struct _AccessibleDrawingAreaFactoryClass
{
	AtkObjectFactoryClass parent_class;
};

GType accessible_drawing_area_factory_get_type (void);
AtkObjectFactory *accessible_drawing_area_factory_new (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ACCESSIBLE_DRAWING_AREA_FACTORY_H__ */
