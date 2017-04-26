#ifndef _PACKET_H_
#define _PACKET_H_

#include <stdio.h>
#include "serial.h"
#include "dmm_config.h"


	
void sendPacket(int mode, float voltage, int range, int extra);
	

#endif /*_PACKET_H_*/
