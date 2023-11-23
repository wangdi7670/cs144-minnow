#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>
#include <algorithm>
#include <assert.h>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return {};
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return {};
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  return {};
}


void TCPSender::fill_msg_payload(std::string& payload, Reader& stream, uint16_t length)
{
  std::string_view sv = stream.peek();
  payload = std::string{sv.substr(0, length)};
}


void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  (void)outbound_stream;

  while (receiver_window_ > 0 && stream_has_src(outbound_stream)) {
    Wrap32 seqno = Wrap32::wrap(next_absolute_num_, isn_);
    bool SYN = false;
    std::string payload{};
    bool FIN = false;


    if (stream_has_SYN()) {
      SYN = true;
      receiver_window_--;
    }

    if (receiver_window_ > 0 && outbound_stream.bytes_buffered() > 0) {
      uint64_t temp = std::min(TCPConfig::MAX_PAYLOAD_SIZE, outbound_stream.bytes_buffered());
      uint16_t length = (temp < receiver_window_) ? temp : receiver_window_;

      fill_msg_payload(payload, outbound_stream, length);
      outbound_stream.pop(length);
      receiver_window_ -= length;
      
      assert(receiver_window_ >= 0);
    }

    if (outbound_stream.is_finished() && receiver_window_ > 0) {
      FIN = true;
      receiver_window_--;
    }

    TCPSenderMessage msg{seqno, SYN, Buffer{payload}, FIN};
    messages_.push_back(msg);
    next_absolute_num_ += msg.sequence_length();
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  struct TCPSenderMessage msg; 
  msg.seqno = Wrap32::wrap(next_absolute_num_, isn_);
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  (void)msg;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  (void)ms_since_last_tick;
}
