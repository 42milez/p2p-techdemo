#include "RUdpOutgoingCommand.h"

RUdpOutgoingCommand::RUdpOutgoingCommand()
    : fragment_offset_(),
      fragment_length_(),
      round_trip_timeout_(),
      round_trip_timeout_limit_(),
      sent_time_(),
      send_attempts_(),
      reliable_sequence_number_(),
      unreliable_sequence_number_()
{}