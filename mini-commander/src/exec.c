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
exec_command(char *cmd)
{
    pid_t pid;
    char *argv[5];
    char command[1000];
    pid_t PID = getpid();
	    
    /* make local copy of cmd; now we can minipulate command without
       changing cmd (important for history) */
    strncpy(command, cmd, sizeof(command));
    /* Add terminating \0 to the end */
    command[sizeof(command)-1] = '\0';

    expand_command(command);

    /* execute command */
    show_message((gchar *) _("starting...")); 	    
    if ((pid = fork()) < 0) {
	show_message((gchar *) _("fork error")); 
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
    int show_a_message = 0;

    /* call waitpid to remove the child process and to prevent that it
       becomes a zombie */

    while(waitpid(0, NULL, WNOHANG) > 0)
	show_a_message = 1;

    if(show_a_message)
	show_message((gchar *) _("child exited"));
    return;
    sig = 0;
}

static
void sighandle_sigalrm(int sig)
{
    /* sleep a bit (or wait for the next signal) so that the following
       message is not overwritten by the "child exited" message */
    sleep(1);

    show_message((gchar *) _("no /bin/sh"));
   
    /*
       gnome_dialog_run
       (GNOME_DIALOG
       (gnome_message_box_new((gchar *) _("No /bin/sh found!"),
       GNOME_MESSAGE_BOX_WARNING,
       GNOME_STOCK_BUTTON_CANCEL,
       NULL)
       )
       ); */
    return;
    sig = 0;
}

void
init_exec_signal_handler(void)
{
    int sa_flags = 0;		  /* Default flags */
    struct sigaction act = {{0}}; /*this sets all fields to 0, no
				  matter how many fields there are*/

    /* Try to set appropriate signal options */
#if defined(SA_NOMASK)
    sa_flags = SA_NOMASK;
#elif defined(SA_NODEFER)
    sa_flags = SA_NODEFER;
#endif

    act.sa_handler = &sighandle_sigchld;
    /* act.sa_mask =  */
    act.sa_flags = sa_flags;
    /* act.sa_restorer = NULL; */

    /* install signal handler */
    sigaction(SIGCHLD, &act, NULL); 

    act.sa_handler = &sighandle_sigalrm;
    /* act.sa_mask =  */
    act.sa_flags = sa_flags;
    /* act.sa_restorer = NULL; */

    /* install signal handler */
    sigaction(SIGALRM, &act, NULL); 
}
