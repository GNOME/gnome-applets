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

#ifndef INVEST_QUOTES_RETRIEVER_H
#define INVEST_QUOTES_RETRIEVER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define INVEST_TYPE_QUOTES_RETRIEVER invest_quotes_retriever_get_type ()
G_DECLARE_FINAL_TYPE (InvestQuotesRetriever, invest_quotes_retriever,
                      INVEST, QUOTES_RETRIEVER, GObject)

InvestQuotesRetriever *invest_quotes_retriever_new        (const gchar           *symbol,
                                                           const gchar           *filename);

void                   invest_quotes_retriever_start      (InvestQuotesRetriever *retriever);

const gchar           *invest_quotes_retriever_get_symbol (InvestQuotesRetriever *retriever);

G_END_DECLS

#endif
