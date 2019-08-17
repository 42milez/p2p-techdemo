#include "RUdpChannel.h"
#include "RUdpCommandSize.h"
#include "RUdpCommon.h"
#include "RUdpProtocol.h"

RUdpProtocol::RUdpProtocol()
    : bandwidth_throttle_epoch_(),
      chamber_(std::make_unique<RUdpChamber>()),
      dispatch_hub_(std::make_unique<RUdpDispatchHub>())
{}

#define IS_PEER_NOT_CONNECTED(peer) \
    !peer->StateIs(RUdpPeerState::CONNECTED) && peer->StateIs(RUdpPeerState::DISCONNECT_LATER)

void
RUdpProtocol::BandwidthThrottle(uint32_t service_time, uint32_t incoming_bandwidth, uint32_t outgoing_bandwidth,
                                const std::vector<std::shared_ptr<RUdpPeer>> &peers)
{
    if (UDP_TIME_DIFFERENCE(service_time, bandwidth_throttle_epoch_) >= HOST_BANDWIDTH_THROTTLE_INTERVAL) {
        auto time_current = TimeGet();
        auto time_elapsed = time_current - bandwidth_throttle_epoch_;
        auto peers_remaining = dispatch_hub_->connected_peers();
        auto data_total = ~0u;
        auto bandwidth = ~0u;
        auto throttle = 0;
        auto bandwidth_limit = 0;
        auto needs_adjustment = dispatch_hub_->bandwidth_limited_peers() > 0 ? true : false;

        if (time_elapsed < HOST_BANDWIDTH_THROTTLE_INTERVAL)
            return;

        bandwidth_throttle_epoch_ = time_current;

        if (peers_remaining == 0)
            return;

        //  Throttle outgoing bandwidth
        // --------------------------------------------------

        if (outgoing_bandwidth != 0) {
            data_total = 0;
            bandwidth = outgoing_bandwidth * (time_elapsed / 1000);

            for (const auto &peer : peers) {
                if (IS_PEER_NOT_CONNECTED(peer))
                    continue;

                data_total += peer->outgoing_data_total();
            }
        }

        //  Throttle peer bandwidth : Case A ( adjustment is needed )
        // --------------------------------------------------

        while (peers_remaining > 0 && needs_adjustment) {
            needs_adjustment = false;

            if (data_total <= bandwidth)
                throttle = PEER_SEGMENT_THROTTLE_SCALE;
            else
                throttle = (bandwidth * PEER_SEGMENT_THROTTLE_SCALE) / data_total;

            for (auto &peer : peers) {
                uint32_t peer_bandwidth;

                if ((IS_PEER_NOT_CONNECTED(peer)) ||
                    peer->incoming_bandwidth() == 0 ||
                    peer->outgoing_bandwidth_throttle_epoch() == time_current) {
                    continue;
                }

                peer_bandwidth = peer->incoming_bandwidth() * (time_elapsed / 1000);
                if ((throttle * peer->outgoing_data_total()) / PEER_SEGMENT_THROTTLE_SCALE <= peer_bandwidth)
                    continue;

                peer->segment_throttle_limit(
                    (peer_bandwidth * PEER_SEGMENT_THROTTLE_SCALE) / peer->outgoing_data_total());

                if (peer->segment_throttle_limit() == 0)
                    peer->segment_throttle_limit(1);

                if (peer->segment_throttle() > peer->segment_throttle_limit())
                    peer->segment_throttle(peer->segment_throttle_limit());

                peer->outgoing_bandwidth_throttle_epoch(time_current);
                peer->incoming_data_total(0);
                peer->outgoing_data_total(0);

                needs_adjustment = true;

                --peers_remaining;

                bandwidth -= peer_bandwidth;
                data_total -= peer_bandwidth;
            }
        }

        //  Throttle peer bandwidth : Case B ( adjustment is NOT needed )
        // --------------------------------------------------

        if (peers_remaining > 0) {
            if (data_total <= bandwidth)
                throttle = PEER_SEGMENT_THROTTLE_SCALE;
            else
                throttle = (bandwidth * PEER_SEGMENT_THROTTLE_SCALE) / data_total;

            for (auto &peer : peers) {
                if ((IS_PEER_NOT_CONNECTED(peer)) || peer->net()->outgoing_bandwidth_throttle_epoch() == time_current)
                    continue;

                peer->net()->segment_throttle_limit(throttle);

                if (peer->net()->segment_throttle() > peer->net()->segment_throttle_limit())
                    peer->net()->segment_throttle(peer->net()->segment_throttle_limit());

                peer->command()->incoming_data_total(0);
                peer->command()->outgoing_data_total(0);
            }
        }

        //  Recalculate Bandwidth Limits
        // --------------------------------------------------

        if (dispatch_hub_->recalculate_bandwidth_limits()) {
            dispatch_hub_->recalculate_bandwidth_limits(false);
            peers_remaining = dispatch_hub_->connected_peers();
            bandwidth = incoming_bandwidth;
            needs_adjustment = true;

            if (bandwidth == 0) {
                bandwidth_limit = 0;
            }
            else {
                while (peers_remaining > 0 && needs_adjustment) {
                    needs_adjustment = false;
                    bandwidth_limit = bandwidth / peers_remaining;

                    for (auto &peer: peers) {
                        if ((IS_PEER_NOT_CONNECTED(peer)) ||
                            peer->net()->incoming_bandwidth_throttle_epoch() == time_current)
                            continue;

                        if (peer->net()->outgoing_bandwidth() > 0 &&
                            peer->net()->outgoing_bandwidth() >= bandwidth_limit)
                            continue;

                        peer->net()->incoming_bandwidth_throttle_epoch(time_current);

                        needs_adjustment = true;

                        --peers_remaining;

                        bandwidth -= peer->net()->outgoing_bandwidth();
                    }
                }
            }

            std::shared_ptr<RUdpProtocolType> cmd;

            for (auto &peer : peers) {
                if (IS_PEER_NOT_CONNECTED(peer))
                    continue;

                cmd->header.command = PROTOCOL_COMMAND_BANDWIDTH_LIMIT | PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE;
                cmd->header.channel_id = 0xFF;
                cmd->bandwidth_limit.outgoing_bandwidth = htonl(outgoing_bandwidth);

                if (peer->net()->incoming_bandwidth_throttle_epoch() == time_current)
                    cmd->bandwidth_limit.incoming_bandwidth = htonl(peer->net()->outgoing_bandwidth());
                else
                    cmd->bandwidth_limit.incoming_bandwidth = htonl(bandwidth_limit);

                peer->QueueOutgoingCommand(cmd, nullptr, 0, 0);
            }
        }
    }
}

