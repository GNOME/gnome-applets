#ifndef __DISKUSAGE_H__
#define __DISKUSAGE_H__

#include "applet-lib.h"
#include "applet-widget.h"

#define DU_UPDATE_TIME 2000

/* default height when horizontal panel */
#define DU_FRAME_HEIGHT 42

/* default width when vertical panel */
#define DU_FRAME_WIDTH 44

/* width of a bar when horiz. pan. */
#define DU_BAR_WIDTH 12

/* height of a bar when ver. pan. */
#define DU_BAR_HEIGHT 10

#define DU_FS_TOTAL  0
#define DU_FS_USED   1
#define DU_FS_FREE   2

#define FS_SIZE   3

#define DU_MAX_FS 10

typedef struct _FilesystemInfo FilesystemInfo;

struct _FilesystemInfo {
	unsigned sizeinfo [FS_SIZE];
};

typedef struct _DiskusageInfo DiskusageInfo;

struct _DiskusageInfo {
	FilesystemInfo filesystems [DU_MAX_FS];
	unsigned n_filesystems;
	unsigned previous_n_filesystems;
	PanelOrientType orient;
};


void diskusage_read (DiskusageInfo *);

#endif
