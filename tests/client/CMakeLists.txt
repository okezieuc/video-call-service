add_executable(video_call_client_test
  main.cpp
  FrameHandlerTest.cpp
  MainWindowTest.cpp
)
target_link_libraries(video_call_client_test PRIVATE
  GTest::gtest
  video_call_client_lib
)

gtest_discover_tests(video_call_client_test)
