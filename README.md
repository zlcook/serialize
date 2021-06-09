# serialize
利用boost的Serialization库实现序列化和反序列化

# 类UML图
![uml](https://github.com/zlcook/serialize/blob/main/doc/serialize_uml.png)

* SharedBuffer: 字节数组，序列化化的数据最终存在SharedBuffer中。
* MemSink： 继承[boost::iostreams::sink](https://www.boost.org/doc/libs/1_76_0/libs/iostreams/doc/index.html)类，覆盖write方法，使用SharedBuffer接收写入的数据。
配合boost::iostreams::stream等类，stream输入方法的数据就会调用MemSource的write方法存储起来。
* MemSource: 继承[boost::iostreams::source](https://www.boost.org/doc/libs/1_76_0/libs/iostreams/doc/index.html)类，覆盖read方法，从SharedBuffer对外提供读数据。
配合[boost::iostreams::stream](https://www.boost.org/doc/libs/1_76_0/libs/iostreams/doc/index.html)等类使用，stream输出方法的数据来源就依赖MemSource的read方法提供。

* [boost::archive::binary_oarchive](https://www.boost.org/doc/libs/1_76_0/libs/serialization/doc/index.html)：序列化类，构造是需要提供std::ostream类实例，用于写入实体对象序列化后得到的数据。
* [boost::archive::binary_iarchive](https://www.boost.org/doc/libs/1_76_0/libs/serialization/doc/index.html)：反序列化类，构造是需要提供std::istream类实例，用于读取序列化后的数据进行反序列化生成实体对象。

* OutMsgArchive: 提供Emit业务方法用于添加要序列化的对象
* InMsgArchive: 提供Absorb方法用户得到反序列化的对象，InMsgArchive构造时要提供SharedBuffer对象，该对象为OutMsgArchive得到的序列化后的原始数据。


# 参考
* [Boost Serialization](https://www.boost.org/doc/libs/1_76_0/libs/serialization/doc/index.html)
* [Boost Iostreams](https://www.boost.org/doc/libs/1_76_0/libs/iostreams/doc/index.html)

* 基于[yas](https://github.com/niXman/yas)序列化库实现的案例参考[plato/archive.hpp](https://github.com/zlcook/plato/blob/master/plato/util/archive.hpp)

* 其它序列化库的对比结果: [数据来源](https://github.com/thekvs/cpp-serializers)

|serializer | object's size |  avg. total time|
|---|---|---|
|thrift-binary  | 17017 |  1190.22|
|thrift-compact | 13378 |  3474.32|
|protobuf |    16116 |  2312.78|
|boost  | 17470 |  1195.04|
|msgpack| 13402 |  2560.6|
|cereal|  17416 |  1052.46|
|avro |   16384 |  4488.18|
|yas| 17416  | 302.7|
|yas-compact | 13321 |  2063.34|

