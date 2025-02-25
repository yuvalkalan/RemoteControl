#include "socket.h"

#define set_flags(ack, syn) ((ack ? (1 << TCP_FLAG_ACK) : 0) + (syn ? (1 << TCP_FLAG_SYN) : 0))
#define is_ack_flag(flags) (flags & (1 << TCP_FLAG_ACK))
#define is_syn_flag(flags) (flags & (1 << TCP_FLAG_SYN))
// time macro
#define millis() to_ms_since_boot(get_absolute_time()) // millis function from arduino

bool Socket::is_valid_ack(const TCPPacket &packet)
{
    return m_seq == packet.header.ack;
}

void Socket::send_packet(const TCPPacket &packet)
{
    /**
     * this function provide raw packet sending
     */
    // printf("write buffer with %d bytes...\n", packet.header.length + 1);
    // for (size_t i = 0; i < packet.header.length + 1; i++)
    // {
    //     printf("0x%x ", ((uint8_t *)&packet)[i]);
    // }
    // printf("\n");
    write_burst(CC1101_TXFIFO_BURST, (uint8_t *)&packet, packet.header.length + 1); // writes TX_Buffer +1 because of pktlen must be also transfered
    transmit_workmode();                                                            // sent package over the air
    receive_workmode();                                                             // set CC1101 in receive mode
}
bool Socket::receive_packet(TCPPacket &packet)
{
    /**
     * this function provide raw packet receiving
     */
    // =========== read packet from cc1101 module ===========
    uint8_t bytes_in_RXFIFO = read_single_byte(CC1101_RXBYTES); // reads the number of bytes in RXFIFO
    if ((bytes_in_RXFIFO & 0x7F) && !(bytes_in_RXFIFO & 0x80))  // if bytes in buffer and no RX Overflow
    {
        read_burst((uint8_t *)&packet, CC1101_RXFIFO_BURST, bytes_in_RXFIFO);
        packet.rssi = rssi_convert(packet.rssi); // converts receiver strength to dBm
        packet.lqi = lqi_convert(packet.lqi);    // get rf quialtiy indicator
    }
    else
    {
        // printf("did not receive data!\n");
        idle_workmode(); // set to IDLE
        flush_rx();      // flush RX Buffer
        sleep_us(100);
        receive_workmode(); // set to receive mode
        return false;       // if no data, exit
    }
    // ======================================================
    return true;
}

bool Socket::wait_for_ack(const TCPPacket &tx_packet)
{
    bool got_packet = false;
    bool timeout = false;
    auto start_time = millis();
    TCPPacket packet;
    while (!got_packet && !timeout)
    {
        while (!packet_available() && !timeout) // wait until receive packet
        {
            // sleep_ms(1);
            timeout = millis() - start_time > TCP_TIMEOUT;
        }
        if (timeout)
        {
            printf("ACK: timeout!\n");
            continue;
        }
        if (!receive_packet(packet)) // if not received packet
        {
            printf("ACK: could not read packet!\n");
            continue;
        }
        if (!is_ack_flag(packet.header.flags)) // if not ack response
        {
            printf("ACK: not ack packet!\n");
            continue;
        }
        if (packet.header.src_address != m_tunnel_address) // if not receive data from destination
        {
            printf("ACK: packet not from tunnel address! tunnel address: %d, src address: %d\n", m_tunnel_address, packet.header.src_address);
            continue;
        }
        if (packet.header.dst_address != m_address) // if dst address is not my address
        {
            printf("ACK: packet not to my address! my address: %d, src address: %d\n", m_address, packet.header.src_address);
            continue;
        }
        if (!is_valid_ack(packet)) // check packet ack
        {
            printf("ACK: invalid packet ack! should be %d, got %d\n", m_seq, packet.header.ack);
            continue;
        }
        got_packet = true;
    }
    tx_packet.rssi = packet.rssi;
    tx_packet.lqi = packet.lqi;
    return got_packet;
}

void Socket::update_seq(const TCPPacket &packet)
{
    // update seq number depending on the packet type
    if (is_syn_flag(packet.header.flags)) // if syn, update by one
        m_seq++;
    else
    {
        if (!is_ack_flag(packet.header.flags)) // if ack, dont update seq
        {
            m_seq += packet.header.length; // otherwise, update by length
        }
    }
}

void Socket::send_ack()
{
    TCPPacket ack_packet;                 // setup tcp ack packet
    wrap_packet(ack_packet, true, false); // generate packet
    // printf("sending ack...\n");
    send_packet(ack_packet); // send ack packet
    update_seq(ack_packet);
    // printf("ack sent!\n");
}

void Socket::wrap_packet(const TCPPacket &packet, bool ack, bool syn)
{
    packet.header.src_address = m_address;
    packet.header.dst_address = m_tunnel_address;
    packet.header.seq = m_seq;
    packet.header.ack = m_ack;
    packet.header.flags = set_flags(ack, syn); // set flags
}

bool Socket::is_connected()
{
    return m_connected;
}

