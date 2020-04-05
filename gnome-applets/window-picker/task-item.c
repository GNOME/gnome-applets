/*
 * Copyright (C) 2008 Canonical Ltd
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Jason Smith <jassmith@gmail.com>
 *              Sebastian Geiger <sbastig@gmx.net>
 */

#include "config.h"
#include "task-item.h"

#include <math.h>
#include <glib/gi18n-lib.h>
#include <cairo/cairo.h>

#include "task-list.h"

struct _TaskItem {
  GtkEventBox  parent;
  WnckWindow  *window;
  WnckScreen  *screen;
  GdkPixbuf   *pixbuf;
  GdkRectangle area;
  gint64       urgent_time;
  guint        blink_timer;
  gboolean     mouse_over;
  GdkMonitor  *monitor;
  TaskList    *list;
  WpApplet    *windowPickerApplet;
};

G_DEFINE_TYPE (TaskItem, task_item, GTK_TYPE_EVENT_BOX)

#define DEFAULT_TASK_ITEM_HEIGHT 26
//make the TaskItem two pixles wider to allow for space of the border
#define DEFAULT_TASK_ITEM_WIDTH 28 + 2

enum {
    TASK_ITEM_MONITOR_CHANGED,
    LAST_SIGNAL
};

static guint task_item_signals[LAST_SIGNAL] = { 0 };


/* D&D stuff */

enum {
  TARGET_STRING_DRAGGED,
  TARGET_TASK_ITEM_DRAGGED /* if this item is dragged */
};

static const GtkTargetEntry drop_types[] = {
    { (gchar *) "STRING", 0, TARGET_STRING_DRAGGED },
    { (gchar *) "text/plain", 0, TARGET_STRING_DRAGGED },
    { (gchar *) "text/uri-list", 0, TARGET_STRING_DRAGGED },
    { (gchar *) "task-item", GTK_TARGET_OTHER_WIDGET | GTK_TARGET_SAME_APP, TARGET_TASK_ITEM_DRAGGED } // drag and drop target
};

static const gint n_drop_types = G_N_ELEMENTS(drop_types);

static const GtkTargetEntry drag_types[] = {
    { (gchar *) "task-item", GTK_TARGET_OTHER_WIDGET | GTK_TARGET_SAME_APP, TARGET_TASK_ITEM_DRAGGED } // drag and drop source
};

static const gint n_drag_types = G_N_ELEMENTS(drag_types);

static void
update_expand (TaskItem       *self,
               GtkOrientation  orientation)
{
  GtkWidget *widget;

  widget = GTK_WIDGET (self);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      gtk_widget_set_hexpand (widget, FALSE);
      gtk_widget_set_vexpand (widget, TRUE);
    }
  else
    {
      gtk_widget_set_hexpand (widget, TRUE);
      gtk_widget_set_vexpand (widget, FALSE);
    }
}

static void
placement_changed_cb (GpApplet        *applet,
                      GtkOrientation   orientation,
                      GtkPositionType  position,
                      TaskItem        *self)
{
  update_expand (self, orientation);
}

static void
update_hints (TaskItem *item)
{
    GtkWidget *toplevel;
    GtkAllocation item_allocation;
    WnckWindow *window;
    gint x, y, toplevel_x, toplevel_y;

    window = item->window;

    /* Skip problems */
    if (!WNCK_IS_WINDOW (window)) return;

    /* Skip invisible windows */
    if (!gtk_widget_get_visible (GTK_WIDGET (item))) return;

    toplevel = gtk_widget_get_toplevel (GTK_WIDGET (item));

    if (!gtk_widget_translate_coordinates (GTK_WIDGET (item), toplevel, 0, 0, &x, &y))
        return;

    if (!gtk_widget_get_window (toplevel))
        return;

    gdk_window_get_origin (gtk_widget_get_window (toplevel), &toplevel_x, &toplevel_y);

    /* Set the minimize hint for the window */
    gtk_widget_get_allocation (GTK_WIDGET (item), &item_allocation);

    wnck_window_set_icon_geometry (window,
                                   toplevel_x + x, toplevel_y + y,
                                   item_allocation.width,
                                   item_allocation.height);
}

