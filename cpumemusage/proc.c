#include <config.h>

#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>

#include <assert.h>

#include "proc.h"

#include <stdio.h>

static unsigned needed_cpu_flags =
(1 << GLIBTOP_CPU_USER) +
(1 << GLIBTOP_CPU_IDLE);

static unsigned needed_mem_flags =
(1 << GLIBTOP_MEM_TOTAL) +
(1 << GLIBTOP_MEM_USED) +
(1 << GLIBTOP_MEM_FREE);

static unsigned needed_swap_flags = 0;

void
proc_read_cpu (ProcInfo *ps)
{
	glibtop_cpu cpu;
	int i;

	for (i=1; i<PROC_CPU_SIZE; i++)
		ps->cpu_last [i] = ps->cpu_now [i];

	glibtop_get_cpu (&cpu);
	
	assert ((cpu.flags & needed_cpu_flags) == needed_cpu_flags);
	
	ps->cpu_now [PROC_CPU_USER] = cpu.user;
	ps->cpu_now [PROC_CPU_NICE] = cpu.nice;
	ps->cpu_now [PROC_CPU_SYS] = cpu.sys;
	ps->cpu_now [PROC_CPU_IDLE] = cpu.idle;
	
	for (i=1; i<PROC_CPU_SIZE; i++)
		ps->cpu [i] = ps->cpu_now [i] - ps->cpu_last [i];
	
	ps->cpu [PROC_CPU_TOTAL] = 
		ps->cpu [PROC_CPU_USER] +
		ps->cpu [PROC_CPU_NICE] +
		ps->cpu [PROC_CPU_SYS] +
		ps->cpu [PROC_CPU_IDLE];
}

void
proc_read_mem (ProcInfo *ps)
{
	glibtop_mem mem;
	glibtop_swap swap;
	/* int i; */

	glibtop_get_mem (&mem);

	assert ((mem.flags & needed_mem_flags) == needed_mem_flags);

	ps->mem [PROC_MEM_TOTAL] = mem.total;
	ps->mem [PROC_MEM_USED] = mem.used;
	ps->mem [PROC_MEM_FREE] = mem.free;
	ps->mem [PROC_MEM_SHARED] = mem.shared;
	ps->mem [PROC_MEM_BUF] = mem.buffer;
	
	ps->mem [PROC_MEM_USER] = ps->mem [PROC_MEM_TOTAL]
		- ps->mem [PROC_MEM_FREE]
		- ps->mem [PROC_MEM_SHARED];

	/* printf ("Mem: %u %u %u %u %u\n",
		ps->mem [PROC_MEM_TOTAL],
		ps->mem [PROC_MEM_USER],
		ps->mem [PROC_MEM_FREE],
		ps->mem [PROC_MEM_SHARED],
		ps->mem [PROC_MEM_BUF]);
		*/

	glibtop_get_swap (&swap);

	assert ((swap.flags & needed_swap_flags) == needed_swap_flags);

	ps->swap [PROC_SWAP_TOTAL] = swap.total;
	ps->swap [PROC_SWAP_USED] = swap.used;
	ps->swap [PROC_SWAP_FREE] = swap.free;

	/* printf ("Swap: %u %u %u\n",
		ps->swap [PROC_SWAP_TOTAL],
		ps->swap [PROC_SWAP_USED],
		ps->swap [PROC_SWAP_FREE]);
		*/
}
