#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sdk/core/element.h>
#include <sdk/core/pad.h>

class ElementMock : public Element {
 public:
  ElementMock(const string& name) : Element(name) {}
  ~ElementMock() {}
  void AddPad(Pad* pad) { Element::AddPad(pad); }

  MOCK_METHOD(void, PushFrame, (cv::Mat & frame), (override));
};

TEST(ElementTest, TestElementContructor) {
  string name = "elem";
  ElementMock elem(name);
  EXPECT_EQ(elem.GetName(), name);
}

TEST(ElementTest, TestAddGetPad) {
  string name = "elem";
  ElementMock elem(name);
  Pad pad(kPadSink, "sink");
  EXPECT_EQ(elem.GetPad("sink"), nullptr);
  elem.AddPad(&pad);
  EXPECT_NE(elem.GetPad("sink"), nullptr);
}

TEST(ElementTest, TestPushFrame) {
  string name = "elem";
  ElementMock elem(name);
  Pad pad(kPadSink, "sink");
  elem.AddPad(&pad);
  EXPECT_CALL(elem, PushFrame).Times(1);
  cv::Mat frame(DEFAULT_MAT_SIZE, DEFAULT_MAT_TYPE);
  pad.PushFrame(frame);
}
