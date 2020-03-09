#ifndef P2P_TECHDEMO_LIB_RUDP_CONNECTION_H_
#define P2P_TECHDEMO_LIB_RUDP_CONNECTION_H_

#include "lib/core/io/socket.h"

#include "chamber.h"
#include "network_config.h"

namespace rudp
{
    class Connection
    {
      public:
        explicit Connection(const NetworkConfig &address);

        ssize_t
        Receive(NetworkConfig &received_address, std::vector<uint8_t> &buffer, size_t buffer_count);

        ssize_t
        Send(const NetworkConfig &address, const std::unique_ptr<Chamber> &chamber);

      private:
        std::unique_ptr<Socket> socket_;
    };
} // namespace rudp

#endif // P2P_TECHDEMO_LIB_RUDP_CONNECTION_H_
