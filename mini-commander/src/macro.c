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

#include <sys/types.h>
#include <string.h>
#include <regex.h>
#include <config.h>
#include <gnome.h>

#include "macro.h"
#include "mini-commander_applet.h"
#include "preferences.h"
#include "message.h"


static int prefix_number(char *command);


/* search for the longest matching prefix */
static int
prefix_number(char *command)
{
    int i;
    int found_prefix_no = -1;
    unsigned int found_prefix_len = 0;

    for(i=0; i<=MAX_NUM_MACROS-1; i++)
	if (prop.macro_pattern[i] != NULL &&
 	    strlen(prop.macro_pattern[i]) > found_prefix_len &&
	    strncmp(command, prop.macro_pattern[i], strlen(prop.macro_pattern[i])) == 0 &&
	    (strstr(prop.macro_command[i], "$1") != NULL || strlen(prop.macro_pattern[i]) == strlen(command)))
	    {
		/* found a matching prefix;
		   if macro does not contain "$1" then the prefix has
		   to to have the same lenght as the command */
		found_prefix_no = i; found_prefix_len =
		strlen(prop.macro_pattern[i]); }

    return(found_prefix_no);
}

int
prefix_length(char *command)
{
    int pn = prefix_number(command);

    if(pn > -1)
	return strlen(prop.macro_pattern[pn]);

    /* no prefix found */
    return(0);
}

int
prefix_length_Including_whithespaces(char *command)
{
    char *c_ptr;

    c_ptr = command + prefix_length(command);

    while(*c_ptr != '\000' && *c_ptr == ' ')
	c_ptr++;

    return(c_ptr - command);
}

char *
get_prefix(char *command)
{
    int pn = prefix_number(command);

    if(pn > -1)
	return((char *) prop.macro_pattern[pn]);

    /* no prefix found */
    return((char *) NULL);
}


void
expand_command(char *command)
{
    char command_exec[1000] = "";
    char placeholder[100];
    int inside_placeholder;
    int parameter_number;
    char *macro;
    char *char_p;
    int i;
    regmatch_t regex_matches[MAX_NUM_MACRO_PARAMETERS];

    for(i = 0; i < MAX_NUM_MACROS - 1; ++i)
	if(prop.macro_regex[i] != NULL &&
	   regexec(prop.macro_regex[i], command, MAX_NUM_MACRO_PARAMETERS, regex_matches, 0) != REG_NOMATCH)
	    {
		/* found a matching macro regex pattern; */
		fprintf(stderr, "%u: %s\n", i, prop.macro_pattern[i]);

		macro = prop.macro_command[i];

		inside_placeholder = 0;
		for(char_p = macro; *char_p != '\0'; ++char_p)
		    {
			if(inside_placeholder == 0
			   && *char_p == '\\' 
			   && *(char_p + 1) >= '0'
			   && *(char_p + 1) <= '9')			    
			    {
				strcpy(placeholder, "");
				inside_placeholder = 1;
				/* fprintf(stderr, ">%c\n", *char_p); */
			    }
			else if(inside_placeholder
				&& (*(char_p + 1) < '0'
				    || *(char_p + 1) > '9'))
			    {
				inside_placeholder = 2;
				/* fprintf(stderr, "<%c\n", *char_p); */
			    }
/* 			else */
/* 			    fprintf(stderr, "|%c\n", *char_p); */
			
			if(inside_placeholder == 0)
			    strncat(command_exec, char_p, 1);
			else
			    strncat(placeholder, char_p, 1);

			if(inside_placeholder == 2)
			    {
				parameter_number = atoi(placeholder + 1);

				if(parameter_number <= MAX_NUM_MACRO_PARAMETERS
				   && regex_matches[parameter_number].rm_eo > 0)
				    strncat(command_exec,
					    command + regex_matches[parameter_number].rm_so,
					    regex_matches[parameter_number].rm_eo 
					    - regex_matches[parameter_number].rm_so);

				inside_placeholder = 0;
				/* fprintf(stderr, "placeholder: %s, %d\n", placeholder, parameter_number); */
			    }

		    }
		break;
	    }

    if(command_exec[0] == '\0')
	/* no macro found */
	strcpy(command_exec, command);

    /* fprintf(stderr, "command_exec: %s\n", command_exec); */
    
    strcpy(command, command_exec);
}
