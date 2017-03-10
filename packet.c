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
	sprintf(toSend.packetData, "<m:%d;v:%f>",  mode, voltage);
		bt_send_str(toSend.packetData);
}
