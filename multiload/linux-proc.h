#ifndef LINUX_PROC_H__
#define LINUX_PROC_H__

void GetLoad(int Maximum, int *usr, int *nice, int *sys, int *free);
void GetMemory(int Maximum, int *used, int *shared, int *buffer, int *cached);
void GetSwap (int Maximum, int *used, int *free);

#endif
