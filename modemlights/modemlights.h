/*###################################################################*/
/*##                         modemlights applet 0.1.0 alpha        ##*/
/*##                                          by John Ellis        ##*/
/*###################################################################*/

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/ppp_defs.h>

#include <gnome.h>
#include "applet-lib.h"
#include "applet-widget.h"

#define MODEMLIGHTS_APPLET_VERSION_MAJ 0
#define MODEMLIGHTS_APPLET_VERSION_MIN 1
#define MODEMLIGHTS_APPLET_VERSION_REV 0

extern gint UPDATE_DELAY;
extern gchar lock_file[];

void start_callback_update();

void property_load(char *path);
void property_save(char *path);
void property_show(AppletWidget *applet, gpointer data);


