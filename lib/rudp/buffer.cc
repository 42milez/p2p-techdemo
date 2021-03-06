#include <cstring>

#include "buffer.h"
#include "const.h"
#include "enum.h"
#include "lib/core/logger.h"
#include "lib/core/singleton.h"
#include "lib/rudp/command/command_size.h"

namespace rudp
{
    namespace
    {
        enum class BufferVariant : uint8_t
        {
            ProtocolTypeSP,
            VecUInt8
        };
    }

    Buffer::Buffer()
        : buffer_()
        , offset_()
        , size_()
    {
    }

    void
    Buffer::Add(const ProtocolTypeSP &data)
    {
        buffer_ = data;
        size_   = COMMAND_SIZES.at(data->header.command & PROTOCOL_COMMAND_MASK);
    }

    void
    Buffer::CopyHeaderFrom(const std::vector<uint8_t> &data, size_t offset, size_t size)
    {
        buffer_ = data;
        offset_ = offset;

        if (size != 0)
            size_ = size;
        else
            size_ = data.size() * sizeof(uint8_t);
    }

    void
    Buffer::CopySegmentFrom(const std::shared_ptr<Segment> &segment, size_t offset, size_t size)
    {
        buffer_ = segment->Data(offset, size);
        offset_ = offset;

        if (size != 0)
            size_ = size;
        else
            size_ = segment->DataLength();
    }

    std::string
    Buffer::ProtocolCommandAsString()
    {
        if (buffer_.index() != static_cast<int>(BufferVariant::ProtocolTypeSP))
            return "NOT A COMMAND";

        auto protocol = std::get<static_cast<int>(BufferVariant::ProtocolTypeSP)>(buffer_);

        if (protocol == nullptr)
            return "INVALID COMMAND";

        auto cmd_number = protocol->header.command & PROTOCOL_COMMAND_MASK;

        if (COMMANDS_AS_STRING.size() <= cmd_number)
            return "INVALID COMMAND";

        return COMMANDS_AS_STRING.at(cmd_number);
    }

    std::vector<uint8_t>::iterator
    Buffer::CopyTo(std::vector<uint8_t>::iterator it)
    {
        if (buffer_.index() == static_cast<int>(BufferVariant::ProtocolTypeSP)) {
            auto protocol = std::get<static_cast<int>(BufferVariant::ProtocolTypeSP)>(buffer_);

            if (protocol) {
                std::memcpy(&(*it), &(*protocol), size_);
                core::LOG_DEBUG_VA("command was copied to buffer: {0} (sn: {1}, size: {2})", ProtocolCommandAsString(),
                             protocol->header.reliable_sequence_number, size_);
            }

            return it + size_;
        }

        // FIXME: get reference of buffer_
        auto payload = std::get<static_cast<int>(BufferVariant::VecUInt8)>(buffer_);

        if (size_ > 0) {
            std::memcpy(&(*it), &(payload.at(0)), size_);
            core::LOG_DEBUG_VA("payload was copied to buffer (size: {0})", size_);
        }

        return it + size_;
    }
} // namespace rudp
