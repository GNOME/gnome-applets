#ifndef _WINDOW_PICKER_APPLET_H_
#define _WINDOW_PICKER_APPLET_H_

#include <panel-applet.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define SHOW_WIN_KEY "show-all-windows"
#define SHOW_APPLICATION_TITLE_KEY "show-application-title"
#define SHOW_HOME_TITLE_KEY "show-home-title"
#define ICONS_GREYSCALE_KEY "icons-greyscale-mask"
#define EXPAND_TASK_LIST "expand-task-list"

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

    /*< private >*/
    WindowPickerAppletPrivate *priv;
};

struct _WindowPickerAppletClass {
    PanelAppletClass parent_class;
};


GType window_picker_applet_get_type (void) G_GNUC_CONST;

/* Getters for private fields */
GSettings *window_picker_applet_get_settings (WindowPickerApplet *picker);
GtkWidget* window_picker_applet_get_tasks (WindowPickerApplet* windowPickerApplet);
GtkWidget* window_picker_applet_get_title (WindowPickerApplet *windowPickerApplet);

G_END_DECLS

#endif /* _WINDOW_PICKER_APPLET_H_ */
