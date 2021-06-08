#pragma once
#include <glog/logging.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/stream.hpp>
#include <type_traits>

#include "MemDevice.hpp"

namespace zlcook {

/**
 * 通过Emit方法添加Msg元素，Msg会被序列化存储在SINK中
 **/
template <typename MSG, typename SINK = MemSink, typename ENABLE = void>
class OutMsgArchive {
 public:
  using binary_oarchive_t = boost::archive::binary_oarchive;
  using ostream_t = boost::iostreams::stream<SINK>;

  OutMsgArchive(size_t reserved)
      : count_(0),
        postream_(new ostream_t(reserved)),
        poarchive_(new binary_oarchive_t(*postream_)) {}

  OutMsgArchive(const OutMsgArchive& oma) = delete;

  OutMsgArchive(OutMsgArchive&& oma) = delete;

  void Emit(const MSG& msg) {
    (*poarchive_) & msg;
    count_++;
  }

  // flush buffer into SINK
  void Flush() { postream_->flush(); }

  // return count of msg
  size_t Count() const { return count_; }

  // return size of bytes used by msg
  size_t Size() const { return (*postream_)->Size(); }

  // return msg bytes
  SharedBuffer GetBuffer() { return (*postream_)->GetBuffer(); }

 private:
  std::size_t count_;
  std::shared_ptr<ostream_t> postream_;
  std::shared_ptr<binary_oarchive_t> poarchive_;
};

/**
 *  通过Absorb得到MSG实例，该方法会反序列化存储在SOURCE中的数据得到实例
 * */
template <typename MSG, typename SOURCE = MemSource, typename ENABLE = void>
class InMsgArchive {
 public:
  using binary_iarchive_t = boost::archive::binary_iarchive;
  using istream_t = boost::iostreams::stream<SOURCE>;

  InMsgArchive(const SharedBuffer& buffer, size_t count)
      : count_(count),
        pistream_(new istream_t(buffer)),
        piarchive_(new binary_iarchive_t(*pistream_)) {}

  // return MSG install by deserialize
  std::unique_ptr<MSG> Absorb() {
    if (count_ <= 0) {
      return std::unique_ptr<MSG>(nullptr);
    }
    std::unique_ptr<MSG> pmsg(new MSG());
    (*piarchive_) & (*pmsg);
    --count_;
    return pmsg;
  }

  // the count of msg
  size_t Count() { return count_; }

  // the bytes size of all msgs
  size_t Size() { return (*pistream_)->Size(); }

 private:
  size_t count_;
  std::shared_ptr<istream_t> pistream_;
  std::shared_ptr<binary_iarchive_t> piarchive_;
};

// template specialization
// 对于trivial类，不需要oarchive，直接进行内存拷贝即可
template <typename MSG, typename SINK>
class OutMsgArchive<
    MSG, SINK,
    typename std::enable_if<std::is_trivial<MSG>::value &&
                            std::is_standard_layout<MSG>::value>::type> {
 public:
  OutMsgArchive(size_t reserved) : count_(0), psink_(new SINK(reserved)) {}

  OutMsgArchive(const OutMsgArchive& oma) = delete;

  OutMsgArchive(OutMsgArchive&& oma) = delete;

  void Emit(const MSG& msg) {
    psink_->write(&msg, sizeof(msg));
    count_++;
  }

  void Flush() {}

  // return count of msg
  size_t Count() const { return count_; }

  // return size of bytes used by msg
  size_t Size() const { return psink_->Size(); }

  // return msg bytes
  SharedBuffer GetBuffer() { return psink_->GetBuffer(); }

 private:
  size_t count_;
  std::shared_ptr<SINK> psink_;
};

// template specialization
// 对于trivial类，不需要iarchive，直接进行内存拷贝即可
template <typename MSG, typename SOURCE>
class InMsgArchive<
    MSG, SOURCE,
    typename std::enable_if<std::is_trivial<MSG>::value &&
                            std::is_standard_layout<MSG>::value>::type> {
 public:
  InMsgArchive(const SharedBuffer& buffer, size_t count)
      : count_(count), psource_(new SOURCE(buffer)) {}

  std::unique_ptr<MSG> Absorb() {
    if (count_ <= 0) {
      return std::unique_ptr<MSG>(nullptr);
    }
    std::unique_ptr<MSG> pmsg(new MSG());
    CHECK_EQ(sizeof(MSG), psource_->read(pmsg.get(), sizeof(MSG)));
    --count_;
    return pmsg;
  }

  // the count of msg
  size_t Count() { return count_; }

  // the bytes size of all msgs
  size_t Size() { return psource_->Size(); }

 private:
  size_t count_;
  std::shared_ptr<SOURCE> psource_;
};
}  // namespace zlcook
