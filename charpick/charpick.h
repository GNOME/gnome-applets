/* charpick.h -- header file for character picker applet */

#include <gnome.h>
#include <gconf/gconf-client.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>

#define NO_LAST_INDEX -1

typedef struct _charpick_data charpick_data;
/* this type has basically all data for this program */
struct _charpick_data {
  GList *chartable;
  gchar * charlist;  
  gunichar selected_unichar;
  gint last_index;
  GtkWidget *box;
  GtkWidget *frame;
  GtkWidget *applet;
  GtkToggleButton *last_toggle_button;
  gint panel_size;
  gboolean panel_vertical;
  GtkWidget *propwindow;
  GtkWidget *about_dialog;
  GtkWidget *pref_tree;
  GtkWidget *menu;
};


typedef struct _charpick_button_cb_data charpick_button_cb_data;
/* This is the data type for the button callback function. */

struct _charpick_button_cb_data {
  gint button_index;
  charpick_data * p_curr_data;
};


void start_callback_update();

void build_table              (charpick_data     *curr_data);
void add_to_popup_menu (charpick_data *curr_data);
void populate_menu (charpick_data *curr_data);
void save_chartable (charpick_data *curr_data);
void show_preferences_dialog  (BonoboUIComponent *uic,
			       charpick_data     *curr_data,
			       const char        *verbname);
gchar *run_edit_dialog  (gchar *string, gchar *title);
void set_atk_name_description (GtkWidget         *widget,
			       const char        *name,
			       const char        *description);
gboolean key_writable (PanelApplet *applet, const char *key);

