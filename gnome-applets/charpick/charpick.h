/* charpick.h -- header file for character picker applet */

#include "charpick-applet.h"

#define CHARPICK_SCHEMA  "org.gnome.gnome-applets.charpick"
#define KEY_CURRENT_LIST "current-list"
#define KEY_CHARTABLE    "chartable"

typedef struct _CharpickApplet
{
  GpApplet parent;

  GList *chartable;
  gchar * charlist;  
  gunichar selected_unichar;
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
  GtkWidget *add_edit_dialog;
  GtkWidget *add_edit_entry;
  GSettings *settings;
  guint rebuild_id;
  GtkWidget *invisible;
} charpick_data;

typedef struct _charpick_button_cb_data charpick_button_cb_data;
/* This is the data type for the button callback function. */

struct _charpick_button_cb_data {
  gint button_index;
  charpick_data * p_curr_data;
};

void start_callback_update(void);

void build_table              (charpick_data     *curr_data);
void add_to_popup_menu (charpick_data *curr_data);
void populate_menu (charpick_data *curr_data);
void save_chartable (charpick_data *curr_data);
void show_preferences_dialog (GSimpleAction *action,
                              GVariant      *parameter,
                              gpointer       user_data);

void add_edit_dialog_create (charpick_data	 *curr_data,
			     gchar		 *string,
			     gchar		 *title);
void set_atk_name_description (GtkWidget         *widget,
			       const char        *name,
			       const char        *description);
