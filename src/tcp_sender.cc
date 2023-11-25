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
    total += m.second.sequence_length();
  }
  return total;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return consecutive_retransmissions_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if (!messages_.empty()) {
    std::optional<TCPSenderMessage> res = messages_.front();
    if (messages_.front().sequence_length() > 0) {
      uint64_t ab = res.value().seqno.unwrap(isn_, next_absolute_num_);
      outstanding_segments_.insert({ab, messages_.front()});
      if (!timer_.running_) {
        timer_.start(time_, initial_RTO_ms_);
      }
    } 
    messages_.erase(messages_.begin());
    return res;
  }
  return {};
}

bool TCPSender::stream_has_FIN(Reader& stream) const
{
  if (transmitted_FIN) {
    assert(stream.bytes_buffered() == 0);
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
  
  receiver_window_ = (msg.window_size == 0) ? 1 : msg.window_size;

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

  bool is_ack_some = false;

  // remove any that have now been fully acknowledged
  for (auto iter = outstanding_segments_.begin(); iter != outstanding_segments_.end(); ) {
    const auto& [absolute_num, seg] = *iter;
    uint64_t right_edge = absolute_num + seg.sequence_length() - 1;
    if (receiver_ab_ackno_ > right_edge) {
      is_ack_some = true;
      iter = outstanding_segments_.erase(iter);
    } else {
      iter++;
    }
  }

  if (is_ack_some) {
    if (outstanding_segments_.empty()) {
      timer_.stop();
    } else {
      consecutive_retransmissions_ = 0;
      timer_.start(time_, initial_RTO_ms_);
    }
  }
}


void TCPSender::retransmit_earliest()
{ 
  assert(!outstanding_segments_.empty());

  // find earliest
  auto iter = outstanding_segments_.begin();
  uint64_t min_absoulute = iter->first;

  for (const auto& pair : outstanding_segments_) {
    min_absoulute = std::min(min_absoulute, pair.first);
  }

  assert(outstanding_segments_.contains(min_absoulute));
  messages_.insert(messages_.begin(), outstanding_segments_.at(min_absoulute));

  outstanding_segments_.erase(min_absoulute);
}
  

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  (void)ms_since_last_tick;

  time_ += ms_since_last_tick;

  if (timer_.running_ && timer_.expire(time_)) {
    if (outstanding_segments_.empty()) {
      timer_.stop();
      return;
    }

    retransmit_earliest();
    if (receiver_window_ != 0) {
      consecutive_retransmissions_++;
      uint64_t rto = timer_.RTO_;
      timer_.start(time_, 2*rto);
    } 
  }
}

void TCPSender::Timer::start(uint64_t start_time, uint64_t RTO)
{
  running_ = true;
  start_time_ = start_time;
  RTO_ = RTO;
}

bool TCPSender::Timer::expire(uint64_t current_time) const
{
  assert(running_ == true);
  return current_time >= (start_time_ + RTO_);
}

void TCPSender::Timer::stop()
{
  running_ = false;
} 