#include "http_get.h"
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int
http_get_to_file(gchar *a_host, gint a_port, gchar *a_resource, FILE *a_file)
{
  struct hostent         *l_hostent = NULL;
  struct sockaddr_in      l_addr;
  int                     l_return = 0;
  int                     l_socket = 0;
  
  /* init the l_addr struct */
  memset(&l_addr, 0, sizeof(struct sockaddr_in));
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
  
 ec:
  return l_return;
}

