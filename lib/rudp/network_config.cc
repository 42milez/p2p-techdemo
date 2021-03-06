#include <cstring>

#include "network_config.h"

namespace rudp
{
    NetworkConfig::NetworkConfig()
        : host_()
        , port_()
        , wildcard_()
    {
    }

    void
    NetworkConfig::Reset()
    {
        std::fill_n(host_.begin(), HOST_LENGTH, 0);
        port_     = 0;
        wildcard_ = 0;
    }

    void
    NetworkConfig::SetIP(const uint8_t *ip, size_t size)
    {
        auto len = size > HOST_LENGTH ? HOST_LENGTH : size;

        std::fill_n(host_.begin(), HOST_LENGTH, 0);

        memcpy(&host_, ip, len); // network byte-order (big endian)
    }

    NetworkConfig &
    NetworkConfig::operator=(const NetworkConfig &address)
    {
        if (this == &address)
            return *this;

        std::copy(address.host().begin(), address.host().end(), host_.begin());
        port_ = address.port();

        return *this;
    }

    bool
    NetworkConfig::operator==(const NetworkConfig &address) const
    {
        auto same_host = memcmp(&host_, &address.host(), HOST_LENGTH) == 0;
        auto same_port = port_ == address.port();

        return same_host & same_port;
    }

    bool
    NetworkConfig::operator!=(const NetworkConfig &address) const
    {
        auto same_host = memcmp(&host_, &address.host(), HOST_LENGTH) == 0;
        auto same_port = port_ == address.port();

        return !(same_host & same_port);
    }
} // namespace rudp
