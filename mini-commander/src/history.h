#include "mini-commander_applet.h"

int exists_history_entry(MCData *mcdata, int pos);
extern char *get_history_entry(MCData *mcdata, int pos);
extern void set_history_entry(MCData *mcdata, int pos, char * entry);
extern void append_history_entry(MCData *mcdata, char * entry, gboolean load_history);
