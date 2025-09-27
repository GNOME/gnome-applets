/*
 * Copyright (C) 1998 Oliver Maruhn <oliver@maruhn.com>,
 *               2002 Sun Microsystems
 *
 * Authors: Oliver Maruhn <oliver@maruhn.com>,
 *          Mark McLoughlin <mark@skynet.ie>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "mini-commander-applet.h"

#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "mini-commander-applet-private.h"
#include "preferences.h"
#include "command-line.h"
#include "gsettings.h"

#include "browser-mini.xpm"
#include "history-mini.xpm"

#define COMMANDLINE_BROWSER_STOCK "commandline-browser"
#define COMMANDLINE_HISTORY_STOCK "commandline-history"

#define COMMANDLINE_DEFAULT_ICON_SIZE 6

G_DEFINE_TYPE (MiniCommanderApplet, mini_commander_applet, GP_TYPE_APPLET)

static gboolean icons_initialized = FALSE;
static GtkIconSize button_icon_size = 0;

static void
show_help (GSimpleAction *action,
           GVariant      *parameter,
           gpointer       user_data)
{
  gp_applet_show_help (GP_APPLET (user_data), NULL);
}

static void
about_box (GSimpleAction *action,
           GVariant      *parameter,
           gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static const GActionEntry mini_commander_menu_actions [] = {
	{ "preferences", mc_show_preferences, NULL, NULL, NULL },
	{ "help",        show_help,           NULL, NULL, NULL },
	{ "about",       about_box,           NULL, NULL, NULL },
	{ NULL }
};

typedef struct {
    const char  *stock_id;
    const char **icon_data;
} CommandLineStockIcon;

static CommandLineStockIcon items[] = {
    { COMMANDLINE_BROWSER_STOCK, browser_mini_xpm },
    { COMMANDLINE_HISTORY_STOCK, history_mini_xpm }
};

static void
register_command_line_stock_icons (GtkIconFactory *factory)
{
    guint i;

    for (i = 0; i < G_N_ELEMENTS (items); ++i) {
       GtkIconSet *icon_set;
       GdkPixbuf *pixbuf;

       G_GNUC_BEGIN_IGNORE_DEPRECATIONS
       pixbuf = gdk_pixbuf_new_from_xpm_data ((items[i].icon_data));
       G_GNUC_END_IGNORE_DEPRECATIONS

       icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);
       gtk_icon_factory_add (factory, items[i].stock_id, icon_set);

       gtk_icon_set_unref (icon_set);
       g_object_unref (G_OBJECT (pixbuf));
    }

}

static void
command_line_init_stock_icons (void)
{

    GtkIconFactory *factory;

    if (icons_initialized)
	    return;

    factory = gtk_icon_factory_new ();
    gtk_icon_factory_add_default (factory);

    register_command_line_stock_icons (factory);

    button_icon_size = gtk_icon_size_register ("mini-commander-icon",
                                                 COMMANDLINE_DEFAULT_ICON_SIZE,
                                                 COMMANDLINE_DEFAULT_ICON_SIZE);

    icons_initialized = TRUE;
    g_object_unref (factory);

}

void
mc_set_atk_name_description (GtkWidget  *widget,
                             const char *name,
                             const char *description)
{
    AtkObject *aobj;

    aobj = gtk_widget_get_accessible (widget);
    if (GTK_IS_ACCESSIBLE (aobj) == FALSE)
        return;

    atk_object_set_name (aobj, name);
    atk_object_set_description (aobj, description);
}

/* This is a hack around the fact that gtk+ doesn't
 * propogate button presses on button2/3.
 */
static gboolean
button_press_hack (GtkWidget      *widget,
		   GdkEventButton *event,
		   MCData         *mc)
{
    if (event->button == 3 || event->button == 2) {
	gtk_propagate_event (GTK_WIDGET (mc), (GdkEvent *) event);
	return TRUE;
    }

    return FALSE;
}

/* Send button presses on the applet to the entry. This makes Fitts' law work (ie click on the bottom
** most pixel and the key press will be sent to the entry */
static gboolean
send_button_to_entry_event (GtkWidget *widget, GdkEventButton *event, MCData *mc)
{

	if (event->button == 1) {
		gtk_widget_grab_focus (mc->entry);
		return TRUE;
	}
	return FALSE;

}

static gboolean
key_press_cb (GtkWidget *widget, GdkEventKey *event, MCData *mc)
{
	switch (event->keyval) {
	case GDK_KEY_b:
		if (event->state == GDK_CONTROL_MASK) {
			mc_show_file_browser (NULL, mc);
			return TRUE;
		}
		break;
	case GDK_KEY_h:
		if (event->state == GDK_CONTROL_MASK) {
			mc_show_history (NULL, mc);
			return TRUE;
		}
		break;
	default:
		break;
	}

	return FALSE;

}

