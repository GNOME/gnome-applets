#ifndef LINUX_PROC_H__
#define LINUX_PROC_H__

#include <load-graph.h>

void GetLoad (int Maximum, int data [5], LoadGraph *g) G_GNUC_INTERNAL;
void GetDiskLoad (int Maximum, int data [3], LoadGraph *g) G_GNUC_INTERNAL;
#if 0
void GetPage (int Maximum, int data [3], LoadGraph *g) G_GNUC_INTERNAL;
#endif /* 0 */
void GetMemory (int Maximum, int data [4], LoadGraph *g) G_GNUC_INTERNAL;
void GetSwap (int Maximum, int data [2], LoadGraph *g) G_GNUC_INTERNAL;
void GetLoadAvg (int Maximum, int data [2], LoadGraph *g) G_GNUC_INTERNAL;
void GetNet (int Maximum, int data [3], LoadGraph *g) G_GNUC_INTERNAL;

#endif