static gboolean on_task_item_button_released (
    GtkWidget      *widget,
    GdkEventButton *event,
    TaskItem       *item)
{
    WnckWindow *window;
    WnckScreen *screen;
    WnckWorkspace *workspace;
    g_return_val_if_fail (TASK_IS_ITEM(item), TRUE);
    window = item->window;
    g_return_val_if_fail (WNCK_IS_WINDOW (window), TRUE);
    screen = item->screen;
    workspace = wnck_window_get_workspace (window);
    /* If we are in a drag and drop action, then we are not activating
     * the window which received a click
     */
    if(GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "drag-true"))) {
        return TRUE;
    }
    if (event->button == 1) {
        if (WNCK_IS_WORKSPACE (workspace)
            && workspace != wnck_screen_get_active_workspace (screen))
        {
            wnck_workspace_activate (workspace, gtk_get_current_event_time ());
        }
        if (wnck_window_is_active (window)) {
            wnck_window_minimize (window);
        } else {
            wnck_window_activate_transient (window, gtk_get_current_event_time ());
        }
    }
    return TRUE;
}

static void
task_item_set_visibility (TaskItem *item)
{
    WnckScreen *screen;
    WnckWindow *window;
    WnckWorkspace *workspace;
    gboolean show_all;
    gboolean show_window;

    g_return_if_fail (TASK_IS_ITEM (item));
    if (!WNCK_IS_WINDOW (item->window)) {
        gtk_widget_hide (GTK_WIDGET (item));
        return;
    }
    window = item->window;
    screen = item->screen;
    workspace = wnck_screen_get_active_workspace (screen);
    show_all = wp_applet_get_show_all_windows (item->windowPickerApplet);
    show_window = FALSE;

    if (!wnck_window_is_skip_tasklist (window)) {
        if(workspace != NULL) { //this can happen sometimes
            if (wnck_workspace_is_virtual (workspace)) {
                show_window = wnck_window_is_in_viewport (window, workspace);
            } else {
                show_window = wnck_window_is_on_workspace (window, workspace);
            }
        }
        show_window = show_window || show_all;
    }
    if (show_window) {
        gtk_widget_show (GTK_WIDGET (item));
    } else {
        gtk_widget_hide (GTK_WIDGET (item));
    }
}

static GtkSizeRequestMode
task_item_get_request_mode (GtkWidget *widget)
{
  TaskItem *self;
  GtkOrientation orientation;

  self = TASK_ITEM (widget);
  orientation = gp_applet_get_orientation (GP_APPLET (self->windowPickerApplet));

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    return GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT;
  else
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
task_item_get_preferred_width (GtkWidget *widget,
                               gint      *minimal_width,
                               gint      *natural_width)
{
  *minimal_width = *natural_width = DEFAULT_TASK_ITEM_WIDTH;
}

static void
task_item_get_preferred_width_for_height (GtkWidget *widget,
                                          gint       height,
                                          gint      *minimal_width,
                                          gint      *natural_width)
{
  *minimal_width = *natural_width = height + 6;
}

static void
task_item_get_preferred_height (GtkWidget *widget,
                                gint      *minimal_height,
                                gint      *natural_height)
{
  *minimal_height = *natural_height = DEFAULT_TASK_ITEM_HEIGHT;
}

static void
task_item_get_preferred_height_for_width (GtkWidget *widget,
                                          gint       width,
                                          gint      *minimal_height,
                                          gint      *natural_height)
{
  *minimal_height = *natural_height = width + 6;
}

static GdkPixbuf *
task_item_sized_pixbuf_for_window (TaskItem   *item,
                                   WnckWindow *window,
                                   gint        size)
{
    GdkPixbuf *pixbuf;
    gint width, height;

    pixbuf = NULL;
    g_return_val_if_fail (WNCK_IS_WINDOW (window), NULL);

    if (wnck_window_has_icon_name (window))
      {
        const gchar *icon_name;
        GtkIconTheme *icon_theme;

        icon_name = wnck_window_get_icon_name (window);
        icon_theme = gtk_icon_theme_get_default ();

        if (gtk_icon_theme_has_icon (icon_theme, icon_name))
          {
            GdkPixbuf *internal;

            internal = gtk_icon_theme_load_icon (icon_theme, icon_name, size,
                                                 GTK_ICON_LOOKUP_FORCE_SIZE,
                                                 NULL);
            pixbuf = gdk_pixbuf_copy (internal);
            g_object_unref (internal);
          }
      }

    if (!pixbuf)
      {
        pixbuf = gdk_pixbuf_copy (wnck_window_get_icon (item->window));
      }

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);

    if (MAX (width, height) != size)
      {
        GdkPixbuf *unscaled_pixbuf;
        gdouble scale;

        scale = (gdouble) size / (gdouble) MAX (width, height);
        unscaled_pixbuf = pixbuf;
        pixbuf = gdk_pixbuf_scale_simple (unscaled_pixbuf, (gint) (width * scale), (gint) (height * scale), GDK_INTERP_HYPER);
        g_object_unref (unscaled_pixbuf);
      }

    return pixbuf;
}

