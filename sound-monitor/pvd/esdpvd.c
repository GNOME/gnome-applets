/* esound persistent volume daemon, this program is part of the
 * GNOME Esound Monitor Control applet
 * (C) 2000 John Ellis
 *
 * Author: John Ellis
 *
 */

#include "esdpvd.h"

static GList *stream_list = NULL;
static GList *sample_list = NULL;
static gchar *datafile_path = NULL;
static gint update_timeout_id = -1;
static gchar *esd_host = NULL; /* fix me, should be configurable on command line */

static void update_esd_volume(PlayerData *pd)
{
	gint esd_fd;

	esd_fd = esd_open_sound(esd_host);
	if (esd_fd >= 0)
		{
		if (pd->data_type == DATA_STREAM)
			{
			esd_set_stream_pan(esd_fd, pd->id, pd->vol_left, pd->vol_right);
			}
		else
			{
			esd_set_default_sample_pan(esd_fd, pd->id, pd->vol_left, pd->vol_right);
			}
		esd_close(esd_fd);
		}
}


/*
 * -------------------------------------------------------------------------
 * config calls
 * -------------------------------------------------------------------------
 */

static gchar *config_prefix_by_type(PlayerData *pd)
{
	if (pd->data_type == DATA_SAMPLE)
		{
		return "samples";
		}

	/* we get here, assume stream */

	if (pd->type == TYPE_RECORD)
		{
		return "streams_record";
		}
	if (pd->type == TYPE_PLAY)
		{
		return "streams_play";
		}
	if (pd->type == TYPE_MONITOR)
		{
		return "streams_monitor";
		}
	if (pd->type == TYPE_SAMPLE)
		{
		return "streams_sample";
		}
	if (pd->type == TYPE_ADPCM)
		{
		return "streams_adpcm";
		}

	return "unknown";
}

static gchar *parse_text(gchar *pretext, gchar *extra)
{
	gchar *text = g_strconcat(pretext, extra, NULL);
	gchar *ptr = text;

	while (*ptr != '\0')
		{
		if (*ptr == '/') *ptr ='_';
		ptr++;
		}

	return text;
}

static gchar *config_volumes(PlayerData *pd)
{
	static gchar *text = NULL;

	g_free(text);
	text = g_strdup_printf("%d,%d", pd->vol_left, pd->vol_right);

	return text;
}

static void config_save_state(PlayerData *pd)
{
	gchar *prefix;

	prefix = g_strconcat(datafile_path, "/", config_prefix_by_type(pd), "/", NULL);
	gnome_config_push_prefix(prefix);

	if (pd->name)
		{
		gchar *entry = parse_text(pd->name, NULL);
		gnome_config_set_string(entry, config_volumes(pd));
		g_free(entry);
		}

	gnome_config_pop_prefix();

	g_free(prefix);
}

static gint config_load_state(PlayerData *pd)
{
	gchar *prefix;
	gchar *vol_info = NULL;

	prefix = g_strconcat(datafile_path, "/", config_prefix_by_type(pd), "/", NULL);
	gnome_config_push_prefix(prefix);

	if (pd->name)
		{
		gchar *entry = parse_text(pd->name, "=");
		vol_info = gnome_config_get_string(entry);
		g_free(entry);
		}

	gnome_config_pop_prefix();

	g_free(prefix);

	if (vol_info)
		{
		int l = -1;
		int r = -1;
		sscanf(vol_info, "%d,%d", &l, &r);

		if (l >= 0 && r >= 0)
			{
			pd->vol_left = l;
			pd->vol_right = r;
			g_free(vol_info);
			return TRUE;
			}
		
		g_free(vol_info);
		return FALSE;
		}

	return FALSE;
}

/*
 * -------------------------------------------------------------------------
 * the generic list functions
 * -------------------------------------------------------------------------
 */

static void free_player_data(PlayerData *pd)
{
	g_free(pd->name);
	g_free(pd);
}

