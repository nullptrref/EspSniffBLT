#pragma once
#include <stdint.h>

template <typename T, typename CharT = uint8_t>
union RawView
{
  RawView(T t) : type(t) {}
  static constexpr size_t size = sizeof(T);
  T type;
  CharT bytes[size / sizeof(CharT)];

  T &operator ->() { return type; }
};