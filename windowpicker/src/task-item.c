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

#include "task-item.h"
#include "task-list.h"

#include <math.h>
#include <glib/gi18n.h>
#include <cairo/cairo.h>

struct _TaskItem {
  GtkEventBox  parent;
  WnckWindow  *window;
  WnckScreen  *screen;
  GdkPixbuf   *pixbuf;
  GdkRectangle area;
  GTimeVal     urgent_time;
  guint        blink_timer;
  gboolean     mouse_over;
  GdkMonitor  *monitor;
  WpApplet    *windowPickerApplet;
};

G_DEFINE_TYPE (TaskItem, task_item, GTK_TYPE_EVENT_BOX)

#define DEFAULT_TASK_ITEM_HEIGHT 26
//make the TaskItem two pixles wider to allow for space of the border
#define DEFAULT_TASK_ITEM_WIDTH 28 + 2

enum {
    TASK_ITEM_CLOSED_SIGNAL,
    TASK_ITEM_MONITOR_CHANGED,
    LAST_SIGNAL
};

static guint task_item_signals[LAST_SIGNAL] = { 0 };


/* D&D stuff */

enum {
    TARGET_WIDGET_DRAGGED, /* if this item is dragged */
    TARGET_ITEM_DRAGGED
};

static const GtkTargetEntry drop_types[] = {
    { (gchar *) "STRING", 0, 0 },
    { (gchar *) "text/plain", 0, 0},
    { (gchar *) "text/uri-list", 0, 0},
    { (gchar *) "widget", GTK_TARGET_OTHER_WIDGET, TARGET_WIDGET_DRAGGED}, //drag and drop target
    { (gchar *) "item", GTK_TARGET_SAME_WIDGET, TARGET_ITEM_DRAGGED }
};

static const gint n_drop_types = G_N_ELEMENTS(drop_types);

static const GtkTargetEntry drag_types[] = {
    { (gchar *) "widget", GTK_TARGET_OTHER_WIDGET, TARGET_WIDGET_DRAGGED}, //drag and drop source
    { (gchar *) "item", GTK_TARGET_SAME_WIDGET, TARGET_ITEM_DRAGGED }
};

static const gint n_drag_types = G_N_ELEMENTS(drag_types);

static void task_item_close (TaskItem *item);

