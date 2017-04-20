#ifndef _PACKET_H_
#define _PACKET_H_

	#define PACKET_SIZE 30
	#include <stdio.h>
	#include "serial.h"
	
	void sendPacket(int mode, float voltage, int range);
	

#endif /*_PACKET_H_*/
