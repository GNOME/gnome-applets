/* GNOME sound-Monitor applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

/*
 * Portions of code in this file originally based on vu-meter by Gregory McLean
 * almost completely different by now.
 */

#include "sound-monitor.h"
#include "esdcalls.h"

static gint open_sound (const gchar *esd_host, gint use_input)
{
	gint sound = -1;

	if (!use_input)
		{
		sound = esd_monitor_stream (ESD_BITS16|ESD_STEREO|ESD_STREAM|ESD_PLAY,
					    RATE, esd_host, "volume_meter_applet");
		}
	else
		{
		sound = esd_record_stream (ESD_BITS16|ESD_STEREO|ESD_STREAM|ESD_PLAY|ESD_RECORD,
					   RATE, esd_host, "volume_meter_applet");
		}

	if (sound < 0)
		{
		GtkWidget *box;
		box = gnome_error_dialog("Cannot connect to the sound daemon.\nThe sound daemon can be started by selecting 'Start ESD' from\n the right click menu, or by running 'esd' at a command prompt.");
		}

	return sound;
}

static void sound_compute_volume(SoundData *sd)
{
	unsigned short l, r;
	unsigned short val_l, val_r;
	int i;

	l = sd->left;
	r = sd->right;
	for (i = 1; i < NSAMP / 2;)
		{
		val_r = abs (sd->buffer[i++]);
		val_l = abs (sd->buffer[i++]);
		l = (val_l > l) ? val_l : l;
		r = (val_r > r) ? val_r : r;
		}
	l /= 327;
	r /= 327;

	sd->left = l;
	sd->right = r;
}

static void sound_read(gpointer data, gint source, GdkInputCondition condition)
{
	SoundData *sd = data;

	if (!(read (source, sd->buffer, NSAMP * 2)))
		{
		return;
		printf("...nothing");
		}

	sd->new_data = TRUE;

	sound_compute_volume(sd);

	return;
}

void sound_get_volume(SoundData *sd, gint *l, gint *r)
{
	if (!sd) return;

	*l = sd->left;
	*r = sd->right;

	/* this is reset after every call */
	sd->left = 0;
	sd->right = 0;
}

void sound_get_buffer(SoundData *sd, short **buffer, gint *length)
{
	if (!sd) return;

	*buffer = sd->buffer;
	*length = NSAMP;
}

/* returns TRUE if successful, FALSE if failed */
SoundData *sound_init(const gchar *host, gint monitor_input)
{
	SoundData *sd;
	gint fd;

	fd = open_sound(host, monitor_input);

	if (fd < 1) return NULL;

	sd = g_new0(SoundData, 1);

	sd->fd = fd;
	sd->input_cb_id = gdk_input_add (sd->fd, GDK_INPUT_READ, (GdkInputFunction)sound_read, sd);

	sd->left = sd->right = 0;
	sd->new_data = FALSE;

	return sd;
}

void sound_free(SoundData *sd)
{
	if (!sd) return;

	if (sd->input_cb_id > -1)
		{
		gdk_input_remove(sd->input_cb_id);
		}
	if (sd->fd > -1)
		{
		esd_close(sd->fd);
		}

	g_free(sd);
}

/* esd controls */

gint esd_control(ControlType function, const gchar *host)
{
	gint esd_fd;
	gint ret;

	switch (function)
		{
		case Control_Start:
			/* EEK! ESD should have a flag to start the daemon in
			 * the background, and return to cmd line when ready */
			ret = system("esd &");
			if (ret == -1 || ret == 127) return FALSE;
			sleep(3);
			return TRUE;
			break;
		case Control_Standby:
			esd_fd = esd_open_sound(host);
			if (esd_fd >= 0)
				{
				esd_standby(esd_fd);
				esd_close(esd_fd);
				return TRUE;
				}
			break;
		case Control_Resume:
			esd_fd = esd_open_sound(host);
			if (esd_fd >= 0)
				{
				esd_resume(esd_fd);
				esd_close(esd_fd);
				return TRUE;
				}
			break;
		}

	return FALSE;
}

StatusType esd_status(const gchar *host)
{
	esd_standby_mode_t mode;
	gint esd_fd = 0;

	esd_fd = esd_open_sound(host);
	if (esd_fd>= 0)
		{
		mode = esd_get_standby_mode(esd_fd);
		esd_close(esd_fd);
		}
	else
		{
		/* hmm, well, this must be an error, right? */
		return Status_Error;
		}

	if (mode == ESM_ON_STANDBY) return Status_Standby;
	if (mode == ESM_ON_AUTOSTANDBY) return Status_AutoStandby;
	if (mode == ESM_RUNNING) return Status_Ready;

	return Status_Error;
}


