#include <applet-widget.h>
#include "gwmh.h"

#define CONFIG_WIDTH 450
#define CONFIG_HEIGHT 48
#define CONFIG_ROWHEIGHT 24
#define CONFIG_ROWS 2
#define CONFIG_PIXMAP 0
typedef struct _TasklistTask TasklistTask;
typedef struct _Config Config;

struct _TasklistTask
{
  gint x, y;
  gint width, height;
  GdkPixmap pixmap;
  GdkBitmap bitmap;
  GwmhTask *task;
};

struct _Config
{
  gboolean show_winops;
  gboolean confirm_kill;
  gboolean all_tasks;
  gboolean minimized_tasks;
  gboolean show_all;
  gboolean show_normal;
  gboolean show_minimized;
  gboolean numrows_follows_panel;
};

void menu_popup (TasklistTask *temp_task, guint button, guint32 activate_time);
void config_load (const gchar *privcfgpath);
void config_save (const gchar *privcfgpath);
void properties_show(void);
void tasklist_layout (void);