void
mc_applet_draw (MCData *mc)
{
    GtkWidget *icon;
    GtkWidget *button;
    GtkWidget *hbox_buttons;
    MCPreferences prefs = mc->preferences;
    int        size_frames = 0;
    gchar     *command_text = NULL;

    if (mc->entry != NULL)
	command_text = g_strdup (gtk_editable_get_chars (GTK_EDITABLE (mc->entry), 0, -1));

    mc->cmd_line_size_y = mc->preferences.normal_size_y - size_frames;

    if (mc->applet_box) {
        gtk_widget_destroy (mc->applet_box);
    }

    if ( (mc->orient == GTK_ORIENTATION_VERTICAL) && (prefs.panel_size_x < 36) )
      mc->applet_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    else
      mc->applet_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_container_set_border_width (GTK_CONTAINER (mc->applet_box), 0);

    mc_create_command_entry (mc);

    if (command_text != NULL) {
	gtk_entry_set_text (GTK_ENTRY (mc->entry), command_text);
	g_free (command_text);
    }

    /* hbox for message label and buttons */
    if (mc->orient == GTK_ORIENTATION_VERTICAL)
      if (prefs.panel_size_x < 36)
	hbox_buttons = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      else
	hbox_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    else
      if (prefs.normal_size_y > 36)
	hbox_buttons = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
      else
	hbox_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_set_homogeneous (GTK_BOX (hbox_buttons), TRUE);

    /* add file-browser button */
    button = gtk_button_new ();

    g_signal_connect (button, "clicked",
		      G_CALLBACK (mc_show_file_browser), mc);
    g_signal_connect (button, "button_press_event",
		      G_CALLBACK (button_press_hack), mc);

    icon = gtk_image_new_from_stock (COMMANDLINE_BROWSER_STOCK, button_icon_size);
    gtk_container_add (GTK_CONTAINER (button), icon);

    gtk_widget_set_tooltip_text (button, _("Browser"));
    gtk_box_pack_start (GTK_BOX (hbox_buttons), button, TRUE, TRUE, 0);

    mc_set_atk_name_description (button,
                                 _("Browser"),
                                 _("Click this button to start the browser"));

    /* add history button */
    button = gtk_button_new ();

    g_signal_connect (button, "clicked",
		      G_CALLBACK (mc_show_history), mc);
    g_signal_connect (button, "button_press_event",
		      G_CALLBACK (button_press_hack), mc);

    icon = gtk_image_new_from_stock (COMMANDLINE_HISTORY_STOCK, 								     button_icon_size);
    gtk_container_add (GTK_CONTAINER (button), icon);

    gtk_widget_set_tooltip_text (button, _("History"));
    gtk_box_pack_end (GTK_BOX (hbox_buttons), button, TRUE, TRUE, 0);

    mc_set_atk_name_description (button,
                                 _("History"),
                                 _("Click this button for the list of previous commands"));

    gtk_box_pack_start (GTK_BOX (mc->applet_box), mc->entry, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (mc->applet_box), hbox_buttons, TRUE, TRUE, 0);

    gtk_container_add (GTK_CONTAINER (mc), mc->applet_box);

    gtk_widget_show_all (mc->applet_box);
}

static void
placement_changed_cb (GpApplet            *applet,
                      GtkOrientation       orientation,
                      GtkPositionType      position,
                      MiniCommanderApplet *self)
{
  self->orient = orientation;
  mc_applet_draw (self);
}

static void
mc_pixel_size_changed (GtkWidget     *widget,
                       GtkAllocation *allocation,
                       MCData        *mc)
{
  if (mc->orient == GTK_ORIENTATION_VERTICAL) {
    if (mc->preferences.panel_size_x == allocation->width)
      return;
    mc->preferences.panel_size_x = allocation->width;
  } else {
    if (mc->preferences.normal_size_y == allocation->height)
      return;
    mc->preferences.normal_size_y = allocation->height;
  }

  mc_applet_draw (mc);
}

