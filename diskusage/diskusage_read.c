#include <config.h>

#include <glibtop.h>
#include <glibtop/fsusage.h>
#include <glibtop/mountlist.h>

#include "diskusage.h"

#include <stdio.h>
#include <unistd.h>

#include <assert.h>

/* #define DU_DEBUG */

/* Many systems reserve some space on each filesystem that only the superuser
 * can use. Set this to true if you want count this space towards the among of
 * free space. This would be the opposite of what 'df' does. */
#undef ADD_RESERVED_SPACE

void
diskusage_read (DiskusageInfo *ps)
{
	glibtop_fsusage fsu;
	glibtop_mountlist mountlist;
	glibtop_mountentry *mount_list;
	unsigned i = 0, index;
	
	/* Get list of currently mounted filesystems. */
	
	mount_list = glibtop_get_mountlist (&mountlist, 0);

	assert (mount_list != NULL);

	for (index = 0; index < mountlist.number; index++) {
#ifdef DU_DEBUG
#if 0
		printf ("Mount_Entry (%d): %-30s %-10s %-20s\n", index,
			mount_list [index].mountdir,
			mount_list [index].type,
			mount_list [index].devname);
#endif
#endif
		
		glibtop_get_fsusage (&fsu, mount_list [index].mountdir);

#if 0		
		assert (fsu.flags != 0);
#endif
		
		fsu.blocks /= 2;
		fsu.bfree /= 2;
		fsu.bavail /= 2;
		
		if (fsu.blocks == 0)
			continue;

		/*
		 * copy dev_name and mount_dir strings
		 */
		if ((ps->filesystems + i)->dev_name)
			free ((ps->filesystems + i)->dev_name);
		(ps->filesystems + i)->dev_name = 
			g_new(gchar, strlen (mount_list [index].devname)+1);
		strcpy ((ps->filesystems + i)->dev_name, 
				mount_list [index].devname);

		if ((ps->filesystems + i)->mount_dir)
			free ((ps->filesystems + i)->mount_dir);
		(ps->filesystems + i)->mount_dir = 
			g_new(gchar, strlen (mount_list [index].mountdir)+1);
		strcpy ((ps->filesystems + i)->mount_dir, 
				mount_list [index].mountdir);


		ps->filesystems[i].sizeinfo [DU_FS_TOTAL] =
			fsu.blocks;
#ifdef ADD_RESERVED_SPACE
		ps->filesystems[i].sizeinfo [DU_FS_FREE] =
			fsu.bfree;
#else
		ps->filesystems[i].sizeinfo [DU_FS_FREE] =
			fsu.bavail;
#endif
		ps->filesystems[i].sizeinfo [DU_FS_USED] =
			fsu.blocks - fsu.bfree;

#ifdef DU_DEBUG
		printf("Usage: %ld, %ld, %ld\n",
		       ps->filesystems[i].sizeinfo [DU_FS_TOTAL],
		       ps->filesystems[i].sizeinfo [DU_FS_FREE],
		       ps->filesystems[i].sizeinfo [DU_FS_USED]);
#endif
		
		/* FIXME
		 * need to make a linked listfor ps->filesystems, to
		 * get rid of hardcoded arraysize
		 */
		if (i >= DU_MAX_FS)
			continue;

		i++;
	}

	ps->n_filesystems = i;

	free (mount_list);
	
#ifdef DU_DEBUG
	printf("n_filesystems = %u \n", i);
#endif
}
