#include "proc.h"

#include <stdio.h>

void
proc_read_cpu (ProcInfo *ps)
{
	int i;
	FILE *f;

	f = fopen ("/proc/stat", "r");

	for (i=1; i<PROC_CPU_SIZE; i++)
		ps->cpu_last [i] = ps->cpu_now [i];

	fscanf (f, "cpu %u %u %u %u\n",
		&ps->cpu_now [PROC_CPU_USER],
		&ps->cpu_now [PROC_CPU_NICE],
		&ps->cpu_now [PROC_CPU_SYS],
		&ps->cpu_now [PROC_CPU_IDLE]);

	for (i=1; i<PROC_CPU_SIZE; i++)
		ps->cpu [i] = ps->cpu_now [i] - ps->cpu_last [i];

	ps->cpu [PROC_CPU_TOTAL] = 
		ps->cpu [PROC_CPU_USER] +
		ps->cpu [PROC_CPU_NICE] +
		ps->cpu [PROC_CPU_SYS] +
		ps->cpu [PROC_CPU_IDLE];

	fclose (f);
}

void
proc_read_mem (ProcInfo *ps)
{
	int i;
	unsigned u;
	FILE *f;

	f = fopen ("/proc/meminfo", "r");

	fscanf (f, "%*[^\n]\nMem: %u %u %u %u %u %u\n",
		&ps->mem [PROC_MEM_TOTAL],
		&ps->mem [PROC_MEM_USED],
		&ps->mem [PROC_MEM_FREE],
		&ps->mem [PROC_MEM_SHARED],
		&ps->mem [PROC_MEM_BUF],
		&u);

	ps->mem [PROC_MEM_USER] = ps->mem [PROC_MEM_TOTAL]
		- ps->mem [PROC_MEM_FREE]
		- ps->mem [PROC_MEM_SHARED];

	/* printf ("Mem: %u %u %u %u %u %u\n",
		ps->mem [PROC_MEM_TOTAL],
		ps->mem [PROC_MEM_USER],
		ps->mem [PROC_MEM_FREE],
		ps->mem [PROC_MEM_SHARED],
		ps->mem [PROC_MEM_BUF],
		u);
		*/

	fscanf (f, "Swap: %u %u %u\n",
		&ps->swap [PROC_SWAP_TOTAL],
		&ps->swap [PROC_SWAP_USED],
		&ps->swap [PROC_SWAP_FREE]);

	/* printf ("Swap: %u %u %u\n",
		ps->swap [PROC_SWAP_TOTAL],
		ps->swap [PROC_SWAP_USED],
		ps->swap [PROC_SWAP_FREE]);
		*/

	fclose (f);
}
