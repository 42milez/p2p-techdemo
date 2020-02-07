#ifndef P2P_TECHDEMO_RUDPCHANNEL_H
#define P2P_TECHDEMO_RUDPCHANNEL_H

#include <array>
#include <list>
#include <tuple>

#include "core/errors.h"
#include "core/logger.h"
#include "core/singleton.h"
#include "lib/rudp/command/RUdpIncomingCommand.h"

class RUdpChannel
{
public:
    RUdpChannel();

    std::vector<std::shared_ptr<RUdpIncomingCommand>>
    NewIncomingReliableCommands();

    void
    Reset();

    inline void
    DecrementReliableWindow(size_t idx) { --reliable_windows_.at(idx); }

    inline void
    IncrementOutgoingReliableSequenceNumber() {
        core::Singleton<core::Logger>::Instance().Debug("outgoing reliable sequence number was incremented (RUdpChannel): {0} -> {1}",
                                                        outgoing_reliable_sequence_number_,
                                                        outgoing_reliable_sequence_number_ + 1);
        ++outgoing_reliable_sequence_number_;
    }

    inline bool
    IncomingUnreliableCommandExists() { return !incoming_unreliable_commands_.empty(); }

    inline void
    IncrementOutgoingUnreliableSequenceNumber() { ++outgoing_unreliable_sequence_number_; }

    // ToDo: 各要素が取りうる値は 0 or 1 であるため、boolean を検討する
    inline void
    IncrementReliableWindow(size_t idx) { ++reliable_windows_.at(idx); }

    inline void
    MarkReliableWindowAsUnused(uint16_t position) { used_reliable_windows_ &= ~ (1u << position); }

    inline void
    MarkReliableWindowAsUsed(uint16_t position) { used_reliable_windows_ |= 1u << position; }

    std::tuple<std::shared_ptr<RUdpIncomingCommand>, Error>
    ExtractFirstCommand(uint16_t start_sequence_number, int total_length, uint32_t fragment_count);

    std::tuple<std::shared_ptr<RUdpIncomingCommand>, Error>
    QueueIncomingCommand(const std::shared_ptr<RUdpProtocolType> &cmd, VecUInt8 &data, uint16_t flags,
                         uint32_t fragment_count);

    inline uint16_t
    ReliableWindow(size_t idx) { return reliable_windows_.at(idx); }

public:
    inline uint16_t
    incoming_reliable_sequence_number() { return incoming_reliable_sequence_number_; }

    inline uint16_t
    incoming_unreliable_sequence_number() { return incoming_unreliable_sequence_number_; }

    inline uint16_t
    outgoing_reliable_sequence_number() { return outgoing_reliable_sequence_number_; }

    inline uint16_t
    outgoing_unreliable_sequence_number() { return outgoing_unreliable_sequence_number_; }

    inline void
    outgoing_unreliable_sequence_number(uint16_t val) { outgoing_unreliable_sequence_number_ = val; }

    inline uint16_t
    used_reliable_windows() { return used_reliable_windows_; }

private:
    std::list<std::shared_ptr<RUdpIncomingCommand>> incoming_reliable_commands_;
    std::list<std::shared_ptr<RUdpIncomingCommand>> incoming_unreliable_commands_;
    std::array<uint16_t, PEER_RELIABLE_WINDOWS> reliable_windows_;

    uint16_t incoming_reliable_sequence_number_;
    uint16_t incoming_unreliable_sequence_number_;
    uint16_t outgoing_reliable_sequence_number_;
    uint16_t outgoing_unreliable_sequence_number_;
    uint16_t used_reliable_windows_; // 使用中のバッファ（reliable_windows[PEER_RELIABLE_WINDOWS]）
};

#endif // P2P_TECHDEMO_RUDPCHANNEL_H