static double
get_alpha_ratio (TaskItem *item)
{
    gint64 current_time;
    gdouble delta_ms;

    current_time = g_get_monotonic_time ();

    delta_ms = (current_time - item->urgent_time) / 1000.0;

    return 0.66 + (cos (3.15 * delta_ms / 600) / 3);
}

/* Callback to draw the icon, this function is responsible to draw the different states of the icon
 * for example it will draw the rectange around an active icon, the white circle on hover, etc.
 */
static gboolean task_item_draw (
    GtkWidget      *widget,
    cairo_t *cr,
    WpApplet* windowPickerApplet)
{
    TaskItem *item;
    GdkRectangle area;
    GdkPixbuf *pbuf;
    gint size;
    gboolean active;
    gboolean icons_greyscale;
    gboolean attention;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (TASK_IS_ITEM (widget), FALSE);
    item = TASK_ITEM (widget);
    g_return_val_if_fail (WNCK_IS_WINDOW (item->window), FALSE);

    area = item->area;
    pbuf = item->pixbuf;
    size = MIN (area.height, area.width) - 8;
    active = wnck_window_is_active (item->window);
    /* load the GSettings key for gray icons */
    icons_greyscale = wp_applet_get_icons_greyscale (item->windowPickerApplet);
    attention = wnck_window_or_transient_needs_attention (item->window);

    if (GDK_IS_PIXBUF (pbuf) &&
        gdk_pixbuf_get_width (pbuf) != size &&
        gdk_pixbuf_get_height (pbuf) != size)
    {
        g_object_unref (pbuf);
        pbuf = NULL;
    }

    if (active) { /* paint frame around the icon */
        /* We add -1 for x to make it bigger to the left
         * and +1 for width to make it bigger at the right */
        cairo_rectangle (cr, 1, 1, area.width - 2, area.height - 2);
        cairo_set_source_rgba (cr, .8, .8, .8, .2);
        cairo_fill_preserve (cr);
        if(item->mouse_over) {
            cairo_set_source_rgba (cr, .9, .9, 1, 0.45);
            cairo_stroke (cr);
        } else {
            cairo_set_line_width (cr, 1);
            cairo_set_source_rgba (cr, .8, .8, .8, .4);
            cairo_stroke (cr);
        }
    } else if(item->mouse_over) {
        int glow_x, glow_y;
        cairo_pattern_t *glow_pattern;
        glow_x = area.width / 2;
        glow_y = area.height / 2;
        glow_pattern = cairo_pattern_create_radial (
            glow_x, glow_y, glow_x * 0.3,
            glow_x, glow_y, glow_x * 1.4
        );
        cairo_pattern_add_color_stop_rgba (glow_pattern, 0, 1, 1, 1, 1);
        cairo_pattern_add_color_stop_rgba (glow_pattern, 0.6, 1, 1, 1, 0);
        cairo_set_source (cr, glow_pattern);
        cairo_paint (cr);
    }

    if (!pbuf) {
        pbuf = item->pixbuf = task_item_sized_pixbuf_for_window (item, item->window, size);
    }

    if (active || item->mouse_over || attention || !icons_greyscale) {
        gdk_cairo_set_source_pixbuf (
            cr,
            pbuf,
            ((area.width - gdk_pixbuf_get_width (pbuf)) / 2.0),
            ((area.height - gdk_pixbuf_get_height (pbuf)) / 2.0)
        );
    } else { /* create grayscale pixbuf */
        GdkPixbuf *desat = gdk_pixbuf_new (
            GDK_COLORSPACE_RGB,
            TRUE,
            gdk_pixbuf_get_bits_per_sample (pbuf),
            gdk_pixbuf_get_width (pbuf),
            gdk_pixbuf_get_height (pbuf)
        );
        if (desat) {
            gdk_pixbuf_saturate_and_pixelate (
                pbuf,
                desat,
                0, //means zero saturation == gray
                FALSE
            );
        } else { /* just paint the colored version as a fallback */
            desat = g_object_ref (pbuf);
        }
        gdk_cairo_set_source_pixbuf (
            cr,
            desat,
            ((area.width - gdk_pixbuf_get_width (desat)) / 2.0),
            ((area.height - gdk_pixbuf_get_height (desat)) / 2.0));
        g_object_unref (desat);
    }

    if (!item->mouse_over && attention) { /* urgent */
        gdouble alpha;

        alpha = get_alpha_ratio (item);

        cairo_paint_with_alpha (cr, alpha);
    } else if (item->mouse_over || active || !icons_greyscale) { /* focused */
        cairo_paint (cr);
    } else { /* not focused */
        cairo_paint_with_alpha (cr, .65);
    }

    return FALSE;
}

