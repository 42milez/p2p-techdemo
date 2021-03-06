#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "lib/core/network/system.h"
#include "lib/rudp/host.h"
#include "lib/test/util.h"

#include "var.h"

TEST_CASE("guest peer can send reliable command to host peer", "[feature][reliable_command]")
{
    // host address
    core::IpAddress host_ip{"::FFFF:127.0.0.1"};
    rudp::NetworkConfig host_address;
    host_address.host_v6(host_ip.GetIPv6());
    host_address.port(test::HOST_PORT);

    // host
    rudp::NetworkConfig address;
    address.port(test::HOST_PORT);
    auto host       = std::make_unique<rudp::Host>(address, core::SysCh::MAX, 32, 100, 100);
    auto host_event = std::make_unique<rudp::Event>();

    // guest1
    rudp::NetworkConfig guest1_address;
    guest1_address.port(test::GUEST1_PORT);
    auto guest1       = std::make_unique<rudp::Host>(guest1_address, core::SysCh::MAX, 2, 100, 100);
    auto guest1_event = std::make_unique<rudp::Event>();

    // guest2
    rudp::NetworkConfig guest2_address;
    guest2_address.port(test::GUEST2_PORT);
    auto guest2       = std::make_unique<rudp::Host>(guest2_address, core::SysCh::MAX, 2, 100, 100);
    auto guest2_event = std::make_unique<rudp::Event>();

    SECTION("guest peer 1 and 2 can send reliable command to host peer")
    {
        //  guest peer 1 [1/2]
        // --------------------------------------------------

        guest1->Connect(host_address, core::SysCh::MAX, 0);
        guest1->Service(guest1_event, 0);

        test::wait(
            [&host, &host_event, &guest1, &guest1_event]() {
                host->Service(host_event, 0);
                guest1->Service(guest1_event, 0);
                return (host->PeerState(0) == rudp::RUdpPeerState::CONNECTED) &&
                       (guest1->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
            },
            test::DEFAULT_TIMEOUT);

        std::string msg_1_1{"command from guest1 (1/2)"};
        auto data_1_1    = std::vector<uint8_t>{msg_1_1.begin(), msg_1_1.end()};
        auto flags_1_1   = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
        auto segment_1_1 = std::make_shared<rudp::Segment>(&data_1_1, flags_1_1);

        guest1->Send(0, core::SysCh::RELIABLE, segment_1_1);
        guest1->Service(guest1_event, 0);

        test::wait(
            [&host, &host_event]() {
                return host->Service(host_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED &&
                       host_event->TypeIs(rudp::EventType::RECEIVE);
            },
            test::DEFAULT_TIMEOUT);

        REQUIRE(host_event->channel_id() == static_cast<uint8_t>(core::SysCh::RELIABLE));
        REQUIRE(host_event->TypeIs(rudp::EventType::RECEIVE));
        REQUIRE(host_event->DataAsString() == msg_1_1);

        test::wait(
            [&guest1, &guest1_event]() {
              guest1->Service(guest1_event, 0);
              return guest1_event->TypeIs(rudp::EventType::RECEIVE_ACK);
            },
            test::DEFAULT_TIMEOUT);

        //  guest peer 1 [2/2]
        // --------------------------------------------------

        std::string msg_1_2{"command from guest1 (2/2)"};
        auto data_1_2    = std::vector<uint8_t>{msg_1_2.begin(), msg_1_2.end()};
        auto flags_1_2   = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
        auto segment_1_2 = std::make_shared<rudp::Segment>(&data_1_2, flags_1_2);

        guest1->Send(0, core::SysCh::RELIABLE, segment_1_2);
        guest1->Service(guest1_event, 0);

        test::wait(
            [&host, &host_event]() {
              return host->Service(host_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED &&
                     host_event->TypeIs(rudp::EventType::RECEIVE);
            },
            test::DEFAULT_TIMEOUT);

        REQUIRE(host_event->channel_id() == static_cast<uint8_t>(core::SysCh::RELIABLE));
        REQUIRE(host_event->TypeIs(rudp::EventType::RECEIVE));
        REQUIRE(host_event->DataAsString() == msg_1_2);

        test::wait(
            [&guest1, &guest1_event]() {
              guest1->Service(guest1_event, 0);
              return guest1_event->TypeIs(rudp::EventType::RECEIVE_ACK);
            },
            test::DEFAULT_TIMEOUT);

        //  guest peer 2 [1/2]
        // --------------------------------------------------

        guest2->Connect(host_address, core::SysCh::MAX, 0);
        guest2->Service(guest2_event, 0);

        test::wait(
            [&host, &host_event, &guest2, &guest2_event]() {
                host->Service(host_event, 0);
                guest2->Service(guest2_event, 0);
                return (host->PeerState(0) == rudp::RUdpPeerState::CONNECTED) &&
                       (guest2->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
            },
            test::DEFAULT_TIMEOUT);

        std::string msg_2_1{"command from guest2 (1/2)"};
        auto payload_2_1 = std::vector<uint8_t>{msg_2_1.begin(), msg_2_1.end()};
        auto flags_2_1   = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
        auto segment_2_1 = std::make_shared<rudp::Segment>(&payload_2_1, flags_2_1);

        guest2->Send(0, core::SysCh::RELIABLE, segment_2_1);
        guest2->Service(guest2_event, 0);

        test::wait(
            [&host, &host_event]() {
                return host->Service(host_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED &&
                       host_event->TypeIs(rudp::EventType::RECEIVE);
            },
            test::DEFAULT_TIMEOUT);

        REQUIRE(host_event->channel_id() == static_cast<uint8_t>(core::SysCh::RELIABLE));
        REQUIRE(host_event->TypeIs(rudp::EventType::RECEIVE));
        REQUIRE(host_event->DataAsString() == msg_2_1);

        test::wait(
            [&guest2, &guest2_event]() {
              guest2->Service(guest2_event, 0);
              return guest2_event->TypeIs(rudp::EventType::RECEIVE_ACK);
            },
            test::DEFAULT_TIMEOUT);

        //  guest peer 2 [2/2]
        // --------------------------------------------------

        std::string msg_2_2{"command from guest2 (2/2)"};
        auto data_2_2    = std::vector<uint8_t>{msg_2_2.begin(), msg_2_2.end()};
        auto flags_2_2   = static_cast<uint32_t>(rudp::SegmentFlag::RELIABLE);
        auto segment_2_2 = std::make_shared<rudp::Segment>(&data_2_2, flags_2_2);

        guest2->Send(0, core::SysCh::RELIABLE, segment_2_2);
        guest2->Service(guest2_event, 0);

        test::wait(
            [&host, &host_event]() {
              return host->Service(host_event, 0) == rudp::EventStatus::AN_EVENT_OCCURRED &&
                     host_event->TypeIs(rudp::EventType::RECEIVE);
            },
            test::DEFAULT_TIMEOUT);

        REQUIRE(host_event->channel_id() == static_cast<uint8_t>(core::SysCh::RELIABLE));
        REQUIRE(host_event->TypeIs(rudp::EventType::RECEIVE));
        REQUIRE(host_event->DataAsString() == msg_2_2);

        test::wait(
            [&guest2, &guest2_event]() {
              guest2->Service(guest2_event, 0);
              return guest2_event->TypeIs(rudp::EventType::RECEIVE_ACK);
            },
            test::DEFAULT_TIMEOUT);
    }
}
