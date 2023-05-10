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
  virtual void paint(){};
};

/*TODO: change this mock_method when changing the InspectorClient interface */
class WindowMock : public Window {
 public:
  WindowMock(const std::string w_name) : Window(w_name){};

 public:
  MOCK_METHOD(void, Update, (GstBuffer*), (override));
  MOCK_METHOD(void, SetCaps, (GstCaps*), (override));
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
    for (auto window : test_windows) {
      delete window;
    }
    delete test_inspector;
    gst_object_unref(test_pad);
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

TEST_F(InspectorTestSuite, TestAddClients) {
  for (auto window : test_windows) {
    test_inspector->AddClient(window);
  }
  EXPECT_EQ(test_inspector->GetNumClients(), test_windows.size())
      << "couldn't add all subs";
}

TEST_F(InspectorTestSuite, TestRemoveClient) {
  test_inspector->AddClient(test_windows[0]);
  EXPECT_EQ(test_inspector->GetNumClients(), 1);
  test_inspector->RemoveClient(test_windows[0]);
  EXPECT_EQ(test_inspector->GetNumClients(), 0);
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
  int num_buffers = 20;
  // how to test a single function without setting all these attach/subscribe?
  test_inspector->Attach(test_pad);

  for (auto window : test_windows) {
    test_inspector->AddClient(window);
    EXPECT_CALL(*window, Update).Times(num_buffers);
  }

  GstBuffer* buffer;
  for (int count = 0; count < num_buffers; count++) {
    buffer = gst_buffer_new();
    gst_pad_push(test_pad, buffer);
  }
  /* wait a lil bit because the update method is called from different
   * thread??!! */
  std::this_thread::sleep_for(30ms);
}

TEST_F(InspectorTestSuite, TestCapsUpdated) {
  int num_events = 1;

  test_inspector->Attach(test_pad);

  for (auto window : test_windows) {
    test_inspector->AddClient(window);
    EXPECT_CALL(*window, SetCaps);
  }

  for (int i = 0; i < num_events; i++) {
    GstCaps* caps = gst_caps_from_string("somecaps");
    GstEvent* event = gst_event_new_caps(caps);

    gst_pad_push_event(test_pad, event);

    /*TODO: something still owns these events/caps that make it raise a
     * CRITICAL assertion 'GST_MINI_OBJECT_REFCOUNT_VALUE(mini_object) > 0'
     * failed after these unrefs*/
    gst_event_unref(event);
    gst_caps_unref(caps);
  }
}
