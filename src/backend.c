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

#include "backend.h"

/* Check for all available devices. This really should be
 * portable for at least all plattforms using the gnu c lib
 */
GList* 
get_available_devices(void)
{
	struct if_nameindex *devices, *ptr;
	GList *device_glist = NULL;
	
	devices = if_nameindex();
	for (ptr = devices; ptr->if_index; ptr++) {
		device_glist = g_list_append(device_glist, g_strdup(ptr->if_name));
	}
	if_freenameindex(devices);
	
	return device_glist;
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
	if (devinfo->name)
		g_free(devinfo->name);
	if (devinfo->ip)
		g_free(devinfo->ip);
	if (devinfo->netmask)
		g_free(devinfo->netmask);
	if (devinfo->ptpip)
		g_free(devinfo->ptpip);
	if (devinfo->hwaddr)
		g_free(devinfo->hwaddr);
}

#ifdef USE_LIBGTOP
static void
get_netload(const char *device, guint64 *tx, guint64 *rx)
{
	glibtop_netload netload;
	
	g_assert(device);
	glibtop_get_netload(&netload, device);
	*rx = netload.bytes_in;
	*tx = netload.bytes_out;
}

#else

/* This is a quick hack, because libgtop2 2.8.x seems to be broken */
static void
get_netload(const char *device, guint64 *tx, guint64 *rx)
{
	FILE *fp;
	char *line = NULL;
	
	*tx = *rx = 0;
	
	if (!(fp = fopen("/proc/net/dev", "r"))) return;
	while (!feof(fp)) {
		char c, *ptr, *end;
		size_t num = 0;
				
		if (line) g_free(line);
		line = NULL;
		for (c = 0; 1; ++num) {
			fread(&c, 1, 1, fp);
			if (feof(fp) || ferror(fp) || (c == '\n')) break;
			line = g_realloc(line, num + 2);
			line[num] = c; line[num + 1] = 0;
		}
		if (num < strlen(device)) continue;
		for (ptr = line; *ptr; ptr++) if (isalpha(*ptr)) break;
			
		if (strncmp(device, ptr, strlen(device))) continue;

		for (; *ptr; ptr++) if (*ptr == ':') break;		
		ptr++;
		for (; *ptr; ptr++) if (isdigit(*ptr)) break;
		end = 0;
		*rx = g_ascii_strtoull(ptr, &end, 0);
		
		ptr = end;
		for (num = 0; num < 7; ++num) {
			for (; *ptr; ptr++) if (isdigit(*ptr)) break;
			for (; *ptr; ptr++) if (!isdigit(*ptr)) break;
		}	
		for (; *ptr; ptr++) if (isdigit(*ptr)) break;
		end = 0;
		*tx = g_ascii_strtoull(ptr, &end, 0);
	}
	if (line) g_free(line);
	fclose(fp);	
}	
#endif

/* Uses some ioctl()s to get information about a network
 * device. Don't know if this is portable, but chances are
 * good, cause its the same way ifconfig does it
 */
DevInfo
get_device_info(const char *device)
{
	int fd;
	struct ifreq request;
	DevInfo devinfo;
	short int flags;
	
	g_assert(device);
	
	memset(&devinfo, 0, sizeof(DevInfo));
	devinfo.name = g_strdup(device);
	get_netload(device, &devinfo.tx, &devinfo.rx);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return devinfo;
	
	memset(&request, 0x0, sizeof(request));
	strcpy(request.ifr_name, device);

	if (ioctl(fd, SIOCGIFFLAGS, &request) == 0) flags = request.ifr_flags;
	else flags = 0;
	devinfo.up = flags & IFF_UP ? TRUE : FALSE;
	devinfo.running = flags & IFF_RUNNING ? TRUE : FALSE;
	
	/* Get the ip */
	if (ioctl(fd, SIOCGIFADDR, &request) == 0)  {
		struct sockaddr_in *address = (struct sockaddr_in*)&request.ifr_addr;
		devinfo.ip = g_strdup_printf("%s", inet_ntoa(address->sin_addr));
	} else devinfo.ip = NULL;
	
	/* Get the hardware/physical adress/ MAC */
	if (ioctl(fd, SIOCGIFHWADDR, &request) == 0) {
		unsigned char hwaddr[6];
		struct sockaddr *hwaddress = &request.ifr_hwaddr;
		memcpy(hwaddr, hwaddress->sa_data, 6);
		devinfo.hwaddr = g_strdup_printf("%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
	
	/* I hope this works good enough to distiguish between big & little endian */
#if __BYTE_ORDER == __LITTLE_ENDIAN
			hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
#else
			hwaddr[5], hwaddr[4], hwaddr[3], hwaddr[2], hwaddr[1], hwaddr[0]);
#endif		
	} else devinfo.hwaddr = NULL;
	
	/* Get the subnetmask */
	if (ioctl(fd, SIOCGIFNETMASK, &request) == 0) {
		struct sockaddr_in *address = (struct sockaddr_in*)&request.ifr_netmask;
		devinfo.netmask = g_strdup_printf("%s", inet_ntoa(address->sin_addr));
	} else devinfo.netmask = NULL;

	/* Check if the device is a ptp and if this is the
 	 * case, get the ptp-ip */
	devinfo.ptpip = NULL;
	if (flags & IFF_POINTOPOINT) {
		if (ioctl(fd, SIOCGIFDSTADDR, &request) == 0) {
			struct sockaddr_in *address = (struct sockaddr_in*)&request.ifr_dstaddr;
			devinfo.ptpip = g_strdup_printf("%s", inet_ntoa(address->sin_addr));
		}
	}

	if (flags & IFF_LOOPBACK) devinfo.type = DEV_LO;
	if (flags & IFF_POINTOPOINT) devinfo.type = DEV_PPP;
	
	if (ioctl(fd, SIOCGIWNAME, &request) == 0) 
		devinfo.type = DEV_WIRELESS;
	
/* Really stupid way to figure out type of device for (s|p)lip and eth */
	if (devinfo.type == DEV_UNKNOWN) {
		if (strstr(device, "plip")) devinfo.type = DEV_PLIP;
		if (strstr(device, "slip")) devinfo.type = DEV_SLIP;
		if (strstr(device, "eth")) devinfo.type = DEV_ETHERNET;
	}
	
	close(fd);
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
