#ifndef P2P_TECHDEMO_MODULE_TRANSPORTER_H
#define P2P_TECHDEMO_MODULE_TRANSPORTER_H

#include <list>
#include <map>
#include <vector>

#include "core/io/ip_address.h"
#include "core/errors.h"

#include "lib/rudp/RUdpBuffer.h"
#include "lib/rudp/RUdpCompressor.h"
#include "lib/rudp/RUdpEvent.h"
#include "lib/rudp/RUdpHost.h"
#include "lib/rudp/RUdpSegment.h"
#include "lib/rudp/RUdpPeer.h"

class Transporter
{
public:
    Transporter();

    ~Transporter();

    Error create_client(const std::string &address, int port, int in_bandwidth = 0, int out_bandwidth = 0,
                        int client_port = 0);

    Error create_server(uint16_t port, size_t peer_count = 32, uint32_t in_bandwidth = 0, uint32_t out_bandwidth = 0);

    void poll();

    void close_connection(uint32_t wait_usec = 100);

    void disconnect(int peer, bool now = false);

    Error get_packet(const uint8_t **buffer, int &buffer_size);

    Error put_packet(const uint8_t *buffer, int buffer_size);

private:
    enum class CompressionMode: int
    {
        NONE,
        RANGE_CODER,
        FASTLZ,
        ZLIB,
        ZSTD
    };

    enum class ConnectionStatus: int
    {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

    struct Packet
    {
        RUdpSegment *packet;
        int from;
        int channel;
    };

    enum class SysMsg: int
    {
        ADD_PEER,
        REMOVE_PEER
    };

    enum class TargetPeer: int
    {
        BROADCAST,
        SERVER
    };

    enum class TransferMode: int
    {
        UNRELIABLE,
        UNRELIABLE_ORDERED,
        RELIABLE
    };

    std::shared_ptr<RUdpCompressor> _compressor;
    std::shared_ptr<RUdpHost> _host;

    std::list<Packet> _incoming_packets;
    std::map<int, RUdpPeer *> _peer_map;
    std::vector<uint8_t> _dst_compressor_mem;
    std::vector<uint8_t> _src_compressor_mem;

    CompressionMode _compression_mode;

    ConnectionStatus _connection_status;

    IpAddress _bind_ip;

    Packet _current_packet;

    RUdpEvent _event;

    RUdpPeer *_peer;

    SysCh _channel_count;

    TransferMode _transfer_mode;

    uint32_t _unique_id;

    int _target_peer;

    int _transfer_channel;

    bool _active;

    bool _always_ordered;

    bool _refuse_connections;

    bool _server;

private:
    size_t _udp_compress(const std::vector<RUdpBuffer> &in_buffers,
                         size_t in_limit,
                         std::vector<uint8_t> &out_data,
                         size_t out_limit);

    size_t _udp_decompress(std::vector<uint8_t> &in_data,
                           size_t in_limit,
                           std::vector<uint8_t> &out_data,
                           size_t out_limit);

    void _udp_destroy();

    void _pop_current_packet();

    void _setup_compressor();
};

#endif // P2P_TECHDEMO_MODULE_TRANSPORTER_H
