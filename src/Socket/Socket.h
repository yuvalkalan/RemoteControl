#pragma once
#include "CC1101/CC1101.h"
#include <algorithm>

#define TCP_PACKET_SIZE 62 // total size of packet

#define TCP_FLAG_ACK 1        // ack flag bit
#define TCP_FLAG_SYN 0        // syn flag bit
#define TCP_TIMEOUT 50        // packet ack timeout
#define TCP_DEFAULT_RETRIES 5 // default sending retries

struct TCPHeader
{
    uint8_t length = 0; // must be first in order to send properly
    uint8_t dst_address = 0;
    uint8_t src_address = 0;
    uint8_t flags = 0;
    uint16_t seq = 0;
    uint16_t ack = 0;
};

struct TCPPacket
{ // do not change the order of the variables
    mutable TCPHeader header;
    uint8_t body[TCP_PACKET_SIZE - sizeof(TCPHeader)] = {};
    mutable int8_t rssi = 0; // packet length + 1
    mutable uint8_t lqi = 0; // packet length + 2

    TCPPacket()
    {
        header.length = TCP_PACKET_SIZE - 1;
    }
};

class Socket : public CC1101
{
public: // TODO: make private
    uint16_t m_seq;
    uint16_t m_ack;
    uint8_t m_tunnel_address;
    uint8_t m_tries;
    bool m_connected;

private: // raw trans-receive functions
    void send_packet(const TCPPacket &packet);
    bool receive_packet(TCPPacket &packet);

private: // ack functions
    bool is_valid_ack(const TCPPacket &packet);
    bool wait_for_ack(const TCPPacket &tx_packet);
    void update_seq(const TCPPacket &packet);
    void send_ack();

private:
    void wrap_packet(const TCPPacket &packet, bool ack = false, bool syn = false);

public: // getters
    bool is_connected();

public:
    Socket(uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address, uint8_t retries = TCP_DEFAULT_RETRIES);
    bool connect(uint8_t dst_address);
    bool accept();
    bool send(const TCPPacket &packet);
    bool receive(TCPPacket &packet);
};
