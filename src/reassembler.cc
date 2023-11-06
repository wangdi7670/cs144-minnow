#include "reassembler.hh"

using namespace std;


Reassembler::Packet::Packet(uint64_t first_index, std::string& data, bool is_last_str) :
  first_index_(first_index), 
  data_(std::move(data)),
  is_last_str_(is_last_str)
{}


uint64_t Reassembler::Packet::get_last_index()
{
  return first_index_ + data_.size() - 1;
}


void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  (void)first_index;
  (void)data;
  (void)is_last_substring;
  (void)output;
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return {};
}
