/*
 * GNOME time/date display module.
 * (C) 1997 The Free Software Foundation
 *
 * Authors: Miguel de Icaza
 *          Federico Mena
 *
 * Feel free to implement new look and feels :-)
 */

#include <stdio.h>
#ifdef HAVE_LIBINTL
#    include <libintl.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <gnome.h>
#include <gdk/gdkx.h>
#include "applet-lib.h"
#include "applet-widget.h"

typedef void (*ClockUpdateFunc) (GtkWidget *, time_t);

typedef struct {
	int timeout;
	ClockUpdateFunc update_func;
	PanelOrientType orient;
} ClockData;

typedef struct {
	GtkWidget *date;
	GtkWidget *time;
} ComputerClock;

GtkWidget *applet = NULL;
GtkWidget *clockw = NULL;

ClockData *cd = NULL;

static void
free_data(GtkWidget * widget, gpointer data)
{
	g_free(data);
}

static int
clock_timeout_callback(gpointer data)
{
	time_t current_time;

	time(&current_time);

	(*cd->update_func) (clockw, current_time);

	return 1;
}

static void
computer_clock_update_func(GtkWidget *clock, time_t current_time)
{
	ComputerClock *cc;
	char *strtime;
	char date[20], hour[20];

	cc = gtk_object_get_user_data(GTK_OBJECT(clock));

	strtime = ctime(&current_time);

	if(cd->orient == ORIENT_LEFT || cd->orient == ORIENT_RIGHT)
		strtime[3] ='\n';
	strncpy(date, strtime, 10);
	date[10] = '\0';
	gtk_label_set(GTK_LABEL(cc->date), date);

	strtime += 11;
	strncpy(hour, strtime, 5);
	hour[5] = '\0';
	gtk_label_set(GTK_LABEL(cc->time), hour);
}

static void
create_computer_clock_widget(GtkWidget ** clock, ClockUpdateFunc * update_func)
{
	GtkWidget *frame;
	GtkWidget *align;
	GtkWidget *vbox;
	ComputerClock *cc;

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_widget_show(frame);

	align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
	gtk_container_border_width(GTK_CONTAINER(align), 4);
	gtk_container_add(GTK_CONTAINER(frame), align);
	gtk_widget_show(align);

	vbox = gtk_vbox_new(FALSE, FALSE);
	gtk_container_add(GTK_CONTAINER(align), vbox);
	gtk_widget_show(vbox);

	cc = g_new(ComputerClock, 1);
	cc->date = gtk_label_new("");
	cc->time = gtk_label_new("");

	gtk_box_pack_start_defaults(GTK_BOX(vbox), cc->date);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), cc->time);
	gtk_widget_show(cc->date);
	gtk_widget_show(cc->time);

	gtk_object_set_user_data(GTK_OBJECT(frame), cc);
	gtk_signal_connect(GTK_OBJECT(frame), "destroy",
			   (GtkSignalFunc) free_data,
			   cc);

	*clock = frame;
	*update_func = computer_clock_update_func;
}

static void
destroy_clock(GtkWidget * widget, void *data)
{
	gtk_timeout_remove(cd->timeout);
	g_free(cd);
}

static GtkWidget *
create_clock_widget(void)
{
	GtkWidget *clock;
	time_t current_time;

	cd = g_new(ClockData, 1);

	/*FIXME: different clock types here */
	create_computer_clock_widget(&clock, &cd->update_func);

	/* Install timeout handler */

	cd->timeout = gtk_timeout_add(3000, clock_timeout_callback, clock);

	cd->orient = ORIENT_UP;

	gtk_signal_connect(GTK_OBJECT(clock), "destroy",
			   (GtkSignalFunc) destroy_clock,
			   NULL);
	/* Call the clock's update function so that it paints its first state */

	time(&current_time);

	(*cd->update_func) (clock, current_time);

	return clock;
}

/*these are commands sent over corba: */
static void
applet_change_orient(GtkWidget *w, PanelOrientType o, gpointer data)
{
	time_t current_time;

	time(&current_time);
	cd->orient = o;
	(*cd->update_func) (clockw, current_time);
}

static gint
destroy_applet(GtkWidget *widget, gpointer data)
{
	gtk_exit(0);
	return FALSE;
}

/*
void
test_callback(int id, gpointer data)
{
	puts("TEST");
}
*/


int
main(int argc, char **argv)
{
	panel_corba_register_arguments();
	gnome_init("clock_applet", NULL, argc, argv, 0, NULL);

	applet = applet_widget_new(argv[0]);
	if (!applet)
		g_error("Can't create applet!\n");

	clockw = create_clock_widget();
	gtk_widget_show(clockw);
	applet_widget_add(APPLET_WIDGET(applet), clockw);
	gtk_widget_show(applet);
	gtk_signal_connect(GTK_OBJECT(applet),"destroy",
			   GTK_SIGNAL_FUNC(destroy_applet),
			   NULL);
	gtk_signal_connect(GTK_OBJECT(applet),"change_orient",
			   GTK_SIGNAL_FUNC(applet_change_orient),
			   NULL);

/*
	gnome_panel_applet_register_callback(applet_id,
					     "test",
					     "TEST CALLBACK",
					     test_callback,
					     NULL);
*/

	applet_widget_gtk_main();

	return 0;
}
