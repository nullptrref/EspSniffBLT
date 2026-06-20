#pragma once
#include "esp32-hal-log.h"
#include "HCIPacket.h"

void hexDump(void *data, size_t len, int maxCharPerRow = 16);
void printHCIPacketInfo(HCIPacket *p, size_t len);