#include "peer_net.h"
#include "lib/rudp/const.h"

namespace rudp
{
    std::string
    PeerStateAsString(RUdpPeerState state)
    {
        if (state == RUdpPeerState::CONNECTING)
            return "connecting";

        if (state == RUdpPeerState::ACKNOWLEDGING_CONNECT)
            return "acknowledging_connect";

        if (state == RUdpPeerState::CONNECTION_PENDING)
            return "connection_pending";

        if (state == RUdpPeerState::CONNECTION_SUCCEEDED)
            return "connection_succeeded";

        if (state == RUdpPeerState::CONNECTED)
            return "connected";

        if (state == RUdpPeerState::DISCONNECT_LATER)
            return "disconnect_later";

        if (state == RUdpPeerState::DISCONNECTING)
            return "disconnecting";

        if (state == RUdpPeerState::ACKNOWLEDGING_DISCONNECT)
            return "acknowledging_disconnect";

        if (state == RUdpPeerState::ZOMBIE)
            return "zombie";

        return "disconnected";
    }

    PeerNet::PeerNet()
        : state_(RUdpPeerState::DISCONNECTED)
        , incoming_bandwidth_()
        , incoming_bandwidth_throttle_epoch_()
        , last_send_time_()
        , mtu_(HOST_DEFAULT_MTU)
        , outgoing_bandwidth_()
        , outgoing_bandwidth_throttle_epoch_()
        , segment_throttle_(PEER_DEFAULT_SEGMENT_THROTTLE)
        , segment_throttle_acceleration_(PEER_SEGMENT_THROTTLE_ACCELERATION)
        , segment_throttle_counter_()
        , segment_throttle_deceleration_(PEER_SEGMENT_THROTTLE_DECELERATION)
        , segment_throttle_epoch_()
        , segment_throttle_interval_(PEER_SEGMENT_THROTTLE_INTERVAL)
        , segment_throttle_limit_(PEER_SEGMENT_THROTTLE_SCALE)
        , segment_loss_()
        , segment_loss_epoch_()
        , segment_loss_variance_()
        , segments_lost_()
        , segments_sent_()
        , window_size_(PROTOCOL_MAXIMUM_WINDOW_SIZE)
    {
    }

    void
    PeerNet::CalculateSegmentLoss(uint32_t service_time)
    {
        uint32_t segment_loss = segments_lost_ * PEER_SEGMENT_LOSS_SCALE / segments_sent_;

        segment_loss_variance_ -= segment_loss_variance_ / 4;

        if (segment_loss >= segment_loss_) {
            segment_loss_ += (segment_loss - segment_loss_) / 8;
            segment_loss_variance_ += (segment_loss - segment_loss_) / 4;
        }
        else {
            segment_loss_ -= (segment_loss_ - segment_loss) / 8;
            segment_loss_variance_ += (segment_loss_ - segment_loss) / 4;
        }

        segment_loss_epoch_ = service_time;
        segments_sent_      = 0;
        segments_lost_      = 0;
    }

    void
    PeerNet::Reset()
    {
        state_                             = RUdpPeerState::DISCONNECTED;
        incoming_bandwidth_                = 0;
        incoming_bandwidth_throttle_epoch_ = 0;
        last_send_time_                    = 0;
        mtu_                               = HOST_DEFAULT_MTU;
        outgoing_bandwidth_                = 0;
        outgoing_bandwidth_throttle_epoch_ = 0;
        segment_throttle_                  = PEER_DEFAULT_SEGMENT_THROTTLE;
        segment_throttle_acceleration_     = PEER_SEGMENT_THROTTLE_ACCELERATION;
        segment_throttle_counter_          = 0;
        segment_throttle_deceleration_     = PEER_SEGMENT_THROTTLE_DECELERATION;
        segment_throttle_epoch_            = 0;
        segment_throttle_interval_         = PEER_SEGMENT_THROTTLE_INTERVAL;
        segment_throttle_limit_            = PEER_SEGMENT_THROTTLE_SCALE;
        segment_loss_                      = 0;
        segment_loss_epoch_                = 0;
        segment_loss_variance_             = 0;
        segments_lost_                     = 0;
        segments_sent_                     = 0;
        window_size_                       = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    }

    void
    PeerNet::Setup()
    {
        state_ = RUdpPeerState::CONNECTING;

        if (outgoing_bandwidth_ == 0)
            window_size_ = PROTOCOL_MAXIMUM_WINDOW_SIZE;
        else
            window_size_ = (outgoing_bandwidth_ / PEER_WINDOW_SIZE_SCALE) * PROTOCOL_MINIMUM_WINDOW_SIZE;

        if (window_size_ < PROTOCOL_MINIMUM_WINDOW_SIZE)
            window_size_ = PROTOCOL_MINIMUM_WINDOW_SIZE;

        if (window_size_ > PROTOCOL_MAXIMUM_WINDOW_SIZE)
            window_size_ = PROTOCOL_MAXIMUM_WINDOW_SIZE;
    }

    void
    PeerNet::UpdateSegmentThrottleCounter()
    {
        segment_throttle_counter_ += PEER_SEGMENT_THROTTLE_COUNTER;
        segment_throttle_counter_ %= PEER_SEGMENT_THROTTLE_SCALE;
    }
} // namespace rudp
