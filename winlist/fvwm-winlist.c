/*
 * I put this in because the code to talk to Fvwm is derived from a work
 * covered by the copyright below 
 *
 * Copyright 1994, Robert Nation. No guarantees or warantees or anything
 * are provided or implied in any way whatsoever. Use this program at your
 * own risk. Permission to use this program for any purpose is given,
 * as long as the copyright is kept intact. 
 */
 

#include <unistd.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

#include <sys/time.h>
#include <sys/types.h>

#include "fvwmlib.h"
#include "module.h"
#include "winlist.h"

static gint fvwm_readfd, fvwm_writefd;

static void handle_fvwm_fd(gpointer data, gint fd,GdkInputCondition condition);
static void handle_fvwm_message(gint fd);


/* narfed this out of fvwm.h, need to clean this up! */
#define WINDOWLISTSKIP         262144 

void
DeadPipe(int nonsense)
{
	fprintf(stderr, "SIGPIPE detected\n");
}

/******************************************************************************
  SendFvwmPipe - Send a message back to fvwm 
    Based on SendInfo() from FvwmIdent:
      Copyright 1994, Robert Nation and Nobutaka Suzuki.
******************************************************************************/
void
SendFvwmPipe(char *message,unsigned long window)
{
	int w;
	char *hold,*temp,*temp_msg;
	hold=message;

g_message("Sending ->%s<-", message);

	while(1) {
		temp=strchr(hold,',');
		if (temp!=NULL) {
			temp_msg=malloc(temp-hold+1);
			strncpy(temp_msg,hold,(temp-hold));
			temp_msg[(temp-hold)]='\0';
			hold=temp+1;
		} else temp_msg=hold;
		
		write(fvwm_writefd,&window, sizeof(unsigned long));
		
		w=strlen(temp_msg);
		write(fvwm_writefd,&w,sizeof(int));
		write(fvwm_writefd,temp_msg,w);
		
		/* keep going */
		w=1;
		write(fvwm_writefd,&w,sizeof(int));
		
		if(temp_msg!=hold) free(temp_msg);
		else break;
	}
}

/************************************************************************
 *
 * Reads a single packet of info from fvwm. Prototype is:
 * unsigned long header[3];
 * unsigned long *body;
 * int fd[2];
 * void DeadPipe(int nonsense); /* Called if the pipe is no longer open
 *
 * ReadFvwmPacket(fd[1],header, &body);
 *
 * Returns:
 *   > 0 everything is OK.
 *   = 0 invalid packet.
 *   < 0 pipe is dead. (Should never occur)
 *
 **************************************************************************/
int
ReadFvwmPacket(int fd, unsigned long *header, unsigned long **body)
{
	int count,total,count2,body_length;
	char *cbody;
	extern void DeadPipe(int);
	
	if((count = read(fd,header,3*sizeof(unsigned long))) >0) {
		if(header[0] == START_FLAG) {
			body_length = header[2]-3;
			*body = (unsigned long *)
				malloc(body_length * sizeof(unsigned long));
			cbody = (char *)(*body);
			total = 0;
			while(total < body_length*sizeof(unsigned long)) {
				if((count2=
				    read(fd,&cbody[total],
					 body_length*sizeof(unsigned long)-total)) >0) {
					total += count2;
				} else if(count2 < 0) {
					DeadPipe(1);
				}
			}
		} else {
			count = 0;
		}
	}
	if(count <= 0)
		DeadPipe(1);
	return count;
}

void
init_winlist(int argc, char **argv, int firstarg)
{
  gint tag;

  g_print("init_winlist running on %p\n", winlist);
  g_hash_table_insert(winlist, (gpointer)1, "Hello");
  g_hash_table_insert(winlist, (gpointer)2, "Bubye");
  g_hash_table_insert(winlist, (gpointer)3, "This is a test");

#if 0
  fvwm_writefd = atoi(argv[firstarg]);
  fvwm_readfd = atoi(argv[firstarg + 1]);

  g_message("write, read fd are %d %d",fvwm_writefd, fvwm_readfd);

#if 0
  tag=gdk_input_add(fvwm_readfd,
		GDK_INPUT_READ,
		handle_fvwm_fd,
		NULL);
g_message("gdk_input_add tag is %d",tag);

  SendFvwmPipe("Send_WindowList", 0);
#endif

#endif
/*  EndLessLoop();  */
}

void
EndLessLoop()
{
	fd_set readset;
	struct timeval tv;
	
	while(1) {
		g_message("Waiting on input");
		FD_ZERO(&readset);
		FD_SET(fvwm_readfd,&readset);
		tv.tv_sec=0;
		tv.tv_usec=0;

		if (!select(fvwm_readfd+1,&readset,NULL,NULL,&tv)) {
			FD_ZERO(&readset);
			FD_SET(fvwm_readfd,&readset);
			select(fvwm_readfd+1,&readset,NULL,NULL,NULL);
		}

		if (!FD_ISSET(fvwm_readfd,&readset)) continue;
		/*ReadFvwmPipe(); */
		g_message("Got input");
		handle_fvwm_message(fvwm_readfd);
	}
}

void
switch_to_window(guint win_id)
{
  g_warning("switch_to_window(%d) unimplemented\n", win_id);
}

#if 1
static void
handle_fvwm_message(gint fd)
{
  unsigned long msg_header[HEADER_SIZE];
  unsigned char *msg_body_char;
  unsigned long *msg_body;

g_message("Entering handle_fvwm_message, header size is %d",HEADER_SIZE);

  if(ReadFvwmPacket(fd, msg_header, &msg_body) <= 0) return;
/*  msg_body = (unsigned long *)msg_body_char; */

g_message("Message type is %d", msg_header[1]);

  switch(msg_header[1]) /* message type */
    {
    case M_ADD_WINDOW:
      g_print("New window %#x created, we should skip it: %d\n",
	      msg_body[0],
	      msg_body[8]&WINDOWLISTSKIP);
      break;
    case M_DESTROY_WINDOW:
      g_print("Window %#x destroyed\n", msg_body[0]);
      break;
    case M_WINDOW_NAME:
    case M_ICON_NAME:
      g_print("New name for window is %s\n", (char *)&msg_body[3]);
      break;
    }
  free(msg_body);
}

static void
handle_fvwm_fd(gpointer data,
	       gint fd,
	       GdkInputCondition condition)
{
g_message("Entering handle_fvwm_fd");

  switch(condition)
    {
    case GDK_INPUT_READ:
      handle_fvwm_message(fd);
      break;
    case GDK_INPUT_EXCEPTION:
      gtk_main_quit();
      break;
    default:
      g_error("Unexpected condition on the fvwm connection\n");
    }
}
#endif