/*
void
RUdpProtocol::Connect(const std::shared_ptr<RUdpPeer> &peer)
{
    if (!peer->net()->StateIs(RUdpPeerState::CONNECTED) && !peer->net()->StateIs(RUdpPeerState::DISCONNECT_LATER)) {
        if (peer->net()->incoming_bandwidth() != 0)
            increase_bandwidth_limited_peers();

        increase_connected_peers();
    }
}

void
RUdpProtocol::Disconnect(const std::shared_ptr<RUdpPeer> &peer)
{
    if (peer->net()->StateIs(RUdpPeerState::CONNECTED) || peer->net()->StateIs(RUdpPeerState::DISCONNECT_LATER)) {
        if (peer->net()->incoming_bandwidth() != 0)
            decrease_bandwidth_limited_peers();

        decrease_connected_peers();
    }
}
*/

void
RUdpProtocol::NotifyDisconnect(std::shared_ptr<RUdpPeer> &peer, const std::unique_ptr<RUdpEvent> &event)
{
    if (peer->StateIsGreaterThanOrEqual(RUdpPeerState::CONNECTION_PENDING))
        // ピアを切断するのでバンド幅を再計算する
        dispatch_hub_->recalculate_bandwidth_limits(true);

    // ピアのステートが以下の３つの内のいずれかである場合
    // 1. DISCONNECTED,
    // 2. ACKNOWLEDGING_CONNECT,
    // 3. CONNECTION_PENDING
    //if (peer->state != RUdpPeerState::CONNECTING && peer->state < RUdpPeerState::CONNECTION_SUCCEEDED)
    if (!peer->StateIs(RUdpPeerState::CONNECTING)
        && peer->StateIsLessThanOrEqual(RUdpPeerState::CONNECTION_SUCCEEDED)) {
        ResetPeer(peer);
    }
        // ピアが接続済みである場合
    else if (event != nullptr) {
        event->type = RUdpEventType::DISCONNECT;
        event->peer = peer;
        event->data = 0;

        ResetPeer(peer);
    }
    else {
        peer->event_data(0);

        dispatch_hub_->DispatchState(peer, RUdpPeerState::ZOMBIE);
    }
}

