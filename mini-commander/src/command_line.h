extern GtkWidget *entry_command;

void init_command_entry(void);
gint show_history_signal(GtkWidget *widget, gpointer data);
gint show_file_browser_signal(GtkWidget *widget, gpointer data);
void command_entry_update_color(void);
void command_entry_update_size(void);
