/* From wmload.c, v0.9.2, licensed under the GPL. */
#include <config.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#if defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/socket.h>
#include <net/if.h>
#endif

#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <glibtop/loadavg.h>
#include <glibtop/netload.h>
#include <glibtop/netlist.h>

#include "linux-proc.h"

static const unsigned needed_cpu_flags =
(1 << GLIBTOP_CPU_USER) +
(1 << GLIBTOP_CPU_IDLE);

static const unsigned needed_page_flags =
(1 << GLIBTOP_SWAP_PAGEIN) +
(1 << GLIBTOP_SWAP_PAGEOUT);

static const unsigned needed_mem_flags =
(1 << GLIBTOP_MEM_USED) +
(1 << GLIBTOP_MEM_FREE);

static const unsigned needed_swap_flags =
(1 << GLIBTOP_SWAP_USED) +
(1 << GLIBTOP_SWAP_FREE);

static const unsigned needed_loadavg_flags =
(1 << GLIBTOP_LOADAVG_LOADAVG);

static const unsigned needed_netload_flags =
(1 << GLIBTOP_NETLOAD_IF_FLAGS) +
(1 << GLIBTOP_NETLOAD_BYTES_TOTAL);


void
GetLoad (int Maximum, int data [4], LoadGraph *g)
{
    int usr, nice, sys, free;
    int total;

    glibtop_cpu cpu;
	
    glibtop_get_cpu (&cpu);
	
    g_return_if_fail ((cpu.flags & needed_cpu_flags) == needed_cpu_flags);
    
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

#if 0
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
#endif /* 0 */

void
GetMemory (int Maximum, int data [5], LoadGraph *g)
{
    int user, shared, buffer, cached;
    
    glibtop_mem mem;
	
    glibtop_get_mem (&mem);
	
    g_return_if_fail ((mem.flags & needed_mem_flags) == needed_mem_flags);

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
    g_return_if_fail ((swap.flags & needed_swap_flags) == needed_swap_flags);

    if (swap.total == 0) {
	used = 0;
	free = Maximum;
    }
    else {
	used    = rint (Maximum * (float)swap.used / swap.total);
	free    = rint (Maximum * (float)swap.free / swap.total);
    }

    data [0] = used;
    data [1] = free;
}

void
GetLoadAvg (int Maximum, int data [2], LoadGraph *g)
{
    const float max_loadavg = 10.0f;

    float loadavg1;
    float used, free;

    glibtop_loadavg loadavg;
    glibtop_get_loadavg (&loadavg);

    g_return_if_fail ((loadavg.flags & needed_loadavg_flags) == needed_loadavg_flags);

    loadavg1 = MIN(loadavg.loadavg[0], max_loadavg);

    used    = loadavg1 / max_loadavg;
    free    = (max_loadavg - loadavg1) / max_loadavg;

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

    static int ticks = 0;
    static int max = 500;
    static gulong past[COUNT_TYPES] = {0};

    gulong present[COUNT_TYPES] = {0};

    int delta[COUNT_TYPES], i;
    gchar **devices;
    glibtop_netlist netlist;


    devices = glibtop_get_netlist(&netlist);

    for(i = 0; i < netlist.number; ++i)
    {
	int index;
	glibtop_netload netload;

	glibtop_get_netload(&netload, devices[i]);

	g_return_if_fail((netload.flags & needed_netload_flags) == needed_netload_flags);

	if (!(netload.if_flags & (1L << GLIBTOP_IF_FLAGS_UP)))
	    continue;

	if (netload.if_flags & (1L << GLIBTOP_IF_FLAGS_LOOPBACK))
	    continue;

	if (netload.if_flags & (1L << GLIBTOP_IF_FLAGS_POINTOPOINT))
	{
	    if (g_str_has_prefix(devices[i], "sl"))
		index = SLIP_COUNT;
	    else
		index = PPP_COUNT;
	}
	else if (g_str_has_prefix(devices[i], "eth"))
	    index = ETH_COUNT;
	else
	    index = OTHER_COUNT;

	present[index] += netload.bytes_total;
    }

    g_strfreev(devices);


    memset(data, 0, 5 * sizeof data[0]);

    if(ticks < 2) /* avoid initial spike */
    {
	ticks++;
    }
    else
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

    memcpy(past, present, sizeof past);
}



