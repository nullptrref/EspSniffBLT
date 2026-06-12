#include "PCAP.h"

template<typename T>
void PCAP::io_write(const Bytes<T> &b) { mIO.write(b.bytes, b.sizeT); }

/* send file header */
esp_err_t PCAP::start(Stream *io, LinkType linktype)
{
  mIO = io;

  if (!mIO) return ESP_FAIL;
  
  io_write<uint32_t>(magic_number);
  io_write<uint16_t>(version_major);
  io_write<uint16_t>(version_minor);
  io_write<uint32_t>(0);
  io_write<uint32_t>(0);
  io_write<uint32_t>(snaplen);
  io_write<int>(static_cast<int>(linktype));
  return ESP_OK;
}

esp_err_t PCAP::flush()
{
  if (!mIO) return ESP_FAIL;
  mIO->flush();
  return ESP_OK;
}

esp_err_t PCAP::end()
{
  mIO = nullptr;
  return ESP_OK;
}

/* write packet */
void PCAP::sendPacket(timeval timestamp, uint8_t *buf, uint32_t len)
{
  if (!mIO || !buf || !len) return;

  uint32_t incl_len = len;

#if defined(ESP32)
  /* safty check that the packet isn't too big (I ran into problems with the ESP8266 here) */
  if (incl_len > snaplen)
    incl_len = snaplen;
#endif

  io_write<uint32_t>(timestamp.tv_sec);
  io_write<uint32_t>(timestamp.tv_usec);

  io_write<uint32_t>(incl_len); // Captured Packet Length
  io_write<uint32_t>(len);      // Original Packet Length

  mIO->write(buf, incl_len);
}