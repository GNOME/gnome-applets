/* charpick.h -- header file for character picker applet */

#include <config.h>
#include <gnome.h>
#include <applet-widget.h>
#define CHARPICK_VERSION "0.05"
#define NO_LAST_INDEX -1
#define DEFAULT_ROWS 2
#define DEFAULT_COLS 4
#define DEFAULT_SIZE 22
#define DEFAULT_MIN_CELLS (2*4)
#define MAX_BUTTONS 25
#define MAX_BUTTONS_WITH_BUFFER ((2*MAX_BUTTONS)-1)

typedef struct _charpick_persistant_properties charpick_persistant_properties;
/* This is the data type for user definable properties of the charpick applet.
 * It should include anything we might want to save when (and if)
 * we save the session.
 */
struct _charpick_persistant_properties {
  const gchar * default_charlist;
  gboolean      follow_panel_size;
  gint          min_cells;
  gint          rows;
  gint          cols;
  gint          size; /* this is the height and width of a cell */
};

typedef struct _charpick_data charpick_data;
/* this type has basically all data for this program */
struct _charpick_data {
  const gchar * charlist;
  gchar selected_char;
  gint last_index;
  GtkWidget * *toggle_buttons;
  GtkWidget * *labels;
  GtkWidget *table;
  GtkWidget *event_box;
  GtkWidget *frame;
  GtkWidget *applet;
  gint panel_size;
  gboolean panel_vertical;
  charpick_persistant_properties * properties;
};

/* typedef struct _prop_sb_cb_data prop_sb_cb_data; */



typedef struct _charpick_button_cb_data charpick_button_cb_data;
/* This is the data type for the button callback function. */

struct _charpick_button_cb_data {
  gint button_index;
  charpick_data * p_curr_data;
};

/*static const gchar *def_list = "áâãñæª";*/

extern charpick_data curr_data;

void start_callback_update();

void build_table(charpick_data *curr_data);
void property_load(char *path, gpointer data);
void property_save(char *path, charpick_persistant_properties *properties);
void property_show(AppletWidget *applet, gpointer data);







