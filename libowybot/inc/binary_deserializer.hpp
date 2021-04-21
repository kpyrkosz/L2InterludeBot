#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>

struct serializer_skip
{
  serializer_skip(std::size_t to_skip)
    : to_skip_(to_skip)
  {
  }

  void operator()(const char*, std::size_t& current_pos) { current_pos += to_skip_; }

private:
  std::size_t to_skip_;
};

class binary_deserializer // inplace serializer
{

  const char* data_;
  const std::size_t max_size_;
  std::size_t current_pos_;

public:
  binary_deserializer(const std::string& binary_stuff)
    : binary_deserializer(binary_stuff.data(), binary_stuff.size())
  {
  }

  binary_deserializer(const char* binary_stuff, std::size_t size)
    : data_(binary_stuff)
    , max_size_(size)
    , current_pos_(0)
  {
  }

  inline binary_deserializer& operator>>(std::string& text);

  template <typename T>
  typename std::enable_if<std::is_pod<T>::value, binary_deserializer&>::type operator>>(T& out);

  template <typename Manip>
  binary_deserializer& operator>>(Manip&& manip);
};

binary_deserializer&
binary_deserializer::operator>>(std::string& text)
{
  if (max_size_ < current_pos_ + sizeof(unsigned))
    throw std::underflow_error("read_string header underflow");
  unsigned size;
  *this >> size;
  if (max_size_ < current_pos_ + size)
    throw std::underflow_error("read_string data underflow");
  text = std::string(data_ + current_pos_, size);
  current_pos_ += size;
  return *this;
}

template <typename Manip>
binary_deserializer&
binary_deserializer::operator>>(Manip&& manip)
{
  manip(data_, current_pos_);
  return *this;
}

template <typename T>
typename std::enable_if<std::is_pod<T>::value, binary_deserializer&>::type
binary_deserializer::operator>>(T& out)
{
  if (max_size_ < current_pos_ + sizeof(T))
    throw std::underflow_error("read_T underflow");
  std::copy(data_ + current_pos_, data_ + current_pos_ + sizeof(T), (char*)&out);
  current_pos_ += sizeof(T);
  return *this;
}
