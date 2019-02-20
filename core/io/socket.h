#ifndef P2P_TECHDEMO_CORE_IO_SOCKET_H
#define P2P_TECHDEMO_CORE_IO_SOCKET_H

#include <memory>

#include "core/base/status.h"
#include "ip.h"

class Socket
{
public:
    enum class PollType
    {
        POLL_TYPE_IN,
        POLL_TYPE_OUT,
        POLL_TYPE_IN_OUT
    };

    enum class Type
    {
        TYPE_NONE,
        TYPE_TCP,
        TYPE_UDP
    };

    virtual Status open(Type p_type, IP::Type ip_type) = 0;

    virtual void close() = 0;

    virtual Status bind(IpAddress &ip, uint16_t port) = 0;

    virtual Status listen(int max_pending) = 0;

    virtual Status connect_to_host(IpAddress &ip, uint16_t port) = 0;

    virtual Status poll(PollType type, int timeout) = 0;

    virtual Status recv(uint8_t &buffer, int len, int &read_byte_count) = 0;

    virtual Status recvfrom(uint8_t &buffer, int len, int &read_byte_count) = 0;

    virtual Status send(const uint8_t &buffer, int len, int send_byte_count) = 0;

    virtual Status sendto(const uint8_t &buffer, int len, int send_byte_count) = 0;

    virtual std::shared_ptr<Socket> accept(IpAddress &ip, uint16_t port) = 0;

    virtual bool is_open() const = 0;

    virtual int get_available_bytes() const = 0;

    virtual void set_broadcasting_enabled(bool enabled) = 0;

    virtual void set_blocking_enabled(bool enabled) = 0;

    virtual void set_tcp_no_delay_enabled(bool enabled) = 0;

    virtual void set_reuse_address_enabled(bool enabled) = 0;
};

#endif // P2P_TECHDEMO_CORE_IO_SOCKET_H
