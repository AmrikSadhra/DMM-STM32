#include "packet.h"

typedef struct {
	char packetData[PACKET_SIZE];
} packet;

typedef struct {
	float voltage;
	float current;
} generic;

void sendPacket(int mode, float voltage, int range){
		packet toSend;
		sprintf(toSend.packetData, "<m:%f;%d>",  voltage, range);
		bt_send_str(toSend.packetData);
}
