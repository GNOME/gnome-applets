/*
 * Mini-Commander Applet
 * Copyright (C) 1998 Oliver Maruhn <om@linuxhq.com>
 *
 * Author: Oliver Maruhn <om@linuxhq.com>
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

/*
  If you expect tons of C code for command completion then you will
  probably be astonished...

  These routines have probably to be rewritten in future. But they
  should work on every system work a bash-alike shell.  
*/

#include <string.h>
#include <config.h>
#include <gnome.h>
#include <applet-widget.h>

#include "preferences.h"
#include "message.h"

static char shellScript[] = 
"
for dir in `echo $PATH|sed \"s/^:/. /; s/:$/ ./; s/::/ . /g; s/:/ /g\"`
do
   for file in $dir/$cmd*
   do
      if test -x $file -a ! -d $file
      then
         echo `basename $file`
      fi
   done
done
";

int cmdCompletion(char *cmd)
{
    FILE *pipe_fp;
    char buffer[MAX_COMMAND_LENGTH] = "";
    char dummyBuffer[MAX_COMMAND_LENGTH] = "";
    char shellCommand[2048];
    int completionNotUnique = FALSE;

    if(strlen(cmd) > 0)
	{
	    showMessage((gchar *) _("completing..."));

	    strcpy(shellCommand, "/bin/sh -c '");
	    strcat(shellCommand, "cmd=\"");
	    strcat(shellCommand, cmd);
	    strcat(shellCommand, "\"\n");
	    strcat(shellCommand, shellScript);
	    strcat(shellCommand, "'");
	    
	    if((pipe_fp = popen(shellCommand, "r")) == NULL)
		showMessage((gchar *) _("no /bin/sh"));
	    
	    /* get first line from shell script answer */
	    fgets(buffer, MAX_COMMAND_LENGTH-1, pipe_fp);
	    /* erase \n */
	    if(strlen(buffer) > 1)
		buffer[strlen(buffer)-1]='\000';

	    /* get the rest */
	    while(fgets(dummyBuffer, MAX_COMMAND_LENGTH-1, pipe_fp))
		if(strlen(dummyBuffer) > 1)
		    completionNotUnique = TRUE;
	    
	    if(strlen(buffer) > 1 && completionNotUnique == FALSE)
		{
		    strcpy(cmd, buffer);
		    showMessage((gchar *) _("completed"));
		}
	    else if(strlen(buffer) > 1 && completionNotUnique == TRUE)
		showMessage((gchar *) _("not unique"));
	    else
		showMessage((gchar *) _("not found"));

	    pclose(pipe_fp);
	}
    else
	showMessage((gchar *) _("not unique"));
}






