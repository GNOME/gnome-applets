#ifndef MODULE_H
#define MODULE_H

struct queue_buff_struct
{
  struct queue_buff_struct *next;
  unsigned long *data;
  int size;
  int done;
};

extern int npipes;
extern int *readPipes;
extern int *writePipes;
extern struct queue_buff_struct **pipeQueue;

#define START_FLAG 0xffffffff

#define M_TOGGLE_PAGING      1
#define M_NEW_PAGE           2
#define M_NEW_DESK           4
#define M_ADD_WINDOW         8
#define M_RAISE_WINDOW      16
#define M_LOWER_WINDOW      32
#define M_CONFIGURE_WINDOW  64
#define M_FOCUS_CHANGE     128
#define M_DESTROY_WINDOW   256
#define M_ICONIFY          512
#define M_DEICONIFY       1024
#define M_WINDOW_NAME     2048
#define M_ICON_NAME       4096
#define M_RES_CLASS       8192
#define M_RES_NAME       16384
#define M_END_WINDOWLIST 32768
#define M_ICON_LOCATION  65536
#define M_MAP           131072
#define MAX_MESSAGES       18
#define MAX_MASK        262143

#define HEADER_SIZE         3
#define MAX_PACKET_SIZE    27
#define MAX_BODY_SIZE      (MAX_PACKET_SIZE - HEADER_SIZE)


#endif /* MODULE_H */



