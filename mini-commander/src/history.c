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

/* Actually the command history is a simple list.  So, I guess this
   here could also be done with the list routines of glib. */

#include <string.h>
#include <config.h>
#include <gnome.h>

#include "history.h"
#include "preferences.h"
#include "message.h"

static char *history_command[LENGTH_HISTORY_LIST];
static void delete_history_entry(int element_number);


int
exists_history_entry(int pos)
{
    return(history_command[pos] != NULL);
}

char *
get_history_entry(int pos)
{
    return(history_command[pos]);
}

void
set_history_entry(int pos, char * entry)
{
    if(history_command[pos] != NULL)
	free(history_command[pos]);
    history_command[pos] = (char *)malloc(sizeof(char) * (strlen(entry) + 1));
    strcpy(history_command[pos], entry);
}

void
append_history_entry(char * entry)
{
    int pos;

    /* remove older dupes */
    for(pos = 0; pos <= LENGTH_HISTORY_LIST - 1; pos++)
	{
	    if(exists_history_entry(pos) && strcmp(entry, history_command[pos]) == 0)
		/* dupe found */
		delete_history_entry(pos);
	}

    /* delete oldest entry */
    if(history_command[0] != NULL)
	free(history_command[0]);

    /* move entries */
    for(pos = 0; pos < LENGTH_HISTORY_LIST - 1; pos++)
	{
	    history_command[pos] = history_command[pos+1];
	    /* printf("%s\n", history_command[pos]); */
	}

    /* append entry */
    history_command[LENGTH_HISTORY_LIST - 1] = (char *)malloc(sizeof(char) * (strlen(entry) + 1));
    strcpy(history_command[LENGTH_HISTORY_LIST - 1], entry);
}

void
delete_history_entry(int element_number)
{
    int pos;

    for(pos = element_number; pos > 0; --pos)
	history_command[pos] = history_command[pos - 1];

    history_command[0] = NULL;   
}
