/* charpick.h -- header file for character picker applet */

#include <gnome.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#define CHARPICK_VERSION "0.05"
#define NO_LAST_INDEX -1
#define DEFAULT_ROWS 2
#define DEFAULT_COLS 4
#define DEFAULT_SIZE 22
#define DEFAULT_MIN_CELLS (2*4)
#define MAX_BUTTONS 25
#define MAX_BUTTONS_WITH_BUFFER ((2*MAX_BUTTONS)-1)

typedef struct _charpick_data charpick_data;
/* this type has basically all data for this program */
struct _charpick_data {
  const gchar * charlist;
  gchar * default_charlist;
  gchar selected_char;
  gint last_index;
  GtkWidget *box;
  GtkWidget *event_box;
  GtkWidget *frame;
  GtkWidget *applet;
  GtkToggleButton *last_toggle_button;
  gint panel_size;
  gboolean panel_vertical;
  GtkWidget *propwindow;
};


typedef struct _charpick_button_cb_data charpick_button_cb_data;
/* This is the data type for the button callback function. */

struct _charpick_button_cb_data {
  gint button_index;
  charpick_data * p_curr_data;
};


void start_callback_update();

void build_table(charpick_data *curr_data);
void property_show(BonoboUIComponent *uic, gpointer data, const gchar *verbname);
void set_atk_name_description(GtkWidget *widget, const gchar *name,
                              const gchar *description);







