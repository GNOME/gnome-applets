#include <applet-widget.h>

#define HISTORY_DEPTH 50
#define MAX_COMMAND_LENGTH 500
#define MAX_PREFIXES 99
#define MAX_PREFIX_LENGTH 25

typedef struct struct_properties properties;

struct struct_properties
{
    int show_time;
    int show_date;
    int show_handle;
    int show_frame;
    int auto_complete_history;
    int normal_size_x;
    int normal_size_y;
    int reduced_size_x;
    int reduced_size_y;
    int cmd_line_size_y;
    int flat_layout;
    int cmd_line_color_fg_r;
    int cmd_line_color_fg_g;
    int cmd_line_color_fg_b;
    int cmd_line_color_bg_r;
    int cmd_line_color_bg_g;
    int cmd_line_color_bg_b;
    char *prefix[MAX_PREFIXES];
    char *command[MAX_PREFIXES];
};

extern properties prop;
extern properties prop_tmp;

extern GtkWidget *applet;

void load_session(void);

void save_session(void);

gint save_session_signal(GtkWidget *widget, const char *privcfgpath, const char *globcfgpath);

void properties_box(AppletWidget *applet, gpointer data);
