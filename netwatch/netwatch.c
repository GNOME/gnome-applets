/*
 * GNOME panel netwatch network status watcher module.
 * (C) 1997 Red Hat Software
 *
 * Author: Michael K. Johnson <johnsonm@redhat.com>
 */


/*
 * This program is currently quite specific to Linux.
 * Feel free to re-implement the Linux-specific parts
 * on top of other operating systems, or on top of
 * popen("/sbin/ifconfig") or something like that.
 *
 * In addition, some pieces are specific to functionality
 * available with Red Hat Linux.  Feel free to either work
 * towards more generic alternatives (the Red Hat stuff should
 * stay for best results on Red Hat systems) or to port the
 * Red Hat components (like netreport) to other Linux
 * distributions or other Unix variants.  The tools specific
 * to Red Hat on which this code relies are licensed under the GPL.
 */


#include <fcntl.h>
#ifdef __linux__ /* add others as correct */
#include <net/if.h>
#endif
#include <regex.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#ifdef HAVE_LIBINTL
#include <libintl.h>
#define _(String) gettext(String)
#else
#define _(String) (String)
#endif
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include "gnome.h"
#include "../panel_cmds.h"
#include "../applet_cmds.h"
#include "../panel.h"


/* definitions */
#define APPLET_ID "Netwatch"
#define INT_DOWN 0
#define INT_UP 1


typedef struct _Interface Interface;
struct _Interface {
  gchar *name;
  GtkWidget *button;
  int status;
};



/* globals */
static PanelCmdFunc panel_cmd_func;
static GSList *interface_list;
static guint update_tag;
static struct sigaction old_sigio_sigaction;







/* We need a fork/exec so that netreport can find this process as the
 * parent pid.  This is a simplified fork/exec that serves our purposes.
 * 
 * Why doesn't something like this exist in libc?
 * Sometimes I don't want to system()
 */
static int
fork_exec(char *path, char *arg)
{
  pid_t child;
  int status;

  if (!(child = fork())) {
    /* child */
    execl(path, path, arg, 0);
    perror(path);
    /* Can't call exit(), because it flushes the X protocol stream */
    _exit (1);
  }

  /* Hmm, I'm a parent.  Might as well wait and avoid zombies, eh? */
  wait4 (child, &status, 0, NULL);
  if (WIFEXITED(status) && (WEXITSTATUS(status) == 0)) {
    return 0;
  } else {
    return 1;
  }
}





#ifdef __linux__

static int
has_redhat_ppp_maps()
{
  /* for now, just test if it is a Red Hat system.  Other systems
   * that adopt the same thing can add tests for their systems here
   */
  if (access("/etc/redhat-release", R_OK)) return 1;
  return 0;
}


static gchar *
ppp_logical_to_physical(const gchar *logical_name)
{
  /* way too much space for "/var/run/ppp-ppp??.dev" */
  char map_file_name[100];
  /* way too much space for "ppp??" */
  char buffer[20];
  int f;
  char *physical_device = NULL;

  sprintf(map_file_name, "/var/run/ppp-%s.dev", logical_name);
  if (f = open(map_file_name, O_RDONLY) >= 0) {
    if (read(f, buffer, 20) > 0) {
      physical_device = g_strdup(buffer);
    }
    close(f);
  }

  return physical_device;
}


/* get_interface_status_by_name takes an interface name and returns
 * INT_DOWN or INT_UP (it doesn't know about changing; that
 * is handled elsewhere)
 *
 * ifconfig source code was quite helpful here.  :-)
 */
