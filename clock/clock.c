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
#include "gnome.h"
#include "../panel_cmds.h"
#include "../applet_cmds.h"
#include "../panel.h"


#define APPLET_ID "Clock"

#define CLOCK_DATA "clock_data"


static PanelCmdFunc panel_cmd_func;

gpointer applet_cmd_func(AppletCommand *cmd);


typedef void (*ClockUpdateFunc) (GtkWidget *clock, time_t current_time);


typedef struct {
	int timeout;
	ClockUpdateFunc update_func;
} ClockData;

typedef struct {
	GtkWidget *date;
	GtkWidget *time;
} ComputerClock;


static void
free_data(GtkWidget *widget, gpointer data)
{
	g_free(data);
}

static int
clock_timeout_callback (gpointer data)
{
	time_t     current_time;
	GtkWidget *clock;
	ClockData *cd;
	
	time (&current_time);

	clock = data;
	cd = gtk_object_get_data(GTK_OBJECT(clock), CLOCK_DATA);

	(*cd->update_func) (clock, current_time);

	return 1;
}

static void
computer_clock_update_func(GtkWidget *clock, time_t current_time)
{
	ComputerClock *cc;
	char          *strtime;
	char           date[20], hour[20];

	cc = gtk_object_get_user_data(GTK_OBJECT(clock));

	strtime = ctime (&current_time);

	strncpy (date, strtime, 10);
	date[10] = '\0';
	gtk_label_set (GTK_LABEL (cc->date), date);

	strtime += 11;
	strncpy (hour, strtime, 5);
	hour[5] = '\0';
	gtk_label_set (GTK_LABEL (cc->time), hour);
}

static void
create_computer_clock_widget(GtkWidget **clock, ClockUpdateFunc *update_func)
{
	GtkWidget     *frame;
	GtkWidget     *align;
	GtkWidget     *vbox;
	ComputerClock *cc;

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
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
destroy_clock (GtkWidget *widget, void *data)
{
	ClockData *cd;

	cd = gtk_object_get_data(GTK_OBJECT(widget), CLOCK_DATA);
	
	gtk_timeout_remove (cd->timeout);

	g_free(cd);
}

static GtkWidget *
create_clock_widget (GtkWidget *window, char *params)
{
	ClockData       *cd;
	GtkWidget       *clock;
	time_t           current_time;

	cd = g_new(ClockData, 1);

	/* FIXME: for now we ignore the params (which maybe specify
	 * the clock type) and just create a simple computer clock
	 */

	create_computer_clock_widget(&clock, &cd->update_func);

	/* Install timeout handler */

	cd->timeout = gtk_timeout_add(3000, clock_timeout_callback, clock);

	gtk_object_set_data(GTK_OBJECT(clock), CLOCK_DATA, cd);

	gtk_signal_connect(GTK_OBJECT(clock), "destroy",
			   (GtkSignalFunc) destroy_clock,
			   NULL);

	/* Call the clock's update function so that it paints its first state */

	time(&current_time);
	
	(*cd->update_func) (clock, current_time);

	return clock;
}

static void
create_instance (Panel *panel, char *params, int xpos, int ypos)
{
	PanelCommand cmd;
	GtkWidget *clock;

	clock = create_clock_widget (panel->window, params);
	cmd.cmd = PANEL_CMD_REGISTER_TOY;
	cmd.params.register_toy.applet = clock;
	cmd.params.register_toy.id     = APPLET_ID;
	cmd.params.register_toy.xpos   = xpos;
	cmd.params.register_toy.ypos   = ypos;
	cmd.params.register_toy.flags  = APPLET_HAS_PROPERTIES;

	(*panel_cmd_func) (&cmd);
}

gpointer
applet_cmd_func(AppletCommand *cmd)
{
	g_assert(cmd != NULL);

	switch (cmd->cmd) {
		case APPLET_CMD_QUERY:
			return APPLET_ID;

		case APPLET_CMD_INIT_MODULE:
			panel_cmd_func = cmd->params.init_module.cmd_func;
			break;

		case APPLET_CMD_DESTROY_MODULE:
			break;

		case APPLET_CMD_GET_DEFAULT_PARAMS:
			return g_strdup("");

		case APPLET_CMD_CREATE_INSTANCE:
			create_instance(cmd->panel,
					cmd->params.create_instance.params,
					cmd->params.create_instance.xpos,
					cmd->params.create_instance.ypos);
			break;

		case APPLET_CMD_GET_INSTANCE_PARAMS:
			return g_strdup("");

		case APPLET_CMD_PROPERTIES:
			fprintf(stderr, "Clock properties not yet implemented\n"); /* FIXME */
			break;

		default:
			fprintf(stderr,
				APPLET_ID " applet_cmd_func: Oops, unknown command type %d\n",
				(int) cmd->cmd);
			break;
	}

	return NULL;
}
