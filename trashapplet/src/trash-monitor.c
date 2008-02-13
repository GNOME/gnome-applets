/*
 * trash-monitor.c: monitor the state of the trash directories.
 *
 * Copyright © 2008 Ryan Lortie, Matthias Clasen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <gio/gio.h>

#include "trash-monitor.h"

/* theory of operation:
 *
 * it's not possible to use file monitoring to keep track of the exact number
 * of items in the trash, so we operate in the following way:
 *
 * 1) always keep track of if the trash is empty or not.
 *
 *   - on initialisation, we check if there is at least one item in the
 *     trash and decide based on that if it is empty or not.
 *
 *   - if we receive a file monitor "created" event then the trash is no
 *     longer empty
 *
 *   - if we receive a file monitor "deleted" event, we need to check again
 *     (for at least one item) to determine emptiness.  this is done with a
 *     timeout of 0.1s that is reschedule on each delete so that we don't
 *     do a query for every single delete event (think: "empty trash").
 *
 *   - if the empty state changes, emit "notify::empty"
 *
 * 2) only query the actual number of items in the trash when requested
 *
 *   - keep a cache and used the cached value if present.
 *
 *   - we know when the trash is empty, so no need to check in this case
 *     either.
 *
 *   - else, manually scan the trash directory.
 *
 *   - invalidate the cache on file monitor events.  even though we can't
 *     be *absolutely* sure that it changed, emit "notify::items".
 */

struct _TrashMonitor
{
  GObject parent;

  GFile *trash;
  gboolean empty;
  guint empty_check_id;
  GFileMonitor *file_monitor;

  gboolean counted_items_valid;
  int counted_items;
};

struct _TrashMonitorClass
{
  GObjectClass parent_class;
};

enum
{
  PROP_NONE,
  PROP_EMPTY,
  PROP_ITEMS
};

G_DEFINE_TYPE (TrashMonitor, trash_monitor, G_TYPE_OBJECT);

static void
trash_monitor_check_empty (TrashMonitor *monitor)
{
  GFileEnumerator *enumerator;
  GError *error = NULL;
  gboolean empty;

  enumerator = g_file_enumerate_children (monitor->trash, "",
                                          G_FILE_QUERY_INFO_NONE,
                                          NULL, &error);

  if (enumerator)
    {
      GFileInfo *info;

      if ((info = g_file_enumerator_next_file (enumerator, NULL, NULL)))
        {
          g_object_unref (info);

          /* not empty, clearly */
          empty = FALSE;
        }
      else
        {
          /* couldn't even read one file.  empty. */
          empty = TRUE;
        }

      g_object_unref (enumerator);
    }
  else
    {
      static gboolean showed_warning;

      if (!showed_warning)
        {
          g_warning ("could not obtain enumerator for trash:///: %s",
                      error->message);
          showed_warning = TRUE;
        }

      g_error_free (error);

      /* can't even open the trash directory.  assume empty. */
      empty = TRUE;
    }

  if (empty != monitor->empty)
    {
      monitor->empty = empty;
      g_object_notify (G_OBJECT (monitor), "empty");
    }
}

static gboolean
trash_monitor_idle_check_empty (gpointer user_data)
{
  TrashMonitor *monitor = user_data;

  trash_monitor_check_empty (monitor);
  monitor->empty_check_id = 0;

  return FALSE;
}

static int
trash_monitor_count_items (TrashMonitor *monitor)
{
  GFileEnumerator *enumerator;
  GError *error = NULL;
  GFileInfo *info;
  int total;

  if (monitor->counted_items_valid)
    return monitor->counted_items;

  if (monitor->empty)
    {
      monitor->counted_items = 0;
      return 0;
    }

  enumerator = g_file_enumerate_children (monitor->trash, "",
                                          G_FILE_QUERY_INFO_NONE, NULL, NULL);

  if (enumerator == NULL)
    /* the empty-check will have already thrown a g_warning for this */
    return 0;

  total = 0;
  while ((info = g_file_enumerator_next_file (enumerator, NULL, NULL)))
    {
      g_object_unref (info);
      total++;
    }

  if (error)
    {
      static gboolean showed_warning;

      if (!showed_warning)
        {
          g_warning ("error while enumerating trash:///: %s\n",
                     error->message);
          showed_warning = TRUE;
        }

      g_error_free (error);
    }
  else
    {
      monitor->counted_items_valid = TRUE;
      monitor->counted_items = total;
    }

  g_object_unref (enumerator);

  return total;
}

