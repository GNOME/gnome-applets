/*
 * GNOME battery monitor display module.
 * (C) 1997 The Free Software Foundation
 *
 * Author: Micah Stetson
 *
 * Much copied from Miguel de Icaza's clock applet.
 * Fixed for new panel API and i18n added by Federico Mena.
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include "gnome.h"
#include "../panel_cmds.h"
#include "../applet_cmds.h"
#include "../panel.h"


#define APPLET_ID "Battery Monitor"

#define TIMEOUT 5000  /* ms */

#define PROC_APM "/proc/apm"

static PanelCmdFunc panel_cmd_func;

static GtkTooltips *tooltips;

static char *ac_pixmap_filename;
static char *bat_pixmap_filename;

static GtkWidget *window;
static GtkWidget *batcharge;
static GtkWidget *statlabel;
static GtkWidget *minlabel;

/* On Linux we also show the APM driver version number and the
   APM BIOS version number */
#ifdef	__linux__
static GtkWidget *driverlabel;
static GtkWidget *bioslabel;
#endif	/* __linux__ */

static gint batmon_timeout = -1;


/* Masks for batflag */
enum {
	HIGH_MASK = 1,
	LOW_MASK = 2,
	CRIT_MASK = 4,
	CHARGE_MASK = 8,
	NOEXIST_MASK = 128
};


static gint
batmon_timeout_callback (gpointer *data)
{
	gboolean	linestat = TRUE;
	gint		batflag;
	gint		batpct;
	gint		batmin;
	gchar		str[30];
	gchar		*fname;
	
/* In an effort to be platform independent, the section of code below
   (up to the #endif) is the only Linux-specific code in this function
   to make this work with other Unices, just add a #ifdef for your
   platform and write some code that gets the data and sets linestat
   to TRUE if we are connected to an AC line, FALSE otherwise, sets batflag
   to represent the battery status whether high, low, critical, charging,
   or nonexistent (use the bitmasks above, or'ed together for
   combinations of these (i.e. low and charging would be
   (LOW_MASK | CHARGE_MASK))), sets batpct to a number from 0-100 to
   represent percentage of battery charge (-1 if unknown), and sets
   batmin to the number of minutes of life the battery has left.  If
   there is a better way to make this work on Unices other than Linux (most
   probable), please tell me about it. */
#ifdef __linux__
	FILE		*APM;
	gchar		*string[9];
	gchar		*mempoint;
	gchar		*apmstr;
	gint		i;
	int             intval;
	
	/* We have two pointers because strsep() changes the
	   pointer to point to a different position in the
	   string.  If we only had one, we couldn't free() it
	   properly. */
	mempoint = g_malloc((45 * sizeof(gchar)));
	apmstr = mempoint;
	
	if (!(APM = fopen (PROC_APM, "r"))) {
		g_warning (_("Can't open /proc/apm; can't get data."));
		/* returning FALSE will remove our timout */
		return FALSE;
	}
	
	if (!(fgets (apmstr, 45, APM))) {
		g_warning (_("Something wrong with /proc/apm; can't get data."));
		/* returning FALSE will remove our timout */
		return FALSE;
	}
	
	fclose (APM);
	
	for (i = 0; i < 9; ++i) {
		string[i] = strsep (&apmstr, " ");
	}
	
	/* string[3] is "0x00" if we aren't connected, "0x01" if we are */
	sscanf (string[3], "%x", &intval);
	linestat = intval;
	/* In the Linux source tree, drivers/char/apm_bios.c says that
	   in its flag, bit 0 signifies High, 1 Low, 2 Critical,
	   3 Charging and 7 Nonexistent.  I have tried to make this the
	   same as the meaning of my batflag variable.  The question is,
	   will a batflag equal to the hexadecimal number printed by
	   the APM driver represent the same bits on a big-endian machine
	   as a little one?  On the other hand, does the Linux APM driver
	   work on non-Intel machines?  If someone knows the answer to
	   these questions and/or can give me a better solution, I'd be
	   very glad to hear about it. */
	sscanf (string[5], "%x", &batflag);
	/* remove '%' from string */
	string[6][strlen (string[6]) - 1] = '\0';
	sscanf (string[6], "%i", &batpct);
	sscanf (string[7], "%i", &batmin);
	if (strncmp (string[8], "sec", 3) == 0)
		batmin /= 60;
	
	/* As stated above, we show the APM Driver and APM BIOS versions. */
	gtk_label_set (GTK_LABEL (driverlabel), string[0]);
	gtk_label_set (GTK_LABEL (bioslabel), string[1]);
	
	g_free (mempoint);
#else
	batflag = NOEXIST_MASK;
	batpct = 100;
	batmin = 99999999; /* some random number */
	linestat = 1;
#endif /* __linux__ */

	/* I've never done this before; is there a better way to
	   check which flags are set? */
	switch (batflag) {
		case NOEXIST_MASK:
			gtk_label_set (GTK_LABEL (statlabel), _("There is no battery?!?"));
			break;
		case HIGH_MASK | CHARGE_MASK:
			gtk_label_set (GTK_LABEL (statlabel), _("High and charging."));
			break;
		case HIGH_MASK:
			gtk_label_set (GTK_LABEL (statlabel), _("High."));
			break;
		case LOW_MASK | CHARGE_MASK:
			gtk_label_set (GTK_LABEL (statlabel), _("Low and charging."));
			break;
		case LOW_MASK:
			gtk_label_set (GTK_LABEL (statlabel), _("Low."));
			break;
		case CRIT_MASK | CHARGE_MASK:
			gtk_label_set (GTK_LABEL (statlabel), _("Critical and charging."));
			break;
		case CRIT_MASK:
			gtk_label_set (GTK_LABEL (statlabel), _("Critical!!"));
			break;
		case CHARGE_MASK:
			gtk_label_set (GTK_LABEL (statlabel), _("Charging."));
			break;
	}
	
	gtk_progress_bar_update (GTK_PROGRESS_BAR (batcharge),
		(gfloat) batpct / 100);
	
	if (batmin < 100000000)
		sprintf (str, _("%d minutes of battery"), batmin);
	else {	/* would have to be an error */
		g_warning (_("More than 100,000,000 minutes of battery life?!?"));
		return FALSE;
	}
	gtk_label_set (GTK_LABEL (minlabel), str);
	gtk_tooltips_set_tip (tooltips, GTK_WIDGET (data), str, NULL);

	if (linestat)
		fname = ac_pixmap_filename;
	else
		fname = bat_pixmap_filename;

	gnome_pixmap_load_file (GNOME_PIXMAP (GTK_BUTTON (data)->child), fname); 

	return TRUE;
}

