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

#include "macro.h"
#include "mini-commander_applet.h"
#include "preferences.h"
#include "message.h"


static int prefixNumber(char *command);


/* search for the longest matching prefix */
static int
prefixNumber(char *command)
{
    int i;
    int found_prefix_no = -1;
    unsigned int found_prefix_len = 0;

    for(i=0; i<=MAX_PREFIXES-1; i++)
	if (prop.prefix[i] != (char *) NULL &&
 	    strlen(prop.prefix[i]) > found_prefix_len &&
	    strncmp(command, prop.prefix[i], strlen(prop.prefix[i])) == 0 &&
	    (strstr(prop.command[i], "$1") != NULL || strlen(prop.prefix[i]) == strlen(command)))
	    {
		/* found a matching prefix;
		   if macro does not contain "$1" then the prefix has
		   to to have the same lenght as the command */
		found_prefix_no = i; found_prefix_len =
		strlen(prop.prefix[i]); }

    return(found_prefix_no);
}

int
prefixLength(char *command)
{
    int pn = prefixNumber(command);

    if(pn > -1)
	return strlen(prop.prefix[pn]);

    /* no prefix found */
    return(0);
}

int
prefixLength_IncludingWhithespaces(char *command)
{
    char *cPtr;

    cPtr = command + prefixLength(command);

    while(*cPtr != '\000' && *cPtr == ' ')
	cPtr++;

    return(cPtr - command);
}

char *
getPrefix(char *command)
{
    int pn = prefixNumber(command);

    if(pn > -1)
	return((char *) prop.prefix[pn]);

    /* no prefix found */
    return((char *) NULL);
}


void
expandCommand(char *command)
{
    /* FIXME: code cleanup needed */
    int pn = prefixNumber(command);
    int prefixLen = prefixLength_IncludingWhithespaces(command);
    char *prefix = (pn > -1) ? prop.prefix[pn] : (char *) NULL;
    char *macro = (pn > -1) ? prop.command[pn] : (char *) NULL;
    char commandExec[1000];
    char buffer[1000];
    char *substPtr;
    int j;

    if(prefix == NULL)
	return;

    /* there is a prefix */
    strcpy(buffer, macro);
    if ((substPtr = strstr(buffer, "$1")) != (char *) NULL)
	{
	    /* "$1" found */
	    *substPtr = '\000';
		    strcpy(commandExec, buffer);
		    strcat(commandExec, command + prefixLen);
		    strcat(commandExec, substPtr + 2);
	}
    else
	{
	    /* no $1 in this macro */
	    strcpy(commandExec, macro); 
	}

    for(j=0; j<10; j++)
	{
	    /* substitute up to ten $1's */
	    strcpy(buffer, commandExec);
	    if ((substPtr = strstr(buffer, "$1")) != (char *) NULL)
		{
		    /* "$1" found */
		    *substPtr = '\000';
		    strcpy(commandExec, buffer);
		    strcat(commandExec, command + prefixLen);
		    strcat(commandExec, substPtr + 2);
		}
	}
    
    strcpy(command, commandExec);
}
