#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>

#include "linux-proc.h"
#include "netload.h"

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
			error_dialog("Failed to open the /proc/net/ip_acct file. Please ensure IP accounting is enabled in this kernel.", 1);
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
		strncpy(retval[devs - 1].name, p, 5);
		retval[devs - 1].name[5] = 0;
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
	static	error_printed = 0;

	if (refresh || !di){
		if (di)
			free(di);
		di = ReadProc();
	}

	d = di;
	while (d && d->name[0] != 0){
		if (!strcmp(device, d->name)){
			return d->bytes;
		}
		d++;
	}
	error_printed = 1;
	error_dialog("IP accounting is enabled, but not activated for the specified device. Either activate it (with a command like \nipfwadm -A -i -P all -W <device name>\nwhere device name is something like ppp0 or eth0.\nAlso check that the device properties are set properly.", 0);
	return 0;
}
