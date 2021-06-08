#pragma once

#include <memory>
/**
 * char buffer
 * 1. add more char
 * 2. resize, assign
 * */

namespace zlcook {

struct SharedBuffer {
  typedef std::shared_ptr<char> shared_array_type;

  static void deleter(char* p) { delete[] p; }

  SharedBuffer(size_t size) : size_(0) { Resize(size); }

  SharedBuffer(shared_array_type data, size_t size)
      : data_(data), size_(size) {}

  SharedBuffer(const SharedBuffer& buf) : data_(buf.data_), size_(buf.size_) {}

  SharedBuffer(SharedBuffer&& buf)
      : data_(std::move(buf.data_)), size_(buf.size_) {
    buf.size_ = 0;
  }

  SharedBuffer& operator=(const SharedBuffer&) = default;

  SharedBuffer& operator=(SharedBuffer&&) = default;

  void Resize(size_t new_size) {
    if (new_size > size_) {
      data_.reset(new char[new_size], SharedBuffer::deleter);
    }
    size_ = new_size;
  }

  shared_array_type data_;
  // capacity size of data_
  std::size_t size_;
};

}  // namespace zlcook
