#ifndef __DISKUSAGE_H__
#define __DISKUSAGE_H__

#include <glibtop.h>
#include <glibtop/xmalloc.h>
#include <glibtop/fsusage.h>
#include <glibtop/mountlist.h>

#include "properties.h"
#include <applet-widget.h>

#define DU_PIE_GAP 4		/* gap between pie and border */
#define DU_TEXT_GAP 4		/* gap between text and border (on the right) */

#define DU_MOUNTPOINT_X 5	/* offset of Mountpoint text  */
#define DU_MOUNTPOINT_HOR_X 10	/* offset of Mountpoint text  */
#define DU_MOUNTPOINT_Y 15	/* (counting from end of area of piechart) */	

#define DU_FREESPACE_X 5        /* offset of Free Space text */
#define DU_FREESPACE_HOR_X 10	/* offset of Free Space text */
#define DU_FREESPACE_Y 30


#define DU_MOUNTPOINT_Y_VERT 15	
#define DU_MOUNTPOINT2_Y_VERT 30 /* broken into 2 lines when vertical panel */	
#define DU_FREESPACE_Y_VERT 45	
#define DU_FREESPACE2_Y_VERT 60	 /* broken into 2 lines when vertical panel */	


typedef struct _DiskusageInfo DiskusageInfo;
struct _DiskusageInfo {
	unsigned n_filesystems;    /* no. of filesystems with >0 total blocks (/proc has 0) */
	PanelOrientType orient;
	unsigned selected_filesystem;
	unsigned pixel_size;
};

extern GtkWidget *disp;
extern diskusage_properties props;

void setup_colors (void);
void start_timer (void);
void diskusage_resize (void);
void load_my_font (void);

#endif
