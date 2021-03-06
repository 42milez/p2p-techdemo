#include "outgoing_command.h"

namespace rudp
{
    OutgoingCommand::OutgoingCommand()
        : round_trip_timeout_()
        , round_trip_timeout_limit_()
        , sent_time_()
        , send_attempts_()
        , reliable_sequence_number_()
        , unreliable_sequence_number_()
    {
    }
} // namespace rudp
