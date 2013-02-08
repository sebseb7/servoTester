#ifndef MAIN_H_
#define MAIN_H_

#include "LPC13Uxx.h"
#define CDC		0
#define UART  	1
#define BRIDGE	2
extern void Comport_Init(char device,int baudrate);



#endif