static void
show_batmon_window (GtkWidget *widget, gpointer data)
{
	gtk_widget_show (window);
	gtk_grab_add(window);
}

static gint
hide_batmon_window(GtkWidget *widget, gpointer data)
{
	gtk_grab_remove(window);
	gtk_widget_hide(window);
	return FALSE;
}

static void
destroy_instance(GtkWidget *widget, gpointer data)
{
	gtk_timeout_remove(batmon_timeout);
	batmon_timeout = -1;
}

static GtkWidget *
create_batmon_widget (GtkWidget *window)
{
	GtkWidget *button;
	GtkWidget *pixmap;
	
	button = gtk_button_new ();

	pixmap = gnome_pixmap_new_from_file (ac_pixmap_filename);
	gtk_container_add (GTK_CONTAINER (button), pixmap);
	gtk_widget_show(pixmap);
	
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    (GtkSignalFunc) show_batmon_window,
			    NULL);
	gtk_signal_connect (GTK_OBJECT (button), "destroy",
			    (GtkSignalFunc) destroy_instance,
			    NULL);

	/* batmon_timeout_callback((gpointer) button); */

	return button;
}

static void
create_batmon_window (void)
{
	GtkWidget *box1;
	GtkWidget *box2;
	GtkWidget *widget1;
	GtkWidget *widget2;
	
	/* Begin definition of window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), _("APM Stats"));
	gtk_container_border_width (GTK_CONTAINER (window), 10);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    (GtkSignalFunc) hide_batmon_window,
			    NULL);

	box1 = gtk_vbox_new (TRUE, 0);
	gtk_container_add (GTK_CONTAINER (window), box1);
	gtk_widget_show (box1);

/* In Linux we show the APM driver version and APM BIOS version. */
#ifdef	__linux__
	box2 = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (box1), box2);
	gtk_widget_show (box2);

	widget1 = gtk_label_new (_("Linux APM Driver Version:"));
	gtk_box_pack_start (GTK_BOX (box2), widget1, FALSE, FALSE, 0);
	gtk_widget_show (widget1);

	driverlabel = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (box2), driverlabel, FALSE, FALSE, 0);
	gtk_widget_show (driverlabel);

	box2 = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (box1), box2);
	gtk_widget_show (box2);

	widget1 = gtk_label_new (_("APM BIOS Version:"));
	gtk_box_pack_start (GTK_BOX (box2), widget1, FALSE, FALSE, 0);
	gtk_widget_show (widget1);

	bioslabel = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (box2), bioslabel, FALSE, FALSE, 0);
	gtk_widget_show (bioslabel);

	widget1 = gtk_hseparator_new();
	gtk_box_pack_start_defaults (GTK_BOX (box1), widget1);
	gtk_widget_show (widget1);
