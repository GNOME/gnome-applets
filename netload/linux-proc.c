#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>

#include "linux-proc.h"
#include "netload.h"

#include <config.h>

#include <glibtop.h>
#include <glibtop/netload.h>

#define REQUIRED_NETLOAD_FLAGS (1 << GLIBTOP_NETLOAD_BYTES_TOTAL)

unsigned long int
GetTraffic(const char *device)
{
	glibtop_netload netload;

	glibtop_get_netload (&netload, device);

	if (netload.flags & REQUIRED_NETLOAD_FLAGS)
		return netload.bytes_total;
	else
		return netload.packets_total;
}
