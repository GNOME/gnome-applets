/*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <oliver@maruhn.com>
 *
 * Author: Oliver Maruhn <oliver@maruhn.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string.h>
#include <config.h>
#include <gnome.h>

#include "history.h"
#include "preferences.h"
#include "message.h"

static char *historyCommand[HISTORY_DEPTH];


int
existsHistoryEntry(int pos)
{
    return(historyCommand[pos] != NULL);
}

char *
getHistoryEntry(int pos)
{
    return(historyCommand[pos]);
}

void
setHistoryEntry(int pos, char * entry)
{
    free(historyCommand[pos]);
    historyCommand[pos] = (char *) malloc(sizeof(char) * (strlen(entry) + 1));
    strcpy(historyCommand[pos], entry);
}

void
appendHistoryEntry(char * entry)
{
    int pos;

    if(historyCommand[HISTORY_DEPTH - 1] == NULL
       || strcmp(entry, historyCommand[HISTORY_DEPTH - 1]) != 0)
	{
	    /* this command is no dupe -> update history */
	    free(historyCommand[0]);
	    for(pos = 0; pos < HISTORY_DEPTH - 1; pos++)
		{
		    historyCommand[pos] = historyCommand[pos+1];
		    /* printf("%s\n", historyCommand[pos]); */
		}
	    historyCommand[HISTORY_DEPTH - 1] = (char *) malloc(sizeof(char) * (strlen(entry) + 1));
	    strcpy(historyCommand[HISTORY_DEPTH - 1], entry);
	}
}

