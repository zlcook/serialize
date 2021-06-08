add_executable(
    MemDeviceTest
    MemDeviceTest.cpp
    )

target_link_libraries(
    MemDeviceTest
    glog
    gtest
    )
add_test(NAME MemDeviceTest COMMAND MemDeviceTest)
set_tests_properties(MemDeviceTest
    PROPERTIES PFAIL_REGULAR_EXPRESSION "FAILED")

add_executable(
    ArchiveTest
    ArchiveTest.cpp
    )

target_link_libraries(
    ArchiveTest
    glog
    gtest
    boost_serialization
    )
add_test(NAME ArchiveTest COMMAND ArchiveTest)
set_tests_properties(ArchiveTest
    PROPERTIES PFAIL_REGULAR_EXPRESSION "FAILED")

add_executable(
    MsgArchiveTest
    MsgArchiveTest.cpp
    )

target_link_libraries(
    MsgArchiveTest
    glog
    gtest
    boost_serialization
    )
add_test(NAME MsgArchiveTest COMMAND MsgArchiveTest)
set_tests_properties(MsgArchiveTest
    PROPERTIES PFAIL_REGULAR_EXPRESSION "FAILED")

