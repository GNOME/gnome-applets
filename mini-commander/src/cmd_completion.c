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

#include "cmd_completion.h"
#include "preferences.h"
#include "macro.h"
#include "message.h"

static char shellScript[] = 
"\n\
for dir in `echo $PATH|sed \"s/^:/. /; s/:$/ ./; s/::/ . /g; s/:/ /g\"`\n\
do\n\
   for file in $dir/$cmd*\n\
   do\n\
      if test -x $file -a ! -d $file\n\
      then\n\
         echo `basename $file`\n\
      fi\n\
   done\n\
done\n\
";

void
cmdCompletion(char *cmd)
{
    FILE *pipe_fp;
    char buffer[MAX_COMMAND_LENGTH] = "";
    char dummyBuffer[MAX_COMMAND_LENGTH] = "";
    char shellCommand[2048];
    char largestPossibleCompletion[MAX_COMMAND_LENGTH] = "";
    int completionNotUnique = FALSE;   
    int numWhitespaces, i, pos;
  
    if(strlen(cmd) == 0)
	{
	    showMessage((gchar *) _("not unique"));
	    return;
	}

    showMessage((gchar *) _("completing..."));

    numWhitespaces = prefixLength_IncludingWhithespaces(cmd) - prefixLength(cmd);

    strcpy(shellCommand, "/bin/sh -c '");
    strcat(shellCommand, "cmd=\"");
    strcat(shellCommand, cmd + prefixLength_IncludingWhithespaces(cmd));
    strcat(shellCommand, "\"\n");
    strcat(shellCommand, shellScript);
    strcat(shellCommand, "'");
    
    if((pipe_fp = popen(shellCommand, "r")) == NULL)
	showMessage((gchar *) _("no /bin/sh"));

    /* get first line from shell script answer */
    fscanf(pipe_fp, "%s\n", largestPossibleCompletion);

    /* get the rest */
    while(fscanf(pipe_fp, "%s\n", buffer) == 1){
	completionNotUnique = TRUE;
	pos = 0;
	while(largestPossibleCompletion[pos] != '\000' 
	      && buffer[pos] != '\000'
	      && strncmp(largestPossibleCompletion, buffer, pos + 1) == 0)
	    pos++;
	strncpy(largestPossibleCompletion, buffer, pos);
	/* strncpy does not add \000 to the end */
	largestPossibleCompletion[pos] = '\000';
    }
    pclose(pipe_fp);
      
    if(strlen(largestPossibleCompletion) > 0)
	{
	    if(getPrefix(cmd) != NULL)
		strcpy(cmd, getPrefix(cmd));
	    else
		strcpy(cmd, "");

	    /* fill up the whitespaces */
	    for(i = 0; i < numWhitespaces; i++)
		strcat(cmd, " ");	

	    strcat(cmd, largestPossibleCompletion);

	    if(!completionNotUnique)
		showMessage((gchar *) _("completed"));
	    else
		showMessage((gchar *) _("not unique"));		
	}
    else
	showMessage((gchar *) _("not found"));
}






