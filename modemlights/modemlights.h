/* GNOME modemlights applet
 * (C) 1999 John Ellis
 *
 * Authors: John Ellis
 *          Martin Baulig
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

#ifdef __OpenBSD__
#  include <net/bpf.h>
#  include <net/if_pppvar.h>
#  include <net/if_ppp.h>
#endif /* __OpenBSD__ */

#ifdef __FreeBSD__
#    include <net/if_ppp.h>
#endif /* __FreeBSD__ */

#include <net/ppp_defs.h>

#include <gnome.h>
#include <applet-widget.h>

#define MODEMLIGHTS_APPLET_VERSION_MAJ 1
#define MODEMLIGHTS_APPLET_VERSION_MIN 0
#define MODEMLIGHTS_APPLET_VERSION_REV 2

extern gint UPDATE_DELAY;
extern gchar *lock_file;
extern gint verify_lock_file;
extern gchar *command_connect;
extern gchar *command_disconnect;
extern int ask_for_confirmation;
extern gchar *device_name;
extern gint use_ISDN;
extern gint show_extra_info;

extern GtkWidget *applet;

void start_callback_update(void);
void reset_orientation(void);

void property_load(char *path);
void property_save(char *path);
void property_show(AppletWidget *applet, gpointer data);