static gint
get_interface_status_by_name(const gchar *interface_name)
{
  int sock = 0;
  int pfs[] = {AF_INET, AF_IPX, AF_AX25, AF_APPLETALK, 0};
  int p = 0;
  struct ifreq ifr;
  gchar *physical_name = NULL;
  /* If we don't find it, interface must be down */
  int retcode = INT_DOWN;

  while (!sock && pfs[p]) {
    sock = socket(pfs[p++], SOCK_DGRAM, 0);
  }

  if (!sock) {
    return INT_DOWN;
  }

  if (has_redhat_ppp_maps()) {
    if (!strncmp("ppp", interface_name, 3)) {
      /* It is a PPP device, so we need to map it from logical to physical
       * before we look it up.
       */
      physical_name = ppp_logical_to_physical(interface_name);
      if (!physical_name) {
        /* If we can't find a mapping, it's because it's down. */
        return INT_DOWN;
      }
    }
  }

  if (!physical_name) physical_name = g_strdup(interface_name);

  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, physical_name);

  if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
    retcode = INT_DOWN;
  }
  if (ifr.ifr_flags & IFF_UP) {
    retcode = INT_UP;
  }

  free(physical_name);
  close(sock);
  return retcode;
}


#endif /* __linux__ */



static void
set_interface_status(Interface *interface)
{
  regex_t p;
  regmatch_t *pmatch;
  gchar *interface_type = NULL;
  gchar *interface_name = NULL;
  size_t matchlen;

  regcomp(&p, "^([a-z]+)", REG_EXTENDED|REG_ICASE);
  pmatch = alloca(sizeof(regmatch_t) * p.re_nsub);
  regexec(&p, interface->name, p.re_nsub, pmatch, 0);
  matchlen = pmatch[0].rm_eo - pmatch[0].rm_so;
  interface_type = alloca(matchlen + 1);
  strncpy(interface_type, interface->name+pmatch[0].rm_so, matchlen);

  interface->status = get_interface_status_by_name(interface->name);
  if (interface->status == INT_DOWN) {
    interface_name = alloca(matchlen+15);
    sprintf(interface_name, "netwatch_%s_down", interface_type);
  } else {
    interface_name = alloca(matchlen+13);
    sprintf(interface_name, "netwatch_%s_up", interface_type);
  }

  gtk_widget_set_name(interface->button, interface_name);

  regfree(&p);
}


static Interface*
create_interface_by_name(gchar *devname)
{
  Interface *dev;

  dev = g_malloc(sizeof(Interface));
  if (!dev) return NULL;

  dev->name = g_strdup(devname);
  if (!dev->name) return NULL;
  /* Not sure a button is the right thing; what does pressing it do?
   * But I want something that can be sized and take pixmaps out of
   * a style file automatically.  I guess I want a blank_widget...
   * Maybe I'll come up with something for the button to do later,
   * like pop up a window with stats on the device?
   */
  dev->button = gtk_button_new();
  gtk_widget_show(dev->button);
  gtk_widget_set_usize(dev->button, 24, 48);
  set_interface_status(dev);

  return dev;
}





static void
update_status(int ignore)
{
  g_slist_foreach(interface_list, (GFunc) set_interface_status, NULL);
}

#ifdef __linux__

/* start accepting SIGIO as notification of network status
 * changes.  Ignore failure; we'll be polling as well in any
 * case; this just lets us know sooner on Red Hat systems.
 */
static void
netwatch_netreport_init (void)
{
  struct sigaction act;

  act.sa_handler = update_status;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  sigaction(SIGIO, &act, &old_sigio_sigaction);
  fork_exec("/sbin/netreport", NULL);
}


static void
netwatch_destroy (void)
{
  fork_exec("/sbin/netreport", "-r");
  sigaction(SIGIO, &old_sigio_sigaction, NULL);
}

#endif /* __linux__ */




static void
properties (GtkWidget *parent_widget)
{
  /* FIXME */
  return;
}



