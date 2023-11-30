#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  (void)route_prefix;
  (void)prefix_length;
  (void)next_hop;
  (void)interface_num;
}

void Router::route() {}

Router::RouteRule::RouteRule(uint32_t route_prefix, uint8_t prefix_length, std::optional<Address>& next_hop, size_t interface_index) :
  route_prefix_(route_prefix), 
  prefix_length_(prefix_length), 
  next_hop_(next_hop), 
  interface_index_(interface_index)
{}

bool Router::RouteRule::operator<( const RouteRule& other ) const
{
  if (route_prefix_ == other.route_prefix_ && prefix_length_ == other.prefix_length_) {
    return false;
  } else {
    return prefix_length_ < other.prefix_length_;
  }
}
