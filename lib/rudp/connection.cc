#include "lib/core/error_macros.h"
#include "lib/core/logger.h"
#include "lib/core/singleton.h"

#include "chamber.h"
#include "connection.h"

namespace rudp
{
    Connection::Connection(const NetworkConfig &address)
    {
        socket_ = std::make_unique<core::Socket>();

        if (socket_ == nullptr) {
            // throw exception
            // ...
        }

        socket_->Open(core::SocketType::UDP, core::IP::Type::ANY);

        socket_->SetBlockingEnabled(false);
        socket_->SetBroadcastingEnabled(true);

        core::IpAddress ip{};

        if (address.wildcard()) {
            ip = core::IpAddress("*");
        }
        else {
            ip.SetIpv6(address.host());
        }

        auto ret = socket_->Bind(ip, address.port());

        if (ret != core::Error::OK) {
            // throw exception
            // ...
        }
    }

    ssize_t
    Connection::Receive(NetworkConfig &received_address, std::vector<uint8_t> &buffer, size_t buffer_count)
    {
        ERR_FAIL_COND_V(buffer_count != 1, -1)

        auto err = socket_->Wait();

        if (err == core::Error::ERR_BUSY)
            return 0;

        if (err != core::Error::OK)
            return -1;

        ssize_t read_count;
        core::IpAddress ip;

        uint16_t port = 0;
        err = socket_->RecvFrom(buffer, read_count, ip, port);

        received_address.port(port);

        if (err == core::Error::ERR_BUSY)
            return 0;

        if (err != core::Error::OK)
            return -1;

        received_address.SetIP(ip.GetIPv6(), 16);

        core::LOG_DEBUG_VA("received length: {0}", read_count);

        return read_count;
    }

    ssize_t
    Connection::Send(const NetworkConfig &address, const std::unique_ptr<Chamber> &chamber)
    {
        core::IpAddress dest;

        dest.SetIpv6(address.host());

        std::vector<uint8_t> out;

        auto size = chamber->Write(out);
        ssize_t sent = 0;

        auto err = socket_->SendTo(&(out.at(0)), size, sent, dest, address.port());

        core::LOG_DEBUG_VA("bytes sent: {0}", sent);

        if (err != core::Error::OK) {
            if (err == core::Error::ERR_BUSY)
                return 0;

            return -1;
        }

        return sent;
    }
} // namespace rudp
