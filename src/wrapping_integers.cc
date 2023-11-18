#include "wrapping_integers.hh"
#include "stdint.h"
#include <iostream>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  (void)n;
  (void)zero_point;

  uint64_t size = 1L << 32;
  uint64_t temp = size - zero_point.raw_value_;
  uint32_t res = 0;

  if (n < temp) {
    res = zero_point.raw_value_ + n;
  } else {
    res = (n - temp) % size;
  }

  return Wrap32 { res };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  (void)zero_point;
  (void)checkpoint;

  uint64_t size = 1L << 32;
  uint32_t index_bound = UINT32_MAX;  // 2^32 - 1
  uint32_t ISN = zero_point.raw_value_;  // initial sequence number

  uint64_t ab{};  // absolute number
  if (ISN <= this->raw_value_ && this->raw_value_ <= index_bound) {
    ab = this->raw_value_ - ISN;
  } else if (this->raw_value_ < ISN) {
    ab = this->raw_value_ + (size - ISN);
  } else {
    std::cout << "exception!" << "\n";
  }

  if (checkpoint <= ab) {
    return ab;
  }

  ab += (checkpoint - ab) / size * size;

  ab = checkpoint - ab <= ab + size - checkpoint ? ab : ab + size;

  return ab;
}
