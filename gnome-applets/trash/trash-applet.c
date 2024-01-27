/*
 * Copyright (c) 2004  Michiel Sikkes <michiel@eyesopened.nl>,
 *               2004  Emmanuele Bassi <ebassi@gmail.com>
 *               2008  Ryan Lortie <desrt@desrt.ca>
 *                     Matthias Clasen <mclasen@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
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
#include "trash-applet.h"

#include <string.h>

#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gio/gio.h>

#include "trash-empty.h"

struct _TrashApplet
{
  GpApplet parent;

  GFileMonitor *trash_monitor;
  GFile *trash;

  GtkWidget *image;
  GIcon *icon;
  gint items;
};

G_DEFINE_TYPE (TrashApplet, trash_applet, GP_TYPE_APPLET)

static void trash_applet_do_empty    (GSimpleAction *action,
                                      GVariant      *parameter,
                                      gpointer       user_data);
static void trash_applet_open_folder (GSimpleAction *action,
                                      GVariant      *parameter,
                                      gpointer       user_data);

static void
trash_applet_show_help (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       user_data)
{
  gp_applet_show_help (GP_APPLET (user_data), NULL);
}

static void
trash_applet_show_about (GSimpleAction *action,
                         GVariant      *parameter,
                         gpointer       user_data)
{
  gp_applet_show_about (GP_APPLET (user_data));
}

static const GActionEntry trash_applet_menu_actions [] = {
	{ "open",  trash_applet_open_folder, NULL, NULL, NULL },
	{ "empty", trash_applet_do_empty,    NULL, NULL, NULL },
	{ "help",  trash_applet_show_help,   NULL, NULL, NULL },
	{ "about", trash_applet_show_about,   NULL, NULL, NULL },
	{ NULL }
};

static void
trash_applet_monitor_changed (TrashApplet *applet)
{
  GError *error = NULL;
  GFileInfo *info;
  GIcon *icon;
  gint items;

  info = g_file_query_info (applet->trash,
                            G_FILE_ATTRIBUTE_STANDARD_ICON","
                            G_FILE_ATTRIBUTE_TRASH_ITEM_COUNT,
                            0, NULL, &error);

  if (!info)
    {
      g_critical ("could not query trash:/: '%s'", error->message);
      g_error_free (error);

      return;
    }

  icon = g_file_info_get_icon (info);
  items = g_file_info_get_attribute_uint32 (info,
                                            G_FILE_ATTRIBUTE_TRASH_ITEM_COUNT);

  if (!g_icon_equal (icon, applet->icon))
    {
      /* note: the size is meaningless here,
       * since we do set_pixel_size() later
       */
      gtk_image_set_from_gicon (GTK_IMAGE (applet->image),
                                icon, GTK_ICON_SIZE_MENU);

      if (applet->icon)
        g_object_unref (applet->icon);

      applet->icon = g_object_ref (icon);
    }

  if (items != applet->items)
    {
      if (items)
        {
          char *text;

          text = g_strdup_printf (dngettext (GETTEXT_PACKAGE,
                                             "%d Item in Trash",
                                             "%d Items in Trash",
                                             items), items);
          gtk_widget_set_tooltip_text (GTK_WIDGET (applet), text);
          g_free (text);
        }
      else
        gtk_widget_set_tooltip_text (GTK_WIDGET (applet),
                                     _("No Items in Trash"));

      applet->items = items;
    }

  g_object_unref (info);
}

static void
trash_applet_set_icon_size (TrashApplet *applet,
                            gint         size)
{
  /* copied from button-widget.c in the panel */
  if (size < 22)
    size = 16;
  else if (size < 24)
    size = 22;
  else if (size < 32)
    size = 24;
  else if (size < 48)
    size = 32;
  else
    size = 48;

  /* GtkImage already contains a check to do nothing if it's the same */
  gtk_image_set_pixel_size (GTK_IMAGE (applet->image), size);
}