static void
mini_commander_applet_fill (MCData *mc)
{
    GSettings *settings;
    const char *menu_resource;
    GAction *action;

    settings = g_settings_new (GNOME_DESKTOP_LOCKDOWN_SCHEMA);
    if (g_settings_get_boolean (settings, "disable-command-line")) {
	    GtkWidget *error_dialog;

	    error_dialog = gtk_message_dialog_new (NULL,
						   GTK_DIALOG_DESTROY_WITH_PARENT,
						   GTK_MESSAGE_ERROR,
						   GTK_BUTTONS_OK,
						   _("Command line has been disabled by your system administrator"));

	    gtk_window_set_resizable (GTK_WINDOW (error_dialog), FALSE);
	    gtk_window_set_screen (GTK_WINDOW (error_dialog),
				   gtk_widget_get_screen (GTK_WIDGET (mc)));
	    gtk_dialog_run (GTK_DIALOG (error_dialog));
	    gtk_widget_destroy (error_dialog);

	    /* Note that this is only kosher if this is an out of process thing,
	       which we really are.  We really don't need/want this applet when
	       command line is disabled */
	    /* exit (1); */
	    g_object_unref (settings);
	    return;
    }
    g_object_unref (settings);

    mc->global_settings = g_settings_new (MINI_COMMANDER_GLOBAL_SCHEMA);
    mc->settings = gp_applet_settings_new (GP_APPLET (mc), MINI_COMMANDER_SCHEMA);

    gp_applet_set_flags (GP_APPLET (mc), GP_APPLET_FLAGS_EXPAND_MINOR);
    mc_load_preferences (mc);
    command_line_init_stock_icons ();

    g_signal_connect (mc, "placement-changed",
		      G_CALLBACK (placement_changed_cb), mc);
    g_signal_connect (mc, "size-allocate",
		      G_CALLBACK (mc_pixel_size_changed), mc);

    mc->orient = gp_applet_get_orientation (GP_APPLET (mc));
    mc_applet_draw(mc);
    gtk_widget_show (GTK_WIDGET (mc));

    g_signal_connect (mc, "button_press_event",
		      G_CALLBACK (send_button_to_entry_event), mc);
    g_signal_connect (mc, "key_press_event",
		      G_CALLBACK (key_press_cb), mc);

    menu_resource = GRESOURCE_PREFIX "/ui/mini-commander-applet-menu.ui";
    gp_applet_setup_menu_from_resource (GP_APPLET (mc),
                                        menu_resource,
                                        mini_commander_menu_actions);

    action = gp_applet_menu_lookup_action (GP_APPLET (mc), "preferences");
	g_object_bind_property (mc, "locked-down",
	                        action, "enabled",
	                        G_BINDING_DEFAULT|G_BINDING_INVERT_BOOLEAN|G_BINDING_SYNC_CREATE);

    mc_set_atk_name_description (GTK_WIDGET (mc),
                                 _("Mini-Commander applet"),
                                 _("This applet adds a command line to the panel"));
}

static void
mini_commander_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (mini_commander_applet_parent_class)->constructed (object);
  mini_commander_applet_fill (MINI_COMMANDER_APPLET (object));
}

static void
mini_commander_applet_dispose (GObject *object)
{
  MiniCommanderApplet *self;

  self = MINI_COMMANDER_APPLET (object);

  g_clear_object (&self->global_settings);
  g_clear_object (&self->settings);
  g_clear_object (&self->provider);

  if (self->preferences.macros != NULL)
    {
      mc_macros_free (self->preferences.macros);
      self->preferences.macros = NULL;
    }

  g_clear_pointer (&self->preferences.cmd_line_color_fg, g_free);
  g_clear_pointer (&self->preferences.cmd_line_color_bg, g_free);

  if (self->prefs_dialog.dialog != NULL)
    {
      gtk_widget_destroy (self->prefs_dialog.dialog);
      g_object_unref (self->prefs_dialog.macros_store);

      self->prefs_dialog.dialog = NULL;
    }

  g_clear_pointer (&self->file_select, gtk_widget_destroy);

  G_OBJECT_CLASS (mini_commander_applet_parent_class)->dispose (object);
}

static void
mini_commander_applet_class_init (MiniCommanderAppletClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->constructed = mini_commander_applet_constructed;
  object_class->dispose = mini_commander_applet_dispose;
}

static void
mini_commander_applet_init (MiniCommanderApplet *self)
{
  self->provider = gtk_css_provider_new ();
}

void
mini_commander_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **documenters;
  const char *copyright;

  comments = _("This GNOME applet adds a command line to "
               "the panel. It features command completion, "
               "command history, and changeable macros.");

  authors = (const char *[])
    {
      "Oliver Maruhn <oliver@maruhn.com>",
      "Mark McLoughlin <mark@skynet.ie>",
      NULL
    };

  documenters = (const char *[])
    {
      "Dan Mueth <d-mueth@uchicago.edu>",
      "Oliver Maruhn <oliver@maruhn.com>",
      "Sun GNOME Documentation Team <gdocteam@sun.com>",
      NULL
    };

  copyright = "\xc2\xa9 1998-2005 Oliver Maruhn and others";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
