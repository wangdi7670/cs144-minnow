#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  // Your code here.
  (void)data;
  std::size_t size = std::min(available_capacity(), data.size());

  pushed_size_ += size;

  buffer.append(data.substr(0, size));
}

void Writer::close()
{
  // Your code here.
  closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  // return {};
  return capacity_ - buffer.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  // return {};
  return pushed_size_;
}

string_view Reader::peek() const
{
  // Your code here.
  // return {};
  std::string_view sv(buffer);
  return sv;
}

bool Reader::is_finished() const
{
  // Your code here.
  // return {};
  return closed_ && (buffer.size() == 0);
}

bool Reader::has_error() const
{
  // Your code here.
  // return {};
  return error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  (void)len;
  len = std::min(len, buffer.size());

  buffer.erase(0, len);
  
  poped_size_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  // return {};
  return buffer.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  // return {};
  return poped_size_;
}
