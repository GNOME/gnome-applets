/*  backend.c
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Netspeed Applet was writen by Jörgen Scheibengruber <mfcn@gmx.de>
 */

#include <glibtop/netlist.h>
#include <glibtop/netload.h>
#include "backend.h"

/* Check for all available devices. This really should be
 * portable for at least all plattforms using the gnu c lib
 */
GList* 
get_available_devices(void)
{
	glibtop_netlist buf;
	char **devices, **dev, *prev = "";
	GList *device_glist = NULL;

	devices = glibtop_get_netlist(&buf);

	/* TODO: I assume that duplicates follow each other. Can I do so? */
	for(dev = devices; *dev; ++dev) {
		if (strcmp(prev, *dev) == 0) continue;
		device_glist = g_list_append(device_glist, g_strdup(*dev));
		prev = *dev;
	}

	g_strfreev(devices);

	return device_glist;
}

const gchar*
get_default_route(void)
{
	FILE *fp;
	static char device[50];
	
	fp = fopen("/proc/net/route", "r");
	
	if (fp == NULL) return NULL;
	
	while (!feof(fp)) {
		char buffer[1024]; 
		unsigned int ip, gw, flags, ref, use, metric, mask, mtu, window, irtt;
		int retval;
		
		fgets(buffer, 1024, fp);
		
		retval = sscanf(buffer, "%49s %x %x %x %d %d %d %x %d %d %d",
				device, &ip, &gw, &flags, &ref, &use, &metric, &mask, &mtu, &window, &irtt);
		
		if (retval != 11) continue;
			
		if (gw == 0) {
			fclose(fp);
			return device;
		}			
	}
	fclose(fp);	
	return NULL;
}


void
free_devices_list(GList *list)
{
	g_list_foreach(list, (GFunc)g_free, NULL);
	g_list_free(list);
}


/* Frees a DevInfo struct and all the stuff it contains
 */
void
free_device_info(DevInfo *devinfo)
{
	g_free(devinfo->name);
	g_free(devinfo->ip);
	g_free(devinfo->netmask);
	g_free(devinfo->ptpip);
	g_free(devinfo->hwaddr);
	g_free(devinfo->ipv6);
}




static char*
format_ipv4(guint32 ip)
{
	char str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ip, str, sizeof str);
	return g_strdup(str);
}


static char*
format_ipv6(const guint8 ip[16])
{
	char str[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, ip, str, sizeof str);
	return g_strdup(str);
}


/* TODO:
   these stuff are not portable because of ioctl
*/
static void
get_additional_info(DevInfo *devinfo)
{
	int fd = -1;
	struct ifreq request = {};

	g_strlcpy(request.ifr_name, devinfo->name, sizeof request.ifr_name);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		goto out;

	if (ioctl(fd, SIOCGIFFLAGS, &request) == -1)
		goto out;

	/* Check if the device is a ptp and if this is the
	* case, get the ptp-ip */
	devinfo->ptpip = NULL;
	if (request.ifr_flags & IFF_POINTOPOINT) {
		if (ioctl(fd, SIOCGIFDSTADDR, &request) >= 0) {
			struct sockaddr_in* addr;
			addr = (struct sockaddr_in*)&request.ifr_dstaddr;
			devinfo->ptpip = format_ipv4(addr->sin_addr.s_addr);
		}
	}


	if (ioctl(fd, SIOCGIWNAME, &request) >= 0) {
		devinfo->type = DEV_WIRELESS;
	}

	if (ioctl(fd, SIOCGIWENCODE, &request) >= 0) {
		g_assert_not_reached();
	}
    
    
 out:
	if(fd != -1)
		close(fd);
}




DevInfo
get_device_info(const char *device)
{
	glibtop_netload netload;
	DevInfo devinfo = {0};
	guint8 *hw;
    
	g_assert(device);

	devinfo.name = g_strdup(device);
	devinfo.type = DEV_UNKNOWN;

	glibtop_get_netload(&netload, device);
	devinfo.tx = netload.bytes_out;
	devinfo.rx = netload.bytes_in;

	devinfo.up = (netload.if_flags & (1L << GLIBTOP_IF_FLAGS_UP) ? TRUE : FALSE);
	devinfo.running = (netload.if_flags & (1L << GLIBTOP_IF_FLAGS_RUNNING) ? TRUE : FALSE);

	devinfo.ip = format_ipv4(netload.address);
	devinfo.netmask = format_ipv4(netload.subnet);
	devinfo.ipv6 = format_ipv6(netload.address6);

	hw = netload.hwaddress;
	devinfo.hwaddr = g_strdup_printf(
		"%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
		hw[0], hw[1], hw[2], hw[3],
		hw[4], hw[5], hw[6], hw[7]);

	/* stolen from gnome-applets/multiload/linux-proc.c */

	if(netload.if_flags & (1L << GLIBTOP_IF_FLAGS_LOOPBACK)) {
		devinfo.type = DEV_LO;
	}
	else if(netload.if_flags & (1L << GLIBTOP_IF_FLAGS_POINTOPOINT)) {
		if (g_str_has_prefix(device, "plip")) {
			devinfo.type = DEV_PLIP;
		}
		else if (g_str_has_prefix(device, "sl")) {
			devinfo.type = DEV_SLIP;
		}
		else {
			devinfo.type = DEV_PPP;
		}
	}
	else if (g_str_has_prefix(device, "eth")) {
		devinfo.type = DEV_ETHERNET;
	}

	get_additional_info(&devinfo);
    
	return devinfo;
}

gboolean
compare_device_info(const DevInfo *a, const DevInfo *b)
{
	g_assert(a && b);
	g_assert(a->name && b->name);
	
	if (!g_str_equal(a->name, b->name)) return TRUE;
	if (a->ip && b->ip) {
		if (!g_str_equal(a->ip, b->ip)) return TRUE;
	} else {
		if (a->ip || b->ip) return TRUE;
	}			
	/* Ignore hwaddr, ptpip and netmask... I think this is ok */
	if (a->up != b->up) return TRUE;
	if (a->running != b->running) return TRUE;

	return FALSE;	
}
