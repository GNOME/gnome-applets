#ifndef __DISKUSAGE_READ_H__
#define __DISKUSAGE_READ_H__

#define DU_FS_TOTAL  0
#define DU_FS_USED   1
#define DU_FS_FREE   2

#define FS_SIZE   3

#define DU_FS_SIZE 1

typedef struct _FilesystemInfo FilesystemInfo;

struct _FilesystemInfo {
	unsigned filesystem [FS_SIZE];
};

typedef struct _DiskusageInfo DiskusageInfo;

struct _DiskusageInfo {
	FilesystemInfo filesystems [DU_FS_SIZE];
};


void diskusage_read (DiskusageInfo *);

#endif
