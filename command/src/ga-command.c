/*
 * Copyright (C) 2018-2020 Alberts Muktupāvels
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "ga-command.h"

#define BUFFER_SIZE 64

struct _GaCommand
{
  GObject        parent;

  char          *command;
  unsigned int   interval;

  char         **argv;

  gboolean       started;

  GPid           pid;

  GIOChannel    *channel;

  GString       *input;

  unsigned int   io_watch_id;
  unsigned int   child_watch_id;

  unsigned int   timeout_id;
};

enum
{
  PROP_0,

  PROP_COMMAND,
  PROP_INTERVAL,

  LAST_PROP
};

static GParamSpec *command_properties[LAST_PROP] = { NULL };

enum
{
  OUTPUT,
  ERROR,

  LAST_SIGNAL
};

static unsigned int command_signals[LAST_SIGNAL] = { 0 };

static void initable_iface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (GaCommand, ga_command, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                initable_iface_init))

static void command_execute (GaCommand *self);

static void
command_clear (GaCommand *self)
{
  if (self->pid != 0)
    {
      g_spawn_close_pid (self->pid);
      self->pid = 0;
    }

  if (self->channel != NULL)
    {
      g_io_channel_unref (self->channel);
      self->channel = NULL;
    }

  if (self->input != NULL)
    {
      g_string_free (self->input, TRUE);
      self->input = NULL;
    }

  if (self->io_watch_id != 0)
    {
      g_source_remove (self->io_watch_id);
      self->io_watch_id = 0;
    }

  if (self->child_watch_id != 0)
    {
      g_source_remove (self->child_watch_id);
      self->child_watch_id = 0;
    }
}

static gboolean
execute_cb (gpointer user_data)
{
  GaCommand *self;

  self = GA_COMMAND (user_data);
  self->timeout_id = 0;

  command_execute (self);

  return G_SOURCE_REMOVE;
}

static void
start_timeout (GaCommand *self)
{
  command_clear (self);

  g_assert (self->timeout_id == 0);
  self->timeout_id = g_timeout_add_seconds (self->interval, execute_cb, self);
  g_source_set_name_by_id (self->timeout_id, "[gnome-applets] execute_cb");
}

static gboolean
read_cb (GIOChannel   *source,
         GIOCondition  condition,
         gpointer      user_data)
{
  GaCommand *self;
  char buffer[BUFFER_SIZE];
  gsize bytes_read;
  GError *error;
  GIOStatus status;

  self = GA_COMMAND (user_data);

  error = NULL;
  status = g_io_channel_read_chars (source,
                                    buffer,
                                    BUFFER_SIZE,
                                    &bytes_read,
                                    &error);

  if (status == G_IO_STATUS_AGAIN)
    {
      g_clear_error (&error);

      return G_SOURCE_CONTINUE;
    }
  else if (status != G_IO_STATUS_NORMAL)
    {
      if (error != NULL)
        {
          g_signal_emit (self, command_signals[ERROR], 0, error);
          g_error_free (error);

          start_timeout (self);
        }

      self->io_watch_id = 0;

      return G_SOURCE_REMOVE;
    }

  g_string_append_len (self->input, buffer, bytes_read);

  return G_SOURCE_CONTINUE;
}

static void
child_watch_cb (GPid     pid,
                gint     status,
                gpointer user_data)
{
  GaCommand *self;

  self = GA_COMMAND (user_data);

  g_signal_emit (self, command_signals[OUTPUT], 0, self->input->str);
  start_timeout (self);
}

static void
command_execute (GaCommand *self)
{
  GSpawnFlags spawn_flags;
  GError *error;
  int command_stdout;
  GIOChannel *channel;
  GIOStatus status;
  GIOCondition condition;

  spawn_flags = G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD;
  error = NULL;

  if (!g_spawn_async_with_pipes (NULL, self->argv, NULL, spawn_flags,
                                 NULL, NULL, &self->pid, NULL, &command_stdout,
                                 NULL, &error))
    {
      g_signal_emit (self, command_signals[ERROR], 0, error);
      g_error_free (error);

      start_timeout (self);
      return;
    }

  channel = self->channel = g_io_channel_unix_new (command_stdout);
  g_io_channel_set_close_on_unref (channel, TRUE);

  g_assert (error == NULL);
  status = g_io_channel_set_encoding (channel, NULL, &error);

  if (status != G_IO_STATUS_NORMAL)
    {
      g_signal_emit (self, command_signals[ERROR], 0, error);
      g_error_free (error);

      start_timeout (self);
      return;
    }

  g_assert (error == NULL);
  status = g_io_channel_set_flags (channel, G_IO_FLAG_NONBLOCK, &error);

  if (status != G_IO_STATUS_NORMAL)
    {
      g_signal_emit (self, command_signals[ERROR], 0, error);
      g_error_free (error);

      start_timeout (self);
      return;
    }

  self->input = g_string_new (NULL);

  condition = G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP;
  self->io_watch_id = g_io_add_watch (channel, condition, read_cb, self);

  self->child_watch_id = g_child_watch_add (self->pid, child_watch_cb, self);
}

static gboolean
ga_command_initable_init (GInitable     *initable,
                          GCancellable  *cancellable,
                          GError       **error)
{
  GaCommand *self;

  self = GA_COMMAND (initable);

  if (self->command == NULL || *self->command == '\0')
    {
      g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                   "Empty command");

      return FALSE;
    }

  if (!g_shell_parse_argv (self->command, NULL, &self->argv, error))
    return FALSE;

  return TRUE;
}

static void
initable_iface_init (GInitableIface *iface)
{
  iface->init = ga_command_initable_init;
}

static void
ga_command_finalize (GObject *object)
{
  GaCommand *self;

  self = GA_COMMAND (object);

  ga_command_stop (self);

  g_clear_pointer (&self->command, g_free);
  g_clear_pointer (&self->argv, g_strfreev);

  G_OBJECT_CLASS (ga_command_parent_class)->finalize (object);
}

static void
ga_command_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  GaCommand *self;

  self = GA_COMMAND (object);

  switch (property_id)
    {
      case PROP_COMMAND:
        g_assert (self->command == NULL);
        self->command = g_value_dup_string (value);
        break;

      case PROP_INTERVAL:
        self->interval = g_value_get_uint (value);
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
install_properties (GObjectClass *object_class)
{
  command_properties[PROP_COMMAND] =
    g_param_spec_string ("command",
                         "command",
                         "command",
                         NULL,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
                         G_PARAM_STATIC_STRINGS);

  command_properties[PROP_INTERVAL] =
    g_param_spec_uint ("interval",
                       "interval",
                       "interval",
                       1,
                       600,
                       1,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE |
                       G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class,
                                     LAST_PROP,
                                     command_properties);
}

static void
install_signal (void)
{
  command_signals[OUTPUT] =
    g_signal_new ("output",
                  GA_TYPE_COMMAND,
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  NULL,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);

  command_signals[ERROR] =
    g_signal_new ("error",
                  GA_TYPE_COMMAND,
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  NULL,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_ERROR);
}

static void
ga_command_class_init (GaCommandClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->finalize = ga_command_finalize;
  object_class->set_property = ga_command_set_property;

  install_properties (object_class);
  install_signal ();
}

static void
ga_command_init (GaCommand *self)
{
}

GaCommand *
ga_command_new (const char    *command,
                unsigned int   interval,
                GError       **error)
{
  return g_initable_new (GA_TYPE_COMMAND, NULL, error,
                         "command", command,
                         "interval", interval,
                         NULL);
}

const char *
ga_command_get_command (GaCommand *self)
{
  return self->command;
}

void
ga_command_set_interval (GaCommand    *self,
                         unsigned int  interval)
{
  if (self->interval == interval)
    return;

  self->interval = interval;

  if (!self->started)
    return;

  ga_command_restart (self);
}

void
ga_command_start (GaCommand *self)
{
  if (self->started)
    return;

  self->started = TRUE;

  command_execute (self);
}

void
ga_command_stop (GaCommand *self)
{
  if (self->timeout_id != 0)
    {
      g_source_remove (self->timeout_id);
      self->timeout_id = 0;
    }

  command_clear (self);

  self->started = FALSE;
}

void
ga_command_restart (GaCommand *self)
{
  ga_command_stop (self);
  ga_command_start (self);
}
