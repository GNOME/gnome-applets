#ifndef __DISKUSAGE_H__
#define __DISKUSAGE_H__

#include <applet-widget.h>


#define DU_PIE_GAP 4		/* gap between pie and border */
#define DU_TEXT_GAP 4		/* gap between text and border (on the right) */

#define DU_MOUNTPOINT_X 5	/* offset of Mountpoint text  */
#define DU_MOUNTPOINT_Y 15	/* (counting from end of area of piechart) */	

#define DU_FREESPACE_X 5
#define DU_FREESPACE_Y 30


#define DU_MOUNTPOINT_Y_VERT 15	
#define DU_MOUNTPOINT2_Y_VERT 30 /* broken into 2 lines when vertical panel */	
#define DU_FREESPACE_Y_VERT 45	
#define DU_FREESPACE2_Y_VERT 60	 /* broken into 2 lines when vertical panel */	


#define DU_FS_TOTAL  0
#define DU_FS_USED   1
#define DU_FS_FREE   2

#define FS_SIZE   3

#define DU_MAX_FS 50

typedef struct _FilesystemInfo FilesystemInfo;

struct _FilesystemInfo {
	unsigned sizeinfo [FS_SIZE];
	gchar *dev_name;
	gchar *mount_dir;
};

typedef struct _DiskusageInfo DiskusageInfo;

struct _DiskusageInfo {
	FilesystemInfo filesystems [DU_MAX_FS];
	unsigned n_filesystems;
	PanelOrientType orient;
	unsigned selected_filesystem;
};


void diskusage_read (DiskusageInfo *);

#endif
