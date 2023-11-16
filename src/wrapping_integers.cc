#include "wrapping_integers.hh"

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
  return {};
}
