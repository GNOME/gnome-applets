#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "linux-proc.h"

static char *skip_token(const char *p)
{
	while (isspace(*p)) p++;
        while (*p && !isspace(*p)) p++;
        	return (char *)p;
}  

/*
 * Return a -1-terminated array structures containing the name & byte counts.
 */

Device_Info *ReadProc()
{
	Device_Info	*retval = NULL;
	int	devs = 0;
	char	buffer[256];
	static FILE	*fp = NULL;

	if (!fp){
		/* Read /proc/net/ip_acct. */
		if (!(fp = fopen("/proc/net/ip_acct", "r"))){
			fprintf(stderr, "Failed to open ip_acct file.");
			return NULL;
		}
	}

	fseek(fp, 0, 0);
	retval = (Device_Info *)g_malloc(sizeof(Device_Info));
	/* Skip the header line. */
	fgets(buffer, 255, fp);

	while (fgets(buffer, 255, fp)){
		char	*p;
		retval = (Device_Info *)g_realloc(retval, (sizeof(Device_Info) * (++devs + 1)));

		/* Skip over the network thing. */		
		p = skip_token(buffer) + 1;
		strncpy(retval[devs - 1].name, p, 4);
		retval[devs - 1].name[4] = 0;
		retval[devs - 1].bytes = strtoul(buffer + 64, &p, 10);
#if 0
		printf("%d: %s - %ld\n", devs, retval[devs - 1].name, retval[devs - 1].bytes);
#endif
	}

	retval[devs].name[0] = 0;

	return retval;
}	

/*
 * Return the byte count for a single device. Cache the result from reading the /proc file.
 * refresh = 1 means reload the /proc file.
 */
unsigned long int
GetTraffic(int refresh, char *device)
{
	static	Device_Info	*di = NULL;
	Device_Info	*d;

	if (refresh || !di){
		if (di)
			free(di);
		di = ReadProc();
	}

	d = di;
	while (d->name[0] != 0){
		if (!strcmp(device, d->name)){
			return d->bytes;
		}
		d++;
	}
	return 0;
}

/*
 * Find out what we have available.
 */

#if 0
void main()
{
	printf("Got %ld\n", GetTraffic(1, "ppp0"));
}
#endif
