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

#ifndef INVEST_IMAGE_RETRIEVER_H
#define INVEST_IMAGE_RETRIEVER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define INVEST_TYPE_IMAGE_RETRIEVER invest_image_retriever_get_type ()
G_DECLARE_FINAL_TYPE (InvestImageRetriever, invest_image_retriever,
                      INVEST, IMAGE_RETRIEVER, GObject)

InvestImageRetriever *invest_image_retriever_new     (const gchar          *url);

void                  invest_image_retriever_start   (InvestImageRetriever *retriever);

const gchar          *invest_image_retriever_get_url (InvestImageRetriever *retriever);

G_END_DECLS

#endif
