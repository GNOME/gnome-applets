/*
 * Copyright (C) 2014 Sebastian Geiger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _WINDOW_PICKER_APPLET_H_
#define _WINDOW_PICKER_APPLET_H_

#include <panel-applet.h>

G_BEGIN_DECLS

#define WINDOW_PICKER_APPLET_TYPE             (window_picker_applet_get_type())
#define WINDOW_PICKER_APPLET(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), WINDOW_PICKER_APPLET_TYPE, WindowPickerApplet))
#define WINDOW_PICKER_APPLET_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), WINDOW_PICKER_APPLET_TYPE, WindowPickerAppletClass))
#define IS_WINDOW_PICKER_APPLET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), WINDOW_PICKER_APPLET_TYPE))
#define IS_WINDOW_PICKER_APPLET_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), WINDOW_PICKER_APPLET_TYPE))
#define WINDOW_PICKER_APPLET_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), WINDOW_PICKER_APPLET_TYPE, WindowPickerAppletClass))

typedef struct _WindowPickerApplet        WindowPickerApplet;
typedef struct _WindowPickerAppletClass   WindowPickerAppletClass;
typedef struct _WindowPickerAppletPrivate WindowPickerAppletPrivate;

struct _WindowPickerApplet  {
    PanelApplet parent;
    WindowPickerAppletPrivate *priv;
};

struct _WindowPickerAppletClass {
    PanelAppletClass parent_class;
};

GType window_picker_applet_get_type (void) G_GNUC_CONST;

/* Getters for private fields */
GSettings *window_picker_applet_get_settings (WindowPickerApplet *picker);
GtkWidget* window_picker_applet_get_tasks (WindowPickerApplet* windowPickerApplet);
gboolean window_picker_applet_get_show_all_windows (WindowPickerApplet *picker);
gboolean window_picker_applet_get_show_application_title (WindowPickerApplet *picker);
gboolean window_picker_applet_get_show_home_title (WindowPickerApplet *picker);
gboolean window_picker_applet_get_icons_greyscale (WindowPickerApplet *picker);
gboolean window_picker_applet_get_expand_task_list (WindowPickerApplet *picker);

G_END_DECLS

#endif /* _WINDOW_PICKER_APPLET_H_ */