static void player_list_mark_dirty(GList *list)
{
	GList *work = list;
	while(work)
		{
		PlayerData *pd = work->data;
		pd->dirty = TRUE;
		work = work->next;
		}
}

static void player_list_free_dirty(GList **list)
{
	GList *work = *list;
	while(work)
		{
		PlayerData *pd = work->data;
		work = work->next;
		if (pd->dirty)
			{
			*list = g_list_remove(*list, pd);
			free_player_data(pd);
			}
		}
}

/*
 * -------------------------------------------------------------------------
 * stream functions
 * -------------------------------------------------------------------------
 */

static void stream_data_update(PlayerData *pd, esd_player_info_t *stream, gint save)
{
	gint updated = FALSE;
	gint type = -1;

	if (pd->id != stream->source_id)
		{
		pd->id = stream->source_id;
		updated = TRUE;
		}
	if ((pd->name && strcmp(pd->name, stream->name) != 0) ||
	    (!pd->name && strlen(stream->name) > 0))
		{
		g_free(pd->name);
		pd->name = g_strdup(stream->name);
		updated = TRUE;
		}
	if (pd->vol_left != stream->left_vol_scale)
		{
		pd->vol_left = stream->left_vol_scale;
		updated = TRUE;
		}
	if (pd->vol_right != stream->right_vol_scale)
		{
		pd->vol_right = stream->right_vol_scale;
		updated = TRUE;
		}

	if ((stream->format & ESD_MASK_MODE) == ESD_STREAM)
		{
		if ((stream->format & ESD_MASK_FUNC) == ESD_RECORD)
			type = TYPE_RECORD;
		else if ((stream->format & ESD_MASK_FUNC) == ESD_PLAY)
			type = TYPE_PLAY;
		else
			type = TYPE_MONITOR;
		}
	else if ((stream->format & ESD_MASK_MODE) == ESD_SAMPLE)
		type = TYPE_SAMPLE;
	else
		type = TYPE_ADPCM;

	if (pd->type != type)
		{
		pd->type = type;
		updated = TRUE;
		}

	if (updated && save)
		{
		config_save_state(pd);
		}

	pd->dirty = FALSE;
}

static void stream_list_register(esd_player_info_t *stream)
{
	GList *work = stream_list;
	gint updated = FALSE;

	while(work && !updated)
		{
		PlayerData *pd = work->data;
		if (pd->id == stream->source_id)
			{
			stream_data_update(pd, stream, TRUE);
			updated = TRUE;
			}
		work = work->next;
		}

	if (!updated)
		{
		PlayerData *pd = g_new0(PlayerData, 1);
		pd->data_type = DATA_STREAM;
		stream_data_update(pd, stream, FALSE);
		if (config_load_state(pd))
			{
			if (stream->left_vol_scale != pd->vol_left ||
			    stream->right_vol_scale != pd->vol_right)
				{
				update_esd_volume(pd);
				}
			}
		else
			{
			config_save_state(pd);
			}
		stream_list = g_list_append(stream_list, pd);
		}
}

/*
 * -------------------------------------------------------------------------
 * sample functions
 * -------------------------------------------------------------------------
 */

static void sample_data_update(PlayerData *pd, esd_sample_info_t *sample, gint save)
{
	gint updated = FALSE;

	if (pd->id != sample->sample_id)
		{
		pd->id = sample->sample_id;
		updated = TRUE;
		}
	if ((pd->name && strcmp(pd->name, sample->name) != 0) ||
	    (!pd->name && strlen(sample->name) > 0))
		{
		g_free(pd->name);
		pd->name = g_strdup(sample->name);
		updated = TRUE;
		}
	if (pd->vol_left != sample->left_vol_scale)
		{
		pd->vol_left = sample->left_vol_scale;
		updated = TRUE;
		}
	if (pd->vol_right != sample->right_vol_scale)
		{
		pd->vol_right = sample->right_vol_scale;
		updated = TRUE;
		}

	if (updated && save)
		{
		config_save_state(pd);
		}

	pd->dirty = FALSE;
}

