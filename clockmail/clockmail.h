/*###################################################################*/
/*##                         clock & mail applet 0.1.0 alpha       ##*/
/*###################################################################*/

#include <config.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <gnome.h>
#include "applet-lib.h"
#include "applet-widget.h"

#define CLOCKMAIL_APPLET_VERSION_MAJ 0
#define CLOCKMAIL_APPLET_VERSION_MIN 1
#define CLOCKMAIL_APPLET_VERSION_REV 0

extern int BLINK_DELAY;
extern int BLINK_TIMES;

extern int MILIT_TIME;
extern int ALWAYS_BLINK;

void property_load();
void property_save();
void property_show();