#endif	/* __linux__ */

	box2 = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (box1), box2);
	gtk_widget_show (box2);

	widget1 = gtk_label_new (_("Battery Status:"));
	gtk_box_pack_start (GTK_BOX (box2), widget1, FALSE, FALSE, 0);
	gtk_widget_show (widget1);

	statlabel = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (box2), statlabel, FALSE, FALSE, 0);
	gtk_widget_show (statlabel);

	batcharge = gtk_progress_bar_new ();
	gtk_box_pack_start_defaults (GTK_BOX (box1), batcharge);
	gtk_widget_show (batcharge);

	widget1 = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	gtk_box_pack_start_defaults (GTK_BOX (box1), widget1);
	gtk_widget_show (widget1);

	minlabel = gtk_label_new ("");
	gtk_container_add (GTK_CONTAINER (widget1), minlabel);
	gtk_widget_show (minlabel);

	widget1 = gtk_alignment_new (1.0, 0.5, 0.0, 0.0);
	gtk_box_pack_start_defaults (GTK_BOX (box1), widget1);
	gtk_widget_show (widget1);

	widget2 = gtk_button_new_with_label (_("Close"));
	gtk_signal_connect(GTK_OBJECT (widget2), "clicked",
			   (GtkSignalFunc) hide_batmon_window,
			   NULL);
	gtk_container_add (GTK_CONTAINER (widget1), widget2);
	GTK_WIDGET_SET_FLAGS (widget2, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(widget2);
	gtk_widget_show(widget2);
}

static void
init_module(void)
{
	tooltips = gtk_tooltips_new();
	ac_pixmap_filename = gnome_unconditional_pixmap_file ("batmon-ac.xpm");
	bat_pixmap_filename = gnome_unconditional_pixmap_file ("batmon-bat.xpm");

	create_batmon_window();
}

static void
destroy_module(void)
{
	gtk_tooltips_unref(tooltips);

	g_free(ac_pixmap_filename);
	g_free(bat_pixmap_filename);

	gtk_widget_destroy(window);
}

static void
create_instance (PanelWidget *panel, char *params, int pos)
{
	PanelCommand cmd;
	GtkWidget *batmon;

	if (batmon_timeout != -1)
		return; /* Only one instance allowed --- XXX: maybe should put up a dialog box */

	batmon = create_batmon_widget (GTK_WIDGET(panel));

	batmon_timeout = gtk_timeout_add(TIMEOUT,
					 (GtkFunction) batmon_timeout_callback,
					 batmon);

	cmd.cmd = PANEL_CMD_REGISTER_TOY;
	cmd.params.register_toy.applet = batmon;
	cmd.params.register_toy.id     = APPLET_ID;
	cmd.params.register_toy.pos    = pos;
	cmd.params.register_toy.flags  = 0;

	(*panel_cmd_func) (&cmd);
}

gpointer
applet_cmd_func (AppletCommand *cmd)
{
	g_assert(cmd != NULL);
	
	switch (cmd->cmd) {
		case APPLET_CMD_QUERY:
			return APPLET_ID;
			
		case APPLET_CMD_INIT_MODULE:
			panel_cmd_func = cmd->params.init_module.cmd_func;
			init_module();
			break;
			
		case APPLET_CMD_DESTROY_MODULE:
			destroy_module();
			break;

		case APPLET_CMD_GET_DEFAULT_PARAMS:
			return g_strdup("");

		case APPLET_CMD_CREATE_INSTANCE:
			create_instance(cmd->panel,
					cmd->params.create_instance.params,
					cmd->params.create_instance.pos);
			break;

		case APPLET_CMD_GET_INSTANCE_PARAMS:
			return g_strdup("");

		case APPLET_CMD_ORIENTATION_CHANGE_NOTIFY:
			break;

		case APPLET_CMD_PROPERTIES:
			fprintf(stderr, "Battery monitor properties not yet implemented\n");
			break;

		default:
			fprintf(stderr,
				APPLET_ID " applet_cmd_func: Oops, unknown command type %d\n",
				(int) cmd->cmd);
			break;
	}

	return NULL;
}