static void
trash_monitor_changed (GFileMonitor      *file_monitor,
                       GFile             *child,
                       GFile             *other_file,
                       GFileMonitorEvent  event_type,
                       gpointer           user_data)
{
  TrashMonitor *monitor = user_data;

  switch (event_type) 
  {
    case G_FILE_MONITOR_EVENT_DELETED:
      /* wait until 0.1s after the last delete event before checking.
       * this prevents us from re-checking for every delete event while
       * emptying the trash.
       */
      if (monitor->empty_check_id)
        g_source_remove (monitor->empty_check_id);

      monitor->empty_check_id = g_timeout_add (100,
                                               trash_monitor_idle_check_empty,
                                               monitor);
      break;

    case G_FILE_MONITOR_EVENT_CREATED:
      if (monitor->empty)
        {
          /* not any more... */
          monitor->empty = FALSE;
          g_object_notify (G_OBJECT (monitor), "empty");
        }
      break;

    default:
      return;
  }  

  if (monitor->counted_items_valid)
    {
      monitor->counted_items_valid = FALSE;
      g_object_notify (G_OBJECT (monitor), "items");
    }
}

static void
trash_monitor_get_property (GObject *object, guint prop_id,
                            GValue *value, GParamSpec *pspec)
{
  TrashMonitor *monitor = TRASH_MONITOR (object);

  switch (prop_id)
  {
    case PROP_EMPTY:
      g_value_set_boolean (value, monitor->empty);
      break;

    case PROP_ITEMS:
      g_value_set_int (value, trash_monitor_count_items (monitor));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
trash_monitor_class_init (TrashMonitorClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->get_property = trash_monitor_get_property;

  g_object_class_install_property (object_class, PROP_EMPTY,
    g_param_spec_boolean ("empty",
                          "trash is empty",
                          "true if the trash is empty",
                          TRUE,
                          G_PARAM_READABLE));

  g_object_class_install_property (object_class, PROP_ITEMS,
    g_param_spec_int ("items",
                      "item count",
                      "the number of items in the trash",
                      0, G_MAXINT, 0,
                      G_PARAM_READABLE));
}

static void
trash_monitor_init (TrashMonitor *monitor)
{
  GError *error;

  monitor->trash = g_file_new_for_uri ("trash:///");
  monitor->file_monitor = g_file_monitor_directory (monitor->trash, 0,
                                                    NULL, &error);

  if (monitor->file_monitor == NULL)
    {
      g_warning ("failed to register watch on trash:///: %s", error->message);
      g_error_free (error);
    }

  trash_monitor_check_empty (monitor);

  g_signal_connect_object (monitor->file_monitor, "changed",
			   G_CALLBACK (trash_monitor_changed), monitor, 0);
}

TrashMonitor *
trash_monitor_new (void)
{
  static TrashMonitor *monitor;

  if (!monitor)
    monitor = g_object_new (TRASH_TYPE_MONITOR, NULL);

  return g_object_ref (monitor);
}

/*
 * if @file is a directory, delete its contents (but not @file itself).
 * if @file is not a directory, do nothing.
 */
static void
trash_monitor_delete_contents (GFile        *file,
                               GCancellable *cancellable)
{
  GFileEnumerator *enumerator;
  GFileInfo *info;
  GFile *child;

  if (g_cancellable_is_cancelled (cancellable))
    return;

  enumerator = g_file_enumerate_children (file,
                                          G_FILE_ATTRIBUTE_STANDARD_NAME,
                                          G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                          cancellable, NULL);
  if (enumerator) 
    {
      while ((info = g_file_enumerator_next_file (enumerator,
                                                  cancellable, NULL)) != NULL)
        {
          child = g_file_get_child (file, g_file_info_get_name (info));

          /* it's just as fast to assume that all entries are subdirectories
           * and try to erase their contents as it is to actually do the
           * extra stat() call up front...
           */
          trash_monitor_delete_contents (child, cancellable);

          g_file_delete (child, cancellable, NULL);

          g_object_unref (child);
          g_object_unref (info);

          if (g_cancellable_is_cancelled (cancellable))
            break;
        }

      g_object_unref (enumerator);
    }
}

static gboolean
trash_monitor_empty_job (GIOSchedulerJob *job,
                         GCancellable    *cancellable,
                         gpointer         user_data)
{
  TrashMonitor *monitor = user_data;

  trash_monitor_delete_contents (monitor->trash, cancellable);

  return FALSE;
}

void
trash_monitor_empty_trash (TrashMonitor *monitor,
			   GCancellable *cancellable,
			   gpointer      func, 
			   gpointer      user_data)
{
  g_io_scheduler_push_job (trash_monitor_empty_job,
                           monitor, NULL, 0, cancellable);
}
