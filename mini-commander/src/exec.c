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
#include <unistd.h>

#include "exec.h"
#include "preferences.h"
#include "macro.h"
#include "message.h"

/* 
   These routines will only work on UNIX-alike system because fork()
   is needed. Perhaps I will add an alternative for non UNIX system in
   future (by using gnome-lib functions) 
*/



void
execCommand(char *cmd)
{
    pid_t pid;
    char *argv[5];
    char command[1000];
    pid_t PID = getpid();
	    
    /* make local copy of cmd; now we can minipulate command without
       changing cmd (important for history) */
    strcpy(command, cmd);

    expandCommand(command);

    /* execute command */
    showMessage((gchar *) _("starting...")); 	    
    if ((pid = fork()) < 0) {
	showMessage((gchar *) _("fork error")); 
    }
    else if (pid == 0) 
	{
	    /* child */
	    /* try the bash as interactive login so that personal prolfiles are reflected */
/* 	    argv[0] = "bash"; */
/* 	    argv[1] = "--login"; */
/* 	    argv[2] = "-c"; */
/* 	    argv[3] = command; */
/* 	    argv[4] = NULL;   */
/* 	    execv("/bin/sh", argv); */

	    /* looks like there is no bash -> try sh */
	    argv[0] = "sh";
	    argv[1] = "-c";
	    argv[2] = command;
	    argv[3] = NULL;  
	    execv("/bin/sh", argv);

	    /* if this line is reached there is really big trouble */

	    /* send SIGALRM signal to parent process; this causes a
               "no /bin/sh" message to be shown*/
	    kill(PID, SIGALRM);

	    /* terminate this forked process; exit(0) would kill the
               parent process, too */
	    _exit(0); 
	} 
}

static
void sighandle_sigchld(int sig)
{
    pid_t pid = 0;
    int status;

    /* call waitpid to remove the child process and to prevent that it
       becomes a zombie */
    /* if (waitpid(0, &status, WNOHANG ) > 0) */
    /* 	  showMessage((gchar *) _("child exited"));  */
    wait(&status);
    showMessage((gchar *) _("child exited"));
}

static
void sighandle_sigalrm(int sig)
{
    /* sleep a bit (or wait for the next signal) so that the following
       message is not overwritten by the "child exited" message */
    sleep(1);

    showMessage((gchar *) _("no /bin/sh"));
   
    /*
       gnome_dialog_run
       (GNOME_DIALOG
       (gnome_message_box_new((gchar *) _("No /bin/sh found!"),
       GNOME_MESSAGE_BOX_WARNING,
       GNOME_STOCK_BUTTON_CANCEL,
       NULL)
       )
       ); */
}

void
initExecSignalHandler(void)
{
    struct sigaction act;

    act.sa_handler= &sighandle_sigchld;
    /* act.sa_mask =  */
    act.sa_flags = SA_NOMASK;
    act.sa_restorer = NULL;

    /* install signal handler */
    sigaction(SIGCHLD, &act, NULL); 

    act.sa_handler= &sighandle_sigalrm;
    /* act.sa_mask =  */
    act.sa_flags = SA_NOMASK;
    act.sa_restorer = NULL;

    /* install signal handler */
    sigaction(SIGALRM, &act, NULL); 
}
