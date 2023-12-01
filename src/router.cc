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

  RouteRule rule{route_prefix, prefix_length, next_hop, interface_num};
  route_table_.insert(rule);
}

bool Router::RouteRule::is_match_rule(uint32_t ip_address) const
{
  uint32_t x = 1U << 31;

  for (uint8_t i = 0; i < prefix_length_; i++) {
    uint32_t temp = x >> i;
    if ((temp & ip_address) != (temp & route_prefix_)) {
      return false;
    }
  }

  return true;
}

std::optional<Router::RouteRule> Router::longest_prefix_match(const InternetDatagram& internet_datagram) const
{
  // route_table_  是按照prefix_length_ 的长度有小到大排序的，
  // 我这里倒序遍历，所以第一个符合要求的 route_rule 即 longest-prefix-match
  for (auto it = route_table_.rbegin(); it != route_table_.rend(); it++) {
    if ((*it).is_match_rule(internet_datagram.header.dst)) {
      return *it;
    }
  }

  return {};
}

void Router::route() 
{
  for (auto& interface : interfaces_) {
    while (true) {
      // if no packet in this interface, stop looking this
      std::optional<InternetDatagram> packet = interface.maybe_receive();
      if (!packet.has_value()) {
        break;
      }
      auto internet_datagram = packet.value();
      
      //  If the TTL was zero already, or hits zero after the decrement, the router should drop the datagram
      if (internet_datagram.header.ttl == 0 || (--internet_datagram.header.ttl) == 0) {
        continue;
      }

      // find the longest-prefix-match rule
      // If no routes matched, the router drops the datagram.
      auto rule = longest_prefix_match(internet_datagram);
      if (!rule.has_value()) {
        continue;
      }
      RouteRule route_rule = rule.value();

      internet_datagram.header.compute_checksum();

      if (route_rule.next_hop_.has_value()) {
        interfaces_[route_rule.interface_index_].send_datagram(internet_datagram, route_rule.next_hop_.value());
      } else {
        Address next_hop = Address::from_ipv4_numeric(internet_datagram.header.dst);
        interfaces_[route_rule.interface_index_].send_datagram(internet_datagram, next_hop);
      }
    }
  }
}

Router::RouteRule::RouteRule(uint32_t route_prefix, uint8_t prefix_length, const std::optional<Address>& next_hop, size_t interface_index) :
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
    return prefix_length_ <= other.prefix_length_;
  }
}