static void on_size_allocate (
    GtkWidget     *widget,
    GtkAllocation *allocation,
    TaskItem      *item)
{
    g_return_if_fail (TASK_IS_ITEM (item));

    item->area.x = allocation->x;
    item->area.y = allocation->y;
    item->area.width = allocation->width;
    item->area.height = allocation->height;
    update_hints (item);
}

static gboolean on_button_pressed (
    GtkWidget      *button,
    GdkEventButton *event,
    TaskItem       *item)
{
    WnckWindow *window;
    g_return_val_if_fail (TASK_IS_ITEM (item), FALSE);
    window = item->window;
    g_return_val_if_fail (WNCK_IS_WINDOW (window), FALSE);

    if (event->button == 3) {
        GtkWidget *menu = wnck_action_menu_new (window);

        gp_applet_popup_menu_at_widget (GP_APPLET (item->windowPickerApplet),
                                        GTK_MENU (menu), GTK_WIDGET (item),
                                        (GdkEvent *) event);

        return TRUE;
    }
    return FALSE;
}

static gboolean on_query_tooltip (
    GtkWidget *widget,
    gint x, gint y,
    gboolean keyboard_mode,
    GtkTooltip *tooltip,
    TaskItem *item)
{
    WnckWindow *window = item->window;
    g_return_val_if_fail (WNCK_IS_WINDOW (window), FALSE);
    gtk_tooltip_set_text (tooltip, wnck_window_get_name(window));
    gtk_tooltip_set_icon (tooltip, wnck_window_get_icon (window));
    return TRUE;
}

static gboolean on_enter_notify (
    GtkWidget *widget,
    GdkEventCrossing *event,
    TaskItem *item)
{
    g_return_val_if_fail (TASK_IS_ITEM (item), FALSE);
    item->mouse_over = TRUE;
    gtk_widget_queue_draw (widget);
    return FALSE;
}

static gboolean on_leave_notify (
    GtkWidget *widget,
    GdkEventCrossing *event,
    TaskItem *item)
{
    g_return_val_if_fail (TASK_IS_ITEM (item), FALSE);
    item->mouse_over = FALSE;
    gtk_widget_queue_draw (widget);
    return FALSE;
}

static gboolean on_blink (TaskItem *item) {
    g_return_val_if_fail (TASK_IS_ITEM (item), FALSE);
    gtk_widget_queue_draw (GTK_WIDGET (item));
    if (wnck_window_or_transient_needs_attention (item->window)) {
        return TRUE;
    } else {
        item->blink_timer = 0;
        return FALSE;
    }
}

static void on_window_state_changed (
    WnckWindow      *window,
    WnckWindowState  changed_mask,
    WnckWindowState  new_state,
    TaskItem *taskItem)
{
    g_return_if_fail (WNCK_IS_WINDOW (window));
    g_return_if_fail (TASK_IS_ITEM (taskItem));
    if (new_state & WNCK_WINDOW_STATE_URGENT && !taskItem->blink_timer) {
        taskItem->blink_timer = g_timeout_add (30, (GSourceFunc) on_blink, taskItem);
        taskItem->urgent_time = g_get_monotonic_time ();
    }
    task_item_set_visibility (taskItem);
}

static void on_window_workspace_changed (
    WnckWindow *window, TaskItem *taskItem)
{
    g_return_if_fail (TASK_IS_ITEM (taskItem));
    task_item_set_visibility (taskItem);
}