static void
trash_applet_size_allocate (GtkWidget    *widget,
                            GdkRectangle *allocation)
{
  TrashApplet *applet = TRASH_APPLET (widget);

  switch (gp_applet_get_orientation (GP_APPLET (applet)))
  {
    case GTK_ORIENTATION_VERTICAL:
      trash_applet_set_icon_size (applet, allocation->width);
      break;

    case GTK_ORIENTATION_HORIZONTAL:
      trash_applet_set_icon_size (applet, allocation->height);
      break;

    default:
      g_assert_not_reached ();
      break;
  }

  GTK_WIDGET_CLASS (trash_applet_parent_class)
    ->size_allocate (widget, allocation);
}

static void
trash_applet_setup (TrashApplet *self)
{
  const GtkTargetEntry drop_types[] = { { (char *) "text/uri-list" } };
  const gchar *resource_name;

  /* needed to clamp ourselves to the panel size */
  gp_applet_set_flags (GP_APPLET (self), GP_APPLET_FLAGS_EXPAND_MINOR);

  /* Set up the menu */
  resource_name = GRESOURCE_PREFIX "/ui/trash-menu.ui";
  gp_applet_setup_menu_from_resource (GP_APPLET (self),
                                      resource_name,
                                      trash_applet_menu_actions);

  /* setup the image */
  self->image = g_object_ref_sink (gtk_image_new ());
  gtk_container_add (GTK_CONTAINER (self), self->image);
  gtk_widget_show (self->image);

  /* setup the trash backend */
  self->trash = g_file_new_for_uri ("trash:/");
  self->trash_monitor = g_file_monitor_file (self->trash, 0, NULL, NULL);
  g_file_monitor_set_rate_limit (self->trash_monitor, 200);
  g_signal_connect_swapped (self->trash_monitor, "changed",
                            G_CALLBACK (trash_applet_monitor_changed),
                            self);

  /* setup drag and drop */
  gtk_drag_dest_set (GTK_WIDGET (self), GTK_DEST_DEFAULT_ALL,
                     drop_types, G_N_ELEMENTS (drop_types),
                     GDK_ACTION_MOVE);

  /* synthesise the first update */
  self->items = -1;
  trash_applet_monitor_changed (self);
}

static void
trash_applet_constructed (GObject *object)
{
  G_OBJECT_CLASS (trash_applet_parent_class)->constructed (object);
  trash_applet_setup (TRASH_APPLET (object));
}

static void
trash_applet_dispose (GObject *object)
{
  TrashApplet *applet = TRASH_APPLET (object);

  if (applet->trash_monitor)
    g_object_unref (applet->trash_monitor);
  applet->trash_monitor = NULL;

  if (applet->trash)
    g_object_unref (applet->trash);
  applet->trash = NULL;

  if (applet->image)
    g_object_unref (applet->image);
  applet->image = NULL;

  if (applet->icon)
    g_object_unref (applet->icon);
  applet->icon = NULL;

  G_OBJECT_CLASS (trash_applet_parent_class)->dispose (object);
}

static gboolean
trash_applet_button_release (GtkWidget      *widget,
                             GdkEventButton *event)
{
  TrashApplet *applet = TRASH_APPLET (widget);

  if (event->button == 1)
    {
      trash_applet_open_folder (NULL, NULL, applet);

      return TRUE;
    }

  if (GTK_WIDGET_CLASS (trash_applet_parent_class)->button_release_event)
    return GTK_WIDGET_CLASS (trash_applet_parent_class)
        ->button_release_event (widget, event);
  else
    return FALSE;
}
static gboolean
trash_applet_key_press (GtkWidget   *widget,
                        GdkEventKey *event)
{
  TrashApplet *applet = TRASH_APPLET (widget);

  switch (event->keyval)
    {
     case GDK_KEY_KP_Enter:
     case GDK_KEY_ISO_Enter:
     case GDK_KEY_3270_Enter:
     case GDK_KEY_Return:
     case GDK_KEY_space:
     case GDK_KEY_KP_Space:
      trash_applet_open_folder (NULL, NULL, applet);
      return TRUE;

     default:
      break;
    }

  if (GTK_WIDGET_CLASS (trash_applet_parent_class)->key_press_event)
    return GTK_WIDGET_CLASS (trash_applet_parent_class)
      ->key_press_event (widget, event);
  else
    return FALSE;
}

