#ifndef __GUMMA_H__
#define __GUMMA_H__

#include <gmodule.h>

typedef enum {
	GUMMA_VERB_PLAY,
	GUMMA_VERB_PAUSE,
	GUMMA_VERB_STOP,
	GUMMA_VERB_EJECT,
	GUMMA_VERB_NEXT,
	GUMMA_VERB_PREV
} GummaVerb;

typedef enum {
	GUMMA_STATE_PLAYING,
	GUMMA_STATE_PAUSED,
	GUMMA_STATE_STOPPED,
	GUMMA_STATE_ERROR
} GummaState;

typedef struct {
	gint minutes;
	gint seconds;
	gint track;
} GummaTimeInfo;

#ifdef GUMMA_PLUGIN
G_MODULE_EXPORT void gp_about ();
G_MODULE_EXPORT GtkWidget *gp_get_config_page ();
G_MODULE_EXPORT void gp_init ();
G_MODULE_EXPORT void gp_denit ();
G_MODULE_EXPORT GummaState gp_get_state ();
G_MODULE_EXPORT void gp_do_verb ();
G_MODULE_EXPORT void gp_data_dropped (GtkSelectionData *data);
G_MODULE_EXPORT void gp_get_time (GummaTimeInfo *tinfo);
#endif /*GUMMA_PLUGIN*/

#endif /*__GUMMA_H__*/
