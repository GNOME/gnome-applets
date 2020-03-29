/*
 * Copyright (C) 2008 Carlos Garcia Campos
 * Copyright (C) 2020 Alberts Muktupāvels
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
#include "cpufreq-selector-service.h"

#include <polkit/polkit.h>

#include "cpufreq-selector.h"
#include "cpufreq-selector-gen.h"

struct _CPUFreqSelectorService
{
  GObject             parent;

  PolkitAuthority    *authority;

  CPUFreqSelectorGen *selector;
  guint               bus_name_id;

  guint               quit_timeout_id;

  GHashTable         *selectors;
};

enum
{
  SIGNAL_QUIT,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (CPUFreqSelectorService, cpufreq_selector_service, G_TYPE_OBJECT)

static gboolean
quit_timeout_cb (gpointer user_data)
{
  CPUFreqSelectorService *self;

	self = CPUFREQ_SELECTOR_SERVICE (user_data);
	self->quit_timeout_id = 0;

  g_signal_emit (self, signals[SIGNAL_QUIT], 0);

  return G_SOURCE_REMOVE;
}

static void
reset_quit_timeout (CPUFreqSelectorService *self)
{
  if (self->quit_timeout_id != 0)
    g_source_remove (self->quit_timeout_id);

  self->quit_timeout_id = g_timeout_add_seconds (30, quit_timeout_cb, self);

  g_source_set_name_by_id (self->quit_timeout_id,
                           "[gnome-applets] quit_timeout_cb");
}

static CPUFreqSelector *
get_selector_for_cpu (CPUFreqSelectorService *self,
                      guint                   cpu)
{
  CPUFreqSelector *selector;

  selector = g_hash_table_lookup (self->selectors, GUINT_TO_POINTER (cpu));

  if (selector == NULL)
    {
      selector = cpufreq_selector_new (cpu);

      g_hash_table_insert (self->selectors, GUINT_TO_POINTER (cpu), selector);
    }

  return selector;
}

static gboolean
service_check_policy (CPUFreqSelectorService  *self,
                      GDBusMethodInvocation   *invocation,
                      GError                 **error)
{
  const char *sender;
  PolkitSubject *subject;
  PolkitAuthorizationResult *result;
  gboolean is_authorized;

  sender = g_dbus_method_invocation_get_sender (invocation);
  subject = polkit_system_bus_name_new (sender);

  result = polkit_authority_check_authorization_sync (self->authority,
                                                      subject,
                                                      "org.gnome.cpufreqselector",
                                                      NULL,
                                                      POLKIT_CHECK_AUTHORIZATION_FLAGS_ALLOW_USER_INTERACTION,
                                                      NULL,
                                                      error);

  g_object_unref (subject);

  if (*error != NULL)
    {
      g_warning ("Check policy: %s", (*error)->message);
      g_object_unref (result);

      return FALSE;
    }

  is_authorized = polkit_authorization_result_get_is_authorized (result);
  g_object_unref (result);

  if (!is_authorized)
    {
      g_set_error (error,
                   G_DBUS_ERROR,
                   G_DBUS_ERROR_ACCESS_DENIED,
                   "Caller is not authorized");
    }

  return is_authorized;
}

static gboolean
handle_can_set_cb (CPUFreqSelectorGen    *object,
                   GDBusMethodInvocation *invocation,
                   gpointer               user_data)
{
  CPUFreqSelectorService *self;
  const char *sender;
  PolkitSubject *subject;
  GError *error;
  PolkitAuthorizationResult *result;
  gboolean can_set;

  self = CPUFREQ_SELECTOR_SERVICE (user_data);

  reset_quit_timeout (self);

  sender = g_dbus_method_invocation_get_sender (invocation);
  subject = polkit_system_bus_name_new (sender);

  error = NULL;
  result = polkit_authority_check_authorization_sync (self->authority,
                                                      subject,
                                                      "org.gnome.cpufreqselector",
                                                      NULL,
                                                      0,
                                                      NULL,
                                                      &error);

  g_object_unref (subject);

  if (error != NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      g_error_free (error);

      return FALSE;
    }

  can_set = FALSE;
  if (polkit_authorization_result_get_is_authorized (result) ||
      polkit_authorization_result_get_is_challenge (result))
    can_set = TRUE;

  g_object_unref (result);

  cpufreq_selector_gen_complete_can_set (object, invocation, can_set);

  return TRUE;
}

static gboolean
handle_set_frequency_cb (CPUFreqSelectorGen    *object,
                         GDBusMethodInvocation *invocation,
                         guint                  cpu,
                         guint                  frequency,
                         gpointer               user_data)
{
  CPUFreqSelectorService *self;
  GError *error;
  CPUFreqSelector *selector;

  self = CPUFREQ_SELECTOR_SERVICE (user_data);

  reset_quit_timeout (self);

  error = NULL;
  if (!service_check_policy (self, invocation, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      g_error_free (error);

      return FALSE;
    }

  selector = get_selector_for_cpu (self, cpu);

  if (selector == NULL)
    {
      error = g_error_new (G_DBUS_ERROR,
                           G_DBUS_ERROR_FAILED,
                           "Error setting frequency on cpu %d: No cpufreq support",
                           cpu);

      g_dbus_method_invocation_return_gerror (invocation, error);
      g_error_free (error);

      return FALSE;
    }

  cpufreq_selector_set_frequency (selector, frequency, &error);

  if (error != NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      g_error_free (error);

      return FALSE;
    }

  cpufreq_selector_gen_complete_set_frequency (object, invocation);

  return TRUE;
}

static gboolean
handle_set_governor_cb (CPUFreqSelectorGen    *object,
                        GDBusMethodInvocation *invocation,
                        guint                  cpu,
                        const gchar           *governor,
                        gpointer               user_data)
{
  CPUFreqSelectorService *self;
  GError *error;
  CPUFreqSelector *selector;

  self = CPUFREQ_SELECTOR_SERVICE (user_data);

  reset_quit_timeout (self);

  error = NULL;
  if (!service_check_policy (self, invocation, &error))
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      g_error_free (error);

      return FALSE;
    }

  selector = get_selector_for_cpu (self, cpu);

  if (selector == NULL)
    {
      error = g_error_new (G_DBUS_ERROR,
                           G_DBUS_ERROR_FAILED,
                           "Error setting governor on cpu %d: No cpufreq support",
                           cpu);

      g_dbus_method_invocation_return_gerror (invocation, error);
      g_error_free (error);

      return FALSE;
    }

  cpufreq_selector_set_governor (selector, governor, &error);

  if (error != NULL)
    {
      g_dbus_method_invocation_return_gerror (invocation, error);
      g_error_free (error);

      return FALSE;
    }

  cpufreq_selector_gen_complete_set_governor (object, invocation);

  return TRUE;
}

static void
bus_acquired_cb (GDBusConnection *connection,
                 const char      *name,
                 gpointer         user_data)
{
  CPUFreqSelectorService *self;
  GDBusInterfaceSkeleton *skeleton;
  GError *error;
  gboolean exported;

  self = CPUFREQ_SELECTOR_SERVICE (user_data);
  skeleton = G_DBUS_INTERFACE_SKELETON (self->selector);

  error = NULL;
  exported = g_dbus_interface_skeleton_export (skeleton,
                                               connection,
                                               "/org/gnome/cpufreq_selector/selector",
                                               &error);

  if (!exported)
    {
      g_warning ("Failed to export interface: %s", error->message);
      g_error_free (error);

      return;
    }

  g_signal_connect (self->selector,
                    "handle-can-set",
                    G_CALLBACK (handle_can_set_cb),
                    self);

  g_signal_connect (self->selector,
                    "handle-set-frequency",
                    G_CALLBACK (handle_set_frequency_cb),
                    self);

  g_signal_connect (self->selector,
                    "handle-set-governor",
                    G_CALLBACK (handle_set_governor_cb),
                    self);
}

static void
name_acquired_cb (GDBusConnection *connection,
                  const char      *name,
                  gpointer         user_data)
{
}

static void
name_lost_cb (GDBusConnection *connection,
              const char      *name,
              gpointer         user_data)
{
}

static void
cpufreq_selector_service_dispose (GObject *object)
{
  CPUFreqSelectorService *self;

  self = CPUFREQ_SELECTOR_SERVICE (object);

  if (self->bus_name_id != 0)
    {
      g_bus_unown_name (self->bus_name_id);
      self->bus_name_id = 0;
    }

  if (self->selector)
    {
      GDBusInterfaceSkeleton *skeleton;

      skeleton = G_DBUS_INTERFACE_SKELETON (self->selector);

      g_dbus_interface_skeleton_unexport (skeleton);
      g_clear_object (&self->selector);
    }

  if (self->quit_timeout_id != 0)
    {
      g_source_remove (self->quit_timeout_id);
      self->quit_timeout_id = 0;
    }

  g_clear_object (&self->authority);

  g_clear_pointer (&self->selectors, g_hash_table_destroy);

  G_OBJECT_CLASS (cpufreq_selector_service_parent_class)->dispose (object);
}

static void
install_signals (void)
{
  signals[SIGNAL_QUIT] =
    g_signal_new ("quit",
                  CPUFREQ_TYPE_SELECTOR_SERVICE,
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL,
                  NULL,
                  NULL,
                  G_TYPE_NONE,
                  0);
}

static void
cpufreq_selector_service_class_init (CPUFreqSelectorServiceClass *self_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (self_class);

  object_class->dispose = cpufreq_selector_service_dispose;

  install_signals ();
}

static void
cpufreq_selector_service_init (CPUFreqSelectorService *self)
{
  GError *error;

  error = NULL;
  self->authority = polkit_authority_get_sync (NULL, &error);

  if (error != NULL)
    {
      g_printerr ("%s\n", error->message);
      g_error_free (error);
      return;
    }

  self->selector = cpufreq_selector_gen_skeleton_new ();
  self->bus_name_id = g_bus_own_name (G_BUS_TYPE_SYSTEM,
                                      "org.gnome.CPUFreqSelector",
                                      G_BUS_NAME_OWNER_FLAGS_NONE,
                                      bus_acquired_cb,
                                      name_acquired_cb,
                                      name_lost_cb,
                                      self,
                                      NULL);

  self->selectors = g_hash_table_new_full (NULL, NULL, NULL, g_object_unref);

  reset_quit_timeout (self);
}

CPUFreqSelectorService *
cpufreq_selector_service_new (void)
{
  return g_object_new (CPUFREQ_TYPE_SELECTOR_SERVICE, NULL);
}
