#include "reassembler.hh"
#include "algorithm"
#include "iostream"

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

  uint64_t av = output.available_capacity();
  uint64_t bound = next_index + av;

  // truncating data
  if (data.size() == 0) {
    if (!is_last_substring || first_index >= bound) {
      return;
    }
    Packet packet(first_index, data, is_last_substring);
    v.push_back(packet);
  }
  else if ( next_index == first_index ) {
    data = data.substr( 0, 0 + av );
    Packet packet( first_index, data, is_last_substring );
    v.push_back( packet );
  }
  else if ( next_index < first_index ) {
    if (first_index >= bound) {
      return;
    }
    uint64_t number = bound - first_index;
    data = data.substr(0, 0+number);

    Packet packet( first_index, data, is_last_substring );
    v.push_back( packet );
  }
  else if ( next_index > first_index ) {
    uint64_t start = next_index - first_index;

    data = data.substr(start, start+av);

    Packet packet( next_index, data, is_last_substring );
    v.push_back( packet );

    if (v.front().first_index_ != next_index) {
      std::cout << "exception: first_index_ != next_index" << "\n";
    }
  } else {
    std::cout << "exception!"
              << "\n";
  }

  deal_with_overlap();

  while (v.size() > 0 && next_index == v.front().first_index_) {
    Packet& p = v.front();
    output.push(p.data_);
    if (p.is_last_str_) {
      output.close();
    }

    next_index += p.data_.size();
    v.erase(v.begin());
  }    
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  uint64_t bytes = 0;
  for (auto packet : v) {
    bytes += packet.data_.size();
  }
  return bytes;
}

void Reassembler::reorder_v()
{
  std::sort(v.begin(), v.end(), [](const Packet& p1, const Packet& p2){
    return p1.first_index_ < p2.first_index_;
  });
}


void Reassembler::deal_with_overlap()
{
  if (v.size() == 0) {
    std::cout << "exception: v.size = 0" << "\n";
  }

  reorder_v();
  std::vector<Packet> ans{};
  ans.push_back(v.front());

  for (uint64_t i = 1; i < v.size(); i++) {
    if (ans.back().get_last_index() >= v[i].first_index_) {
      if (ans.back().get_last_index() < v[i].get_last_index()) {
        uint64_t start = v[i].first_index_;
        uint64_t begin = ans.back().get_last_index() + 1 - start;
        uint64_t end = v[i].get_last_index() - start + 1;

        std::string temp = v[i].data_.substr(begin, end);
        ans.back().data_.append(temp);
      }
    } else {
      ans.push_back(v[i]);
    }
  }

  v = ans;
}


void Reassembler::print_packets() const
{
  for (uint64_t i = 0; i < v.size(); i++) {
    std::cout << "first: " << v[i].first_index_ << " data: " << v[i].data_ << "\n";
  }
}