static GtkWidget *
create_netwatch (GtkWidget *window, char *parameters)
{

	GtkWidget *parent_widget;
	Interface *this_int;
	gchar *devpath;
	gchar *dev;

	parent_widget = gtk_hbox_new(FALSE, 0);

	g_slist_free(interface_list);
	interface_list = NULL;
	devpath = gnome_config_get_string("/panel/netwatch/devpath=eth0");
	dev = strtok(devpath, ":");
	if (!dev) return NULL;
	do {
		this_int = create_interface_by_name(dev);
		interface_list = g_slist_append(interface_list, this_int);
		gtk_box_pack_end(GTK_BOX(parent_widget), this_int->button,
				FALSE, FALSE, 0);
	} while (dev = strtok(NULL, ":"));
	free(devpath);


#ifdef __linux__
	netwatch_netreport_init();
	/* we call netwatch_destroy explicitly when the applet is
	 * destroyed, but if something goes wrong, it would be nice
	 * to make sure it gets called anyway on a normal exit.
	 * It doesn't hurt to call it twice.
	 */
	/* FIXME: unfortunately, we can't use atexit, because we
	 * can't remove something from atexit's list of functions
	 * to call.  A dispatcher in panel based on a GSlist would
	 * be nice...
	 * Or maybe we just trust APPLET_CMD_DESTROY_MODULE...
	 */
	/* atexit(netwatch_destroy); */
#endif

	update_tag = gtk_timeout_add (10, update_status, NULL);

	return parent_widget;
}



static void
add_devpath_component(gpointer data, gpointer user_data)
{
  Interface *iface = data;
  GString **tmp_devpath = user_data;

  if (!*tmp_devpath) {
    *tmp_devpath = g_string_new(iface->name);
  } else {
    *tmp_devpath = g_string_append_c(*tmp_devpath, ':');
    *tmp_devpath = g_string_append(*tmp_devpath, iface->name);
  }
}

static gchar *
get_interface_path(void)
{
  GString *new_devpath = NULL;
  gchar *path = NULL;

  g_slist_foreach(interface_list, add_devpath_component, &new_devpath);
  path = g_strdup(new_devpath->str); /* caller frees */
  g_string_free(new_devpath, 1);
  return path;
}


static void
create_instance (Panel *panel, char *params, int xpos, int ypos)
{
	GtkWidget *netwatch;
	PanelCommand cmd;

	netwatch = create_netwatch (panel->window, params);

	if (!netwatch)
		return;

	cmd.cmd = PANEL_CMD_REGISTER_TOY;
	cmd.params.register_toy.applet = netwatch;
	cmd.params.register_toy.id     = APPLET_ID;
	cmd.params.register_toy.xpos   = xpos;
	cmd.params.register_toy.ypos   = ypos;
	cmd.params.register_toy.flags  = APPLET_HAS_PROPERTIES;

	(*panel_cmd_func) (&cmd);
}

gpointer
applet_cmd_func(AppletCommand *cmd)
{
	gchar *interface_path = NULL;

	g_assert(cmd != NULL);

	switch (cmd->cmd) {
		case APPLET_CMD_QUERY:
			return APPLET_ID;

		case APPLET_CMD_INIT_MODULE:
			panel_cmd_func = cmd->params.init_module.cmd_func;
			break;

		case APPLET_CMD_DESTROY_MODULE:
			interface_path = get_interface_path();
			gnome_config_set_string("/panel/netwatch/devpath",
						interface_path);
			g_free(interface_path);
			gnome_config_sync();
#ifdef __linux__
			netwatch_destroy();
#endif
			gtk_timeout_remove (update_tag);
			break;

		case APPLET_CMD_GET_DEFAULT_PARAMS:
			fprintf(stderr, "Launcher: APPLET_CMD_GET_DEFAULT_PARAMS not yet supported\n");
			return g_strdup(""); /* FIXME */

		case APPLET_CMD_CREATE_INSTANCE:
			create_instance(cmd->panel,
					cmd->params.create_instance.params,
					cmd->params.create_instance.xpos,
					cmd->params.create_instance.ypos);
			break;

		case APPLET_CMD_GET_INSTANCE_PARAMS:
			return g_strdup(gtk_object_get_user_data(GTK_OBJECT(cmd->applet)));

		case APPLET_CMD_PROPERTIES:
			properties(cmd->applet);
			break;

		default:
			fprintf(stderr,
				APPLET_ID " applet_cmd_func: Oops, unknown command type %d\n",
				(int) cmd->cmd);
			break;
	}

	return NULL;
}
