#ifndef CPULOAD_H__
#define CPULOAD_H__

typedef struct _Cpuload Cpuload;
struct _Cpuload {
	GtkWidget *frame, *disp;
	GdkPixmap *pixmap;
	GdkGC *gc;
	GdkColor ucolor, scolor;
	int timer_index;
};

void cpuload_setup_colors (Cpuload *cpuload);
void cpuload_start_timer (Cpuload *cpuload);

#endif
