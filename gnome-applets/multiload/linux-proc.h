#ifndef LINUX_PROC_H__
#define LINUX_PROC_H__

#include "load-graph.h"

G_GNUC_INTERNAL void GetLoad (int Maximum, int data [5], LoadGraph *g);
G_GNUC_INTERNAL void GetDiskLoad (int Maximum, int data [3], LoadGraph *g);
#if 0
G_GNUC_INTERNAL void GetPage (int Maximum, int data [3], LoadGraph *g);
#endif /* 0 */
G_GNUC_INTERNAL void GetMemory (int Maximum, int data [5], LoadGraph *g);
G_GNUC_INTERNAL void GetSwap (int Maximum, int data [2], LoadGraph *g);
G_GNUC_INTERNAL void GetLoadAvg (int Maximum, int data [2], LoadGraph *g);
G_GNUC_INTERNAL void GetNet (int Maximum, int data [4], LoadGraph *g);

#endif
