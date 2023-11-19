#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  (void)message;
  (void)reassembler;
  (void)inbound_stream;

  if (message.SYN) {
    valid = true;
    zero_point = message.seqno;
  }

  if (!valid) {
    return;
  }

  uint64_t absolute_num = message.seqno.unwrap(zero_point, Wrap32::si2ab(reassembler.get_expected_index()));
  uint64_t stream_index = message.SYN ? 0 : absolute_num - 1;
  reassembler.insert(stream_index, (std::string&)message.payload, message.FIN, inbound_stream);

  ackno = Wrap32::wrap(Wrap32::si2ab(reassembler.get_expected_index()), zero_point);
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  (void)inbound_stream;

  uint64_t ac = inbound_stream.available_capacity();
  uint16_t window = ac > UINT16_MAX ? UINT16_MAX : ac;
  return TCPReceiverMessage{ackno, window};
}
