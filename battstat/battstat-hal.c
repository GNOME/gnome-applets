/*
 * Copyright (C) 2005 by Ryan Lortie <desrt@desrt.ca>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 * $Id$
 */

#include <config.h>

#ifdef HAVE_HAL

#include <dbus/dbus-glib-lowlevel.h>
#include <libhal.h>
#include <string.h>
#include <math.h>

#include "battstat-hal.h"

static LibHalContext *battstat_hal_ctx;
static GSList *batteries;
static GSList *adaptors;

struct battery_status
{
  int current_level, design_level, full_level;
  int time_remaining;
  int charging, discharging;
};

struct battery_info
{
  char *udi; /* must be first member */
  struct battery_status status;
};

struct adaptor_info
{
  char *udi; /* must be first member */
  int present;
};

static void
battery_update_property( LibHalContext *ctx, struct battery_info *battery,
                         const char *key )
{
  DBusError error;

  dbus_error_init( &error );

  if( !strcmp( key, "battery.charge_level.current" ) )
    battery->status.current_level =
      libhal_device_get_property_int( ctx, battery->udi, key, &error );

  else if( !strcmp( key, "battery.charge_level.design" ) )
    battery->status.design_level =
      libhal_device_get_property_int( ctx, battery->udi, key, &error );

  else if( !strcmp( key, "battery.charge_level.last_full" ) )
    battery->status.full_level =
      libhal_device_get_property_int( ctx, battery->udi, key, &error );

  else if( !strcmp( key, "battery.remaining_time" ) )
    battery->status.time_remaining =
      libhal_device_get_property_int( ctx, battery->udi, key, &error );

  else if( !strcmp( key, "battery.rechargeable.is_charging" ) )
    battery->status.charging =
      libhal_device_get_property_bool( ctx, battery->udi, key, &error );

  else if( !strcmp( key, "battery.rechargeable.is_discharging" ) )
    battery->status.discharging =
      libhal_device_get_property_bool( ctx, battery->udi, key, &error );

  dbus_error_free( &error );
}

static void
adaptor_update_property( LibHalContext *ctx, struct adaptor_info *adaptor,
                         const char *key )
{
  DBusError error;

  dbus_error_init( &error );

  if( !strcmp( key, "ac_adaptor.present" ) )
    adaptor->present =
      libhal_device_get_property_bool( ctx, adaptor->udi, key, &error );

  dbus_error_free( &error );
}

static GSList *
find_link_by_udi( GSList *list, const char *udi )
{
  GSList *link;

  for( link = list; link; link = g_slist_next( link ) )
  {
    /* this might be an adaptor!  this is OK as long as 'udi' is the first
     * member of both structures.
     */
    struct battery_info *battery = link->data;

    if( !strcmp( udi, battery->udi ) )
      return link;
  }

  return NULL;
}

static void *
find_device_by_udi( GSList *list, const char *udi )
{
  GSList *link;

  link = find_link_by_udi( list, udi );

  if( link )
    return link->data;
  else
    return NULL;
}

static void
property_callback( LibHalContext *ctx, const char *udi, const char *key,
                   dbus_bool_t is_removed, dbus_bool_t is_added )
{
  struct battery_info *battery;
  struct adaptor_info *adaptor;

  if( (battery = find_device_by_udi( batteries, udi )) )
    battery_update_property( ctx, battery, key );

  if( (adaptor = find_device_by_udi( adaptors, udi )) )
    adaptor_update_property( ctx, adaptor, key );
}

static void
add_to_list( LibHalContext *ctx, GSList **list, const char *udi, int size )
{
  struct battery_info *battery = g_malloc0( size );
  LibHalPropertySetIterator iter;
  LibHalPropertySet *properties;
  DBusError error;

  /* this might be an adaptor!  this is OK as long as 'udi' is the first
   * member of both structures.
   */
  battery->udi = g_strdup( udi );
  *list = g_slist_append( *list, battery );

  dbus_error_init( &error );

  libhal_device_add_property_watch( ctx, udi, &error );

  properties = libhal_device_get_all_properties( ctx, udi, &error );

  for( libhal_psi_init( &iter, properties );
       libhal_psi_has_more( &iter );
       libhal_psi_next( &iter ) )
  {
    const char *key = libhal_psi_get_key( &iter );
    property_callback( ctx, udi, key, FALSE, FALSE );
  }
    
  libhal_free_property_set( properties );
  dbus_error_free( &error );
}

static void
free_list_data( void *data )
{
  /* this might be an adaptor!  this is OK as long as 'udi' is the first
   * member of both structures.
   */
  struct battery_info *battery = data;

  g_free( battery->udi );
  g_free( battery );
}

static GSList *
remove_from_list( LibHalContext *ctx, GSList *list, const char *udi )
{
  GSList *link = find_link_by_udi( list, udi );

  if( link )
  {
    DBusError error;

    dbus_error_init( &error );
    libhal_device_remove_property_watch( ctx, udi, &error );
    dbus_error_free( &error );

    free_list_data( link->data );
    list = g_slist_delete_link( list, link );
  }

  return list;
}

