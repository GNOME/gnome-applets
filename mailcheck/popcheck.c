/* GNOME pop/imap-mail-check-library.
 * (C) 1997, 1998 The Free Software Foundation
 *
 * Author: Lennart Poettering
 *
 */

#include "config.h"

#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <glib.h>

#include "popcheck.h"

#define TIMEOUT 120

#ifdef ENABLE_IPV6
static gboolean have_ipv6(void);
#endif

static int get_server_port(const char *);
static char* get_server_hostname(const char *);
static int connect_socket(const char *, int);
static char *read_line(int);
static int write_line(int, char *);
static int is_pop3_answer_ok(const char *);
static int is_imap_answer_untagged(const char *);
static int is_imap_answer_ok(const char *);
static char *wait_for_imap_answer(int, char *);

#ifdef ENABLE_IPV6
/*Check whether the node is IPv6 enabled.*/
static gboolean have_ipv6(void)
 {
  int s;

  s = socket(AF_INET6, SOCK_STREAM, 0);
  if (s != -1) {
   close(s);
   return TRUE;
  }
  return FALSE;
 }
#endif

static int get_server_port(const char *h)
 {
  const char *x;
  int cnt;

  for (cnt = 0, x = h; *x; x++) {
   if (*x == ':')
     cnt ++;
  }

  x = strchr(h, ']');
  if (x)
   x = strchr(x, ':');
  else
   x = (cnt < 2) ? strchr(h, ':') : NULL;

  if (x)
   {
    return atoi(x+1);
   }
  else
   return 0;
 }
 
static char* get_server_hostname(const char *h)
 {
  const char *e, *n;
  int cnt;
  if (!h) return 0;
  
  for (cnt = 0, n = h; *n; n++) {
   if (*n == ':')
     cnt ++;
  }

  e = NULL;
  n = strchr(h, ']');
  if (n)
   e = strchr(n, ':');
  else {
   if (cnt < 2)
     e = strchr(h, ':');
  }

  if (e) 
   {
    char *x;
    int l;
    l = (n == NULL) ? (e-h) : (n-h-1);

    x = g_malloc(l+5);
    if (n == NULL)
      strncpy(x, h, l);
    else
      strncpy(x, h+1, l);

    x[l] = 0;
    return x;
   }
  else 
   {
    if (n == NULL)
      return strcpy((char*) g_malloc(strlen(h)+1), h); 
    else 
      return strncpy((char*) g_malloc(strlen(h)-1), h+1, n-h-1);
   } 
 }

static int connect_socket(const char *h, int def)
 {
#if defined (ENABLE_IPV6) && defined (HAVE_GETADDRINFO)
  struct addrinfo hints, *res, *result;
#endif
  struct hostent *he;
  struct sockaddr_in peer;
  int fd, p;
  char *hn;
  extern int h_errno;

  h_errno = 0;
  
  hn = get_server_hostname(h);
  if (!hn)
   return NO_SERVER_INFO;
  
  p = get_server_port(h); 
  if (p == 0) p = def;

#if defined (ENABLE_IPV6) && defined (HAVE_GETADDRINFO)
  result = NULL;
  if (have_ipv6()) {
   int status;

   fd = 0;
   status = 0;

   memset(&hints, 0, sizeof(hints));
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_CANONNAME;

   if (getaddrinfo(hn, NULL, &hints, &result) != 0) {
     g_free(hn);

     if (result == NULL) 
       return INVALID_SERVER;
     else 
       return NETWORK_ERROR;
   }

   for (res = result; res; res = res->ai_next) {
     if (res->ai_family != AF_INET6 && res->ai_family != AF_INET)
       continue;

     fd = socket(res->ai_family, SOCK_STREAM, 0);
     if (fd < 0)
       continue;

     if (res->ai_family == AF_INET)
       ((struct sockaddr_in *)res->ai_addr)->sin_port = htons(p);

     if (res->ai_family == AF_INET6) 
       ((struct sockaddr_in6 *)res->ai_addr)->sin6_port = htons(p);

     status = connect(fd, res->ai_addr, res->ai_addrlen);
     if (status != -1)
       break;

     close(fd);
   }
   
   freeaddrinfo(result);

   if (!res) {
     if (fd < 0 || status < 0)
       return NETWORK_ERROR;
     else
       return INVALID_SERVER;
   }
  }
  else
#endif
   {
    he = gethostbyname(hn);
    g_free(hn);

    if (!he) {
     if (h_errno == HOST_NOT_FOUND)
       return INVALID_SERVER;
     else
       return NETWORK_ERROR;
    }

    fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd < 0) 
      return NETWORK_ERROR;

    peer.sin_family = AF_INET;
    peer.sin_port = htons(p);
    peer.sin_addr = *(struct in_addr*) he->h_addr;

    if (connect(fd, (struct sockaddr*) &peer, sizeof(peer)) < 0) 
     {
      close(fd);
      return NETWORK_ERROR;
     }
   } /*have_ipv6*/

  return fd; 
 }  

