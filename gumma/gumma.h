#ifndef __GUMMA_H__
#define __GUMMA_H__

#include <gmodule.h>

typedef enum {
	GUMMA_VERB_PLAY,
	GUMMA_VERB_PAUSE,
	GUMMA_VERB_STOP,
	GUMMA_VERB_EJECT,
	GUMMA_VERB_NEXT,
	GUMMA_VERB_PREV	,
	GUMMA_VERB_FORWARD,
	GUMMA_VERB_REWIND
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

typedef struct _GummaPlugin GummaPlugin;

struct _GummaPlugin {
	gpointer (*init) ();
	void (*denit) (gpointer data);

	void (*do_verb) (GummaVerb verb, gpointer data);
	void (*data_dropped) (GtkSelectionData *selection, 
			      gpointer data);

	GummaState (*get_state) (gpointer data);
	void (*get_time) (GummaTimeInfo *tinfo,
			  gpointer data);

	void (*about) (gpointer data);
	GtkWidget *(*get_config_page) (gpointer data);
};

#ifdef GUMMA_PLUGIN
G_MODULE_EXPORT GummaPlugin *get_plugin (void);
#else
G_MODULE_IMPORT GummaPlugin *get_plugin (void);
#endif /*!GUMMA_PLUGIN*/

#endif /*__GUMMA_H__*/
