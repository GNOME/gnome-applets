#include <panel-applet.h>
#include <preferences.h>

extern GtkWidget *entry_command;

void init_command_entry(PanelApplet *applet);
gint show_history_signal(GtkWidget *widget, gpointer data);
gint show_file_browser_signal(GtkWidget *widget, gpointer data);
void command_entry_update_color(properties *prop);
void command_entry_update_size(properties *prop);
