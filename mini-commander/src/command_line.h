#ifndef _COMMAND_LINE_H_
#define _COMMAND_LINE_H_

#include <panel-applet.h>
#include <preferences.h>
#include "mini-commander_applet.h"

GtkWidget *init_command_entry         (MCData      *mcdata);
int        show_history_signal        (GtkWidget   *widget,
				       MCData      *mcdata);
int        show_file_browser_signal   (GtkWidget   *widget,
				       PanelApplet *applet);
void       command_entry_update_color (GtkWidget   *entry_command,
				       properties  *prop);
void       command_entry_update_size  (GtkWidget   *entry_command,
				       properties  *prop);
 
#endif