EventStatus
RUdpProtocol::DispatchIncomingCommands(std::unique_ptr<RUdpEvent> &event)
{
    while (dispatch_hub_->PeerExists()) {
        auto peer = dispatch_hub_->Dequeue();

        peer->needs_dispatch(false);

        if (peer->StateIs(RUdpPeerState::CONNECTION_PENDING) ||
            peer->StateIs(RUdpPeerState::CONNECTION_SUCCEEDED)) {
            // ピアが接続したら接続中ピアのカウンタを増やし、切断したら減らす
            dispatch_hub_->ChangeState(peer, RUdpPeerState::CONNECTED);

            event->type = RUdpEventType::CONNECT;
            event->peer = peer;
            event->data = peer->event_data();

            return EventStatus::AN_EVENT_OCCURRED;
        }
        else if (peer->StateIs(RUdpPeerState::ZOMBIE)) {
            dispatch_hub_->recalculate_bandwidth_limits(true);

            event->type = RUdpEventType::DISCONNECT;
            event->peer = peer;
            event->data = peer->event_data();

            // ゾンビ状態になったピアはリセットする
            ResetPeer(peer);

            return EventStatus::AN_EVENT_OCCURRED;
        }
        else if (peer->StateIs(RUdpPeerState::CONNECTED)) {
            if (!peer->DispatchedCommandExists())
                continue;

            // 接続済みのピアからはコマンドを受信する
            event->segment = peer->Receive(event->channel_id);

            if (event->segment == nullptr)
                continue;

            event->type = RUdpEventType::RECEIVE;
            event->peer = peer;

            // ディスパッチすべきピアが残っている場合は、ディスパッチ待ちキューにピアを投入する
            if (peer->DispatchedCommandExists()) {
                peer->needs_dispatch(true);

                dispatch_hub_->Enqueue(peer);
            }

            return EventStatus::AN_EVENT_OCCURRED;
        }
    }

    return EventStatus::NO_EVENT_OCCURRED;
}

void
RUdpProtocol::HandleConnect(std::shared_ptr<RUdpPeer> &peer, const RUdpProtocolHeader * header,
                            const RUdpProtocolType * cmd, const RUdpAddress &received_address,
                            uint32_t host_outgoing_bandwidth, uint32_t host_incoming_bandwidth)
{
    auto channel_count = ntohl(cmd->connect.channel_count);

    if (channel_count < PROTOCOL_MINIMUM_CHANNEL_COUNT || channel_count > PROTOCOL_MAXIMUM_CHANNEL_COUNT)
        return;

    peer->SetupConnectedPeer(cmd, received_address, host_incoming_bandwidth, host_outgoing_bandwidth, channel_count);
}

