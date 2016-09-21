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

#ifndef INVEST_CACHE_H
#define INVEST_CACHE_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct
{
  gchar   *symbol;
  gchar   *name;
  gchar   *currency;
  gdouble  last_trade;
  gchar   *last_trade_date;
  gchar   *last_trade_time;
  gdouble  change;
  gdouble  open;
  gdouble  hight;
  gdouble  low;
  gint     volume;

  gdouble  change_pct;
} InvestQuote;

typedef struct
{
  InvestQuote **quotes;
  guint         n_quotes;

  GTimeVal      mtime;
} InvestCache;

InvestCache *invest_cache_read (const gchar *filename);

void         invest_cache_free (InvestCache *cache);

InvestQuote *invest_cache_get  (InvestCache *cache,
                                const gchar *symbol);

G_END_DECLS

#endif
