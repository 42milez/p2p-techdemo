#include <memory>
#include <unistd.h>

#include <catch2/catch.hpp>

#include "lib/core/network/system.h"
#include "lib/core/hash.h"
#include "lib/rudp/host.h"
#include "lib/test/util.h"

#include "var.h"

TEST_CASE("guest peer can establish connection with host peer", "[feature][connection]")
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

    // guest
    rudp::NetworkConfig guest_address;
    guest_address.port(test::GUEST1_PORT);
    auto guest           = std::make_unique<rudp::Host>(guest_address, core::SysCh::MAX, 32, 100, 100);
    auto guest_event     = std::make_unique<rudp::Event>();
    auto guest_unique_id = core::Singleton<core::Hash>::Instance().uniqueID();

    SECTION("guest peer can immediately disconnect from host peer")
    {
        guest->Connect(host_address, core::SysCh::MAX, guest_unique_id);
        guest->Service(guest_event, 0);

        REQUIRE(test::wait(
            [&host, &host_event, &guest, &guest_event]() {
                host->Service(host_event, 0);
                guest->Service(guest_event, 0);
                return (host->PeerState(0) == rudp::RUdpPeerState::CONNECTED) &&
                       (guest->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
            },
            test::DEFAULT_TIMEOUT));

        guest->DisconnectNow(guest->PeerPtr(0), 0);
        guest->Service(guest_event, 0);

        REQUIRE(test::wait(
            [&host, &host_event, &guest, &guest_event]() {
                host->Service(host_event, 0);
                guest->Service(guest_event, 0);
                return (host->PeerState(0) == rudp::RUdpPeerState::DISCONNECTED) &&
                       (guest->PeerState(0) == rudp::RUdpPeerState::DISCONNECTED);
            },
            test::DEFAULT_TIMEOUT));
    }

    SECTION("guest peer can gracefully disconnect from host peer")
    {
        guest->Connect(host_address, core::SysCh::MAX, guest_unique_id);
        guest->Service(guest_event, 0);

        REQUIRE(test::wait(
            [&host, &host_event, &guest, &guest_event]() {
                host->Service(host_event, 0);
                guest->Service(guest_event, 0);
                return (host->PeerState(0) == rudp::RUdpPeerState::CONNECTED) &&
                       (guest->PeerState(0) == rudp::RUdpPeerState::CONNECTED);
            },
            test::DEFAULT_TIMEOUT));

        guest->DisconnectLater(guest->PeerPtr(0), 0);
        guest->Service(guest_event, 0);

        REQUIRE(test::wait(
            [&host, &host_event, &guest, &guest_event]() {
                host->Service(host_event, 0);
                guest->Service(guest_event, 0);
                return (host->PeerState(0) == rudp::RUdpPeerState::DISCONNECTED) &&
                       (guest->PeerState(0) == rudp::RUdpPeerState::DISCONNECTED);
            },
            test::DEFAULT_TIMEOUT));
    }
}
