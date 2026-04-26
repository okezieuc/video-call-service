add_executable(video_call_server_test
  main.cpp
  UdpRelayTest.cpp
)
target_link_libraries(video_call_server_test PRIVATE
  GTest::gtest
  video_call_server_lib
)

gtest_discover_tests(video_call_server_test)
