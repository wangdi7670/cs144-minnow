#pragma once

#include "byte_stream.hh"

#include <string>

class Reassembler
{
  class Packet
  {
  public:
    uint64_t first_index_;
    std::string data_;
    bool is_last_str_;

    Packet(uint64_t first_index, std::string& data, bool is_last_str);

    // get the upper bound of this packet
    uint64_t get_last_index();
  };

private:
  uint64_t next_index {};
  std::vector<Packet> v {};
  
  // reorder v by the first_index of packet
  void reorder_v();

  // ensure no overlapping in v;
  void deal_with_overlap();
  

public:
  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;

  // get expected index
  uint64_t get_expected_index() const {return next_index;}

  // print member variables "v" 
  void print_packets() const;
};
