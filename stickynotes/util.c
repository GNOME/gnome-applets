/* Post-It
 * Copyright (C) 2002-2003 Loban A Rahman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>
#include <time.h>
#include <util.h>

/* Returns the current date in a customizable form, the default
 * looks like this: "Nov 30, '78" */
gchar * get_current_date(const gchar *format)
{
  	time_t clock;
  	struct tm *current;	
	gint date_length;
	gchar *date;
	
  	clock = time(NULL);
  	current = localtime(&clock);

	date_length = 10;
  	date = g_new(gchar, date_length);
  	
	do
	{
		date_length += 5;
		date = (gchar *) g_renew(gchar, date, date_length);
	}
  	while(strftime(date, date_length, format, current) == 0);
	
  	return date;
}
