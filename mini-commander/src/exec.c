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

#include <string.h>
#include <config.h>
#include <gnome.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "exec.h"
#include "preferences.h"
#include "message.h"

/* 
   These routines will only work on UNIX-alike system because fork()
   is needed. Perhaps I will add an alternative for non UNIX system in
   future (by using gnome-lib functions) 
*/


void execCommand(char *cmd)
{
    pid_t pid;
    char *argv[10];
    char url[1000];
    char command[1000];
    char commandExec[1000];
    char macro[1000];
    char *substPtr;
    int prefixRecognized = FALSE;
    int i, j;

    /* make local copy of cmd; now we can minipulate command without
       changing cmd (important for history) */
    strcpy(command, cmd);

    /* prefix used? */
    for(i=0; i<=MAX_PREFIXES-1; i++)
	{
	    if (prop.prefix[i] != (char *) NULL &&
		strlen(prop.prefix[i]) > 0 &&
		strncmp(command, prop.prefix[i], strlen(prop.prefix[i])) == 0)
		{
		    /* prefix recognized */
		    prefixRecognized = TRUE;
		    strcpy(macro, prop.command[i]);

		    if ((substPtr = strstr(macro, "$1")) != (char *) NULL)
			{
			    /* "$1" found */
			    *substPtr = '\000';
			    strcpy(commandExec, macro);
			    strcat(commandExec, command + strlen(prop.prefix[i]));
			    strcat(commandExec, substPtr + 2);
			}
		    else
			{
			    /* no $1 in this macro */
			    strcpy(commandExec, prop.command[i]); 
			}

		    for(j=0; j<10; j++)
			{
			    /* substitute up to ten $1's */
			    strcpy(macro, commandExec);
			    if ((substPtr = strstr(macro, "$1")) != (char *) NULL)
				{
				    /* "$1" found */
				    *substPtr = '\000';
				    strcpy(commandExec, macro);
				    strcat(commandExec, command + strlen(prop.prefix[i]));
				    strcat(commandExec, substPtr + 2);
				}
			}
	    
		    strcpy(command, commandExec);
		}
	}    

    /* execute command */
    showMessage((gchar *) _("starting...")); 	    
    if ((pid = fork()) < 0) {
	showMessage((gchar *) _("fork error")); 
    }
    else if (pid == 0) 
	{
	    /* child */
	    argv[0] = "sh";
	    argv[1] = "-c";
	    argv[2] = command;
	    argv[3] = NULL;
	    execv("/bin/sh", argv);
	    /* showMessage((gchar *) _("exec failed")); */
	    /* if this line is reached there is really big trouble */
	    showMessage((gchar *) _("no /bin/sh"));
	    /* stop this process; I have to find a better solution here */
	    execl("/bin/nice", "nice", NULL);
	} 
}

static void sighandle(int sig)
{
    pid_t pid = 0;
    int status;
    
    /* showMessage((gchar *) _("signal...")); */  
    /*if (sig == SIGCHLD) */
    if (waitpid(0, &status, WNOHANG ) > 0)
	showMessage((gchar *) _("child exited")); 
    signal(SIGCHLD, &sighandle);
    return;
}

void initExecSignalHandler(void)
{
    /* install signal handler */
    signal(SIGCHLD, &sighandle);
}
