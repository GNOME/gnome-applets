#ifndef SWAPLOAD_H__
#define SWAPLOAD_H__

typedef struct _Swapload Swapload;
struct _Swapload {
	GtkWidget *frame, *disp;
	GdkPixmap *pixmap;
	GdkGC *gc;
	GdkColor ucolor, fcolor;
	int timer_index;
};

void swapload_setup_colors (Swapload *swapload);
void swapload_start_timer (Swapload *swapload);

#endif
