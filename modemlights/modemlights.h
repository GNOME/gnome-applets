/*#####################################################*/
/*##           modemlights applet 0.3.0 alpha        ##*/
/*#####################################################*/

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#ifdef __FreeBSD__
#include <net/if_ppp.h>
#endif /* __FreeBSD__ */
#include <net/ppp_defs.h>

#include <gnome.h>
#include "applet-lib.h"
#include "applet-widget.h"

#define MODEMLIGHTS_APPLET_VERSION_MAJ 0
#define MODEMLIGHTS_APPLET_VERSION_MIN 3
#define MODEMLIGHTS_APPLET_VERSION_REV 0

extern gint UPDATE_DELAY;
extern gchar *lock_file;
extern gchar *command_connect;
extern gchar *command_disconnect;

void start_callback_update();

void property_load(char *path);
void property_save(char *path);
void property_show(AppletWidget *applet, gpointer data);