static void sample_list_register(esd_sample_info_t *sample)
{
	GList *work = sample_list;
	gint updated = FALSE;

	while(work && !updated)
		{
		PlayerData *pd = work->data;
		if (pd->id == sample->sample_id)
			{
			sample_data_update(pd, sample, TRUE);
			updated = TRUE;
			}
		work = work->next;
		}

	if (!updated)
		{
		PlayerData *pd = g_new0(PlayerData, 1);
		pd->data_type = DATA_SAMPLE;
		sample_data_update(pd, sample, FALSE);
		if (config_load_state(pd))
			{
			if (sample->left_vol_scale != pd->vol_left ||
			    sample->right_vol_scale != pd->vol_right)
				{
				update_esd_volume(pd);
				}
			}
		else
			{
			config_save_state(pd);
			}
		sample_list = g_list_append(sample_list, pd);
		}
}

/*
 * -------------------------------------------------------------------------
 * session management (very simple)
 * -------------------------------------------------------------------------
 */

static void session_die(GnomeClient *client, gpointer data)
{
	gtk_main_quit();
	return;
	data = NULL;
}

/*
 * -------------------------------------------------------------------------
 * update callback
 * -------------------------------------------------------------------------
 */

static gint update_cb(gpointer data)
{
	esd_info_t *info = NULL;
	esd_sample_info_t *sample = NULL;
	esd_player_info_t *stream = NULL;
	gint esd_fd;

	esd_fd = esd_open_sound(esd_host);
	if (esd_fd >= 0)
		{
		info = esd_get_all_info(esd_fd);
		esd_close(esd_fd);
		}
	else
		{
		return TRUE;
		}

	if (!info) return TRUE;

	/* process streams */

	player_list_mark_dirty(stream_list);
	stream = info->player_list;
	while(stream)
		{
		stream_list_register(stream);
		stream = stream->next;
		}
	player_list_free_dirty(&stream_list);

	/* process samples */

	player_list_mark_dirty(sample_list);
	sample = info->sample_list;
	while(sample)
		{
		sample_list_register(sample);
		sample = sample->next;
		}
	player_list_free_dirty(&sample_list);

	/* finish up */

	esd_free_all_info(info);
	gnome_config_sync_file(datafile_path);

	return TRUE;
	data = NULL;
}


int main (int argc, char *argv[])
{
	gint noX = FALSE;
	gint i;

	for (i = 1; i < argc; i++)
		{
		if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--noX") == 0)
			{
			noX = TRUE;
			}
		else
			{
			/* help */
			printf(_("ESounD Persistent Volume Daemon version %s\n"), VERSION);
			printf(_("options:\n"));
			printf(_("  -n, --noX     Allow to run without X server\n"));
			}
		}

	datafile_path = g_strdup("/esd_persistent_volumes");

	if (noX)
		{
		GMainLoop *loop;	/* glib event loop */
		gnomelib_init ("esdpvd", VERSION);

		/* this is how to do a main loop in glib, without gtk */
		loop = g_main_new (FALSE);

		update_timeout_id = g_timeout_add(CHECK_TIMEOUT_INTERVAL, (GSourceFunc) update_cb, NULL);

		g_main_run (loop);
		g_main_destroy (loop);
		}
	else
		{
		GnomeClient *client;

		gnome_init ("esdpvd", VERSION, argc, argv);

		client = gnome_master_client();
		gtk_signal_connect(GTK_OBJECT (client), "die",
				   GTK_SIGNAL_FUNC (session_die), NULL);

		update_timeout_id = gtk_timeout_add(CHECK_TIMEOUT_INTERVAL, (GtkFunction) update_cb, NULL);

		gtk_main();
		}

	return 0;
	argc = 0;
	argv = NULL;
}

