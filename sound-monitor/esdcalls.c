/* GNOME Esound Monitor Control applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

/* Portions of code in this file based on vu-meter by Gregory McLean */

#include "sound-monitor.h"

static gint open_sound (gchar *esd_host, gint use_input);
static void update_levels (gpointer data, gint source, GdkInputCondition condition);

static gint open_sound (gchar *esd_host, gint use_input)
{
	gint sound = -1;

#ifdef HAVE_ADVANCED
	use_input = FALSE;
#endif

	if (!use_input)
		{
		sound = esd_monitor_stream (ESD_BITS16|ESD_STEREO|ESD_STREAM|ESD_PLAY,
					    RATE, esd_host, "volume_meter_applet");
		}
	else
		{
	/* use this command to monitor input (microphone, line-in, cd, etc.)
	   hmm, well it works for me under Linux kernel 2.0.36, but freezes
	   the system under 2.2.1. I think it is a duplex problem.
	   Yes, this should be an option, but not until it stops freezing my
	   system under 2.2.x :( UPDATE: I added the option, but did not test it.*/
		sound = esd_record_stream (ESD_BITS16|ESD_STEREO|ESD_STREAM|ESD_PLAY|ESD_RECORD,
					   RATE, esd_host, "volume_meter_applet");
		}

	if (sound < 0)
		{ /* TPG: Make a friendly error dialog if we can't open sound */
		GtkWidget *box;
		/*printf("unable to connect to esound");*/
		box = gnome_error_dialog("Cannot connect to the sound daemon.\nThe sound daemon can be started by selecting 'Start ESD' from\n the right click menu, or by running 'esd' at a command prompt.");
		}

	return sound;
}

static void update_levels (gpointer data, gint source, GdkInputCondition condition)
{
	AppData *ad = data;
	register gint i;
	register short val_l, val_r;
	static unsigned short bigl, bigr;

	if (!(read (source, ad->aubuf, NSAMP * 2)))
		{
		return;
		printf("...nothing");
		}
	bigl = 0;
	bigr = 0;
	for (i = 1; i < NSAMP / 2;)
		{
		val_r = abs (ad->aubuf[i++]);
		val_l = abs (ad->aubuf[i++]);
		bigl = (val_l > bigl) ? val_l : bigl;
		bigr = (val_r > bigr) ? val_r : bigr;
		}
	bigl /= 327;
	bigr /= 327;

	/* ok, these values are stored, and will be updated in the
	   next screen redraw update, this will be improved, but for now...
	   the idea is to allow a screen refresh rate slower than the input rate */
	ad->vu_l = bigl;
	ad->vu_r = bigr;
	ad->new_vu_data = TRUE;
}

/* returns TRUE if successful, FALSE if failed */
gint init_sound(AppData *ad)
{
	ad->sound_fd = open_sound(ad->esd_host, ad->monitor_input);

	if(ad->sound_fd > 0) /* TPG: Make sure we have a valid fd... */
		{
		ad->sound_input_cb_id = gdk_input_add (ad->sound_fd, GDK_INPUT_READ, (GdkInputFunction)update_levels, ad);
		ad->esd_status = ESD_STATUS_STANDBY;
		return TRUE;
		}
	ad->esd_status = ESD_STATUS_ERROR;
	return FALSE;
}

/* shuts down the esd connection fd */
void stop_sound(AppData *ad)
{
	if (ad->sound_input_cb_id > -1)
		{
		gdk_input_remove(ad->sound_input_cb_id);
		ad->sound_input_cb_id = -1;
		}
	if (ad->sound_fd > -1)
		{
		esd_close(ad->sound_fd);
		ad->sound_fd = -1;
		}
}

void esd_sound_control(gint function, AppData *ad)
{
	if (function == ESD_CONTROL_START)
		{
		/* EEK! ESD should have a flag to start the daemon in
		 * the background, and return to cmd line when ready */
		system("esd &");
		sleep(3);
		init_sound(ad);
		}
	else if (function == ESD_CONTROL_STANDBY)
		{
		gint esd_fd = esd_open_sound(ad->esd_host);
		if (esd_fd>= 0)
			{
			esd_standby(esd_fd);
			esd_close(esd_fd);
			}
		}
	else if (function == ESD_CONTROL_RESUME)
		{
		gint esd_fd = esd_open_sound(ad->esd_host);
		if (esd_fd>= 0)
			{
			esd_resume(esd_fd);
			esd_close(esd_fd);
			}
		}
}

gint esd_check_status(AppData *ad)
{
	esd_standby_mode_t mode;
	gint esd_fd = 0;

	esd_fd = esd_open_sound(ad->esd_host);
	if (esd_fd>= 0)
		{
		mode = esd_get_standby_mode(esd_fd);
		esd_close(esd_fd);
		}
	else
		{
		/* hmm, well, this must be an error, right? */
		return ESD_STATUS_ERROR;
		}

	if (mode == ESM_ON_STANDBY) return ESD_STATUS_STANDBY;
	if (mode == ESM_ON_AUTOSTANDBY) return ESD_STATUS_AUTOSTANDBY;
	if (mode == ESM_RUNNING) return ESD_STATUS_READY;

	return ESD_STATUS_ERROR;
}
