#ifndef _SESSION_H
#define _SESSION_H

#include "battery.h"

void battery_session_load(char * cfgpath, BatteryData * bat);
int battery_session_save(GtkWidget * w,
			 const char * cfgpath,
			 const char * globcfgpath,
			 gpointer data);
void battery_session_defaults(BatteryData * bat);

#endif /* _SESSION_H */