Error
RUdpProtocol::HandleVerifyConnect(const std::unique_ptr<RUdpEvent> &event, std::shared_ptr<RUdpPeer> &peer,
                                  const RUdpProtocolType *cmd)
{
    if (!peer->StateIs(RUdpPeerState::CONNECTING))
        return Error::OK;

    auto channel_count = ntohl(cmd->verify_connect.channel_count);

    if (channel_count < PROTOCOL_MINIMUM_CHANNEL_COUNT ||
        channel_count > PROTOCOL_MAXIMUM_CHANNEL_COUNT ||
        ntohl(cmd->verify_connect.segment_throttle_interval) != peer->net()->segment_throttle_interval() ||
        ntohl(cmd->verify_connect.segment_throttle_acceleration) != peer->net()->segment_throttle_acceleration() ||
        ntohl(cmd->verify_connect.segment_throttle_deceleration) != peer->net()->segment_throttle_deceleration() ||
        cmd->verify_connect.connect_id != peer->connect_id())
    {
        peer->event_data(0);

        dispatch_hub_->DispatchState(peer, RUdpPeerState::ZOMBIE);

        return Error::ERROR;
    }

    peer->command()->RemoveSentReliableCommands(1, 0xFF);

    //if (channel_count < peer->channel_count())
    //    peer->channel_count(channel_count);

    peer->outgoing_peer_id(ntohs(cmd->verify_connect.outgoing_peer_id));
    peer->incoming_session_id(cmd->verify_connect.incoming_session_id);
    peer->outgoing_session_id(cmd->verify_connect.outgoing_session_id);

    auto mtu = ntohl(cmd->verify_connect.mtu);

    if (mtu < PROTOCOL_MINIMUM_MTU)
        mtu = PROTOCOL_MINIMUM_MTU;
    else if (mtu > PROTOCOL_MAXIMUM_MTU)
        mtu = PROTOCOL_MAXIMUM_MTU;

    if (mtu < peer->mtu())
        peer->mtu(mtu);

    auto window_size = ntohl(cmd->verify_connect.window_size);

    if (window_size < PROTOCOL_MINIMUM_WINDOW_SIZE)
        window_size = PROTOCOL_MINIMUM_WINDOW_SIZE;

    if (window_size > PROTOCOL_MAXIMUM_WINDOW_SIZE)
        window_size = PROTOCOL_MAXIMUM_WINDOW_SIZE;

    if (window_size < peer->net()->window_size())
        peer->net()->window_size(window_size);

    peer->incoming_bandwidth(ntohl(cmd->verify_connect.incoming_bandwidth));
    peer->outgoing_bandwidth(ntohl(cmd->verify_connect.outgoing_bandwidth));

    dispatch_hub_->NotifyConnect(event, peer);

    return Error::OK;
}

Error
RUdpProtocol::HandleDisconnect(std::shared_ptr<RUdpPeer> &peer, const RUdpProtocolType *cmd)
{
    if (peer->StateIs(RUdpPeerState::DISCONNECTED) ||
        peer->StateIs(RUdpPeerState::ZOMBIE) ||
        peer->StateIs(RUdpPeerState::ACKNOWLEDGING_DISCONNECT))
    {
        return Error::OK;
    }

    ResetPeerQueues(peer);

    if (peer->StateIs(RUdpPeerState::CONNECTION_SUCCEEDED) ||
        peer->StateIs(RUdpPeerState::DISCONNECTING) ||
        peer->StateIs(RUdpPeerState::CONNECTING))
    {
        dispatch_hub_->DispatchState(peer, RUdpPeerState::ZOMBIE);
    }
    else if (!peer->StateIs(RUdpPeerState::CONNECTED) && !peer->StateIs(RUdpPeerState::DISCONNECT_LATER))
    {
        if (peer->StateIs(RUdpPeerState::CONNECTION_PENDING))
        {
            dispatch_hub_->recalculate_bandwidth_limits(true);
        }

        peer->Reset();
    }
    else if (cmd->header.command & PROTOCOL_COMMAND_FLAG_ACKNOWLEDGE)
    {
        dispatch_hub_->ChangeState(peer, RUdpPeerState::ACKNOWLEDGING_DISCONNECT);
    }
    else
    {
        dispatch_hub_->DispatchState(peer, RUdpPeerState::ZOMBIE);
    }

    if (!peer->StateIs(RUdpPeerState::DISCONNECTED))
    {
        peer->event_data(ntohl(cmd->disconnect.data));
    }

    return Error::OK;
}

