#if !defined(__CDPLAYER_H__)
#define __CDPLAYER_H__

typedef struct {
  int timeout;
  cdrom_device_t cdrom_device;

  struct {
    GtkWidget *time;
    GtkWidget *track;

    /* control button */
    GtkWidget *play_pause;
    GtkWidget *stop;
    GtkWidget *prev;
    GtkWidget *next;
    GtkWidget *eject;
  } panel;
} CDPlayerData;



#endif
