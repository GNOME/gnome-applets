/*
 * Copyright (C) 2004-2005 Raphael Slinckx
 * Copyright (C) 2009-2011 Enrico Minack
 * Copyright (C) 2016 Alberts Muktupāvels
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *     Alberts Muktupāvels <alberts.muktupavels@gmail.com>
 *     Enrico Minack <enrico-minack@gmx.de>
 *     Raphael Slinckx <raphael@slinckx.net>
 */

#ifndef INVEST_APPLET_H
#define INVEST_APPLET_H

#include <panel-applet.h>

G_BEGIN_DECLS

#define INVEST_TYPE_APPLET         (invest_applet_get_type ())
#define INVEST_APPLET(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), INVEST_TYPE_APPLET, InvestApplet))
#define INVEST_APPLET_CLASS(c)     (G_TYPE_CHECK_CLASS_CAST ((c),    INVEST_TYPE_APPLET, InvestAppletClass))
#define INVEST_IS_APPLET(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), INVEST_TYPE_APPLET))
#define INVEST_IS_APPLET_CLASS(c)  (G_TYPE_CHECK_CLASS_TYPE ((c),    INVEST_TYPE_APPLET))
#define INVEST_APPLET_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o),   INVEST_TYPE_APPLET, InvestAppletClass))

typedef struct _InvestApplet      InvestApplet;
typedef struct _InvestAppletClass InvestAppletClass;

GType invest_applet_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
