#ifndef LINUX_PROC_H__
#define LINUX_PROC_H__

unsigned long int GetTraffic(int refresh, char *device);

typedef struct Device_Info {
	char	name[6];
	unsigned long int	bytes;
} Device_Info;

#endif
