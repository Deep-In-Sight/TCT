#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/core/base-sink.h>
#include <sdk/core/pad.h>
#include <sdk/core/queue.h>

using namespace std::chrono_literals;

class SinkMock : public BaseSink {
 public:
  SinkMock() {}
  ~SinkMock() {}
  MOCK_METHOD(void, SinkFrame, (cv::Mat & frame), (override));
};

class SinkFake : public BaseSink {
 public:
  SinkFake() {}
  ~SinkFake() {}
  void SinkFrame(cv::Mat& frame) override { this_thread::sleep_for(10ms); }
};

TEST(QueueTest, TestQueueContructor) {
  Queue queue("queue");
  EXPECT_EQ(queue.GetName(), "queue");
  EXPECT_EQ(queue.GetSourcePad()->GetName(), "src");
  EXPECT_EQ(queue.GetSinkPad()->GetName(), "sink");
}

TEST(QueueTest, TestQueuePushFrame) {
  int num_frame = 10;
  Queue queue("queue");
  Pad* queue_src = queue.GetSourcePad();
  SinkMock elem;
  queue_src->Link(elem.GetSinkPad());

  EXPECT_CALL(elem, SinkFrame).Times(num_frame);

  cv::Mat frame(DEFAULT_MAT_SIZE, DEFAULT_MAT_TYPE);
  for (int i = 0; i < num_frame; i++) {
    queue.PushFrame(frame);
  }
  this_thread::sleep_for(1ms);
}

TEST(QueueTest, TestDropFrame) {
  int num_frame = 10;
  int max_queue_depth = 5;
  Queue queue("queue");
  queue.SetMaxQueueDepth(max_queue_depth);
  Pad* queue_src = queue.GetSourcePad();
  SinkFake elem;
  queue_src->Link(elem.GetSinkPad());

  cv::Mat frame(DEFAULT_MAT_SIZE, DEFAULT_MAT_TYPE);
  for (int i = 0; i < num_frame; i++) {
    queue.PushFrame(frame);
  }
  EXPECT_LE(queue.GetQueueDepth(), max_queue_depth);
}