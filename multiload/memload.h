#ifndef MEMLOAD_H__
#define MEMLOAD_H__

typedef struct _Memload Memload;
struct _Memload {
	GtkWidget *frame, *disp;
	GdkPixmap *pixmap;
	GdkGC *gc;
	GdkColor ucolor, scolor;
	GdkColor bcolor, ccolor;
	int timer_index;
};

void memload_setup_colors (Memload *memload);
void memload_start_timer (Memload *memload);

#endif