static GSList *
free_entire_list( GSList *list )
{
  GSList *item;
  
  for( item = list; item; item = g_slist_next( item ) )
    free_list_data( item->data );

  g_slist_free( list );

  return NULL;
}

static void
device_added_callback( LibHalContext *ctx, const char *udi )
{
  DBusError error;

  dbus_error_init( &error );

  if( libhal_device_property_exists( ctx, udi, "battery", &error ) )
    add_to_list( ctx, &batteries, udi, sizeof (struct battery_info) );

  if( libhal_device_property_exists( ctx, udi, "ac_adapter", &error ) )
    add_to_list( ctx, &adaptors, udi, sizeof (struct adaptor_info) );
}

static void
device_removed_callback( LibHalContext *ctx, const char *udi )
{
  batteries = remove_from_list( ctx, batteries, udi );
  adaptors = remove_from_list( ctx, adaptors, udi );
}

/* ---- public functions ---- */

char *
battstat_hal_initialise( void )
{
  DBusConnection *connection;
  LibHalContext *ctx;
  DBusError error;
  char *error_str;
  char **devices;
  int i, num;

  if( battstat_hal_ctx != NULL )
    return g_strdup( "Already initialised!" );

  dbus_error_init( &error );

  if( (connection = dbus_bus_get( DBUS_BUS_SYSTEM, &error )) == NULL )
    goto error_out;

  dbus_connection_setup_with_g_main( connection, g_main_context_default() );

  if( (ctx = libhal_ctx_new()) == NULL )
  {
    dbus_set_error( &error, "HAL error", "Could not create libhal_ctx" );
    goto error_out;
  }

  libhal_ctx_set_device_property_modified( ctx, property_callback );
  libhal_ctx_set_device_added( ctx, device_added_callback );
  libhal_ctx_set_device_removed( ctx, device_removed_callback );
  libhal_ctx_set_dbus_connection( ctx, connection );

  if( libhal_ctx_init( ctx, &error ) == 0 )
    goto error_freectx;
  
  devices = libhal_find_device_by_capability( ctx, "battery", &num, &error );

  if( devices == NULL )
    goto error_freectx;

  /* FIXME: for now, if 0 battery devices are present on first scan, then fail.
   * This allows fallover to the legacy (ACPI, APM, etc) backends if the
   * installed version of HAL doesn't know about batteries.  This check should
   * be removed at some point in the future (maybe circa GNOME 2.13..).
   */
  if( num == 0 )
  {
    dbus_free_string_array( devices );
    dbus_set_error( &error, "HAL error", "No batteries found" );
    goto error_freectx;
  }

  for( i = 0; i < num; i++ )
    add_to_list( ctx, &batteries, devices[i], sizeof (struct battery_info) );
  dbus_free_string_array( devices );

  devices = libhal_find_device_by_capability( ctx, "ac_adapter", &num, &error );

  if( devices == NULL )
  {
    batteries = free_entire_list( batteries );
    goto error_freectx;
  }

  for( i = 0; i < num; i++ )
    add_to_list( ctx, &adaptors, devices[i], sizeof (struct adaptor_info) );
  dbus_free_string_array( devices );

  dbus_error_free( &error );

  battstat_hal_ctx = ctx;

  return NULL;

error_freectx:
  libhal_ctx_free( ctx );

error_out:
  error_str = g_strdup_printf( "Unable to initialise HAL: %s: %s",
                               error.name, error.message );
  dbus_error_free( &error );
  return error_str;
}

void
battstat_hal_cleanup( void )
{
  if( battstat_hal_ctx == NULL )
    return;

  libhal_ctx_free( battstat_hal_ctx );
  batteries = free_entire_list( batteries );
  adaptors = free_entire_list( adaptors );
}

#include "battstat.h"

/* transitional function */
void
battstat_hal_get_battery_info( BatteryStatus *status )
{
  struct battery_info *battery;
  double percent;

  if( g_slist_length( batteries ) == 0 )
  {
    status->present = FALSE;
    status->percent = 0;
    status->minutes = -1;
    status->on_ac_power = TRUE;
    status->charging = FALSE;

    return;
  }

  /* just report on the first battery for now */
  battery = batteries->data;

  percent = (double) battery->status.current_level;
  percent /= (double) battery->status.full_level;
  percent *= 100;
  status->percent = (int) floor( percent + 0.5 ); 

  /* for HAL, 0 is 'unknown' */
  if( battery->status.time_remaining != 0 )
    status->minutes = (battery->status.time_remaining + 30) / 60;
  else
    status->minutes = -1;

  status->charging = battery->status.charging;
  status->on_ac_power = !battery->status.discharging;
}

#endif /* HAVE_HAL */
