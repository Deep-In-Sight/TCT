#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/core/element.h>
#include <sdk/core/pad.h>
#include <sdk/core/queue.h>

class DummyElement : public Element {
 public:
  DummyElement() {
    sink_ = new Pad(kPadSink, "sink");
    AddPad(sink_);
  }
  ~DummyElement() { delete sink_; }
  Pad* GetSinkPad() { return sink_; }
  MOCK_METHOD(void, PushFrame, (cv::Mat & frame), (override));
  Pad* sink_;
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
  DummyElement elem;
  queue_src->Link(elem.GetSinkPad());

  EXPECT_CALL(elem, PushFrame).Times(num_frame);

  cv::Mat frame;
  for (int i = 0; i < num_frame; i++) {
    queue.PushFrame(frame);
  }
  this_thread::sleep_for(chrono::milliseconds(1));
}