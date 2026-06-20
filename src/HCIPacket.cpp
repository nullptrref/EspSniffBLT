#include "HCIPacket.h"
#include "Utils.h"
#include "esp32-hal.h"

static uint8_t sBuf[sizeof(HCIPacket)+HCI_BUFFER_CAP] = {0};
static HCIPacket *sHCI = (HCIPacket *)sBuf;

void HCIPacket_send(HCIPacket *p, size_t sz)
{
    hexDump(p, sz);
    esp_vhci_host_send_packet((uint8_t *)p, sz);
    delay(20);
}


void HCIPacket_send_Command(int commandField, int groupField, void *data, size_t len)
{
    sHCI->type = HCI_Command_Packet;
    sHCI->CommandPacket.opcode = _MK_OPCODE(commandField, groupField);
    sHCI->CommandPacket.len = len;
    if (len) xthal_memcpy(sHCI->CommandPacket.data, data, len);
    HCIPacket_send(sHCI, 4 + len);
}

void HCIPacket_send_Event(int eventCode, void *data, size_t len)
{
    sHCI->type = HCI_Event_Packet;
    sHCI->EventPacket.eventCode = eventCode;
    sHCI->EventPacket.len = len;
    if (len) xthal_memcpy(sHCI->EventPacket.data, data, len);
    HCIPacket_send(sHCI, 4 + len);
}

void HCIPacket_send_AsyncData(int connectionHandle, int packetBoundaryFlags, int broadcastFlags, void *data, size_t len)
{
    sHCI->type = HCI_Asynchronous_Data_Packet;
    sHCI->ACLDataPacket.connectionHandle = connectionHandle;
    sHCI->ACLDataPacket.packetBoundaryFlags = packetBoundaryFlags;
    sHCI->ACLDataPacket.broadcastFlags = broadcastFlags;
    sHCI->ACLDataPacket.len = len;
    if (len) xthal_memcpy(sHCI->ACLDataPacket.data, data, len);
    HCIPacket_send(sHCI, 5 + len);
}

void HCIPacket_send_SyncData(int connectionHandle, int packetStatusFlags, void *data, size_t len)
{
    sHCI->type = HCI_Synchronous_Data_Packet;
    sHCI->SCODataPacket.connectionHandle = connectionHandle;
    sHCI->SCODataPacket.packetStatusFlags = packetStatusFlags;
    sHCI->SCODataPacket.len = len;
    if (len) xthal_memcpy(sHCI->SCODataPacket.data, data, len);
    HCIPacket_send(sHCI, 4 + len);
}

void HCIPacket_send_ExtendedCommand(int LSB_opcode, int MSB_opcode, int LSB_len, int MSB_len, void *data, size_t len)
{
    sHCI->type = HCI_Extended_Command;
    sHCI->ExtendedCommandPacket.LSB_opcode = LSB_opcode;
    sHCI->ExtendedCommandPacket.MSB_opcode = MSB_opcode;
    sHCI->ExtendedCommandPacket.LSB_len = LSB_len;
    sHCI->ExtendedCommandPacket.MSB_len = MSB_len;
    if (len) xthal_memcpy(sHCI->ExtendedCommandPacket.data, data, len);
    HCIPacket_send(sHCI, 4 + len);
}