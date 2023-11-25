#pragma once

#include <map>

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include "assert.h"

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  // my code here
  uint16_t receiver_window_{1};
  uint64_t receiver_ab_ackno_{0};                                // ackno that receiver sended is sequence number, but we keep track of absolute number of ackno 

  std::vector<TCPSenderMessage> messages_{};                     // ready-segments
  uint64_t next_absolute_num_{};                                 // absolute number that tcp_sender will send next
  // std::vector<TCPSenderMessage> outstanding_segments_{};
  std::map<uint64_t, TCPSenderMessage> outstanding_segments_{};  // k is absolute-num, v is segment

  bool transmitted_FIN{false};                                   // have ever transmitted FIN?
  uint64_t consecutive_retransmissions_{0};                        

  class Timer {
  public:
    uint64_t start_time_{0};
    uint64_t RTO_{0};
    bool running{false};

    void start(uint64_t start_time, uint64_t RTO);

    bool expire(uint64_t current_time) const;
  };

  Timer timer{};                                                 // retransmission timer

private:
  bool stream_has_src(Reader& stream) const
  {
    return stream_has_SYN() || stream.bytes_buffered() > 0 || stream_has_FIN(stream);
  }

  bool stream_has_SYN() const
  {
    return next_absolute_num_ == 0;
  }

  bool stream_has_FIN(Reader& stream) const;

  // receiver's space-available of window
  uint64_t space_available() const;

  void fill_msg_payload(std::string& payload, Reader& stream, uint64_t length);

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