Socket::Socket(uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address, uint8_t retries)
    : CC1101(freq, mode, channel, address), m_seq(0), m_ack(0), m_tunnel_address(0), m_tries(retries), m_connected(false)
{
}
bool Socket::connect(uint8_t dst_address)
{
    // reset counters
    m_seq = 0;
    m_ack = 0;
    m_connected = false;
    m_tunnel_address = dst_address; // set tunnel address to destination address
    // ========== SYN ==========

    // setup tcp syn packet
    TCPPacket syn_packet;
    wrap_packet(syn_packet, false, true); // generate packet

    printf("sending syn...\n");
    send_packet(syn_packet); // send syn packet
    update_seq(syn_packet);
    // printf("packet sent!\n");

    // ========== SYN + ACK ==========
    bool receive_syn_ack = false;
    bool timeout = false;
    auto start_time = millis();
    TCPPacket syn_ack_packet;
    while (!receive_syn_ack && !timeout)
    {
        while (!packet_available() && !timeout) // wait until receive packet
        {
            // sleep_ms(1);
            timeout = millis() - start_time > TCP_TIMEOUT;
        }
        if (timeout)
            continue;
        if (!receive_packet(syn_ack_packet)) // if not received packet
            continue;
        // printf("got packet!\n");
        if (!(is_ack_flag(syn_ack_packet.header.flags) && is_syn_flag(syn_ack_packet.header.flags))) // if not syn + ack response
            continue;
        if (syn_ack_packet.header.src_address != m_tunnel_address) // if not receive data from destination
            continue;
        if (!is_valid_ack(syn_ack_packet)) // check packet ack
            continue;
        receive_syn_ack = true;
    }
    if (timeout) // timeout - did not receive data
        return false;
    printf("receive syn + ack!\n");
    m_ack = syn_ack_packet.header.seq + 1; // set ack

    // ========== ACK ==========
    send_ack();
    m_connected = true;
    printf("connected!\n");
    return true;
}
bool Socket::accept()
{
    // reset counters
    m_seq = 0;
    m_ack = 0;
    m_connected = false;
    // ========== SYN ==========
    bool receive_syn = false;
    bool timeout = false;
    auto start_time = millis();
    TCPPacket syn_packet;
    while (!receive_syn && !timeout)
    {
        while (!packet_available() && !timeout) // wait until receive packet
        {
            // sleep_ms(1);
            timeout = millis() - start_time > TCP_TIMEOUT;
        }
        if (!receive_packet(syn_packet)) // if not received packet
            continue;
        // printf("got packet!\n");

        if (!is_syn_flag(syn_packet.header.flags)) // if not syn packet
            continue;
        receive_syn = true;
    }
    if (timeout) // timeout - did not receive data
        return false;
    m_ack = syn_packet.header.seq + 1;                // set ack
    m_tunnel_address = syn_packet.header.src_address; // set tunnel address
    printf("got syn packet!\n");
    // ========== SYN + ACK ==========
    TCPPacket syn_ack_packet;
    wrap_packet(syn_ack_packet, true, true); // generate packet
    printf("sending syn ack...\n");
    send_packet(syn_ack_packet); // send syn packet
    update_seq(syn_ack_packet);
    // printf("packet sent!\n");
    // ========== ACK ==========
    if (!wait_for_ack(syn_ack_packet))
        return false;
    printf("connected!\n");
    m_connected = true;
    return true;
}

bool Socket::send(const TCPPacket &packet)
{
    wrap_packet(packet); // create header
    bool got_ack = false;
    uint8_t tries = m_tries + 1;
    auto start_seq = m_seq;
    while (--tries && !got_ack)
    {
        send_packet(packet);
        printf("#%d: data sent! seq=%d, ack=%d\n", m_tries - tries + 1, m_seq, m_ack);
        update_seq(packet);
        got_ack = wait_for_ack(packet);
        if (got_ack)
            printf("got ack!\n");
        else
            printf("did not got ack!\n");
        m_seq = start_seq;
    }
    if (!tries)
        return false;

    update_seq(packet);
    return true;
}
bool Socket::receive(TCPPacket &packet)
{
    bool got_packet = false;
    bool timeout = false;
    auto start_time = millis();
    while (!got_packet && !timeout)
    {
        while (!packet_available() && !timeout) // wait until receive packet
        {
            // sleep_ms(1);
            timeout = millis() - start_time > TCP_TIMEOUT;
        }
        if (timeout)
        {
            printf("timeout!\n");
            continue;
        }
        if (!receive_packet(packet)) // if not received packet
        {
            printf("could not read packet!\n");
            continue;
        }
        if (is_ack_flag(packet.header.flags) || is_syn_flag(packet.header.flags)) // if syn or ack response
        {
            printf("packet with flags!\n");
            continue;
        }
        if (packet.header.src_address != m_tunnel_address) // if not receive data from destination
        {
            printf("packet not from tunnel address! tunnel address: %d, src address: %d\n", m_tunnel_address, packet.header.src_address);
            continue;
        }
        if (packet.header.dst_address != m_address) // if dst address is not my address
        {
            printf("packet not to my address! my address: %d, src address: %d\n", m_address, packet.header.src_address);
            continue;
        }
        if (!is_valid_ack(packet)) // check packet ack
        {
            printf("invalid packet ack!\n");
            continue;
        }
        got_packet = true;
    }
    if (timeout) // timeout - did not receive data
        return false;
    printf("got packet! ");
    printf("length: %d ", packet.header.length);
    printf("dst_address: %d ", packet.header.dst_address);
    printf("src_address: %d ", packet.header.src_address);
    printf("flags: %d ", packet.header.flags);
    printf("seq: %d ", packet.header.seq);
    printf("ack: %d\n", packet.header.ack);
    m_ack = packet.header.seq + packet.header.length; // update ack
    send_ack();
    return true;
}
