/* GNOME modemlights applet
 * (C) 2000 John Ellis
 *
 * Authors: John Ellis
 *          Martin Baulig
 *
 */

#include <config.h>

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
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

void property_load(const char *path);
void property_save(const char *path, gint to_default);
void property_show(AppletWidget *applet, gpointer data);


