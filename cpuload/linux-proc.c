/* From wmload.c, v0.9.2, licensed under the GPL. */
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>

#include "linux-proc.h"

#define NCPUSTATES 4

static long cp_time[NCPUSTATES];
static long last[NCPUSTATES];

static char *skip_token(const char *p)
{
	while (isspace(*p)) p++;
        while (*p && !isspace(*p)) p++;
        	return (char *)p;
}  

void GetLoad(int Maximum, int *usr, int *nice, int *sys, int *free)
{
  char buffer[100];/*[4096+1];*/
  int fd, len;
  int total;
  char *p;

  fd = open("/proc/stat", O_RDONLY);
  len = read(fd, buffer, sizeof(buffer)-1);
  close(fd);
  buffer[len] = '\0';

  p = skip_token(buffer);               /* "cpu" */

  cp_time[0] = strtoul(p, &p, 0);       /* user   */
  cp_time[1] = strtoul(p, &p, 0);       /* nice   */
  cp_time[2] = strtoul(p, &p, 0);       /* system */
  cp_time[3] = strtoul(p, &p, 0);       /* idle   */

  *usr  = cp_time[0] - last[0];
  *nice = cp_time[1] - last[1];
  *sys  = cp_time[2] - last[2];
  *free = cp_time[3] - last[3];
  total = *usr + *nice + *sys + *free;

  last[0] = cp_time[0];
  last[1] = cp_time[1];
  last[2] = cp_time[2];
  last[3] = cp_time[3];

  *usr = rint(Maximum * (float)(*usr)   /total);
  *nice =rint(Maximum * (float)(*nice)  /total);
  *sys = rint(Maximum * (float)(*sys)   /total);
  *free = rint(Maximum * (float)(*free) /total);
}