static void on_window_icon_changed (WnckWindow *window, TaskItem *item) {
    g_return_if_fail (TASK_IS_ITEM (item));
    g_clear_object (&item->pixbuf);
    gtk_widget_queue_draw (GTK_WIDGET (item));
}

static GdkMonitor *
get_window_monitor (WnckWindow *window)
{
    gint x;
    gint y;
    gint w;
    gint h;
    GdkMonitor *window_monitor;
    GdkDisplay *gdk_display;

    wnck_window_get_geometry (window, &x, &y, &w, &h);

    gdk_display = gdk_display_get_default ();
    window_monitor = gdk_display_get_monitor_at_point (gdk_display,
                                                       x + w / 2,
                                                       y + h / 2);

    return window_monitor;
}

static void
set_monitor (TaskItem   *item,
             GdkMonitor *monitor)
{
  if (item->monitor)
    g_object_remove_weak_pointer (G_OBJECT (item->monitor),
                                  (gpointer *) &item->monitor);

  item->monitor = monitor;

  if (item->monitor)
    g_object_add_weak_pointer (G_OBJECT (item->monitor),
                               (gpointer *) &item->monitor);
}

static void
on_window_geometry_changed (WnckWindow *window,
                            TaskItem   *item)
{
    GdkMonitor *old_monitor;
    GdkMonitor *window_monitor;

    window_monitor = get_window_monitor (window);

    old_monitor = item->monitor;

    if (old_monitor != window_monitor)
      {
        set_monitor (item, window_monitor);

        g_signal_emit (item, task_item_signals[TASK_ITEM_MONITOR_CHANGED], 0);
      }
}

static void on_screen_active_window_changed (
    WnckScreen    *screen,
    WnckWindow    *old_window,
    TaskItem      *item)
{
    WnckWindow *window;
    window = item->window;

    if ((WNCK_IS_WINDOW (old_window) && window == old_window) ||
        window == wnck_screen_get_active_window (screen))
    {
        /* queue a draw to reflect that we are [no longer] the active window */
        gtk_widget_queue_draw (GTK_WIDGET (item));
    }
}

static void on_screen_active_workspace_changed (
    WnckScreen    *screen,
    WnckWorkspace *old_workspace,
    TaskItem      *taskItem)
{
    g_return_if_fail (TASK_IS_ITEM (taskItem));
    task_item_set_visibility (taskItem);
}

static void on_screen_active_viewport_changed (
    WnckScreen  *screen,
    TaskItem    *item)
{
    g_return_if_fail (item != NULL);
    g_return_if_fail (TASK_IS_ITEM(item));
    task_item_set_visibility (item);
}

static gboolean activate_window (GtkWidget *widget) {
    TaskItem *item;
    GtkWidget *parent;
    WnckWindow *window;
    guint time;

    g_return_val_if_fail (TASK_IS_ITEM(widget), FALSE);
    item = TASK_ITEM (widget);
    parent = gtk_widget_get_parent (widget);

    time = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (parent), "event-time"));

    window = item->window;
    if (WNCK_IS_WINDOW (window))
        wnck_window_activate (window, time);

    g_object_set_data (G_OBJECT (parent), "event-source", GINT_TO_POINTER (0));

    return FALSE;
}

/* Emitted when a drag leaves the destination */
static void on_drag_leave (
    GtkWidget *widget,
    GdkDragContext *context,
    guint time,
    gpointer user_data)
{
  GtkWidget *parent;

  parent = gtk_widget_get_parent (widget);
  g_object_set_data (G_OBJECT (parent), "active-widget", NULL);

  g_object_set_data (G_OBJECT (widget), "drag-true", GINT_TO_POINTER (0));
}

static gboolean
on_drag_drop (GtkWidget      *widget,
              GdkDragContext *context,
              gint            x,
              gint            y,
              guint           time,
              gpointer        user_data)
{
  return FALSE;
}

static gboolean
find_drop_type_by_name (char *source_name)
{
  for (gulong i = 0; i < G_N_ELEMENTS (drop_types); i++)
    {
      gchar * target_name;

      target_name = drop_types[i].target;

      if (g_strcmp0 (source_name, target_name) == 0)
        return TRUE;
    }

  return FALSE;
}