static gboolean
trash_applet_drag_motion (GtkWidget      *widget,
                          GdkDragContext *context,
                          gint            x,
                          gint            y,
                          guint           time)
{
  GList *target;

  /* refuse drops of panel applets */
  for (target = gdk_drag_context_list_targets (context); target; target = target->next)
    {
      const char *name = gdk_atom_name (target->data);

      if (!strcmp (name, "application/x-panel-icon-internal"))
        break;
    }

  if (target)
    gdk_drag_status (context, 0, time);
  else
    gdk_drag_status (context, GDK_ACTION_MOVE, time);

  return TRUE;
}

static void
error_dialog (TrashApplet *applet,
              const gchar *error,
              ...) G_GNUC_PRINTF (2, 3);

/* TODO - Must HIGgify this dialog */
static void
error_dialog (TrashApplet *applet, const gchar *error, ...)
{
  va_list args;
  gchar *error_string;
  GtkWidget *dialog;

  g_return_if_fail (error != NULL);

  va_start (args, error);
  error_string = g_strdup_vprintf (error, args);
  va_end (args);

  dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                   "%s", error_string);

  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK (gtk_widget_destroy),
                    NULL);

  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_window_set_screen (GTK_WINDOW(dialog),
                         gtk_widget_get_screen (GTK_WIDGET (applet)));
  gtk_widget_show (dialog);

  g_free (error_string);
}

static void
trash_applet_do_empty (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       user_data)
{
  TrashApplet *applet = (TrashApplet *) user_data;
  trash_empty (GTK_WIDGET (applet));
}

static void
trash_applet_open_folder (GSimpleAction *action,
                          GVariant      *parameter,
                          gpointer       user_data)
{
  TrashApplet *applet = (TrashApplet *) user_data;
  GError *err = NULL;

  gtk_show_uri (gtk_widget_get_screen (GTK_WIDGET (applet)),
                "trash:",
                gtk_get_current_event_time (),
                &err);

  if (err)
    {
      error_dialog (applet, _("Error while spawning Nautilus:\n%s"),
      err->message);
      g_error_free (err);
    }
}

static gboolean
confirm_delete_immediately (GtkWidget *parent_view,
                            gint num_files,
                            gboolean all)
{
  GdkScreen *screen;
  GtkWidget *dialog, *hbox, *vbox, *image, *label;
  gchar *str, *prompt, *detail;
  int response;

  screen = gtk_widget_get_screen (parent_view);

  dialog = gtk_dialog_new ();
  gtk_window_set_screen (GTK_WINDOW (dialog), screen);
  atk_object_set_role (gtk_widget_get_accessible (dialog), ATK_ROLE_ALERT);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Delete Immediately?"));
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  gtk_widget_realize (dialog);
  gdk_window_set_transient_for (gtk_widget_get_window (GTK_WIDGET (dialog)),
                                gdk_screen_get_root_window (screen));
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  gtk_box_set_spacing (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), 14);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), hbox,
                      FALSE, FALSE, 0);

  image = gtk_image_new_from_icon_name ("dialog-question", GTK_ICON_SIZE_DIALOG);
  gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
  gtk_widget_set_valign (image, GTK_ALIGN_START);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  if (all)
    {
      prompt = _("Cannot move items to trash, do you want to delete them immediately?");
      detail = g_strdup_printf (dngettext (GETTEXT_PACKAGE,
                                           "The selected item cannot be moved to the trash",
                                           "None of the %d selected items can be moved to the Trash",
                                           num_files), num_files);
    }
  else
    {
      prompt = _("Cannot move some items to trash, do you want to delete these immediately?");
      detail = g_strdup_printf (dngettext (GETTEXT_PACKAGE,
                                           "The selected item cannot be moved to the Trash",
                                           "%d of the selected items cannot be moved to the Trash",
                                           num_files) , num_files);
    }

  str = g_strconcat ("<span weight=\"bold\" size=\"larger\">",
                     prompt, "</span>", NULL);
  label = gtk_label_new (str);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  g_free (str);

  label = gtk_label_new (detail);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  g_free (detail);

  gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Cancel"),
                         GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Delete"),
                         GTK_RESPONSE_YES);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                   GTK_RESPONSE_YES);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  gtk_widget_destroy (GTK_WIDGET (dialog));

  return response == GTK_RESPONSE_YES;
}

