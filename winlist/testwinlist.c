/* simple program to get winlist from fvwm as a module */
/* Michael Fulbright <msf@redhat.com> 1998             */

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "fvwmlib.h"
#include "module.h"

/* pipe to talk to fvwm with */
int Fvwm_fd[2];

/* function defs */
void EndLessLoop(void);
void ReadFvwmPipe(void);
int  ReadFvwmPacket(int fd,unsigned long *header, unsigned long **body);
void ProcessMessage(unsigned long type, unsigned long *body);
void SendFvwmPipe(char *message, unsigned long window);
void DeadPipe(int nonsense);

#ifdef TEST
/* start of main program */
void
main(int argc, char **argv)
{

	if((argc != 6)&&(argc != 7)) {
		fprintf(stderr,"%s Version %s should only be executed "
			"by fvwm!\n","testwinlist", "0.1");
		exit(1);
	}


/*	if ((argc==7)&&(!mystrcasecmp(argv[6],"Transient"))) Transient=1; */

	Fvwm_fd[0] = atoi(argv[1]);
	Fvwm_fd[1] = atoi(argv[2]);
	
fprintf(stderr, "write and read fds are %d %d\n", Fvwm_fd[0], Fvwm_fd[1]);

	signal (SIGPIPE, DeadPipe);
	
	/* Request a list of all windows,
	 * wait for ConfigureWindow packets */
	SendFvwmPipe("Send_WindowList",0);

	/* Recieve all messages from Fvwm */
	EndLessLoop();
}
#endif

/******************************************************************************
  EndLessLoop -  Read and redraw until we get killed, blocking when can't read
******************************************************************************/
void
EndLessLoop()
{
	fd_set readset;
	struct timeval tv;
	
	while(1) {
		FD_ZERO(&readset);
		FD_SET(Fvwm_fd[1],&readset);
		tv.tv_sec=0;
		tv.tv_usec=0;

		if (!select(Fvwm_fd[1]+1,&readset,NULL,NULL,&tv)) {
			FD_ZERO(&readset);
			FD_SET(Fvwm_fd[1],&readset);
			select(Fvwm_fd[1]+1,&readset,NULL,NULL,NULL);
		}

		if (!FD_ISSET(Fvwm_fd[1],&readset)) continue;
		ReadFvwmPipe();
	}
}
  
/******************************************************************************
  ReadFvwmPipe - Read a single message from the pipe from Fvwm
    Originally Loop() from FvwmIdent:
      Copyright 1994, Robert Nation and Nobutaka Suzuki.
******************************************************************************/
void
ReadFvwmPipe()
{
	int count,total,count2=0,body_length;
	unsigned long header[3],*body;
	char *cbody;
	if(ReadFvwmPacket(Fvwm_fd[1],header,&body) > 0)	{
		ProcessMessage(header[1],body);
		free(body);
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


/******************************************************************************
  ProcessMessage - Process the message coming from Fvwm
    Skeleton based on processmessage() from FvwmIdent:
      Copyright 1994, Robert Nation and Nobutaka Suzuki.
******************************************************************************/
void
ProcessMessage(unsigned long type,unsigned long *body)
{
	int redraw=0,i;
	long flags;
	char *name,*string;
	
	switch(type) {
	case M_ADD_WINDOW:
	case M_CONFIGURE_WINDOW:
		fprintf(stderr, "Win id = 0x%x\n",body[0]);
#if 0
		if (body[0]==win) {
			win_x=(int)body[3];
			win_y=(int)body[4];
			win_title=(int)body[9];
			win_border=(int)body[10];
			if (win_border!=0) win_border++;
		}
		if (FindItem(&windows,body[0])!=-1) break;
		if (!(body[8]&WINDOWLISTSKIP) || !UseSkipList)
			AddItem(&windows,body[0],body[8]);
#endif
		break;
	case M_DESTROY_WINDOW:
#if 0
		if ((i=DeleteItem(&windows,body[0]))==-1) break;
		RemoveButton(&buttons,i);
		if (WindowIsUp) AdjustWindow();
		redraw=1;
#endif
		break;
	case M_WINDOW_NAME:
	case M_ICON_NAME:
#if 0
		if ((type==M_ICON_NAME && !UseIconNames) || 
		    (type==M_WINDOW_NAME && UseIconNames)) break;
		if ((i=UpdateItemName(&windows,body[0],(char *)&body[3]))==-1) break;
		string=(char *)&body[3];
		name=makename(string,ItemFlags(&windows,body[0]));
		if (UpdateButton(&buttons,i,name,-1)==-1)
			AddButton(&buttons,name,1);
		free(name);
		if (WindowIsUp) AdjustWindow();
		redraw=1;
#endif
		break;
	case M_DEICONIFY:
	case M_ICONIFY:
#if 0
		if ((i=FindItem(&windows,body[0]))==-1) break;
		flags=ItemFlags(&windows,body[0]);
		if (type==M_DEICONIFY && !(flags&ICONIFIED)) break;
		if (type==M_ICONIFY && flags&ICONIFIED) break;
		flags^=ICONIFIED;
		UpdateItemFlags(&windows,body[0],flags);
		string=ItemName(&windows,i);
		name=makename(string,flags);
		if (UpdateButton(&buttons,i,name,-1)!=-1) redraw=1;
		free(name);
#endif
		break;
	case M_END_WINDOWLIST:
#if 0
		if (!WindowIsUp) MakeMeWindow();
		redraw=1;
#endif
		break;
	case M_NEW_DESK:
		break;
	case M_NEW_PAGE:
		break;
	}
	
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
	
	while(1) {
		temp=strchr(hold,',');
		if (temp!=NULL) {
			temp_msg=malloc(temp-hold+1);
			strncpy(temp_msg,hold,(temp-hold));
			temp_msg[(temp-hold)]='\0';
			hold=temp+1;
		} else temp_msg=hold;
		
		write(Fvwm_fd[0],&window, sizeof(unsigned long));
		
		w=strlen(temp_msg);
		write(Fvwm_fd[0],&w,sizeof(int));
		write(Fvwm_fd[0],temp_msg,w);
		
		/* keep going */
		w=1;
		write(Fvwm_fd[0],&w,sizeof(int));
		
		if(temp_msg!=hold) free(temp_msg);
		else break;
	}
}


void
DeadPipe(int nonsense)
{
	fprintf(stderr, "SIGPIPE detected\n");
	exit(1);
}
