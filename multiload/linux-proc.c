/* From wmload.c, v0.9.2, licensed under the GPL. */
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>

#include <assert.h>

#include <config.h>

#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>

#include "linux-proc.h"

#define NCPUSTATES 4

static long cp_time[NCPUSTATES];
static long last[NCPUSTATES];

static unsigned needed_cpu_flags =
(1 << GLIBTOP_CPU_USER) +
(1 << GLIBTOP_CPU_NICE) +
(1 << GLIBTOP_CPU_SYS) +
(1 << GLIBTOP_CPU_IDLE);

static unsigned needed_mem_flags =
(1 << GLIBTOP_MEM_USED) +
(1 << GLIBTOP_MEM_FREE);

static unsigned needed_swap_flags = 0;

void
GetLoad (int Maximum, int data [4])
{
    static int init = 0;
    int usr, nice, sys, free;
    int total;

    glibtop_cpu cpu;
	
    glibtop_get_cpu (&cpu);
	
    assert ((cpu.flags & needed_cpu_flags) == needed_cpu_flags);
    
    cp_time [0] = cpu.user;
    cp_time [1] = cpu.nice;
    cp_time [2] = cpu.sys;
    cp_time [3] = cpu.idle;

    if (!init) {
	memcpy (last, cp_time, sizeof (last));
	init = 1;
    }

    usr  = cp_time [0] - last [0];
    nice = cp_time [1] - last [1];
    sys  = cp_time [2] - last [2];
    free = cp_time [3] - last [3];

    total = usr + nice + sys + free;

    last [0] = cp_time [0];
    last [1] = cp_time [1];
    last [2] = cp_time [2];
    last [3] = cp_time [3];

    if (!total) total = Maximum;

    usr  = rint (Maximum * (float)(usr)  / total);
    nice = rint (Maximum * (float)(nice) / total);
    sys  = rint (Maximum * (float)(sys)  / total);
    free = rint (Maximum * (float)(free) / total);

    data [0] = usr;
    data [1] = sys;
    data [2] = nice;
    data [3] = free;
}

void
GetMemory (int Maximum, int data [4])
{
    int used, shared, buffer, cached;

    glibtop_mem mem;
	
    glibtop_get_mem (&mem);
	
    assert ((mem.flags & needed_mem_flags) == needed_mem_flags);

    mem.total = mem.free + mem.used + mem.shared +
	mem.buffer + mem.cached;

    used    = rint (Maximum * (float)mem.used   / mem.total);
    shared  = rint (Maximum * (float)mem.shared / mem.total);
    buffer  = rint (Maximum * (float)mem.buffer / mem.total);
    cached  = rint (Maximum * (float)mem.cached / mem.total);

    data [0] = used;
    data [1] = shared;
    data [2] = buffer;
    data [3] = cached;
}

void
GetSwap (int Maximum, int data [2])
{
    int used, free;

    glibtop_swap swap;
	
    glibtop_get_swap (&swap);
	
    assert ((swap.flags & needed_swap_flags) == needed_swap_flags);

    swap.total = swap.free + swap.used;

    if (swap.total == 0) {	/* Avoid division by zero */
	used = free = 0;
	return;
    }

    used    = rint (Maximum * (float)swap.used / swap.total);
    free    = rint (Maximum * (float)swap.free / swap.total);

    data [0] = used;
    data [1] = free;
}

void
GetNet (int Maximum, int data [3])
{
#define SLIP_COUNT	0
#define PPP_COUNT	1
#define OTHER_COUNT	2
#define COUNT_TYPES	3
    int fields[4], present[COUNT_TYPES], delta[COUNT_TYPES], i;
    static int ticks, past[COUNT_TYPES];
    static char *netdevfmt;
    char *cp, buffer[256];
    int found = 0;
    FILE *fp;

    /*
     * We use the maximum number of bits we've seen come through to scale
     * the deltas; thus, the load meter is more like a bandwidth-saturation
     * meter.  Ideally, we'd like to initialize max to the user's link speed
     * in bytes/sec.  If it's set too low, the spikes in the first part of
     * the loadmeter will be too high until we find the maximum burst
     * throughput.  If it's set too high, the spikes will be permanently
     * too small.   We set it a bit below what a 14.4 running SLIP can do.
     */
    static int max = 500;

    if (!netdevfmt)
    {
	FILE *fp = popen("uname -r", "r");	/* still wins if /proc isn't */

	/* pre-linux-2.2 format -- transmit packet count in 8th field */
	netdevfmt = "%d %d %*d %*d %*d %d %*d %d %*d %*d %*d %*d %d";

	if (!fp)
	    return;
	else
	{
	    int major, minor;

	    if (fscanf(fp, "%d.%d.%*d", &major, &minor) != 2)
	    {
		pclose(fp);
		return;
	    }

	    if (major >= 2 && minor >= 2)
		/* Linux 2.2 -- transmit packet count in 10th field */
		netdevfmt = "%d %d %*d %*d %*d %d %*d %*d %*d %*d %d %*d %d";
	    pclose(fp);
	}
    }

    fp = fopen("/proc/net/dev", "r");
    if (!fp)
	return;

    memset(present, '0', sizeof(present));

    while (fgets(buffer, sizeof(buffer) - 1, fp))
    {
	int	*resp;

	for (cp = buffer; *cp == ' '; cp++)
	    continue;

	if (cp[0] == 'l' && cp[1] == 'o')	/* skip loopback device */
	    continue;
	if (cp[0] == 's' && cp[0] == 'l' && cp[0] == 'i' && cp[0] == 'p')
	    resp = present + SLIP_COUNT;
	else if (cp[0] == 'p' && cp[0] == 'p' && cp[0] == 'p')
	    resp = present + PPP_COUNT;
	else
	    resp = present + OTHER_COUNT;

	if ((cp = strchr(buffer, ':')))
	{
	    cp++;
	    if (sscanf(cp, netdevfmt,
		       fields, fields+1, fields+2, 
		       fields+3,&found)>4)
		*resp += fields[0] + fields[2];
	}
    }

    fclose(fp);

    memset(data, '\0', sizeof(data));
    if (ticks++ > 0)		/* avoid initial spike */
    {
	int total = 0;

	for (i = 0; i < COUNT_TYPES; i++)
	{
	    delta[i] = (present[i] - past[i]);
	    total += delta[i];
	}
	if (total > max)
	    max = total;
	for (i = 0; i < COUNT_TYPES; i++)
	    data[i]   = rint (Maximum * (float)delta[i]  / max);

#if 0
	printf("dSLIP: %9d  dPPP: %9d   dOther: %9d, max: %9d\n",
	       delta[SLIP_COUNT], delta[PPP_COUNT], delta[OTHER_COUNT], max);

	printf("vSLIP: %9d  vPPP: %9d   vOther: %9d, Maximum: %9d\n",
	       data[0], data[1], data[2], Maximum);
#endif
    }
    memcpy(past, present, sizeof(present));
}



