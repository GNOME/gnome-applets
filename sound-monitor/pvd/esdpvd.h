/* esound persistent volume daemon, this program is part of the
 * GNOME Esound Monitor Control applet
 * (C) 1999 John Ellis
 *
 * Author: John Ellis
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gnome.h>
#include <esd.h>

#define ESDPVD_VERSION_MAJ 0
#define ESDPVD_VERSION_MIN 1
#define ESDPVD_VERSION_REV 0

#define CHECK_TIMEOUT_INTERVAL 500

enum {
	DATA_STREAM,
	DATA_SAMPLE
};

enum {
	TYPE_NONE,
	TYPE_RECORD,
	TYPE_PLAY,
	TYPE_MONITOR,
	TYPE_SAMPLE,
	TYPE_ADPCM
};

typedef struct _PlayerData PlayerData;
struct _PlayerData
{
	gint id;
	gchar *name;
	gint vol_left;
	gint vol_right;
	gint type;

	gint data_type;

	gint dirty;
	gint saved;
};


