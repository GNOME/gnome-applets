/* From wmload.c, v0.9.2, licensed under the GPL. */
#include <config.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#ifdef __FreeBSD__
#include <sys/socket.h>
#include <net/if.h>
#endif

#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <glibtop/loadavg.h>
#include <glibtop/netload.h>

#include "linux-proc.h"

static unsigned needed_cpu_flags =
(1 << GLIBTOP_CPU_USER) +
(1 << GLIBTOP_CPU_IDLE);

static unsigned needed_page_flags = 
(1 << GLIBTOP_SWAP_PAGEIN) +
(1 << GLIBTOP_SWAP_PAGEOUT);

static unsigned needed_mem_flags =
(1 << GLIBTOP_MEM_USED) +
(1 << GLIBTOP_MEM_FREE);

static unsigned needed_swap_flags =
(1 << GLIBTOP_SWAP_USED) +
(1 << GLIBTOP_SWAP_FREE);

static unsigned needed_loadavg_flags =
(1 << GLIBTOP_LOADAVG_LOADAVG);

static unsigned needed_netload_flags =
(1 << GLIBTOP_NETLOAD_IF_FLAGS) +
(1 << GLIBTOP_NETLOAD_BYTES_TOTAL);

void
GetLoad (int Maximum, int data [4], LoadGraph *g)
{
    int usr, nice, sys, free;
    int total;

    glibtop_cpu cpu;
	
    glibtop_get_cpu (&cpu);
	
    assert ((cpu.flags & needed_cpu_flags) == needed_cpu_flags);
    
    g->cpu_time [0] = cpu.user;
    g->cpu_time [1] = cpu.nice;
    g->cpu_time [2] = cpu.sys;
    g->cpu_time [3] = cpu.idle;

    if (!g->cpu_initialized) {
	memcpy (g->cpu_last, g->cpu_time, sizeof (g->cpu_last));
	g->cpu_initialized = 1;
    }

    usr  = g->cpu_time [0] - g->cpu_last [0];
    nice = g->cpu_time [1] - g->cpu_last [1];
    sys  = g->cpu_time [2] - g->cpu_last [2];
    free = g->cpu_time [3] - g->cpu_last [3];

    total = usr + nice + sys + free;

    g->cpu_last [0] = g->cpu_time [0];
    g->cpu_last [1] = g->cpu_time [1];
    g->cpu_last [2] = g->cpu_time [2];
    g->cpu_last [3] = g->cpu_time [3];

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
GetPage (int Maximum, int data [3], LoadGraph *g)
{
    static int max = 100; /* guess at maximum page rate (= in + out) */
    static u_int64_t lastin = 0;
    static u_int64_t lastout = 0;
    int in, out, idle;

    glibtop_swap swap;
	
    glibtop_get_swap (&swap);
	
    assert ((swap.flags & needed_page_flags) == needed_page_flags);

    if ((lastin > 0) && (lastin < swap.pagein)) {
	in = swap.pagein - lastin;
    }
    else {
	in = 0;
    }
    lastin = swap.pagein;

    if ((lastout > 0) && (lastout < swap.pageout)) {
	out = swap.pageout - lastout;
    }
    else {
	out = 0;
    }
    lastout = swap.pageout;

    if ((in + out) > max) {
	/* Maximum page rate has increased. Change the scale without
	   any indication whatsoever to the user (not a nice thing to
	   do). */
	max = in + out;
    }

    in   = rint (Maximum * ((float)in / max));
    out  = rint (Maximum * ((float)out / max));
    idle = Maximum - in - out;

    data [0] = in;
    data [1] = out;
    data [2] = idle;
}

void
GetMemory (int Maximum, int data [5], LoadGraph *g)
{
    int user, shared, buffer, cached;
    
    glibtop_mem mem;
	
    glibtop_get_mem (&mem);
	
    assert ((mem.flags & needed_mem_flags) == needed_mem_flags);

    user    = rint (Maximum * (float)mem.user / (float)mem.total);
    shared  = rint (Maximum * (float)mem.shared / (float)mem.total);
    buffer  = rint (Maximum * (float)mem.buffer / (float)mem.total);
    cached = rint (Maximum * (float)mem.cached / (float)mem.total);
    
    data [0] = user;
    data [1] = shared;
    data [2] = buffer;
    data[3] = cached;
    data [4] = Maximum-user-shared-buffer-cached;
}

void
GetSwap (int Maximum, int data [2], LoadGraph *g)
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
GetLoadAvg (int Maximum, int data [2], LoadGraph *g)
{
    float used, free;
    float max_loadavg = 10.0;
    int index;

    glibtop_loadavg loadavg;
	
    glibtop_get_loadavg (&loadavg);
	
    assert ((loadavg.flags & needed_loadavg_flags) == needed_loadavg_flags);

	index = 0;
	
/* i need to figure out what this means... */
/*
    switch (g->prop_data->loadavg_type) {
    case LOADAVG_1:
	index = 0;
	break;
    case LOADAVG_5:
	index = 1;
	break;
    case LOADAVG_15:
	index = 2;
	break;
    default:
	g_assert_not_reached ();
	return;
    }
*/
/*
    if (g->prop_data_ptr->adj_data [2])
	max_loadavg = (float) g->prop_data_ptr->adj_data [2];
*/
    if (loadavg.loadavg [index] > max_loadavg)
	loadavg.loadavg [index] = max_loadavg;

    used    = (loadavg.loadavg [index]) / max_loadavg;
    free    = (max_loadavg - loadavg.loadavg [index]) / max_loadavg;

    data [0] = rint ((float) Maximum * used);
    data [1] = rint ((float) Maximum * free);
}

void
GetNet (int Maximum, int data [5], LoadGraph *g)
{
#define SLIP_COUNT	0
#define PPP_COUNT	1
#define ETH_COUNT       2
#define OTHER_COUNT	3
#define COUNT_TYPES	4
    gulong inbytes, outbytes;
    gulong present[COUNT_TYPES];
    int delta[COUNT_TYPES], i;
    static int ticks = 0;
    static gulong past[COUNT_TYPES] = {0};
#ifdef __FreeBSD__
    struct if_nameindex *ifindex, *ifptr;
    static int max = 500;

    ifindex = if_nameindex();
    if (!ifindex)
        return;

    memset(present, 0, sizeof (present));

    for (ifptr = ifindex; ifptr->if_index && ifptr->if_name; ifptr++)
    {
        int index;
        glibtop_netload netload; 

        glibtop_get_netload(&netload, ifptr->if_name);
        if (!netload.flags)
            continue;

        assert ((netload.flags & needed_netload_flags) == needed_netload_flags);

        if (!(netload.if_flags & (1L << GLIBTOP_IF_FLAGS_UP)))
            continue;
        if (netload.if_flags & (1L << GLIBTOP_IF_FLAGS_LOOPBACK))
            continue;

        if (netload.if_flags & (1L << GLIBTOP_IF_FLAGS_POINTOPOINT)) {
            index = strncmp(ifptr->if_name, "sl", 2) ? PPP_COUNT : SLIP_COUNT;
        } else {
            index = ETH_COUNT;
        }

        present[index] += netload.bytes_total;
    }

    if_freenameindex(ifindex);    

#else
    static char *netdevfmt = NULL;
    char *cp, buffer[256];
    FILE *fp;
    
    /* FIXME: this is hosed for multiple applets with the static variables */

    /* FIXME: This function is generally incredibly evil -George
       (too lazy to rewrite it though) */

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

	/* pre-linux-2.2 format -- transmit packet count in 8th field (bytes in 1st and 7th) */
	netdevfmt = "%lu %*d %*d %*d %*d %*d %lu";

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
		/* Linux 2.2 -- transmit packet count in 10th field (bytes in 1st and 9th) */
		netdevfmt = "%lu %*d %*d %*d %*d %*d %*d %*d %lu";
	    pclose(fp);
	}
    }

    fp = fopen("/proc/net/dev", "r");
    if (!fp)
	return;

    for (i = 0; i < COUNT_TYPES; i++)
	    present[i] = 0;

    while (fgets(buffer, sizeof(buffer) - 1, fp))
    {
	int index;

	for (cp = buffer; *cp == ' '; cp++)
	    continue;

	if (strncmp (cp, "lo", 2) == 0) {
		continue;
	} else if (strncmp (cp, "slip", 4) == 0) {
		index = SLIP_COUNT;
	} else if (strncmp (cp, "ppp", 3) == 0) {
		index = PPP_COUNT;
	} else if (strncmp (cp, "eth", 3) == 0) {
		index = ETH_COUNT;
	} else {
      		index = OTHER_COUNT;
	}

	if ((cp = strchr(buffer, ':')))
	{
	    cp++;
	    if ( sscanf(cp, netdevfmt, &inbytes, &outbytes) == 2 )
		present[index] += inbytes + outbytes;
	}
    }

    fclose(fp);
#endif

    for (i = 0; i < 5; i++)
	    data[i] = 0;
    if (ticks++ > 0)		/* avoid initial spike */
    {
	int total = 0;

	for (i = 0; i < COUNT_TYPES; i++)
	{
	    /* protect against weirdness */
	    if (present[i] >= past[i])
		    delta[i] = (present[i] - past[i]);
	    else
		    delta[i] = 0;
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
    data[4] = Maximum - data[3] - data[2] - data[1] - data[0];

    for (i = 0; i < COUNT_TYPES; i++)
	    past[i] = present[i];
}



