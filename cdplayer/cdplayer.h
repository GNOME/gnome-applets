#if !defined(__CDPLAYER_H__)
#define __CDPLAYER_H__

typedef struct {
	int timeout;
	cdrom_device_t cdrom_device;
	int size;
	PanelOrientType orient;
	char *devpath;

	struct {
		GtkWidget *frame;
		/* Main box for all bits 
		   changes depending on orientation,size,etc. */ 
		GtkWidget *box;

		/* Time display LED */
		GtkWidget *time;

		/* Box holding the track stuff */
		struct {
			GtkWidget *display;
			GtkWidget *prev;
			GtkWidget *next;
		} track_control; 

		/* Box holding the play controls */
		struct {
			GtkWidget *play_pause;
			GtkWidget *stop;
			GtkWidget *eject;
		} play_control;
	} panel;
} CDPlayerData;

#endif
