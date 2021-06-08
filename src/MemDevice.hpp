#pragma once

#include <boost/iostreams/concepts.hpp>
#include <cstring>
#include <iosfwd>

#include "SharedBuffer.hpp"

namespace zlcook {

//  使用场景:
//  1.MemSink要能够存储数据，存储的数据a)自己维护b)使用其它组件维护
//  2.MemSink中存储的数据要能传给MemSource对象 a)传递MemSink对象
//  b)传递MemSink对象中保存的数据
//
class MemSink : public boost::iostreams::sink {
 public:
  MemSink(std::size_t size) : buf_(size) {
    beg_ = buf_.data_.get();
    cur_ = beg_;
    end_ = beg_ + size;
  }

  MemSink(const MemSink& lhs)
      : buf_(lhs.buf_.data_, static_cast<size_t>(lhs.cur_ - lhs.beg_)),
        beg_(buf_.data_.get()),
        cur_(beg_),
        end_(beg_ + buf_.size_) {}

  std::streamsize write(const void* s, std::streamsize size) {
    if (cur_ + size > end_) {
      std::size_t oldsize = static_cast<size_t>(cur_ - beg_);
      std::size_t newsize = size + oldsize * 2;
      SharedBuffer new_buf(newsize);
      std::memcpy(new_buf.data_.get(), buf_.data_.get(), oldsize);
      buf_ = new_buf;
      beg_ = buf_.data_.get();
      cur_ = beg_ + oldsize;
      end_ = beg_ + newsize;
    }

    std::memcpy(cur_, s, size);
    cur_ += size;
    return size;
  }

  SharedBuffer GetBuffer() {
    return SharedBuffer(buf_.data_, static_cast<size_t>(cur_ - beg_));
  }

  size_t Size() { return static_cast<size_t>(cur_ - beg_); }

 private:
  // store data
  SharedBuffer buf_;
  // element size: cur_ - beg_
  char* beg_;
  char* cur_;
  // capacity size: end_ - beg_
  char* end_;
};

/**
 * 从MemSource中读取数据
 * MemSource中数据来源于MemSink中
 *
 * */

class MemSource : public boost::iostreams::source {
 public:
  MemSource(SharedBuffer buf)
      : buf_(buf.data_, buf.size_),
        cur_(buf_.data_.get()),
        end_(cur_ + buf_.size_) {}

  std::streamsize read(void* ptr, std::streamsize size) {
    std::size_t left_size = static_cast<std::size_t>(end_ - cur_);
    size_t r_size = std::min(left_size, static_cast<size_t>(size));
    if (r_size > 0) {
      std::memcpy(ptr, cur_, r_size);
      cur_ += r_size;
      return r_size;
    }
    return -1;  // EOF
  }

  size_t Size() { return static_cast<size_t>(end_ - cur_); }

 private:
  SharedBuffer buf_;
  char* cur_;
  char* end_;
};

}  // namespace zlcook
