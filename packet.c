#include "packet.h"

void sendPacket(int mode, float voltage, int range, int extra) {
    char packetData[PACKET_SIZE];
    sprintf(packetData, "<m:%d;v:%f;r:%d;e:%d>", mode, voltage, range, extra);

#ifdef PACKET_DEBUG
    printf("[Android Client] Data Sending: %s\r\n", packetData);
#endif

    bt_send_str(packetData);
}

