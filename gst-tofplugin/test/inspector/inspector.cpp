#include <gmock/gmock.h>
#include <gst/gst.h>
#include <gtest/gtest.h>
#include <lib/inspector/inspector.h>

#include <iostream>
#include <thread>

using namespace std::chrono_literals;

/* jff, aka just for fun */
class Window : public InspectorClient {
 public:
  Window(const std::string w_name){};
  virtual ~Window(){};

 protected:
  virtual void render(){};
};

/*TODO: change this mock_method when changing the InspectorClient interface */
class WindowMock : public Window {
 public:
  WindowMock(const std::string w_name) : Window(w_name){};

 public:
  MOCK_METHOD(void, Update, (void*, size_t), (override));
};

class InspectorTestSuite : public ::testing::Test {
 protected:
  void SetUp() override {
    test_pad = gst_pad_new(NULL, GST_PAD_SRC);
    gst_pad_activate_mode(test_pad, GST_PAD_MODE_PUSH, TRUE);
    test_inspector = new Inspector("inspect1");
    const char* window_names[] = {"hscanner0", "hscanner1", "vscanner0",
                                  "tracker0", "histogram0"};
    for (auto name : window_names) {
      // TODO: all use the same interface for now, do the factory later
      auto window = new WindowMock(name);
      test_windows.push_back(window);
    }
  }

  void TearDown() override {
    gst_object_unref(test_pad);
    for (auto window : test_windows) {
      delete window;
    }
  }

  Inspector* test_inspector;
  GstPad* test_pad;
  std::vector<WindowMock*> test_windows;
};

TEST_F(InspectorTestSuite, TestAttachToPad) {
  int probe_id;

  probe_id = test_inspector->Attach(test_pad);

  EXPECT_NE(probe_id, 0) << "Should not return 0";
}

TEST_F(InspectorTestSuite, TestDetachFromPad) {
  test_inspector->Attach(test_pad);
  test_inspector->Detach();

  /* don't really know how to verify this op,
  Maybe detach then push a buffer to see if callback is executed?
  just let it pass for now */
  SUCCEED();
}

TEST_F(InspectorTestSuite, TestAddSubscribers) {
  int subs_id;
  for (auto window : test_windows) {
    subs_id = test_inspector->AddSubscriber(window);
  }
  EXPECT_EQ(subs_id, test_windows.size()) << "couldn't add all subs";
}

TEST_F(InspectorTestSuite, TestBufferQueued) {
  int num_sample;

  test_inspector->Attach(test_pad);

  GstBuffer* b = gst_buffer_new();
  gst_pad_push(test_pad, b);

  num_sample = test_inspector->GetNumSamples();
  EXPECT_EQ(num_sample, 1) << "buffer not queued";
}

TEST_F(InspectorTestSuite, TestWindowUpdated) {
  for (auto window : test_windows) {
    EXPECT_CALL(*window, Update);
  }
  GstBuffer* b = gst_buffer_new();
  gst_pad_push(test_pad, b);
  /* wait a lil bit because the update method is called from different
   * thread??!! */
  // std::this_thread::sleep_for(10ms);
}
