#if !defined(__LED_H__)
#define __LED_H__

#define DIGIT_WIDTH 9
#define LED_HEIGHT 11
#define LED_WIDTH (DIGIT_WIDTH * 5)

void led_init(GtkWidget * window);
void led_done(void);

void led_create_widget(GtkWidget * window, GtkWidget ** time,
		       GtkWidget ** track);
void led_time(GtkWidget * time, int min, int sec, GtkWidget * track,
	      int trackno);
void led_stop(GtkWidget * time, GtkWidget * track);
void led_pause(GtkWidget * time, int min, int sec, GtkWidget * track,
	       int trackno);
void led_nodisc(GtkWidget * time, GtkWidget * track);

void led_paused(GtkWidget * time, int min, int sec, GtkWidget * track, int trackno);

#endif