/* Emitted when a drag is over the destination */
static gboolean
on_drag_motion (GtkWidget      *item,
                GdkDragContext *context,
                gint            x,
                gint            y,
                guint           time)
{
    GdkAtom target;
    GList *targets;
    GList *current;
    GtkWidget *parent;
    GtkWidget *active_widget;
    guint event_source;

    parent = gtk_widget_get_parent (item);
    active_widget = g_object_get_data (G_OBJECT (parent), "active-widget");

    if (active_widget == item)
      return FALSE;

    event_source = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (parent), "event-source"));

    if (event_source > 0)
      {
        g_source_remove (event_source);
        g_object_set_data (G_OBJECT (parent), "event-source", GINT_TO_POINTER (0));
      }

    g_object_set_data (G_OBJECT (parent), "active-widget", item);

    target = NULL;
    targets = gdk_drag_context_list_targets (context);

    if (targets == NULL)
      return FALSE;

    current = targets;

    while (current != NULL)
      {
        char *name;
        gboolean found;

        /* Choose the best target type */
        target = GDK_POINTER_TO_ATOM (current->data);
        name = gdk_atom_name (target);

        found = find_drop_type_by_name (name);
        g_free (name);

        if (found)
          break;

        current = g_list_next (current);
      }

    g_assert (target != NULL);

    gtk_drag_get_data (item,    // will receive 'drag-data-received' signal
                       context, // represents the current state of the DnD
                       target,  // the target type we want
                       time);   // time stamp

    return TRUE;
}


/* Drag and drop code */

/**
 * When the drag begin we first set the right icon to appear next to the cursor
 */
static void on_drag_begin(GtkWidget *widget, GdkDragContext *context, gpointer user_data) {
    TaskItem *item = TASK_ITEM (widget);
    GdkRectangle area = item->area;
    gint size = MIN (area.height, area.width);
    GdkPixbuf *pixbuf = task_item_sized_pixbuf_for_window (item, item->window, size);
    gtk_drag_source_set_icon_pixbuf(widget, pixbuf);
    g_object_set_data (G_OBJECT (item), "drag-true", GINT_TO_POINTER (1));
}

static void
on_drag_get_data (GtkWidget        *widget,
                  GdkDragContext   *context,
                  GtkSelectionData *selection_data,
                  guint             target_type,
                  guint             time,
                  gpointer          user_data)
{
    TaskItem *item;

    g_assert (user_data != NULL && TASK_IS_ITEM (user_data));

    item = TASK_ITEM (user_data);

    if (target_type != TARGET_TASK_ITEM_DRAGGED)
      g_assert_not_reached ();

    gtk_selection_data_set (selection_data,
                            gtk_selection_data_get_target (selection_data),
                            8,
                            (guchar*) &item,
                            sizeof (TaskItem*));
}

static void
on_drag_end (GtkWidget      *drag_source,
             GdkDragContext *drag_context,
             gpointer        user_data)
{
  GtkWidget *parent;
  guint event_source;

  parent = gtk_widget_get_parent (drag_source);

  g_object_set_data (G_OBJECT (parent), "active-widget", NULL);
  event_source = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (parent), "event-source"));

  if (event_source != 0)
  {
    g_source_remove (event_source);
    g_object_set_data (G_OBJECT (parent), "event-source", GINT_TO_POINTER (0));
  }

  g_object_set_data (G_OBJECT (drag_source), "drag-true", GINT_TO_POINTER (0));
}

static gint
grid_get_pos (GtkWidget *grid,
              GtkWidget *item)
{
  GtkContainer *container;
  GList *items;

  container = GTK_CONTAINER (grid);
  items = gtk_container_get_children (container);

    while (items)
      {
        if (items->data == item)
          {
            gint pos;
            gtk_container_child_get (container, item, "position", &pos, NULL);
            return pos;
          }

        items = items->next;
      }

    return -1;
}

