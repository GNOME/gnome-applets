#include <unistd.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <fvwmlib.h>
#include "winlist.h"

static gint fvwm_readfd, fvwm_writefd;

static void handle_fvwm_msg_read(gpointer data, gint fd,
				 GdkInputCondition condition);

void
init_winlist(int argc, char **argv, int firstarg)
{
  g_print("init_winlist running on %p\n", winlist);
  g_hash_table_insert(winlist, (gpointer)1, "Hello");
  g_hash_table_insert(winlist, (gpointer)2, "Bubye");
  g_hash_table_insert(winlist, (gpointer)3, "This is a test");

#if 0
  fvwm_writefd = atoi(argv[firstarg]);
  fvwm_readfd = atoi(argv[firstarg + 1]);
  gdk_input_add(fvwm_readfd,
		GDK_INPUT_READ|GDK_INPUT_EXCEPTION,
		handle_fvwm_message,
		NULL);
#endif
}

void
switch_to_window(guint win_id)
{
  g_warning("switch_to_window(%d) unimplemented\n", win_id);
}

#if 0
static void
handle_fvwm_message(gint fd)
{
  unsigned char msg_header[HEADER_SIZE], *msg_body_char;
  unsigned long *msg_body;

  if(ReadFvwmPacket(fd, msg_header, &msg_body) <= 0) return;
  msg_body = (unsigned long *)msg_body_char;

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
