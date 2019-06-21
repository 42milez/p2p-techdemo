#ifndef P2P_TECHDEMO_RUDPHOST_H
#define P2P_TECHDEMO_RUDPHOST_H

#include <vector>

#include "RUdpAddress.h"
#include "RUdpConnection.h"
#include "RUdpPeerPod.h"

class RUdpHost
{
private:
    std::vector<uint8_t> _received_data;

    std::vector<std::vector<uint8_t>> _segment_data;

    std::unique_ptr<RUdpAddress> _received_address;

    size_t _received_data_length;

    size_t _duplicate_peers;

    size_t _maximum_segment_size;

    size_t _maximum_waiting_data;

    SysCh _channel_count;

    uint32_t _incoming_bandwidth;

    uint32_t _outgoing_bandwidth;

    uint32_t _mtu;

    uint32_t _service_time;

    std::unique_ptr<RUdpPeerPod> _peer_pod;

    std::shared_ptr<RUdpConnection> _conn;

public:
    RUdpHost(const RUdpAddress &address, SysCh channel_count, size_t peer_count, uint32_t in_bandwidth,
             uint32_t out_bandwidth);

    Error
    udp_host_connect(const RUdpAddress &address, SysCh channel_count, uint32_t data);

    int
    udp_host_service(std::unique_ptr<RUdpEvent> &event, uint32_t timeout);

    uint32_t service_time();
};

#endif // P2P_TECHDEMO_RUDPHOST_H