static void on_drag_received_data (
    GtkWidget *drag_target, //target of the d&d action
    GdkDragContext *context,
    gint x,
    gint y,
    GtkSelectionData *selection_data,
    guint target_type,
    guint time,
    TaskItem *item)
{
  if (selection_data == NULL || gtk_selection_data_get_length (selection_data) < 0)
    {
      gdk_drag_status (context, 0, time);
      return;
    }

  if (target_type == TARGET_TASK_ITEM_DRAGGED)
    {
      GtkWidget *drag_source;
      GtkWidget *taskList;
      GtkWidget *parent;
      gint target_position;
      gpointer *data;

      parent = gtk_widget_get_parent (drag_target);
      taskList = wp_applet_get_tasks (item->windowPickerApplet);
      data = (gpointer *) gtk_selection_data_get_data (selection_data);

      drag_source = GTK_WIDGET (*data);
      g_assert (TASK_IS_ITEM (drag_source));

      if (drag_source == drag_target)
        {
          gdk_drag_status (context, 0, time);
          return; //source and target are identical
        }

      target_position = grid_get_pos (taskList, drag_target);

      g_object_ref (drag_source);
      gtk_box_reorder_child (GTK_BOX (taskList), drag_source, target_position);
      g_object_unref (drag_source);

      // we swapped the source and the target, so the drag_target is no longer part of the drag action
      g_object_set_data (G_OBJECT (drag_target), "drag-true",
                         GINT_TO_POINTER (0));

      // we swapped the source and the target, so the drag_source is the new active widget
      g_object_set_data (G_OBJECT (parent), "active-widget", drag_source);
    }
  else
    {
      GtkWidget *parent;
      guint event_source;

      parent = gtk_widget_get_parent (drag_target);
      event_source = g_timeout_add (1000,
                                    (GSourceFunc) activate_window,
                                    drag_target);

      g_object_set_data (G_OBJECT (parent), "event-source",
                         GINT_TO_POINTER (event_source));

      g_object_set_data (G_OBJECT (parent), "event-time",
                         GINT_TO_POINTER (time));
    }

  gdk_drag_status (context, GDK_ACTION_COPY, time);
}

/* Returning true here, causes the failed-animation not to be shown. Without this the icon of the dnd operation
 * will jump back to where the dnd operation started.
 **/
static gboolean
on_drag_failed (GtkWidget      *drag_source,
                GdkDragContext *context,
                GtkDragResult   result,
                TaskItem       *taskItem)
{
    return TRUE;
}

static void task_item_setup_atk (TaskItem *item) {
    GtkWidget *widget;
    AtkObject *atk;
    WnckWindow *window;
    g_return_if_fail (TASK_IS_ITEM (item));
    widget = GTK_WIDGET (item);
    window = item->window;
    g_return_if_fail (WNCK_IS_WINDOW (window));
    atk = gtk_widget_get_accessible (widget);
    atk_object_set_name (atk, _("Window Task Button"));
    atk_object_set_description (atk, wnck_window_get_name (window));
    atk_object_set_role (atk, ATK_ROLE_PUSH_BUTTON);
}

static void task_item_finalize (GObject *object) {
    TaskItem *item;

    item = TASK_ITEM (object);

    if (item->blink_timer) {
        g_source_remove (item->blink_timer);
    }

    g_clear_object (&item->pixbuf);
    g_clear_object (&item->window);
    g_clear_object (&item->list);

    G_OBJECT_CLASS (task_item_parent_class)->finalize (object);
}