static char *read_line(int s)
 {
  static char response[1024];
  char *c;
  int m = sizeof(response);
  
  c = response;

  while (m--)
   {
    char ch;
    fd_set fs;
    struct timeval t;
    
    FD_ZERO(&fs);
    FD_SET(s, &fs);
    
    t.tv_sec = TIMEOUT;
    t.tv_usec = 0;
    
    if (select(FD_SETSIZE, &fs, 0, 0, &t) <= 0) 
     return NULL;
     
    if (read(s, &ch, sizeof(ch)) != sizeof(ch)) 
     return NULL;
     
    if (ch == 10)
     {
      *c = 0;
      return response;
     } 
     
    *(c++) = ch;
   }    
   
  return NULL; 
 }

static int write_line(int s, char *p)
 {
  char *p2;
  p2 = g_malloc(strlen(p)+3);
  strcat(strcpy(p2, p), "\r\n");

  if (write(s, p2, strlen(p2)) ==  strlen(p2))
   {
    g_free(p2);
    return 1;
   }
   
  g_free(p2); 
  return 0;
 }


static int is_pop3_answer_ok(const char *p)
 {
  if (p)
   if (p[0] == '+') return 1;

  return 0;
 }

int pop3_check(const char *h, const char* n, const char* e)
{
  int s;
  char *c;
  char *x;
  int r = -1, msg = 0, last = 0;
  
  if (!h || !n || !e) return -1;
  
  s = connect_socket(h, 110);

  if (s > 0) {
    if (!is_pop3_answer_ok(read_line(s))) {
      close(s);
      return NO_SERVER_INFO;
    }

    c = g_strdup_printf("USER %s", n);
    if (!write_line(s, c) ||
        !is_pop3_answer_ok(read_line(s))) {
      close(s);
      g_free(c);
      return INVALID_USER;
    }
    g_free(c);

    c = g_strdup_printf("PASS %s", e);
    if (!write_line(s, c) ||
        !is_pop3_answer_ok(read_line(s))) {
      close(s);
      g_free(c);
      return INVALID_PASS;
    }
    g_free(c);

    if (write_line(s, "STAT") &&
        is_pop3_answer_ok(x = read_line(s)) &&
	x != NULL &&
        sscanf(x, "%*s %d %*d", &msg) == 1)
      r = ((unsigned int)msg & 0x0000FFFFL);

    if (r != -1 &&
        write_line(s, "LAST") &&
        is_pop3_answer_ok(x = read_line(s)) &&
	x != NULL &&
        sscanf(x, "%*s %d", &last) == 1)
      r |= (unsigned int)(msg - last) << 16;

    if (write_line(s, "QUIT"))
      read_line(s);

    close(s);
    return r;
  }     
   
  return s;
}

static int is_imap_answer_untagged(const char *tag)
 {
  return tag ? *tag == '*' : 0;
 }

static int is_imap_answer_ok(const char *p)
 {
  if (p) 
   {
    const char *b = strchr(p, ' ');
    if (b)
     {
      if (*(++b) == 'O' && *(++b) == 'K') 
       return 1;
     }
   }
  
  return 0;
 }

static char *wait_for_imap_answer(int s, char *tag)
 {
  char *p;
  int i = 10; /* read not more than 10 lines */
  
  while (i--)
   {
    p = read_line(s);
    if (!p) return 0;
    if (strncmp(p, tag, strlen(tag)) == 0) return p;
   }
   
  return 0; 
 }

int 
imap_check(const char *h, const char* n, const char* e, const char* f)
{
	int s;
	char *c = NULL;
	char *x;
	unsigned int r = (unsigned int) -1;
	int total = 0, unseen = 0;
	int count = 0;
	
	if (!h || !n || !e) return NO_SERVER_INFO;
	
	if (f == NULL ||
	    f[0] == '\0')
		f = "INBOX";
	
	s = connect_socket(h, 143);
	
	if (s < 0)
		return s;
	
	x = read_line(s);
	/* The greeting us untagged */
	if (!is_imap_answer_untagged(x))
		goto return_from_imap_check;
	
	if (!is_imap_answer_ok(x))
		goto return_from_imap_check;
	
	
	c = g_strdup_printf("A1 LOGIN \"%s\" \"%s\"", n, e);
	if (!write_line(s, c))
		goto return_from_imap_check;
	
	if (!is_imap_answer_ok(wait_for_imap_answer(s, "A1"))) {
		g_free (c);
		close (s);
		return INVALID_PASS;
	}
	
	c = g_strdup_printf("A2 STATUS \"%s\" (MESSAGES UNSEEN)",f);
	if (!write_line(s, c))
		goto return_from_imap_check;
	
	/* We only loop 5 times if more than that this is
	 * probably a bogus reply */
	for (count = 0, x = read_line(s);
	     count < 5 && x != NULL;
	     count++, x = read_line(s)) {
		char tmpstr[4096];
		
		if (sscanf(x, "%*s %*s %*s %*s %d %4095s %d",
			   &total, tmpstr, &unseen) != 3)
			continue;
		if (strcmp(tmpstr, "UNSEEN") == 0)
			break;
		
	}
	
	r = (((unsigned int) unseen ) << 16) | /* lt unseen only */
		((unsigned int) total & 0x0000FFFFL);
	
	if (write_line(s, "A3 LOGOUT"))
		read_line(s);

 return_from_imap_check:

	g_free (c);
	close (s);
	
	return r; 
 }

