/*
  ===========================================
       Copyright (c) 2017 Stefan Kremser
              github.com/spacehuhn
  ===========================================
*/

/* include all necessary libraries */
// #include "esp_system.h"
// #include "esp_event.h"

#include "esp_bt_defs.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"

#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include <Esp.h>
#include <PCAP.h>

//===== Run-Time variables =====//
PCAP pcap;

int ch = CHANNEL;
unsigned long lastChannelChange = 0;

void bt_notify_host_send_available(void) { }

//===== FUNCTIONS =====//
int bt_notify_host_recv(uint8_t *data, uint16_t len)
{
  long timestamp = (micros() - millis()) / 1000;                  // current timestamp
  long microseconds = (unsigned int)(micros() - millis() * 1000); // micro seconds offset (0 - 999)

  pcap.sendPacket({ timestamp, microseconds }, data, len);
}

esp_vhci_host_callback_t cbs =
{
  .notify_host_send_available = bt_notify_host_send_available,
  .notify_host_recv = bt_notify_host_recv,
};

//===== SETUP =====//
void setup()
{
  /* start Serial */
  Serial.begin(BAUD_RATE);
  delay(2000);

  Serial.println("<<START>>");
  pcap.start(&Serial, PCAP::LinkType::BLUETOOTH_HCI_H4);

  /* setup */
  nvs_flash_init();
  tcpip_adapter_init();

  esp_bt_controller_config_t cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_bt_controller_init(&cfg) );
  esp_vhci_host_register_callback(&cbs);
  esp_bt_controller_enable(ESP_BT_MODE_BTDM);
  // ESP_ERROR_CHECK( esp_bt_controller_mem_release(WIFI_STORAGE_RAM) );
  // ESP_ERROR_CHECK( esp_bt_(WIFI_MODE_AP) );
  // esp_wifi_set_promiscuous(true);
  // esp_wifi_set_promiscuous_rx_cb(sniffer);

  // wifi_second_chan_t secondCh = (wifi_second_chan_t)NULL;

  // esp_wifi_set_channel(ch,secondCh);
}

void loop()
{
#ifdef CHANNEL_HOPPING
  unsigned long currentTime = millis();
  if (currentTime - lastChannelChange >= HOP_INTERVAL)
  {
    lastChannelChange = currentTime;
    ch++; // increase channel
    if (ch > MAX_CHANNEL)
      ch = 1;
    // esp_gap_ble_set_channels(ch, secondCh);
  }
#endif
}