static void task_item_class_init (TaskItemClass *klass) {
    GObjectClass *obj_class      = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    obj_class->finalize = task_item_finalize;

    widget_class->get_request_mode = task_item_get_request_mode;
    widget_class->get_preferred_width = task_item_get_preferred_width;
    widget_class->get_preferred_width_for_height = task_item_get_preferred_width_for_height;
    widget_class->get_preferred_height = task_item_get_preferred_height;
    widget_class->get_preferred_height_for_width = task_item_get_preferred_height_for_width;

    task_item_signals [TASK_ITEM_MONITOR_CHANGED] =
        g_signal_new ("monitor-changed", TASK_TYPE_ITEM, G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void task_item_init (TaskItem *item) {
    item->blink_timer = 0;
}

GtkWidget *
task_item_new (WpApplet   *windowPickerApplet,
               WnckWindow *window,
               TaskList   *list)
{
    TaskItem *taskItem;
    WnckScreen *screen;
    GtkWidget *item;

    g_return_val_if_fail (WNCK_IS_WINDOW (window), NULL);

    item = g_object_new (
        TASK_TYPE_ITEM,
        "has-tooltip", TRUE,
        "visible-window", FALSE,
        "above-child", TRUE,
        NULL
    );
    gtk_widget_add_events (item, GDK_ALL_EVENTS_MASK);
    gtk_container_set_border_width (GTK_CONTAINER (item), 0);
    taskItem = TASK_ITEM (item);

    taskItem->window = g_object_ref (window);

    screen = wnck_window_get_screen (window);
    taskItem->screen = screen;
    taskItem->windowPickerApplet = windowPickerApplet;

    set_monitor (taskItem, get_window_monitor (window));

    task_item_set_task_list (TASK_ITEM (item), list);

    g_signal_connect_object (windowPickerApplet,
                             "placement-changed",
                             G_CALLBACK (placement_changed_cb),
                             taskItem,
                             0);

    update_expand (taskItem,
                   gp_applet_get_orientation (GP_APPLET (windowPickerApplet)));

    /** Drag and Drop code
     * This item can be both the target and the source of a drag and drop
     * operation. As a target it can receive strings (e.g. links).
     * As a source the dragged icon can be dragged to another position on the
     * task list.
     */
    //target (destination)
    gtk_drag_dest_set (
        item,
        GTK_DEST_DEFAULT_MOTION,
        drop_types, n_drop_types,
        GDK_ACTION_COPY
    );
    gtk_drag_dest_add_uri_targets (item);
    gtk_drag_dest_add_text_targets (item);

    //source
    gtk_drag_source_set (
        item,
        GDK_BUTTON1_MASK,
        drag_types,
        n_drag_types,
        GDK_ACTION_COPY
    );

    /* Drag and drop (target signals)*/
    g_signal_connect (item, "drag-motion",
        G_CALLBACK (on_drag_motion), item);
    g_signal_connect (item, "drag-leave",
        G_CALLBACK (on_drag_leave), item);
    g_signal_connect (item, "drag-drop",
        G_CALLBACK (on_drag_drop), item);
    g_signal_connect (item, "drag_data_received",
        G_CALLBACK(on_drag_received_data), item);
    /* a 'drag-drop' signal is not needed, instead we rely on the drag-failed signal to end a drag operation. */
    g_signal_connect (item, "drag-end",
        G_CALLBACK (on_drag_end), NULL);
    g_signal_connect (item, "drag-failed",
            G_CALLBACK(on_drag_failed), item);
    /* Drag and drop (source signals) */
    g_signal_connect (item, "drag-begin",
        G_CALLBACK (on_drag_begin), item);
    g_signal_connect (item, "drag_data_get",
        G_CALLBACK (on_drag_get_data), item);

    /* Other signals */
    g_signal_connect_object (screen, "viewports-changed",
        G_CALLBACK (on_screen_active_viewport_changed), item, 0);
    g_signal_connect_object (screen, "active-window-changed",
        G_CALLBACK (on_screen_active_window_changed), item, 0);
    g_signal_connect_object (screen, "active-workspace-changed",
        G_CALLBACK (on_screen_active_workspace_changed), item, 0);
    g_signal_connect_object (window, "workspace-changed",
        G_CALLBACK (on_window_workspace_changed), item, 0);
    g_signal_connect_object (window, "state-changed",
        G_CALLBACK (on_window_state_changed), item, 0);
    g_signal_connect_object (window, "icon-changed",
        G_CALLBACK (on_window_icon_changed), item, 0);
    g_signal_connect_object (window, "geometry-changed",
        G_CALLBACK (on_window_geometry_changed), item, 0);

    g_signal_connect(item, "draw",
        G_CALLBACK(task_item_draw), windowPickerApplet);
    g_signal_connect (item, "button-release-event",
        G_CALLBACK (on_task_item_button_released), item);
    g_signal_connect (item, "button-press-event",
        G_CALLBACK (on_button_pressed), item);
    g_signal_connect (item, "size-allocate",
        G_CALLBACK (on_size_allocate), item);
    g_signal_connect (item, "query-tooltip",
        G_CALLBACK (on_query_tooltip), item);
    g_signal_connect (item, "enter-notify-event",
        G_CALLBACK (on_enter_notify), item);
    g_signal_connect (item, "leave-notify-event",
        G_CALLBACK (on_leave_notify), item);
    task_item_set_visibility (taskItem);
    task_item_setup_atk (taskItem);
    return item;
}

GdkMonitor *
task_item_get_monitor (TaskItem *item)
{
    return item->monitor;
}

TaskList *
task_item_get_task_list (TaskItem *item)
{
  return item->list;
}

void
task_item_set_task_list (TaskItem *item,
                         TaskList *list)
{
  if (item->list)
    g_object_unref (item->list);

  item->list = g_object_ref (list);
}

WnckWindow *
task_item_get_window (TaskItem *item)
{
  return item->window;
}