static void
trash_applet_drag_data_received (GtkWidget        *widget,
                                 GdkDragContext   *context,
                                 gint              x,
                                 gint              y,
                                 GtkSelectionData *selectiondata,
                                 guint             info,
                                 guint             time_)
{
  gchar **list;
  gint i;
  GList *trashed = NULL;
  GList *untrashable = NULL;
  GList *l;
  GError *error = NULL;

  list = g_uri_list_extract_uris ((gchar *)gtk_selection_data_get_data (selectiondata));

  for (i = 0; list[i]; i++)
    {
      GFile *file;

      file = g_file_new_for_uri (list[i]);

      if (!g_file_trash (file, NULL, NULL))
        {
          untrashable = g_list_prepend (untrashable, file);
        }
      else
        {
          trashed = g_list_prepend (trashed, file);
        }
    }

  if (untrashable)
    {
      if (confirm_delete_immediately (widget,
                                      g_list_length (untrashable),
                                      trashed == NULL))
        {
          for (l = untrashable; l; l = l->next)
            {
              if (!g_file_delete (l->data, NULL, &error))
                {
/*
* FIXME: uncomment me after branched (we're string frozen)
                  error_dialog (applet,
                                _("Unable to delete '%s': %s"),
                                g_file_get_uri (l->data),
                                error->message);
*/
                                g_clear_error (&error);
                }
            }
        }
    }

  g_list_free_full (untrashable, g_object_unref);
  g_list_free_full (trashed, g_object_unref);

  g_strfreev (list);

  gtk_drag_finish (context, TRUE, FALSE, time_);
}

static void
trash_applet_class_init (TrashAppletClass *self_class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = G_OBJECT_CLASS (self_class);
  widget_class = GTK_WIDGET_CLASS (self_class);

  object_class->constructed = trash_applet_constructed;
  object_class->dispose = trash_applet_dispose;

  widget_class->size_allocate = trash_applet_size_allocate;
  widget_class->button_release_event = trash_applet_button_release;
  widget_class->key_press_event = trash_applet_key_press;
  widget_class->drag_motion = trash_applet_drag_motion;
  widget_class->drag_data_received = trash_applet_drag_data_received;
}

static void
trash_applet_init (TrashApplet *self)
{
}

void
trash_applet_setup_about (GtkAboutDialog *dialog)
{
  const char *comments;
  const char **authors;
  const char **documenters;
  const char *copyright;

  comments = _("A GNOME trash bin that lives in your panel. "
               "You can use it to view the trash or drag "
               "and drop items into the trash.");

  authors = (const char *[])
    {
      "Michiel Sikkes <michiel@eyesopened.nl>",
      "Emmanuele Bassi <ebassi@gmail.com>",
      "Sebastian Bacher <seb128@canonical.com>",
      "James Henstridge <james.henstridge@canonical.com>",
      "Ryan Lortie <desrt@desrt.ca>",
      NULL
    };

  documenters = (const char *[])
    {
      "Michiel Sikkes <michiel@eyesopened.nl>",
      NULL
    };

  copyright = "Copyright \xC2\xA9 2004 Michiel Sikkes, "
              "\xC2\xA9 2008 Ryan Lortie";

  gtk_about_dialog_set_comments (dialog, comments);

  gtk_about_dialog_set_authors (dialog, authors);
  gtk_about_dialog_set_documenters (dialog, documenters);
  gtk_about_dialog_set_translator_credits (dialog, _("translator-credits"));
  gtk_about_dialog_set_copyright (dialog, copyright);
}