// void enet_peer_reset (ENetPeer * peer)
void
RUdpProtocol::ResetPeer(const std::shared_ptr<RUdpPeer> &peer)
{
    dispatch_hub_->Disconnect(peer);

    peer->Reset();

    ResetPeerQueues(peer);
}

void
RUdpProtocol::ResetPeerQueues(const std::shared_ptr<RUdpPeer> &peer)
{
    std::unique_ptr<RUdpChannel> channel;

    if (peer->needs_dispatch())
        peer->needs_dispatch(false);

    if (peer->AcknowledgementExists())
        peer->ClearAcknowledgement();

    peer->command()->clear_sent_reliable_command();
    peer->command()->clear_sent_unreliable_command();

    peer->command()->clear_outgoing_reliable_command();
    peer->command()->clear_outgoing_unreliable_command();

    while (peer->DispatchedCommandExists())
        peer->ClearDispatchedCommandQueue();

    if (peer->ChannelExists())
        peer->ClearChannel();
}

void
RUdpProtocol::SendAcknowledgements(std::shared_ptr<RUdpPeer> &peer)
{
    while (peer->AcknowledgementExists()) {
        auto command = chamber_->EmptyCommandBuffer();
        auto buffer = chamber_->EmptyDataBuffer();
        auto ack = peer->PopAcknowledgement();

        // 送信継続
        // - コマンドバッファに空きがない
        // - データバッファに空きがない
        // - ピアの MTU とパケットサイズの差が RUdpProtocolAcknowledge のサイズ未満
        if (command == nullptr ||
            buffer == nullptr ||
            peer->net()->mtu() - chamber_->segment_size() < sizeof(RUdpProtocolAcknowledge))
        {
            chamber_->continue_sending(true);

            break;
        }

        //buffer->Add(command);
        //buffer->data_length = sizeof(RUdpProtocolAcknowledge);

        chamber_->update_segment_size(sizeof(RUdpProtocolAcknowledge));

        auto reliable_sequence_number = htons(ack->command.header.reliable_sequence_number);

        (*command)->header.command = PROTOCOL_COMMAND_ACKNOWLEDGE;
        (*command)->header.channel_id = ack->command.header.channel_id;
        (*command)->header.reliable_sequence_number = reliable_sequence_number;
        (*command)->acknowledge.received_reliable_sequence_number = reliable_sequence_number;
        (*command)->acknowledge.received_sent_time = htons(ack->sent_time);

        (*buffer)->Add(*command);

        if ((ack->command.header.command & PROTOCOL_COMMAND_MASK) == PROTOCOL_COMMAND_DISCONNECT)
            dispatch_hub_->DispatchState(peer, RUdpPeerState::ZOMBIE);

        //++command;
        //++buffer;
    }

    //chamber_->update_command_count(command);
    //chamber_->update_buffer_count(buffer);
}

bool
RUdpProtocol::SendReliableOutgoingCommands(const std::shared_ptr<RUdpPeer> &peer,
                                           uint32_t service_time)
{
    auto can_ping = peer->LoadReliableCommandsIntoChamber(chamber_, service_time);

    return can_ping;
}

void
RUdpProtocol::SendUnreliableOutgoingCommands(std::shared_ptr<RUdpPeer> &peer, uint32_t service_time)
{
    auto can_disconnect = peer->LoadUnreliableCommandsIntoChamber(chamber_);

    if (can_disconnect)
        dispatch_hub_->Disconnect(peer);
}
