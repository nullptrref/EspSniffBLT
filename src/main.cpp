/*
  ===========================================
       Copyright (c) 2017 Stefan Kremser
              github.com/spacehuhn
  ===========================================
*/

#include "esp_bt_defs.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"

#include "esp32-hal-bt.h"
#include "BLEDevice.h"
#include "esp_bt_main.h"

#include "nvs_flash.h"
#include "driver/gpio.h"

#include <tuple>

#include <Arduino.h>

//! sniffing utils
#include <PCAP.h>
#include <HCIPacket.h>
#include <Utils.h>

static PCAP pcap;

static void bt_notify_host_send_available(void) {}

static int bt_notify_host_recv(uint8_t *data, uint16_t len)
{
    // log_v("[RECV] %u bytes.", len);
    if (false && len >= sizeof(RawHCIPacket))
    {
        auto p = (RawHCIPacket *)data;
        printHCIPacketInfo(p, len);
        return 0;
    }
    long timestamp = (micros() - millis()) / 1000;                  // current timestamp
    long microseconds = (unsigned int)(micros() - millis() * 1000); // micro seconds offset (0 - 999)

    pcap.sendPacket({timestamp, microseconds}, data, len);
    return 0;
}

static esp_vhci_host_callback_t cbs =
    {
        .notify_host_send_available = bt_notify_host_send_available,
        .notify_host_recv = bt_notify_host_recv,
};

//===== SETUP =====//
void setup()
{
    // Initialize NVS (required by BT controller)
    nvs_flash_init();

    /* start Serial */
    Serial.begin(BAUD_RATE);
    delay(200);
    esp_bt_controller_status_t status = esp_bt_controller_get_status();
    log_v("Controller status: 0x%X", status);
    if (status != ESP_BT_CONTROLLER_STATUS_IDLE)
    {
        btStop();
        esp_bluedroid_deinit();
    }

    // Init controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BTDM));
    // Register VHCI callbacks
    ESP_ERROR_CHECK(esp_vhci_host_register_callback(&cbs));

    //! hci_reset
    HCIPacket_send_Command(HCI_HostCtrlr_BasebandCommands::Reset, HCI_OGF_HostControllerandBasebandCommands);
    log_v("Sent HCI Reset packet");

    // TODO
    // HCIPacket HCI_Inquiry = HCIPacket::Command(HCI_LCC_Inquiry, HCI_OGF_LinkControlCommands);
    // HCIPacket_send(HCI_Inquiry);

    // uint8_t hci_set_event_mask[] = {0x01, 0x01, 0x0C, 0x08, 0xFF, 0xF9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F};
    // esp_vhci_host_send_packet(hci_set_event_mask, sizeof(hci_set_event_mask));

    // uint64_t event_mask = 0x3FFFFFFFFFFFF9FF;
    uint64_t event_mask = 0xFFFFFFFF; // all events
    HCIPacket_send_Command(HCI_HostCtrlr_BasebandCommands::Set_Event_Mask, HCI_OGF_HostControllerandBasebandCommands, event_mask);
    log_v("Sent Set_Event_Mask packet");
    
    // uint8_t HCI_Write_Simple_Pairing_Mode[] = {0x01,0x56,0x0C,0x01,0x00};
    // esp_vhci_host_send_packet(HCI_Write_Simple_Pairing_Mode, sizeof(HCI_Write_Simple_Pairing_Mode));
    // log_i("Sent HCI_Write_Simple_Pairing_Mode packet");
    
    // uint8_t HCI_Inquiry[] = {0x01, 0x01, 0x04, 0x05, 0x33, 0x8B, 0x9E, 0x30, 0x00};
    // esp_vhci_host_send_packet(HCI_Inquiry, sizeof(HCI_Inquiry));
    PACKED_STRUCT Inquiry
    {
        uint32_t LAP : 24;
        uint8_t Inquiry_Length;
        uint8_t Num_Responses;
    };
    sizeof(Inquiry);
    Inquiry inq;
    // 0x9E8B00 - 0X9E8B3F
    // General Inquiry Access Code (GIAC)
    inq.LAP = 0x9E8B33; 
    inq.Inquiry_Length = 0x30; // max timeout
    inq.Num_Responses = 0x00; // Unlimited
    HCIPacket_send_Command(HCI_LinkControlCommands::HCI_LCC_Inquiry, HCI_OGF_LinkControlCommands, inq);
    log_i("Sent HCI_Inquiry packet");

    union Inquiry_Scan_Activity
    {
        uint32_t param;
        struct
        {
            uint16_t Scan_Interval;
            uint16_t Scan_Window;
        };
    };
    Inquiry_Scan_Activity isa;
    isa.Scan_Interval = 0x1000; // max: 2.56 sec
    isa.Scan_Window = 0x0012; // deafult
    HCIPacket_send_Command(HCI_HostCtrlr_BasebandCommands::Write_Inquiry_Scan_Activity, HCI_OGF_HostControllerandBasebandCommands, isa);
    log_i("Sent Write_Inquiry_Scan_Activity packet");

    // uint8_t LE_Set_Scan_Params[] = {0x01, 0x0B, 0x20, 0x07, 0x01, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00};
    // esp_vhci_host_send_packet(LE_Set_Scan_Params, sizeof(LE_Set_Scan_Params));

    // HCIPacket_send_Command(HCI_HostCtrlr_BasebandCommands::Write_Authentication_Enable, HCI_OGF_PolicyCommand, true);
    // log_v("Sent Write_Authentication_Enable packet");

    // uint8_t LE_Set_Scan_Enable[] = {0x01, 0x0C, 0x20, 0x02, 0x01, 0x00};
    // esp_vhci_host_send_packet(LE_Set_Scan_Enable, sizeof(LE_Set_Scan_Enable));
    // HCIPacket_send_Command(HCI_HostCtrlr_BasebandCommands::Write_Scan_Enable, HCI_OGF_HostControllerandBasebandCommands, true);
    // log_v("Sent Write_Scan_Enable packet");

    //! Serial specific
    Serial.println("\n<<START>>");
    Serial.flush();

    pcap.start(&Serial, PCAP::LinkType::BLUETOOTH_HCI_H4);
}

void loop()
{
}