#include "http_get.h"
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define GET_BUFF_SIZE 256

static int
http_get_recv(int a_sd, void *a_buff, size_t a_buff_sz);

static int
http_get_send(int a_sd, void *a_buff, size_t a_buff_sz);

int
http_get_to_file(gchar *a_host, gint a_port, gchar *a_resource, FILE *a_file)
{
  struct hostent         *l_hostent = NULL;
  struct sockaddr_in      l_addr;
  int                     l_return = 0;
  int                     l_socket = 0;
  char                    l_buff[GET_BUFF_SIZE];
  int                     l_bytes_read = 0;
  int                     l_reading_headers = 1;
  int                     l_first_cr_returned = 0;
  int                     l_second_cr_returned = 0;

  /* init the l_addr struct */
  memset(&l_addr, 0, sizeof(struct sockaddr_in));
  /* init the buffer */
  memset(l_buff, 0, GET_BUFF_SIZE);
  /* get the host info */
  l_hostent = gethostbyname(a_host);
  if (l_hostent == NULL)
    {
      l_return = -1;
      goto ec;
    }
  /* copy the address into into the structure. */
  memcpy(&l_addr.sin_addr.s_addr,
	 l_hostent->h_addr_list[0],
	 sizeof(unsigned long));
  /* set some stuff */
  l_addr.sin_family = AF_INET;
  l_addr.sin_port = htons(a_port);
  /* new socket */
  if ((l_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      fprintf(stderr, "Failed to set up socket: %s\n", strerror(errno));
      l_return = -1;
      goto ec;
    }
  if (connect(l_socket, (struct sockaddr *)&l_addr, sizeof(struct sockaddr_in)) < 0)
    {
      fprintf(stderr, "Failed to connect to \"%s\": %s\n",
	      a_host, strerror(errno));
      l_return = -1;
      goto ec;
    }
  if (http_get_send(l_socket, "GET ", strlen("GET ")) < strlen("GET "))
    {
      l_return = -1;
      goto ec;
    }
  if (http_get_send(l_socket, a_resource, strlen(a_resource)) < strlen(a_resource))
    {
      l_return = -1;
      goto ec;
    }
  if (http_get_send(l_socket, " HTTP/1.0\n\n", strlen(" HTTP/1.0\n\n")) < strlen(" HTTP/1.0\n\n"))
    {
      l_return = -1;
      goto ec;
    }
  while ((l_bytes_read = http_get_recv(l_socket, l_buff, 1)) > 0)
    {
      if (l_second_cr_returned && (l_buff[0] == '\n'))
	{
	  break;
	}
      else if (l_first_cr_returned && (l_buff[0] == '\n'))
	{
	  continue;
	}
      else if (l_first_cr_returned && (l_buff[0] == '\r'))
	{
	  l_second_cr_returned = 1;
	}
      else if (l_buff[0] == '\r')
	{
	  l_first_cr_returned = 1;
	}
      else
	{
	  l_first_cr_returned = 0;
	  l_second_cr_returned = 0;
	}
    }
  while ((l_bytes_read = http_get_recv(l_socket, l_buff, GET_BUFF_SIZE)) > 0)
    {
      fwrite(l_buff, l_bytes_read, 1, a_file);
      l_return += l_bytes_read;
    }
  fflush(a_file);
  close(l_socket);
  /* fprintf(stderr, "Resource recieved: %d bytes\n", l_return); */
 ec:
  return l_return;
}

static int
http_get_send(int a_sd, void *a_buff, size_t a_buff_sz)
{
  int             l_total_ct;
  int             l_write_ct;
  
  for (l_total_ct = 0; (l_total_ct < (int)a_buff_sz); l_total_ct += l_write_ct)
    {
      if ((l_write_ct = send(a_sd, ((char *) a_buff) + l_total_ct, a_buff_sz - l_total_ct, 0)) < 0)
	{
	  goto ec;
	}
    }
  
  return l_total_ct;
  
 ec:
  return -1;
}

static int
http_get_recv(int a_sd, void *a_buff, size_t a_buff_sz)
{
  int             l_total_ct;
  int             l_read_ct;
  
  for (l_total_ct = 0; (l_total_ct < (int)a_buff_sz); l_total_ct += l_read_ct)
    {
      if ((l_read_ct = recv(a_sd, ((char *) a_buff) + l_total_ct, a_buff_sz - l_total_ct, 0)) < 0)
	{
	  goto ec;
	}
      if (l_read_ct == 0)
	{
	  break;
	}
    }

  return l_total_ct;
 ec:
  return -1;
}