static void update_hints (TaskItem *item) {
    GtkWidget *parent, *widget;
    GtkAllocation allocation_parent, allocation_widget;
    WnckWindow *window;
    gint x, y, x1, y1;
    widget = GTK_WIDGET (item);
    window = item->window;
    /* Skip problems */
    if (!WNCK_IS_WINDOW (window)) return;
    if (!GTK_IS_WIDGET (widget)) return;
    /* Skip invisible windows */
    if (!gtk_widget_get_visible (widget)) return;
    x = y = 0;
    /* Recursively compute the button's coordinates */
    for (parent = widget; parent; parent = gtk_widget_get_parent(parent)) {
        if (gtk_widget_get_parent(parent)) {
            gtk_widget_get_allocation(parent, &allocation_parent);
            x += allocation_parent.x;
            y += allocation_parent.y;
        } else {
            x1 = y1 = 0;
            if (GDK_IS_WINDOW (gtk_widget_get_window(parent)))
                gdk_window_get_origin (gtk_widget_get_window(parent), &x1, &y1);
            x += x1; y += y1;
            break;
        }
    }
    /* Set the minimize hint for the window */
    gtk_widget_get_allocation(widget, &allocation_widget);
    wnck_window_set_icon_geometry (
        window, x, y,
        allocation_widget.width,
        allocation_widget.height
    );
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
    g_return_if_fail (TASK_IS_ITEM (item));
    if (!WNCK_IS_WINDOW (item->window)) {
        gtk_widget_hide (GTK_WIDGET (item));
        return;
    }
    window = item->window;
    screen = item->screen;
    workspace = wnck_screen_get_active_workspace (screen);
    gboolean show_all = wp_applet_get_show_all_windows (item->windowPickerApplet);
    gboolean show_window = FALSE;
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

static void
task_item_get_preferred_width (GtkWidget *widget,
                               gint      *minimal_width,
                               gint      *natural_width)
{
    GtkRequisition requisition;
    requisition.width = DEFAULT_TASK_ITEM_WIDTH;
    *minimal_width = *natural_width = requisition.width;
}

static void
task_item_get_preferred_height (GtkWidget *widget,
                                gint      *minimal_height,
                                gint      *natural_height)
{
    GtkRequisition requisition;
    requisition.height = DEFAULT_TASK_ITEM_HEIGHT;
    *minimal_height = *natural_height = requisition.height;
}

static GdkPixbuf *
task_item_sized_pixbuf_for_window (TaskItem   *item,
                                   WnckWindow *window,
                                   gint size)
{
    GdkPixbuf *pixbuf;
    gint width, height;

    pixbuf = NULL;
    g_return_val_if_fail (WNCK_IS_WINDOW (window), NULL);
    if (wnck_window_has_icon_name (window)) {
        const gchar *icon_name = wnck_window_get_icon_name (window);
        GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();
        if (gtk_icon_theme_has_icon (icon_theme, icon_name)) {
            GdkPixbuf *internal = gtk_icon_theme_load_icon (icon_theme,
                icon_name,
                size,
                GTK_ICON_LOOKUP_FORCE_SIZE,
                NULL
            );
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

    if (MAX (width, height) != size) {
        gdouble scale = (gdouble) size / (gdouble) MAX (width, height);
        GdkPixbuf *tmp = pixbuf;
        pixbuf = gdk_pixbuf_scale_simple (tmp, (gint) (width * scale), (gint) (height * scale), GDK_INTERP_HYPER);
        g_object_unref (tmp);
    }

    return pixbuf;
}

/* Callback to draw the icon, this function is responsible to draw the different states of the icon
 * for example it will draw the rectange around an active icon, the white circle on hover, etc.
 */
static gboolean task_item_draw (
    GtkWidget      *widget,
    cairo_t *cr,
    WpApplet* windowPickerApplet)
{
    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (TASK_IS_ITEM (widget), FALSE);
    TaskItem *item = TASK_ITEM (widget);
    g_return_val_if_fail (WNCK_IS_WINDOW (item->window), FALSE);
    cr = gdk_cairo_create (gtk_widget_get_window(widget));
    GdkRectangle area;
    GdkPixbuf *pbuf;
    area = item->area;
    pbuf = item->pixbuf;
    gint size = MIN (area.height, area.width) - 8;
    gboolean active = wnck_window_is_active (item->window);
    /* load the GSettings key for gray icons */
    gboolean icons_greyscale = wp_applet_get_icons_greyscale (item->windowPickerApplet);
    gboolean attention = wnck_window_or_transient_needs_attention (item->window);
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
        cairo_rectangle (cr, area.x + 1, area.y + 1, area.width - 2, area.height - 2);
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
            area.x + glow_x, area.y + glow_y, glow_x * 0.3,
            area.x + glow_x, area.y + glow_y, glow_x * 1.4
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
            (area.x + (area.width - gdk_pixbuf_get_width (pbuf)) / 2),
            (area.y + (area.height - gdk_pixbuf_get_height (pbuf)) / 2)
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
            (area.x + (area.width - gdk_pixbuf_get_width (desat)) / 2),
            (area.y + (area.height - gdk_pixbuf_get_height (desat)) / 2));
        g_object_unref (desat);
    }
    if (!item->mouse_over && attention) { /* urgent */
        GTimeVal current_time;
        g_get_current_time (&current_time);
        gdouble ms = (
            current_time.tv_sec - item->urgent_time.tv_sec) * 1000 +
            (current_time.tv_usec - item->urgent_time.tv_usec) / 1000;
        gdouble alpha = .66 + (cos (3.15 * ms / 600) / 3);
        cairo_paint_with_alpha (cr, alpha);
    } else if (item->mouse_over || active || !icons_greyscale) { /* focused */
        cairo_paint (cr);
    } else { /* not focused */
        cairo_paint_with_alpha (cr, .65);
    }
    cairo_destroy (cr);
    return FALSE;
}

static void on_size_allocate (
    GtkWidget     *widget,
    GtkAllocation *allocation,
    TaskItem      *item)
{
    g_return_if_fail (TASK_IS_ITEM (item));
    if (allocation->width != allocation->height + 6)
        gtk_widget_set_size_request (widget, allocation->height + 6, -1);
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
        gtk_menu_popup (
            GTK_MENU (menu), NULL, NULL, NULL, NULL,
            event->button, event->time
        );
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
        g_get_current_time (&taskItem->urgent_time);
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

static void
on_window_type_changed (WnckWindow *window,
                        TaskItem *item)
{
    WnckWindowType type;

    if (item->window != window)
        return;

    type = wnck_window_get_window_type (window);

    if (type == WNCK_WINDOW_DESKTOP ||
        type == WNCK_WINDOW_DOCK ||
        type == WNCK_WINDOW_SPLASHSCREEN ||
        type == WNCK_WINDOW_MENU)
      {
          task_item_close (item);
      }
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
on_window_geometry_changed (WnckWindow *window,
                            TaskItem   *item)
{
    GdkMonitor *old_monitor;
    GdkMonitor *window_monitor;

    window_monitor = get_window_monitor (window);

    old_monitor = item->monitor;
    if (old_monitor != window_monitor)
      {
        item->monitor = window_monitor;

        g_signal_emit (item, task_item_signals[TASK_ITEM_MONITOR_CHANGED], 0,
                       old_monitor);
      }
}

static void on_screen_active_window_changed (
    WnckScreen    *screen,
    WnckWindow    *old_window,
    TaskItem      *item)
{
    g_return_if_fail (TASK_IS_ITEM (item));
    WnckWindow *window;
    window = item->window;
    g_return_if_fail (WNCK_IS_WINDOW (window));
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

static void on_screen_window_closed (
    WnckScreen   *screen,
    WnckWindow   *window,
    TaskItem     *item)
{
    g_return_if_fail (TASK_IS_ITEM(item));
    g_return_if_fail (WNCK_IS_WINDOW (item->window));
    if (item->window == window) {
        task_item_close (item);
    }
}

static void
task_item_close (TaskItem   *item)
{
    g_signal_emit (G_OBJECT (item),
                   task_item_signals[TASK_ITEM_CLOSED_SIGNAL], 0);
}

static gboolean activate_window (GtkWidget *widget) {
    gint active;
    TaskItem *item;
    g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
    g_return_val_if_fail (TASK_IS_ITEM(widget), FALSE);
    item = TASK_ITEM (widget);
    g_return_val_if_fail (WNCK_IS_WINDOW (item->window), FALSE);
    active = GPOINTER_TO_INT (
        g_object_get_data (G_OBJECT (widget), "drag-true")
    );
    if (active) {
        WnckWindow *window = item->window;
        if (WNCK_IS_WINDOW (window))
            wnck_window_activate (window, time (NULL));
    }
    g_object_set_data (
        G_OBJECT (widget), "drag-true", GINT_TO_POINTER (0));
    return FALSE;
}

/* Emitted when a drag leaves the destination */
static void on_drag_leave (
    GtkWidget *item,
    GdkDragContext *context,
    guint time,
    gpointer user_data)
{
    g_object_set_data (G_OBJECT (item), "drag-true", GINT_TO_POINTER (0));
}

/* Emitted when a drag is over the destination */
static gboolean on_drag_motion (
    GtkWidget      *item,
    GdkDragContext *context,
    gint            x,
    gint            y,
    guint           time)
{
    GdkAtom         target_type = NULL;

    if (gdk_drag_context_list_targets(context)) {
        /* Choose the best target type */
        target_type = GDK_POINTER_TO_ATOM (
            g_list_nth_data (
                gdk_drag_context_list_targets(context),
                    TARGET_WIDGET_DRAGGED
            )
        );
        g_assert(target_type != NULL);

        gtk_drag_get_data (
            item,         // will receive 'drag-data-received' signal
            context,        // represents the current state of the DnD
            target_type,    // the target type we want
            time            // time stamp
        );
    } else {
        g_warning("Drag ended without target");
    }
    return FALSE;
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

static void on_drag_get_data(
    GtkWidget *widget,
    GdkDragContext *context,
    GtkSelectionData *selection_data,
    guint target_type,
    guint time,
    gpointer user_data)
{
    switch(target_type) {
        case TARGET_WIDGET_DRAGGED:
            g_assert(user_data != NULL && TASK_IS_ITEM(user_data));
            gtk_selection_data_set(
                selection_data,
                gtk_selection_data_get_target (selection_data),
                8,
                (guchar*) &user_data,
                sizeof(gpointer*)
            );
            break;
        default:
            g_assert_not_reached ();
    }
}

static void on_drag_end (
    GtkWidget *widget,
    GdkDragContext *drag_context,
    gpointer user_data)
{
    g_object_set_data (G_OBJECT (widget), "drag-true", GINT_TO_POINTER (0));
}

static gint grid_get_pos (GtkWidget *grid, GtkWidget *item) {
    GtkContainer *container = GTK_CONTAINER (grid);
    GList *items = gtk_container_get_children (container);

    while (items) {
        if (items->data == item) {
	        gint pos;
	        gtk_container_child_get (container, item, "position", &pos, NULL);
	        return pos;
	    }
        items = items->next;
    }
    return -1;
}

static void on_drag_received_data (
    GtkWidget *widget, //target of the d&d action
    GdkDragContext *context,
    gint x,
    gint y,
    GtkSelectionData *selection_data,
    guint target_type,
    guint time,
    TaskItem *item)
{
    if((selection_data != NULL) && (gtk_selection_data_get_length(selection_data) >= 0)) {
        gint active;
        switch (target_type) {
            case TARGET_WIDGET_DRAGGED: {
                GtkWidget *taskList = wp_applet_get_tasks(item->windowPickerApplet);
                gpointer *data = (gpointer *) gtk_selection_data_get_data(selection_data);
                g_assert(GTK_IS_WIDGET(*data));

                GtkWidget *taskItem = GTK_WIDGET(*data);
                g_assert(TASK_IS_ITEM(taskItem));
                if(taskItem == widget) break; //source and target are identical
                gint target_position = grid_get_pos(wp_applet_get_tasks(item->windowPickerApplet), widget);
                g_object_ref(taskItem);
                gtk_box_reorder_child(GTK_BOX(taskList), taskItem, target_position);
                g_object_unref(taskItem);
                break;
            }
            default:
                active = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "drag-true"));
                if (!active) {
                    g_object_set_data (
                        G_OBJECT (widget), "drag-true", GINT_TO_POINTER (1)
                    );
                    g_timeout_add (1000, (GSourceFunc)activate_window, widget);
                }
        }
    }
}

/* Returning true here, causes the failed-animation not to be shown. Without this the icon of the dnd operation
 * will jump back to where the dnd operation started.
 **/
static gboolean
on_drag_failed (GtkWidget      *widget,
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

static void
task_item_dispose (GObject *object)
{
    TaskItem *task_item = TASK_ITEM (object);

    G_OBJECT_CLASS (task_item_parent_class)->dispose (object);
}

static void task_item_finalize (GObject *object) {
    TaskItem *item = TASK_ITEM (object);

    if (item->blink_timer) {
        g_source_remove (item->blink_timer);
    }

    g_clear_object (&item->pixbuf);

    G_OBJECT_CLASS (task_item_parent_class)->finalize (object);
}

static void task_item_class_init (TaskItemClass *klass) {
    GObjectClass *obj_class      = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    obj_class->dispose = task_item_dispose;
    obj_class->finalize = task_item_finalize;
    widget_class->get_preferred_width = task_item_get_preferred_width;
    widget_class->get_preferred_height = task_item_get_preferred_height;
    task_item_signals [TASK_ITEM_CLOSED_SIGNAL] =
    g_signal_new ("task-item-closed",
        G_TYPE_FROM_CLASS (klass),
        G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
        0,
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    task_item_signals [TASK_ITEM_MONITOR_CHANGED] =
        g_signal_new ("monitor-changed", TASK_TYPE_ITEM, G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL, G_TYPE_NONE, 1, GDK_TYPE_MONITOR);
}

static void task_item_init (TaskItem *item) {
    item->blink_timer = 0;
}

GtkWidget *task_item_new (WpApplet* windowPickerApplet, WnckWindow *window) {
    g_return_val_if_fail (WNCK_IS_WINDOW (window), NULL);
    TaskItem *taskItem;
    WnckScreen *screen;
    GtkWidget *item = g_object_new (
        TASK_TYPE_ITEM,
        "has-tooltip", TRUE,
        "visible-window", FALSE,
        "above-child", TRUE,
        NULL
    );
    gtk_widget_set_vexpand(item, TRUE);
    gtk_widget_add_events (item, GDK_ALL_EVENTS_MASK);
    gtk_container_set_border_width (GTK_CONTAINER (item), 0);
    taskItem = TASK_ITEM (item);
    taskItem->window = window;
    screen = wnck_window_get_screen (window);
    taskItem->screen = screen;
    taskItem->windowPickerApplet = windowPickerApplet;
    taskItem->monitor = get_window_monitor (window);

    /** Drag and Drop code
     * This item can be both the target and the source of a drag and drop
     * operation. As a target it can receive strings (e.g. links).
     * As a source the dragged icon can be dragged to another position on the
     * task list.
     */
    //target (destination)
    gtk_drag_dest_set (
        item,
        GTK_DEST_DEFAULT_HIGHLIGHT,
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
    g_signal_connect_object (screen, "window-closed",
        G_CALLBACK (on_screen_window_closed), item, 0);
    g_signal_connect_object (window, "workspace-changed",
        G_CALLBACK (on_window_workspace_changed), item, 0);
    g_signal_connect_object (window, "state-changed",
        G_CALLBACK (on_window_state_changed), item, 0);
    g_signal_connect_object (window, "icon-changed",
        G_CALLBACK (on_window_icon_changed), item, 0);
    g_signal_connect_object (window, "type-changed",
        G_CALLBACK (on_window_type_changed), item, 0);
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
