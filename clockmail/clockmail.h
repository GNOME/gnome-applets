/*###################################################################*/
/*##                         clock & mail applet 0.1.2 alpha       ##*/
/*###################################################################*/

#include <sys/types.h>
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
#define CLOCKMAIL_APPLET_VERSION_REV 2

extern int BLINK_DELAY;
extern int BLINK_TIMES;

extern int AM_PM_ENABLE;
extern int ALWAYS_BLINK;

extern char *mail_file;

void check_mail_file_status (int reset);

void property_load();
void property_save();
void property_show();

