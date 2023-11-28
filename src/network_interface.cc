#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"
#include "assert.h"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}


bool NetworkInterface::has_ip2ethernet_cache(uint32_t ip) const
{
  if (IP2Ethernet_.contains(ip)) {
    size_t expiring_time = IP2Ethernet_.at(ip).second;
    return time_ < expiring_time;
  }
  return false;
}


bool NetworkInterface::has_already_sent_in_last5s( uint32_t ip_address ) const
{
  if (has_requested_ip_.contains(ip_address)) {
    size_t expiring_time = has_requested_ip_.at(ip_address);
    if (time_ < expiring_time) {
      return true;
    }
  }

  return false;
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  (void)dgram;
  (void)next_hop;

  // 注意：internet_datagram的目的地址不一定等于下一跳的地址
  uint32_t next_hop_ip = next_hop.ipv4_numeric();

  EthernetFrame ethernet_frame;

  // If the destination Ethernet address is already known, send it right away
  if (IP2Ethernet_.contains(next_hop_ip)) {

    ethernet_frame.header.type = EthernetHeader::TYPE_IPv4;
    ethernet_frame.header.src = ethernet_address_;
    ethernet_frame.header.dst = IP2Ethernet_.at(next_hop_ip).first;

    ethernet_frame.payload = serialize( dgram );

    ethernetFrame_queue_.push(ethernet_frame);
  } else {
    if (has_already_sent_in_last5s(next_hop_ip)) {
      // just wait
    } else {
      // broadcast an ARP request
      ethernet_frame.header.type = EthernetHeader::TYPE_ARP;
      ethernet_frame.header.src = ethernet_address_;
      ethernet_frame.header.dst = ETHERNET_BROADCAST;

      ARPMessage arp_request;
      arp_request.opcode = ARPMessage::OPCODE_REQUEST;
      arp_request.sender_ethernet_address = ethernet_address_;
      arp_request.sender_ip_address = ip_address_.ipv4_numeric();
      arp_request.target_ip_address = next_hop_ip;

      ethernet_frame.payload = serialize(arp_request);

      ethernetFrame_queue_.push(ethernet_frame);

      // record self ARP request
      has_requested_ip_.insert({next_hop_ip, (time_+HAS_SENT_TIME)});
    }

    // queue the IP datagram so it can be sent after the ARP reply is received
    temporary_ip_packet_.insert({next_hop_ip, dgram});
  }
}

void NetworkInterface::send_internet_datagram_queue()
{
  for (auto iter = temporary_ip_packet_.begin(); iter != temporary_ip_packet_.end(); ) {
    if (has_ip2ethernet_cache((*iter).first)) {
      EthernetFrame ef;
      ef.header.dst = IP2Ethernet_.at((*iter).first).first;
      ef.header.src = ethernet_address_;
      ef.header.type = EthernetHeader::TYPE_IPv4;
      ef.payload = serialize((*iter).second);

      ethernetFrame_queue_.push(ef);

      iter = temporary_ip_packet_.erase(iter); 
    } else {
      iter++;
    }
  }
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  (void)frame;

  if (frame.header.type == EthernetHeader::TYPE_IPv4 && frame.header.dst == ethernet_address_) {
    InternetDatagram internet_datagram;
    if (parse(internet_datagram, frame.payload)) {
      return internet_datagram;
    }
  } else if (frame.header.type == EthernetHeader::TYPE_ARP) {
    ARPMessage ARP_message;
    if (parse(ARP_message, frame.payload)) {
      IP2Ethernet_[ARP_message.sender_ip_address] = {ARP_message.sender_ethernet_address, (time_+ARP_CACHE_TIME)};

      has_requested_ip_[ARP_message.target_ip_address] = (time_+HAS_SENT_TIME);

      // if it’s an ARP request asking for our IP address, send an appropriate ARP reply
      if (ARP_message.opcode == ARPMessage::OPCODE_REQUEST && ARP_message.target_ip_address == ip_address_.ipv4_numeric()) {
        ARPMessage arp_reply;
        
        arp_reply.opcode = ARPMessage::OPCODE_REPLY;
        arp_reply.sender_ip_address = ip_address_.ipv4_numeric();
        arp_reply.sender_ethernet_address = ethernet_address_;
        arp_reply.target_ip_address = ARP_message.sender_ip_address;
        arp_reply.target_ethernet_address = ARP_message.sender_ethernet_address;

        EthernetFrame ef;
        ef.payload = serialize(arp_reply);
        ef.header.type = EthernetHeader::TYPE_ARP;
        ef.header.src = ethernet_address_;
        ef.header.dst = ARP_message.sender_ethernet_address;

        ethernetFrame_queue_.push(ef);
      }

      send_internet_datagram_queue();
    }
  }
  return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  (void)ms_since_last_tick;

  time_ += ms_since_last_tick;

  // erase expired elements
  for (auto iter = IP2Ethernet_.begin(); iter != IP2Ethernet_.end(); ) {
    if (time_ >= (*iter).second.second) {
      iter = IP2Ethernet_.erase(iter);
    } else {
      iter++;
    }
  }

  for (auto iter = has_requested_ip_.begin(); iter != has_requested_ip_.end(); ) {
    if (time_ >= (*iter).second) {
      iter = has_requested_ip_.erase(iter);
    } else {
      iter++;
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if (!ethernetFrame_queue_.empty()) {
    EthernetFrame ef = ethernetFrame_queue_.front();
    ethernetFrame_queue_.pop();
    return ef;
  }
  return {};
}
