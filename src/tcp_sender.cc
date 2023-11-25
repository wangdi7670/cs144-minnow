#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>
#include <algorithm>
#include <assert.h>
#include <iostream>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  uint64_t total = 0;
  for (auto m : messages_) {
    total += m.sequence_length();
  }

  for (auto m : outstanding_segments_) {
    total += m.sequence_length();
  }
  return total;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return {};
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if (!messages_.empty()) {
    std::optional<TCPSenderMessage> res = messages_.front();
    if (messages_.front().sequence_length() > 0) {
      outstanding_segments_.push_back( messages_.front() );
    } 
    messages_.erase(messages_.begin());
    return res;
  }
  return {};
}

bool TCPSender::stream_has_FIN(Reader& stream) const
{
  if (transmitted_FIN) {
    return false;
  }

  return stream.is_finished();
}

void TCPSender::fill_msg_payload(std::string& payload, Reader& stream, uint64_t length)
{
  std::string_view sv = stream.peek();
  payload = std::string{sv.substr(0, length)};
}

uint64_t TCPSender::space_available() const
{
  uint64_t left = receiver_ab_ackno_;
  uint64_t right = left + receiver_window_ - 1;
  assert( next_absolute_num_ >= left );
  return ( next_absolute_num_ > right ) ? 0 : ( right - next_absolute_num_ + 1 );
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  (void)outbound_stream;

  while (space_available() > 0 && stream_has_src(outbound_stream)) {
    Wrap32 seqno = Wrap32::wrap(next_absolute_num_, isn_);
    bool SYN = false;
    std::string payload{};
    bool FIN = false;

    if (stream_has_SYN()) {
      SYN = true;
      next_absolute_num_++;
    }

    uint64_t space = space_available();
    if (space > 0 && outbound_stream.bytes_buffered() > 0) {
      uint64_t temp = std::min(TCPConfig::MAX_PAYLOAD_SIZE, outbound_stream.bytes_buffered());
      uint64_t length = (temp < space) ? temp : space;

      fill_msg_payload(payload, outbound_stream, length);
      outbound_stream.pop(length);
      next_absolute_num_ += length;
      
      assert(next_absolute_num_ <= (receiver_ab_ackno_ + receiver_window_));
    }

    if (stream_has_FIN(outbound_stream) && space_available() > 0) {
      FIN = true;
      transmitted_FIN = true;
      next_absolute_num_++;
    }

    TCPSenderMessage msg{seqno, SYN, Buffer{payload}, FIN};
    messages_.push_back(msg);
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
  
  if (!msg.ackno.has_value()) {
    return;
  }  
  assert(msg.ackno.has_value());

  // Impossible ackno (beyond next seqno) is ignored
  uint64_t receiver_ab = msg.ackno.value().unwrap(isn_, next_absolute_num_);
  if (receiver_ab > next_absolute_num_) {
    return;
  }

  receiver_ab_ackno_ = receiver_ab;
  receiver_window_ = (msg.window_size == 0) ? 1 : msg.window_size;

  while (!outstanding_segments_.empty()) {
    TCPSenderMessage& m = outstanding_segments_.front();
    uint64_t right_edge = m.seqno.unwrap(isn_, next_absolute_num_) + m.sequence_length() - 1;
    if (receiver_ab_ackno_ > right_edge) {
      outstanding_segments_.erase(outstanding_segments_.begin());
    } else {
      return;
    }
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  (void)ms_since_last_tick;
}
