#ifndef LINUX_PROC_H__
#define LINUX_PROC_H__

#include <load-graph.h>

void GetLoad (int Maximum, int data [4], LoadGraph *g);
void GetPage (int Maximum, int data [3], LoadGraph *g);
void GetMemory (int Maximum, int data [4], LoadGraph *g);
void GetSwap (int Maximum, int data [2], LoadGraph *g);
void GetLoadAvg (int Maximum, int data [2], LoadGraph *g);
void GetNet (int Maximum, int data [3], LoadGraph *g);

#endif
