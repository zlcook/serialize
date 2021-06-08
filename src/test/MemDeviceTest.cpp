#include <glog/logging.h>
#include <gtest/gtest.h>

#include <boost/iostreams/stream.hpp>

#include "src/MemDevice.hpp"
#include "src/SharedBuffer.hpp"

namespace zlcook {

struct Edge {
  int src;
  int dst;
};

TEST(MemDeviceTest, Basic) {
  MemSink mem_os(500);
  const int count = 2;
  // write
  {
    for (auto i = 0; i < count; i++) {
      Edge edge{i, i + 1};
      char* p = reinterpret_cast<char*>(&edge);
      mem_os.write(p, sizeof(Edge));
    }
  }

  // read
  MemSource mem_is(mem_os.GetBuffer());
  {
    int i = 0;
    Edge edge;
    while (mem_is.read(reinterpret_cast<char*>(&edge), sizeof(Edge)) != -1) {
      ASSERT_EQ(i++, edge.src);
      ASSERT_EQ(i, edge.dst);
    }
    ASSERT_EQ(2, i);
  }
}

TEST(MemDeviceTest, ReadWriteBasedStream) {
  boost::iostreams::stream<MemSink> os(500);
  std::string str = "abcdefghijklmn";
  // write
  {
    os << str.c_str();
    os.flush();
  }

  // read
  boost::iostreams::stream<MemSource> is((*os).GetBuffer());
  {
    std::string read_str;
    while (!is.eof()) {
      is >> read_str;
      ASSERT_EQ(str, read_str);
    }
    std::cout << read_str << "\n";
  }
}

TEST(MemDeviceTest, ReadWriteBasedStream2) {
  MemSink mem_os(500);
  // os 将会调用MemSink的copy construct，在自己
  // 内部维护一个MemSink实例，将于mem_os脱离关系
  boost::iostreams::stream<MemSink> os(mem_os);
  std::string str = "abcdefghijklmn";
  // write
  {
    os << str.c_str();
    os.flush();
  }

  // read
  boost::iostreams::stream<MemSource> is((*os).GetBuffer());
  {
    std::string read_str;
    while (!is.eof()) {
      is >> read_str;
      ASSERT_EQ(str, read_str);
    }
    std::cout << read_str << "\n";
  }
}

}  // namespace zlcook

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
