#include "Utils.h"

void hexDump(void *data, size_t len, int maxCharPerRow)
{
    log_d("len = %d", len);
    log_printf("| %04X | ", 0);
    for (int i = 0; i < len; i++)
    {
        log_printf("%02X ", ((uint8_t *)data)[i]);
        if (i && i % maxCharPerRow == 0)
            log_printf("\n| %04X | ", i);
    }
    log_printf("\n");
}

void printHCIPacketInfo(HCIPacket *p, size_t len)
{
    log_d("Type: %d", p->type);
    switch (p->type)
    {
    case HCIPacketType::HCI_Command_Packet:
    {
        log_d("HCI_Command_Packet");
        log_d("opcode: %04X", p->CommandPacket.opcode);
        log_d("groupField: %d", p->CommandPacket.groupField);
        log_d("commandField: %d", p->CommandPacket.commandField);
        log_d("len: %d", p->CommandPacket.len);
        hexDump(p->CommandPacket.data, p->CommandPacket.len);
        break;
    }
    case HCIPacketType::HCI_Asynchronous_Data_Packet:
    {
        log_d("HCI_Asynchronous_Data_Packet");
        log_d("ConnectionHandle: %d", p->ACLDataPacket.connectionHandle);
        log_d("PacketBoundaryFlags: %02X", p->ACLDataPacket.packetBoundaryFlags);
        log_d("BroadcastFlags: %02X", p->ACLDataPacket.broadcastFlags);
        log_d("len: %d", p->ACLDataPacket.len);
        hexDump(p->ACLDataPacket.data, p->ACLDataPacket.len);
        break;
    }
    case HCIPacketType::HCI_Synchronous_Data_Packet:
    {
        log_d("HCI_Synchronous_Data_Packet");
        log_d("ConnectionHandle: %d", p->SCODataPacket.connectionHandle);
        log_d("PacketStatusFlags: %02X", p->SCODataPacket.packetStatusFlags);
        log_d("len: %d", p->SCODataPacket.len);
        hexDump(p->SCODataPacket.data, p->SCODataPacket.len);
        break;
    }
    case HCIPacketType::HCI_Event_Packet:
    {
        log_d("HCI_Event_Packet");
        log_d("EventCode: %d", p->EventPacket.eventCode);
        log_d("len: %d", p->EventPacket.len);
        hexDump(p->EventPacket.data, p->EventPacket.len);
        // Num_HCI_Command_Packets, Command_Opcode, Return_Parameters
        PACKED_STRUCT cmd_cmplt
        {
            uint8_t Num_HCI_Command_Packets;
            uint16_t Command_Opcode;
            uint8_t Return_Parameters;
        };
        break;
    }
    
    default:
        log_d("Unknown HCI_PACKET_TYPE: %d", p->type);
        hexDump(p, len);
    }
}