#include "diskusage.h"

#include <stdio.h>
#include <unistd.h>

/*
#define DU_DEBUG
*/

void
diskusage_read (DiskusageInfo *ps)
{
	char tmp_filename[200];
	char df_cmdline[200];
	char buffer[200];
	unsigned i;
	FILE *f;
	
/*	sprintf(tmp_filename, "/tmp/df_%d", getpid());*/
/*	sprintf(df_cmdline, "df > %s", tmp_filename); */
/*
#ifdef DU_DEBUG
	printf("tmp_filename: %s \n", tmp_filename);
	printf("df_cmdline: %s \n", df_cmdline);
#endif
*/
/*	system(df_cmdline); */

/*	f = fopen(tmp_filename, "r");*/
	
	f = popen("df", "r");
	
	fgets(buffer, 200, f);


	
	i=0; /* n_filesystems */

	while (fgets(buffer, 200, f) && (i < DU_MAX_FS)) {
/*
#ifdef DU_DEBUG
		printf("%u %s \n", i, buffer);
#endif
*/
		sscanf(buffer, "%*s %u %u %u",
			&ps->filesystems[i].sizeinfo [DU_FS_TOTAL],
			&ps->filesystems[i].sizeinfo [DU_FS_USED],
			&ps->filesystems[i].sizeinfo [DU_FS_FREE]);
		i++;
	}

	
	ps->n_filesystems = i;
#ifdef DU_DEBUG
		printf("n_filesystems = %u \n", i);
#endif

	fclose (f);
/*
	if (remove(tmp_filename) != 0)
		printf("Error removing file %s \n", tmp_filename);
*/

}
