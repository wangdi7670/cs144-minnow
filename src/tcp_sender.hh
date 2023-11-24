#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  // my code here
  uint16_t receiver_window_{1};
  std::optional<Wrap32> receiver_ackno_{};

  std::vector<TCPSenderMessage> messages_{};
  uint64_t next_absolute_num_{};
  std::vector<TCPSenderMessage> outstanding_segments_{};

  std::optional<Reader&> stream_{};

private:
  bool stream_has_src(Reader& stream) const
  {
    return stream_has_SYN() || stream.bytes_buffered() > 0 || stream.is_finished();
  }

  bool stream_has_SYN() const
  {
    return next_absolute_num_ == 0;
  }

  void fill_msg_payload(std::string& payload, Reader& stream, uint16_t length);

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
