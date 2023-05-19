#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/core/element.h>
#include <sdk/core/pad.h>

class ElementChild : public Element {
 public:
  ElementChild(const string& name) : Element(name) {
    Pad* src_ = new Pad(kPadSource, "src");
    Pad* sink_ = new Pad(kPadSink, "sink");
    AddPad(src_);
    AddPad(sink_);
  }
  ~ElementChild() {}

  MOCK_METHOD(void, PushFrame, (cv::Mat & frame), (override));

 private:
  Pad* src_;
  Pad* sink_;
};

TEST(ElementTest, TestElementContructor) {
  string name = "elem";
  ElementChild elem(name);
  EXPECT_EQ(elem.GetName(), name);
}

TEST(ElementTest, TestAddGetPad) {
  string name = "elem";
  ElementChild elem(name);
  Pad* pad = elem.GetPad("src_nonexist");
  EXPECT_EQ(pad, nullptr);
  ElementChild elem2(name);
  EXPECT_NE(elem2.GetPad("src"), nullptr);
  EXPECT_NE(elem2.GetPad("sink"), nullptr);
}

TEST(ElementTest, TestPushFrame) {
  string name = "elem";
  ElementChild elem(name);
  Pad* pad = elem.GetPad("sink");
  EXPECT_CALL(elem, PushFrame).Times(1);
  cv::Mat frame;
  pad->PushFrame(frame);
}