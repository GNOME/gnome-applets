extern GtkWidget *entryCommand;

void initCommandEntry(void);
gint showHistory_signal(GtkWidget *widget, gpointer data);
gint showFileBrowser_signal(GtkWidget *widget, gpointer data);
void command_entry_update_color(void);
void command_entry_update_size(void);
