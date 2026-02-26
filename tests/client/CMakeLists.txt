add_executable(video_call_client_test
  MainWindowTest.cpp
)
target_link_libraries(video_call_client_test GTest::gtest_main)

gtest_discover_tests(video_call_client_test)
