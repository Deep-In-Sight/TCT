#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/inspector/inspector-queue.h>

#include <iostream>
#include <thread>

using namespace std::chrono_literals;
using testing::NiceMock;

class WindowMock : public PadObserver {
 public:
  WindowMock(const std::string w_name) : name_(w_name){};
  ~WindowMock(){};
  MOCK_METHOD(void, OnNewFrame, (cv::Mat & frame), (override));
  MOCK_METHOD(void, OnFrameFormatChanged, (const MatShape& shape, int type),
              (override));

 private:
  string name_;
};

class InspectorQueueTest : public ::testing::Test {
 protected:
  void SetUp() override {
    test_pad = new Pad(kPadSource, "src");
    test_inspector_queue = new InspectorQueue("inspectorqueue0");
    const char* window_names[] = {"hscanner0", "hscanner1", "vscanner0",
                                  "tracker0", "histogram0"};
    for (auto name : window_names) {
      auto window = new NiceMock<WindowMock>(name);
      test_windows.push_back(window);
    }
  }

  void TearDown() override {
    for (auto window : test_windows) {
      delete window;
    }
    delete test_inspector_queue;
    delete test_pad;
  }

  Pad* test_pad;
  InspectorQueue* test_inspector_queue;
  std::vector<NiceMock<WindowMock>*> test_windows;
};

TEST_F(InspectorQueueTest, TestWindowUpdated) {
  int num_buffers = 20;

  test_pad->AddObserver(test_inspector_queue);

  for (auto window : test_windows) {
    test_inspector_queue->AddInspector(window);
    EXPECT_CALL(*window, OnNewFrame).Times(num_buffers);
  }

  test_pad->SetFrameFormat({1, 10, 10}, CV_32FC1);

  for (int count = 0; count < num_buffers; count++) {
    cv::Mat frame({1, 10, 10}, CV_32FC1);
    test_pad->PushFrame(frame);
  }
  /* wait a lil bit because the update method is called from different
   * thread??!! */
  std::this_thread::sleep_for(1ms);
}
